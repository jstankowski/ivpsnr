/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xWSPSNR.h"
#include "xDistortion.h"
#include "xPixelOps.h"
#include "xMathUtils.h"
#include <cassert>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xWSPSNR
//===============================================================================================================================================================================================================
flt64V4 xWSPSNR::calcPicWSPSNR(const xPicP* Tst, const xPicP* Ref)
{
  assert(Ref != nullptr && Tst != nullptr);
  assert(Ref->isCompatible(Tst));

  flt64V4 WSPSNR = xMakeVec4(flt64_max);

  if(!m_UseWS)
  {
    WSPSNR = calcPicPSNR(Tst, Ref);
  }
  else
  {
    if(m_ThPI.isActive())
    {
      for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
      {
        m_ThPI.addWaitingTask([this, &WSPSNR, &Tst, &Ref, CmpIdx](int32 /*ThreadIdx*/) { WSPSNR[CmpIdx] = xCalcCmpWSPSNR(Tst, Ref, (eCmp)CmpIdx); });
      }
      m_ThPI.waitUntilTasksFinished(m_NumComponents);
    }
    else
    {
      for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
      {
        WSPSNR[CmpIdx] = xCalcCmpWSPSNR(Tst, Ref, (eCmp)CmpIdx);
      }
    }
  }

  if(m_LegacyPeakValue8bitEmulation) //emulates behavior of original WS-PSNR software for 10bit content converted from 8 bit source
  {
    const int32 RealBitDepth = Tst->getBitDepth();
    if(RealBitDepth == 10)
    {
      const int32 RealMaxValue = xBitDepth2MaxValue(RealBitDepth);
      const int32 FakeMaxValue = xBitDepth2MaxValue(8) << (RealBitDepth - 8);
      const flt64 ModifierPSNR = 10 * (log10(xPow2(RealMaxValue)) - log10(xPow2(FakeMaxValue)));
      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { WSPSNR[CmpIdx] -= ModifierPSNR; }
    }
  }

  return WSPSNR;
}
flt64V4 xWSPSNR::calcPicWSPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, int32 NumNonMasked)
{
  assert(Ref != nullptr && Tst != nullptr && Msk != nullptr);
  assert(Ref->isCompatible    (Tst));
  assert(Ref->isSameSizeMargin(Msk));

  if(NumNonMasked == NOT_VALID) { NumNonMasked = xPixelOps::CountNonZero(Msk->getAddr(eCmp::LM), Msk->getStride(), Msk->getWidth(), Msk->getHeight()); }

  flt64V4 WSPSNR = xMakeVec4(flt64_max);

  if(!m_UseWS)
  {
    WSPSNR = calcPicPSNRM(Tst, Ref, Msk);
  }
  else
  {
    if(m_ThPI.isActive())
    {
      for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
      {
        m_ThPI.addWaitingTask([this, &WSPSNR, &Tst, &Ref, &Msk, &NumNonMasked, CmpIdx](int32 /*ThreadIdx*/) { WSPSNR[CmpIdx] = xCalcCmpWSPSNRM(Tst, Ref, Msk, NumNonMasked, (eCmp)CmpIdx); });
      }
      m_ThPI.waitUntilTasksFinished(m_NumComponents);
    }
    else
    {
      for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
      {
        WSPSNR[CmpIdx] = xCalcCmpWSPSNRM(Tst, Ref, Msk, NumNonMasked, (eCmp)CmpIdx);
      }
    }
  }

  if(m_DebugCallbackMSK) { m_DebugCallbackMSK(NumNonMasked); }

  return WSPSNR;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

flt64 xWSPSNR::xCalcCmpWSPSNR(const xPicP* Tst, const xPicP* Ref, eCmp CmpId)
{
  const int32   Width     = Ref->getWidth ();
  const int32   Height    = Ref->getHeight();
  const uint16* TstPtr    = Tst->getAddr  (CmpId);
  const uint16* RefPtr    = Ref->getAddr  (CmpId);
  const int32   TstStride = Tst->getStride();
  const int32   RefStride = Ref->getStride();

  uint64* RowSSDs = m_RowDistortions[(int32)CmpId].data();
  for(int32 y = 0; y < Height; y++)
  {
    uint64 RowSSD = xDistortion::CalcSSD(RefPtr, TstPtr, Width);
    RowSSDs[y] = RowSSD;
    TstPtr += TstStride;
    RefPtr += RefStride;
  }

  xKBNS KBNS; for(int32 y = 0; y < Height; y++) { KBNS.acc((flt64)RowSSDs[y] * m_EquirectangularWeights[y]); }
  flt64 CmpError = KBNS.result() * m_DistortionCorrection;
  flt64 WSPSNR   = CalcPSNRfromSSD(CmpError, Tst->getArea(), Tst->getBitDepth());
  
  if(m_FakeValsForExact && CmpError == 0) { WSPSNR = CalcPSNRfromSSD(1, Tst->getArea(), Tst->getBitDepth()); } //fake WSPSNR to avoid returning flt64_max
  
  return WSPSNR;
}
flt64 xWSPSNR::xCalcCmpWSPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32 NumNonMasked, eCmp CmpId)
{
  const int32   Width     = Ref->getWidth ();
  const int32   Height    = Ref->getHeight();
  const uint16* TstPtr    = Tst->getAddr  (CmpId   );
  const uint16* RefPtr    = Ref->getAddr  (CmpId   );
  const uint16* MskPtr    = Msk->getAddr  (eCmp::LM);
  const int32   TstStride = Tst->getStride();
  const int32   RefStride = Ref->getStride();
  const int32   MskStride = Msk->getStride();

  uint64* RowSSDs = m_RowDistortions[(int32)CmpId].data();
  for(int32 y = 0; y < Height; y++)
  {
    uint64 RowSSD = xDistortion::CalcWeightedSSD(RefPtr, TstPtr, MskPtr, Width);
    RowSSDs[y] = RowSSD;
    TstPtr += TstStride;
    RefPtr += RefStride;
    MskPtr += MskStride;
  }

  xKBNS KBNS; for(int32 y = 0; y < Height; y++) { KBNS.acc((flt64)RowSSDs[y] * m_EquirectangularWeights[y]); }
  flt64 CmpError = KBNS.result() * m_DistortionCorrection;
  flt64 WSPSNR   = CalcPSNRfromMaskedSSD(CmpError, NumNonMasked, Tst->getBitDepth(), Msk->getBitDepth());

  if(m_FakeValsForExact && CmpError == 0) { WSPSNR = CalcPSNRfromSSD(1, Tst->getArea(), Tst->getBitDepth()); } //fake WSPSNR to avoid returning flt64_max

  return WSPSNR;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
