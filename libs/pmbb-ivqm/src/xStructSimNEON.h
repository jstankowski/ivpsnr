/*
    SPDX-FileCopyrightText: 2025 Patrycja Kaźmierczak <patrycja.kazmierczak@student.put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefIVQM.h"

#if X_SIMD_CAN_USE_NEON

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xStructSimNEON
{
public:
  static flt64 CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 BlockSize, flt64 C1, flt64 C2, bool CalcL);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_NEON
