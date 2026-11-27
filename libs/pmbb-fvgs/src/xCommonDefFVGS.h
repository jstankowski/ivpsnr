#pragma once
#include "xCommonDefCORE.h"
#include "xString.h"
#include "xVec.h"
#include "xSeq.h"
#include <unordered_map>


#define CONV_SCALE   0 //broken
#define DZIEMBO      0
#define DZIEMBO_NOG  0*4095//(4096*16)//4096
#define GOP_FLAS     0
#define DEBUG_DZIEMBO_YUVS 0


namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================

enum class eProjType : int32
{
  INVALID         = NOT_VALID,
  UNKNOWN         = 0,
  Perspective     = 1,
  Equirectangular = 2,
};
static inline eProjType xStrToProjType(std::string_view ProjType)
{
  std::string ProjTypeL = xString::toLower(ProjType);
  return ProjTypeL=="perspective"      ? eProjType::Perspective      :
         ProjTypeL=="equirectangular"  ? eProjType::Equirectangular  :
                                        eProjType::INVALID;
}
static inline std::string_view xProjTypeToString(eProjType ProjType) 
{
  return ProjType==eProjType::Perspective     ? "Perspective"     :
         ProjType==eProjType::Equirectangular ? "Equirectangular" :
                                                "INVALID";
}

enum class eProvenance : int32
{
  INVALID = NOT_VALID,
  UNKNOWN = 0,
  Real    = 1,
  Virt    = 2,
};
static inline eProvenance xStrToProvenance(std::string_view Provenance)
{
  std::string ProvenanceL = xString::toLower(Provenance);
  return ProvenanceL=="real" ? eProvenance::Real :
         ProvenanceL=="virt" ? eProvenance::Virt :
                               eProvenance::INVALID;
}
static inline std::string_view xProvenanceToString(eProvenance Provenance)
{
  return Provenance==eProvenance::Real ? "Real" :
         Provenance==eProvenance::Virt ? "Virt" :
                                         "INVALID";
}

enum class eColmapCameraModel : int32
{
  INVALID               = NOT_VALID,
  SIMPLE_PINHOLE        = 0,
  PINHOLE               = 1,
  SIMPLE_RADIAL         = 2,
  RADIAL                = 3,
  OPENCV                = 4,
  OPENCV_FISHEYE        = 5,
  FULL_OPENCV           = 6,
  FOV                   = 7,
  SIMPLE_RADIAL_FISHEYE = 8,
  RADIAL_FISHEYE        = 9,
  THIN_PRISM_FISHEYE    = 10,
};

static inline eColmapCameraModel xStrToColmapCameraModel(std::string_view Model)
{
  return Model=="SIMPLE_PINHOLE"        ? eColmapCameraModel::SIMPLE_PINHOLE        :
         Model=="PINHOLE"               ? eColmapCameraModel::PINHOLE               :
         Model=="SIMPLE_RADIAL"         ? eColmapCameraModel::SIMPLE_RADIAL         :
         Model=="RADIAL"                ? eColmapCameraModel::RADIAL                :
         Model=="OPENCV"                ? eColmapCameraModel::OPENCV                :
         Model=="OPENCV_FISHEYE"        ? eColmapCameraModel::OPENCV_FISHEYE        :
         Model=="FULL_OPENCV"           ? eColmapCameraModel::FULL_OPENCV           :
         Model=="FOV"                   ? eColmapCameraModel::FOV                   :
         Model=="SIMPLE_RADIAL_FISHEYE" ? eColmapCameraModel::SIMPLE_RADIAL_FISHEYE :
         Model=="RADIAL_FISHEYE"        ? eColmapCameraModel::RADIAL_FISHEYE        :
         Model=="THIN_PRISM_FISHEYE"    ? eColmapCameraModel::THIN_PRISM_FISHEYE    :
                                          eColmapCameraModel::INVALID;
}

static inline std::string_view xColmapCameraModelToStr(eColmapCameraModel Model)
{
  switch(Model)
  {
  case eColmapCameraModel::SIMPLE_PINHOLE       : return "SIMPLE_PINHOLE";
  case eColmapCameraModel::PINHOLE              : return "PINHOLE";
  case eColmapCameraModel::SIMPLE_RADIAL        : return "SIMPLE_RADIAL";
  case eColmapCameraModel::RADIAL               : return "RADIAL";
  case eColmapCameraModel::OPENCV               : return "OPENCV";
  case eColmapCameraModel::OPENCV_FISHEYE       : return "OPENCV_FISHEYE";
  case eColmapCameraModel::FULL_OPENCV          : return "FULL_OPENCV";
  case eColmapCameraModel::FOV                  : return "FOV";
  case eColmapCameraModel::SIMPLE_RADIAL_FISHEYE: return "SIMPLE_RADIAL_FISHEYE";
  case eColmapCameraModel::RADIAL_FISHEYE       : return "RADIAL_FISHEYE";
  case eColmapCameraModel::THIN_PRISM_FISHEYE   : return "THIN_PRISM_FISHEYE";
  default                                       : return "INVALID";
  }
}

enum class eCamParamsFormat : int32
{
  INVALID = NOT_VALID,
  PLY   ,
  COLMAP,
  INI   ,
};

static inline eCamParamsFormat xStrToCamParamsFormat(const std::string& Format)
{
  std::string FormatU = xString::toUpper(Format);
  return FormatU=="PLY"    ? eCamParamsFormat::PLY    :
         FormatU=="COLMAP" ? eCamParamsFormat::COLMAP :
         FormatU=="INI"    ? eCamParamsFormat::INI    :
                             eCamParamsFormat::INVALID;
}

static inline std::string_view xCamParamsFormatToStr(eCamParamsFormat Format)
{
  switch(Format)
  {
  case eCamParamsFormat::PLY   : return "PLY"    ;
  case eCamParamsFormat::COLMAP: return "COLMAP" ;
  case eCamParamsFormat::INI   : return "INI"    ;
  default                      : return "INVALID";
  }
}

//===============================================================================================================================================================================================================
static constexpr bool c_ScaleColors  = false;
static constexpr bool c_ConvColors   = true; //true
static constexpr bool c_ConvOpacity  = false;
static constexpr bool c_ConvRotation = false;
static constexpr bool c_ConvSphHarm  = false;// true;
  
enum class xAttrFLT : int32 //flt32
{
  PX = 0, PY, PZ, 
  SX, SY, SZ,           //on log scale, compute exp(x) to get scale factor
  RX, RY, RZ, RW,       //quaternion
  OP,                   //compute sigmoid(a) to get opacity value between 0 and 1
  HR0,  HG0,  HB0,         //DC
  HR1,  HG1,  HB1,         //AC, max 15 per splat
  HR2,  HG2,  HB2,
  HR3,  HG3,  HB3,
  HR4,  HG4,  HB4,
  HR5,  HG5,  HB5,
  HR6,  HG6,  HB6,
  HR7,  HG7,  HB7,
  HR8,  HG8,  HB8,
  HR9,  HG9,  HB9,
  HR10, HG10, HB10,
  HR11, HG11, HB11,
  HR12, HG12, HB12,
  HR13, HG13, HB13,
  HR14, HG14, HB14,
  HR15, HG15, HB15,
  COUNT
};

enum class xAttrINT : int32 //uint16
{
  PX = 0, PY, PZ,
  SX, SY, SZ,
  RX, RY, RZ,       //axis-angle
  OP,
  HY0,  HU0,  HV0,
  HY1,  HU1,  HV1,
  HY2,  HU2,  HV2,
  HY3,  HU3,  HV3,
  HY4,  HU4,  HV4,
  HY5,  HU5,  HV5,
  HY6,  HU6,  HV6,
  HY7,  HU7,  HV7,
  HY8,  HU8,  HV8,
  HY9,  HU9,  HV9,
  HY10, HU10, HV10,
  HY11, HU11, HV11,
  HY12, HU12, HV12,
  HY13, HU13, HV13,
  HY14, HU14, HV14,
  HY15, HU15, HV15,
  COUNT
};

using tAttrSeq = std::array<xSeq*,    (size_t)xAttrINT::COUNT>;

class xConfig
{
protected:
  int32V2 m_PictureSize      = { NOT_VALID, NOT_VALID };
  int32   m_BlockSize        = 64; //64
  int32   m_GroupCount       = 16;
  int32   m_MaxSeqNumSplats  = 0;
public:
  bool    m_NonLinearScaling = false;
protected:
  bool    m_EnabledColorScaling = false;

  std::string m_YuvFileInputDir;
  std::string m_YuvFileOutputDir;

  std::unordered_map<xAttrINT, uint16> m_AttributeBPS =
  {
    { xAttrINT::PX  , (uint16)16 }, { xAttrINT::PY  , (uint16)16 }, { xAttrINT::PZ  , (uint16)16 },
    { xAttrINT::SX  , (uint16)12 }, { xAttrINT::SY  , (uint16)12 }, { xAttrINT::SZ  , (uint16)12 },
    { xAttrINT::RX  , (uint16)12 }, { xAttrINT::RY  , (uint16)12 }, { xAttrINT::RZ  , (uint16)12 },
    { xAttrINT::OP  , (uint16)12 },
    { xAttrINT::HY0 , (uint16)12 }, { xAttrINT::HU0 , (uint16)12 }, { xAttrINT::HV0 , (uint16)12 },
    { xAttrINT::HY1 , (uint16)12 }, { xAttrINT::HU1 , (uint16)12 }, { xAttrINT::HV1 , (uint16)12 },
    { xAttrINT::HY2 , (uint16)12 }, { xAttrINT::HU2 , (uint16)12 }, { xAttrINT::HV2 , (uint16)12 },
    { xAttrINT::HY3 , (uint16)12 }, { xAttrINT::HU3 , (uint16)12 }, { xAttrINT::HV3 , (uint16)12 },
    { xAttrINT::HY4 , (uint16)12 }, { xAttrINT::HU4 , (uint16)12 }, { xAttrINT::HV4 , (uint16)12 },
    { xAttrINT::HY5 , (uint16)12 }, { xAttrINT::HU5 , (uint16)12 }, { xAttrINT::HV5 , (uint16)12 },
    { xAttrINT::HY6 , (uint16)12 }, { xAttrINT::HU6 , (uint16)12 }, { xAttrINT::HV6 , (uint16)12 },
    { xAttrINT::HY7 , (uint16)12 }, { xAttrINT::HU7 , (uint16)12 }, { xAttrINT::HV7 , (uint16)12 },
    { xAttrINT::HY8 , (uint16)12 }, { xAttrINT::HU8 , (uint16)12 }, { xAttrINT::HV8 , (uint16)12 },
    { xAttrINT::HY9 , (uint16)12 }, { xAttrINT::HU9 , (uint16)12 }, { xAttrINT::HV9 , (uint16)12 },
    { xAttrINT::HY10, (uint16)12 }, { xAttrINT::HU10, (uint16)12 }, { xAttrINT::HV10, (uint16)12 },
    { xAttrINT::HY11, (uint16)12 }, { xAttrINT::HU11, (uint16)12 }, { xAttrINT::HV11, (uint16)12 },
    { xAttrINT::HY12, (uint16)12 }, { xAttrINT::HU12, (uint16)12 }, { xAttrINT::HV12, (uint16)12 },
    { xAttrINT::HY13, (uint16)12 }, { xAttrINT::HU13, (uint16)12 }, { xAttrINT::HV13, (uint16)12 },
    { xAttrINT::HY14, (uint16)12 }, { xAttrINT::HU14, (uint16)12 }, { xAttrINT::HV14, (uint16)12 },
    { xAttrINT::HY15, (uint16)12 }, { xAttrINT::HU15, (uint16)12 }, { xAttrINT::HV15, (uint16)12 }
  };
  std::unordered_map<xAttrINT, std::string> m_AttributeString =
  {
    { xAttrINT::PX  , "PX"   }, { xAttrINT::PY  , "PY"   }, { xAttrINT::PZ  , "PZ"   },
    { xAttrINT::SX  , "SX"   }, { xAttrINT::SY  , "SY"   }, { xAttrINT::SZ  , "SZ"   },
    { xAttrINT::RX  , "RX"   }, { xAttrINT::RY  , "RY"   }, { xAttrINT::RZ  , "RZ"   },
    { xAttrINT::OP  , "OP"   },
    { xAttrINT::HY0 , "CY"   }, { xAttrINT::HU0 , "CU"   }, { xAttrINT::HV0 , "CV"   },
    { xAttrINT::HY1 , "HY1"  }, { xAttrINT::HU1 , "HU1"  }, { xAttrINT::HV1 , "HV1"  },
    { xAttrINT::HY2 , "HY2"  }, { xAttrINT::HU2 , "HU2"  }, { xAttrINT::HV2 , "HV2"  },
    { xAttrINT::HY3 , "HY3"  }, { xAttrINT::HU3 , "HU3"  }, { xAttrINT::HV3 , "HV3"  },
    { xAttrINT::HY4 , "HY4"  }, { xAttrINT::HU4 , "HU4"  }, { xAttrINT::HV4 , "HV4"  },
    { xAttrINT::HY5 , "HY5"  }, { xAttrINT::HU5 , "HU5"  }, { xAttrINT::HV5 , "HV5"  },
    { xAttrINT::HY6 , "HY6"  }, { xAttrINT::HU6 , "HU6"  }, { xAttrINT::HV6 , "HV6"  },
    { xAttrINT::HY7 , "HY7"  }, { xAttrINT::HU7 , "HU7"  }, { xAttrINT::HV7 , "HV7"  },
    { xAttrINT::HY8 , "HY8"  }, { xAttrINT::HU8 , "HU8"  }, { xAttrINT::HV8 , "HV8"  },
    { xAttrINT::HY9 , "HY9"  }, { xAttrINT::HU9 , "HU9"  }, { xAttrINT::HV9 , "HV9"  },
    { xAttrINT::HY10, "HY10" }, { xAttrINT::HU10, "HU10" }, { xAttrINT::HV10, "HV10" },
    { xAttrINT::HY11, "HY11" }, { xAttrINT::HU11, "HU11" }, { xAttrINT::HV11, "HV11" },
    { xAttrINT::HY12, "HY12" }, { xAttrINT::HU12, "HU12" }, { xAttrINT::HV12, "HV12" },
    { xAttrINT::HY13, "HY13" }, { xAttrINT::HU13, "HU13" }, { xAttrINT::HV13, "HV13" },
    { xAttrINT::HY14, "HY14" }, { xAttrINT::HU14, "HU14" }, { xAttrINT::HV14, "HV14" },
    { xAttrINT::HY15, "HY15" }, { xAttrINT::HU15, "HU15" }, { xAttrINT::HV15, "HV15" }
  };



  /*std::unordered_map<std::string, xAttrINT> m_StringAttribute =
  {
      { "PX" , xAttrINT::PX }, { "PY" , xAttrINT::PY }, {  "PZ" , xAttrINT::PZ },
      { "SX" , xAttrINT::SX }, { "SY" , xAttrINT::SY }, {  "SZ" , xAttrINT::SZ },
      { "RX" , xAttrINT::RX }, { "RY" , xAttrINT::RY }, {  "RZ" , xAttrINT::RZ },
      { "OP" , xAttrINT::OP },
      { "CY0", xAttrINT::CY }, { "CU0", xAttrINT::CU }, {  "CV0", xAttrINT::CV },
      { "HY" , xAttrINT::HY }, { "HU" , xAttrINT::HU }, {  "HV" , xAttrINT::HV }
  };*/

public:
  void    setPictureSize(int32V2 PictureSize           )       { m_PictureSize = PictureSize; }
  int32V2 getPictureSize(                              ) const { return m_PictureSize; }
  void    setBitDepth   (uint16 BitDepth, xAttrINT Attr)       { m_AttributeBPS[Attr] = BitDepth; }
  uint16  getBitDepth   (xAttrINT Attr                 ) const { return m_AttributeBPS.at(Attr); }
  void    setBlockSize  (int32 BlockSize               )       { m_BlockSize = BlockSize; }
  int32   getBlockSize  (                              ) const { return m_BlockSize; }
  void    setGroupCount (int32 GroupCount              )       { m_GroupCount = GroupCount; }
  int32   getGroupCount (                              ) const { return m_GroupCount; }
  void    setMaxSeqNumSplats(int32 MaxSeqNumSplats) { m_MaxSeqNumSplats = MaxSeqNumSplats; }
  int32   getMaxSeqNumSplats() const { return m_MaxSeqNumSplats; }

  std::vector<flt32> getParamWeights() 
  { 
    std::vector<float> paramWeights;
    for (int i = 0; i < 3; i++) paramWeights.push_back(4.0);  //P 4.0
    for (int i = 0; i < 3; i++) paramWeights.push_back(1.0);  //S 1.0
    for (int i = 0; i < 3; i++) paramWeights.push_back(0.75); //R 0.75
    paramWeights.push_back(2.0); //O 1.0
    for (int i = 0; i < 3; i++) paramWeights.push_back(1.0); //C 1.0
    for (int i = 0; i < 3; i++) //shcount
    {
      paramWeights.push_back(0.25); //HY 1.0
      paramWeights.push_back(0.0 ); //HU
      paramWeights.push_back(0.0 ); //HV
    }
    for (int i = 0; i < 5; i++) //shcount
    {
        paramWeights.push_back(0.125); //HY 1.0
        paramWeights.push_back(0.0  ); //HU
        paramWeights.push_back(0.0  ); //HV
    }
    for (int i = 0; i < 7; i++) //shcount
    {
        paramWeights.push_back(0.0625); //HY 1.0
        paramWeights.push_back(0.0); //HU
        paramWeights.push_back(0.0); //HV
    }
    return paramWeights;
  }

  void    setEnabledColorScaling(bool EnabledColorScaling) { m_EnabledColorScaling = EnabledColorScaling; }
  bool    getEnabledColorScaling(                        ) { return m_EnabledColorScaling; }

  void        setYuvInputDir (std::string YuvDir)       { m_YuvFileInputDir = YuvDir; }
  void        setYuvOutputDir(std::string YuvDir)       { m_YuvFileOutputDir = YuvDir; }
  std::string getYuvInputDir(                  ) const { return m_YuvFileInputDir; }
  std::string getYuvOutputDir(                  ) const { return m_YuvFileOutputDir; }

  std::string Attr2Str(xAttrINT Attr) const { return m_AttributeString.at(Attr); }
};

constexpr std::array<flt64, 16> cSHCoeff =
{
   0.28209479177387814,
   0.4886025119029199 ,
   0.4886025119029199 ,
   0.4886025119029199 ,
   1.0925484305920792 ,
  -1.0925484305920792 ,
   0.31539156525252005,
  -1.0925484305920792 ,
   0.5462742152960396 ,
  -0.5900435899266435 ,
   2.890611442640554  ,
  -0.4570457994644658 ,
   0.3731763325901154 ,
  -0.4570457994644658 ,
   1.445305721320277  ,
  -0.5900435899266435 ,
};

static const std::unordered_map<xAttrINT, xAttrFLT> cAttrINT2FLT =
{
  {xAttrINT::PX, xAttrFLT::PX}, {xAttrINT::PY, xAttrFLT::PY}, {xAttrINT::PZ, xAttrFLT::PZ},
  {xAttrINT::SX, xAttrFLT::SX}, {xAttrINT::SY, xAttrFLT::SY}, {xAttrINT::SZ, xAttrFLT::SZ},
  {xAttrINT::RX, xAttrFLT::RX}, {xAttrINT::RY, xAttrFLT::RY}, {xAttrINT::RZ, xAttrFLT::RZ},
  {xAttrINT::OP, xAttrFLT::OP},

  {xAttrINT::HY0,  xAttrFLT::HR0}, {xAttrINT::HU0,  xAttrFLT::HG0}, {xAttrINT::HV0,  xAttrFLT::HB0},
  {xAttrINT::HY1,  xAttrFLT::HR1}, {xAttrINT::HU1,  xAttrFLT::HG1}, {xAttrINT::HV1,  xAttrFLT::HB1},
  {xAttrINT::HY2,  xAttrFLT::HR2}, {xAttrINT::HU2,  xAttrFLT::HG2}, {xAttrINT::HV2,  xAttrFLT::HB2},
  {xAttrINT::HY3,  xAttrFLT::HR3}, {xAttrINT::HU3,  xAttrFLT::HG3}, {xAttrINT::HV3,  xAttrFLT::HB3},
  {xAttrINT::HY4,  xAttrFLT::HR4}, {xAttrINT::HU4,  xAttrFLT::HG4}, {xAttrINT::HV4,  xAttrFLT::HB4},
  {xAttrINT::HY5,  xAttrFLT::HR5}, {xAttrINT::HU5,  xAttrFLT::HG5}, {xAttrINT::HV5,  xAttrFLT::HB5},
  {xAttrINT::HY6,  xAttrFLT::HR6}, {xAttrINT::HU6,  xAttrFLT::HG6}, {xAttrINT::HV6,  xAttrFLT::HB6},
  {xAttrINT::HY7,  xAttrFLT::HR7}, {xAttrINT::HU7,  xAttrFLT::HG7}, {xAttrINT::HV7,  xAttrFLT::HB7},
  {xAttrINT::HY8,  xAttrFLT::HR8}, {xAttrINT::HU8,  xAttrFLT::HG8}, {xAttrINT::HV8,  xAttrFLT::HB8},
  {xAttrINT::HY9,  xAttrFLT::HR9}, {xAttrINT::HU9,  xAttrFLT::HG9}, {xAttrINT::HV9,  xAttrFLT::HB9},
  {xAttrINT::HY10, xAttrFLT::HR10}, {xAttrINT::HU10, xAttrFLT::HG10}, {xAttrINT::HV10, xAttrFLT::HB10},
  {xAttrINT::HY11, xAttrFLT::HR11}, {xAttrINT::HU11, xAttrFLT::HG11}, {xAttrINT::HV11, xAttrFLT::HB11},
  {xAttrINT::HY12, xAttrFLT::HR12}, {xAttrINT::HU12, xAttrFLT::HG12}, {xAttrINT::HV12, xAttrFLT::HB12},
  {xAttrINT::HY13, xAttrFLT::HR13}, {xAttrINT::HU13, xAttrFLT::HG13}, {xAttrINT::HV13, xAttrFLT::HB13},
  {xAttrINT::HY14, xAttrFLT::HR14}, {xAttrINT::HU14, xAttrFLT::HG14}, {xAttrINT::HV14, xAttrFLT::HB14},
  {xAttrINT::HY15, xAttrFLT::HR15}, {xAttrINT::HU15, xAttrFLT::HG15}, {xAttrINT::HV15, xAttrFLT::HB15}
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB