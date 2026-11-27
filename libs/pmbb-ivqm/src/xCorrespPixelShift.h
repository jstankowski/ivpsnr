/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2025 Patrycja Kaźmierczak <patrycja.kazmierczak@student.put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVQM.h"
#include "xPic.h"

//portable implementation
#include "xCorrespPixelShiftSTD.h"

//AVX512 implementation
#if X_SIMD_CAN_USE_AVX512 && __has_include("xCorrespPixelShiftAVX512.h")
#define X_CORRESPPIXELSHIFT_CAN_USE_AVX512 1
#include "xCorrespPixelShiftAVX512.h"
#else
#define X_CORRESPPIXELSHIFT_CAN_USE_AVX512 0
#endif

//AVX implementation
#if X_SIMD_CAN_USE_AVX && __has_include("xCorrespPixelShiftAVX.h")
#define X_CORRESPPIXELSHIFT_CAN_USE_AVX 1
#include "xCorrespPixelShiftAVX.h"
#else
#define X_CORRESPPIXELSHIFT_CAN_USE_AVX 0
#endif

//SSE implementation
#if X_SIMD_CAN_USE_SSE && __has_include("xCorrespPixelShiftSSE.h")
#define X_CORRESPPIXELSHIFT_CAN_USE_SSE 1
#include "xCorrespPixelShiftSSE.h"
#else
#define X_CORRESPPIXELSHIFT_CAN_USE_SSE 0
#endif

//NEON implementation
#if X_SIMD_CAN_USE_NEON && __has_include("xCorrespPixelShiftNEON.h")
#define X_CORRESPPIXELSHIFT_CAN_USE_NEON 1
#include "xCorrespPixelShiftNEON.h"
#else
#define X_CORRESPPIXELSHIFT_CAN_USE_NEON 0
#endif

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// Corresponding Pixel Shift
//===============================================================================================================================================================================================================
class xCorrespPixelShift
{
public:
  //asymetric Q
  static inline uint64V4 CalcDistAsymmetricRow (const xPicP* Tst, const xPicP* Ref, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCorrespPixelShiftSTD::CalcDistAsymmetricRow(Tst, Ref, y, GlobalColorShift, SearchRange, CmpWeights); }
  static inline uint64V4 CalcDistAsymmetricRow (const xPicI* Tst, const xPicI* Ref, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCorrespPixelShiftSTD ::CalcDistAsymmetricRow(Tst, Ref, y, GlobalColorShift, SearchRange, CmpWeights); }
  static inline uint64V4 CalcDistAsymmetricRowM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCorrespPixelShiftSTD::CalcDistAsymmetricRowM(Tst, Ref, Msk, y, GlobalColorShift, SearchRange, CmpWeights); }
  static inline uint64V4 CalcDistAsymmetricRowM(const xPicI* Tst, const xPicI* Ref, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCorrespPixelShiftSTD::CalcDistAsymmetricRowM(Tst, Ref, Msk, y, GlobalColorShift, SearchRange, CmpWeights); }

  //shift-compensated picture generation
#if X_CORRESPPIXELSHIFT_CAN_USE_AVX512
  static void GenShftCompRowI16(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftAVX512::GenShftCompRowI16(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
  static void GenShftCompRowU32(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftAVX512::GenShftCompRowU32(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
#elif X_CORRESPPIXELSHIFT_CAN_USE_AVX
  static void GenShftCompRowI16(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftAVX::GenShftCompRowI16(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
  static void GenShftCompRowU32(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftAVX::GenShftCompRowU32(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
#elif X_CORRESPPIXELSHIFT_CAN_USE_SSE
  static void GenShftCompRowI16(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftSSE::GenShftCompRowI16(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
  static void GenShftCompRowU32(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftSSE::GenShftCompRowU32(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
#elif X_CORRESPPIXELSHIFT_CAN_USE_NEON
  static void GenShftCompRowI16(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftNEON::GenShftCompRowI16(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
  static void GenShftCompRowU32(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftNEON::GenShftCompRowU32(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
#else
  static void GenShftCompRowI16(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftSTD::GenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
  static void GenShftCompRowU32(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftSTD::GenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
#endif
  static void GenShftCompRowSTD(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftSTD::GenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }

  static void GenShftCompRow (xPicI* DstRef, const xPicI* Ref, const xPicI* Tst,                   const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftSTD::GenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
  static void GenShftCompRowM(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftSTD::GenShftCompRowM(DstRef, Ref, Tst, Msk, y, GlobalColorShift, SearchRange, CmpWeights); }
  static void GenShftCompRowM(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftSTD::GenShftCompRowM(DstRef, Ref, Tst, Msk, y, GlobalColorShift, SearchRange, CmpWeights); }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#undef X_CORRESPPIXELSHIFT_CAN_USE_AVX512
#undef X_CORRESPPIXELSHIFT_CAN_USE_AVX
#undef X_CORRESPPIXELSHIFT_CAN_USE_SSE
#undef X_CORRESPPIXELSHIFT_CAN_USE_NEON