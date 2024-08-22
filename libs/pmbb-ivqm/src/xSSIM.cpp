/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xSSIM.h"
#include "xMathUtils.h"
#include "xPixelOps.h"
#include "xString.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xSSIM
//===============================================================================================================================================================================================================
void xSSIM::create(int32V2 Size, int32 BitDepth, int32 /*Margin*/)
{
  m_Size        = Size;
  m_BitDepth    = BitDepth;

  fltTP MaxValue = (fltTP)xBitDepth2MaxValue(BitDepth);
  m_C1 = xPow2(c_K1<fltTP> * MaxValue);
  m_C2 = xPow2(c_K2<fltTP> * MaxValue);

  for(int32 CmpIdx = 0; CmpIdx < 4; CmpIdx++)
  { 
    m_RowSums[CmpIdx].resize(Size.getY(), 0.0);
  }
}
void xSSIM::destroy()
{
  m_Size = { NOT_VALID, NOT_VALID };
  m_C1   = std::numeric_limits<fltTP>::quiet_NaN();
  m_C2   = std::numeric_limits<fltTP>::quiet_NaN();
}
flt64V4 xSSIM::calcPicSSIM(const xPicP* Tst, const xPicP* Ref, bool CalcL)
{
  assert(Ref != nullptr && Tst != nullptr);
  assert(Ref->isCompatible(Tst) && Ref->getHeight() <= m_Size.getY() && Ref->isSameBitDepth(m_BitDepth));

  flt64V4 SSIM = xMakeVec4<flt64>(0.0);
  for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
  {
    SSIM[CmpIdx] = xCalcCmpSSIM(Tst, Ref, (eCmp)CmpIdx, CalcL);
  }

  return SSIM;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

flt64 xSSIM::xCalcCmpSSIM(const xPicP* Tst, const xPicP* Ref, eCmp CmpId, bool CalcL)
{
  const int32 Width  = Ref->getWidth();
  const int32 Height = Ref->getHeight();

  memset(m_RowSums[(int32)CmpId].data(), 0, m_RowSums[(int32)CmpId].size() * sizeof(flt64));

  for(int32 y = c_FilterRange; y < Height - c_FilterRange; y++)
  {
    m_ThPI.addWaitingTask([this, &Tst, &Ref, CmpId, y, CalcL](int32 ) { m_RowSums[(int32)CmpId][y] = xCalcRowSSIM(Tst, Ref, CmpId, y); });
  }
  m_ThPI.waitUntilTasksFinished(Height - 2 * c_FilterRange);

  if(m_UseWS)
  {
    for(int32 y = 0; y < Height; y++) { m_RowSums[(int32)CmpId][y] = m_RowSums[(int32)CmpId][y] * m_EquirectangularWeights[y]; }
  }

  const int64  NumActive = (int64)Width * (int64)Height;
  flt64 PicSumSSIM = xKBNS::Accumulate(m_RowSums[(int32)CmpId]);
  flt64 SSIM = PicSumSSIM / (flt64)NumActive;
  return SSIM;
}
flt64 xSSIM::xCalcRowSSIM(const xPicP* Tst, const xPicP* Ref, eCmp CmpId, const int32 y)
{
  const int32   Width     = Ref->getWidth ();
  const int32   TstStride = Tst->getStride();
  const int32   RefStride = Ref->getStride();
  const uint16* TstPtr    = Tst->getAddr(CmpId) + y * TstStride;
  const uint16* RefPtr    = Ref->getAddr(CmpId) + y * RefStride;
  
  flt64 RowSumSSIM = 0.0;
  for(int32 x = c_FilterRange; x < Width - c_FilterRange; x++)
  {
    flt64 PelSSIM = xStructSim<fltTP>::CalcPel(TstPtr + x, RefPtr + x, TstStride, RefStride, c_FilterF, m_C1, m_C2);
    RowSumSSIM += PelSSIM;
  }

  return RowSumSSIM;
}

//===============================================================================================================================================================================================================
// xIVSSIM
//===============================================================================================================================================================================================================

void xIVSSIM::create(int32V2 Size, int32 BitDepth, int32 Margin)
{
  xSSIM::create(Size, BitDepth, Margin);
  m_TstSCP = new xPicP(Size, BitDepth, Margin);
  m_RefSCP = new xPicP(Size, BitDepth, Margin);
}
void xIVSSIM::destroy()
{
  m_TstSCP->destroy(); delete m_TstSCP; m_TstSCP = nullptr;
  m_RefSCP->destroy(); delete m_RefSCP; m_RefSCP = nullptr;
  xSSIM::destroy();
}
flt64 xIVSSIM::calcPicIVSSIM(const xPicP* Tst, const xPicP* Ref)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst));
  assert(m_RefSCP->isCompatible(Ref) && m_TstSCP->isCompatible(Tst)); 

  int32V4 GlobalColorDiffRef2Tst = xGlobClrDiff::CalcGlobalColorDiff(Ref, Tst, m_CmpUnntcbCoef, &m_ThPI);

  xShftCompPic::GenShftCompPics(m_RefSCP, m_TstSCP, Ref, Tst, GlobalColorDiffRef2Tst, m_SearchRange, m_CmpWeightsSearch, &m_ThPI);

  flt64V4 SSIMs_T2R = calcPicSSIM(Tst, m_RefSCP);
  flt64V4 SSIMs_R2T = calcPicSSIM(Ref, m_TstSCP);

  const int32V4 CmpWeightsAverage             = c_UseRuntimeCmpWeights ? m_CmpWeightsAverage : c_DefaultCmpWeights;
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
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst));
  assert(TstSCP != nullptr && TstSCP->isCompatible(Ref) && RefSCP != nullptr && RefSCP->isCompatible(Tst));

  flt64V4 SSIMs_T2R = calcPicSSIM(Tst, RefSCP);
  flt64V4 SSIMs_R2T = calcPicSSIM(Ref, TstSCP);

  const int32V4 CmpWeightsAverage             = c_UseRuntimeCmpWeights ? m_CmpWeightsAverage : c_DefaultCmpWeights;
  const int32   SumCmpWeight                  = CmpWeightsAverage.getSum();
  const flt64   ComponentWeightInvDenominator = 1.0 / (flt64)SumCmpWeight;

  flt64 SSIM_T2R = (SSIMs_T2R * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;
  flt64 SSIM_R2T = (SSIMs_R2T * (flt64V4)CmpWeightsAverage).getSum() * ComponentWeightInvDenominator;

  flt64 IVSSIM = xMin(SSIM_T2R, SSIM_R2T);

  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(SSIM_R2T, SSIM_T2R); }
  
  return IVSSIM;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB