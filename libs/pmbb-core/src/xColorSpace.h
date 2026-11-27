/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2025 Patrycja Kaźmierczak <patrycja.kazmierczak@student.put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefCORE.h"

//portable implementation
#include "xColorSpaceSTD.h"

//SSE implementation
#if X_SIMD_CAN_USE_SSE && __has_include("xColorSpaceSSE.h")
#define X_CAN_USE_SSE 1
#include "xColorSpaceSSE.h"
#else
#define X_CAN_USE_SSE 0
#endif

//AVX implementation
#if X_SIMD_CAN_USE_AVX && __has_include("xColorSpaceAVX.h")
#define X_CAN_USE_AVX 1
#include "xColorSpaceAVX.h"
#else
#define X_CAN_USE_AVX 0
#endif

//AVX512 implementation
#if X_SIMD_CAN_USE_AVX512 && __has_include("xColorSpaceAVX512.h")
#define X_CAN_USE_AVX512 1
#include "xColorSpaceAVX512.h"
#else
#define X_CAN_USE_AVX512 0
#endif

//NEON implementation
#if X_SIMD_CAN_USE_NEON && __has_include("xColorSpaceNEON.h")
#define X_CAN_USE_NEON 1
#include "xColorSpaceNEON.h"
#else
#define X_CAN_USE_NEON 0
#endif

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xColorSpace
{
public:
  static inline void ConvertRGB2YCbCr(uint16* Y, uint16* U, uint16* V, const uint16* R, const uint16* G, const uint16* B, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
  {
#if X_CAN_USE_AVX512
    xColorSpaceAVX512::ConvertRGB2YCbCr_I32(Y, U, V, R, G, B, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
#elif X_CAN_USE_AVX
    xColorSpaceAVX::   ConvertRGB2YCbCr_I32(Y, U, V, R, G, B, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
#elif X_CAN_USE_SSE
    xColorSpaceSSE::   ConvertRGB2YCbCr_I32(Y, U, V, R, G, B, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
#elif X_CAN_USE_NEON
    xColorSpaceNEON::  ConvertRGB2YCbCr_I32(Y, U, V, R, G, B, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
#else
    xColorSpaceSTD::   ConvertRGB2YCbCr_I32(Y, U, V, R, G, B, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
#endif
  }

  static inline void ConvertYCbCr2RGB(uint16* R, uint16* G, uint16* B, const uint16* Y, const uint16* U, const uint16* V, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
  {
#if X_CAN_USE_AVX512
    xColorSpaceAVX512::ConvertYCbCr2RGB_I32(R, G, B, Y, U, V, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
#elif X_CAN_USE_AVX
    xColorSpaceAVX::   ConvertYCbCr2RGB_I32(R, G, B, Y, U, V, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
#elif X_CAN_USE_SSE
    xColorSpaceSSE::   ConvertYCbCr2RGB_I32(R, G, B, Y, U, V, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
#elif X_CAN_USE_NEON
    xColorSpaceNEON::  ConvertYCbCr2RGB_I32(R, G, B, Y, U, V, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
#else
    xColorSpaceSTD::   ConvertYCbCr2RGB_I32(R, G, B, Y, U, V, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
#endif
  }
};

//===============================================================================================================================================================================================================

#undef X_CAN_USE_SSE
#undef X_CAN_USE_AVX
#undef X_CAN_USE_AVX512
#undef X_CAN_USE_NEON

} //end of namespace PMBB
