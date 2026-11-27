/*
    SPDX-FileCopyrightText: 2025-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefFVGS.h"
#include "xSeq.h"
#include "xFileListUtils.h"
#include "xGaussianCloud.h"
#include "xPointCloud.h"
#include <unordered_map>
#include <map>

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================

class xPly
{
public:
  using tStr  = std::string;
  using tCamD = xGaussianCloudCmn::tCameraData;
  using tPrOf = std::map<tStr    , int32>;
  using tAITO = std::map<xAttrFLT, int32>;
  using tRes  = xInOutResult;

  static constexpr std::string_view c_PropertyPattern    = "property float ";
  static constexpr int32            c_PropertyPatternLen = (int32)c_PropertyPattern.length();
  static constexpr int32            c_MaxNumSphHarm      = 45;
  static constexpr int32            c_NumSplatsInBatch   = 16 * 1024;
  static constexpr int32            c_NumPointsInBatch   = 4 * c_NumSplatsInBatch;
  static constexpr bool             c_SphHarmOrderPCO    = true; //true --> [Point, Component, Order]; false --> [Point, Order, Component]

  //properties pattern for typical reference cloud
  static constexpr int32 c_NumPropertiesPatternR = 62;
  static constexpr std::array<std::string_view, c_NumPropertiesPatternR> c_PropertyListPatternR =
  {
    "x", "y", "z", "nx", "ny", "nz", "f_dc_0", "f_dc_1", "f_dc_2", 
    "f_rest_0" , "f_rest_1" , "f_rest_2" , "f_rest_3" , "f_rest_4" , "f_rest_5" , "f_rest_6" , "f_rest_7" , "f_rest_8" , "f_rest_9" ,
    "f_rest_10", "f_rest_11", "f_rest_12", "f_rest_13", "f_rest_14", "f_rest_15", "f_rest_16", "f_rest_17", "f_rest_18", "f_rest_19",
    "f_rest_20", "f_rest_21", "f_rest_22", "f_rest_23", "f_rest_24", "f_rest_25", "f_rest_26", "f_rest_27", "f_rest_28", "f_rest_29",
    "f_rest_30", "f_rest_31", "f_rest_32", "f_rest_33", "f_rest_34", "f_rest_35", "f_rest_36", "f_rest_37", "f_rest_38", "f_rest_39",
    "f_rest_40", "f_rest_41", "f_rest_42", "f_rest_43", "f_rest_44",
    "opacity", "scale_0", "scale_1", "scale_2", "rot_0", "rot_1", "rot_2", "rot_3",
  };

  static constexpr int32 c_NumPropertiesPatternD = 59;
  static constexpr std::array<std::string_view, c_NumPropertiesPatternD> c_PropertyListPatternD =
  {
    "x", "y", "z", "opacity", "scale_1", "scale_2", "rot_0", "rot_1", "rot_2", "rot_3", "f_dc_0", "f_dc_1", "f_dc_2",
    "f_rest_0" , "f_rest_1" , "f_rest_2" , "f_rest_3" , "f_rest_4" , "f_rest_5" , "f_rest_6" , "f_rest_7" , "f_rest_8" , "f_rest_9" ,
    "f_rest_10", "f_rest_11", "f_rest_12", "f_rest_13", "f_rest_14", "f_rest_15", "f_rest_16", "f_rest_17", "f_rest_18", "f_rest_19",
    "f_rest_20", "f_rest_21", "f_rest_22", "f_rest_23", "f_rest_24", "f_rest_25", "f_rest_26", "f_rest_27", "f_rest_28", "f_rest_29",
    "f_rest_30", "f_rest_31", "f_rest_32", "f_rest_33", "f_rest_34", "f_rest_35", "f_rest_36", "f_rest_37", "f_rest_38", "f_rest_39",
    "f_rest_40", "f_rest_41", "f_rest_42", "f_rest_43", "f_rest_44"
  };

protected:
  // camera parameters in the camera_position
  enum class eCamParam : int32
  {
    ImgIdx = 0, CamIdx, PosX, PosY, PosZ, QuatW, QuatX, QuatY, QuatZ, FocalX, FocalY, CenterX, CenterY, Width, Height,
    COUNT
  };
  static constexpr int32 c_NumCamParams = (int32)eCamParam::COUNT;
  static constexpr std::array<std::string_view, c_NumCamParams> c_CamParamNames =
  {
    "imageid", "cameraid", "pos.x", "pos.y", "pos.z", "quat.w", "quat.x", "quat.y", "quat.z", "focal.x", "focal.y", "center.x", "center.y", "width", "height"
  };

  tCamD m_CameraData;
  int32 m_NumSplats        = NOT_VALID;
  int32 m_NumSphHarmCoeffs = 0;
  int32 m_NumSphHarm       = 0;
  tPrOf m_PropertyOffset;
  tAITO m_AttrIdToOffset;
  bool  m_MatchesPatternR  = false;
  bool  m_MatchesPatternD  = false;

public:
  xPly () {}
  ~xPly() {}

  tRes readGC (xGaussianCloudP* DstCloud, xStream* Stream, bool HeaderOnly = false);
  tRes readGC (xGaussianCloudI* DstCloud, xStream* Stream, bool HeaderOnly = false);
  tRes writeGC(xStream* Stream, const xGaussianCloudP* SrcCloud);
  tRes writePC(xStream* Stream, const xPointCloud*     SrcCloud, bool Binary);

protected:
  tStr        xFindHeader(xStream* Stream);
  int32       xFindPropertyOffset(const tStr& AttributeName);
  bool        xCheckPatternMatch(const std::string_view* Pattern, int32 NumProperties);
  tRes        xParseHeaderGSC (const tStr& Header);
  static tStr xFormatHeaderGSC(int32 NumSplats, int32 NumSphericalHarmonics);
  static tStr xFormatHeaderPC (int32 NumUnits, bool Binary);

};

//===============================================================================================================================================================================================================

class xSeqPlyList : public xSeqFile, public xFileListUtils
{
public:
  using tStr = std::string;

protected:
  tStr  m_FileNamePattern;
  int32 m_1stFrameHint = NOT_VALID;
  int32 m_MaxNumFiles  = NOT_VALID;
  int32 m_1stFileIdx   = NOT_VALID;
  bool  m_SingleFile   = false;
  bool  m_CountFrom0   = false; //for reading lists starting from non-zero (e.g. img0034.png) treat 1st frame as haveing index 0
  int32 m_NumOfFrames  = NOT_VALID;
  int32 m_CurrFrameIdx = NOT_VALID;

  xPly m_Ply;

public:
  void create (bool CountFramesFromZero, int32 FirstFrameHint, int32 MaxNumFiles);
  void destroy() {}

  tResult readGC(xGaussianCloudP* DstCloud);
  tResult readGC(xGaussianCloudI* DstCloud);

  tResult seekFrame(int32 FrameIdx);

  tStr    get1stFileName () const { return xFormatFileName(m_1stFileIdx); }  
  int32   getNumOfFrames () const { return m_NumOfFrames; }
  int32   getCurrFrameIdx() const { return m_CurrFrameIdx; }

protected:
  virtual bool    xBackendAllowsRead  () const final { return true ; }
  virtual bool    xBackendAllowsWrite () const final { return false; }
  virtual bool    xBackendAllowsAppend() const final { return false; }
  virtual bool    xBackendAllowsSeek  () const final { return true ; }
  virtual tResult xBackendOpen        (tCSR FileName, eMode OpMode) final;
  virtual tResult xBackendClose       (                           ) final;

protected:
  inline tStr xFormatFileName(int32 FrameIdx) const { return formatNamedPattern(m_FileNamePattern, FrameIdx, fmt::arg("FrameIdx", FrameIdx)); }
  tResult xPlyListOpenRead  ();
  tResult xPlyListOpenWrite ();
  tResult xPlyListFileVerify(tCSR FileName);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB