/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefFVGS.h"
#include "xMatrix.h"
#include "xReprojection.h"
#include "xCameraModel.h"
#include "xCfgINI.h"

/*
//===============================================================================================================================================================================================================
Cam param file syntax (.txt file format):
param_cam00
873.6992728660372900	0.0000000000000000	939.3144013150633800	
0.0000000000000000	880.8059166544752500	509.3805694393765300	
0.0000000000000000	0.0000000000000000	1.0000000000000000	
0
0
0.0264282413538898	-0.0425232723300337	-0.9987458732677129	5.2002438394948713	
-0.5476053176081434	0.8352384118640591	-0.0500520876096998	-0.2001012509099338	
0.8363192955960213	0.5482413397922036	-0.0012120877132428	-5.2364864777130515	

//===============================================================================================================================================================================================================
Cam param file syntax (.conf (INI) file format):

[Camera0001]                            #camera name - single camera section
  #### mandatory fields
  Resolution  = 1920x1080               #camera original resolution (valid for intrin params)
  Projection  = Perspective             #camera projection type [Perspective, Equirectangular]

  Intrinsic   = 873.6992728660372900,   0.0000000000000000, 939.3144013150633800,
                  0.0000000000000000, 880.8059166544752500, 509.3805694393765300,
                  0.0000000000000000,   0.0000000000000000,   1.0000000000000000

  Extrinsic   =   0.0264282413538898,  -0.0425232723300337,  -0.9987458732677129,   5.2002438394948713,
                 -0.5476053176081434,   0.8352384118640591,  -0.0500520876096998,  -0.2001012509099338,
                  0.8363192955960213,   0.5482413397922036,  -0.0012120877132428,  -5.2364864777130515

  #### optional fields
  Location    = 0,0             #hint at camera location in system (as X,Y or X,Y,Z)
  Type        = Real            #camera type [Real, Virtual]
  Znear       = 3.5
  Zfar        = 25
  AngleOfView = 0, 0, 0, 0      #angle of view in equirectangular representation (L, R, T, B) id degrees

//===============================================================================================================================================================================================================
*/

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================

template <class MatrixType> class xCameraParams
{
public:
  using tCamParams  = xCameraParams<MatrixType>;
  using tMatrix4x4  = xMatrix<4, 4, MatrixType>;
  using tMatrix3x4  = xMatrix<3, 4, MatrixType>;
  using tMatrix3x3  = xMatrix<3, 3, MatrixType>;
  using tMatrix3x1  = xMatrix<3, 1, MatrixType>;
  using tReproject  = xReprojection<MatrixType>;

public:
  using tRotation    = xRotationMat     <MatrixType>;
  using tTranslation = xTranslationVec  <MatrixType>;
  using tIntrinsic   = xIntrinsicMat    <MatrixType>;
  using tExtrinsic   = xExtrinsicMat    <MatrixType>;
  using tDistortion  = xDistortionParams<MatrixType>;
  using tAngleOfview = xAngleOfview     <MatrixType>;

  using tProjMat     = xMatrix<4, 4, MatrixType>;
  using tPiP         = xMatrix<4, 4, MatrixType>;

protected:
  int32          m_Idx;            //camera identifier - internal

  std::string    m_CamName;        //camera name  
  int32V2        m_Resolution;     //image resolution
  eProjType      m_ProjType;       //camera projection type
                                   
  tIntrinsic     m_Intrinsics;     //intrinsic camera parameters
  tExtrinsic     m_Extrinsics;     //extrinsic camera parameters (global) [R|T]
  tDistortion    m_Distortion;     //distortion parameters

  eProvenance    m_Provenance;
  int32V2        m_Location;       //location in system

  MatrixType     m_Znear;          //z near
  MatrixType     m_Zfar;           //z far

  tAngleOfview   m_AngleOfView;    //angle of view (equirectangular)

  //derrived params
  tExtrinsic     m_ExtrinsicsLocal;   //extrinsic camera parameters (local) [R|-RT]
  tExtrinsic     m_InvExtrinsicsLocal;
  tProjMat       m_Projection;        //projection matrix
  tProjMat       m_InvProjection;     //inverse projection matrix
  
public:
  xCameraParams();
  void        reset();
  std::string print(const std::string& Prefix) const;
  void        calculateDerrived       () { calculateLocalExtrinsics(); calculateProjectionMats(); }
  void        calculateLocalExtrinsics();
  void        calculateProjectionMats ();
  
  //read from file
  static tCamParams* readParamsFromString(std::string CamParamsFileContent, std::string ZnearZfarFileContent, std::string CamName, MatrixType DefaultZnear, MatrixType DefaultZfar);
  static void readCamParams(tCamParams* CamParams, std::string CfgCamParams);
  static void readZnearZfar(tCamParams* CamParams, std::string CfgZnearZfar);
  bool        readFromConf (const xCfgINI::xSection& CfgSection);
  std::string formatToPretyConf ();

  //modify
  void rescaleToNewResolution(int32V2 NewResolution);

  //calculate RiR and distance
  static inline tMatrix4x4 calculateRIR     (const tCamParams& TrgtCamParams, const tCamParams& SrcCamParams) { return xMatrix<4,4,MatrixType>::MultiplyMatrixByMatrix(TrgtCamParams.getExtrinsicsLocal(), SrcCamParams.getInvExtrinsicsLocal()); }
  static inline MatrixType calculateDistance(const tCamParams& TrgtCamParams, const tCamParams& SrcCamParams);

  template<class OtherType> static inline xMatrix<4,4,OtherType> calculateConvertRIR(const tCamParams& TargetCamParams, const tCamParams& SourceCamParams);

  //calculate PiP
  static inline void calculatePIP(tPiP& PIP, const tCamParams& TrgtCamParams, const tCamParams& SrcCamParams) { PIP.setMultiplyMatrixByMatrix(TrgtCamParams.getProjection(), SrcCamParams.getInvProjection()); }
  static inline tPiP calculatePIP(           const tCamParams& TrgtCamParams, const tCamParams& SrcCamParams) { return tPiP::MultiplyMatrixByMatrix(TrgtCamParams.getProjection(), SrcCamParams.getInvProjection()); }

  template<class OtherType> static inline void calculateConvertPIP(xMatrix<4,4,OtherType>& PIP, const tCamParams& TargetCamParams, const tCamParams& SourceCamParams);
  template<class OtherType> static inline xMatrix<4,4,OtherType> calculateConvertPIP(const tCamParams& TargetCamParams, const tCamParams& SourceCamParams);

  //utils
  static xVec2<MatrixType> determineTargetZnearZfar(const tCamParams& TargetCamParams, const tCamParams& SourceCamParams);

  static void applyColmapPose(tCamParams* CamParams, xQuaternion<MatrixType> QuatWorldToCam, xVec3<MatrixType> TransWorldToCam);

public:
  void               setIdx          (int32             Idx     )       { m_Idx = Idx;           }
  int32              getIdx          (                          ) const { return m_Idx;          }

  void               setCamName      (const std::string& CamName)       { m_CamName = CamName;   }
  const std::string& getCamName      (                          ) const { return m_CamName;      }
  void               setResolution   (int32V2         Resolution)       { m_Resolution = Resolution; }
  int32V2            getResolution   (                          ) const { return m_Resolution;       }
  void               setProjType     (eProjType         ProjType)       { m_ProjType = ProjType; }
  eProjType          getProjType     (                          ) const { return m_ProjType;     }

  tIntrinsic&        getIntrinsics   (                          )       { return m_Intrinsics;   }
  const tIntrinsic&  getIntrinsics   (                          ) const { return m_Intrinsics;   }
  tIntrinsic*        getIntrinsicsPtr(                          )       { return &m_Intrinsics;  }
  tExtrinsic&        getExtrinsics   (                          )       { return m_Extrinsics;   }
  const tExtrinsic&  getExtrinsics   (                          ) const { return m_Extrinsics;   }
  tExtrinsic*        getExtrinsicsPtr(                          )       { return &m_Extrinsics;  }
  tDistortion&       getDistortion   (                          )       { return m_Distortion;   }
  const tDistortion& getDistortion   (                          ) const { return m_Distortion;   }
  tDistortion*       getDistortionPtr(                          )       { return &m_Distortion;  }

  void               setProvenance   (eProvenance     Provenance)       { m_Provenance = Provenance; }
  eProvenance        getProvenance   (                          ) const { return m_Provenance;     }
  void               setLocation     (int32V2           Location)       { m_Location = Location; }
  int32V2            getLocation     (                          ) const { return m_Location;     }

  void               setZnear        (MatrixType         Znear  )       { m_Znear = Znear;       }
  MatrixType         getZnear        (                          ) const { return m_Znear;        }
  void               setZfar         (MatrixType         Zfar   )       { m_Zfar = Zfar;         }
  MatrixType         getZfar         (                          ) const { return m_Zfar;         }
  bool               hasValidZs      (                          ) const { return m_Znear != std::numeric_limits<MatrixType>::max() && m_Zfar != std::numeric_limits<MatrixType>::max(); }

  void               setAngleOfView  (tAngleOfview   AngleOfView)       { m_AngleOfView = AngleOfView; }
  tAngleOfview       getAngleOfView  (                          ) const { return m_AngleOfView;        }

  //derrived matrices
  tExtrinsic&        getExtrinsicsLocal   ()       { return m_ExtrinsicsLocal   ; }
  const tExtrinsic&  getExtrinsicsLocal   () const { return m_ExtrinsicsLocal   ; }
  tMatrix4x4&        getInvExtrinsicsLocal()       { return m_InvExtrinsicsLocal; }
  const tMatrix4x4&  getInvExtrinsicsLocal() const { return m_InvExtrinsicsLocal; }
  tProjMat&          getProjection        ()       { return m_Projection        ; }
  const tProjMat&    getProjection        () const { return m_Projection        ; }
  tProjMat&          getInvProjection     ()       { return m_InvProjection     ; }
  const tProjMat&    getInvProjection     () const { return m_InvProjection     ; }
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<class MatrixType> MatrixType xCameraParams<MatrixType>::calculateDistance(const tCamParams& TrgtCamParams, const tCamParams& SrcCamParams)
{
  xMatrix<4,4,MatrixType> RIR = calculateRIR(TrgtCamParams, SrcCamParams);
  MatrixType Distance = sqrt(xPow2(RIR.at(0, 3)) + xPow2(RIR.at(1, 3)) + xPow2(RIR.at(2, 3)));
  return Distance;
}
template<class MatrixType> template<class OtherType> xMatrix<4,4,OtherType> xCameraParams<MatrixType>::calculateConvertRIR(const tCamParams& TrgtCamParams, const tCamParams& SrcCamParams)
{ 
  if constexpr(std::is_same_v<MatrixType, OtherType>) { return calculateRIR(TrgtCamParams, SrcCamParams); }
  else                                                { xMatrix<4,4,OtherType> RIR = (xMatrix<4,4,OtherType>)(calculateRIR(TrgtCamParams, SrcCamParams)); return RIR; }
}
template<class MatrixType> template<class OtherType> void xCameraParams<MatrixType>::calculateConvertPIP(xMatrix<4,4,OtherType>& PIP, const tCamParams& TrgtCamParams, const tCamParams& SrcCamParams)
{ 
  if constexpr(std::is_same_v<MatrixType, OtherType>) { calculatePIP(PIP, TrgtCamParams, SrcCamParams); }
  else                                                { PIP.convert(calculatePIP(TrgtCamParams, SrcCamParams)); }
}
template<class MatrixType> template<class OtherType> xMatrix<4,4,OtherType> xCameraParams<MatrixType>::calculateConvertPIP(const tCamParams& TrgtCamParams, const tCamParams& SrcCamParams)
{ 
  if constexpr(std::is_same_v<MatrixType, OtherType>) { return calculatePIP(TrgtCamParams, SrcCamParams); }
  else                                                { xMatrix<4,4,OtherType> PIP = (xMatrix<4,4,OtherType>)(calculatePIP(TrgtCamParams, SrcCamParams)); return PIP; }
}


//=============================================================================================================================================================================

#ifndef PMBB_xCameraParams_IMPLEMENTATION
extern template class xCameraParams<flt32>;
extern template class xCameraParams<flt64>;
#endif // !PMBB_xCameraParams_IMPLEMENTATION

//=============================================================================================================================================================================

} //end of namespace AVLib

