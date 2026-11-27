/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefBASE.h"

//===============================================================================================================================================================================================================
// base includes
//===============================================================================================================================================================================================================
#if __has_include(<sys/mman.h>)
#include <sys/mman.h>
#endif

#if X_PMBB_CPP20
#include <bit>
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
#ifndef PMBB_SIMD_ALLOWED
#define PMBB_SIMD_ALLOWED 1
#endif

#define PMBB_USE_SIMD  1 // use SIMD

namespace PMBB_NAMESPACE {

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// SIMD section - all SSE (corresponding to x86-64-v2)
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//MSVC does not define __SSEn__ macros. Assuming all extensions present.
//#if defined(_MSC_VER)
//#define __SSE__    1
//#define __SSE2__   1
//#define __SSE3__   1
//#define __SSSE3__  1
//#define __SSE4_1__ 1
//#define __SSE4_2__ 1
//#endif

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
#define X_SIMD_CAN_USE_SSE (X_SIMD_HAS_SSE && PMBB_SIMD_ALLOWED && PMBB_USE_SIMD)

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
#define X_SIMD_CAN_USE_AVX (X_SIMD_HAS_AVX && PMBB_SIMD_ALLOWED && PMBB_USE_SIMD)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// SIMD section - AVX512 [F,BW,CD,DQ,VL] (corresponding to x86-64-v4)
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//AVX512 - what a mess
#if defined(__AVX512F__) && defined(__AVX512BW__) && defined(__AVX512CD__) && defined(__AVX512DQ__) && defined(__AVX512VL__)
#define X_SIMD_HAS_AVX512 (__AVX512F__ && __AVX512BW__ && __AVX512CD__ && __AVX512DQ__ && __AVX512VL__)
#else
#define X_SIMD_HAS_AVX512 0
#endif
#define X_SIMD_CAN_USE_AVX512 (X_SIMD_HAS_AVX512 && PMBB_SIMD_ALLOWED && PMBB_USE_SIMD)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// SIMD section - AVX512 above x86-64-v4
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#if X_SIMD_HAS_AVX512 && defined(__AVX512BITALG__) && defined(__AVX512VBMI2__) && defined(__AVX512VBMI__) && defined(__AVX512IFMA__) &&  defined(__AVX512VPOPCNTDQ__) &&  defined(__AVX512BF16__) && defined(__AVX512VNNI__) && defined(__VPCLMULQDQ__)
#define X_SIMD_HAS_AVX512_ZEN4 1
#else
#define X_SIMD_HAS_AVX512_ZEN4 0
#endif
#define X_SIMD_CAN_USE_AVX512_ZEN4 (X_SIMD_HAS_AVX512_ZEN4 && PMBB_SIMD_ALLOWED && PMBB_USE_SIMD)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// SIMD section - NEON (corresponding to ARMv8.0) + other ARM64 extensions
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//NEON
#if defined (__ARM_NEON)
#define X_SIMD_HAS_NEON 1
#else
#define X_SIMD_HAS_NEON 0
#endif
#define X_SIMD_CAN_USE_NEON (X_SIMD_HAS_NEON && PMBB_SIMD_ALLOWED && PMBB_USE_SIMD)

//CRC32 extension
#if defined(__ARM_FEATURE_CRC32)
#define X_PMBB_HAS_ARM_CRC32 1
#else
#define X_PMBB_HAS_ARM_CRC32 0
#endif
#define X_PMBB_CAN_USE_ARM_CRC32 (X_PMBB_HAS_ARM_CRC32)

//CRYPTO extension
#if defined(__ARM_FEATURE_CRYPTO)
#define X_PMBB_HAS_ARM_CRYPTO 1
#else
#define X_PMBB_HAS_ARM_CRYPTO 0
#endif
#define X_PMBB_CAN_USE_ARM_CRYPTO (X_SIMD_HAS_NEON && X_PMBB_HAS_ARM_CRYPTO)

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
template <class XXX> static inline XXX xClipPos (XXX x                  ) { return xMax((XXX)0, x); }
template <class XXX> static inline XXX xReLU    (XXX x                  ) { return xClipPos(x); }

template <class XXX> static inline XXX xAbs     (XXX a) { return (XXX)std::abs(a); }
template <class XXX> static inline XXX xSign    (XXX a) { return a == 0 ? 0 : a > 0 ? 1 : -1; }
template <class XXX> static inline XXX xSignNoZ (XXX a) { return a >= 0 ? 1 : -1; }
template <class XXX> static inline XXX xPow2    (XXX x) { return x * x; }
template <class XXX> static inline XXX xPow3    (XXX x) { return x * x * x; }

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

template <class XXX> static inline bool xInRange(XXX x, XXX min, XXX max) { return ((x <= max) && (x >= min)); }

//===============================================================================================================================================================================================================
// integer divide by 2^n using shift
//===============================================================================================================================================================================================================

template <class XXX> static inline XXX xDivideUsingShiftPositive(XXX Val, int32 Log2Divisor)
{
  XXX Bias = (1 << (Log2Divisor - 1));
  return (Val + Bias) >> Log2Divisor;
}
template <class XXX> static inline XXX xDivideUsingShiftSigned(XXX Val, int32 Log2Divisor)
{
  //Val>=0 --> Mask= 0; Offset= Bias
  //Val< 0 --> Mask=-1; Offset=-Bias
  XXX Bias   = (XXX)((XXX)1 << (Log2Divisor - 1));
  XXX Mask   = Val >> (sizeof(XXX) * 8 - 1);
  XXX Offset = (Bias ^ Mask) - Mask;
  return (Val + Offset) >> Log2Divisor;
}

//===============================================================================================================================================================================================================
// Fast iteger Log2 using bsr (Bit Scan Reverse) x86/x64 instructions, undefined for Value == 0 (same as log2())
//===============================================================================================================================================================================================================
#if X_PMBB_CPP20
static inline uint32 xFastLog2(uint32 Value) { assert(Value > 0); return 31 - std::countl_zero(Value); }
static inline uint64 xFastLog2(uint64 Value) { assert(Value > 0); return 63 - std::countl_zero(Value); }
#else //X_PMBB_CPP20
  #if defined(X_PMBB_ARCH_AMD64) && (defined(X_PMBB_COMPILER_MSVC) || defined(X_PMBB_COMPILER_ICC))
    #if X_SIMD_HAS_AVX
static inline uint32 xFastLog2(uint32 Value) { assert(Value > 0); return 31 - __lzcnt  (Value); }
static inline uint64 xFastLog2(uint64 Value) { assert(Value > 0); return 63 - __lzcnt64(Value); }
    #else //X_SIMD_HAS_AVX
static inline uint32 xFastLog2(uint32 Value) { assert(Value > 0); unsigned long Log2; _BitScanReverse  (&Log2, (uint32)Value); return Log2; }
static inline uint64 xFastLog2(uint64 Value) { assert(Value > 0); unsigned long Log2; _BitScanReverse64(&Log2, (uint64)Value); return Log2; }
    #endif //X_SIMD_HAS_AVX
  #endif 
  #if defined(X_PMBB_ARCH_ARM64) && defined(X_PMBB_COMPILER_MSVC)
static inline uint32 xFastLog2(uint32 Value) { assert(Value > 0); return 31 - _CountLeadingZeros  (Value); }
static inline uint64 xFastLog2(uint64 Value) { assert(Value > 0); return 63 - _CountLeadingZeros64(Value); }
  #endif 
  #if defined(X_PMBB_COMPILER_GCC) || defined(X_PMBB_COMPILER_CLANG)
static inline uint32 xFastLog2(uint32 Value) { assert(Value > 0); return 31 - __builtin_clz  (Value); }
static inline uint64 xFastLog2(uint64 Value) { assert(Value > 0); return 63 - __builtin_clzll(Value); }
  #endif
#endif //X_PMBB_CPP20

static inline bool xIsPowerOf2(uint32 Value) { assert(Value > 0); return Value == (uint32)1 << xFastLog2(Value); }

//===============================================================================================================================================================================================================
// type safe memset & memcpy
//===============================================================================================================================================================================================================
template <class XXX> static inline void xMemsetX(XXX* Dst, const XXX  Val, uintSize Count) { if constexpr(sizeof(XXX) == 1 && std::is_integral_v<XXX>) { std::memset(Dst, Val, Count); } else { for(uintSize i = 0; i < Count; i++) Dst[i] = Val; } }
template <class XXX> static inline void xMemcpyX(XXX* Dst, const XXX* Src, uintSize Count) { std::memcpy(Dst, Src, Count*sizeof(XXX)); }

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
template<class XXX> constexpr XXX c_RemainderMask4   = 0x0003;
template<class XXX> constexpr XXX c_RemainderMask8   = 0x0007;
template<class XXX> constexpr XXX c_RemainderMask16  = 0x000F;
template<class XXX> constexpr XXX c_RemainderMask32  = 0x001F;
template<class XXX> constexpr XXX c_RemainderMask64  = 0x003F;
template<class XXX> constexpr XXX c_RemainderMask128 = 0x007F;
template<class XXX> constexpr XXX c_RemainderMask4k  = 0x0FFF;

template<class XXX> constexpr XXX c_MultipleMask4   = ~c_RemainderMask4  <XXX>; //0xFFFFFFFC;
template<class XXX> constexpr XXX c_MultipleMask8   = ~c_RemainderMask8  <XXX>; //0xFFFFFFF8;
template<class XXX> constexpr XXX c_MultipleMask16  = ~c_RemainderMask16 <XXX>; //0xFFFFFFF0;
template<class XXX> constexpr XXX c_MultipleMask32  = ~c_RemainderMask32 <XXX>; //0xFFFFFFE0;
template<class XXX> constexpr XXX c_MultipleMask64  = ~c_RemainderMask64 <XXX>; //0xFFFFFFC0;
template<class XXX> constexpr XXX c_MultipleMask128 = ~c_RemainderMask128<XXX>; //0xFFFFFF80;
template<class XXX> constexpr XXX c_MultipleMask4k  = ~c_RemainderMask4k <XXX>; //0xFFFFF000;

//===============================================================================================================================================================================================================
// Common enums
//===============================================================================================================================================================================================================
enum class eCmp : int32 //component identifier
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

  //R G B (A)
  R = 0,
  G = 1,
  B = 2,
  A = 3,
};

enum class eCrF : int16 //chroma format
{
  INVALID = NOT_VALID,
  UNKNOWN = 0,
  CF444   = 444,
  CF422   = 422,  
  CF420   = 420,
  CF400   = 400, //no chroma
  CFx44   = 44 , //no luma
  CFx22   = 22 , //no luma
  CFx20   = 20 , //no luma
};

enum class eImgTp : int8 //Image Type
{
  INVALID = NOT_VALID,
  UNKNOWN = 0,
  YCbCr,   //YUV  (YCbCr, YCoCg, ...)
//YCbCrA,  //YUV+A(alpha)
//YCbCrD,  //YUV+D(depth)
  RGB,     //RGB
  BGR,     //BGR
  GBR,     //GBR
//Bayer,   //M - bayer
};

enum class eClrSpcLC : int32 //colorspaces using luma and chromas
{
  INVALID   = -1,
  //standardized YCbCr
  BT601     = 0,
  SMPTE170M = 0, //alias
  JPEG      = 0, //alias
  BT709     = 1,
  SMPTE240M = 2,
  BT2020    = 3,
  //exotic YCbCr
  JPEG2000  = 4,
  //other
  YCoCg     = 5,
  YCoCgR    = 6,
};

enum class eMrgExt : int32 //picture margin extension mode // trying to be consistent with numpy.pad and scipy.ndimage.generic_filter
{
  INVALID  = -1,
  None     = 0,
  Nearest  = 1,  //( a a a | a b c d | d d d ) //scipy-nearest, numpy-edge       
  Reflect  = 2,  //( c b a | a b c d | d c b ) //scipy-reflect, numpy-symmetric
  Mirror   = 3,  //( d c b | a b c d | c b a ) //scipy-mirror , numpy-reflect  
  Constant = 4,  //( k k k | a b c d | k k k )
  Zero     = 5,  //( 0 0 0 | a b c d | 0 0 0 )
};

enum class eActn : int32
{
  INVALID = NOT_VALID,
  SKIP    = 0, //SKIP - disable value checking,
  WARN    = 1, //WARN - print warning and ignore
  STOP    = 2, //STOP - stop execution
  CNCL    = 3, //CNCL - try to conceal
};

enum class eAppRes : int32
{
  Unknown = -1,
  Good        ,
  Warning     ,
  Error        
};

enum class eFileFmt : int32
{
  INVALID = -1,
  RAW,
  PNG,
  BMP,
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
