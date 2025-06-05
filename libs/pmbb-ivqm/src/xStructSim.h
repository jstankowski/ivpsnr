/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVQM.h"

//portable implementation
#include "xStructSimSTD.h"

//SSE implementation
#if X_SIMD_CAN_USE_SSE && __has_include("xStructSimSSE.h")
#define X_STRUCTSIM_CAN_USE_SSE 1
#include "xStructSimSSE.h"
#else
#define X_STRUCTSIM_CAN_USE_SSE 0
#endif

//AVX implementation
#if X_SIMD_CAN_USE_AVX && __has_include("xStructSimAVX.h")
#define X_STRUCTSIM_CAN_USE_AVX 1
#include "xStructSimAVX.h"
#else
#define X_STRUCTSIM_CAN_USE_AVX 0
#endif

//AVX512 implementation
#if X_SIMD_CAN_USE_AVX512 && __has_include("xStructSimAVX512.h")
#define X_STRUCTSIM_CAN_USE_AVX512 1
#include "xStructSimAVX512.h"
#else
#define X_STRUCTSIM_CAN_USE_AVX512 0
#endif

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xStructSim //Structural Similarity
{
public:
  //Regular Structural Similarity
  static flt64 CalcRglrFlt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimSTD::CalcRglrFlt(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); } //uses gaussian window
  static flt64 CalcRglrInt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimSTD::CalcRglrInt(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); } //uses gaussian window
  static flt64 CalcRglrAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimSTD::CalcRglrAvg(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); } //uses averaging

  //Regular Structural Similarity - with mask
  static flt64 CalcRglrFltM(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 StrideT, int32 StrideR, int32 StrideM, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimSTD::CalcRglrFltM(Tst, Ref, Msk, StrideT, StrideR, StrideM, WndSize, C1, C2, CalcL); } //uses gaussian window
  static flt64 CalcRglrIntM(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 StrideT, int32 StrideR, int32 StrideM, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimSTD::CalcRglrIntM(Tst, Ref, Msk, StrideT, StrideR, StrideM, WndSize, C1, C2, CalcL); } //uses gaussian window
  static flt64 CalcRglrAvgM(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 StrideT, int32 StrideR, int32 StrideM, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimSTD::CalcRglrAvgM(Tst, Ref, Msk, StrideT, StrideR, StrideM, WndSize, C1, C2, CalcL); } //uses averaging

  //Block Structural Similarity
  static inline flt64 CalcBlckInt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimSTD::CalcBlckInt(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); }

#if   X_STRUCTSIM_CAN_USE_AVX512
  static inline flt64 CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimAVX512::CalcBlckAvg(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); }
#elif X_STRUCTSIM_CAN_USE_AVX
  static inline flt64 CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimAVX::CalcBlckAvg(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); }
#elif X_STRUCTSIM_CAN_USE_SSE
  static inline flt64 CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimSSE::CalcBlckAvg(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); }
#else
  static inline flt64 CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimSTD::CalcBlckAvg(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); }
#endif
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#undef X_STRUCTSIM_CAN_USE_SSE
#undef X_STRUCTSIM_CAN_USE_AVX
#undef X_STRUCTSIM_CAN_USE_AVX512

//===============================================================================================================================================================================================================

} //end of namespace PMBB