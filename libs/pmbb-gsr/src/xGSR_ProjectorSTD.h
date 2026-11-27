/*
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefGSR.h"
#include "xProjectedCloud.h"

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================

class xProjectorSTD
{
public:
  using tFlt   = tProjInternalFloatType;
  using tFltV3 = xVec3<tFlt>;

  static constexpr tFlt C0    = (tFlt) 0.28209479177387814; // 1 / (2*sqrt(pi))
  static constexpr tFlt C1    = (tFlt) 0.4886025119029199 ;
  static constexpr tFlt C2_0  = (tFlt) 1.0925484305920792 ;
  static constexpr tFlt C2_1  = (tFlt)-1.0925484305920792 ;
  static constexpr tFlt C2_2  = (tFlt) 0.31539156525252005;
  static constexpr tFlt C2_3  = (tFlt)-1.0925484305920792 ;
  static constexpr tFlt C2_4  = (tFlt) 0.5462742152960396 ;
  static constexpr tFlt C3_0  = (tFlt)-0.5900435899266435 ;
  static constexpr tFlt C3_1  = (tFlt) 2.890611442640554  ;
  static constexpr tFlt C3_2  = (tFlt)-0.4570457994644658 ;
  static constexpr tFlt C3_3  = (tFlt) 0.3731763325901154 ;
  static constexpr tFlt C3_4  = (tFlt)-0.4570457994644658 ;
  static constexpr tFlt C3_5  = (tFlt) 1.445305721320277  ;
  static constexpr tFlt C3_6  = (tFlt)-0.5900435899266435 ;
  
public:
  static void ProjectRng(xProjectedCloud* Projected, uint8* ValidMask, const tGaussI* Cloud, const tCamPar& CamParams, int32V2 PicSize, int32 BegIdx, int32 EndIdx);

  static tFltV3 xEvalSphericalHarmonics(const tGaussI* Cloud, int32 SplatIdx, flt32V3 ViewDir); // Evaluates spherical harmonics at the given view direction. Falls back to DC-only when SphHarmDegree == 0.
  
  static tFltV3 xEvaluateAtDirection(tFlt ViewDirX, tFlt ViewDirY, tFlt ViewDirZ, int32 SphHarmDegree, const tFlt* DC_RGB, const tFlt* SH_R, const tFlt* SH_G, const tFlt* SH_B);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB::GSR
