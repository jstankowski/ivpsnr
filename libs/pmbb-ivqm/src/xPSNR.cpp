/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
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
uint64V4 xPSNR::calcPicSSD(const xPicP* Tst, const xPicP* Ref)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst));
  uint64V4 SSDs = xCalcPicSSD(Tst, Ref);
  return SSDs;
}
flt64V4 xPSNR::calcPicMSE(const xPicP* Tst, const xPicP* Ref)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst));
  uint64V4 SSDs = xCalcPicSSD(Tst, Ref);
  flt64V4  MSEs = CalcMSEsFromSSDs((flt64V4)SSDs, Tst->getArea());
  return MSEs;
}
flt64V4 xPSNR::calcPicPSNR(const xPicP* Tst, const xPicP* Ref)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst));
  uint64V4 SSDs  = xCalcPicSSD(Tst, Ref);
  flt64V4  PSNRs = CalcPSNRsFromSSDs((flt64V4)SSDs, Tst->getArea(), Tst->getBitDepth());
  return PSNRs;  
}
uint64V4 xPSNR::calcPicSSDM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk)
{
  assert(Ref != nullptr && Tst != nullptr && Msk != nullptr && Ref->isCompatible(Tst) && Ref->isSameSizeMargin(Msk));

  uint64V4 SSDMs = xCalcPicSSDM(Tst, Ref, Msk);
  return SSDMs;
}
flt64V4 xPSNR::calcPicMSEM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, int32 NumNonMasked)
{
  assert(Ref != nullptr && Tst != nullptr && Msk != nullptr && Ref->isCompatible(Tst) && Ref->isSameSizeMargin(Msk));

  if(NumNonMasked < 0) { NumNonMasked = xPixelOps::CountNonZero(Msk->getAddr(eCmp::LM), Msk->getStride(), Msk->getWidth(), Msk->getHeight()); }

  uint64V4 SSDMs = xCalcPicSSDM(Tst, Ref, Msk);
  flt64V4  MSEMs = CalcMSEsFromMaskedSSDs((flt64V4)SSDMs, Tst->getArea(), NumNonMasked, Msk->getBitDepth());
  return MSEMs;
}
flt64V4 xPSNR::calcPicPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, int32 NumNonMasked)
{
  assert(Ref != nullptr && Tst != nullptr && Msk != nullptr && Ref->isCompatible(Tst) && Ref->isSameSizeMargin(Msk));

  if(NumNonMasked < 0) { NumNonMasked = xPixelOps::CountNonZero(Msk->getAddr(eCmp::LM), Msk->getStride(), Msk->getWidth(), Msk->getHeight()); }

  uint64V4 SSDMs  = xCalcPicSSDM(Tst, Ref, Msk);
  flt64V4  PSNRMs = CalcPSNRsFromMaskedSSDs((flt64V4)SSDMs, Tst->getArea(), NumNonMasked, Msk->getBitDepth(), Msk->getBitDepth());

  if(m_DebugCallbackMSK) { m_DebugCallbackMSK(NumNonMasked); }

  return PSNRMs;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

uint64V4 xPSNR::xCalcPicSSD(const xPicP* Tst, const xPicP* Ref)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst));

  uint64V4 SSDs = xMakeVec4<uint64>(0);
  for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
  {
    m_ThPI->storeTask([this, &SSDs, &Tst, &Ref, CmpIdx](int32 /*ThIdx*/) { SSDs[CmpIdx] = xCalcCmpSSD(Tst, Ref, (eCmp)CmpIdx); });
  }
  m_ThPI->executeStoredTasks();

  return SSDs;
}
uint64V4 xPSNR::xCalcPicSSDM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk)
{
  assert(Ref != nullptr && Tst != nullptr && Msk != nullptr && Ref->isCompatible(Tst) && Ref->isSameSizeMargin(Msk));

  uint64V4 SSDMs = xMakeVec4<uint64>(0);
  for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
  {
    m_ThPI->storeTask([this, &SSDMs, &Tst, &Ref, &Msk, CmpIdx](int32 /*ThIdx*/) { SSDMs[CmpIdx] = xCalcCmpSSDM(Tst, Ref, Msk, (eCmp)CmpIdx); });
  }
  m_ThPI->executeStoredTasks();

  return SSDMs;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

uint64 xPSNR::xCalcCmpSSD(const xPicP* Tst, const xPicP* Ref, eCmp CmpId)
{
  const int32   Width     = Ref->getWidth   ();
  const int32   Height    = Ref->getHeight  ();
  const uint16* TstPtr    = Tst->getAddr    (CmpId);
  const uint16* RefPtr    = Ref->getAddr    (CmpId);
  const int32   TstStride = Tst->getStride  ();
  const int32   RefStride = Ref->getStride  ();
  const int32   BitDepth  = Ref->getBitDepth();

  uint64 CmpSSD = xDistortion::CalcSSD(TstPtr, RefPtr, TstStride, RefStride, Width, Height, BitDepth);

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

  //uint64 CmpSSD = 0;
  //for(int32 y = 0; y < Height; y++)
  //{
  //  uint64 RowSSD = xDistortion::CalcWeightedSSD(RefPtr, TstPtr, MskPtr, Width);
  //  CmpSSD += RowSSD;
  //  TstPtr += TstStride;
  //  RefPtr += RefStride;
  //  MskPtr += MskStride;
  //}

  uint64 CmpSSD = xDistortion::CalcWeightedSSD(TstPtr, RefPtr, MskPtr, TstStride, RefStride, MskStride, Width, Height);

  return CmpSSD;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

flt64V4 xPSNR::CalcMSEsFromSSDs(flt64V4 SSDs, int64 Area)
{
  flt64V4  MSEs = xMakeVec4(std::numeric_limits<flt64>::max());
  for(int32 c = 0; c < m_NumComponents; c++) { MSEs[c] = SSDs[c] / (flt64)Area; }
  return MSEs;
}
flt64V4 xPSNR::CalcPSNRsFromSSDs(flt64V4 SSDs, int64 Area, int32 BitDepth)
{
  const uint64 MaxValue = xBitDepth2MaxValue(BitDepth);
  const uint64 MAX      = uint64(Area) * xPow2(MaxValue);

  flt64V4  PSNRs = xMakeVec4(std::numeric_limits<flt64>::max());
  for(int32 c = 0; c < m_NumComponents; c++)
  {
    const flt64 SSD = SSDs[c];
    flt64 PSNR = SSD > 0 ? 10.0 * log10((flt64)MAX / SSD) : std::numeric_limits<flt64>::max();
    if(m_FakeValsForExact && SSD == 0) { PSNR = CalcPSNRfromSSD(1, Area, BitDepth); } //fake PSNR to avoid returning flt64_max
    PSNRs[c] = PSNR;
  }
  return PSNRs;
}
flt64V4 xPSNR::CalcMSEsFromMaskedSSDs(flt64V4 SSDMs, int64 /*Area*/, int64 NumNonMasked, int32 BitDepthMsk)
{
  assert(NumNonMasked >= 0);
  const int64 MaxValueMsk = xBitDepth2MaxValue(BitDepthMsk);

  flt64V4 MSEMs = xMakeVec4(std::numeric_limits<flt64>::max());
  for(int32 c = 0; c < m_NumComponents; c++)
  { 
    MSEMs[c] = SSDMs[c] / (flt64)(NumNonMasked * MaxValueMsk);
  }
  return MSEMs;
}
flt64V4 xPSNR::CalcPSNRsFromMaskedSSDs(flt64V4 SSDMs, int64 Area, int64 NumNonMasked, int32 BitDepthPic, int32 BitDepthMsk)
{
  assert(NumNonMasked >= 0);
  const int64 MaxValuePic = xBitDepth2MaxValue(BitDepthPic);
  const int64 MaxValueMsk = xBitDepth2MaxValue(BitDepthMsk);
  const int64 MAX         = NumNonMasked * xPow2(MaxValuePic) * MaxValueMsk;

  flt64V4 PSNRMs = xMakeVec4(std::numeric_limits<flt64>::max());
  for(int32 c = 0; c < m_NumComponents; c++)
  {
    const flt64 SSDM = SSDMs[c];
    flt64 PSNRM = SSDM > 0 ? 10.0 * log10((flt64)MAX / SSDM) : std::numeric_limits<flt64>::max();
    if(m_FakeValsForExact && SSDM == 0) { PSNRM = CalcPSNRfromSSD(1, Area, BitDepthPic); } //fake PSNR to avoid returning flt64_max  }
    PSNRMs[c] = PSNRM;
  }
  return PSNRMs;
}
flt64 xPSNR::CalcMSEfromSSD(flt64 SSD, int64 Area)
{
  flt64 MSE = SSD / (flt64)Area;
  return MSE;
}
flt64 xPSNR::CalcPSNRfromSSD(flt64 SSD, int64 Area, int32 BitDepth)
{
  uint64 MaxValue  = xBitDepth2MaxValue(BitDepth);
  uint64 MAX       = uint64(Area) * xPow2(MaxValue);
  flt64  PSNR = SSD > 0 ? 10.0 * log10((flt64)MAX / SSD) : std::numeric_limits<flt64>::max();
  return PSNR;
}
flt64 xPSNR::CalcMSEfromMaskedSSD(flt64 SSDM, int64 NumNonMasked, int32 BitDepthMsk)
{
  const int64 MaxValueMsk = xBitDepth2MaxValue(BitDepthMsk);
  flt64 MSEM = SSDM / (flt64)(NumNonMasked * MaxValueMsk);
  return MSEM;
}
flt64 xPSNR::CalcPSNRfromMaskedSSD(flt64 SSDM, int64 NumNonMasked, int32 BitDepthPic, int32 BitDepthMsk)
{
  const int64 MaxValuePic = xBitDepth2MaxValue(BitDepthPic);
  const int64 MaxValueMsk = xBitDepth2MaxValue(BitDepthMsk);
  const int64 MAX         = NumNonMasked * xPow2(MaxValuePic) * MaxValueMsk;
  flt64       PSNRM       = SSDM > 0 ? 10.0 * log10((flt64)MAX / SSDM) : std::numeric_limits<flt64>::max();
  return PSNRM;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB