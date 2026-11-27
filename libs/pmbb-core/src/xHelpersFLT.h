/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefCORE.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// flt32/64 to int32 rounding
// WARNING !!!
// Nearbyint uses "Round Half Even (2.5 → 2)"
// Round     uses "Round Half Away (2.5 → 3)"
//===============================================================================================================================================================================================================
#if X_SIMD_HAS_AVX512
static inline uint32 xNearbyF32ToU32(flt32 Flt) { return _mm_cvtss_u32(_mm_set_ss(Flt)); }
static inline int32  xNearbyF32ToI32(flt32 Flt) { return _mm_cvtss_i32(_mm_set_ss(Flt)); }
static inline uint32 xNearbyF64ToU32(flt64 Flt) { return _mm_cvtsd_u32(_mm_set_sd(Flt)); }
static inline int32  xNearbyF64ToI32(flt64 Flt) { return _mm_cvtsd_i32(_mm_set_sd(Flt)); }
static inline uint64 xNearbyF64ToU64(flt64 Flt) { return _mm_cvtsd_u64(_mm_set_sd(Flt)); }
static inline int64  xNearbyF64ToI64(flt64 Flt) { return _mm_cvtsd_i64(_mm_set_sd(Flt)); }
static inline uint32 xRoundF32ToU32 (flt32 Flt) { return _mm_cvttss_u32(_mm_add_ss(_mm_set_ss(Flt), _mm_set_ss(0.5f))); }
static inline int32  xRoundF32ToI32 (flt32 Flt) { __m128  FltSS = _mm_set_ss(Flt); return _mm_cvttss_i32(_mm_add_ss(FltSS, _mm_or_ps(_mm_set_ss(0.5f), _mm_and_ps(FltSS, _mm_set_ss(-0.0f))))); }
static inline uint32 xRoundF64ToU32 (flt64 Flt) { return _mm_cvttsd_u32(_mm_add_sd(_mm_set_sd(Flt), _mm_set_sd(0.5f))); }
static inline int32  xRoundF64ToI32 (flt64 Flt) { __m128d FltSS = _mm_set_sd(Flt); return _mm_cvttsd_i32(_mm_add_sd(FltSS, _mm_or_pd(_mm_set_sd(0.5f), _mm_and_pd(FltSS, _mm_set_sd(-0.0f))))); }
#elif X_PMBB_ARCH_AMD64
static inline uint32 xNearbyF32ToU32(flt32 Flt) { return (uint32)_mm_cvtss_si32(_mm_set_ss(Flt)); }
static inline int32  xNearbyF32ToI32(flt32 Flt) { return _mm_cvtss_si32(_mm_set_ss(Flt)); }
static inline uint32 xNearbyF64ToU32(flt64 Flt) { return (uint32)_mm_cvtsd_si32(_mm_set_sd(Flt)); }
static inline int32  xNearbyF64ToI32(flt64 Flt) { return _mm_cvtsd_si32(_mm_set_sd(Flt)); }
static inline uint64 xNearbyF64ToU64(flt64 Flt) { return (uint64)_mm_cvtsd_si64(_mm_set_sd(Flt)); }
static inline int64  xNearbyF64ToI64(flt64 Flt) { return _mm_cvtsd_si64(_mm_set_sd(Flt)); }
static inline uint32 xRoundF32ToU32 (flt32 Flt) { return (uint32)_mm_cvttss_si32(_mm_add_ss(_mm_set_ss(Flt), _mm_set_ss(0.5f))); }
static inline int32  xRoundF32ToI32 (flt32 Flt) { __m128  FltSS = _mm_set_ss(Flt); return _mm_cvttss_si32(_mm_add_ss(FltSS, _mm_or_ps(_mm_set_ss(0.5f), _mm_and_ps(FltSS, _mm_set_ss(-0.0f))))); }
static inline uint32 xRoundF64ToU32 (flt64 Flt) { return (uint32)_mm_cvttsd_si32(_mm_add_sd(_mm_set_sd(Flt), _mm_set_sd(0.5f))); }
static inline int32  xRoundF64ToI32 (flt64 Flt) { __m128d FltSS = _mm_set_sd(Flt); return _mm_cvttsd_si32(_mm_add_sd(FltSS, _mm_or_pd(_mm_set_sd(0.5f), _mm_and_pd(FltSS, _mm_set_sd(-0.0f))))); }
#elif X_PMBB_ARCH_ARM64
static inline uint32 xNearbyF32ToU32(flt32 Flt) { return vcvtns_u32_f32(Flt); }
static inline int32  xNearbyF32ToI32(flt32 Flt) { return vcvtns_s32_f32(Flt); }
static inline uint32 xNearbyF64ToU32(flt64 Flt) { return vcvtnd_u64_f64(Flt); }
static inline int32  xNearbyF64ToI32(flt64 Flt) { return vcvtnd_s64_f64(Flt); }
static inline uint64 xNearbyF64ToU64(flt64 Flt) { return vcvtnd_u64_f64(Flt); }
static inline int64  xNearbyF64ToI64(flt64 Flt) { return vcvtnd_s64_f64(Flt); }
static inline uint32 xRoundF32ToU32 (flt32 Flt) { return vcvtas_u32_f32(Flt); }
static inline int32  xRoundF32ToI32 (flt32 Flt) { return vcvtas_s32_f32(Flt); }
static inline uint32 xRoundF64ToU32 (flt64 Flt) { return vcvtad_u64_f64(Flt); }
static inline int32  xRoundF64ToI32 (flt64 Flt) { return vcvtad_s64_f64(Flt); }
#else  
static inline uint32 xNearbyF32ToU32(flt32 Flt) { return (uint32)(std::nearbyintf(Flt)); }
static inline int32  xNearbyF32ToI32(flt32 Flt) { return ( int32)(std::nearbyintf(Flt)); }
static inline uint32 xNearbyF64ToU32(flt64 Flt) { return (uint32)(std::nearbyintf(Flt)); }
static inline int32  xNearbyF64ToI32(flt64 Flt) { return ( int32)(std::nearbyintf(Flt)); }
static inline uint64 xNearbyF64ToU64(flt64 Flt) { return (uint64)(std::nearbyint (Flt)); }
static inline int64  xNearbyF64ToI64(flt64 Flt) { return ( int64)(std::nearbyint (Flt)); }
static inline uint32 xRoundF32ToU32 (flt32 Flt) { return (uint32)(std::round     (Flt)); }
static inline int32  xRoundF32ToI32 (flt32 Flt) { return ( int32)(std::round     (Flt)); }
static inline uint32 xRoundF64ToU32 (flt64 Flt) { return (uint32)(std::round     (Flt)); }
static inline int32  xRoundF64ToI32 (flt64 Flt) { return ( int32)(std::round     (Flt)); }
#endif
template <class XXX> static inline int32 xNearbyFltToI32(XXX Flt);
template <> inline int32 xNearbyFltToI32(flt32 Flt) { return xNearbyF32ToI32(Flt); }
template <> inline int32 xNearbyFltToI32(flt64 Flt) { return xNearbyF64ToI32(Flt); }
template <class XXX> static inline int32 xRoundFltToI32(XXX Flt);
template <> inline int32 xRoundFltToI32(flt32 Flt) { return xRoundF32ToI32(Flt); }
template <> inline int32 xRoundFltToI32(flt64 Flt) { return xRoundF64ToI32(Flt); }

//===============================================================================================================================================================================================================
// flt32/64 madness
//===============================================================================================================================================================================================================
//https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
//https://stackoverflow.com/questions/17333/what-is-the-most-effective-way-for-float-and-double-comparison

template<typename XXX> static bool xIsApproximatelyEqual(XXX a, XXX b, XXX Tolerance = 8*std::numeric_limits<XXX>::epsilon())
{
  XXX Diff = std::fabs(a - b); if(Diff <= Tolerance || Diff < std::fmax(std::fabs(a), std::fabs(b)) * Tolerance) { return true; }
  return false;
}
template<typename XXX> static inline bool xIsApproximatelyZero(XXX a, XXX Tolerance = 8*std::numeric_limits<XXX>::epsilon()) { return (std::fabs(a           ) <= Tolerance); }
template<typename XXX> static inline bool xIsApproximatelyOne (XXX a, XXX Tolerance = 8*std::numeric_limits<XXX>::epsilon()) { return (std::fabs(a - (XXX)1.0) <= Tolerance); }

template<typename XXX> static inline XXX xRoundValuesCloseToZeroOrOne(XXX a, XXX Tolerance = 8*std::numeric_limits<XXX>::epsilon())
{
  if     ( xIsApproximatelyZero(a, Tolerance)) { return (XXX)0.0; }
  else if( xIsApproximatelyOne (a, Tolerance)) { return (XXX)1.0; }
  else                                         { return a       ; }
}

//===============================================================================================================================================================================================================
// Floating point contraction macros
//===============================================================================================================================================================================================================
#if defined(X_PMBB_COMPILER_MSVC) // MSVC uses float_control push/pop to preserve contraction state   
#  define X_PMBB_DISABLE_FP_CONTRACT_BEG __pragma(float_control(push)) __pragma(fp_contract(off)) //TODO check float_control(precise, on)
#  define X_PMBB_DISABLE_FP_CONTRACT_END __pragma(float_control(pop))
#elif defined(X_PMBB_COMPILER_GCC) // GCC uses push_options/pop_options for optimize pragmas
#  define X_PMBB_DISABLE_FP_CONTRACT_BEG _Pragma("GCC push_options") _Pragma("GCC optimize(\"-ffp-contract=off\")")
#  define X_PMBB_DISABLE_FP_CONTRACT_END _Pragma("GCC pop_options")
#elif defined(X_PMBB_COMPILER_CLANG) // Clang supports float_control push/pop stack
#  define X_PMBB_DISABLE_FP_CONTRACT_BEG _Pragma("float_control(push)") _Pragma("clang fp contract(off)")
#  define X_PMBB_DISABLE_FP_CONTRACT_END _Pragma("float_control(pop)")
#else // C/C++ Standard fallback (resets to default behavior)
#  define X_PMBB_DISABLE_FP_CONTRACT_BEG _Pragma("STDC FP_CONTRACT OFF")
#  define X_PMBB_DISABLE_FP_CONTRACT_END _Pragma("STDC FP_CONTRACT DEFAULT")
#endif

//===============================================================================================================================================================================================================

} //end of namespace PMBB
