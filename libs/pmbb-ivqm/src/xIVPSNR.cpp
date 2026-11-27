/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/


#include "xIVPSNR.h"
#include "xPixelOps.h"
#include "xDistortion.h"
#include "xKBNS.h"
#include <cassert>
#include <numeric>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xIVPSNR
//===============================================================================================================================================================================================================
flt64V2 xIVPSNR::calcPicIVPSNR(const xPicP* Tst, const xPicP* Ref)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst));

  const int32V4 GlobalColorDiffRef2Tst = -xGlobClrDiff::CalcGlobalColorDiff(Tst, Ref, m_CmpUnntcbCoef, m_ThPI);
  const int32V4 GlobalColorDiffTst2Ref = -GlobalColorDiffRef2Tst;
  if(m_DebugCallbackGCS) { m_DebugCallbackGCS(GlobalColorDiffRef2Tst); }

  flt64V2 QualityT2R = xCalcQualAsymmetricPic(Tst, Ref, GlobalColorDiffRef2Tst);
  flt64V2 QualityR2T = xCalcQualAsymmetricPic(Ref, Tst, GlobalColorDiffTst2Ref);
  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(QualityR2T[0], QualityT2R[0]); }

  flt64V2 IVPSNRs = { xMin(QualityR2T[0], QualityT2R[0]), xMin(QualityR2T[1], QualityT2R[1]) };
  return IVPSNRs;
}
flt64V2 xIVPSNR::calcPicIVPSNR(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst) && TstSCP != nullptr && RefSCP != nullptr && TstSCP->isCompatible(Ref) && RefSCP->isCompatible(Tst));

  flt64V2 QualityT2R = xCalcQualAsymmetricPic(Tst, RefSCP);
  flt64V2 QualityR2T = xCalcQualAsymmetricPic(Ref, TstSCP);
  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(QualityR2T[0], QualityT2R[0]); }

  flt64V2 IVPSNRs = { xMin(QualityR2T[0], QualityT2R[0]), xMin(QualityR2T[1], QualityT2R[1]) };
  return IVPSNRs;
}
flt64V2 xIVPSNR::calcPicIVPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, int32 NumNonMasked)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst));
  assert(Msk != nullptr && Msk->isSameSizeMargin(Ref));

  if(NumNonMasked < 0)
  {
    NumNonMasked = xPixelOps::CountNonZero(Msk->getAddr(eCmp::LM), Msk->getStride(), Msk->getWidth(), Msk->getHeight());
    if(m_DebugCallbackMSK) { m_DebugCallbackMSK(NumNonMasked); }
  }

  const int32V4 GlobalColorDiffRef2Tst = -xGlobClrDiff::CalcGlobalColorDiff(Tst, Ref, m_CmpUnntcbCoef, m_ThPI);
  const int32V4 GlobalColorDiffTst2Ref = -GlobalColorDiffRef2Tst;
  if(m_DebugCallbackGCS) { m_DebugCallbackGCS(GlobalColorDiffRef2Tst); }

  flt64V2 QualityT2R = xCalcQualAsymmetricPicM(Tst, Ref, Msk, GlobalColorDiffRef2Tst, NumNonMasked);
  flt64V2 QualityR2T = xCalcQualAsymmetricPicM(Ref, Tst, Msk, GlobalColorDiffTst2Ref, NumNonMasked);  
  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(QualityR2T[0], QualityT2R[0]); }

  flt64V2 IVPSNRMs = { xMin(QualityR2T[0], QualityT2R[0]), xMin(QualityR2T[1], QualityT2R[1]) };
  return IVPSNRMs;
}
flt64V2 xIVPSNR::calcPicIVPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP, const xPicP* Msk, int32 NumNonMasked)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst) && TstSCP != nullptr && TstSCP->isCompatible(Ref) && RefSCP != nullptr && RefSCP->isCompatible(Tst));
  assert(Msk != nullptr && Msk->isSameSizeMargin(Ref));

  flt64V2 QualityT2R = xCalcQualAsymmetricPicM(Tst, RefSCP, Msk, NumNonMasked);
  flt64V2 QualityR2T = xCalcQualAsymmetricPicM(Ref, TstSCP, Msk, NumNonMasked);
  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(QualityR2T[0], QualityT2R[0]); }

  flt64V2 IVPSNRMs = { xMin(QualityR2T[0], QualityT2R[0]), xMin(QualityR2T[1], QualityT2R[1]) };
  return IVPSNRMs;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q 
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
flt64V2 xIVPSNR::xCalcQualAsymmetricPic(const xPicP* Tst, const xPicP* Ref)
{
  flt64V4 CmpError = xMakeVec4<flt64>(0.0);

  for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
  {
    m_ThPI->storeTask([this, &CmpError, &Tst, &Ref, CmpIdx](int32 /*ThreadIdx*/) { CmpError[CmpIdx] = xCalcDistAsymmetricCmp(Tst, Ref, (eCmp)CmpIdx); });
  }
  m_ThPI->executeStoredTasks();

  flt64V4 CmpQualityPSNR = { 0, 0, 0, 0 };
  for(int32 c = 0; c < m_NumComponents; c++) { CmpQualityPSNR[c] = CalcPSNRfromSSD(CmpError[c] > 0 ? CmpError[c] : 1.0, Tst->getArea(), Tst->getBitDepth()); }

  const int32V4 CmpWeightsAverage = m_CmpWeightsAverage;
  const flt64   PicQualityPSNR = CalcAvgPSNRinPSNRDomain(CmpQualityPSNR, CmpWeightsAverage);
  const flt64   PicQualityMSE  = CalcAvgPSNRinMSEDomain (CmpQualityPSNR, CmpWeightsAverage);
  return { PicQualityPSNR, PicQualityMSE };
}
flt64 xIVPSNR::xCalcDistAsymmetricCmp(const xPicP* Tst, const xPicP* Ref, eCmp CmpId)
{  
  const int32   Width     = Ref->getWidth   ();
  const int32   Height    = Ref->getHeight  ();
  const uint16* TstPtr    = Tst->getAddr    (CmpId);
  const uint16* RefPtr    = Ref->getAddr    (CmpId);
  const int32   TstStride = Tst->getStride  ();
  const int32   RefStride = Ref->getStride  ();
  const int32   BitDepth  = Ref->getBitDepth();

  flt64 CmpError = 0.0;
  if(m_UseWS)
  {
    xKBNS1 CmpErrorAcc;
    for(int32 y = 0; y < Height; y++)
    {
      uint64 RowSSD = xDistortion::CalcSSD(RefPtr, TstPtr, Width, BitDepth);
      flt64 RowError = (flt64)RowSSD * m_EquirectangularWeights[y];
      CmpErrorAcc += RowError;
      TstPtr += TstStride;
      RefPtr += RefStride;
    }
    CmpError = CmpErrorAcc.result();
  }
  else
  {
    uint64 CmpSSD = xDistortion::CalcSSD(TstPtr, RefPtr, TstStride, RefStride, Width, Height, BitDepth);
    CmpError = (flt64)CmpSSD;
  }

  return CmpError;
}
flt64V2 xIVPSNR::xCalcQualAsymmetricPic(const xPicP* Tst, const xPicP* Ref, const int32V4& GCD)
{
  const int32 Height = Ref->getHeight();

  for(int32 y = 0; y < Height; y += c_NumRowsInRng) { m_ThPI->storeTask([this, &Tst, &Ref, &GCD, y, Height](int32) { xCalcDistAsymmetricRng(Tst, Ref, GCD, y, xMin(y + c_NumRowsInRng, Height)); }); }
  m_ThPI->executeStoredTasks();

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

  flt64V4 CmpQualityPSNR = { 0, 0, 0, 0 };
  for(int32 c = 0; c < m_NumComponents; c++) { CmpQualityPSNR[c] = CalcPSNRfromSSD(CmpError[c] > 0 ? CmpError[c] : 1.0, Tst->getArea(), Tst->getBitDepth()); }

  const int32V4 CmpWeightsAverage = m_CmpWeightsAverage;
  const flt64   PicQualityPSNR    = CalcAvgPSNRinPSNRDomain(CmpQualityPSNR, CmpWeightsAverage);
  const flt64   PicQualityMSE     = CalcAvgPSNRinMSEDomain (CmpQualityPSNR, CmpWeightsAverage);
  return { PicQualityPSNR, PicQualityMSE };
}
void xIVPSNR::xCalcDistAsymmetricRng(const xPicP* Tst, const xPicP* Ref, const int32V4& GlobalColorDiff, int32 BegY, int32 EndY)
{
  for(int32 y = BegY; y < EndY; y++)
  {
    m_RowDistsV4[y] = tCPS::CalcDistAsymmetricRow(Tst, Ref, y, GlobalColorDiff, m_SearchRange, m_CmpWeightsSearch);
  }
}
flt64V2 xIVPSNR::xCalcQualAsymmetricPicM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, int32 NumNonMasked)
{
  flt64V4 CmpError = xMakeVec4<flt64>(0.0);

  for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
  {
    m_ThPI->storeTask([this, &CmpError, &Tst, &Ref, &Msk, CmpIdx](int32 /*ThreadIdx*/) { CmpError[CmpIdx] = xCalcDistAsymmetricCmpM(Tst, Ref, Msk, (eCmp)CmpIdx); });
  }
  m_ThPI->executeStoredTasks();

  flt64V4 CmpQualityPSNR = { 0, 0, 0, 0 };
  for(int32 c = 0; c < m_NumComponents; c++) { CmpQualityPSNR[c] = CalcPSNRfromMaskedSSD(CmpError[c] > 0 ? CmpError[c] : 1.0, NumNonMasked, Tst->getBitDepth(), c_MetricMaskBitDepth); }

  const int32V4 CmpWeightsAverage = m_CmpWeightsAverage;
  const flt64   PicQualityPSNR = CalcAvgPSNRinPSNRDomain(CmpQualityPSNR, CmpWeightsAverage);
  const flt64   PicQualityMSE  = CalcAvgPSNRinMSEDomain (CmpQualityPSNR, CmpWeightsAverage);
  return { PicQualityPSNR, PicQualityMSE };

}
flt64 xIVPSNR::xCalcDistAsymmetricCmpM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, eCmp CmpId)
{
  const int32   Width     = Ref->getWidth   ();
  const int32   Height    = Ref->getHeight  ();
  const uint16* TstPtr    = Tst->getAddr    (CmpId);
  const uint16* RefPtr    = Ref->getAddr    (CmpId);
  const uint16* MskPtr    = Msk->getAddr    (eCmp::LM);
  const int32   TstStride = Tst->getStride  ();
  const int32   RefStride = Ref->getStride  ();
  const int32   MskStride = Msk->getStride  ();
//const int32   BitDepth  = Ref->getBitDepth();

  flt64 CmpError = 0.0;
  if(m_UseWS)
  {
    xKBNS1 CmpErrorAcc;
    for(int32 y = 0; y < Height; y++)
    {
      uint64 RowSSD = xDistortion::CalcWghtSSD(RefPtr, TstPtr, MskPtr, Width);
      flt64 RowError = (flt64)RowSSD * m_EquirectangularWeights[y];
      CmpErrorAcc += RowError;
      TstPtr += TstStride;
      RefPtr += RefStride;
      MskPtr += MskStride;
    }
    CmpError = CmpErrorAcc.result();
  }
  else
  {
    uint64 CmpSSD = xDistortion::CalcWghtSSD(TstPtr, RefPtr, MskPtr, TstStride, RefStride, MskStride, Width, Height);
    CmpError = (flt64)CmpSSD;
  }

  return CmpError;
}
flt64V2 xIVPSNR::xCalcQualAsymmetricPicM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32V4& GCD, const int32 NumNonMasked)
{
  const int32 Height = Ref->getHeight();

  for(int32 y = 0; y < Height; y += c_NumRowsInRng)
  {
    m_ThPI->storeTask([this, &Tst, &Ref, &Msk, &GCD, y, Height](int32) { xCalcDistAsymmetricRngM(Tst, Ref, Msk, GCD, y, xMin(y + c_NumRowsInRng, Height)); });
  }
  m_ThPI->executeStoredTasks();

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

  flt64V4 CmpQualityPSNR = { 0, 0, 0, 0 };
  for(int32 c = 0; c < m_NumComponents; c++) { CmpQualityPSNR[c] = CalcPSNRfromMaskedSSD(CmpError[c] > 0 ? CmpError[c] : 1.0, NumNonMasked, Tst->getBitDepth(), c_MetricMaskBitDepth); }

  const int32V4 CmpWeightsAverage = m_CmpWeightsAverage;
  const flt64   PicQualityPSNR    = CalcAvgPSNRinPSNRDomain(CmpQualityPSNR, CmpWeightsAverage);
  const flt64   PicQualityMSE     = CalcAvgPSNRinMSEDomain (CmpQualityPSNR, CmpWeightsAverage);
  return { PicQualityPSNR, PicQualityMSE };
}
void xIVPSNR::xCalcDistAsymmetricRngM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32V4& GlobalColorDiff, int32 BegY, int32 EndY)
{
  for(int32 y = BegY; y < EndY; y++)
  {
    m_RowDistsV4[y] = tCPS::CalcDistAsymmetricRowM(Tst, Ref, Msk, y, GlobalColorDiff, m_SearchRange, m_CmpWeightsSearch);
  }
}

//===============================================================================================================================================================================================================
} //end of namespace PMBB