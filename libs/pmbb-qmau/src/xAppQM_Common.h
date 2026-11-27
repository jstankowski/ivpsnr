/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefIVQM.h"
#include "xUtilsAppQM.h"
#include "xMetricsAppQM.h"
#include "xCfgINI.h"
#include "xSeq.h"
#include "xGlobClrDiff.h"
#include "xShftCompPic.h"
#include "xIVPSNR.h"
#include "xIVSSIM.h"
#include "xPVD.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xAppQM
{
public:
  static constexpr int32 c_MetricsNum = (size_t)eMetric::__NUM;
  static constexpr int32 c_NumPics    = 2;
  using tStr     = std::string;
  using tMetrSel = std::array<bool, c_MetricsNum>;

protected:
  xCfgINI::xParser m_CfgParser;
  tStr             m_ErrorLog;

protected:
  //basic io
  std::string  m_InputFile[c_NumPics];
  int32V2      m_PictureSize;
  int32        m_BitDepth;
  eCrF         m_ChromaFormat;  
  int32        m_StartFrame[c_NumPics];
  int32        m_NumberOfFrames;
  std::string  m_ResultFile;
  tMetrSel     m_CalcMetric = { false };
  //auxliary io
  std::string  m_ShftCompPicFile[c_NumPics];
  //mask io  
  int32        m_BitDepthM     ;  
  eMskTrMd     m_MaskTransMode ;
  int32        m_MaskTransParam;
  //erp 
  bool         m_IsEquirectangular = false    ;
  int32        m_LonRangeDeg       = NOT_VALID;
  int32        m_LatRangeDeg       = NOT_VALID;
  //iv-specific
  int32        m_ShftCompPicRange = NOT_VALID;
  flt32V4      m_UnnoticeableCoef ;
  int32V4      m_CmpWeightsSearch ;
  int32V4      m_CmpWeightsAverage;
  //ssim specific
  xSSIM::eMode m_StructSimMode;
  eMrgExt      m_StructSimBrdExt;
  int32        m_StructSimStride;
  int32        m_StructSimWindow;
  //validation 
  eActn        m_InvalidPelActn;
  eActn        m_NameMismatchActn;
  //operation
  int32        m_NumberOfThreads;
  int32        m_VerboseLevel;
  bool         m_InterleavedPic = true;
  bool         m_DebugDump      = false;
  //derrived
  bool         m_UseMask    = false;
  bool         m_WriteSCP   = false;
  bool         m_CalcPSNRs  = false;
  bool         m_CalcSSIMs  = false;
  bool         m_CalcPVDs   = false;
  bool         m_CalcIVs    = false;
  bool         m_CalcMSs    = false;
  bool         m_CalcGCD    = false;
  bool         m_CalcSCP    = false;
  int32        m_PicMargin  = NOT_VALID;
  int32        m_WindowSize = NOT_VALID;

  bool         m_PrintFrame = false;
  bool         m_GatherTime = false;
  bool         m_PrintDebug = false;


protected:
  //multithreading
  int32        m_HardwareConcurency;
  int32        m_NumberOfThreadsUsed;
  xThreadPool* m_ThreadPool = nullptr;
  tThPI        m_TPI; //thread pool interface


protected:
  //processing data
  int32 m_NumFrames = NOT_VALID;
  int32 m_NumViews  = NOT_VALID;
  //merics data & stats
  std::array<xMetricStat, c_MetricsNum> m_MetricData;

  //processors
  xGlobClrDiffProc m_ProcGCD;
  xShftCompPic     m_ProcSCP;
  xIVPSNR          m_ProcPSNR;
  xIVSSIM          m_ProcSSIM;
  xIVPVD           m_ProcPVD;

  //sequences
  std::array<xSeq, c_NumPics> m_SeqPicSCP; //0=Tst,1=Ref
  xSeqPic*                    m_SeqMskIn ;

  //buffers
  std::array<xPicP, c_NumPics> m_PicInP   ; //0=Tst,1=Ref
  std::array<xPicP, c_NumPics> m_PicSCP   ; //0=Tst,1=Ref
  std::array<xPicP, c_NumPics> m_PicOutP  ; //0=Tst,1=Ref 
  xPicP                        m_PicMskInP; //TODO - replace by xPlane<uint16>

  //merics data & stats
  xTimeStamp m_TimeStamp;

  uint64 m_Ticks____Load = 0;
  uint64 m_TicksValidate = 0;
  uint64 m_TicksPreprocF = 0;
  uint64 m_TicksPreprocM = 0;
  uint64 m_Ticks__Margin = 0;
  uint64 m_Ticks_____GCD = 0;
  uint64 m_Ticks_____SCP = 0;
  uint64 m_TicksWriteSCP = 0;

public:
  //setup
  static const std::string_view c_HelpStringTopRow  ;
  static const std::string_view c_HelpStringDispatch;
  static const std::string_view c_HelpStringMetrics ;
  static const std::string_view c_HelpStringCmnIV   ;
  static const std::string_view c_HelpStringSSIM    ;
  static const std::string_view c_HelpStringValOp   ;
  void registerCmdParamsDispatch();
  void registerCmdParamsCmnIV   ();
  void registerCmdParamsSSIM    ();
  void registerCmdParamsValOp   ();
  bool loadConfiguration(int argc, const char* argv[]);

  bool readConfigurationMetric ();
  bool readConfigurationCmnIV  ();
  bool readConfigurationSSIM   ();
  bool readConfigurationValOp  ();
  bool derriveConfigurationCmn ();
  bool validateConfigurationCmn();

  void formatConfigurationMetrics(std::string& ConfigInfo);
  void formatConfigurationCmnIV  (std::string& ConfigInfo);
  void formatConfigurationSSIM   (std::string& ConfigInfo);
  void formatConfigurationValOp  (std::string& ConfigInfo);

  void formatMetricDescriptions  (std::string& ConfigInfo);

  tStr formatWarningsCmn();

  //multithreading
  void          setupMultithreading         ();
  void          ceaseMultithreading         ();
  virtual int32 determinePreferredNumThreads() = 0;
  virtual int32 determineMaxNumTasks        () = 0;
  tStr          formatMultithreading        ();

  //processors
  void createMetricProcessors ();
  void destroyMetricProcessors();

  //intermediates
  boolV4  m_ExactCmps    = xMakeVec4<bool>(false);
  int32   m_NumNonMasked = 0;
  int32V4 m_GCD_R2T;

  //debug data
  flt64   m_LastR2T = 0;
  flt64   m_LastT2R = 0;

  //metric calculation & other
  void calcMetricsCmn   (int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpcMtr);

  void preprocessMaskCmn(int32 FrameIdx);
  void addStructSimMargs(int32 FrameIdx);
  void calcFrameGCD     (int32 FrameIdx, int32 ViewIdx);
  void calcFrameSCP     (int32 FrameIdx, int32 ViewIdx);
  void calcFrame_____MSE(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc);
  void calcFrame____PSNR(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc);
  void calcFrame__WSPSNR(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc);
  void calcFrame__IVPSNR(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc);
  void calcFrame____SSIM(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc);
  void calcFrame__MSSSIM(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc);
  void calcFrame__IVSSIM(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc);
  void calcFrameIVMSSSIM(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc);
#if X_PMBB_STALLED
  void calcFrameMSIVSSIM(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc);
#endif //X_PMBB_STALLED
  void calcFrame_____PVD(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc);
#if X_PMBB_EXPERIMENTAL
  void calcFrame___IVPVD(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc);
#endif //X_PMBB_EXPERIMENTAL  

  //stats
  tStr calibrateTimeStamp   ();
  tStr formatMetricTimeStats(flt64 InvDurationDenominator);

public:
  const tStr& getErrorLog    () { return m_ErrorLog    ; }
  int32       getVerboseLevel() { return m_VerboseLevel; }
  const tStr& getResultFile  () { return m_ResultFile  ; }

  bool getCalcMetric(eMetric Metric) const { return m_CalcMetric[(int32)Metric]; }

protected:
  std::string xFormatIdx(int32 FrameIdx, int32 ViewIdx);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB