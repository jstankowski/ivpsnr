/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xIVSSIM.h"
#include "xKBNS.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xIVSSIM
//===============================================================================================================================================================================================================

void xIVSSIM::create(int32V2 Size, int32 BitDepth, int32 Margin, bool EnableMS, bool EnableMask)
{
  xSSIM::create(Size, BitDepth, Margin, EnableMS, EnableMask);

#if X_PMBB_STALLED
  if(EnableMS)
  {
    int32V2 LastSize = m_PicSize;
    for(int32 i = 1; i < m_NumScales; i++)
    {
      int32V2 NewSize = LastSize >> 1;
      m_PicSCPDownsampledTst[i] = new xPicP(NewSize, BitDepth, Margin);
      m_PicSCPDownsampledRef[i] = new xPicP(NewSize, BitDepth, Margin);
      LastSize = NewSize;
    }
  }
#endif //X_PMBB_STALLED
}
void xIVSSIM::destroy()
{
  xSSIM::destroy();
#if X_PMBB_STALLED
  for(int32 i = 1; i < c_NumMultiScales; i++)
  {
    if(m_PicSCPDownsampledTst[i]) { m_PicSCPDownsampledTst[i]->destroy(); delete m_PicSCPDownsampledTst[i]; m_PicSCPDownsampledTst[i] = nullptr; }
    if(m_PicSCPDownsampledRef[i]) { m_PicSCPDownsampledRef[i]->destroy(); delete m_PicSCPDownsampledRef[i]; m_PicSCPDownsampledRef[i] = nullptr; }
  }
#endif //X_PMBB_STALLED
}
flt64 xIVSSIM::calcPicIVSSIM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst));
  assert(TstSCP != nullptr && RefSCP != nullptr && TstSCP->isCompatible(Ref) && RefSCP->isCompatible(Tst));

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
flt64 xIVSSIM::calcPicIVMSSSIM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst));
  assert(TstSCP != nullptr && RefSCP != nullptr && TstSCP->isCompatible(Ref) && RefSCP->isCompatible(Tst));

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
flt64 xIVSSIM::calcPicIVSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP, const xPicP* Msk)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst) && TstSCP != nullptr && TstSCP->isCompatible(Ref) && RefSCP != nullptr && RefSCP->isCompatible(Tst));

  flt64V4 SSIMs_T2R = xCalcPicSSIMM(Tst, RefSCP, Msk, m_UseWS, true);
  flt64V4 SSIMs_R2T = xCalcPicSSIMM(Ref, TstSCP, Msk, m_UseWS, true);

  const int32V4 CmpWeightsAverage             = m_CmpWeightsAverage;
  const int32   SumCmpWeight                  = CmpWeightsAverage.getSum();
  const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;

  flt64 SSIM_T2R = (SSIMs_T2R * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
  flt64 SSIM_R2T = (SSIMs_R2T * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;

  flt64 IVSSIM = xMin(SSIM_T2R, SSIM_R2T);

  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(SSIM_R2T, SSIM_T2R); }
  
  return IVSSIM;
}
flt64 xIVSSIM::calcPicIVMSSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP, const xPicP* Msk)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst));
  assert(TstSCP != nullptr && RefSCP != nullptr && TstSCP->isCompatible(Ref) && RefSCP->isCompatible(Tst));

  flt64V4 MSSSIMs_T2R = calcPicMSSSIMM(Tst, RefSCP, Msk);
  flt64V4 MSSSIMs_R2T = calcPicMSSSIMM(Ref, TstSCP, Msk);

  const int32V4 CmpWeightsAverage             = m_CmpWeightsAverage;
  const int32   SumCmpWeight                  = CmpWeightsAverage.getSum();
  const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;

  flt64 MSSSIM_T2R = (MSSSIMs_T2R * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
  flt64 MSSSIM_R2T = (MSSSIMs_R2T * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;

  flt64 IVMSSSIM = xMin(MSSSIM_T2R, MSSSIM_R2T);

  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(MSSSIM_R2T, MSSSIM_T2R); }
  
  return IVMSSSIM;
}
#if X_PMBB_STALLED
flt64 xIVSSIM::calcPicMSIVSSIM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst));
  assert(TstSCP != nullptr && RefSCP != nullptr && TstSCP->isCompatible(Ref) && RefSCP->isCompatible(Tst));

  std::array<flt64V4, c_NumMultiScales> SubScoresT2R; SubScoresT2R.fill(xMakeVec4<flt64>(0));
  std::array<flt64V4, c_NumMultiScales> SubScoresR2T; SubScoresR2T.fill(xMakeVec4<flt64>(0));

  SubScoresT2R[0] = xCalcPicSSIM(Tst, RefSCP, m_UseWS, m_NumScales == 1);
  SubScoresR2T[0] = xCalcPicSSIM(Ref, TstSCP, m_UseWS, m_NumScales == 1);
  for (int32 i = 1; i < m_NumScales; i++)
  {
    if (i == 1) { xDownsamplePic(m_SubPicTst[i], Tst               ); xDownsamplePic(m_SubPicRef[i], Ref               ); }
    else        { xDownsamplePic(m_SubPicTst[i], m_SubPicTst[i - 1]); xDownsamplePic(m_SubPicRef[i], m_SubPicRef[i - 1]); }

    int32V4 GlobalColorDiffRef2Tst = xGlobClrDiff::CalcGlobalColorDiff(m_SubPicRef[i], m_SubPicTst[i], m_CmpUnntcbCoef, m_ThPI);
    xShftCompPic::GenShftCompPics(m_PicSCPDownsampledRef[i], m_PicSCPDownsampledTst[i], m_SubPicRef[i], m_SubPicTst[i], GlobalColorDiffRef2Tst, m_SearchRange, m_CmpWeightsSearch, m_ThPI);

    SubScoresT2R[i] = xCalcPicSSIM(m_SubPicTst[i], m_PicSCPDownsampledRef[i], m_UseWS, i == m_NumScales - 1);
    SubScoresR2T[i] = xCalcPicSSIM(m_SubPicRef[i], m_PicSCPDownsampledTst[i], m_UseWS, i == m_NumScales - 1);
  }

  const std::array<flt64, c_NumMultiScales> MultiScaleExponentWeights = c_MultiScaleExponentWeights<flt64>[m_NumScales - 1];
  //T2R
  //hint: sometimes SubScore can be negative, so do the same as pytorch - use ReLU to avoid (-0.sth)^Scale
  std::array<flt64V4, c_NumMultiScales> m_CorrectedSubScoresT2R;
  for (int32 i = 0; i < c_NumMultiScales; i++) { m_CorrectedSubScoresT2R[i] = SubScoresT2R[i].getVecReLU(); }
  flt64V4 CompoundScoreT2R = xMakeVec4<flt64>(1);
  for (int32 i = 0; i < m_NumScales; i++) { CompoundScoreT2R *= m_CorrectedSubScoresT2R[i].getVecPow1(MultiScaleExponentWeights[i]); }

  //R2T
  std::array<flt64V4, c_NumMultiScales> m_CorrectedSubScoresR2T;
  for (int32 i = 0; i < c_NumMultiScales; i++) { m_CorrectedSubScoresR2T[i] = SubScoresR2T[i].getVecReLU(); }
  flt64V4 CompoundScoreR2T = xMakeVec4<flt64>(1);
  for (int32 i = 0; i < m_NumScales; i++) { CompoundScoreR2T *= m_CorrectedSubScoresR2T[i].getVecPow1(MultiScaleExponentWeights[i]); }

  flt64V4 MSIVSSIMs_T2R = CompoundScoreT2R;
  flt64V4 MSIVSSIMs_R2T = CompoundScoreR2T;

  const int32V4 CmpWeightsAverage = m_CmpWeightsAverage;
  const int32   SumCmpWeight = CmpWeightsAverage.getSum();
  const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;

  flt64 MSIVSSIM_T2R = (MSIVSSIMs_T2R * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
  flt64 MSIVSSIM_R2T = (MSIVSSIMs_R2T * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;

  flt64 MSIVSSIM = xMin(MSIVSSIM_T2R, MSIVSSIM_R2T);
  return MSIVSSIM;
}
#endif //X_PMBB_STALLED

void xIVSSIM::visualizeIVSSIM(xPlane<uint16>* Vis, const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP, eCmp CmpId)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst) && TstSCP != nullptr && TstSCP->isCompatible(Ref) && RefSCP != nullptr && RefSCP->isCompatible(Tst));
  assert(Vis != nullptr);

  xVisPicSSIM(Vis, Tst, RefSCP, CmpId, false);
  xVisPicSSIM(Vis, Ref, TstSCP, CmpId, true );
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
  memset(m_RowSumsSSIM[(int32)CmpId].data(), 0, m_RowSumsSSIM[(int32)CmpId].size() * sizeof(flt64));

  for(int32 y = m_LoopBegY; y < m_LoopEndY; y += m_WndStride)
  {
    m_ThPI->storeTask([this, &Tst, &Ref, CmpId, y, CalcL](int32) { m_RowSumsSSIM[(int32)CmpId][y] = xCalcRowSSIM(Tst, Ref, CmpId, y, CalcL); });
  }
  m_ThPI->executeStoredTasks();

  if(UseWS)
  {
    xKBNS1 SphericalWeightsAcc;
    for(int32 y = m_LoopBegY; y < m_LoopEndY; y += m_WndStride)
    {
      int32 Offset          = m_IsRegular ? y : y + (m_WndSize >> 1);
      flt64 SphericalWeight = m_EquirectangularWeights[Offset];
      m_RowSumsSSIM[(int32)CmpId][y] = m_RowSumsSSIM[(int32)CmpId][y] * SphericalWeight;
      SphericalWeightsAcc += SphericalWeight;
    }
    flt64 SphericalWeightsSum  = SphericalWeightsAcc.result();
    flt64 SphericalWeightsCorr = SphericalWeightsSum / m_NumUnitY;

    flt64 PicSumSSIM  = xKBNS::Accumulate(m_RowSumsSSIM[(int32)CmpId]);
    int64 NumActive   = (int64)m_NumUnitY * (int64)m_NumUnitX;
    flt64 SSIM        = PicSumSSIM / ((flt64)NumActive * SphericalWeightsCorr);
    return SSIM;
  }
  else
  {
    flt64 PicSumSSIM = xKBNS::Accumulate(m_RowSumsSSIM[(int32)CmpId]);
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

flt64V4 xIVSSIM::xCalcPicSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, bool UseWS, bool CalcL)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst) && Ref->getHeight() <= m_PicSize.getY() && Ref->isSameBitDepth(m_BitDepth));
  xInitLoopRanges(Ref->getWidth(), Ref->getHeight());

  flt64V4 SSIM = xMakeVec4<flt64>(0.0);
  for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
  {
    SSIM[CmpIdx] = xCalcCmpSSIMM(Tst, Ref, Msk, (eCmp)CmpIdx, UseWS, CalcL);
  }
  return SSIM;
}
flt64 xIVSSIM::xCalcCmpSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, eCmp CmpId, bool UseWS, bool CalcL)
{
  memset(m_RowSumsSSIM[(int32)CmpId].data(), 0, m_RowSumsSSIM[(int32)CmpId].size() * sizeof(flt64));
  memset(m_RowSumsWght[(int32)CmpId].data(), 0, m_RowSumsWght[(int32)CmpId].size() * sizeof(flt64));

  for(int32 y = m_LoopBegY; y < m_LoopEndY; y += m_WndStride)
  {
    m_ThPI->storeTask([this, &Tst, &Ref,&Msk, CmpId, y, CalcL](int32) { std::tie(m_RowSumsSSIM[(int32)CmpId][y], m_RowSumsWght[(int32)CmpId][y]) = xCalcRowSSIMM(Tst, Ref, Msk, CmpId, y, CalcL); });
  }
  m_ThPI->executeStoredTasks();

  if(UseWS)
  {
    xKBNS1 SphericalWeightsAcc;
    for(int32 y = m_LoopBegY; y < m_LoopEndY; y += m_WndStride)
    {
      int32 Offset          = m_IsRegular ? y : y + (m_WndSize >> 1);
      flt64 SphericalWeight = m_EquirectangularWeights[Offset];
      m_RowSumsSSIM[(int32)CmpId][y] = m_RowSumsSSIM[(int32)CmpId][y] * SphericalWeight;
      m_RowSumsWght[(int32)CmpId][y] = m_RowSumsWght[(int32)CmpId][y] * SphericalWeight;
      SphericalWeightsAcc += SphericalWeight;
    }
    flt64 SphericalWeightsSum  = SphericalWeightsAcc.result();
    flt64 SphericalWeightsCorr = SphericalWeightsSum / m_NumUnitY;

    flt64 PicSumSSIM  = xKBNS::Accumulate(m_RowSumsSSIM[(int32)CmpId]);
    flt64 PicSumWght  = xKBNS::Accumulate(m_RowSumsWght[(int32)CmpId]);
    flt64 SSIM = PicSumSSIM / (PicSumWght * SphericalWeightsCorr);
    return SSIM;
  }
  else
  {
    flt64 PicSumSSIM = xKBNS::Accumulate(m_RowSumsSSIM[(int32)CmpId]);
    flt64 PicSumWght = xKBNS::Accumulate(m_RowSumsWght[(int32)CmpId]);
    flt64 SSIM       = PicSumSSIM / PicSumWght;
    return SSIM;
  }  
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB