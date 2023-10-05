/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVPSNR.h"
#include "xPic.h"

#if X_SIMD_CAN_USE_SSE

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xCorrespPixelShiftSSE
{
public:
  static constexpr bool c_UseRuntimeCmpWeights = xc_USE_RUNTIME_CMPWEIGHTS;

  static int32V4 CalcDistAsymmetricRow(const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);

protected:
  static __m128i xCalcDistWithinBlock  (const xPicI* Ref, const __m128i& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const __m128i& CmpWeights);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_SSE
