/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVQM.h"
#include "xPic.h"

//portable implementation
#include "xCorrespPixelShiftSTD.h"

//SSE implementation
#if X_SIMD_CAN_USE_SSE && __has_include("xCorrespPixelShiftSSE.h")
#define X_CORRESPPIXELSHIFT_CAN_USE_SSE 1
#include "xCorrespPixelShiftSSE.h"
#else
#define X_CORRESPPIXELSHIFT_CAN_USE_SSE 0
#endif

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// Corresponding Pixel Shift
//===============================================================================================================================================================================================================
class xCorrespPixelShift
{
public:
  //asymetric Q planar
  static uint64V4 CalcDistAsymmetricRow   (const xPicP* Tst, const xPicP* Ref, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCorrespPixelShiftSTD::CalcDistAsymmetricRow(Tst, Ref, y, GlobalColorShift, SearchRange, CmpWeights); }
  
  //asymetric Q interleaved
#if X_CORRESPPIXELSHIFT_CAN_USE_SSE
  static inline uint64V4 CalcDistAsymmetricRow(const xPicI* Tst, const xPicI* Ref, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCorrespPixelShiftSSE ::CalcDistAsymmetricRow(Tst, Ref, y, GlobalColorShift, SearchRange, CmpWeights); }
#else //X_CORRESPPIXELSHIFT_CAN_USE_SSE
  static inline uint64V4 CalcDistAsymmetricRow(const xPicI* Tst, const xPicI* Ref, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCorrespPixelShiftSTD ::CalcDistAsymmetricRow(Tst, Ref, y, GlobalColorShift, SearchRange, CmpWeights); }
#endif //X_CORRESPPIXELSHIFT_CAN_USE_SSE

  //asymetric Q interleaved - with mask
  static inline uint64V4 CalcDistAsymmetricRowM(const xPicI* Tst, const xPicI* Ref, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCorrespPixelShiftSTD::CalcDistAsymmetricRowM(Tst, Ref, Msk, y, GlobalColorShift, SearchRange, CmpWeights); }

  //shift-compensated picture generation
  static void GenShftCompRow(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftSTD::GenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
#if X_CORRESPPIXELSHIFT_CAN_USE_SSE
  static void GenShftCompRow(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftSSE::GenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
#else //X_CORRESPPIXELSHIFT_CAN_USE_SSE
  static void GenShftCompRow(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftSTD::GenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
#endif //X_CORRESPPIXELSHIFT_CAN_USE_SSE

  //shift-compensated picture generation - with mask
  static void GenShftCompRowM(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { xCorrespPixelShiftSTD::GenShftCompRowM(DstRef, Ref, Tst, Msk, y, GlobalColorShift, SearchRange, CmpWeights); }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
