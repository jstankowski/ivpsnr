/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVPSNR.h"
#include "xPic.h"
#include "xThreadPool.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xGlobalColorShift
{
public:
  using tThPI = xThreadPoolInterface;

  //global color shift
  static int32V4 Calc (const xPicP* Ref, const xPicP* Tst,                   const flt32V4& CmpUnntcbCoef,                           tThPI* ThreadPoolIf = nullptr);
  static int32V4 CalcM(const xPicP* Ref, const xPicP* Tst, const xPicP* Msk, const flt32V4& CmpUnntcbCoef, const int32 NumNonMasked, tThPI* ThreadPoolIf = nullptr);


protected:
  static flt64 xCalcAvgColorDiff (const uint16* RefPtr, const uint16* TstPtr,                       const int32 RefStride, const int32 TstStride,                        const int32 Width, const int32 Height);
  static int64 xCalcSumColorDiffM(const uint16* RefPtr, const uint16* TstPtr, const uint16* MskPtr, const int32 RefStride, const int32 TstStride, const int32 MskStride, const int32 Width, const int32 Height);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
