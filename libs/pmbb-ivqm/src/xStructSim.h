/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2025 Patrycja Kaźmierczak <patrycja.kazmierczak@student.put.poznan.pl>
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

//NEON implementation
#if X_SIMD_CAN_USE_NEON && __has_include("xStructSimNEON.h")
#define X_STRUCTSIM_CAN_USE_NEON 1
#include "xStructSimNEON.h"
#else
#define X_STRUCTSIM_CAN_USE_NEON 0
#endif

#define CHKW11  { assert(WndSize==11); if(WndSize!=11){ return std::numeric_limits<flt64>::signaling_NaN(); } }
#define CHKW11M { assert(WndSize==11); if(WndSize!=11){ return {std::numeric_limits<flt64>::signaling_NaN(), std::numeric_limits<flt64>::signaling_NaN()}; } }

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xStructSim //Structural Similarity
{
public:
  //Regular Structural Similarity
#if X_STRUCTSIM_CAN_USE_AVX512
  static flt64 CalcRglrFlt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { CHKW11; return xStructSimAVX512::CalcRglrFlt(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); } //uses gaussian window
  static flt64 CalcRglrInt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { CHKW11; return xStructSimAVX512::CalcRglrInt(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); } //uses gaussian window
#else
  static flt64 CalcRglrFlt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { CHKW11; return xStructSimSTD::CalcRglrFlt(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); } //uses gaussian window
  static flt64 CalcRglrInt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { CHKW11; return xStructSimSTD::CalcRglrInt(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); } //uses gaussian window
#endif

#if   X_STRUCTSIM_CAN_USE_AVX512
  static flt64 CalcRglrAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { CHKW11; return xStructSimAVX512::CalcRglrAvg(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); } //uses averaging
#elif X_STRUCTSIM_CAN_USE_AVX
  static flt64 CalcRglrAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { CHKW11; return xStructSimAVX::CalcRglrAvg(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); } //uses averaging
#elif X_STRUCTSIM_CAN_USE_SSE
  static flt64 CalcRglrAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { CHKW11; return xStructSimSSE::CalcRglrAvg(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); } //uses averaging
#else
  static flt64 CalcRglrAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { CHKW11; return xStructSimSTD::CalcRglrAvg(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); } //uses averaging
#endif

  //Block Structural Similarity
  static inline flt64 CalcBlckInt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimSTD::CalcBlckInt(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); }

#if   X_STRUCTSIM_CAN_USE_AVX512
  static inline flt64 CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimAVX512::CalcBlckAvg(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); }
#elif X_STRUCTSIM_CAN_USE_AVX
  static inline flt64 CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimAVX::CalcBlckAvg(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); }
#elif X_STRUCTSIM_CAN_USE_SSE
  static inline flt64 CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimSSE::CalcBlckAvg(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); }
#elif X_STRUCTSIM_CAN_USE_NEON
  static inline flt64 CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimNEON::CalcBlckAvg(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); }
#else
  static inline flt64 CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimSTD::CalcBlckAvg(Tst, Ref, StrideT, StrideR, WndSize, C1, C2, CalcL); }
#endif

  //Regular Structural Similarity - with mask
  static tSSIMM CalcRglrIntM(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 StrideT, int32 StrideR, int32 StrideM, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { CHKW11M; return xStructSimSTD::CalcRglrIntM(Tst, Ref, Msk, StrideT, StrideR, StrideM, WndSize, C1, C2, CalcL); } //uses gaussian window
  static tSSIMM CalcRglrAvgM(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 StrideT, int32 StrideR, int32 StrideM, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { CHKW11M; return xStructSimSTD::CalcRglrAvgM(Tst, Ref, Msk, StrideT, StrideR, StrideM, WndSize, C1, C2, CalcL); } //uses averaging

  //Block Structural Similarity - with mask
  static tSSIMM CalcBlckAvgM(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 StrideT, int32 StrideR, int32 StrideM, int32 WndSize, flt64 C1, flt64 C2, bool CalcL) { return xStructSimSTD::CalcBlckAvgM(Tst, Ref, Msk, StrideT, StrideR, StrideM, WndSize, C1, C2, CalcL); } //uses averaging

};

//===============================================================================================================================================================================================================

class xStructSimMultiBlk //Multi-Block Structural Similarity
{
public:
  //Multi-Block Structural Similarity
  static constexpr int32 c_MaxBatchSize = 8;

  using tCalcPtrMultiBlkAvgTail  = flt64(                       const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL);
  using tCalcPtrMultiBlkAvgBatch = void (flt64* restrict SSIMs, const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL);

#if X_STRUCTSIM_CAN_USE_AVX512
  static int32 getMultiBlockAvgBatchSize(int32 WndSize, int32 WndStride)
  {
    if     (WndSize == 8 && WndStride == 4) { return 7; }
    else if(WndSize == 8 && WndStride == 8) { return 4; }
    else                                    { return 0; }
  }
  static tCalcPtrMultiBlkAvgBatch* getCalcPtrMultiBlkAvgBatch(int32 WndSize, int32 WndStride)
  {
    if     (WndSize == 8 && WndStride == 4) { return xStructSimAVX512::CalcMultiBlckAvg8S4; }
    else if(WndSize == 8 && WndStride == 8) { return xStructSimAVX512::CalcMultiBlckAvg8S8; }
    else                                    { return nullptr                              ; }
  }
  static tCalcPtrMultiBlkAvgTail* getCalcPtrMultiBlkAvgTail(int32 WndSize, int32 /*WndStride*/)
  {
    if  (WndSize == 8) { return xStructSimAVX512::CalcBlckAvg8; }
    else               { return nullptr                       ; }
  }
#else
  static int32                     getMultiBlockAvgBatchSize (int32 /*WndSize*/, int32 /*WndStride*/) { return 0      ; }
  static tCalcPtrMultiBlkAvgBatch* getCalcPtrMultiBlkAvgBatch(int32 /*WndSize*/, int32 /*WndStride*/) { return nullptr; }
  static tCalcPtrMultiBlkAvgTail*  getCalcPtrMultiBlkAvgTail (int32 /*WndSize*/, int32 /*WndStride*/) { return nullptr; }
#endif
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#undef CHKW11
#undef CHKW11M

#undef X_STRUCTSIM_CAN_USE_SSE
#undef X_STRUCTSIM_CAN_USE_AVX
#undef X_STRUCTSIM_CAN_USE_AVX512
#undef X_STRUCTSIM_CAN_USE_NEON

//===============================================================================================================================================================================================================

} //end of namespace PMBB