/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xWSPSNR.h"
#include "xDistortion.h"
#include "xPixelOps.h"
#include <cassert>
#include <numeric>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xWSPSNR
//===============================================================================================================================================================================================================
void xWSPSNR::init(int32 Height)
{
  for(int32 CmpIdx = 0; CmpIdx < 4; CmpIdx++) {  m_RowDistortions[CmpIdx].resize(Height); }
}
void xWSPSNR::initWS(bool UseWS, int32 /*Width*/, int32 Height, int32 /*BitDepth*/, int32 /*LonRangeDeg*/, int32 LatRangeDeg)
{
  m_UseWS = UseWS;

  if(m_UseWS)
  {
    m_EquirectangularWeights.resize(Height);
    const flt64 EquirectangularHeight = 180.0 * (flt64)Height / (flt64)LatRangeDeg;
    const flt64 EquirectangularOffset = (EquirectangularHeight - Height) / 2.0;
    for(int32 h = 0; h < Height; h++)
    {
      m_EquirectangularWeights[h] = cos((h + EquirectangularOffset - (EquirectangularHeight / 2 - 0.5)) * xc_Pi<flt64> / EquirectangularHeight);
    }
    flt64 SumEquirectangularWeights = Accumulate(m_EquirectangularWeights);
    m_DistortionCorrection = Height / SumEquirectangularWeights;

    for(int32 CmpIdx = 0; CmpIdx < 4; CmpIdx++)
    {
      m_RowErrors[CmpIdx].resize(Height);
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

xPSNR::tRes4 xWSPSNR::calcPicWSPSNR(const xPicP* Tst, const xPicP* Ref)
{
  assert(Ref != nullptr && Tst != nullptr);
  assert(Ref->isCompatible(Tst));

  flt64V4 PSNR  = xMakeVec4(flt64_max);
  boolV4  Exact = xMakeVec4(false);

  if(!m_UseWS)
  {
    std::tie(PSNR, Exact) = calcPicPSNR(Tst, Ref);
  }
  else
  {
    if(m_ThreadPoolIf.isActive())
    {
      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
      {
        m_ThreadPoolIf.addWaitingTask([this, &PSNR, &Exact, &Tst, &Ref, CmpIdx](int32 /*ThreadIdx*/) { std::tie(PSNR[CmpIdx], Exact[CmpIdx]) = calcCmpWSPSNR(Tst, Ref, (eCmp)CmpIdx); });
      }
      m_ThreadPoolIf.waitUntilTasksFinished(3);
    }
    else
    {
      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
      {
        std::tie(PSNR[CmpIdx], Exact[CmpIdx]) = calcCmpWSPSNR(Tst, Ref, (eCmp)CmpIdx);
      }
    }
  }

  if(m_LegacyPeakValue8bitEmulation) //emulates behavior of original WS-PSNR software for 10bit content converted from 8 bit source
  {
    const int32 RealBitDepth = Tst->getBitDepth();
    if(RealBitDepth > 8)
    {
      const int32 RealMaxValue = xBitDepth2MaxValue(RealBitDepth);
      const int32 FakeMaxValue = xBitDepth2MaxValue(8) << (RealBitDepth - 8);
      const flt64 ModifierPSNR = 10 * (log10(xPow2(RealMaxValue)) - log10(xPow2(FakeMaxValue)));
      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { PSNR[CmpIdx] -= ModifierPSNR; }
    }
  }

  return std::make_tuple(PSNR, Exact);
}
xPSNR::tRes4 xWSPSNR::calcPicWSPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk)
{
  assert(Ref != nullptr && Tst != nullptr && Msk != nullptr);
  assert(Ref->isCompatible    (Tst));
  assert(Ref->isSameSizeMargin(Msk));

  const int32 NumNonMasked = xPixelOps::CountNonZero(Msk->getAddr(eCmp::LM), Msk->getStride(), Msk->getWidth(), Msk->getHeight());

  flt64V4 PSNR  = xMakeVec4(flt64_max);
  boolV4  Exact = xMakeVec4(false    );

  if(!m_UseWS)
  {
    std::tie(PSNR, Exact) = calcPicPSNRM(Tst, Ref, Msk);
  }
  else
  {
    if(m_ThreadPoolIf.isActive())
    {
      for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
      {
        m_ThreadPoolIf.addWaitingTask([this, &PSNR, &Exact, &Tst, &Ref, &Msk, &NumNonMasked, CmpIdx](int32 /*ThreadIdx*/) { std::tie(PSNR[CmpIdx], Exact[CmpIdx]) = calcCmpWSPSNRM(Tst, Ref, Msk, NumNonMasked, (eCmp)CmpIdx); });
      }
      m_ThreadPoolIf.waitUntilTasksFinished(m_NumComponents);
    }
    else
    {
      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
      {
        std::tie(PSNR[CmpIdx], Exact[CmpIdx]) = calcCmpWSPSNRM(Tst, Ref, Msk, NumNonMasked, (eCmp)CmpIdx);
      }
    }
  }

  if(m_DebugCallbackMSK) { m_DebugCallbackMSK(NumNonMasked); }

  return std::make_tuple(PSNR, Exact);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

xPSNR::tRes1 xWSPSNR::calcCmpWSPSNR(const xPicP* Tst, const xPicP* Ref, eCmp CmpId)
{
  const int32   Width     = Ref->getWidth ();
  const int32   Height    = Ref->getHeight();
  const uint16* TstPtr    = Tst->getAddr  (CmpId);
  const uint16* RefPtr    = Ref->getAddr  (CmpId);
  const int32   TstStride = Tst->getStride();
  const int32   RefStride = Ref->getStride();

  flt64* RowErrors = m_RowErrors[(int32)CmpId].data();
  for(int32 y = 0; y < Height; y++)
  {
    uint64 RowSSD = xDistortion::CalcSSD(RefPtr, TstPtr, Width);
    RowErrors[y] = (flt64)RowSSD * m_EquirectangularWeights[y];
    TstPtr += TstStride;
    RefPtr += RefStride;
  }

  flt64 FrameDistortion = Accumulate(m_RowErrors[(int32)CmpId]) * m_DistortionCorrection;
  flt64 PSNR  = CalcPSNRfromSSD(FrameDistortion, Tst->getArea(), Tst->getBitDepth());
  bool  Exact = FrameDistortion == 0;
  if(Exact)
  {
    flt64 FakePSNR = CalcPSNRfromSSD(1, Tst->getArea(), Tst->getBitDepth());
    PSNR  = FakePSNR;
  }

  return std::make_tuple(PSNR, Exact);
}
xPSNR::tRes1 xWSPSNR::calcCmpWSPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32 NumNonMasked, eCmp CmpId)
{
  const int32   Width     = Ref->getWidth ();
  const int32   Height    = Ref->getHeight();
  const uint16* TstPtr    = Tst->getAddr  (CmpId   );
  const uint16* RefPtr    = Ref->getAddr  (CmpId   );
  const uint16* MskPtr    = Msk->getAddr  (eCmp::LM);
  const int32   TstStride = Tst->getStride();
  const int32   RefStride = Ref->getStride();
  const int32   MskStride = Msk->getStride();

  flt64* RowErrors = m_RowErrors[(int32)CmpId].data();
  for(int32 y = 0; y < Height; y++)
  {
    uint64 RowSSD = xDistortion::CalcWeightedSSD(RefPtr, TstPtr, MskPtr, Width);
    RowErrors[y] = (flt64)RowSSD * m_EquirectangularWeights[y];
    TstPtr += TstStride;
    RefPtr += RefStride;
    MskPtr += MskStride;
  }

  flt64 FrameDistortion = Accumulate(m_RowErrors[(int32)CmpId]) * m_DistortionCorrection;
  flt64 WSPSNR  = CalcPSNRfromMaskedSSD((flt64)FrameDistortion, NumNonMasked, Tst->getBitDepth(), Msk->getBitDepth());
  bool  Exact = FrameDistortion == 0;
  if(Exact)
  {
    flt64 FakePSNR = CalcPSNRfromSSD(1, Tst->getArea(), Tst->getBitDepth());
    WSPSNR = FakePSNR;
  }

  return std::make_tuple(WSPSNR, Exact);
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
