/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefIVPSNR.h"
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
  //results
  using tRes4 = std::tuple<flt64V4, boolV4>;
  using tRes1 = std::tuple<flt64  , bool  >;
  //debug calback types
  using tDCfMSK = std::function<void(int32)>; //debug calback - mask

protected:
  tDCfMSK m_DebugCallbackMSK;

public:
  void  setDebugCallbackMSK(tDCfMSK DebugCallbackMSK) { m_DebugCallbackMSK = DebugCallbackMSK; }  

  tRes4 calcPicPSNR    (const xPicP* Tst, const xPicP* Ref);
  tRes4 calcPicPSNRM   (const xPicP* Tst, const xPicP* Ref, const xPicP* Msk);

protected:
  tRes1 xCalcCmpPSNR (const xPicP* Tst, const xPicP* Ref,                                             eCmp CmpId);
  tRes1 xCalcCmpPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32 NumNonMasked, eCmp CmpId);

public:
  static flt64 CalcPSNRfromSSD      (flt64 SSD, int32 Area, int32 BitDepth);
  static flt64 CalcPSNRfromMaskedSSD(flt64 SSD, int32 NumNonMasked, int32 BitDepthPic, int32 BitDepthMsk);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB