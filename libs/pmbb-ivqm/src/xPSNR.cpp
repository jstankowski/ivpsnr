/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xPSNR.h"
#include "xDistortion.h"
#include "xPixelOps.h"
#include <cassert>
#include <numeric>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xPSNR
//===============================================================================================================================================================================================================
flt64V4 xPSNR::calcPicPSNR(const xPicP* Tst, const xPicP* Ref)
{
  assert(Ref != nullptr && Tst != nullptr);
  assert(Ref->isCompatible(Tst));

  flt64V4 PSNR = xMakeVec4(flt64_max );
  
  if(m_ThPI.isActive())
  {
    for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
    {
      m_ThPI.addWaitingTask([this, &PSNR, &Tst, &Ref, CmpIdx](int32 /**/) { PSNR[CmpIdx] = xCalcCmpPSNR(Tst, Ref, (eCmp)CmpIdx); });
    }
    m_ThPI.waitUntilTasksFinished(m_NumComponents);
  }
  else
  {
    for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
    {
      PSNR[CmpIdx] = xCalcCmpPSNR(Tst, Ref, (eCmp)CmpIdx);
    }
  }

  return PSNR;
}
flt64V4 xPSNR::calcPicPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, int32 NumNonMasked)
{
  assert(Ref != nullptr && Tst != nullptr && Msk != nullptr);
  assert(Ref->isCompatible    (Tst));
  assert(Ref->isSameSizeMargin(Msk));

  if(NumNonMasked == NOT_VALID) { NumNonMasked = xPixelOps::CountNonZero(Msk->getAddr(eCmp::LM), Msk->getStride(), Msk->getWidth(), Msk->getHeight()); }

  flt64V4 PSNR = xMakeVec4(flt64_max);

  if(m_ThPI.isActive())
  {
    for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
    {
      m_ThPI.addWaitingTask([this, &PSNR, Tst, Ref, Msk, NumNonMasked, CmpIdx](int32 /*ThreadIdx*/) { PSNR[CmpIdx] = xCalcCmpPSNRM(Tst, Ref, Msk, NumNonMasked, (eCmp)CmpIdx); });
    }
    m_ThPI.waitUntilTasksFinished(m_NumComponents);
  }
  else
  {
    for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
    {
      PSNR[CmpIdx] = xCalcCmpPSNRM(Tst, Ref, Msk, NumNonMasked, (eCmp)CmpIdx);
    }
  }

  if(m_DebugCallbackMSK) { m_DebugCallbackMSK(NumNonMasked); }

  return PSNR;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

flt64 xPSNR::xCalcCmpPSNR(const xPicP* Tst, const xPicP* Ref, eCmp CmpId)
{
  uint64 SSD  = xCalcCmpSSD(Tst, Ref, CmpId);
  flt64  PSNR = CalcPSNRfromSSD((flt64)SSD, Tst->getArea(), Tst->getBitDepth());

  if(m_FakeValsForExact && SSD == 0) { PSNR = CalcPSNRfromSSD(1, Tst->getArea(), Tst->getBitDepth()); } //fake PSNR to avoid returning flt64_max

  return PSNR;
}
flt64 xPSNR::xCalcCmpPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32 NumNonMasked, eCmp CmpId)
{
  uint64 SSD  = xCalcCmpSSDM(Tst, Ref, Msk, CmpId);
  flt64  PSNR = CalcPSNRfromMaskedSSD((flt64)SSD, NumNonMasked, Tst->getBitDepth(), Msk->getBitDepth());

  if(m_FakeValsForExact && SSD == 0) { PSNR = CalcPSNRfromSSD(1, Tst->getArea(), Tst->getBitDepth()); } //fake PSNR to avoid returning flt64_max

  return PSNR;
}
uint64 xPSNR::xCalcCmpSSD(const xPicP* Tst, const xPicP* Ref, eCmp CmpId)
{
  const int32   Width     = Ref->getWidth ();
  const int32   Height    = Ref->getHeight();
  const uint16* TstPtr    = Tst->getAddr  (CmpId);
  const uint16* RefPtr    = Ref->getAddr  (CmpId);
  const int32   TstStride = Tst->getStride();
  const int32   RefStride = Ref->getStride();

  uint64 CmpSSD = 0;
  for(int32 y = 0; y < Height; y++)
  {
    uint64 RowSSD = xDistortion::CalcSSD(RefPtr, TstPtr, Width);
    CmpSSD += RowSSD;
    TstPtr += TstStride;
    RefPtr += RefStride;
  }

  return CmpSSD;
}
uint64 xPSNR::xCalcCmpSSDM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, eCmp CmpId)
{
  const int32   Width     = Ref->getWidth ();
  const int32   Height    = Ref->getHeight();
  const uint16* TstPtr    = Tst->getAddr  (CmpId   );
  const uint16* RefPtr    = Ref->getAddr  (CmpId   );
  const uint16* MskPtr    = Msk->getAddr  (eCmp::LM);
  const int32   TstStride = Tst->getStride();
  const int32   RefStride = Ref->getStride();
  const int32   MskStride = Msk->getStride();

  uint64 CmpSSD = 0;
  for(int32 y = 0; y < Height; y++)
  {
    uint64 RowSSD = xDistortion::CalcWeightedSSD(RefPtr, TstPtr, MskPtr, Width);
    CmpSSD += RowSSD;
    TstPtr += TstStride;
    RefPtr += RefStride;
    MskPtr += MskStride;
  }  

  return CmpSSD;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

flt64 xPSNR::CalcPSNRfromSSD(flt64 SSD, int32 Area, int32 BitDepth)
{
  uint64 NumPoints = Area;
  uint64 MaxValue  = xBitDepth2MaxValue(BitDepth);
  uint64 MAX       = (NumPoints) * xPow2(MaxValue);
  flt64  PSNR = SSD > 0 ? 10.0 * log10((flt64)MAX / SSD) : flt64_max;
  return PSNR;
}

flt64 xPSNR::CalcPSNRfromMaskedSSD(flt64 SSD, int32 NumNonMasked, int32 BitDepthPic, int32 BitDepthMsk)
{
  const int64 MaxValuePic = xBitDepth2MaxValue(BitDepthPic);
  const int64 MaxValueMsk = xBitDepth2MaxValue(BitDepthMsk);
  const int64 MAX         = (int64)NumNonMasked * xPow2(MaxValuePic) * MaxValueMsk;
  flt64       PSNR        = SSD > 0 ? 10.0 * log10((flt64)MAX / SSD) : flt64_max;
  return PSNR;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB