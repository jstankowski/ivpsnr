/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefFVGS.h"
#include "xVec.h"

//SSE implementation
#if X_SIMD_CAN_USE_SSE
#define X_CAN_USE_SSE 1
#else
#define X_CAN_USE_SSE 0
#endif

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================

class xReprojectionUtils
{
protected:
  union xM4x4F
  {
    flt32  M [4][4];
#if X_CAN_USE_SSE
    __m128 R [4];
#endif //X_CAN_USE_SSE
  };

  //UV(XY) and SourceInvZ - reprojection from plane to plane
  static void xReprojectCoordInvZ_STD(flt32V2& TargetUV, const flt32V2& SourceUV, const float SourceInvZ, const xM4x4F* restrict PIP);
  static void xReprojectCoordInvZ_STD(int32V2& TargetUV, const int32V2& SourceUV, const float SourceInvZ, const xM4x4F* restrict PIP);
  static void xReprojectCoordInvZ_SSE(flt32V2& TargetUV, const flt32V2& SourceUV, const float SourceInvZ, const xM4x4F* restrict PIP);
  static void xReprojectCoordInvZ_SSE(int32V2& TargetUV, const int32V2& SourceUV, const float SourceInvZ, const xM4x4F* restrict PIP);
   
public:
#if X_USE_SSE && X_SSE_ALL
  static void ReprojectCoordInvZ(flt32V2& TargetUV, const flt32V2& SourceUV, const float SourceInvZ, const float* PIP) { xReprojectCoordInvZ_SSE(TargetUV, SourceUV, SourceInvZ, (xM4x4F*)PIP); }
  static void ReprojectCoordInvZ(int32V2& TargetUV, const int32V2& SourceUV, const float SourceInvZ, const float* PIP) { xReprojectCoordInvZ_SSE(TargetUV, SourceUV, SourceInvZ, (xM4x4F*)PIP); }
#else
  static void ReprojectCoordInvZ(flt32V2& TargetUV, const flt32V2& SourceUV, const float SourceInvZ, const float* PIP) { xReprojectCoordInvZ_STD(TargetUV, SourceUV, SourceInvZ, (xM4x4F*)PIP); }
  static void ReprojectCoordInvZ(int32V2& TargetUV, const int32V2& SourceUV, const float SourceInvZ, const float* PIP) { xReprojectCoordInvZ_STD(TargetUV, SourceUV, SourceInvZ, (xM4x4F*)PIP); }
#endif
};

//===============================================================================================================================================================================================================

#ifndef PMBB_xReprojectionUtils_IMPLEMENTATION
#undef X_CAN_USE_SSE
#endif //PMBB_xReprojectionUtils_IMPLEMENTATION


} //end of namespace PMBB

