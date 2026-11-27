/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefCORE.h"

//SSE implementation
#if X_SIMD_CAN_USE_SSE
#define X_CAN_USE_SSE 1
#else
#define X_CAN_USE_SSE 0
#endif

//AVX implementation
#if X_SIMD_CAN_USE_AVX
#define X_CAN_USE_AVX 1
#else
#define X_CAN_USE_AVX 0
#endif

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

/*
ROWS = Y = H
COLS = X = W
M[Y][X]
*/

template<uint32 R, uint32 C> class xMatrixUtilsNxM
{
public:
  static constexpr int32 c_Area = R * C;

protected:
  static void xConvert_STD(flt32* restrict Dst, const flt64* restrict Src);
  static void xConvert_SSE(flt32* restrict Dst, const flt64* restrict Src);
  static void xConvert_AVX(flt32* restrict Dst, const flt64* restrict Src);
  static void xConvert_STD(flt64* restrict Dst, const flt32* restrict Src);
  static void xConvert_SSE(flt64* restrict Dst, const flt32* restrict Src);
  static void xConvert_AVX(flt64* restrict Dst, const flt32* restrict Src);

public:
#if X_CAN_USE_AVX
  static inline void Convert(flt32* Dst, const flt64* Src) { xConvert_AVX(Dst, Src); }
  static inline void Convert(flt64* Dst, const flt32* Src) { xConvert_AVX(Dst, Src); }
#elif X_CAN_USE_SSE
  static inline void Convert(flt32* Dst, const flt64* Src) { xConvert_SSE(Dst, Src); }
  static inline void Convert(flt64* Dst, const flt32* Src) { xConvert_SSE(Dst, Src); }
#else
  static inline void Convert(flt32* Dst, const flt64* Src) { xConvert_STD(Dst, Src); }
  static inline void Convert(flt64* Dst, const flt32* Src) { xConvert_STD(Dst, Src); }
#endif
};

//===============================================================================================================================================================================================================

class xMatrixUtils4x4
{
protected:
  union xM4x4F
  {
    flt32  M [4][4];
#if X_CAN_USE_SSE
    __m128 R [4];
#endif //X_CAN_USE_SSE
#if X_CAN_USE_AVX
    __m256 DR[2];
#endif //X_CAN_USE_AVX
  };

  union xM4x4D
  {
    flt64   M [4][4];
#if X_CAN_USE_SSE
    __m128d R [8];
#endif //X_CAN_USE_SSE
#if X_CAN_USE_AVX
    __m256d DR[4];
#endif //X_CAN_USE_AVX
  };

  static void xMatrixMultiply4x4_STD(xM4x4F* restrict Dst, const xM4x4F* restrict A, const xM4x4F* restrict B);
  static void xMatrixMultiply4x4_SSE(xM4x4F* restrict Dst, const xM4x4F* restrict A, const xM4x4F* restrict B);
  static void xMatrixMultiply4x4_AVX(xM4x4F* restrict Dst, const xM4x4F* restrict A, const xM4x4F* restrict B);

  static void xMatrixConvert4x4_STD (xM4x4F* restrict Dst, const xM4x4D* restrict Src);
  static void xMatrixConvert4x4_SSE (xM4x4F* restrict Dst, const xM4x4D* restrict Src);
  static void xMatrixConvert4x4_AVX (xM4x4F* restrict Dst, const xM4x4D* restrict Src);

  static void xMatrixConvert4x4_STD (xM4x4D* restrict Dst, const xM4x4F* restrict Src);
  static void xMatrixConvert4x4_SSE (xM4x4D* restrict Dst, const xM4x4F* restrict Src);
  static void xMatrixConvert4x4_AVX (xM4x4D* restrict Dst, const xM4x4F* restrict Src);

public:
#if X_USE_AVX && X_AVX1
  static inline void MatrixMultiply4x4(flt32* Dst, const flt32* A, const flt32* B) { xMatrixMultiply4x4_AVX((xM4x4F*)Dst, (xM4x4F*)A, (xM4x4F*)B); }
  static inline void MatrixConvert4x4 (flt32* Dst, const flt64* Src              ) { xMatrixConvert4x4_AVX((xM4x4F*)Dst, (xM4x4D*)Src); }
  static inline void MatrixConvert4x4 (flt64* Dst, const flt32* Src              ) { xMatrixConvert4x4_AVX((xM4x4D*)Dst, (xM4x4F*)Src); }
#elif X_USE_SSE && X_SSE_ALL
  static inline void MatrixMultiply4x4(flt32* Dst, const flt32* A, const flt32* B) { xMatrixMultiply4x4_SSE((xM4x4F*)Dst, (xM4x4F*)A, (xM4x4F*)B); }
  static inline void MatrixConvert4x4 (flt32* Dst, const flt64* Src              ) { xMatrixConvert4x4_SSE ((xM4x4F*)Dst, (xM4x4D*)Src); }
  static inline void MatrixConvert4x4 (flt64* Dst, const flt32* Src              ) { xMatrixConvert4x4_SSE ((xM4x4D*)Dst, (xM4x4F*)Src); }
#else
  static inline void MatrixMultiply4x4(flt32* Dst, const flt32* A, const flt32* B) { xMatrixMultiply4x4_STD((xM4x4F*)Dst, (xM4x4F*)A, (xM4x4F*)B); }
  static inline void MatrixConvert4x4 (flt32* Dst, const flt64* Src              ) { xMatrixConvert4x4_STD ((xM4x4F*)Dst, (xM4x4D*)Src); }
  static inline void MatrixConvert4x4 (flt64* Dst, const flt32* Src              ) { xMatrixConvert4x4_STD ((xM4x4D*)Dst, (xM4x4F*)Src); }
#endif
};

//===============================================================================================================================================================================================================

#ifndef PMBB_xMatrixUtils_IMPLEMENTATION
#undef X_CAN_USE_SSE
#undef X_CAN_USE_AVX
#endif //PMBB_xMatrixUtils_IMPLEMENTATION

} //end of namespace PMBB

