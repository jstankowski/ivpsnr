/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
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


namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xDistortion
{
public:
#if   X_CAN_USE_AVX512

  static inline  int32 CalcSD (const uint16* Tst, const uint16* Ref,                                   int32 Area               ) { return xDistortionAVX512::CalcSD (Tst, Ref,                       Area          ); }
  static inline  int32 CalcSD (const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height) { return xDistortionAVX512::CalcSD (Tst, Ref, TstStride, RefStride, Width,  Height); }
  static inline uint32 CalcSAD(const uint16* Tst, const uint16* Ref,                                   int32 Area               ) { return xDistortionAVX512::CalcSAD(Tst, Ref,                       Area          ); }
  static inline uint32 CalcSAD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height) { return xDistortionAVX512::CalcSAD(Tst, Ref, TstStride, RefStride, Width,  Height); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref,                                   int32 Area               ) { return xDistortionAVX512::CalcSSD(Tst, Ref,                       Area          ); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height) { return xDistortionAVX512::CalcSSD(Tst, Ref, TstStride, RefStride, Width,  Height); }

#elif X_CAN_USE_AVX

  static inline  int32 CalcSD (const uint16* Tst, const uint16* Ref,                                   int32 Area               ) { return xDistortionAVX::CalcSD (Tst, Ref,                       Area          ); }
  static inline  int32 CalcSD (const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height) { return xDistortionAVX::CalcSD (Tst, Ref, TstStride, RefStride, Width,  Height); }
  static inline uint32 CalcSAD(const uint16* Tst, const uint16* Ref,                                   int32 Area               ) { return xDistortionAVX::CalcSAD(Tst, Ref,                       Area          ); }
  static inline uint32 CalcSAD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height) { return xDistortionAVX::CalcSAD(Tst, Ref, TstStride, RefStride, Width,  Height); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref,                                   int32 Area               ) { return xDistortionAVX::CalcSSD(Tst, Ref,                       Area          ); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height) { return xDistortionAVX::CalcSSD(Tst, Ref, TstStride, RefStride, Width,  Height); }

#elif X_CAN_USE_SSE

  static inline  int32 CalcSD (const uint16* Tst, const uint16* Ref,                                   int32 Area               ) { return xDistortionSSE::CalcSD (Tst, Ref,                       Area          ); }
  static inline  int32 CalcSD (const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height) { return xDistortionSSE::CalcSD (Tst, Ref, TstStride, RefStride, Width,  Height); }
  static inline uint32 CalcSAD(const uint16* Tst, const uint16* Ref,                                   int32 Area               ) { return xDistortionSSE::CalcSAD(Tst, Ref,                       Area          ); }
  static inline uint32 CalcSAD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height) { return xDistortionSSE::CalcSAD(Tst, Ref, TstStride, RefStride, Width,  Height); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref,                                   int32 Area               ) { return xDistortionSSE::CalcSSD(Tst, Ref,                       Area          ); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height) { return xDistortionSSE::CalcSSD(Tst, Ref, TstStride, RefStride, Width,  Height); }

#else //X_CAN_USE_???

  static inline  int32 CalcSD (const uint16* Tst, const uint16* Ref,                                   int32 Area               ) { return xDistortionSTD::CalcSD (Tst, Ref,                       Area          ); }
  static inline  int32 CalcSD (const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height) { return xDistortionSTD::CalcSD (Tst, Ref, TstStride, RefStride, Width,  Height); }
  static inline uint32 CalcSAD(const uint16* Tst, const uint16* Ref,                                   int32 Area               ) { return xDistortionSTD::CalcSAD(Tst, Ref,                       Area          ); }
  static inline uint32 CalcSAD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height) { return xDistortionSTD::CalcSAD(Tst, Ref, TstStride, RefStride, Width,  Height); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref,                                   int32 Area               ) { return xDistortionSTD::CalcSSD(Tst, Ref,                       Area          ); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height) { return xDistortionSTD::CalcSSD(Tst, Ref, TstStride, RefStride, Width,  Height); }

#endif //X_CAN_USE_???

  static inline  int64 CalcWeightedSD (const uint16* Tst, const uint16* Ref, const uint16* Mask,                                                    int32 Area               ) { return xDistortionSTD::CalcWeightedSD (Tst, Ref, Mask,                            Area          ); }
  static inline  int64 CalcWeightedSD (const uint16* Tst, const uint16* Ref, const uint16* Mask, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height) { return xDistortionSTD::CalcWeightedSD (Tst, Ref, Mask, TstStride, RefStride, MskStride, Width,  Height); }
  static inline uint64 CalcWeightedSSD(const uint16* Tst, const uint16* Ref, const uint16* Mask,                                                    int32 Area               ) { return xDistortionSTD::CalcWeightedSSD(Tst, Ref, Mask,                            Area          ); }
  static inline uint64 CalcWeightedSSD(const uint16* Tst, const uint16* Ref, const uint16* Mask, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height) { return xDistortionSTD::CalcWeightedSSD(Tst, Ref, Mask, TstStride, RefStride, MskStride, Width,  Height); }

};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#undef X_CAN_USE_SSE
#undef X_CAN_USE_AVX
#undef X_CAN_USE_AVX512