/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVQM.h"
#include "xStructSimSTD.h"

#if X_SIMD_CAN_USE_AVX512

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xStructSimAVX512
{
public:
  static flt64 CalcRglrFlt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 /*WndSize*/, flt64 C1, flt64 C2, bool CalcL);
  static flt64 CalcRglrInt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 /*WndSize*/, flt64 C1, flt64 C2, bool CalcL);
  static flt64 CalcRglrAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 /*WndSize*/, flt64 C1, flt64 C2, bool CalcL)
  {
    return CalcBlckAvg11(Tst - (5 * StrideT + 5), Ref - (5 * StrideR + 5), StrideT, StrideR, C1, C2, CalcL);
  }

  static flt64 CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 BlockSize, flt64 C1, flt64 C2, bool CalcL)
  {
    if     (BlockSize == 8 ) { return CalcBlckAvg8 (Tst, Ref, StrideT, StrideR, C1, C2, CalcL); }
    else if(BlockSize == 16) { return CalcBlckAvg16(Tst, Ref, StrideT, StrideR, C1, C2, CalcL); }
    else if(BlockSize == 32) { return CalcBlckAvg32(Tst, Ref, StrideT, StrideR, C1, C2, CalcL); }
    else if(BlockSize == 4 ) { return CalcBlckAvg4 (Tst, Ref, StrideT, StrideR, C1, C2, CalcL); }
    else if(BlockSize == 11) { return CalcBlckAvg11(Tst, Ref, StrideT, StrideR, C1, C2, CalcL); }
    else                     { return xStructSimSTD::CalcBlckAvg(Tst, Ref, StrideT, StrideR, BlockSize, C1, C2, CalcL); }
  }

  static flt64 CalcBlckAvg4 (const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL);
  static flt64 CalcBlckAvg8 (const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL);
  static flt64 CalcBlckAvg11(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL);
  static flt64 CalcBlckAvg16(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL);
  static flt64 CalcBlckAvg32(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL);

  static void CalcMultiBlckAvg8S4(flt64* restrict SSIMs, const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL);
  static void CalcMultiBlckAvg8S8(flt64* restrict SSIMs, const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_AVX
