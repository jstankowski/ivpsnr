/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define PMBB_xCameraParams_IMPLEMENTATION
#include "xCameraParams.h"
#include "xFile.h"
#include "xFmtScn.h"

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================

template <class MatrixType> xCameraParams<MatrixType>::xCameraParams()
{
  m_Idx = NOT_VALID;
}
template <class MatrixType> void xCameraParams<MatrixType>::reset()
{
  m_Idx        = NOT_VALID;
  
  m_CamName.clear();
  m_Resolution = { NOT_VALID, NOT_VALID };
  m_ProjType   = eProjType::INVALID;

  m_Intrinsics.zero();
  m_Extrinsics.zero();
  m_Distortion.zero();

  m_Provenance = eProvenance::INVALID;
  m_Location   = xMakeVec2(std::numeric_limits<int32>::max());

  m_Znear    = (MatrixType)0;
  m_Zfar     = (MatrixType)0;

  m_AngleOfView.zero();

  m_ExtrinsicsLocal.zero();;
  m_Projection     .zero();;    
  m_InvProjection  .zero();; 
}
template <class MatrixType> std::string xCameraParams<MatrixType>::print(const std::string& Prefix) const
{
  const std::string MatrixPrefix = Prefix + Prefix;
  std::string Result;
  Result.reserve(4096);
  Result += Prefix + fmt::sprintf("Name      : %s\n"       , m_CamName);
  Result += Prefix + fmt::sprintf("Identifier: %d\n"       , m_Idx);
  Result += Prefix + fmt::sprintf("Resolution: Y=%d,X=%d\n", m_Resolution.getY(), m_Resolution.getX());
  Result += Prefix + fmt::sprintf("Projection: %s\n"       , xProjTypeToString(m_ProjType));
  Result += Prefix + "Intrinsic  :\n" + m_Intrinsics.print(MatrixPrefix, MatrixPrefix, "  ");
  Result += Prefix + "Extrinsic  :\n" + m_Extrinsics.print(MatrixPrefix, MatrixPrefix, "  ");
  Result += Prefix + "Distortion:\n" + m_Distortion.print("  ");
  Result += Prefix + fmt::sprintf("Provenance: %s\n", xProvenanceToString(m_Provenance));
  Result += Prefix + fmt::sprintf("Location  : Y=%d,X=%d\n", m_Location.getY(), m_Location.getX());
  Result += Prefix + fmt::sprintf("Znear     : %14.8f\n"   , m_Znear);
  Result += Prefix + fmt::sprintf("Zfar      : %14.8f\n"   , m_Zfar );
  Result += Prefix + "LocalExtMat:\n" + m_ExtrinsicsLocal.print(MatrixPrefix, MatrixPrefix, "  ");
  Result += Prefix + "ProjMat    :\n" + m_Projection     .print(MatrixPrefix, MatrixPrefix, "  ");
  Result += Prefix + "InvProjMat :\n" + m_InvProjection  .print(MatrixPrefix, MatrixPrefix, "  ");
  return Result;
}
template <class MatrixType> void xCameraParams<MatrixType>::calculateLocalExtrinsics()
{
  {
    tMatrix3x3 Rotation;
    tMatrix3x1 Translation;

    m_ExtrinsicsLocal.setIdentity();
    for(uint32 h = 0; h < 3; h++)
    {
      for(uint32 w = 0; w < 3; w++)
      {
        m_ExtrinsicsLocal[h][w] = m_Extrinsics[h][w];
        Rotation         [h][w] = m_Extrinsics[h][w];
      }
      Translation[h][0] = m_Extrinsics[h][3];
    }

    tMatrix3x1 RT = tMatrix3x1::MultiplyMatrixByMatrix(Rotation, Translation);
    for(uint32 h = 0; h < 3; h++) { m_ExtrinsicsLocal[h][3] = -RT[h][0]; }
  }

  //to test
  {
    //m_ExtrinsicsLocal = m_Extrinsics;
    //xRotationMat R       = m_Extrinsics.getRotationMatrix();
    //xTranslation T       = m_Extrinsics.getTranslation();
    //xTranslation MinusRT = xTranslation::MultiplyMatrixByMatrix(R, T).MultiplyByScalar((MatrixType)(-1));
    //m_ExtrinsicsLocal.setTranslation(MinusRT);
  }

  m_InvExtrinsicsLocal.setInvertion(m_ExtrinsicsLocal);
}
template <class MatrixType> void xCameraParams<MatrixType>::calculateProjectionMats()
{
  m_Projection   .setMultiplyMatrixByMatrix(m_Intrinsics, m_ExtrinsicsLocal);
  m_InvProjection.setInvertion(m_Projection);
}
template <class MatrixType> xCameraParams<MatrixType>* xCameraParams<MatrixType>::readParamsFromString(std::string CamParamsFileContent, std::string ZnearZfarFileContent, std::string CamName, MatrixType DefaultZnear, MatrixType DefaultZfar)
{
  std::string::size_type FoundCamParamsPosition = CamParamsFileContent.find(CamName);
  std::string::size_type FoundZnearZfarPosition = ZnearZfarFileContent.find(CamName);

  xCameraParams<MatrixType>* NewCamParams = nullptr;

  if(FoundCamParamsPosition != std::string::npos)
  {
    NewCamParams = new xCameraParams<MatrixType>;
    readCamParams(NewCamParams, CamParamsFileContent.substr(FoundCamParamsPosition, std::string::npos));

    if(FoundZnearZfarPosition != std::string::npos)
    { 
      readZnearZfar(NewCamParams, ZnearZfarFileContent.substr(FoundZnearZfarPosition, std::string::npos));
    }
    else
    {
      NewCamParams->setZnear(DefaultZnear);
      NewCamParams->setZfar (DefaultZfar );
    }
  }

  return NewCamParams;
}
template <class MatrixType> void xCameraParams<MatrixType>::readCamParams(xCameraParams<MatrixType>* CamParams, std::string CfgCamParams)
{
  std::istringstream Input(CfgCamParams);

  //camera name
  std::string CamName;
  Input >> CamName;
  CamParams->setCamName(CamName);

  //intrinsics
  tMatrix4x4& IntrinsicsMat = CamParams->getIntrinsics();
  IntrinsicsMat.zero();
  for(int32 h = 0; h < 3; h++) { for(int32 w = 0; w < 3; w++) { Input >> IntrinsicsMat[h][w]; } }
  IntrinsicsMat[3][3] = (MatrixType)1;

  //distortion ??
  MatrixType Coeff;
  Input >> Coeff;
  Input >> Coeff;

  //extrinsics
  tMatrix4x4& ExtrinsicsMat = CamParams->getExtrinsics();
  ExtrinsicsMat.zero();
  for(int32 h = 0; h < 3; h++) { for(int32 w = 0; w < 4; w++) { Input >> ExtrinsicsMat[h][w]; } }
  ExtrinsicsMat[3][3] = (MatrixType)1;

  CamParams->calculateDerrived();
}
template <class MatrixType> void xCameraParams<MatrixType>::readZnearZfar(xCameraParams<MatrixType>* CamParams, std::string CfgZnearZfar)
{
  std::istringstream Input(CfgZnearZfar);

  //camera name
  std::string CamName;
  Input >> CamName;
  CamParams->setCamName(CamName);

  MatrixType Znear, Zfar;
  Input >> Znear;
  Input >> Zfar;

  CamParams->setZnear(Znear);
  CamParams->setZfar (Zfar );
}
template <class MatrixType> bool xCameraParams<MatrixType>::readFromConf(const xCfgINI::xSection& CfgSection)
{
  //name
  m_CamName = CfgSection.getName();

  //resolution
  const std::string& Resolution = CfgSection.getParam1stArg("Resolution", std::string());
  m_Resolution = xFmtScn::scanResolution(Resolution);
  if(m_Resolution.getX() == NOT_VALID || m_Resolution.getY() == NOT_VALID) { return false; }

  //projection type
  const std::string& Projection = CfgSection.getParam1stArg("Projection", std::string());
  m_ProjType = xStrToProjType(Projection);
  if(m_ProjType == eProjType::INVALID) { return false; }

  //intrinsic
  const std::vector<MatrixType>& IntrinsicVec = CfgSection.getParamArgs("Intrinsic", (MatrixType)NOT_VALID);
  if(IntrinsicVec.size() != 9) { return false; }
  m_Intrinsics.zero();
  m_Intrinsics.load(IntrinsicVec.data(), 3, 3, 3);
  m_Intrinsics[3][3] = (MatrixType)1;

  //extrinsic
  const std::vector<MatrixType>& ExtrinsicVec = CfgSection.getParamArgs("Extrinsic", (MatrixType)NOT_VALID);
  if(ExtrinsicVec.size() != 12) { return false; }
  m_Extrinsics.zero();
  m_Extrinsics.load(ExtrinsicVec.data(), 3, 4, 4);
  m_Extrinsics[3][3] = (MatrixType)1;

  //distortion
  if(CfgSection.findParam("Distortion"))
  {
    const std::vector<MatrixType>& DistortionVec = CfgSection.getParamArgs("Distortion", (MatrixType)NOT_VALID);
    if(DistortionVec.size() == 0) { return false; }
    m_Distortion.zero();
    m_Distortion.importOpenCV(DistortionVec);
  }

  //Provenance
  if(CfgSection.findParam("Provenance"))
  {
    eProvenance Provenance = xStrToProvenance(CfgSection.getParam1stArg("Provenance", std::string("")));
    if(Provenance == eProvenance::INVALID) { return false; }
    m_Provenance = Provenance;
  }

  //location
  if(CfgSection.findParam("Location"))
  {
    const std::vector<int32>& TmpLocation = CfgSection.getParamArgs("Location", std::numeric_limits<int32>::max());
    if(TmpLocation.size() == 2) { m_Location = TmpLocation; }
    else                        { return false; }
  }

  //ZnearZfar
  m_Znear = CfgSection.getParam1stArg("Znear", std::numeric_limits<MatrixType>::max());
  m_Zfar  = CfgSection.getParam1stArg("Zfar" , std::numeric_limits<MatrixType>::max());

  //AOV
  if(CfgSection.findParam("AngleOfView"))
  {
    const std::vector<MatrixType>& TmpAOV = CfgSection.getParamArgs("AngleOfView", (MatrixType)0);
    if(TmpAOV.size() != 4) { return false; }
    m_AngleOfView.set(TmpAOV);
  }
  else
  {
    m_AngleOfView.zero();
  }
  return true;
}
template <class MatrixType> std::string xCameraParams<MatrixType>::formatToPretyConf()
{
  std::string Result;
  Result.reserve(4096);
  Result += "[" + m_CamName + "]\n";
  Result += fmt::sprintf("  Location    = %d,%d\n", m_Location.getY(), m_Location.getX());
  Result += fmt::sprintf("  Resolution  = %dx%d\n", m_Resolution.getY(), m_Resolution.getX());
  Result += fmt::sprintf("  Projection  = %s\n"   , xProjTypeToString(m_ProjType));
  Result += "  Intrinsic   = " + m_Intrinsics.print("", "                ", ", ", 3, 3);
  Result += "  Extrinsic   = " + m_Extrinsics.print("", "                ", ", ", 3, 4);
  Result += "  Distortion  = " + m_Distortion.print(", ");
  Result += fmt::sprintf("  Znear       = %14.8f\n"   , m_Znear);
  Result += fmt::sprintf("  Zfar        = %14.8f\n"   , m_Zfar );
  Result += fmt::sprintf("  AngleOfView = %14.8f, %14.8f, %14.8f, %14.8f\n", m_AngleOfView.getDegLeft(), m_AngleOfView.getDegRight(), m_AngleOfView.getDegTop(), m_AngleOfView.getDegBottom());
  return Result;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template <class MatrixType> void xCameraParams<MatrixType>::rescaleToNewResolution(int32V2 NewResolution)
{
  if(NewResolution == m_Resolution) { return; }

  xVec2<MatrixType> ScalingFactor = xVec2<MatrixType>(NewResolution) / xVec2<MatrixType>(m_Resolution);

  m_Intrinsics.rescaleByFactorXY(ScalingFactor);

  calculateDerrived();

  m_Resolution = NewResolution;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<class MatrixType> xVec2<MatrixType> xCameraParams<MatrixType>::determineTargetZnearZfar(const tCamParams& TargetCamParams, const tCamParams& SourceCamParams)
{
  const int32V2           Resolution        = SourceCamParams.getResolution();
  const xVec2<MatrixType> SourceCorners [4] = { {0, 0}, {(MatrixType)Resolution.getX()-1, 0}, {0, (MatrixType)Resolution.getY()-1}, {(MatrixType)Resolution.getX()-1, (MatrixType)Resolution.getY()-1} };  
  const MatrixType        SourceExtremeZ[2] = { SourceCamParams.getZnear(), SourceCamParams.getZfar() };
  const tMatrix4x4        PIP               = calculatePIP(TargetCamParams, SourceCamParams);

  MatrixType TargetZnear = std::numeric_limits<MatrixType>::max   ();
  MatrixType TargetZfar  = std::numeric_limits<MatrixType>::lowest();

  for (int32 CornerIdx = 0; CornerIdx < 4; CornerIdx++)
  {
    for (int32 DistanceIdx = 0; DistanceIdx < 2; DistanceIdx++)
    {
      const xVec3<MatrixType> SourceCoord = { SourceCorners[CornerIdx], SourceExtremeZ[DistanceIdx] };
      const xVec3<MatrixType> TargetCoord = tReproject::Reproject2Dto2D(SourceCoord, PIP);
      const MatrixType        TargetZ     = TargetCoord.getZ();
      TargetZnear = xMin(TargetZnear, TargetZ);
      TargetZfar  = xMax(TargetZfar , TargetZ);
    }
  }
  return { TargetZnear, TargetZfar };
}

template <class MatrixType> void xCameraParams<MatrixType>::applyColmapPose(tCamParams* CamParams, xQuaternion<MatrixType> QuatWorldToCam, xVec3<MatrixType> TransWorldToCam)
{
  tRotation R;
  R.setQuaternion(QuatWorldToCam);

  const MatrixType Tx = TransWorldToCam.getX(), Ty = TransWorldToCam.getY(), Tz = TransWorldToCam.getZ();
  const MatrixType Cx = -(R[0][0] * Tx + R[1][0] * Ty + R[2][0] * Tz);
  const MatrixType Cy = -(R[0][1] * Tx + R[1][1] * Ty + R[2][1] * Tz);
  const MatrixType Cz = -(R[0][2] * Tx + R[1][2] * Ty + R[2][2] * Tz);

  tExtrinsic& E = CamParams->getExtrinsics();
  E.zero();
  for(int32 h = 0; h < 3; h++) { for(int32 w = 0; w < 3; w++) { E[h][w] = R[h][w]; } }
  E[0][3] = Cx; E[1][3] = Cy; E[2][3] = Cz;
  E[3][3] = (MatrixType)1;
}

//=============================================================================================================================================================================

template class xCameraParams<flt32>;
template class xCameraParams<flt64>;

//=============================================================================================================================================================================

} //end of namespace PMBB
