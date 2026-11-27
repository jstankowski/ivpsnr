/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xPVD.h"
#include "xDistortion.h"
#include "xPixelOps.h"
#include "xCorrespPixelShiftPrms.h"
#include <cassert>
#include <numeric>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xPVD
//===============================================================================================================================================================================================================
flt64 xIVPVD::calcPicPVD(const xPicP* Tst, const xPicP* Ref)
{
  assert(Ref != nullptr && Tst != nullptr);
  assert(Ref->isCompatible(Tst));

  flt64V4 VARs = xCalcPicVAR(Tst, Ref);

  const int32V4 CmpWeightsAverage = c_DefaultCmpWeights;
  const int32   SumCmpWeight      = CmpWeightsAverage.getSum();
  const flt64   CmpWeightInvDenom = 1.0 / (flt64)SumCmpWeight;
  const flt64   MidVal            = (flt64)xBitDepth2MidValue(Ref->getBitDepth());

  flt64 CmbVAR = (VARs * (flt64V4)CmpWeightsAverage).getSum() * CmpWeightInvDenom;
  flt64 PVD    = (flt64)MidVal / (CmbVAR + (flt64)MidVal);
  return PVD;
}
flt64 xIVPVD::calcPicIVPVD(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst) && TstSCP != nullptr && RefSCP != nullptr && TstSCP->isCompatible(Ref) && RefSCP->isCompatible(Tst));

  flt64V4 VARs_T2R = xCalcPicVAR(Tst, RefSCP);
  flt64V4 VARs_R2T = xCalcPicVAR(Ref, TstSCP);

  const int32V4 CmpWeightsAverage = c_DefaultCmpWeights;
  const int32   SumCmpWeight      = CmpWeightsAverage.getSum();
  const flt64   CmpWeightInvDenom = 1.0 / (flt64)SumCmpWeight;
  const flt64   MidVal            = (flt64)xBitDepth2MidValue(Ref->getBitDepth());

  flt64 CmbVAR_T2R = (VARs_T2R * (flt64V4)CmpWeightsAverage).getSum() * CmpWeightInvDenom;
  flt64 CmbVAR_R2T = (VARs_R2T * (flt64V4)CmpWeightsAverage).getSum() * CmpWeightInvDenom;
  flt64 PVD_T2R    = (flt64)MidVal / (CmbVAR_T2R + (flt64)MidVal);
  flt64 PVD_R2T    = (flt64)MidVal / (CmbVAR_R2T + (flt64)MidVal);

  flt64 IVPVD = xMin(PVD_T2R, PVD_R2T);

  if(m_DebugCallbackQAP) { m_DebugCallbackQAP(PVD_R2T, PVD_T2R); }

  return IVPVD;
}
flt64 xIVPVD::calcPicPVDM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk)
{
  assert(Ref != nullptr && Tst != nullptr && Msk != nullptr);
  assert(Ref->isCompatible(Tst) && Ref->isSameSizeMargin(Msk));

  uint64  SumWghts = xCalcSumWeights(Msk);
  if(SumWghts == 0) { return std::numeric_limits<flt64>::quiet_NaN(); }

  flt64V4 VARMs = xCalcPicVARM(Tst, Ref, Msk, SumWghts);

  const int32V4 CmpWeightsAverage = c_DefaultCmpWeights;
  const int32   SumCmpWeight      = CmpWeightsAverage.getSum();
  const flt64   CmpWeightInvDenom = 1.0 / (flt64)SumCmpWeight;
  const flt64   MidVal = (flt64)xBitDepth2MidValue(Ref->getBitDepth());

  flt64 CmbVARM = (VARMs * (flt64V4)CmpWeightsAverage).getSum() * CmpWeightInvDenom;
  flt64 PVDM = (flt64)MidVal / (CmbVARM + (flt64)MidVal);
  return PVDM;
}
flt64 xIVPVD::calcPicIVPVDM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP, const xPicP* Msk)
{
  assert(Tst != nullptr && Ref != nullptr && Ref->isCompatible(Tst) && TstSCP != nullptr && RefSCP != nullptr && TstSCP->isCompatible(Ref) && RefSCP->isCompatible(Tst));
	assert(Msk != nullptr && Ref->isSameSizeMargin(Msk));
	
	uint64 SumWghts = xCalcSumWeights(Msk);
  if(SumWghts == 0) { return std::numeric_limits<flt64>::quiet_NaN(); }

	flt64V4 VARMs_T2R = xCalcPicVARM(Tst, RefSCP, Msk, SumWghts);
	flt64V4 VARMs_R2T = xCalcPicVARM(Ref, TstSCP, Msk, SumWghts);
	
	const int32V4 CmpWeightsAverage = c_DefaultCmpWeights;
	const int32   SumCmpWeight      = CmpWeightsAverage.getSum();
	const flt64   CmpWeightInvDenom = 1.0 / (flt64)SumCmpWeight;
	const flt64   MidVal            = (flt64)xBitDepth2MidValue(Ref->getBitDepth());
	
	flt64 CmbVARM_T2R = (VARMs_T2R * (flt64V4)CmpWeightsAverage).getSum() * CmpWeightInvDenom;
	flt64 CmbVARM_R2T = (VARMs_R2T * (flt64V4)CmpWeightsAverage).getSum() * CmpWeightInvDenom;
	flt64 PVDM_T2R    = (flt64)MidVal / (CmbVARM_T2R + (flt64)MidVal);
	flt64 PVDM_R2T    = (flt64)MidVal / (CmbVARM_R2T + (flt64)MidVal);
	
	flt64 IVPVDM = xMin(PVDM_T2R, PVDM_R2T);
	
	if(m_DebugCallbackQAP) { m_DebugCallbackQAP(PVDM_R2T, PVDM_T2R); }
	
	return IVPVDM;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

flt64V4 xIVPVD::xCalcPicVAR(const xPicP* Tst, const xPicP* Ref)
{
  flt64V4 VARs = xMakeVec4(0.0);
  for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
  {
    m_ThPI->storeTask([&VARs, &Tst, &Ref, CmpIdx](int32 /**/) { VARs[CmpIdx] = xCalcCmpVAR(Tst, Ref, (eCmp)CmpIdx); });
  }
  m_ThPI->executeStoredTasks();
  return VARs;
}
flt64 xIVPVD::xCalcCmpVAR(const xPicP* Tst, const xPicP* Ref, eCmp CmpId)
{
  const int32   Width     = Ref->getWidth   ();
  const int32   Height    = Ref->getHeight  ();
  const int64   Area      = Ref->getArea    ();
  const uint16* TstPtr    = Tst->getAddr    (CmpId);
  const uint16* RefPtr    = Ref->getAddr    (CmpId);
  const int32   TstStride = Tst->getStride  ();
  const int32   RefStride = Ref->getStride  ();
  const int32   BitDepth  = Ref->getBitDepth();

  int64  SD  = 0;
  uint64 SSD = 0;
  std::tie(SD, SSD) = xDistortion::CalcSSS(TstPtr, RefPtr, TstStride, RefStride, Width, Height, BitDepth);

  flt64 AvgSSD = (flt64)SSD / (flt64)Area;
  flt64 AvgSD  = (flt64) SD / (flt64)Area;

  flt64 VAR = AvgSSD - xPow2(AvgSD);
  return VAR;
}
flt64V4 xIVPVD::xCalcPicVARM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, uint64 SumWghts)
{
  flt64V4 VARMs = xMakeVec4(0.0);
  for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
  {
    m_ThPI->storeTask([&VARMs, &Tst, &Ref, &Msk, SumWghts, CmpIdx](int32 /**/) { VARMs[CmpIdx] = xCalcCmpVARM(Tst, Ref, Msk, SumWghts, (eCmp)CmpIdx); });
  }
  m_ThPI->executeStoredTasks();
  return VARMs;
}
flt64 xIVPVD::xCalcCmpVARM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, uint64 SumWghts, eCmp CmpId)
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

  int64  SDM  = 0;
  uint64 SSDM = 0;
  std::tie(SDM, SSDM) = xDistortion::CalcWghtSSS(TstPtr, RefPtr, MskPtr, TstStride, RefStride, MskStride, Width, Height);

  flt64 AvgSSDM = (flt64)SSDM / (flt64)SumWghts;
  flt64 AvgSDM  = (flt64) SDM / (flt64)SumWghts;

  flt64 VARM = AvgSSDM - xPow2(AvgSDM);
  return VARM;
}
uint64 xIVPVD::xCalcSumWeights(const xPicP* Msk)
{
  const int32   Width     = Msk->getWidth ();
  const int32   Height    = Msk->getHeight();
  const uint16* MskPtr    = Msk->getAddr  (eCmp::LM);
  const int32   MskStride = Msk->getStride();

  uint64 SumWeights = 0;
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++) { SumWeights += MskPtr[x]; }
    MskPtr += MskStride;
  }
  return SumWeights;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB