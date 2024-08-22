/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "xFile.h"
#include "xSeq.h"
#include "xIVPSNR.h"
#include "xSSIM.h"
#include "xCfgINI.h"
#include "xFmtScn.h"
#include "xMemory.h"
#include "xMiscUtilsCORE.h"
#include "xMathUtils.h"
#include "xShftCompPic.h"
#include <math.h>
#include <fstream>
#include <time.h>
#include <limits>
#include <numeric>
#include <cassert>
#include <thread>
#include <filesystem>
#include "fmt/chrono.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

enum class eRes : int32
{
  Unknown = -1,
  Good        ,
  Warning     ,
  Error        
};

enum class eFileFmt : int32
{
  INVALID = -1,
  RAW,
  PNG,
};

static inline eFileFmt xStr2FileFmt(const std::string& FileFmt)
{
  std::string FileFmtU = xString::toUpper(FileFmt);
  return FileFmt=="RAW" ? eFileFmt::RAW    :
         FileFmt=="PNG" ? eFileFmt::PNG    :
                          eFileFmt::INVALID;
}
static inline std::string xFileFmt2Str(eFileFmt FileFmt)
{
  return FileFmt==eFileFmt::RAW ? "RAW"    :
         FileFmt==eFileFmt::PNG ? "PNG"    :
                                  "INVALID";
}

enum class eClrSpcApp : int32
{
  INVALID         = -1,
  //RGBs
  RGB             = 2,
  BGR             = 3,
  GBR             = 4,  
  //generic YCbCr
  YCbCr           = 16,
  //standardized YCbCr
  YCbCr_BT601     = 17,
  YCbCr_SMPTE170M = 18,
  YCbCr_BT709     = 19,
  YCbCr_SMPTE240M = 20,
  YCbCr_BT2020    = 21,
};

static inline eClrSpcApp xStr2ClrSpcApp(const std::string& ClrSpc)
{
  std::string ClrSpcU = xString::toUpper(ClrSpc);
  return ClrSpc=="RGB"             ? eClrSpcApp::RGB             :
         ClrSpc=="BGR"             ? eClrSpcApp::BGR             :
         ClrSpc=="GBR"             ? eClrSpcApp::GBR             :
         ClrSpc=="YCbCr"           ? eClrSpcApp::YCbCr           :
         ClrSpc=="YCbCr_BT601"     ? eClrSpcApp::YCbCr_BT601     :
         ClrSpc=="YCbCr_SMPTE170M" ? eClrSpcApp::YCbCr_SMPTE170M :
         ClrSpc=="YCbCr_BT709"     ? eClrSpcApp::YCbCr_BT709     :
         ClrSpc=="YCbCr_SMPTE240M" ? eClrSpcApp::YCbCr_SMPTE240M :
         ClrSpc=="YCbCr_BT2020"    ? eClrSpcApp::YCbCr_BT2020    :
                                     eClrSpcApp::INVALID;
}
static inline std::string xClrSpcApp2Str(eClrSpcApp ClrSpc)
{
  return ClrSpc==eClrSpcApp::RGB             ? "RGB"             :
         ClrSpc==eClrSpcApp::BGR             ? "BGR"             :
         ClrSpc==eClrSpcApp::GBR             ? "GBR"             :
         ClrSpc==eClrSpcApp::YCbCr           ? "YCbCr"           :
         ClrSpc==eClrSpcApp::YCbCr_BT601     ? "YCbCr_BT601"     :
         ClrSpc==eClrSpcApp::YCbCr_SMPTE170M ? "YCbCr_SMPTE170M" :
         ClrSpc==eClrSpcApp::YCbCr_BT709     ? "YCbCr_BT709"     :
         ClrSpc==eClrSpcApp::YCbCr_SMPTE240M ? "YCbCr_SMPTE240M" :
         ClrSpc==eClrSpcApp::YCbCr_BT2020    ? "YCbCr_BT2020"    :
                                               "INVALID"         ;
}

static inline eClrSpcLC xClrSpcAppToClrSpc(eClrSpcApp ClrSpcApp)
{
  return ClrSpcApp==eClrSpcApp::YCbCr_BT601     ? eClrSpcLC::BT601     :
         ClrSpcApp==eClrSpcApp::YCbCr_SMPTE170M ? eClrSpcLC::SMPTE170M :
         ClrSpcApp==eClrSpcApp::YCbCr_BT709     ? eClrSpcLC::BT709     :
         ClrSpcApp==eClrSpcApp::YCbCr_SMPTE240M ? eClrSpcLC::SMPTE240M :
         ClrSpcApp==eClrSpcApp::YCbCr_BT2020    ? eClrSpcLC::BT2020    :
                                                  eClrSpcLC::INVALID   ;
}

static inline bool isRGB         (eClrSpcApp ClrSpc) { return ClrSpc == eClrSpcApp::RGB || ClrSpc == eClrSpcApp::BGR || ClrSpc == eClrSpcApp::GBR; }
static inline bool isYCbCr       (eClrSpcApp ClrSpc) { return (int32)ClrSpc >= (int32)eClrSpcApp::YCbCr; }
static inline bool isDefinedYCbCr(eClrSpcApp ClrSpc) { return (int32)ClrSpc >  (int32)eClrSpcApp::YCbCr; }

enum class eMetric : int32 //values must start from 0 and be continous
{
  UNDEFINED = -1,
  //PSNR - based
      PSNR  = 0,
    WSPSNR,
    IVPSNR,
  //SSIM - based
      SSIM,
    MSSSIM,
    IVSSIM,
  //must be after last metric
  __NUM  
};

static inline eMetric xStrToMetric(const std::string_view Metric)
{
  std::string MetricU = xString::toUpper(Metric);
  return MetricU ==     "PSNR" ? eMetric::    PSNR :
         MetricU ==   "WSPSNR" ? eMetric::  WSPSNR :
         MetricU ==   "IVPSNR" ? eMetric::  IVPSNR :
         MetricU ==     "SSIM" ? eMetric::    SSIM :
         MetricU ==   "MSSSIM" ? eMetric::  MSSSIM :
         MetricU ==   "IVSSIM" ? eMetric::  IVSSIM :
                                 eMetric::UNDEFINED;
}
static inline std::string xMetricToStr(eMetric Metric)
{
  return Metric == eMetric::    PSNR ?     "PSNR" :
         Metric == eMetric::  WSPSNR ?   "WSPSNR" :
         Metric == eMetric::  IVPSNR ?   "IVPSNR" :
         Metric == eMetric::    SSIM ?     "SSIM" :
         Metric == eMetric::  MSSSIM ?   "MSSSIM" :
         Metric == eMetric::  IVSSIM ?   "IVSSIM" :
                                       "UNDEFINED";
}

struct xMetricInfo
{
  static constexpr int32 MetricsNum       = (int32)eMetric::__NUM;
  static constexpr int32 MaxMetricNameLen = 8;

                                                          //     PSNR   WSPSNR IVPSNR SSIM   MSSSIM IVSSIM 
  static constexpr bool             IsPerCmp    [MetricsNum] = { true , true , false, true , true , false  };
  static constexpr bool             IsPerPic    [MetricsNum] = { false, false, true , false, false, true   };
  static constexpr bool             IsNormalized[MetricsNum] = { false, false, false, true , true , true   };
  static constexpr std::string_view Unit        [MetricsNum] = { "dB" , "dB" , "dB" , "  " , "  " , "  "   };
  static constexpr std::string_view Description [MetricsNum] =
  {
    "Peak Signal-to-Noise Ratio",
    "Spherical Weighted - Peak Signal-to-Noise Ratio",
    "Immersive Video - Peak Signal-to-Noise Ratio",
    "Structural Similarity Index Measure",
    "Multi Scale Structural Similarity Index Measure",
    "Immersive Video - Structural Similarity Index Measure",
  };
};

class xMetricStat
{
protected:
  eMetric     m_Metric    = eMetric::UNDEFINED;
  std::string m_SuffixCmp = "";
  std::string m_SuffixPic = "";
  int32V4     m_CmpWeightsAverage = { 0, 0, 0, 0 };
  flt64       m_CmpWeightAverageInvDenom = 0.0;
  bool        m_MaskMode = false;
  uint64      m_SumTicks = 0;
  tDurationMS m_AvgDuration;
  std::vector<flt64V4> m_ValCmp;
  std::vector<flt64  > m_ValPic;
  flt64V4     m_AvgCmp;
  flt64       m_AvgPic;  
  bool        m_AnyFake  = false;
  bool        m_Enabled  = false;

public:
  eMetric getMetric () const { return m_Metric ; }
  bool    getEnabled() const { return m_Enabled; }

  void initMetric(eMetric Metric, int32 NumFrames)
  {
    m_Metric    = Metric;
    m_SuffixCmp = "";
    m_SuffixPic = "";
    m_SumTicks  = 0;
    m_AvgDuration = tDurationMS(0);
    constexpr flt64 InitValue = std::numeric_limits<flt64>::quiet_NaN();
    bool IsPerCmp = xMetricInfo::IsPerCmp[(int32)Metric];
    if(IsPerCmp) { m_ValCmp.resize(NumFrames, xMakeVec4(InitValue)); }
    m_ValPic.resize(NumFrames, InitValue);    
    m_AvgPic   = InitValue;
    m_AvgCmp   = xMakeVec4(InitValue);
    m_AnyFake  = false;
    m_Enabled  = true;
  }
  void initSuffixes(bool UseMask, bool UseRGB)
  {
    if     (UseMask && UseRGB) { m_SuffixCmp = "-M R:G:B  "; }
    else if(UseMask          ) { m_SuffixCmp = "-M Y:Cb:Cr"; }
    else if(UseRGB           ) { m_SuffixCmp = " R:G:B    "; }
    else                       { m_SuffixCmp = " Y:Cb:Cr  "; }

    bool IsPerPic = xMetricInfo::IsPerPic[(int32)m_Metric];
    if(IsPerPic)
    {
      if     (UseMask && UseRGB) { m_SuffixPic = "-M-RGB    "; }
      else if(UseMask          ) { m_SuffixPic = "-M        "; }
      else if(UseRGB           ) { m_SuffixPic = "-RGB      "; }
      else                       { m_SuffixPic = "          "; }
    }
    else
    {
      if     (UseMask && UseRGB) { m_SuffixPic = "-M-RGB    "; }
      else if(UseMask          ) { m_SuffixPic = "-M-YCbCr  "; }
      else if(UseRGB           ) { m_SuffixPic = "-RGB      "; }
      else                       { m_SuffixPic = "-YCbCr    "; }
    }
  }

  void initCmpWeightsAverage(const int32V4& CmpWeightsAverage)
  {
    m_CmpWeightsAverage = CmpWeightsAverage;
    const int32 SumCmpWeightAverage = CmpWeightsAverage.getSum();
    m_CmpWeightAverageInvDenom      = 1.0 / (flt64)SumCmpWeightAverage;
  }
  void setPerCmpMeric(const flt64V4& PerCmpMetric, int32 FrameIdx)
  { 
    m_ValCmp[FrameIdx] = PerCmpMetric;
    m_ValPic[FrameIdx] = (m_ValCmp[FrameIdx][0] * m_CmpWeightsAverage[0]
                        + m_ValCmp[FrameIdx][1] * m_CmpWeightsAverage[1]
                        + m_ValCmp[FrameIdx][2] * m_CmpWeightsAverage[2]) * m_CmpWeightAverageInvDenom;
  }
  void setPerPicMeric(flt64 PerPicMetric, int32 FrameIdx) { m_ValPic[FrameIdx] = PerPicMetric; }
  void setAnyFake    (bool AnyFake) { m_AnyFake = AnyFake; }
  void addTicks      (uint64 DurationTicks) { m_SumTicks += DurationTicks; }

  void calcAvgMetric(int32 NumFrames)
  {
    if(!m_ValPic.empty()) { m_AvgPic = xKBNS::Accumulate(m_ValPic) / (flt64)NumFrames; }
    if(!m_ValCmp.empty()) { m_AvgCmp = xKBNS::Accumulate(m_ValCmp) / (flt64)NumFrames; }
  }

  std::string formatPerCmpMetric(int32 FrameIdx)
  {
    const std::string MetricName   = xMetricToStr(m_Metric);
    const bool        IsNormalized = xMetricInfo::IsNormalized[(int32)m_Metric];
    const std::string SingleFormat = IsNormalized ? "{:8.6f} " : "{:8.4f} ";

    std::string Result = fmt::format("{:>8}{} ", MetricName, m_SuffixCmp);    
    Result += fmt::format(SingleFormat + SingleFormat + SingleFormat, m_ValCmp[FrameIdx][0], m_ValCmp[FrameIdx][1], m_ValCmp[FrameIdx][2]);
    return Result;
  }

  std::string formatPerPicMetric(int32 FrameIdx)
  {
    const std::string MetricName   = xMetricToStr(m_Metric);
    const bool        IsNormalized = xMetricInfo::IsNormalized[(int32)m_Metric];
    const std::string SingleFormat = IsNormalized ? "{:8.6f} " : "{:8.4f} ";

    std::string Result = fmt::format("{:>8}{} ", MetricName, m_SuffixPic);    
    Result += fmt::format(SingleFormat + "                  ", m_ValPic[FrameIdx]);
    return Result;
  }

  std::string formatAvgMetric(const std::string LineHeader)
  {
    const std::string      MetricName   = xMetricToStr(m_Metric);
    const bool             IsPerCmp     = xMetricInfo::IsPerCmp[(int32)m_Metric];
    const bool             IsNormalized = xMetricInfo::IsNormalized[(int32)m_Metric];
    const std::string_view Unit         = xMetricInfo::Unit    [(int32)m_Metric];
    const std::string      NameFormat   = LineHeader.empty() ? "{:<8}{} " : "{:>8}{} ";
    const std::string      SingleFormat = IsNormalized ? "{:10.8f} {}  " : "{:10.6f} {}  ";

    std::string Result;

    if(IsPerCmp)
    {
      Result += LineHeader + fmt::format(NameFormat, MetricName, m_SuffixCmp);
      Result += fmt::format(SingleFormat + SingleFormat + SingleFormat, m_AvgCmp[0], Unit, m_AvgCmp[1], Unit, m_AvgCmp[2], Unit);
      Result += "\n";
    }
     
    Result += LineHeader + fmt::format(NameFormat, MetricName, m_SuffixPic);
    Result += fmt::format(SingleFormat + "                  ", m_AvgPic, Unit);
   
    return Result;
  }

  void calcAvgDuration(flt64 InvDurationDenominator)
  {
    m_AvgDuration = tDurationMS(m_SumTicks * InvDurationDenominator);
  }

  std::string formatAvgTime(const std::string LineHeader, tDurationMS PreMetricOps = tDurationMS(0))
  {
    const std::string      MetricName = xMetricToStr(m_Metric);
    const std::string      Suffix     = "";

    std::string Result = LineHeader;
    if(LineHeader.empty()) { Result += fmt::format("{:<8}{} ", MetricName, Suffix); }
    else                   { Result += fmt::format("{:>8}{} ", MetricName, Suffix); }    

    Result += fmt::format("{:9.2f} ms", m_AvgDuration.count());

    if(PreMetricOps.count() > 0) { Result += fmt::format("   Total {:7.2f} ms", (m_AvgDuration + PreMetricOps).count()); }

    return Result;
  }
};

//===============================================================================================================================================================================================================

class xAppQMIV
{
public:
  static const std::string_view c_BannerString;
  static const std::string_view c_HelpString;

  static constexpr int32 c_MetricsNum = (size_t)eMetric::__NUM;

protected:
  xCfgINI::xParser m_CfgParser;
  std::string      m_ErrorLog;

protected:
  static constexpr int32 NumInputsSeq = 2;
  static constexpr int32 NumInputsMax = 3;

public:
  //basic io
  std::string m_InputFile[NumInputsMax];
  eFileFmt    m_FileFormat;
  int32V2     m_PictureSize;
  int32       m_BitDepth;
  eCrF        m_ChromaFormat;
  int32       m_StartFrame[NumInputsSeq];
  int32       m_NumberOfFrames;
  std::string m_ResultFile;
  std::array<bool, c_MetricsNum> m_CalcMetric = { false };
  //mask io
  int32       m_BitDepthM;        
  eCrF        m_ChromaFormatM;
  //erp 
  bool        m_IsEquirectangular;
  int32       m_LonRangeDeg;
  int32       m_LatRangeDeg;
  //colorspace
  eClrSpcApp  m_ColorSpaceInput;
  eClrSpcApp  m_ColorSpaceMetric;
  //iv-specific
  int32       m_SearchRange;
  int32V4     m_CmpWeightsSearch;
  int32V4     m_CmpWeightsAverage;
  flt32V4     m_UnnoticeableCoef;
  //validation 
  eActn       m_InvalidPelActn;
  eActn       m_NameMismatchActn;
  //operation
  int32       m_NumberOfThreads;
  bool        m_InterleavedPic;
  int32       m_VerboseLevel;
  //derrived
  bool        m_UseMask;
  bool        m_UsePicI;
  int32       m_NumInputsCur;
  bool        m_CvtYCbCr2RGB;
  bool        m_CvtRGB2YCbCr;
  bool        m_ReorderRGB;
  bool        m_WriteSCP;
  bool        m_CalcPSNRs;
  bool        m_CalcSSIMs;
  bool        m_CalcIVs;
  bool        m_CalcGCD;
  bool        m_CalcSCP;
  int32       m_PicMargin;
  int32       m_WindowSize;
  bool        m_PrintFrame;
  bool        m_GatherTime;
  bool        m_PrintDebug;

protected:
  //multithreading
  int32        m_HardwareConcurency;
  int32        m_NumberOfThreadsUsed;
  xThreadPool* m_ThreadPool = nullptr;
  tThPI        m_TPI; //thread pool interface

protected:
  //processing data
  int32 m_NumFrames = 0;

  //sequences and buffers
  std::array<xSeqBase*, NumInputsMax> m_SeqIn  ; //0=Tst,1=Ref,2=Msk
  std::array<xPicP    , NumInputsMax> m_PicInP ; //0=Tst,1=Ref,2=Msk
  std::array<xPicI    , NumInputsSeq> m_PicInI ; //0=Tst,1=Ref
  std::array<xPicP    , NumInputsSeq> m_PicSCP ; //0=Tst,1=Ref

  //processors
  xGlobClrDiffProc m_ProcGCD;
  xShftCompPicProc m_ProcSCP;
  xIVPSNRM         m_ProcPSNR;
  xIVSSIM          m_ProcSSIM;

  //intermediates
  boolV4  m_ExactCmps    = xMakeVec4<bool>(false);
  int32   m_NumNonMasked = 0;
  int32V4 m_GCD_R2T;

  //debug data
  flt64   m_LastR2T = 0;
  flt64   m_LastT2R = 0;
  

  //merics data & stats
  std::array<xMetricStat, c_MetricsNum> m_MetricData;

  tTimePoint m_ProcBegTime  = tTimePoint::min();
  tTimePoint m_ProcEndTime  = tTimePoint::min();
  uint64     m_ProcBegTicks = 0;
  uint64     m_ProcEndTicks = 0;

  uint64 m_Ticks____Load = 0;
  uint64 m_TicksValidate = 0;
  uint64 m_Ticks_Preproc = 0;
  uint64 m_Ticks_____GCD = 0;
  uint64 m_Ticks_____SCP = 0;

  flt64  m_InvDurationDenominator = 0;

public:
  void        registerCmdParams   ();
  bool        loadConfiguration   (int argc, const char* argv[]);
  bool        readConfiguration   ();
  std::string formatConfiguration ();
  eRes        validateInputFiles  ();
  std::string formatWarnings      ();

  void        setupMultithreading ();
  void        ceaseMultithreading ();
  std::string formatMultithreading();
  

  eRes        setupSeqAndBuffs ();
  eRes        ceaseSeqAndBuffs ();

  void        createProcessors ();
  void        destroyProcessors();

  eRes        processAllFrames ();

  eRes        validateFrames   (int32 FrameIdx);
  void        preprocessFrames (int32 FrameIdx);
  void        calcFrame____PSNR(int32 FrameIdx);
  void        calcFrame__WSPSNR(int32 FrameIdx);
  void        calcFrame__IVPSNR(int32 FrameIdx);
  void        calcFrame____SSIM(int32 FrameIdx);
  void        calcFrame__MSSSIM(int32 FrameIdx);
  void        calcFrame__IVSSIM(int32 FrameIdx);

  std::string calibrateTimeStamp();
  void        combineFrameStats ();

  std::string formatResultsStdOut();
  std::string formatResultsFile  ();

public:
  const std::string& getErrorLog() { return m_ErrorLog; }
  int32 getVerboseLevel() { return m_VerboseLevel; }

  bool getCalcMetric(eMetric Metric) const { return m_CalcMetric[(int32)Metric]; }



};

//===============================================================================================================================================================================================================

} //end of namespace PMBB