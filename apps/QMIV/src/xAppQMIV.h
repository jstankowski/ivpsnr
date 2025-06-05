/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "xFile.h"
#include "xSeq.h"
#include "xIVPSNR.h"
#include "xIVSSIM.h"
#include "xCfgINI.h"
#include "xFmtScn.h"
#include "xMemory.h"
#include "xMiscUtilsCORE.h"
#include "xKBNS.h"
#include "xShftCompPic.h"
#include "xTimeUtils.h"
#include <math.h>
#include <fstream>
#include <time.h>
#include <limits>
#include <numeric>
#include <cassert>
#include <thread>
#include <filesystem>
#include "fmt/chrono.h"
#include "xUtilsQMIV.h"
#include "xMetricQMIV.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xAppQMIV
{
public:
  static const std::string_view c_BannerString;
  static const std::string_view c_HelpString  ;

  static constexpr int32 c_MetricsNum = (size_t)eMetric::__NUM;

protected:
  xCfgINI::xParser m_CfgParser;
  std::string      m_ErrorLog;

protected:
  static constexpr int32 NumInputsSeq = 2;
  static constexpr int32 NumInputsMax = 3;
  static constexpr int32 NumOutputMax = 2;

public:
  //basic io
  std::string m_InputFile[NumInputsMax];
  eFileFmt    m_FileFormat;
  int32V2     m_PictureSize;
  int32       m_BitDepth;
  eCrF        m_ChromaFormat;
  int32       m_StartFrame[NumInputsSeq];
  int32       m_NumberOfFrames;
  std::string m_OutputFile[NumOutputMax];
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
  int32       m_SearchRange      ;
  int32V4     m_CmpWeightsSearch ;
  int32V4     m_CmpWeightsAverage;
  flt32V4     m_UnnoticeableCoef ;
  //ssim specific
  xSSIM::eMode m_StructSimMode  ;
  //eMrgExt      m_StructSimBrdExt;  - not ready jet
  int32        m_StructSimStride;
  int32        m_StructSimWindow;
  //validation 
  eActn       m_InvalidPelActn;
  eActn       m_NameMismatchActn;
  //operation
  int32       m_NumberOfThreads;
  bool        m_InterleavedPic = true;
  int32       m_VerboseLevel;
  //derrived
  bool        m_UseMask;
  bool        m_FileFormatRGB;
  bool        m_CvtYCbCr2RGB;
  bool        m_CvtRGB2YCbCr;
  bool        m_ReorderRGB;
  bool        m_InputRGB;  
  int32       m_NumInputsCur;  
  bool        m_WriteSCP = false;
  bool        m_CalcPSNRs;
  bool        m_CalcSSIMs;
  bool        m_CalcIVs;
  bool        m_CalcMSs;
  bool        m_CalcGCD;
  bool        m_CalcSCP;
  bool        m_UsePicI;
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
  std::array<xSeqPic*, NumInputsMax> m_SeqIn  ; //0=Tst,1=Ref,2=Msk
  std::array<xPicP   , NumInputsMax> m_PicInP ; //0=Tst,1=Ref,2=Msk
  std::array<xPicI   , NumInputsSeq> m_PicInI ; //0=Tst,1=Ref
  std::array<xPicP   , NumInputsSeq> m_PicSCP ; //0=Tst,1=Ref
  std::array<xPicI   , NumInputsSeq> m_PicSCI ; //0=Tst,1=Ref
  std::array<xPicP   , NumOutputMax> m_PicOutP; //0=Tst,1=Ref 
  std::array<xSeq    , NumInputsMax> m_SeqOut ; //0=Tst,1=Ref,2=Msk

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

  xTimeStamp m_TimeStamp;

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
  eAppRes     validateInputFiles  ();
  std::string formatWarnings      ();

  void        setupMultithreading ();
  void        ceaseMultithreading ();
  std::string formatMultithreading();
  

  eAppRes     setupSeqAndBuffs ();
  eAppRes     ceaseSeqAndBuffs ();

  void        createProcessors ();
  void        destroyProcessors();

  eAppRes     processAllFrames ();

  eAppRes     validateFrames   (int32 FrameIdx);
  void        preprocessFrames (int32 FrameIdx);
  void        calcFrameGCD     (int32 FrameIdx);
  void        calcFrameSCP     (int32 FrameIdx);
  void        calcFrame_____MSE(int32 FrameIdx);
  void        calcFrame____PSNR(int32 FrameIdx);
  void        calcFrame__WSPSNR(int32 FrameIdx);
  void        calcFrame__IVPSNR(int32 FrameIdx);
  void        calcFrame____SSIM(int32 FrameIdx);
  void        calcFrame__MSSSIM(int32 FrameIdx);
  void        calcFrame__IVSSIM(int32 FrameIdx);
  void        calcFrameIVMSSSIM(int32 FrameIdx);

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