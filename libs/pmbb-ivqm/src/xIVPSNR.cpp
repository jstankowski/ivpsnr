/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xPixelOps.h"
#include "xIVPSNR.h"
#include "xMathUtils.h"
#include <cassert>
#include <numeric>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xIVPSNR
//===============================================================================================================================================================================================================
flt64 xIVPSNR::calcPicIVPSNR(const xPicP* Tst, const xPicP* Ref, const xPicI* TstI, const xPicI* RefI)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst));

  int32V4 GlobalColorDiffRef2Tst = -xGlobClrDiff::CalcGlobalColorDiff(Tst, Ref, m_CmpUnntcbCoef, &m_ThPI);
  if(m_DebugCallbackGCS) { m_DebugCallbackGCS(GlobalColorDiffRef2Tst); }
  
  flt64 IVPSNR = std::numeric_limits<flt64>::quiet_NaN();
  if(RefI != nullptr && TstI != nullptr) { IVPSNR = calcPicIVPSNR(TstI, RefI, GlobalColorDiffRef2Tst); }
  else                                   { IVPSNR = calcPicIVPSNR(Tst , Ref , GlobalColorDiffRef2Tst); } 

  return IVPSNR;
}

flt64 xIVPSNR::calcPicIVPSNR(const xPicP* Tst, const xPicP* Ref, const int32V4& GlobalColorDiffRef2Tst)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst));

  const int32V4 GlobalColorDiffTst2Ref = -GlobalColorDiffRef2Tst;

  flt64 R2T = xCalcQualAsymmetricPic(Tst, Ref, GlobalColorDiffRef2Tst);
  flt64 T2R = xCalcQualAsymmetricPic(Ref, Tst, GlobalColorDiffTst2Ref);  
  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(R2T, T2R); }

  flt64 IVPSNR = xMin(R2T, T2R);
  return IVPSNR;
}
flt64 xIVPSNR::calcPicIVPSNR(const xPicI* Tst, const xPicI* Ref, const int32V4& GlobalColorDiffRef2Tst)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst));

  const int32V4 GlobalColorDiffTst2Ref = -GlobalColorDiffRef2Tst;

  flt64 R2T = xCalcQualAsymmetricPic(Tst, Ref, GlobalColorDiffRef2Tst);
  flt64 T2R = xCalcQualAsymmetricPic(Ref, Tst, GlobalColorDiffTst2Ref);  
  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(R2T, T2R); }

  flt64 IVPSNR = xMin(R2T, T2R);
  return IVPSNR;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q planar
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
flt64 xIVPSNR::xCalcQualAsymmetricPic(const xPicP* Tst, const xPicP* Ref, const int32V4& GCD)
{
  const int32 Height = Ref->getHeight();

  if(m_ThPI.isActive())
  {
    for(int32 y = 0; y < Height; y++) { m_ThPI.addWaitingTask([this, &Tst, &Ref, &GCD, y](int32) { m_RowDistsV4[y] = tCPS::xCalcDistAsymmetricRow(Tst, Ref, y, GCD, m_SearchRange, m_CmpWeightsSearch); }); }
    m_ThPI.waitUntilTasksFinished(Height);
  }
  else
  {
    for(int32 y = 0; y < Height; y++) { m_RowDistsV4[y] = tCPS::xCalcDistAsymmetricRow(Tst, Ref, y, GCD, m_SearchRange, m_CmpWeightsSearch); }
  }

  flt64V4 CmpError = xMakeVec4<flt64>(0.0);
  if(m_UseWS)
  {
    xKBNS4 KBNS; for(int32 y = 0; y < Height; y++) { KBNS.acc((flt64V4)m_RowDistsV4[y] * m_EquirectangularWeights[y]); }
    CmpError = KBNS.result();
  }
  else //!m_UseWS
  {    
    CmpError = (flt64V4)std::accumulate(m_RowDistsV4.begin(), m_RowDistsV4.end(), xMakeVec4<uint64>(0));
  }

  flt64V4 CmpQuality  = { 0, 0, 0, 0 };
  for(int32 c = 0; c < m_NumComponents; c++) { CmpQuality[c] = CalcPSNRfromSSD(CmpError[c] > 0 ? CmpError[c] : 1.0, Tst->getArea(), Tst->getBitDepth()); }

  const int32V4 CmpWeightsAverage = c_UseRuntimeCmpWeights ? m_CmpWeightsAverage : c_DefaultCmpWeights;
  const int32   SumCmpWeight      = CmpWeightsAverage.getSum();
  const flt64   CmpWeightInvDenom = 1.0 / (flt64)SumCmpWeight;
  const flt64   PicQuality        = (CmpQuality * (flt64V4)CmpWeightsAverage).getSum() * CmpWeightInvDenom;
  return PicQuality;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q interleaved
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
flt64 xIVPSNR::xCalcQualAsymmetricPic(const xPicI* Tst, const xPicI* Ref, const int32V4& GCD)
{
  const int32 Height = Ref->getHeight();

  if(m_ThPI.isActive())
  {
    for(int32 y = 0; y < Height; y++) { m_ThPI.addWaitingTask([this, &Tst, &Ref, &GCD, y](int32) { m_RowDistsV4[y] = tCPS::xCalcDistAsymmetricRow(Tst, Ref, y, GCD, m_SearchRange, m_CmpWeightsSearch); }); }
    m_ThPI.waitUntilTasksFinished(Height);
  }
  else
  {
    for(int32 y = 0; y < Height; y++) { m_RowDistsV4[y] = tCPS::xCalcDistAsymmetricRow(Tst, Ref, y, GCD, m_SearchRange, m_CmpWeightsSearch); }
  }

  flt64V4 CmpError = { 0, 0, 0, 0 };
  if(m_UseWS)
  {
    xKBNS4 KBNS; for(int32 y = 0; y < Height; y++) { KBNS.acc((flt64V4)m_RowDistsV4[y] * m_EquirectangularWeights[y]); }
    CmpError = KBNS.result();
  }
  else //!m_UseWS
  {    
    CmpError = (flt64V4)std::accumulate(m_RowDistsV4.begin(), m_RowDistsV4.end(), xMakeVec4<uint64>(0));
  }

  flt64V4 CmpQuality  = { 0, 0, 0, 0 };
  for(int32 c = 0; c < m_NumComponents; c++) { CmpQuality[c] = CalcPSNRfromSSD(CmpError[c] > 0 ? CmpError[c] : 1.0, Tst->getArea(), Tst->getBitDepth()); }

  const int32V4 CmpWeightsAverage = c_UseRuntimeCmpWeights ? m_CmpWeightsAverage : c_DefaultCmpWeights;
  const int32   SumCmpWeight      = CmpWeightsAverage.getSum();
  const flt64   CmpWeightInvDenom = 1.0 / (flt64)SumCmpWeight;
  const flt64   PicQuality        = (CmpQuality * (flt64V4)CmpWeightsAverage).getSum() * CmpWeightInvDenom;
  return PicQuality;
}

//===============================================================================================================================================================================================================
// xIVPSNRM
//===============================================================================================================================================================================================================
flt64 xIVPSNRM::calcPicIVPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const xPicI* TstI, const xPicI* RefI)
{
  assert(Ref != nullptr && Tst != nullptr && Msk != nullptr && RefI != nullptr && TstI != nullptr);
  assert(Ref ->isCompatible    (Tst ));
  assert(RefI->isCompatible    (TstI));
  assert(Ref ->isSameSizeMargin(Msk ));

  const int32 NumNonMasked = xPixelOps::CountNonZero(Msk->getAddr(eCmp::LM), Msk->getStride(), Msk->getWidth(), Msk->getHeight());
  if(m_DebugCallbackMSK) { m_DebugCallbackMSK(NumNonMasked); }

  const int32V4 GlobalColorDiffRef2Tst = xGlobClrDiff::CalcGlobalColorDiffM(Ref, Tst, Msk, m_CmpUnntcbCoef, NumNonMasked , &m_ThPI);
  if(m_DebugCallbackGCS) { m_DebugCallbackGCS(GlobalColorDiffRef2Tst); }

  flt64 IVPSNR = calcPicIVPSNRM(TstI, RefI, Msk, NumNonMasked, GlobalColorDiffRef2Tst);
  return IVPSNR;
}
flt64 xIVPSNRM::calcPicIVPSNRM(const xPicI* Tst, const xPicI* Ref, const xPicP* Msk, int32 NumNonMasked, const int32V4& GlobalColorDiffRef2Tst)
{
  assert(Ref != nullptr && Tst != nullptr && Msk != nullptr);
  assert(Ref->isCompatible(Tst));
  assert(Ref->isSameSizeMargin(Msk));

  const int32V4 GlobalColorDiffTst2Ref = -GlobalColorDiffRef2Tst;

  flt64 R2T = xCalcQualAsymmetricPicM(Ref, Tst, Msk, GlobalColorDiffTst2Ref, NumNonMasked);
  flt64 T2R = xCalcQualAsymmetricPicM(Tst, Ref, Msk, GlobalColorDiffRef2Tst, NumNonMasked);
  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(R2T, T2R); }

  flt64 IVPSNR = xMin(R2T, T2R);
  return IVPSNR;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q interleaved
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
flt64 xIVPSNRM::xCalcQualAsymmetricPicM(const xPicI* Tst, const xPicI* Ref, const xPicP* Msk, const int32V4& GCD, const int32 NumNonMasked)
{
  const int32 Height = Ref->getHeight();

  if(m_ThPI.isActive())
  {
    for(int32 y = 0; y < Height; y++)
    {
      m_ThPI.addWaitingTask([this, &Tst, &Ref, &Msk, &GCD, y](int32) { m_RowDistsV4[y] = tCPS::xCalcDistAsymmetricRowM(Tst, Ref, Msk, y, GCD, m_SearchRange, m_CmpWeightsSearch); });
    }
    m_ThPI.waitUntilTasksFinished(Height);
  }
  else
  {
    for(int32 y = 0; y < Height; y++) { m_RowDistsV4[y] = tCPS::xCalcDistAsymmetricRowM(Tst, Ref, Msk, y, GCD, m_SearchRange, m_CmpWeightsSearch); }
  }

  flt64V4 CmpError = { 0, 0, 0, 0 };
  if(m_UseWS)
  {
    xKBNS4 KBNS; for(int32 y = 0; y < Height; y++) { KBNS.acc((flt64V4)m_RowDistsV4[y] * m_EquirectangularWeights[y]); }
    CmpError = KBNS.result();
  }
  else //!m_UseWS
  {    
    CmpError = (flt64V4)std::accumulate(m_RowDistsV4.begin(), m_RowDistsV4.end(), xMakeVec4<uint64>(0));
  }

  flt64V4 CmpQuality = { 0, 0, 0, 0 };
  for(int32 c = 0; c < m_NumComponents; c++) { CmpQuality[c] = CalcPSNRfromMaskedSSD(CmpError[c] > 0 ? CmpError[c] : 1.0, NumNonMasked, Tst->getBitDepth(), Msk->getBitDepth()); }

  const int32V4 CmpWeightsAverage = c_UseRuntimeCmpWeights ? m_CmpWeightsAverage : c_DefaultCmpWeights;
  const int32   SumCmpWeight      = CmpWeightsAverage.getSum();
  const flt64   CmpWeightInvDenom = 1.0 / (flt64)SumCmpWeight;
  const flt64   PicQuality        = (CmpQuality * (flt64V4)CmpWeightsAverage).getSum() * CmpWeightInvDenom;
  return PicQuality;
}

//===============================================================================================================================================================================================================
} //end of namespace PMBB