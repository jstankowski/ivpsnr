/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xIVPSNR.h"
#include "xGlobalColorShift.h"
#include "xPixelOps.h"
#include <cassert>
#include <numeric>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xIVPSNRM
//===============================================================================================================================================================================================================
flt64 xIVPSNRM::calcPicIVPSNRM(const xPicP* Ref, const xPicP* Tst, const xPicP* Msk, const xPicI* RefI, const xPicI* TstI)
{
  assert(Ref != nullptr && Tst != nullptr && Msk != nullptr && RefI != nullptr && TstI != nullptr);
  assert(Ref ->isCompatible    (Tst ));
  assert(RefI->isCompatible    (TstI));
  assert(Ref ->isSameSizeMargin(Msk ));

  const int32 NumNonMasked = xPixelOps::CountNonZero(Msk->getAddr(eCmp::LM), Msk->getStride(), Msk->getWidth(), Msk->getHeight());

  const int32V4 GlobalColorShiftRef2Tst = CalcGlobalColorShiftM(Ref, Tst, Msk, m_CmpUnntcbCoef, NumNonMasked , &m_ThreadPoolIf);
  const int32V4 GlobalColorShiftTst2Ref = -GlobalColorShiftRef2Tst;
    
  flt64 R2T = xCalcQualAsymmetricPicM(RefI, TstI, Msk, GlobalColorShiftRef2Tst, NumNonMasked);
  flt64 T2R = xCalcQualAsymmetricPicM(TstI, RefI, Msk, GlobalColorShiftTst2Ref, NumNonMasked);

  flt64 IVPSNR = xMin(R2T, T2R);

  if(m_DebugCallbackGCS) { m_DebugCallbackGCS(GlobalColorShiftRef2Tst); }
  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(R2T, T2R               ); }
  if(m_DebugCallbackMSK) { m_DebugCallbackMSK(NumNonMasked           ); }

  return IVPSNR;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q interleaved
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
flt64 xIVPSNRM::xCalcQualAsymmetricPicM(const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32V4& GlobalColorShift, const int32 NumNonMasked)
{
  const int32 Height = Ref->getHeight();

  if(m_ThreadPoolIf.isActive())
  {
    for(int32 y = 0; y < Height; y++)
    {
      m_ThreadPoolIf.addWaitingTask(
        [this, &Tst, &Ref, &Msk, &GlobalColorShift, y](int32 /*ThreadIdx*/)
        {
          const uint64V4 RowDist = xCalcDistAsymmetricRowM(Ref, Tst, Msk, y, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
          for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { m_RowDistortions[CmpIdx][y] = RowDist[CmpIdx]; }
        });
    }
    m_ThreadPoolIf.waitUntilTasksFinished(Height);
  }
  else
  {
    for(int32 y = 0; y < Height; y++)
    {
      const uint64V4 RowDist = xCalcDistAsymmetricRowM(Ref, Tst, Msk, y, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { m_RowDistortions[CmpIdx][y] = RowDist[CmpIdx]; }
    }
  }

  flt64V4 FrameError = { 0, 0, 0, 0 };
  if(m_UseWS)
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { m_RowErrors[CmpIdx][y] = (flt64)(m_RowDistortions[CmpIdx][y]) * m_EquirectangularWeights[y]; }
    }

    for(uint32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { FrameError[CmpIdx] = Accumulate(m_RowErrors[CmpIdx]); }
  }
  else //!m_UseWS
  {    
    for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { FrameError[CmpIdx] = (flt64)std::accumulate(m_RowDistortions[CmpIdx].begin(), m_RowDistortions[CmpIdx].end(), (uint64)0); }
  }

  flt64V4 FrameQuality  = { 0, 0, 0, 0 };
  const int64 MaxValuePic = Ref->getMaxPelValue();
  const int64 MaxValueMsk = Msk->getMaxPelValue();
  const int64 MAX         = (int64)NumNonMasked * xPow2(MaxValuePic) * MaxValueMsk;

  for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
  { 
    FrameQuality[CmpIdx] = FrameError[CmpIdx] > 0 ? 10 * log10((flt64)MAX / (flt64)FrameError[CmpIdx]) : flt64_max;
  }

  const int32V4 CmpWeightsAverage             = c_UseRuntimeCmpWeights ? m_CmpWeightsAverage : c_DefaultCmpWeights;
  const int32   SumCmpWeight                  = CmpWeightsAverage.getSum();
  const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;
  const flt64   WeightedFrameQuality          = (FrameQuality * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
  return WeightedFrameQuality;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB