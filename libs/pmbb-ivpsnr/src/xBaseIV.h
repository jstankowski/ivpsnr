/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVPSNR.h"
#include "xPic.h"

//SSE implementation
#if X_SIMD_CAN_USE_SSE
#define X_IVPSNR_CAN_USE_SSE 1
#else
#define X_IVPSNR_CAN_USE_SSE 0
#endif

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xBaseIV
{
public:
  //IV params
  static constexpr bool    c_UseRuntimeCmpWeights = xc_USE_RUNTIME_CMPWEIGHTS;
  static constexpr int32   c_DefaultSearchRange   = 2;
  static constexpr int32V4 c_DefaultCmpWeights    = { 4, 1, 1, 0 };
  static constexpr flt32V4 c_DefaultUnntcbCoef    = { 0.01f, 0.01f, 0.01f, 0.0f };

protected:
  int32   m_SearchRange       = c_DefaultSearchRange;
  int32V4 m_CmpWeightsAverage = c_DefaultCmpWeights;
  int32V4 m_CmpWeightsSearch  = c_DefaultCmpWeights;
  flt32V4 m_CmpUnntcbCoef     = c_DefaultUnntcbCoef;

public:
  void  setSearchRange(const int32   SearchRange) { m_SearchRange       = SearchRange; }
  void  setCmpWeights (const int32V4& CmpWeights) { m_CmpWeightsAverage = CmpWeights; m_CmpWeightsSearch = CmpWeights; }
  void  setUnntcbCoef (const flt32V4& UnntcbCoef) { m_CmpUnntcbCoef     = UnntcbCoef; }

protected:
  //asymetric Q planar
  static int32V4 xCalcDistAsymmetricRow   (const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
  static int32   xFindBestPixelWithinBlock(const xPicP* Ref, const int32V4& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights);
  
  //asymetric Q interleaved
#if X_IVPSNR_CAN_USE_SSE
  static inline int32V4 xCalcDistAsymmetricRow(const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCalcDistAsymmetricRow_SSE(Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
#else //X_IVPSNR_CAN_USE_SSE
  static inline int32V4 xCalcDistAsymmetricRow(const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCalcDistAsymmetricRow_STD(Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
#endif //X_IVPSNR_CAN_USE_SSE

  //asymetric Q interleaved - STD
  static int32V4 xCalcDistAsymmetricRow_STD   (const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
  static int32   xFindBestPixelWithinBlock_STD(const xPicI* Ref, const int32V4& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights);

  //asymetric Q interleaved - SSE
#if X_IVPSNR_CAN_USE_SSE
  static int32V4 xCalcDistAsymmetricRow_SSE(const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
  static __m128i xCalcDistWithinBlock_SSE  (const xPicI* Ref, const __m128i& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const __m128i& CmpWeights);
#endif //X_IVPSNR_CAN_USE_SSE

};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

