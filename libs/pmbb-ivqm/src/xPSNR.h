/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefIVQM.h"
#include "xMetricCommon.h"
#include "xPic.h"
#include "xVec.h"

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

  uint64V4 calcPicSSD   (const xPicP* Tst, const xPicP* Ref);
  flt64V4  calcPicMSE   (const xPicP* Tst, const xPicP* Ref);
  flt64V4  calcPicPSNR  (const xPicP* Tst, const xPicP* Ref);

  uint64V4 calcPicSSDM  (const xPicP* Tst, const xPicP* Ref, const xPicP* Msk);
  flt64V4  calcPicMSEM  (const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, int32 NumNonMasked = NOT_VALID);
  flt64V4  calcPicPSNRM (const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, int32 NumNonMasked = NOT_VALID);

protected:
  uint64V4      xCalcPicSSD  (const xPicP* Tst, const xPicP* Ref);
  uint64V4      xCalcPicSSDM (const xPicP* Tst, const xPicP* Ref, const xPicP* Msk);
  static uint64 xCalcCmpSSD  (const xPicP* Tst, const xPicP* Ref,                   eCmp CmpId);
  static uint64 xCalcCmpSSDM (const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, eCmp CmpId);

public:
  flt64V4 CalcMSEsFromSSDs       (flt64V4 SSDs , int64 Area                                                          );
  flt64V4 CalcPSNRsFromSSDs      (flt64V4 SSDs , int64 Area,                     int32 BitDepthPic                   );
  flt64V4 CalcMSEsFromMaskedSSDs (flt64V4 SSDMs, int64 Area, int64 NumNonMasked,                    int32 BitDepthMsk);
  flt64V4 CalcPSNRsFromMaskedSSDs(flt64V4 SSDMs, int64 Area, int64 NumNonMasked, int32 BitDepthPic, int32 BitDepthMsk);

  static flt64 CalcMSEfromSSD       (flt64 SSD , int64 Area                                              );
  static flt64 CalcPSNRfromSSD      (flt64 SSD , int64 Area,         int32 BitDepthPic                   );
  static flt64 CalcMSEfromMaskedSSD (flt64 SSDM, int64 NumNonMasked,                    int32 BitDepthMsk);
  static flt64 CalcPSNRfromMaskedSSD(flt64 SSDM, int64 NumNonMasked, int32 BitDepthPic, int32 BitDepthMsk);

  static flt64 getFakePSNR(int32 Area, int32 BitDepth) { return CalcPSNRfromSSD(1, Area, BitDepth); }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB