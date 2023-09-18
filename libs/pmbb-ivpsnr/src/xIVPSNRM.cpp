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

  const int32V4 GlobalColorShiftRef2Tst = xGlobalColorShift::CalcM(Ref, Tst, Msk, m_CmpUnntcbCoef, NumNonMasked , &m_ThreadPoolIf);
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

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q interleaved - STD
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint64V4 xIVPSNRM::xCalcDistAsymmetricRowM_STD(const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32  Width     = Tst->getWidth ();
  const int32  TstStride = Tst->getStride();
  const int32  TstOffset = y * TstStride;
  const int32  MskStride = Msk->getStride();
  const int32  MskOffset = y * MskStride;

  uint64V4 RowDist = { 0, 0, 0, 0 };

  const uint16V4* TstPtr = Tst->getAddr(        ) + TstOffset;
  const uint16*   MskPtr = Msk->getAddr(eCmp::LM) + MskOffset;
        
  for(int32 x = 0; x < Width; x++)
  {
    const int32   CurrMskValue  = (int32)MskPtr[x];
    if(CurrMskValue == 0) { continue; } //skip masked pixels
    const int32V4 CurrTstValue  = (int32V4)(TstPtr[x]) + GlobalColorShift;
    const int32   BestRefOffset = xFindBestPixelWithinBlockM_STD(Ref, CurrTstValue, Msk, x, y, SearchRange, CmpWeights);
    const int32V4 Diff = CurrTstValue - (int32V4)(Ref->getAddr()[BestRefOffset]);
    const int32V4 Dist = Diff.getVecPow2();
    RowDist += ((uint64V4)Dist) * CurrMskValue;
  } //x

  return RowDist;
}
int32 xIVPSNRM::xFindBestPixelWithinBlockM_STD(const xPicI* Ref, const int32V4& TstPel, const xPicP* Msk, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 BegY = CenterY - SearchRange;
  const int32 EndY = CenterY + SearchRange;
  const int32 BegX = CenterX - SearchRange;
  const int32 EndX = CenterX + SearchRange;

  const uint16V4* RefPtr = Ref->getAddr  ();
  const uint16*   MskPtr = Msk->getAddr  (eCmp::LM);
  const int32     Stride = Ref->getStride();

  int32 BestError  = std::numeric_limits<int32>::max();
  int32 BestOffset = NOT_VALID;

  for(int32 y = BegY; y <= EndY; y++)
  {
    for(int32 x = BegX; x <= EndX; x++)
    {
      const int32   Offset = y * Stride + x;
      if(MskPtr[Offset] == 0) { continue; }
      const int32V4 RefPel = (int32V4)(RefPtr[Offset]);
      const int32V4 Dist   = (TstPel - RefPel).getVecPow2();
      if constexpr (c_UseRuntimeCmpWeights)
      {
        const int32 Error = (Dist * CmpWeights).getSum();
        if (Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
      else
      {
        const int32 Error = (Dist[0] << 2) + Dist[1] + Dist[2];
        if (Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
    } //x
  } //y

  return BestOffset;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB