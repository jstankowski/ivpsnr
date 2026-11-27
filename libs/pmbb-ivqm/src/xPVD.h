/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVQM.h"
#include "xMetricCommon.h"
#include "xPic.h"
#include "xVec.h"
#include "xGlobClrDiff.h"
#include "xCorrespPixelShiftPrms.h"
#include "xCorrespPixelShift.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xIVPVD : public xMetricCommon, public xGlobClrDiffPrms, public xCorrespPixelShiftPrms
{
public:
  using tDCfQAP = std::function<void(flt64, flt64)>;   //QAP = QualAsymmetricPic
protected:
  tDCfQAP m_DebugCallbackQAP;
public:
  void  setDebugCallbackQAP(tDCfQAP DebugCallbackQAP) { m_DebugCallbackQAP = DebugCallbackQAP; }

public:
  flt64 calcPicPVD  (const xPicP* Tst, const xPicP* Ref);
  flt64 calcPicIVPVD(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP);

  flt64 calcPicPVDM  (const xPicP* Tst, const xPicP* Ref                                          , const xPicP* Msk);
  flt64 calcPicIVPVDM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP, const xPicP* Msk);

protected:
  flt64V4      xCalcPicVAR(const xPicP* Tst, const xPicP* Ref);
  static flt64 xCalcCmpVAR(const xPicP* Tst, const xPicP* Ref, eCmp CmpId);

  flt64V4      xCalcPicVARM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, uint64 SumWghts);
  static flt64 xCalcCmpVARM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, uint64 SumWghts, eCmp CmpId);

  static uint64 xCalcSumWeights(const xPicP* Msk);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB