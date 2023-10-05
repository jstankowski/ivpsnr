/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVPSNR.h"
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

class xCorrespPixelShift
{
//IV params & setup ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
public:
  static constexpr bool    c_UseRuntimeCmpWeights = xc_USE_RUNTIME_CMPWEIGHTS;
  static constexpr int32   c_DefaultSearchRange   = 2;
  static constexpr int32V4 c_DefaultCmpWeights    = { 4, 1, 1, 0 };

protected:
  int32   m_SearchRange       = c_DefaultSearchRange;
  int32V4 m_CmpWeightsAverage = c_DefaultCmpWeights;
  int32V4 m_CmpWeightsSearch  = c_DefaultCmpWeights;

public:
  void  setSearchRange(const int32   SearchRange) { m_SearchRange       = SearchRange; }
  void  setCmpWeights (const int32V4& CmpWeights) { m_CmpWeightsAverage = CmpWeights; m_CmpWeightsSearch = CmpWeights; }

//IV routines ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
protected:
  //asymetric Q planar
  static int32V4 xCalcDistAsymmetricRow   (const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
  static int32   xFindBestPixelWithinBlock(const xPicP* Ref, const int32V4& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights);
  
  //asymetric Q interleaved
#if X_CORRESPPIXELSHIFT_CAN_USE_SSE
  static inline int32V4 xCalcDistAsymmetricRow(const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCorrespPixelShiftSSE::CalcDistAsymmetricRow(Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
#else //X_CORRESPPIXELSHIFT_CAN_USE_SSE
  static inline int32V4 xCalcDistAsymmetricRow(const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCorrespPixelShiftSTD::CalcDistAsymmetricRow(Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
#endif //X_CORRESPPIXELSHIFT_CAN_USE_SSE

  //asymetric Q interleaved - with mask
  static inline uint64V4 xCalcDistAsymmetricRowM(const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCorrespPixelShiftSTD::CalcDistAsymmetricRowM(Ref, Tst, Msk, y, GlobalColorShift, SearchRange, CmpWeights); }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
