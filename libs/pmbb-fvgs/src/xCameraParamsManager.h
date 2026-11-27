/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefFVGS.h"
#include "xCameraParams.h"
#include <array>
#include <istream>
#include <string_view>

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================

template <class MatrixType> class xCameraParamsManager
{
public:
  static constexpr int32 c_MaxNumCamGroups = 4;
  static constexpr int32 c_RealGroupIdx    = 0;
  static constexpr int32 c_VirtGroupIdx    = 1;

  //columns that can be used in a posetrace CSV (the first six are the columns of the format without a header)
  enum class ePoseTraceCol : int32 { INVALID = NOT_VALID, X, Y, Z, Yaw, Pitch, Roll, Qw, Qx, Qy, Qz, Fx, Fy, Zoom, NUM };
  static constexpr int32 c_NumPoseTraceCols = (int32)ePoseTraceCol::NUM;
  static constexpr std::array<std::string_view, c_NumPoseTraceCols> c_PoseTraceColNames = { "x", "y", "z", "yaw", "pitch", "roll", "qw", "qx", "qy", "qz", "fx", "fy", "zoom" };

  using tCamIdStrVec     = std::vector<std::string>;
  using tCamIdxVec        = std::vector<int32>;
  using tCamIdStrToCamIdx = std::map<std::string, int32>;
  using tCamIdxToCamIdStr = std::map<int32, std::string>;
  using tCamParams        = xCameraParams<MatrixType>;
  using tRotation         = typename tCamParams::tRotation;
  using tTranslation      = typename tCamParams::tTranslation;
  using tCamParamsMap     = std::map<int32, tCamParams>;
  using tCamParamsPtrMap  = std::map<int32, tCamParams*>;

protected:
  tCamIdStrVec m_SelCamIdStr[c_MaxNumCamGroups];
  tCamIdStrVec m_AllCamIdStr;
  tCamIdxVec   m_SelCamIdx[c_MaxNumCamGroups];

  tCamIdStrToCamIdx m_CamIdStrToCamIdx;
  tCamIdxToCamIdStr m_CamIdxToCamIdStr;
  tCamParamsMap     m_AllCamParams;
  tCamParamsPtrMap  m_SelCamParams[c_MaxNumCamGroups];

protected:
  //which column of a posetrace row holds which value
  struct xPoseTraceLayout
  {
    int32 m_NumCols = 0;                 //number of columns in a row
    int32 m_ColIdx[c_NumPoseTraceCols];  //position in a row of every ePoseTraceCol, NOT_VALID if the column is absent
    xPoseTraceLayout() { for(int32 i = 0; i < c_NumPoseTraceCols; i++) { m_ColIdx[i] = NOT_VALID; } }
    bool has(ePoseTraceCol Col) const { return m_ColIdx[(int32)Col] != NOT_VALID; }
  };

  //layout of a posetrace without a header: X,Y,Z,Yaw,Pitch,Roll
  static xPoseTraceLayout makePoseTraceLegacyLayout();
  //read the layout from the header row (column names); prints the reason and returns false if the names or their combination are not usable
  static bool             parsePoseTraceHeader(const std::string& Row, xPoseTraceLayout& Layout);
  //parse one data row; Values are indexed by ePoseTraceCol and set only for the columns present in the layout
  static bool             parsePoseTraceRow   (const std::string& Row, const xPoseTraceLayout& Layout, flt64 (&Values)[c_NumPoseTraceCols]);
  //build a virtual camera from the row values, intrinsics and Znear/Zfar come from the reference camera (focal length is replaced or scaled if the trace has it)
  static tCamParams       buildPoseTraceCam   (const tCamParams& RefCam, int32V2 Resolution, const xPoseTraceLayout& Layout, const flt64 (&Values)[c_NumPoseTraceCols]);
  //set everything of a virtual camera except the intrinsics: name, resolution, projection, Znear/Zfar (from the reference camera) and the extrinsics (R = world to camera rotation, Pos = camera center)
  static void             initPoseTraceCam    (tCamParams& CP, const tCamParams& RefCam, int32V2 Resolution, const tRotation& Rotation, const tTranslation& Position);

  //viewpoints file: the first line is the number of viewpoints, then every viewpoint is a list of numbers (white space separated):
  //  Pos(3) Target(3) Up(3) Fx Fy, optionally followed by Width Height or by Cx Cy Width Height
  //Target is the point the camera looks at, Fx Fy Cx Cy are in pixels of a picture Height rows high
  //there is no version marker, the number of numbers per viewpoint tells which layout it is
  struct xViewpointLayout { int32 m_NumValues; bool m_HasCenter; bool m_HasPicSize; };
  static constexpr int32 c_NumViewpointLayouts = 3;
  static constexpr std::array<xViewpointLayout, c_NumViewpointLayouts> c_ViewpointLayouts =
  {{
    { 11, false, false }, //Pos Target Up Fx Fy
    { 13, false, true  }, //Pos Target Up Fx Fy Width Height
    { 15, true , true  }  //Pos Target Up Fx Fy Cx Cy Width Height
  }};
  struct xViewpoint
  {
    tTranslation      m_Pos;         //camera position
    tTranslation      m_Target;      //point the camera looks at
    tTranslation      m_Up;          //up vector
    xVec2<MatrixType> m_Focal;       //focal length Fx, Fy
    xVec2<MatrixType> m_Center;      //principal point Cx, Cy measured from the top left corner, (0, 0) if not given (the picture center is used)
    flt64             m_OrigHeight;  //height of the picture the focal length and the center are given for, 0 if not given (they are given for the rendered picture)
  };
  //true if the row starts with the number of viewpoints and is not a row of a CSV posetrace (that one always has commas), i.e. the file is a viewpoints file
  static bool             parseViewpointsCount(const std::string& Row, int32& NumViewpoints);
  //read the numbers of all viewpoints (the rest of the stream) and check them, prints the reason and returns false if the file is not usable
  static bool             parseViewpoints     (std::istream& Stream, int32 NumViewpoints, std::vector<xViewpoint>& Viewpoints);
  //build a virtual camera looking from Pos to Target (focal length and center are scaled by Height/OrigHeight); false if Pos == Target or Up is parallel to the view direction
  static bool             buildViewpointCam   (const tCamParams& RefCam, int32V2 Resolution, const xViewpoint& Viewpoint, tCamParams& CP);

public:
  void  setCamGroup(int32 CamGroupIdx, const tCamIdStrVec& CamIdStrs) { m_SelCamIdStr[CamGroupIdx] = CamIdStrs; }
  void  initAllCams( );
  //load from old style DERS-like .txt file
  void  loadParams (int32 CamGroupIdx, std::string CamParamsTemplate, std::string CamParamsFile, MatrixType Znear, MatrixType Zfar, int32V2 Resolution);
  //load from .cfg file in INI format
  bool  loadParams (int32 CamGroupIdx, const std::string& CamNameTemplate, const std::string& CamParamsFilePath);
  //load from COLMAP directory (cameras+images; .txt preferred, if absent find .bin)
  bool  loadParams (int32 CamGroupIdx, const std::string& CamParamsDir);
  //load from camera_position entries embedded in a Gaussian-splat PLY header
  bool  loadParams (int32 CamGroupIdx, const std::string& PlyFilePath, int32V2 Resolution);
  //load from posetrace CSV, reusing intrinsics/Znear/Zfar from an already-loaded real camera
  //without a header every row is X,Y,Z,Yaw,Pitch,Roll (angles in degrees)
  //with a header (column names, any order, case insensitive) a row has: x,y,z + rotation as yaw,pitch,roll OR qw,qx,qy,qz + optionally focal length as fx,fy (pixels, scale of the reference camera) OR zoom (multiplier of the reference focal length)
  //a file starting with a line that holds only the number of viewpoints is a viewpoints file (position, target point, up vector and intrinsics of every viewpoint)
  bool  loadPoseTraceParams(int32 CamGroupIdx, const std::string& PoseTraceFilePath, int32V2 Resolution);
  //load real cameras (c_RealGroupIdx) dispatching on format - PLY/COLMAP/INI - the CamNameTemplate is only used for INI
  bool  loadRealCams(eCamParamsFormat Format, const std::string& CamParamsFile, const std::string& CamNameTemplate, int32V2 Resolution);
  //adopt already-computed params (e.g. parsed from a PLY's camera_position entries by the caller)
  void  setCamParams(int32 CamGroupIdx, const std::vector<tCamParams>& Cams);

  void  overwriteInvalidZs(MatrixType Znear, MatrixType Zfar) { for(auto& [CamIdx, CamParams] : m_AllCamParams) { if(!CamParams.hasValidZs()) { CamParams.setZnear(Znear); CamParams.setZfar(Zfar); } } }
  void  overwriteAllZs    (MatrixType Znear, MatrixType Zfar) { for(auto& [CamIdx, CamParams] : m_AllCamParams) { CamParams.setZnear(Znear); CamParams.setZfar(Zfar); } }
  
  void  rescaleAllToNewResolution(int32V2 Resolution ) { for(auto& [CamIdx, CamParams] : m_AllCamParams) { CamParams.rescaleToNewResolution(Resolution); } }


public:
  int32             getNumCams(int32 CamGroupIdx) const { return (int32)m_SelCamIdx[CamGroupIdx].size(); }
  const tCamIdxVec& getCamIdx (int32 CamGroupIdx) const { return m_SelCamIdx[CamGroupIdx]; }

  int32              CamIdStrToCamIdx(const std::string& CamIdStr) const { return m_CamIdStrToCamIdx.at(CamIdStr); }
  const std::string& CamIdxToCamIdStr(int32              CamIdx  ) const { return m_CamIdxToCamIdStr.at(CamIdx); }

  const tCamIdStrToCamIdx& getCamIdStrToCamIdx() const { return m_CamIdStrToCamIdx; }
  const tCamIdxToCamIdStr& getCamIdxToCamIdStr() const { return m_CamIdxToCamIdStr; }

  const tCamParams&    getCamParams    (int32 CamIdx) const { return m_AllCamParams.at(CamIdx); }
  const tCamParamsMap& getAllCamParams (            ) const { return m_AllCamParams; }

public: 
  int32 getNumRealCams() const { return getNumCams(c_RealGroupIdx); }
  int32 getNumVirtCams() const { return getNumCams(c_VirtGroupIdx); }

  const tCamIdxVec& getRealCamIdx() const { return getCamIdx(c_RealGroupIdx); }
  const tCamIdxVec& getVirtCamIdx() const { return getCamIdx(c_VirtGroupIdx); }

  const tCamParamsPtrMap& getRealCamParams() { return m_SelCamParams[c_RealGroupIdx]; }
  const tCamParamsPtrMap& getVirtCamParams() { return m_SelCamParams[c_VirtGroupIdx]; }
};

//===============================================================================================================================================================================================================

#ifndef PMBB_xCameraParamsManager_IMPLEMENTATION
extern template class xCameraParamsManager<flt32>;
extern template class xCameraParamsManager<flt64>;
#endif // !PMBB_xCameraParamsManager_IMPLEMENTATION

//===============================================================================================================================================================================================================

} //end of namespace PMBB

