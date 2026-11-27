/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefFVGS.h"
#include "xMatrix.h"
#include "xReprojectionUtils.h"

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================
// xReprojection
//===============================================================================================================================================================================================================

template <class fltTP> class xReprojection
{
public:
  typedef xMatrix<4,4, fltTP>  tMatrix4x4;

public:
  //Depth <--> NormalizedDepth <--> InvZ conversion
  static fltTP DepthToInvZ           (const int32 Depth, const int32 MinDepthValue, const int32 MaxDepthValue, const fltTP Znear, const fltTP Zfar);
  static int32 InvZToDepth           (const fltTP InvZ, const int32 MinDepthValue, const int32 MaxDepthValue, const fltTP Znear, const fltTP Zfar);
  static fltTP NormalizedDepthToInvZ (const fltTP NormalizedDepth, const fltTP Znear, const fltTP Zfar);
  static fltTP InvZToNormalizedDepth (const fltTP InvZ, const fltTP Znear, const fltTP Zfar);
  static fltTP DepthToNormalizedDepth(const int32 Depth, const int32 MinDepthValue, const int32 MaxDepthValue);
  static int32 NormalizedDepthToDepth(const fltTP NormalizedDepth, const int32 MinDepthValue, const int32 MaxDepthValue);

  //Recalculation of Z or InvZ for global depth (U and V in current camera, Z in center camera ----> Z in current camera)
  static fltTP ReprojectGlobalInvZ(const int32V2 SourceUV, const fltTP TargetInvZ, const tMatrix4x4& PIP);
  static fltTP ReprojectGlobalZ   (const int32V2 SourceUV, const fltTP TargetZ   , const tMatrix4x4& PIP);

  //Pixel coord reprojection using InvZ
  static flt32V2 ReprojectCoordInvZ(const flt32V2 SourceUV, const fltTP SourceInvZ, const tMatrix4x4& PIP);
  static flt32V2 ReprojectCoordInvZ(const int32V2 SourceUV, const fltTP SourceInvZ, const tMatrix4x4& PIP);
  static int32V2 ReprojectCoordInvZAndRound(const int32V2 SourceUV, const fltTP SourceInvZ, const tMatrix4x4& PIP);
  static int32V2 ReprojectCoordZAndRound   (const int32V2 SourceUV, const fltTP SourceZ   , const tMatrix4x4& PIP);

  //Pixel coord reprojection using Depth
  static int32V2 ReprojectCoord(const int32V2 InputXY, const int32 Depth, const int32 MinDepthValue, const int32 MaxDepthValue, const fltTP Znear, const fltTP Zfar, const tMatrix4x4& PIP);

  //
  static flt32V3 ReprojectCoordAndNormalizedDepth(const flt32V3 SourceUVD, const fltTP Znear, const fltTP Zfar, const tMatrix4x4& PIP);

  //Math correct
  static xVec3<fltTP> Reproject3Dto3D(xVec3<fltTP> SourceXYZ, const tMatrix4x4& RIR    );
  static xVec3<fltTP> Reproject3Dto2D(xVec3<fltTP> SourceXYZ, const tMatrix4x4& Proj   );
  static xVec3<fltTP> Reproject2Dto3D(xVec3<fltTP> SourceUVZ, const tMatrix4x4& InvProj);
  static xVec3<fltTP> Reproject2Dto2D(xVec3<fltTP> SourceUVZ, const tMatrix4x4& PIP    );
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#ifndef PMBB_xReprojection_IMPLEMENTATION
extern template class xReprojection<flt32>;
extern template class xReprojection<flt64>;
#endif // !PMBB_xReprojection_IMPLEMENTATION

//===============================================================================================================================================================================================================





//===============================================================================================================================================================================================================
// xReprojectionSystems
//===============================================================================================================================================================================================================

template <class fltTP> class xReprojectionSystems
{
  static xVec3<fltTP> ReprojectCartesian3DtoSpherical3D(const xVec3<fltTP> CartesianXYZ);
  static xVec3<fltTP> ReprojectSpherical3DtoCartesian3D(const xVec3<fltTP> SphericalRPT);
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#ifndef AVlib_xReprojection_IMPLEMENTATION
extern template class xReprojectionSystems<flt32>;
extern template class xReprojectionSystems<flt64>;
#endif // !AVlib_xCameraModel_IMPLEMENTATION

//===============================================================================================================================================================================================================

//template <class fltTP> class xReprojectionERP
//{
//  static inline floatV3 Reproject3DtoERP(xVec3<fltTP> SourceXYZ, xVec2<fltTP> SizeERF, xVec2<fltTP> Origin, xVec2<fltTP> AngleOfViewERP)
//  {
//    const fltTP X = SourceXYZ.getX();
//    const fltTP Y = SourceXYZ.getY();
//    const fltTP Z = SourceXYZ.getZ();
//
//    fltTP Radius      = sqrt(xPow2(X) + xPow2(Y) + xPow2(Z));
//    fltTP Azimuth     = atan2(Z, X); //phi
//    fltTP Inclination = asin(Y / R); //theta 
//
//    fltTP TargetU = (Azimuth - )
//
//    return { TargetU, TargetV, TargetZ };
//  }
//
//};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

