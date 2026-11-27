/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefCORE.h"
#include "xDistortionSTD.h"

#if X_SIMD_CAN_USE_SSE

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xDistortionSSE
{
public:
  //SD, SAD, SSD
  static  int64 CalcSD14 (const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height);
  static  int64 CalcSD16 (const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height);
  static uint64 CalcSAD16(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height);
  static uint64 CalcSSD14(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height);
  static tSDSSD CalcSSS14(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height);

  static inline  int64 CalcSD (const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return BitDepth <= 14 ? CalcSD14 (Tst, Ref, TstStride, RefStride, Width, Height) : CalcSD16(Tst, Ref, TstStride, RefStride, Width, Height); }
  static inline uint64 CalcSAD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32         ) { return CalcSAD16(Tst, Ref, TstStride, RefStride, Width, Height); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return BitDepth <= 14 ? CalcSSD14(Tst, Ref, TstStride, RefStride, Width, Height) : xDistortionSTD::CalcSSD16(Tst, Ref, TstStride, RefStride, Width, Height); }
  static inline tSDSSD CalcSSS(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return BitDepth <= 14 ? CalcSSS14(Tst, Ref, TstStride, RefStride, Width, Height) : xDistortionSTD::CalcSSS16(Tst, Ref, TstStride, RefStride, Width, Height); }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_SSE
