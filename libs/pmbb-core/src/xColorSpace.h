/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
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

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xColorSpace
{
public:
#if X_CAN_USE_AVX512
  static inline void ConvertRGB2YCbCr(uint16* Y, uint16* U, uint16* V, const uint16* R, const uint16* G, const uint16* B, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
  {
    xColorSpaceAVX512::ConvertRGB2YCbCr_I32(Y, U, V, R, G, B, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
  }
  static inline void ConvertYCbCr2RGB(uint16* R, uint16* G, uint16* B, const uint16* Y, const uint16* U, const uint16* V, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
  {
    xColorSpaceAVX512::ConvertYCbCr2RGB_I32(R, G, B, Y, U, V, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
  }
#elif X_CAN_USE_AVX
  static inline void ConvertRGB2YCbCr(uint16* Y, uint16* U, uint16* V, const uint16* R, const uint16* G, const uint16* B, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
  {
    xColorSpaceAVX::ConvertRGB2YCbCr_I32(Y, U, V, R, G, B, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
  }
  static inline void ConvertYCbCr2RGB(uint16* R, uint16* G, uint16* B, const uint16* Y, const uint16* U, const uint16* V, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
  {
    xColorSpaceAVX::ConvertYCbCr2RGB_I32(R, G, B, Y, U, V, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
  }
#elif X_CAN_USE_SSE
  static inline void ConvertRGB2YCbCr(uint16* Y, uint16* U, uint16* V, const uint16* R, const uint16* G, const uint16* B, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
  {
    xColorSpaceSSE::ConvertRGB2YCbCr_I32(Y, U, V, R, G, B, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
  }
  static inline void ConvertYCbCr2RGB(uint16* R, uint16* G, uint16* B, const uint16* Y, const uint16* U, const uint16* V, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
  {
    xColorSpaceSSE::ConvertYCbCr2RGB_I32(R, G, B, Y, U, V, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
  }
#else
  static inline void ConvertRGB2YCbCr(uint16* Y, uint16* U, uint16* V, const uint16* R, const uint16* G, const uint16* B, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
  {
    xColorSpaceSTD::ConvertRGB2YCbCr_I32(Y, U, V, R, G, B, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
  }
  static inline void ConvertYCbCr2RGB(uint16* R, uint16* G, uint16* B, const uint16* Y, const uint16* U, const uint16* V, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
  {
    xColorSpaceSTD::ConvertYCbCr2RGB_I32(R, G, B, Y, U, V, DstStride, SrcStride, Width, Height, BitDepth, ClrSpc);
  }
#endif
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
