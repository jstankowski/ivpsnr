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
  static constexpr flt32V4 c_DefaultUnntcbCoef = { 0.01f, 0.01f, 0.01f, 0.0f };

protected:
  flt32V4 m_CmpUnntcbCoef = c_DefaultUnntcbCoef;

public:
  void  setUnntcbCoef(const flt32V4& UnntcbCoef) { m_CmpUnntcbCoef = UnntcbCoef; }

protected:
  static int32V4 CalcGlobalColorShift (const xPicP* Ref, const xPicP* Tst,                   const flt32V4& CmpUnntcbCoef,                           tThPI* ThreadPoolIf = nullptr);
  static int32V4 CalcGlobalColorShiftM(const xPicP* Ref, const xPicP* Tst, const xPicP* Msk, const flt32V4& CmpUnntcbCoef, const int32 NumNonMasked, tThPI* ThreadPoolIf = nullptr);

  static flt64 xCalcAvgColorDiff (const uint16* RefPtr, const uint16* TstPtr,                       const int32 RefStride, const int32 TstStride,                        const int32 Width, const int32 Height);
  static int64 xCalcSumColorDiffM(const uint16* RefPtr, const uint16* TstPtr, const uint16* MskPtr, const int32 RefStride, const int32 TstStride, const int32 MskStride, const int32 Width, const int32 Height);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
