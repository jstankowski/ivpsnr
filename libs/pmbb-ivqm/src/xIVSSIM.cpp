/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xIVSSIM.h"
#include "xKBNS.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xIVSSIM
//===============================================================================================================================================================================================================

void xIVSSIM::create(int32V2 Size, int32 BitDepth, int32 Margin, bool EnableMS, bool PreAllocateSCP)
{
  xSSIM::create(Size, BitDepth, Margin, EnableMS);
  if(PreAllocateSCP)
  {
    m_TstSCP = new xPicP(Size, BitDepth, Margin);
    m_RefSCP = new xPicP(Size, BitDepth, Margin);
  }
}
void xIVSSIM::destroy()
{
  if(m_TstSCP != nullptr) { m_TstSCP->destroy(); delete m_TstSCP; m_TstSCP = nullptr; }
  if(m_RefSCP != nullptr) { m_RefSCP->destroy(); delete m_RefSCP; m_RefSCP = nullptr; }
  xSSIM::destroy();
}
flt64 xIVSSIM::calcPicIVSSIM(const xPicP* Tst, const xPicP* Ref)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst));
  if(m_TstSCP != nullptr) { m_TstSCP = new xPicP(); m_TstSCP->create(Tst); }
  if(m_RefSCP != nullptr) { m_RefSCP = new xPicP(); m_RefSCP->create(Tst); }
  assert(m_RefSCP->isCompatible(Ref) && m_TstSCP->isCompatible(Tst)); 

  int32V4 GlobalColorDiffRef2Tst = xGlobClrDiff::CalcGlobalColorDiff(Ref, Tst, m_CmpUnntcbCoef, m_ThPI);

  xShftCompPic::GenShftCompPics(m_RefSCP, m_TstSCP, Ref, Tst, GlobalColorDiffRef2Tst, m_SearchRange, m_CmpWeightsSearch, m_ThPI);

  flt64V4 SSIMs_T2R = xCalcPicSSIM(Tst, m_RefSCP, m_UseWS, true);
  flt64V4 SSIMs_R2T = xCalcPicSSIM(Ref, m_TstSCP, m_UseWS, true);

  const int32V4 CmpWeightsAverage             = m_CmpWeightsAverage;
  const int32   SumCmpWeight                  = CmpWeightsAverage.getSum();
  const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;

  flt64 SSIM_T2R = (SSIMs_T2R * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
  flt64 SSIM_R2T = (SSIMs_R2T * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;

  flt64 IVSSIM = xMin(SSIM_T2R, SSIM_R2T);

  if(m_DebugCallbackGCS) { m_DebugCallbackGCS(GlobalColorDiffRef2Tst); }
  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(SSIM_R2T, SSIM_T2R); }
  
  return IVSSIM;
}
flt64 xIVSSIM::calcPicIVSSIM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst) && TstSCP != nullptr && TstSCP->isCompatible(Ref) && RefSCP != nullptr && RefSCP->isCompatible(Tst));

  flt64V4 SSIMs_T2R = xCalcPicSSIM(Tst, RefSCP, m_UseWS, true);
  flt64V4 SSIMs_R2T = xCalcPicSSIM(Ref, TstSCP, m_UseWS, true);

  const int32V4 CmpWeightsAverage             = m_CmpWeightsAverage;
  const int32   SumCmpWeight                  = CmpWeightsAverage.getSum();
  const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;

  flt64 SSIM_T2R = (SSIMs_T2R * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
  flt64 SSIM_R2T = (SSIMs_R2T * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;

  flt64 IVSSIM = xMin(SSIM_T2R, SSIM_R2T);

  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(SSIM_R2T, SSIM_T2R); }
  
  return IVSSIM;
}
flt64 xIVSSIM::calcPicIVMSSSIM(const xPicP* Tst, const xPicP* Ref)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst) && m_RefSCP->isCompatible(Ref) && m_TstSCP->isCompatible(Tst)); 

  int32V4 GlobalColorDiffRef2Tst = xGlobClrDiff::CalcGlobalColorDiff(Ref, Tst, m_CmpUnntcbCoef, m_ThPI);

  xShftCompPic::GenShftCompPics(m_RefSCP, m_TstSCP, Ref, Tst, GlobalColorDiffRef2Tst, m_SearchRange, m_CmpWeightsSearch, m_ThPI);

  flt64V4 MSSSIMs_T2R = calcPicMSSSIM(Tst, m_RefSCP);
  flt64V4 MSSSIMs_R2T = calcPicMSSSIM(Ref, m_TstSCP);

  const int32V4 CmpWeightsAverage             = m_CmpWeightsAverage;
  const int32   SumCmpWeight                  = CmpWeightsAverage.getSum();
  const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;

  flt64 MSSSIM_T2R = (MSSSIMs_T2R * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
  flt64 MSSSIM_R2T = (MSSSIMs_R2T * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;

  flt64 IVMSSSIM = xMin(MSSSIM_T2R, MSSSIM_R2T);

  if(m_DebugCallbackGCS) { m_DebugCallbackGCS(GlobalColorDiffRef2Tst); }
  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(MSSSIM_R2T, MSSSIM_T2R); }
  
  return IVMSSSIM;
}
flt64 xIVSSIM::calcPicIVMSSSIM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst) && TstSCP != nullptr && TstSCP->isCompatible(Ref) && RefSCP != nullptr && RefSCP->isCompatible(Tst));

  flt64V4 MSSSIMs_T2R = calcPicMSSSIM(Tst, RefSCP);
  flt64V4 MSSSIMs_R2T = calcPicMSSSIM(Ref, TstSCP);

  const int32V4 CmpWeightsAverage             = m_CmpWeightsAverage;
  const int32   SumCmpWeight                  = CmpWeightsAverage.getSum();
  const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;

  flt64 MSSSIM_T2R = (MSSSIMs_T2R * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
  flt64 MSSSIM_R2T = (MSSSIMs_R2T * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;

  flt64 IVMSSSIM = xMin(MSSSIM_T2R, MSSSIM_R2T);

  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(MSSSIM_R2T, MSSSIM_T2R); }
  
  return IVMSSSIM;
}
flt64 xIVSSIM::calcPicIVSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP, const xPicP* Msk, int32 NumNonMasked)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst) && TstSCP != nullptr && TstSCP->isCompatible(Ref) && RefSCP != nullptr && RefSCP->isCompatible(Tst));

  if(NumNonMasked < 0) { NumNonMasked = xPixelOps::CountNonZero(Msk->getAddr(eCmp::LM), Msk->getStride(), Msk->getWidth(), Msk->getHeight()); }

  flt64V4 SSIMs_T2R = xCalcPicSSIMM(Tst, RefSCP, Msk, NumNonMasked, m_UseWS, true);
  flt64V4 SSIMs_R2T = xCalcPicSSIMM(Ref, TstSCP, Msk, NumNonMasked, m_UseWS, true);

  const int32V4 CmpWeightsAverage             = m_CmpWeightsAverage;
  const int32   SumCmpWeight                  = CmpWeightsAverage.getSum();
  const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;

  flt64 SSIM_T2R = (SSIMs_T2R * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
  flt64 SSIM_R2T = (SSIMs_R2T * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;

  flt64 IVSSIM = xMin(SSIM_T2R, SSIM_R2T);

  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(SSIM_R2T, SSIM_T2R); }
  
  return IVSSIM;

}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

flt64V4 xIVSSIM::xCalcPicSSIM(const xPicP* Tst, const xPicP* Ref, bool UseWS, bool CalcL)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst) && Ref->getHeight() <= m_PicSize.getY() && Ref->isSameBitDepth(m_BitDepth));
  xInitLoopRanges(Ref->getWidth(), Ref->getHeight());

  flt64V4 SSIM = xMakeVec4<flt64>(0.0);
  for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
  {
    SSIM[CmpIdx] = xCalcCmpSSIM(Tst, Ref, (eCmp)CmpIdx, UseWS, CalcL);
  }

  return SSIM;
}
flt64 xIVSSIM::xCalcCmpSSIM(const xPicP* Tst, const xPicP* Ref, eCmp CmpId, bool UseWS, bool CalcL)
{
  memset(m_RowSums[(int32)CmpId].data(), 0, m_RowSums[(int32)CmpId].size() * sizeof(flt64));

  for(int32 y = m_LoopBegY; y < m_LoopEndY; y += m_WndStride)
  {
    m_ThPI->storeTask([this, &Tst, &Ref, CmpId, y, CalcL](int32) { m_RowSums[(int32)CmpId][y] = xCalcRowSSIM(Tst, Ref, CmpId, y, CalcL); });
  }
  m_ThPI->executeStoredTasks();

  if(UseWS)
  {
    xKBNS1 WeightsAcc;
    for(int32 y = m_LoopBegY; y < m_LoopEndY; y += m_WndStride)
    {
      int32 Offset = m_IsRegular ? y : y + (m_WndSize >> 1);
      flt64 Weight = m_EquirectangularWeights[Offset];
      m_RowSums[(int32)CmpId][y] = m_RowSums[(int32)CmpId][y] * Weight;
      WeightsAcc += Weight;
    }
    flt64 WeightsSum  = WeightsAcc.result();
    flt64 WeightsCorr = WeightsSum / m_NumUnitY;

    flt64 PicSumSSIM  = xKBNS::Accumulate(m_RowSums[(int32)CmpId]);
    int64 NumActive   = (int64)m_NumUnitY * (int64)m_NumUnitX;
    flt64 SSIM        = PicSumSSIM / ((flt64)NumActive * WeightsCorr);
    return SSIM;
  }
  else
  {
    flt64 PicSumSSIM = xKBNS::Accumulate(m_RowSums[(int32)CmpId]);
    int64 NumActive  = (int64)m_NumUnitY * (int64)m_NumUnitX;
    flt64 SSIM       = PicSumSSIM / (flt64)NumActive;
    return SSIM;
  }  
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

flt64V4 xIVSSIM::xCalcPicMSSSIM(const xPicP* Tst, const xPicP* Ref)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst) && Ref->isSameSize(m_PicSize) && Ref->isSameBitDepth(m_BitDepth));

  std::array<flt64V4, c_NumMultiScales> SubScores; SubScores.fill(xMakeVec4<flt64>(0));

  SubScores[0] = xCalcPicSSIM(Tst, Ref, m_UseWS, m_NumScales==1);
  for(int32 i = 1; i < m_NumScales; i++)
  {
    if(i == 1) { xDownsamplePic(m_SubPicTst[i], Tst             ); xDownsamplePic(m_SubPicRef[i], Ref             ); }
    else       { xDownsamplePic(m_SubPicTst[i], m_SubPicTst[i-1]); xDownsamplePic(m_SubPicRef[i], m_SubPicRef[i-1]); }

    SubScores[i] = xCalcPicSSIM(m_SubPicTst[i], m_SubPicRef[i], m_UseWS, i == m_NumScales - 1);
  }

  //hint: sometimes SubScore can be negative, so do the same as pytorch - use ReLU to avoid (-0.sth)^Scale
  std::array<flt64V4, c_NumMultiScales> m_CorrectedSubScores;
  for(int32 i = 0; i < c_NumMultiScales; i++) { m_CorrectedSubScores[i] = SubScores[i].getVecReLU(); }

  const std::array<flt64, c_NumMultiScales> MultiScaleExponentWeights = c_MultiScaleExponentWeights<flt64>[m_NumScales-1];
  flt64V4 CompoundScore = xMakeVec4<flt64>(1);
  for(int32 i = 0; i < m_NumScales; i++) { CompoundScore *= m_CorrectedSubScores[i].getVecPow1(MultiScaleExponentWeights[i]); }

  return CompoundScore;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

flt64V4 xIVSSIM::xCalcPicSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, int32 NumNonMasked, bool UseWS, bool CalcL)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst) && Ref->getHeight() <= m_PicSize.getY() && Ref->isSameBitDepth(m_BitDepth));
  assert(m_IsRegular && m_WndStride == 1);
  xInitLoopRanges(Ref->getWidth(), Ref->getHeight());

  flt64V4 SSIM = xMakeVec4<flt64>(0.0);
  for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
  {
    SSIM[CmpIdx] = xCalcCmpSSIMM(Tst, Ref, Msk, (eCmp)CmpIdx, NumNonMasked, UseWS, CalcL);
  }
  return SSIM;
}
flt64 xIVSSIM::xCalcCmpSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, eCmp CmpId, int32 NumNonMasked, bool UseWS, bool CalcL)
{
  memset(m_RowSums[(int32)CmpId].data(), 0, m_RowSums[(int32)CmpId].size() * sizeof(flt64));

  for(int32 y = m_LoopBegY; y < m_LoopEndY; y += m_WndStride)
  {
    m_ThPI->storeTask([this, &Tst, &Ref,&Msk, CmpId, y, CalcL](int32) { m_RowSums[(int32)CmpId][y] = xCalcRowSSIMM(Tst, Ref, Msk, CmpId, y, CalcL); });
  }
  m_ThPI->executeStoredTasks();

  if(UseWS)
  {
    xKBNS1 WeightsAcc;
    for(int32 y = m_LoopBegY; y < m_LoopEndY; y += m_WndStride)
    {
      int32 Offset = m_IsRegular ? y : y + (m_WndSize >> 1);
      flt64 Weight = m_EquirectangularWeights[Offset];
      m_RowSums[(int32)CmpId][y] = m_RowSums[(int32)CmpId][y] * Weight;
      WeightsAcc += Weight;
    }
    flt64 WeightsSum  = WeightsAcc.result();
    flt64 WeightsCorr = WeightsSum / m_NumUnitY;

    flt64 PicSumSSIM  = xKBNS::Accumulate(m_RowSums[(int32)CmpId]);
    flt64 SSIM        = PicSumSSIM / ((flt64)NumNonMasked * WeightsCorr);
    return SSIM;
  }
  else
  {
    flt64 PicSumSSIM = xKBNS::Accumulate(m_RowSums[(int32)CmpId]);
    flt64 SSIM       = PicSumSSIM / (flt64)NumNonMasked;
    return SSIM;
  }  
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB