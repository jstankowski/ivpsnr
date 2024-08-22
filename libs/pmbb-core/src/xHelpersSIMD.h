/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefCORE.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// SIMD "view" types for debuging under GCC or CLANG
//===============================================================================================================================================================================================================
#if (defined(__clang__) || defined(__GNUC__)) && !defined(NDEBUG)

#if X_SIMD_CAN_USE_SSE
using v128I64 = __v2di ;
using v128U64 = __v2du ;
using v128I32 = __v4si ;
using v128U32 = __v4su ;
using v128I16 = __v8hi ;
using v128U16 = __v8hu ;
using v128I8  = __v16qi;
using v128U8  = __v16qu;
#endif

#if X_SIMD_CAN_USE_AVX
using v256I64 = __v4di ;
using v256U64 = __v4du ;
using v256I32 = __v8si ;
using v256U32 = __v8su ;
using v256I16 = __v16hi;
using v256U16 = __v16hu;
using v256I8  = __v32qi;
using v256U8  = __v32qu;
#endif

#if X_SIMD_CAN_USE_AVX512
using v512I64 = __v8di ;
using v512U64 = __v8du ;
using v512I32 = __v16si;
using v512U32 = __v16su;
using v512I16 = __v32hi;
using v512U16 = __v32hu;
using v512I8  = __v64qi;
using v512U8  = __v64qu;
#endif

#endif //(defined(__clang__) || defined(__GNUC__)) && !defined(NDEBUG)

//===============================================================================================================================================================================================================
// _mm512_setr_epi16 is missing in GCC
//===============================================================================================================================================================================================================
#if defined(__GNUC__) && X_SIMD_CAN_USE_AVX512
static inline __m512i _mm512_setr_epi16(short  e0, short  e1, short  e2, short  e3, short  e4, short  e5, short  e6, short e7,
                                        short  e8, short  e9, short e10, short e11, short e12, short e13, short e14, short e15,
                                        short e16, short e17, short e18, short e19, short e20, short e21, short e22, short e23,
                                        short e24, short e25, short e26, short e27, short e28, short e29, short e30, short e31)
{
  return _mm512_set_epi16(e31,  e30,  e29,  e28,  e27,  e26,  e25,  e24,
                          e23,  e22,  e21,  e20,  e19,  e18,  e17,  e16,
                          e15,  e14,  e13,  e12,  e11,  e10,   e9,   e8,
                           e7,   e6,   e5,   e4,   e3,   e2,   e1,   e0);
}

static inline __m512i _mm512_setr_epi32(short  e0, short  e1, short  e2, short  e3, short  e4, short  e5, short  e6, short e7,
                                        short  e8, short  e9, short e10, short e11, short e12, short e13, short e14, short e15)
{
  return _mm512_set_epi32(e15,  e14,  e13,  e12,  e11,  e10,   e9,   e8,
                           e7,   e6,   e5,   e4,   e3,   e2,   e1,   e0);
}

#endif

//===============================================================================================================================================================================================================
// SIMD horizontal pairwise sum
//===============================================================================================================================================================================================================
#if X_SIMD_CAN_USE_AVX512
inline __m512i _mm512_hadd_epi32(__m512i a, __m512i b)
{
  const __m512i SelU32T = _mm512_setr_epi32(0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30);
  const __m512i SelU32B = _mm512_setr_epi32(1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31);
  __m512i T = _mm512_permutex2var_epi32(a, SelU32T, b);
  __m512i B = _mm512_permutex2var_epi32(a, SelU32B, b);
  __m512i R = _mm512_add_epi32(T, B);
  return R;
}
#endif //X_SIMD_CAN_USE_AVX512

//===============================================================================================================================================================================================================
// SIMD horizontal sum reduction
//===============================================================================================================================================================================================================
#if X_SIMD_CAN_USE_SSE
static inline int32 xHorVecSum_epi32(__m128i v)
{
  __m128i hi64  = _mm_unpackhi_epi64(v, v);
  __m128i sum64 = _mm_add_epi32(hi64, v);
  __m128i hi32  = _mm_shuffle_epi32(sum64, _MM_SHUFFLE(2, 3, 0, 1));    // Swap the low two elements
  __m128i sum32 = _mm_add_epi32(sum64, hi32);
  return _mm_cvtsi128_si32(sum32);       // movd
  //alternative, probably slower
  //v = _mm_hadd_epi32(v, v);
  //v = _mm_hadd_epi32(v, v);
  //return _mm_extract_epi32(v, 0);
}
static inline int64 xHorVecSum_epi64(__m128i v)
{
  __m128i hi64  = _mm_unpackhi_epi64(v, v);
  __m128i sum64 = _mm_add_epi64(hi64, v);
  return _mm_cvtsi128_si64(sum64);
  //alternative, probably slower
  //return (_mm_extract_epi64(v, 0) + _mm_extract_epi64(v, 1));
}
#endif //X_SIMD_CAN_USE_SSE

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if X_SIMD_CAN_USE_AVX
static inline int32 xHorVecSum_epi32(__m256i v)
{
  __m128i Sum_I32_V128 = _mm_add_epi32(_mm256_castsi256_si128(v), _mm256_extracti128_si256(v, 1));
  return xHorVecSum_epi32(Sum_I32_V128);
}
static inline int64 xHorVecSum_epi64(__m256i v)
{
  __m128i Sum_I64_V128 = _mm_add_epi64(_mm256_castsi256_si128(v), _mm256_extracti128_si256(v, 1));
  return xHorVecSum_epi64(Sum_I64_V128);
}
#endif //X_SIMD_CAN_USE_AVX

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if X_SIMD_CAN_USE_AVX512
static inline int32 xHorVecSum_epi32(__m512i v)
{
  __m256i Sum_I32_V256 = _mm256_add_epi32(_mm512_castsi512_si256(v),_mm512_extracti64x4_epi64(v, 1));
  return xHorVecSum_epi32(Sum_I32_V256);
}
static inline int64 xHorVecSum_epi64(__m512i v)
{
  __m256i Sum_I64_V256 = _mm256_add_epi64(_mm512_castsi512_si256(v), _mm512_extracti64x4_epi64(v, 1));
  return xHorVecSum_epi64(Sum_I64_V256);
}
#endif //X_SIMD_CAN_USE_AVX512

//===============================================================================================================================================================================================================
// SIMD horizontal min/max
//===============================================================================================================================================================================================================
#if X_SIMD_CAN_USE_SSE
static inline uint16 _mm_hmin_epu16(__m128i Src_U16_V) //horizontal min
{
  uint16 Min = (uint16)(_mm_cvtsi128_si32(_mm_minpos_epu16(Src_U16_V)) & 0xFFFF);
  return Min;
}
static inline uint16 _mm_hmax_epu16(__m128i Src_U16_V) //horizontal max
{
  __m128i NegMax_U16_V = _mm_andnot_si128(Src_U16_V, _mm_set1_epi32(0xFFFFFFFF));
  uint16  NegMax = (uint16)(_mm_cvtsi128_si32(_mm_minpos_epu16(NegMax_U16_V)) & 0xFFFF);
  uint16  Max = ~NegMax;
  return Max;
}
static inline uint32 _mm_hmax_epu32(__m128i Src_U32_V) //horizontal max
{
  //__m128i A = _mm_shuffle_epi32(Src_U32_V, _MM_SHUFFLE(2, 3, 2, 3));

  __m128i B = _mm_max_epu32(Src_U32_V, _mm_shuffle_epi32(Src_U32_V, _MM_SHUFFLE(3, 2, 3, 2)));
  __m128i C = _mm_max_epu32(B, _mm_shuffle_epi32(B, _MM_SHUFFLE(1, 1, 1, 1)));

  uint32 Res = _mm_cvtsi128_si32(C);

  return Res;
}
#endif //X_SIMD_CAN_USE_SSE

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if X_SIMD_CAN_USE_AVX
static inline __m128i _mm256_lmin_epu16(__m256i Src_U16_V) //cross-lane min
{
  __m128i Min_U16_V = _mm_min_epu16(_mm256_castsi256_si128(Src_U16_V), _mm256_extracti128_si256(Src_U16_V, 1));
  return Min_U16_V;
}
static inline __m128i _mm256_lmax_epu16(__m256i Src_U16_V) //cross-lane max
{
  __m128i Max_U16_V = _mm_max_epu16(_mm256_castsi256_si128(Src_U16_V), _mm256_extracti128_si256(Src_U16_V, 1));
  return Max_U16_V;
}
static inline __m128i _mm256_lmax_epu32(__m256i Src_U32_V) //cross-lane max
{
  __m128i Max_U16_V = _mm_max_epu32(_mm256_castsi256_si128(Src_U32_V), _mm256_extracti128_si256(Src_U32_V, 1));
  return Max_U16_V;
}
static inline uint16 _mm256_hmin_epu16(__m256i Src_U16_V) //horizontal min
{
  __m128i Min_U16_V = _mm256_lmin_epu16(Src_U16_V);
  uint16 Min = _mm_hmin_epu16(Min_U16_V);
  return Min;
}
static inline uint16 _mm256_hmax_epu16(__m256i Src_U16_V) //horizontal max
{
  __m128i Max_U16_V = _mm256_lmax_epu16(Src_U16_V);
  uint16 Max = _mm_hmax_epu16(Max_U16_V);
  return Max;
}
static inline uint32 _mm256_hmax_epu32(__m256i Src_U32_V) //horizontal max
{
  __m128i Max_U32_V = _mm256_lmax_epu32(Src_U32_V);
  uint16 Max = _mm_hmax_epu32(Max_U32_V);
  return Max;
}
#endif //X_SIMD_CAN_USE_AVX

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if X_SIMD_CAN_USE_AVX512

static inline uint16 _mm512_hmin_epu16(__m512i Src_U16_V) //horizontal min
{
  __m256i Min_U16_V = _mm256_min_epu16(_mm512_castsi512_si256(Src_U16_V), _mm512_extracti64x4_epi64(Src_U16_V, 1));
  uint16 Min = _mm256_hmin_epu16(Min_U16_V);
  return Min;
}
static inline uint16 _mm512_hmax_epu16(__m512i Src_U16_V) //horizontal max
{
  __m256i Max_U16_V = _mm256_max_epu16(_mm512_castsi512_si256(Src_U16_V), _mm512_extracti64x4_epi64(Src_U16_V, 1));
  uint16 Max = _mm256_hmax_epu16(Max_U16_V);
  return Max;
}
static inline uint16 _mm512_hmax_epu32(__m512i Src_U32_V) //horizontal max
{
  __m256i Max_U32_V = _mm256_max_epu32(_mm512_castsi512_si256(Src_U32_V), _mm512_extracti64x4_epi64(Src_U32_V, 1));
  uint16 Max = _mm256_hmax_epu32(Max_U32_V);
  return Max;
}

#endif //X_SIMD_CAN_USE_AVX512

//===============================================================================================================================================================================================================

} //end of namespace PMBB
