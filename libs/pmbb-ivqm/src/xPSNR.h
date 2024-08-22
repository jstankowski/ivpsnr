/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefIVQM.h"
#include "xMetricCommon.h"
#include "xPic.h"
#include "xVec.h"
#include <vector>
#include <tuple>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xPSNR : public xMetricCommon
{
public:
  //debug calback types
  using tDCfMSK = std::function<void(int32)>; //debug calback - mask

protected:
  tDCfMSK m_DebugCallbackMSK;
  bool    m_FakeValsForExact = false; //for exact components - emmit fake values

public:
  void  setDebugCallbackMSK(tDCfMSK DebugCallbackMSK) { m_DebugCallbackMSK = DebugCallbackMSK; } 
  void  setFakeValsForExact(bool FVFE) { m_FakeValsForExact = FVFE; }

  flt64V4 calcPicPSNR  (const xPicP* Tst, const xPicP* Ref);
  flt64V4 calcPicPSNRM (const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, int32 NumNonMasked = NOT_VALID);

protected:
  flt64         xCalcCmpPSNR (const xPicP* Tst, const xPicP* Ref,                                             eCmp CmpId);
  flt64         xCalcCmpPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32 NumNonMasked, eCmp CmpId);
  static uint64 xCalcCmpSSD  (const xPicP* Tst, const xPicP* Ref,                                             eCmp CmpId);
  static uint64 xCalcCmpSSDM (const xPicP* Tst, const xPicP* Ref, const xPicP* Msk,                           eCmp CmpId);

public:
  static flt64 CalcPSNRfromSSD      (flt64 SSD, int32 Area, int32 BitDepth);
  static flt64 CalcPSNRfromMaskedSSD(flt64 SSD, int32 NumNonMasked, int32 BitDepthPic, int32 BitDepthMsk);

  static flt64 getFakePSNR(int32 Area, int32 BitDepth) { return CalcPSNRfromSSD(1, Area, BitDepth); }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB