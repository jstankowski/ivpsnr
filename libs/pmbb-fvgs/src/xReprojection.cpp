/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define PMBB_xReprojection_IMPLEMENTATION
#include "xReprojection.h"
#include "xHelpersFLT.h"

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================
// special rounding routines
//===============================================================================================================================================================================================================
template <class XXX> static inline int32V2 xRoundVecFltToI32(const xVec2<XXX>& FltV);
template <> inline int32V2 xRoundVecFltToI32(const xVec2<flt32>& FltV) { return { xRoundF32ToI32(FltV[0]), xRoundF32ToI32(FltV[1])}; }
template <> inline int32V2 xRoundVecFltToI32(const xVec2<flt64>& FltV) { return { xRoundF64ToI32(FltV[0]), xRoundF64ToI32(FltV[1])}; }

//===============================================================================================================================================================================================================
// xReprojection
//===============================================================================================================================================================================================================

//Depth <--> NormalizedDepth <--> InvZ conversion
template <class fltTP> fltTP xReprojection<fltTP>::DepthToInvZ(const int32 Depth, const int32 MinDepthValue, const int32 MaxDepthValue, const fltTP Znear, const fltTP Zfar)
{
  int32 DepthRange                        = (MaxDepthValue - MinDepthValue);
  fltTP InvZnear                          = (fltTP)1.0 / Znear;
  fltTP InvZfar                           = (fltTP)1.0 / Zfar;
  fltTP InvZnearMinusInvZfarDivDepthRange = (InvZnear - InvZfar) / (fltTP)(DepthRange);
  fltTP InvZ                              = ((fltTP)(Depth - MinDepthValue)) * InvZnearMinusInvZfarDivDepthRange + InvZfar;
  return InvZ;
}
template <class fltTP> int32 xReprojection<fltTP>::InvZToDepth(const fltTP InvZ, const int32 MinDepthValue, const int32 MaxDepthValue, const fltTP Znear, const fltTP Zfar)
{
  int32 DepthRange                        = (MaxDepthValue - MinDepthValue);
  fltTP InvZnear                          = (fltTP)1.0 / Znear;
  fltTP InvZfar                           = (fltTP)1.0 / Zfar;
  fltTP DepthRangeDivInvZnearMinusInvZfar = (fltTP)(DepthRange) / (InvZnear - InvZfar);
  int32 Depth                             = xRoundFltToI32((InvZ - InvZfar) * DepthRangeDivInvZnearMinusInvZfar + MinDepthValue);
  return Depth;
}
template <class fltTP> fltTP xReprojection<fltTP>::NormalizedDepthToInvZ(const fltTP NormalizedDepth, const fltTP Znear, const fltTP Zfar)
{
  fltTP InvZnear                = (fltTP)1.0 / Znear;
  fltTP InvZfar                 = (fltTP)1.0 / Zfar;
  fltTP InvZnearMinusInvZfar    = (InvZnear - InvZfar);
  fltTP InvZ                    = NormalizedDepth * InvZnearMinusInvZfar + InvZfar;
  return InvZ;
}
template <class fltTP> fltTP xReprojection<fltTP>::InvZToNormalizedDepth(const fltTP InvZ, const fltTP Znear, const fltTP Zfar)
{
  fltTP InvZnear                = (fltTP)1.0 / Znear;
  fltTP InvZfar                 = (fltTP)1.0 / Zfar;
  fltTP InvInvZnearMinusInvZfar = (fltTP)1.0 / (InvZnear - InvZfar);
  fltTP NormalizedDepth         = (InvZ - InvZfar) * InvInvZnearMinusInvZfar;
  return NormalizedDepth;
}
template <class fltTP> fltTP xReprojection<fltTP>::DepthToNormalizedDepth(const int32 Depth, const int32 MinDepthValue, const int32 MaxDepthValue)
{
  fltTP DepthRange      = (fltTP)(MaxDepthValue - MinDepthValue);
  fltTP NormalizedDepth = ((fltTP)(Depth - MinDepthValue)) / (fltTP)DepthRange;
  return NormalizedDepth;
}
template <class fltTP> int32 xReprojection<fltTP>::NormalizedDepthToDepth(const fltTP NormalizedDepth, const int32 MinDepthValue, const int32 MaxDepthValue)
{
  fltTP DepthRange      = (fltTP)(MaxDepthValue - MinDepthValue);
  int32 Depth           = xRoundFltToI32(NormalizedDepth * DepthRange) + MinDepthValue;
  return Depth;
}


//Recalculation of Z or InvZ for global depth (U and V in current camera, Z in center camera ----> Z in current camera)
template <class fltTP>  fltTP xReprojection<fltTP>::ReprojectGlobalInvZ(const int32V2 SourceUV, const fltTP TargetInvZ, const tMatrix4x4& PIP)
{
  const fltTP SourceU = (fltTP)SourceUV.getX();
  const fltTP SourceV = (fltTP)SourceUV.getY();

  fltTP TargetZ    = (fltTP)1.0 / TargetInvZ;
  fltTP SourceInvZ = (SourceU * PIP[2][0] + SourceV * PIP[2][1] + PIP[2][2]) / (TargetZ - PIP[2][3]);
  return SourceInvZ;
}
template <class fltTP>  fltTP xReprojection<fltTP>::ReprojectGlobalZ(const int32V2 SourceUV, const fltTP TargetZ, const tMatrix4x4& PIP)
{
  const fltTP SourceU = (fltTP)SourceUV.getX();
  const fltTP SourceV = (fltTP)SourceUV.getY();

  fltTP SourceZ = (TargetZ - PIP[2][3]) / (SourceU * PIP[2][0] + SourceV * PIP[2][1] + PIP[2][2]);
  return SourceZ;
}

//Pixel coord reprojection using InvZ
template <class fltTP>  flt32V2 xReprojection<fltTP>::ReprojectCoordInvZ(const flt32V2 SourceUV, const fltTP SourceInvZ, const tMatrix4x4& PIP)
{
  flt32V2 TargetUV;
  if constexpr(std::is_same_v<fltTP, float>) { xReprojectionUtils::ReprojectCoordInvZ(TargetUV, SourceUV, SourceInvZ, PIP[0].data()); }
  else
  {
    const fltTP SourceU = (fltTP)SourceUV.getX();
    const fltTP SourceV = (fltTP)SourceUV.getY();

    fltTP RowX    = SourceU * PIP[0][0] + SourceV * PIP[0][1] + PIP[0][2] + SourceInvZ * PIP[0][3];
    fltTP RowY    = SourceU * PIP[1][0] + SourceV * PIP[1][1] + PIP[1][2] + SourceInvZ * PIP[1][3];
    fltTP RowZ    = SourceU * PIP[2][0] + SourceV * PIP[2][1] + PIP[2][2] + SourceInvZ * PIP[2][3];

    fltTP InvRovZ = (fltTP)1.0 / RowZ;
    fltTP TargetU = RowX * InvRovZ;
    fltTP TargetV = RowY * InvRovZ;    
    TargetUV = { (float)TargetU, (float)TargetV };
  }
  return TargetUV;
}
template <class fltTP> flt32V2 xReprojection<fltTP>::ReprojectCoordInvZ(const int32V2 SourceUV, const fltTP SourceInvZ, const tMatrix4x4& PIP)
{
  flt32V2 TargetUV;
  if constexpr(std::is_same_v<fltTP, float>) { xReprojectionUtils::ReprojectCoordInvZ(TargetUV, (flt32V2)SourceUV, SourceInvZ, PIP[0].data()); }
  else                                            { TargetUV = ReprojectCoordInvZ((flt32V2)SourceUV, SourceInvZ, PIP); }
  return TargetUV;
}
template <class fltTP> int32V2 xReprojection<fltTP>::ReprojectCoordInvZAndRound(const int32V2 SourceUV, const fltTP SourceInvZ, const tMatrix4x4& PIP)
{
  int32V2 TargetUV;
  if constexpr(std::is_same_v<fltTP, float>) { xReprojectionUtils::ReprojectCoordInvZ(TargetUV, SourceUV, SourceInvZ, PIP[0].data()); }
  else                                       { TargetUV = xRoundVecFltToI32(ReprojectCoordInvZ((flt32V2)SourceUV, SourceInvZ, PIP)); }
  return TargetUV;
}
template <class fltTP> int32V2 xReprojection<fltTP>::ReprojectCoordZAndRound(const int32V2 SourceUV, const fltTP SourceZ, const tMatrix4x4& PIP)
{
  return ReprojectCoordInvZAndRound(SourceUV, (fltTP)1.0 / SourceZ, PIP);
}

//Pixel coord reprojection using Depth
template <class fltTP> int32V2 xReprojection<fltTP>::ReprojectCoord(const int32V2 InputXY, const int32 Depth, const int32 MinDepthValue, const int32 MaxDepthValue, const fltTP Znear, const fltTP Zfar, const tMatrix4x4& PIP)
{ 
  fltTP InvZ = DepthToInvZ(Depth, MinDepthValue, MaxDepthValue, Znear, Zfar);
  return xRoundVecFltToI32(ReprojectCoordInvZ(InputXY, InvZ, PIP));
}

//
template <class fltTP> flt32V3 xReprojection<fltTP>::ReprojectCoordAndNormalizedDepth(const flt32V3 SourceUVD, const fltTP Znear, const fltTP Zfar, const tMatrix4x4& PIP)
{
  const fltTP SourceU  = (fltTP)SourceUVD.getX();
  const fltTP SourceV  = (fltTP)SourceUVD.getY();
  const fltTP SourceD  = (fltTP)SourceUVD.getZ();

  const fltTP InvZnear                = (fltTP)1.0 / Znear;
  const fltTP InvZfar                 = (fltTP)1.0 / Zfar;
  const fltTP InvZnearMinusInvZfar    = InvZnear - InvZfar;
  const fltTP InvInvZnearMinusInvZfar = (fltTP)1.0 / InvZnearMinusInvZfar;
  const fltTP SourceInvZ              = SourceD * InvZnearMinusInvZfar + InvZfar;

  fltTP RowX    = SourceU * PIP[0][0] + SourceV * PIP[0][1] + PIP[0][2] + SourceInvZ * PIP[0][3];
  fltTP RowY    = SourceU * PIP[1][0] + SourceV * PIP[1][1] + PIP[1][2] + SourceInvZ * PIP[1][3];
  fltTP RowZ    = SourceU * PIP[2][0] + SourceV * PIP[2][1] + PIP[2][2] + SourceInvZ * PIP[2][3];

  fltTP InvRovZ = (fltTP)1.0 / RowZ;
  fltTP TargetU = RowX * InvRovZ;
  fltTP TargetV = RowY * InvRovZ;  
  fltTP TargetD = InvInvZnearMinusInvZfar * (SourceInvZ * InvRovZ - InvZfar);
  return { (flt32)TargetU, (flt32)TargetV, (flt32)TargetD };
}

//Math correct
template <class fltTP>  xVec3<fltTP> xReprojection<fltTP>::Reproject3Dto3D(xVec3<fltTP> SourceXYZ, const tMatrix4x4& RIR)
{
  const fltTP SourceX = SourceXYZ.getX();
  const fltTP SourceY = SourceXYZ.getY();
  const fltTP SourceZ = SourceXYZ.getZ();

  const fltTP TargetX = SourceX * RIR[0][0] + SourceY * RIR[0][1] + SourceZ * RIR[0][2] + RIR[0][3];
  const fltTP TargetY = SourceX * RIR[1][0] + SourceY * RIR[1][1] + SourceZ * RIR[1][2] + RIR[1][3];
  const fltTP TargetZ = SourceX * RIR[2][0] + SourceY * RIR[2][1] + SourceZ * RIR[2][2] + RIR[2][3];

  return { TargetX, TargetY, TargetZ };
}
template <class fltTP>  xVec3<fltTP> xReprojection<fltTP>::Reproject3Dto2D(xVec3<fltTP> SourceXYZ, const tMatrix4x4& Proj)
{
  const fltTP SourceX = SourceXYZ.getX();
  const fltTP SourceY = SourceXYZ.getY();
  const fltTP SourceZ = SourceXYZ.getZ();

  const fltTP TargetX = SourceX * Proj[0][0] + SourceY * Proj[0][1] + SourceZ * Proj[0][2] + Proj[0][3];
  const fltTP TargetY = SourceX * Proj[1][0] + SourceY * Proj[1][1] + SourceZ * Proj[1][2] + Proj[1][3];
  const fltTP TargetZ = SourceX * Proj[2][0] + SourceY * Proj[2][1] + SourceZ * Proj[2][2] + Proj[2][3];

  const fltTP TargetU = TargetX / TargetZ;
  const fltTP TargetV = TargetY / TargetZ;

  return { TargetU, TargetV, TargetZ };
}
template <class fltTP>  xVec3<fltTP> xReprojection<fltTP>::Reproject2Dto3D(xVec3<fltTP> SourceUVZ, const tMatrix4x4& InvProj)
{
  const fltTP SourceZ = SourceUVZ.getZ();
  const fltTP SourceX = SourceUVZ.getX() * SourceZ;
  const fltTP SourceY = SourceUVZ.getY() * SourceZ;
  
  const fltTP TargetX = SourceX * InvProj[0][0] + SourceY * InvProj[0][1] + SourceZ * InvProj[0][2] + InvProj[0][3];
  const fltTP TargetY = SourceX * InvProj[1][0] + SourceY * InvProj[1][1] + SourceZ * InvProj[1][2] + InvProj[1][3];
  const fltTP TargetZ = SourceX * InvProj[2][0] + SourceY * InvProj[2][1] + SourceZ * InvProj[2][2] + InvProj[2][3];

  return { TargetX, TargetY, TargetZ };
}
template <class fltTP>  xVec3<fltTP> xReprojection<fltTP>::Reproject2Dto2D(xVec3<fltTP> SourceUVZ, const tMatrix4x4& PIP)
{
  const fltTP SourceZ = SourceUVZ.getZ();;
  const fltTP SourceX = SourceUVZ.getX() * SourceZ;
  const fltTP SourceY = SourceUVZ.getY() * SourceZ;

  const fltTP TargetX = SourceX * PIP[0][0] + SourceY * PIP[0][1] + SourceZ * PIP[0][2] + PIP[0][3];
  const fltTP TargetY = SourceX * PIP[1][0] + SourceY * PIP[1][1] + SourceZ * PIP[1][2] + PIP[1][3];
  const fltTP TargetZ = SourceX * PIP[2][0] + SourceY * PIP[2][1] + SourceZ * PIP[2][2] + PIP[2][3];

  const fltTP TargetU = TargetX / TargetZ;
  const fltTP TargetV = TargetY / TargetZ;

  return { TargetU, TargetV, TargetZ };
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template class xReprojection<flt32>;
template class xReprojection<flt64>;

//===============================================================================================================================================================================================================




//===============================================================================================================================================================================================================
// xReprojectionSystems
//===============================================================================================================================================================================================================

template <class fltTP> xVec3<fltTP> xReprojectionSystems<fltTP>::ReprojectCartesian3DtoSpherical3D(const xVec3<fltTP> CartesianXYZ)
{
  const fltTP X = CartesianXYZ.getX();
  const fltTP Y = CartesianXYZ.getY();
  const fltTP Z = CartesianXYZ.getZ();

  fltTP Radius      = sqrt(xPow2(X) + xPow2(Y) + xPow2(Z));
  fltTP Azimuth     = atan2(Z, X); //phi
  fltTP Inclination = asin(Y / Radius); //theta 

  return { Radius, Azimuth, Inclination };
}
template <class fltTP> xVec3<fltTP> xReprojectionSystems<fltTP>::ReprojectSpherical3DtoCartesian3D(const xVec3<fltTP> SphericalRPT)
{
  const fltTP Radius      = SphericalRPT[0];
  const fltTP Azimuth     = SphericalRPT[1]; //phi
  const fltTP Inclination = SphericalRPT[2]; //theta
      
  fltTP CartesianX = Radius * cos(Inclination) * cos(Azimuth);
  fltTP CartesianY = Radius * sin(Inclination);
  fltTP CartesianZ = Radius * cos(Inclination) * sin(Azimuth);

  return { CartesianX, CartesianY, CartesianZ };
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template class xReprojectionSystems<flt32>;
template class xReprojectionSystems<flt64>;

//===============================================================================================================================================================================================================

} //end of namespace PMBB
