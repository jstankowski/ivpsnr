/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefPMBB-BASE.h"

//===============================================================================================================================================================================================================
// base includes
//===============================================================================================================================================================================================================
#if __has_include(<sys/mman.h>)
#include <sys/mman.h>
#endif

//===============================================================================================================================================================================================================
// Namespace
//===============================================================================================================================================================================================================
#ifndef PMBB_NAMESPACE
#define PMBB_NAMESPACE PMBB
#endif

namespace PMBB_NAMESPACE { using namespace PMBB_BASE; }

//===============================================================================================================================================================================================================
// SIMD section - common
//===============================================================================================================================================================================================================
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_AMD64) || defined(_M_IX86))
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif

#define USE_SIMD  1 // use SIMD

namespace PMBB_NAMESPACE {

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// SIMD section - all SSE (corresponding to x86-64-v2)
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//MSVC does not define __SSEn__ macros. Assuming all extensions present.
#if defined(_MSC_VER)
#define __SSE__    1
#define __SSE2__   1
#define __SSE3__   1
#define __SSSE3__  1
#define __SSE4_1__ 1
#define __SSE4_2__ 1
#endif

//SSE    - since Pentium III
//SSE2   - since Pentium 4 (Willamette, Northwood, Gallatin)
//SSE3   - since Pentium 4 (Prescott, Cedar Mill)
//SSSE3  - since Core 2 (Conroe, Merom)
//SSE4_1 - since Core 2 (Penryn, Wolfdale)
//SSE4_1 - since Core iX nnn (Nehalem, Westmere)
#if defined(__SSE__) && defined(__SSE2__) && defined(__SSE3__) && defined(__SSSE3__) && defined(__SSE4_1__) && defined(__SSE4_2__)
#define X_SIMD_HAS_SSE (__SSE__ && __SSE2__ && __SSE3__ && __SSSE3__ && __SSE4_1__ && __SSE4_2__)
#else
#define X_SIMD_HAS_SSE 0
#endif
#define X_SIMD_CAN_USE_SSE (X_SIMD_HAS_SSE && USE_SIMD)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// SIMD section - AVX & AVX2 (corresponding to x86-64-v3)
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//AVX  - since Core iX 2nnn (Sandy Bridge)
//AVX2 - since Core iX 4nnn (Haswell), Ryzen 1xxx (Zen1)
#if defined(__AVX__) && defined(__AVX2__)
#define X_SIMD_HAS_AVX (__AVX__ && __AVX2__)
#else
#define X_SIMD_HAS_AVX 0
#endif
#define X_SIMD_CAN_USE_AVX (X_SIMD_HAS_AVX && USE_SIMD)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// SIMD section - AVX512 (corresponding to x86-64-v4)
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//AVX512 - what a mess
#if defined(__AVX512F__) && defined(__AVX512BW__) && defined(__AVX512CD__) && defined(__AVX512DQ__) && defined(__AVX512VL__)
#define X_SIMD_HAS_AVX512 (__AVX512F__ && __AVX512BW__ && __AVX512CD__ && __AVX512DQ__ && __AVX512VL__)
#else
#define X_SIMD_HAS_AVX512 0
#endif
#define X_SIMD_CAN_USE_AVX512 (X_SIMD_HAS_AVX512 && USE_SIMD)

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
// Basic ops
//===============================================================================================================================================================================================================

template <class XXX> static inline XXX xMin     (XXX a, XXX b              ) { return std::min(a, b);            }
template <class XXX> static inline XXX xMax     (XXX a, XXX b              ) { return std::max(a, b);            }
template <class XXX> static inline XXX xMin     (XXX a, XXX b, XXX c       ) { return xMin(xMin(a,b),c);         }
template <class XXX> static inline XXX xMax     (XXX a, XXX b, XXX c       ) { return xMax(xMax(a,b),c);         }
template <class XXX> static inline XXX xMin     (XXX a, XXX b, XXX c, XXX d) { return xMin(xMin(a,b),xMin(c,d)); }
template <class XXX> static inline XXX xMax     (XXX a, XXX b, XXX c, XXX d) { return xMax(xMax(a,b),xMax(c,d)); }

template <class XXX> static inline XXX xClip    (XXX x, XXX min, XXX max) { return xMax(min, xMin(x, max)); }
template <class XXX> static inline XXX xClipU   (XXX x, XXX max         ) { return xMax((XXX)0, xMin(x,max));}
template <class XXX> static inline XXX xClipU8  (XXX x                  ) { return xMax((XXX)0,xMin(x,(XXX)255));}
template <class XXX> static inline XXX xClipS8  (XXX x                  ) { return xMax((XXX)-128,xMin(x,(XXX)127));}
template <class XXX> static inline XXX xClipU16 (XXX x                  ) { return xMax((XXX)0,xMin(x,(XXX)65536));}
template <class XXX> static inline XXX xClipS16 (XXX x                  ) { return xMax((XXX)-32768,xMin(x,(XXX)32767));}

template <class XXX> static inline XXX xAbs     (XXX a) { return (XXX)std::abs(a); }
template <class XXX> static inline XXX xPow2    (XXX x) { return x * x; }

template <class XXX> static inline XXX xBitDepth2MidValue(XXX BitDepth) { return (1 << (BitDepth - 1)); }
template <class XXX> static inline XXX xBitDepth2MaxValue(XXX BitDepth) { return ((1 << BitDepth) - 1); }
template <class XXX> static inline XXX xBitDepth2NumLevel(XXX BitDepth) { return (1 << BitDepth); }
template <class XXX> static inline XXX xBitDepth2BitMask (XXX BitDepth) { return xBitDepth2MaxValue(BitDepth); }

template <class XXX> static inline XXX xLog2SizeToSize(XXX Log2Size) { return (1<<Log2Size); } 
template <class XXX> static inline XXX xLog2SizeToArea(XXX Log2Size) { return (1<<((Log2Size)<<1)); } 

template <class XXX> static inline XXX xNumUnitsCoveringLength(XXX Length, XXX Log2UnitSzize) { return ((Length - 1) >> Log2UnitSzize) + 1; }

template <class XXX> static inline XXX xRoundCntrToNearestMultiple(XXX Value, XXX Log2Multiple) { return (((Value + ((1 << Log2Multiple) >> 1)) >> Log2Multiple) << Log2Multiple); } //positive integer only
template <class XXX> static inline XXX xRoundUpToNearestMultiple  (XXX Value, XXX Log2Multiple) { return (((Value + ((1 << Log2Multiple) -  1)) >> Log2Multiple) << Log2Multiple); } //positive integer only
template <class XXX> static inline XXX xRoundDownToNearestMultiple(XXX Value, XXX Log2Multiple) { return (( Value                               >> Log2Multiple) << Log2Multiple); } //positive integer only

//===============================================================================================================================================================================================================
// flt32/64 to int32 rounding
//===============================================================================================================================================================================================================
#if X_SIMD_HAS_SSE
static inline int32 xRoundFlt32ToInt32(flt32 Flt) { return _mm_cvtss_si32(_mm_set_ss(Flt)); }
static inline int32 xRoundFlt64ToInt32(flt64 Flt) { return _mm_cvtsd_si32(_mm_set_sd(Flt)); }
#else  //X_SIMD_HAS_SSE
static inline int32 xRoundFlt32ToInt32(flt32 Flt) { return (int32)(std::round(Flt)); }
static inline int32 xRoundFlt64ToInt32(flt64 Flt) { return (int32)(std::round(Flt)); }
#endif //X_SIMD_HAS_SSE
template <class XXX> static inline int32 xRoundFltToInt32(XXX Flt);
template <> inline int32 xRoundFltToInt32(flt32 Flt) { return xRoundFlt32ToInt32 (Flt); }
template <> inline int32 xRoundFltToInt32(flt64 Flt) { return xRoundFlt64ToInt32(Flt); }

//===============================================================================================================================================================================================================
// flt32/64 madness
//===============================================================================================================================================================================================================
//https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
//https://stackoverflow.com/questions/17333/what-is-the-most-effective-way-for-float-and-double-comparison

template<typename XXX> static bool xIsApproximatelyEqual(XXX a, XXX b, XXX Tolerance = std::numeric_limits<XXX>::epsilon())
{
  XXX Diff = std::fabs(a - b); if(Diff <= Tolerance || Diff < std::fmax(std::fabs(a), std::fabs(b)) * Tolerance) { return true; }
  return false;
}
template<typename XXX> static inline bool xIsApproximatelyZero(XXX a, XXX Tolerance = std::numeric_limits<XXX>::epsilon()) { return (std::fabs(a           ) <= Tolerance); }
template<typename XXX> static inline bool xIsApproximatelyOne (XXX a, XXX Tolerance = std::numeric_limits<XXX>::epsilon()) { return (std::fabs(a - (XXX)1.0) <= Tolerance); }

template<typename XXX> static inline XXX xRoundValuesCloseToZeroOrOne(XXX a, XXX Tolerance = std::numeric_limits<XXX>::epsilon())
{
  if     ( xIsApproximatelyZero(a, Tolerance)) { return (XXX)0.0; }
  else if( xIsApproximatelyOne (a, Tolerance)) { return (XXX)1.0; }
  else                                         { return a       ; }
}

//===============================================================================================================================================================================================================
// Fast iteger Log2 using bsr (Bit Scan Reverse) x86/x64 instructions, undefined for Value == 0 (same as log2())
//===============================================================================================================================================================================================================
#if defined(X_COMPILER_MSVC)
static inline uint32 xFastLog2(uint32 Value) { unsigned long Log2; _BitScanReverse  (&Log2, (uint32)Value); return Log2; }
static inline uint64 xFastLog2(uint64 Value) { unsigned long Log2; _BitScanReverse64(&Log2, (uint64)Value); return Log2; }
#elif (X_COMPILER_GCC || X_COMPILER_CLANG)
static inline uint32 xFastLog2(uint32 Value) { return 31 - __builtin_clz  (Value); }
static inline uint64 xFastLog2(uint64 Value) { return 63 - __builtin_clzll(Value); }
#else
#error Unrecognized compiler
#endif

//=============================================================================================================================================================================
// Leading zero count
//=============================================================================================================================================================================
#if defined X_SIMD_HAS_AVX
#if X_COMPILER_MSVC
static inline uint16 xLZCNT(uint16 Val) { return __lzcnt16(Val); }
static inline uint32 xLZCNT(uint32 Val) { return __lzcnt  (Val); }
static inline uint64 xLZCNT(uint64 Val) { return __lzcnt64(Val); }
#elif X_COMPILER_GCC
static inline uint16 xLZCNT(uint16 Val) { return __builtin_ia32_lzcnt_u16(Val); }
static inline uint32 xLZCNT(uint32 Val) { return __builtin_ia32_lzcnt_u32(Val); }
static inline uint64 xLZCNT(uint64 Val) { return __builtin_ia32_lzcnt_u64(Val); }
#elif X_COMPILER_CLANG
static inline uint16 xLZCNT(uint16 Val) { return __lzcnt16(Val); }
static inline uint32 xLZCNT(uint32 Val) { return __lzcnt32(Val); }
static inline uint64 xLZCNT(uint64 Val) { return __lzcnt64(Val); }
#else
#error Unrecognized compiler
#endif
#else //X_SIMD_HAS_AVX
#if X_COMPILER_MSVC
static inline uint16 xLZCNT(uint16 Val) { unsigned long Index; return (uint16)(_BitScanReverse  (&Index, Val) ? (31 - Index) : 16); }
static inline uint32 xLZCNT(uint32 Val) { unsigned long Index; return (uint32)(_BitScanReverse  (&Index, Val) ? (31 - Index) : 32); }
static inline uint64 xLZCNT(uint64 Val) { unsigned long Index; return (uint64)(_BitScanReverse64(&Index, Val) ? (63 - Index) : 64); }
#elif (X_COMPILER_GCC || X_COMPILER_CLANG)
static inline uint16 xLZCNT(uint16 Val) { return Val ? (uint16)__builtin_clz  (Val) : 16; }
static inline uint32 xLZCNT(uint32 Val) { return Val ? (uint32)__builtin_clz  (Val) : 32; }
static inline uint64 xLZCNT(uint64 Val) { return Val ? (uint64)__builtin_clzll(Val) : 64; }
#else
#error Unrecognized compiler
#endif
#endif //X_SIMD_HAS_AVX

//=============================================================================================================================================================================
// Num significant bits (similar to xLog2, but returns 0 for Val==0, uses faster (i.e. on Zen) lzcnt
//=============================================================================================================================================================================
static inline uint16 xNumSignificantBits(uint16 Val) { return 16 - xLZCNT(Val); }
static inline uint32 xNumSignificantBits(uint32 Val) { return 32 - xLZCNT(Val); }
static inline uint64 xNumSignificantBits(uint64 Val) { return 32 - xLZCNT(Val); }

//uint16 xNumSignificantBits(uint16 Val)
//{
//  if(!Val) { return 0; }
//
//  int32 NumSigBits = 16;
//  if (!(Val & 0xff00)) { NumSigBits -= 8; Val <<= 8; }
//  if (!(Val & 0xf000)) { NumSigBits -= 4; Val <<= 4; }
//  if (!(Val & 0xc000)) { NumSigBits -= 2; Val <<= 2; }
//  if (!(Val & 0x8000)) { NumSigBits -= 1; Val <<= 1; }
//
//  return NumSigBits;
//}
//=============================================================================================================================================================================
// Byte swap
//=============================================================================================================================================================================
#if X_COMPILER_MSVC
static inline uint16 xSwapBytes16(uint16 Value) { return _byteswap_ushort(Value); }
static inline  int16 xSwapBytes16( int16 Value) { return _byteswap_ushort(Value); }
static inline uint32 xSwapBytes32(uint32 Value) { return _byteswap_ulong (Value); }
static inline  int32 xSwapBytes32( int32 Value) { return _byteswap_ulong (Value); }
static inline uint64 xSwapBytes64(uint64 Value) { return _byteswap_uint64(Value); }
static inline  int64 xSwapBytes64( int64 Value) { return _byteswap_uint64(Value); }
#elif (X_COMPILER_GCC || X_COMPILER_CLANG)
static inline uint16 xSwapBytes16(uint16 Value) { return __builtin_bswap16(Value); }
static inline  int16 xSwapBytes16( int16 Value) { return __builtin_bswap16(Value); }
static inline uint32 xSwapBytes32(uint32 Value) { return __builtin_bswap32(Value); }
static inline  int32 xSwapBytes32( int32 Value) { return __builtin_bswap32(Value); }
static inline uint64 xSwapBytes64(uint64 Value) { return __builtin_bswap64(Value); }
static inline  int64 xSwapBytes64( int64 Value) { return __builtin_bswap64(Value); }
#else
#error Unrecognized compiler
#endif

//===============================================================================================================================================================================================================
// type safe memset & memcpy
//===============================================================================================================================================================================================================
template <class XXX> static inline void xMemsetX(XXX* Dst, const XXX  Val, uint32 Count) { if constexpr(sizeof(XXX) == 1) { std::memset(Dst, Val, Count); } else { for(uint32 i = 0; i < Count; i++) Dst[i] = Val; } }
template <class XXX> static inline void xMemcpyX(XXX* Dst, const XXX* Src, uint32 Count) { std::memcpy(Dst, Src, Count*sizeof(XXX)); }

//===============================================================================================================================================================================================================
// Time is money
//===============================================================================================================================================================================================================
using tClock      = std::chrono::high_resolution_clock       ;
using tTimePoint  = tClock::time_point                       ;
using tDuration   = tClock::duration                         ;
using tDurationMS = std::chrono::duration<double, std::milli>;
using tDurationS  = std::chrono::duration<double            >;

//===============================================================================================================================================================================================================
// Math constants
//===============================================================================================================================================================================================================
template<class XXX> constexpr XXX xc_Pi       = XXX( 3.14159265358979323846L ); // pi
template<class XXX> constexpr XXX xc_2Pi      = XXX( 6.28318530717958647692L ); // 2*pi
template<class XXX> constexpr XXX xc_4Pi      = XXX(12.56637061435917295384L ); // 4*pi
template<class XXX> constexpr XXX xc_PiDiv2   = XXX( 1.57079632679489661923L ); // pi/2
template<class XXX> constexpr XXX xc_1DivPi   = XXX( 0.318309886183790671538L); // 1/pi
template<class XXX> constexpr XXX xc_1Div2Pi  = XXX( 0.15915494309189533577L ); // 1/(2*pi)
template<class XXX> constexpr XXX xc_DegToRad = xc_Pi<XXX> / XXX(180);
template<class XXX> constexpr XXX xc_RadToDeg = XXX(180) / xc_Pi<XXX>;

//===============================================================================================================================================================================================================
// Multiple and remainder
//===============================================================================================================================================================================================================
static constexpr uint32 c_MultipleMask4    = 0xFFFFFFFC;
static constexpr uint32 c_MultipleMask8    = 0xFFFFFFF8;
static constexpr uint32 c_MultipleMask16   = 0xFFFFFFF0;
static constexpr uint32 c_MultipleMask32   = 0xFFFFFFE0;
static constexpr uint32 c_MultipleMask64   = 0xFFFFFFC0;
static constexpr uint32 c_MultipleMask128  = 0xFFFFFF80;

static constexpr uint32 c_RemainderMask4   = 0x00000003;
static constexpr uint32 c_RemainderMask8   = 0x00000007;
static constexpr uint32 c_RemainderMask16  = 0x0000000F;
static constexpr uint32 c_RemainderMask32  = 0x0000001F;
static constexpr uint32 c_RemainderMask64  = 0x0000003F;
static constexpr uint32 c_RemainderMask128 = 0x0000007F;

//===============================================================================================================================================================================================================
// Common enums
//===============================================================================================================================================================================================================
enum class eCmp : int32
{
  INVALID = NOT_VALID,

  //generic
  C0 = 0,
  C1 = 1,
  C2 = 2,
  C3 = 3,

  //Y Cb Cr
  LM = 0,     //Luma   L  (Y)
  CB = 1,     //Chroma Cb (U)
  CR = 2,     //Chroma Cr (V)

  //R G B
  R = 0,
  G = 1,
  B = 2,
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
