/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefIVQM.h"
#include "xCorrespPixelShiftCmn.h"

#if X_SIMD_CAN_USE_SSE

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xCorrespPixelShiftSSE : public xCorrespPixelShiftCmn
{
public:
  static void GenShftCompRowI16(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
  static void GenShftCompRowU32(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_SSE
