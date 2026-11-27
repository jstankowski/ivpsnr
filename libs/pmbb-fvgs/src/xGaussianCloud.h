/*
    SPDX-FileCopyrightText: 2025-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefFVGS.h"
#include <unordered_map>
#include <vector>

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================

class xGaussianCloudCmn
{
public:
  using tStr = std::string;

  class xCameraPosition
  {
  public:
    int32   m_ImgIdx  = NOT_VALID;
    int32   m_CamIdx  = NOT_VALID;
    flt64V3 m_PositionXYZ     ;
    flt64V4 m_OriginWXYZ      ; // quaternion w,x,y,z
    flt64V2 m_FocalLengthXY   ;
    flt64V2 m_PrincipalPointXY;    
    int32V2 m_ImageSizeXY     = { NOT_VALID, NOT_VALID }; // size of the image of the camera
    tStr    m_Name            ;

  public:
    xCameraPosition() {}

    xCameraPosition(int32 ImgIdx, int32 CamIdx, flt64V3 PositionXYZ, flt64V4 OriginWXYZ, flt64V2 FocalLengthXY, flt64V2 PrincipalPointXY, int32V2 ImageSizeXY, tStr Name)
    {
      m_ImgIdx           = ImgIdx          ;
      m_CamIdx           = CamIdx          ;
      m_PositionXYZ      = PositionXYZ     ;
      m_OriginWXYZ       = OriginWXYZ      ;
      m_FocalLengthXY    = FocalLengthXY   ;
      m_PrincipalPointXY = PrincipalPointXY;
      m_Name             = Name            ;
      m_ImageSizeXY      = ImageSizeXY     ;
    }
  };

  using tCameraData = std::vector<xCameraPosition>;

protected:
  tCameraData m_CameraData;

  int32 m_MaxNumSplats   = NOT_VALID;
  int32 m_NumValidSplats = NOT_VALID;
  int32 m_AttCount       = NOT_VALID;
  int32 m_SphHarmDegree  = NOT_VALID;
  int32 m_NumSphHarm     = NOT_VALID;  

  int64 m_POC            = NOT_VALID;
  int64 m_Timestamp      = NOT_VALID;

public:
  void               setCameraData(const tCameraData& CameraData)       { m_CameraData = CameraData; }
  const tCameraData& getCameraData(                             ) const { return m_CameraData      ; }

  void setNumValidSplats(int32 NumValidSplats) { m_NumValidSplats = NumValidSplats; }
  void setAttCount      (int32 AttCount      ) { m_AttCount       = AttCount      ; }
  void setSphHarmDegree (int32 SphHarmDegree ) { m_SphHarmDegree  = SphHarmDegree ; }
  void setSphHarmCount  (int32 NumSphHarm    ) { m_NumSphHarm     = NumSphHarm    ; }

  int32 getMaxNumSplats  () const { return m_MaxNumSplats  ; }
  int32 getNumValidSplats() const { return m_NumValidSplats; }
  int32 getAttCount      () const { return m_AttCount      ; }
  int32 getSphHarmDegree () const { return m_SphHarmDegree ; }
  int32 getNumSphHarm    () const { return m_NumSphHarm    ; }

  //time
  inline void  setPOC      (int64 POC      )       { m_POC = POC; }
  inline int64 getPOC      (               ) const { return m_POC; }
  inline void  setTimestamp(int64 Timestamp)       { m_Timestamp = Timestamp; }
  inline int64 getTimestamp(               ) const { return m_Timestamp; }
};

//===============================================================================================================================================================================================================

class xGaussianCloudP : public xGaussianCloudCmn
{
private:
  std::array< flt32*, static_cast<size_t>(xAttrFLT::COUNT)> m_Attributes = { nullptr };

public:
  xGaussianCloudP () {};
  ~xGaussianCloudP() { destroy(); };

  void create (int32 MaxNumSplats);
  void upsize (int32 MaxNumSplats);
  void destroy();
  void clear  ();
  
  flt32*       getAttrAddr(xAttrFLT Attr)       { return m_Attributes[(uintSize)Attr]; }
  const flt32* getAttrAddr(xAttrFLT Attr) const { return m_Attributes[(uintSize)Attr]; }

  void setAttr(xAttrFLT Attr, uintSize SplatIdx, flt32 Value) { m_Attributes[(size_t)Attr][SplatIdx] = Value; }
};

//===============================================================================================================================================================================================================

struct xGaussianObject 
{
  flt32 PositionXYX[3 ];
  flt32 Opacity        ;
  flt32 ScaleXYZ   [3 ];
  flt32 RotWXYZ    [4 ];
  flt32 DC_RGB     [3 ];
  flt32 Rest_RGB   [45];
};

static_assert(sizeof (xGaussianObject) == 236);
static_assert(alignof(xGaussianObject) == 4  );

class xGaussianCloudI : public xGaussianCloudCmn
{
protected:
  std::vector<xGaussianObject> m_GaussianObjects;

public:
  xGaussianCloudI () {};
  ~xGaussianCloudI() { destroy(); };

  void create (int32 MaxNumSplats);
  void upsize (int32 MaxNumSplats);
  void destroy();
  void clear  ();

  xGaussianObject*       getGaussianObjects()       { return m_GaussianObjects.data(); }
  const xGaussianObject* getGaussianObjects() const { return m_GaussianObjects.data(); }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
