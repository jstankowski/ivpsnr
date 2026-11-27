/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2025 Patrycja Kaźmierczak <patrycja.kazmierczak@student.put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefCORE.h"
#include "xVec.h"

//portable implementation
#include "xDistortionSTD.h"

//SSE implementation
#if X_SIMD_CAN_USE_SSE && __has_include("xDistortionSSE.h")
#define X_CAN_USE_SSE 1
#include "xDistortionSSE.h"
#else
#define X_CAN_USE_SSE 0
#endif

//AVX implementation
#if X_SIMD_CAN_USE_AVX && __has_include("xDistortionAVX.h")
#define X_CAN_USE_AVX 1
#include "xDistortionAVX.h"
#else
#define X_CAN_USE_AVX 0
#endif

//AVX512 implementation
#if X_SIMD_CAN_USE_AVX512 && __has_include("xDistortionAVX512.h")
#define X_CAN_USE_AVX512 1
#include "xDistortionAVX512.h"
#else
#define X_CAN_USE_AVX512 0
#endif

//NEON implementation
#if X_SIMD_CAN_USE_NEON && __has_include("xDistortionNEON.h")
#define X_CAN_USE_NEON 1
#include "xDistortionNEON.h"
#else
#define X_CAN_USE_NEON 0
#endif

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xDistortion
{
public:
#if   X_CAN_USE_AVX512

  static inline  int64 CalcSD (const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionAVX512::CalcSD (Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }
  static inline uint64 CalcSAD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionAVX512::CalcSAD(Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionAVX512::CalcSSD(Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }
  static inline tSDSSD CalcSSS(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionAVX512::CalcSSS(Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }

#elif X_CAN_USE_AVX

  static inline  int64 CalcSD (const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionAVX::CalcSD (Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }
  static inline uint64 CalcSAD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionAVX::CalcSAD(Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionAVX::CalcSSD(Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }
  static inline tSDSSD CalcSSS(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionAVX::CalcSSS(Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }

#elif X_CAN_USE_SSE

  static inline  int64 CalcSD (const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionSSE::CalcSD (Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }
  static inline uint64 CalcSAD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionSSE::CalcSAD(Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionSSE::CalcSSD(Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }
  static inline tSDSSD CalcSSS(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionSSE::CalcSSS(Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }

#elif X_CAN_USE_NEON

  static inline  int32 CalcSD (const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionNEON::CalcSD (Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }
  static inline uint32 CalcSAD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionNEON::CalcSAD(Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionNEON::CalcSSD(Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }
  static inline tSDSSD CalcSSS(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionSTD ::CalcSSS(Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }

#else

  static inline  int64 CalcSD (const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionSTD::CalcSD (Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }
  static inline uint64 CalcSAD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionSTD::CalcSAD(Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionSTD::CalcSSD(Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }
  static inline tSDSSD CalcSSS(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 BitDepth) { return xDistortionSTD::CalcSSS(Tst, Ref, TstStride, RefStride, Width, Height, BitDepth); }

#endif

  static inline  int64 CalcSD (const uint16* Tst, const uint16* Ref, int32 Area, int32 BitDepth) { return CalcSD (Tst, Ref, NOT_VALID, NOT_VALID, Area, 1, BitDepth); }
  static inline uint64 CalcSAD(const uint16* Tst, const uint16* Ref, int32 Area, int32 BitDepth) { return CalcSAD(Tst, Ref, NOT_VALID, NOT_VALID, Area, 1, BitDepth); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref, int32 Area, int32 BitDepth) { return CalcSSD(Tst, Ref, NOT_VALID, NOT_VALID, Area, 1, BitDepth); }

  static inline  int64 CalcWghtSD (const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height) { return xDistortionSTD::CalcWghtSD (Tst, Ref, Msk, TstStride, RefStride, MskStride, Width, Height); }
  static inline uint64 CalcWghtSSD(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height) { return xDistortionSTD::CalcWghtSSD(Tst, Ref, Msk, TstStride, RefStride, MskStride, Width, Height); }
  static inline tSDSSD CalcWghtSSS(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height) { return xDistortionSTD::CalcWghtSSS(Tst, Ref, Msk, TstStride, RefStride, MskStride, Width, Height); }

  static inline  int64 CalcWghtSD (const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 Area) { return CalcWghtSD (Tst, Ref, Msk, NOT_VALID, NOT_VALID, NOT_VALID, Area, 1); }
  static inline uint64 CalcWghtSSD(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 Area) { return CalcWghtSSD(Tst, Ref, Msk, NOT_VALID, NOT_VALID, NOT_VALID, Area, 1); }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#undef X_CAN_USE_SSE
#undef X_CAN_USE_AVX
#undef X_CAN_USE_AVX512
#undef X_CAN_USE_NEON