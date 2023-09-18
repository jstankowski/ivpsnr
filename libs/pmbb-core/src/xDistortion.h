/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefPMBB-CORE.h"
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

  static inline  int32 CalcSD (const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xDistortionAVX512::CalcSD (Org, Dist,                   Area          ); }
  static inline  int32 CalcSD (const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xDistortionAVX512::CalcSD (Org, Dist, OStride, DStride, Width,  Height); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xDistortionAVX512::CalcSSD(Org, Dist,                   Area          ); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xDistortionAVX512::CalcSSD(Org, Dist, OStride, DStride, Width,  Height); }

#elif X_CAN_USE_AVX

  static inline  int32 CalcSD (const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xDistortionAVX::CalcSD (Org, Dist,                   Area          ); }
  static inline  int32 CalcSD (const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xDistortionAVX::CalcSD (Org, Dist, OStride, DStride, Width,  Height); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xDistortionAVX::CalcSSD(Org, Dist,                   Area          ); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xDistortionAVX::CalcSSD(Org, Dist, OStride, DStride, Width,  Height); }

#elif X_CAN_USE_SSE

  static inline  int32 CalcSD (const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xDistortionSSE::CalcSD (Org, Dist,                   Area          ); }
  static inline  int32 CalcSD (const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xDistortionSSE::CalcSD (Org, Dist, OStride, DStride, Width,  Height); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xDistortionSSE::CalcSSD(Org, Dist,                   Area          ); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xDistortionSSE::CalcSSD(Org, Dist, OStride, DStride, Width,  Height); }

#else //X_CAN_USE_???

  static inline  int32 CalcSD (const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xDistortionSTD::CalcSD (Org, Dist,                   Area          ); }
  static inline  int32 CalcSD (const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xDistortionSTD::CalcSD (Org, Dist, OStride, DStride, Width,  Height); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xDistortionSTD::CalcSSD(Org, Dist,                   Area          ); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xDistortionSTD::CalcSSD(Org, Dist, OStride, DStride, Width,  Height); }

#endif //X_CAN_USE_???

  static inline  int64 CalcWeightedSD (const uint16* Org, const uint16* Dist, const uint16* Mask,                                              int32 Area               ) { return xDistortionSTD::CalcWeightedSD (Org, Dist, Mask,                            Area          ); }
  static inline  int64 CalcWeightedSD (const uint16* Org, const uint16* Dist, const uint16* Mask, int32 OStride, int32 DStride, int32 MStride, int32 Width, int32 Height) { return xDistortionSTD::CalcWeightedSD (Org, Dist, Mask, OStride, DStride, MStride, Width,  Height); }
  static inline uint64 CalcWeightedSSD(const uint16* Org, const uint16* Dist, const uint16* Mask,                                              int32 Area               ) { return xDistortionSTD::CalcWeightedSSD(Org, Dist, Mask,                            Area          ); }
  static inline uint64 CalcWeightedSSD(const uint16* Org, const uint16* Dist, const uint16* Mask, int32 OStride, int32 DStride, int32 MStride, int32 Width, int32 Height) { return xDistortionSTD::CalcWeightedSSD(Org, Dist, Mask, OStride, DStride, MStride, Width,  Height); }

};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#undef X_CAN_USE_SSE
#undef X_CAN_USE_AVX
#undef X_CAN_USE_AVX512