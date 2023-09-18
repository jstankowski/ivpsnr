/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xIVPSNR.h"
#include "xGlobalColorShift.h"
#include <cassert>
#include <numeric>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xIVPSNR
//===============================================================================================================================================================================================================
flt64 xIVPSNR::calcPicIVPSNR(const xPicP* Ref, const xPicP* Tst, const xPicI* RefI, const xPicI* TstI)
{
  assert(Ref != nullptr && Tst != nullptr);
  assert(Ref->isCompatible(Tst));

  int32V4 GlobalColorShiftRef2Tst = xGlobalColorShift::Calc(Ref, Tst, m_CmpUnntcbCoef, &m_ThreadPoolIf);
  int32V4 GlobalColorShiftTst2Ref = -GlobalColorShiftRef2Tst;
  
  flt64 R2T = std::numeric_limits<flt64>::quiet_NaN();
  flt64 T2R = std::numeric_limits<flt64>::quiet_NaN();
  if(RefI != nullptr && TstI != nullptr)
  {
    R2T = xCalcQualAsymmetricPic(RefI, TstI, GlobalColorShiftRef2Tst);
    T2R = xCalcQualAsymmetricPic(TstI, RefI, GlobalColorShiftTst2Ref);
  }
  else
  {
    R2T = xCalcQualAsymmetricPic(Ref, Tst, GlobalColorShiftRef2Tst);
    T2R = xCalcQualAsymmetricPic(Tst, Ref, GlobalColorShiftTst2Ref);
  }

  flt64 IVPSNR = xMin(R2T, T2R);

  if(m_DebugCallbackGCS) { m_DebugCallbackGCS(GlobalColorShiftRef2Tst); }
  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(R2T, T2R               ); }

  return IVPSNR;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q planar
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
flt64 xIVPSNR::xCalcQualAsymmetricPic(const xPicP* Ref, const xPicP* Tst, const int32V4& GlobalColorShift)
{
  const int32 Height = Ref->getHeight();
  const int32 Area   = Ref->getArea  ();

  if(m_ThreadPoolIf.isActive())
  {
    for(int32 y = 0; y < Height; y++)
    {
      m_ThreadPoolIf.addWaitingTask(
        [this, &Tst, &Ref, &GlobalColorShift, y](int32 /*ThreadIdx*/)
        {
          const int32V4 RowDist = xCalcDistAsymmetricRow(Ref, Tst, y, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
          for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { m_RowDistortions[CmpIdx][y] = RowDist[CmpIdx]; }
        });
    }
    m_ThreadPoolIf.waitUntilTasksFinished(Height);
  }
  else
  {
    for(int32 y = 0; y < Height; y++)
    {
      const int32V4 RowDist = xCalcDistAsymmetricRow(Ref, Tst, y, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { m_RowDistortions[CmpIdx][y] = RowDist[CmpIdx]; }
    }
  }

  flt64V4 FrameError = xMakeVec4<flt64>(0.0);;
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
  flt64   PSNR_20logMAX = 20 * log10((1 << Ref->getBitDepth()) - 1);
  for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { FrameQuality[CmpIdx] = PSNR_20logMAX - 10 * log10((FrameError[CmpIdx]) / Area); }

  const int32V4 CmpWeightsAverage             = c_UseRuntimeCmpWeights ? m_CmpWeightsAverage : c_DefaultCmpWeights;
  const int32   SumCmpWeight                  = CmpWeightsAverage.getSum();
  const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;
  const flt64   WeightedFrameQuality          = (FrameQuality * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
  return WeightedFrameQuality;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q interleaved
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
flt64 xIVPSNR::xCalcQualAsymmetricPic(const xPicI* Ref, const xPicI* Tst, const int32V4& GlobalColorShift)
{
  const int32 Height = Ref->getHeight();
  const int32 Area   = Ref->getArea  ();

  if(m_ThreadPoolIf.isActive())
  {
    for(int32 y = 0; y < Height; y++)
    {
      m_ThreadPoolIf.addWaitingTask(
        [this, &Tst, &Ref, &GlobalColorShift, y](int32 /*ThreadIdx*/)
        {
          const int32V4 RowDist = xCalcDistAsymmetricRow(Ref, Tst, y, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
          for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { m_RowDistortions[CmpIdx][y] = RowDist[CmpIdx]; }
        });
    }
    m_ThreadPoolIf.waitUntilTasksFinished(Height);
  }
  else
  {
    for(int32 y = 0; y < Height; y++)
    {
      const int32V4 RowDist = xCalcDistAsymmetricRow(Ref, Tst, y, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
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
  flt64   PSNR_20logMAX = 20 * log10((1 << Ref->getBitDepth()) - 1);
  for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { FrameQuality[CmpIdx] = PSNR_20logMAX - 10 * log10((FrameError[CmpIdx]) / Area); }

  const int32V4 CmpWeightsAverage             = c_UseRuntimeCmpWeights ? m_CmpWeightsAverage : c_DefaultCmpWeights;
  const int32   SumCmpWeight                  = CmpWeightsAverage.getSum();
  const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;
  const flt64   WeightedFrameQuality          = (FrameQuality * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
  return WeightedFrameQuality;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB