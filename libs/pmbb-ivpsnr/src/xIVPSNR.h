/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xWSPSNR.h"
#include "xBaseIV.h"

//SSE implementation
#if X_SIMD_CAN_USE_SSE
#define X_IVPSNR_CAN_USE_SSE 1
#else
#define X_IVPSNR_CAN_USE_SSE 0
#endif

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xIVPSNR : public xWSPSNR, public xBaseIV
{
public:
  //debug calback types
  using tDCfGCS = std::function<void(const int32V4&)>;
  using tDCfQAP = std::function<void(flt64, flt64)>;

protected:
  tDCfGCS m_DebugCallbackGCS;
  tDCfQAP m_DebugCallbackQAP;

public:
  void  setDebugCallbackGCS(tDCfGCS DebugCallbackGCS) { m_DebugCallbackGCS = DebugCallbackGCS; }
  void  setDebugCallbackQAP(tDCfQAP DebugCallbackQAP) { m_DebugCallbackQAP = DebugCallbackQAP; }

  flt64 calcPicIVPSNR  (const xPicP* Ref, const xPicP* Tst, const xPicI* RefI = nullptr, const xPicI* TstI = nullptr);

protected:
  //asymetric Q planar
  flt64          xCalcQualAsymmetricPic   (const xPicP* Ref, const xPicP* Tst, const int32V4& GlobalColorShift);
  //asymetric Q interleaved
  flt64          xCalcQualAsymmetricPic   (const xPicI* Ref, const xPicI* Tst, const int32V4& GlobalColorShift);
};

//===============================================================================================================================================================================================================

class xIVPSNRM : public xIVPSNR
{
public:
  flt64 calcPicIVPSNRM (const xPicP* Ref, const xPicP* Tst, const xPicP* Mask, const xPicI* RefI, const xPicI* TstI);

protected:

  //asymetric Q interleaved
  flt64                  xCalcQualAsymmetricPicM(const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32V4& GlobalColorShift, const int32 NumNonMasked);
  static inline uint64V4 xCalcDistAsymmetricRowM(const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights) { return xCalcDistAsymmetricRowM_STD(Ref, Tst, Msk, y, GlobalColorShift, SearchRange, CmpWeights); }

  //asymetric Q interleaved - STD
  static uint64V4 xCalcDistAsymmetricRowM_STD   (const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
  static int32    xFindBestPixelWithinBlockM_STD(const xPicI* Ref, const int32V4& TstPel, const xPicP* Msk, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#undef X_CAN_USE_SSE