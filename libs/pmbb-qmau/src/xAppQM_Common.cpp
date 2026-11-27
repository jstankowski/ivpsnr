/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xAppQM_Common.h"
#include "xMiscUtilsCORE.h"
#include "xUtilsAppQM.h"
#include "xSeq.h"
#include "xFmtScn.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

const std::string_view xAppQM::c_HelpStringTopRow = " Cmd | ParamName        | Description";

const std::string_view xAppQM::c_HelpStringDispatch =
R"PMBBRAWSTRING(usage::dynamic_dispatch -----------------------------------------------------
       DispatchForce
       DispatchVerbose )PMBBRAWSTRING";
const std::string_view xAppQM::c_HelpStringMetrics = 
R"PMBBRAWSTRING(
usage::metrics --------------------------------------------------------------
 -ml   MetricList         List of quality metrics to be calculated, must be coma separated,
                          quotes are required. "All" enables all available metrics.
                          [MSE, PSNR, WSPSNR, IVPSNR, SSIM, MSSSIM, IVSSIM, IVMSSSIM, )PMBBRAWSTRING"
#if X_PMBB_STALLED
  R"PMBBRAWSTRING(MSIVSSIM, )PMBBRAWSTRING"
#endif
R"PMBBRAWSTRING(PVD, )PMBBRAWSTRING"
#if X_PMBB_EXPERIMENTAL
R"PMBBRAWSTRING(IVPVD)PMBBRAWSTRING"
#endif //X_PMBB_EXPERIMENTAL
R"PMBBRAWSTRING(]
                          (optional, default="PSNR, IVPSNR, IVSSIM"))PMBBRAWSTRING";
const std::string_view xAppQM::c_HelpStringCmnIV =
R"PMBBRAWSTRING(
usage::IV_specific ----------------------------------------------------------
 -sr   SearchRange        IV-metric search range around center point (optional, default=2 --> 5x5)
 -unc  UnnoticeableCoef   IV-metric unnoticeable color difference threshold coeff "Lm:Cb:Cr:0" or "R:G:B:0"
                          (per component coeff, default="0.01:0.01:0.01:0", quotes are mandatory) )PMBBRAWSTRING";
const std::string_view xAppQM::c_HelpStringSSIM =
R"PMBBRAWSTRING(
usage::structural_similarity_specific ---------------------------------------
 -ssm  StructSimMode      (optional, default=BlockAveraged)
                          [BlockAveraged, RegularGaussianInt, RegularAveraged] 
 -ssb  StructSimBrdExt    (optional, applies to Regular mode only, default=None)
                          [None, Nearest, Reflect, Mirror, Zero] (see scipy.ndimage.generic_filter)
 -sss  StructSimStride    (optional, default=4)
 -ssw  StructSimWindow    (optional, applies to Block modes only, default=8, [8,16,32]) )PMBBRAWSTRING";
const std::string_view xAppQM::c_HelpStringValOp =
R"PMBBRAWSTRING(
usage::valiation ------------------------------------------------------------
 -ipa  InvalidPelActn     Select action taken if invalid pixel value is detected (optional, default=STOP)
                          [SKIP = no checking, WARN = print warning and ignore, STOP = stop execution,
                          CNCL = try to conceal by clipping to bit depth range]
 -nma  NameMismatchActn   Select action taken if parameters derived from filename are different
                          than provided as input parameters. Checks resolution, bit depth and chroma format.
                          (optional, default=WARN) 
                          [SKIP = no checking, WARN = print warning and ignore, STOP = stop execution]

usage::software_operation ---------------------------------------------------
 -nth  NumberOfThreads    Number of worker threads (optional, default=-2,
                          suggested ~8 for IVPSNR, all physical cores for SSIM)
                          [-1 = all available threads, -2 = reasonable auto]
 -v    VerboseLevel       Verbose level (optional, default=1)

 -c    "config.cfg"       External config file - in INI format (optional)

-----------------------------------------------------------------------------
VerboseLevel:
  0 = final (average) metric values only
  1 = 0 + configuration + detected frame numbers
  2 = 1 + argc/argv + frame level metric values
  3 = 2 + computing time (could slightly slow down computations)
  4 = 3 + IV specific debug data (GlobalColorShift, R2T+T2R, NumNonMasked)
  9 = stdout flood )PMBBRAWSTRING";


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Setup
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void xAppQM::registerCmdParamsDispatch()
{
  //dispatcher params to be ignored
  m_CfgParser.addCmdFakeParm("", "DispatchForce"  );
  m_CfgParser.addCmdFakeParm("", "DispatchVerbose");
}
void xAppQM::registerCmdParamsCmnIV()
{
  m_CfgParser.addCmdParm("sr" , "SearchRange"     , "", "SearchRange"     );
  m_CfgParser.addCmdParm("unc", "UnnoticeableCoef", "", "UnnoticeableCoef");
}
void xAppQM::registerCmdParamsSSIM()
{
  m_CfgParser.addCmdParm("ssm", "StructSimMode"    , "", "StructSimMode"  );
  m_CfgParser.addCmdParm("ssb", "StructSimBrdExt"  , "", "StructSimBrdExt");
  m_CfgParser.addCmdParm("sss", "StructSimStride"  , "", "StructSimStride");
  m_CfgParser.addCmdParm("ssw", "StructSimWindow"  , "", "StructSimWindow");
}
void xAppQM::registerCmdParamsValOp()
{
  //validation 
  m_CfgParser.addCmdParm("ipa", "InvalidPelActn"   , "", "InvalidPelActn"  );
  m_CfgParser.addCmdParm("nma", "NameMismatchActn" , "", "NameMismatchActn");
  //operation
  m_CfgParser.addCmdParm("nth", "NumberOfThreads"  , "", "NumberOfThreads");
  m_CfgParser.addCmdParm("v"  , "VerboseLevel"     , "", "VerboseLevel"   );  
}
bool xAppQM::loadConfiguration(int argc, const char* argv[])
{
  bool CommandlineResult = m_CfgParser.loadFromCmdln(argc, argv);
  if(!CommandlineResult) 
  { 
    m_ErrorLog += "! invalid commandline\n";
    m_ErrorLog += m_CfgParser.getParsingLog();
  }
  return CommandlineResult;
}
bool xAppQM::readConfigurationMetric()
{
  bool Correct = true;
  if(m_CfgParser.findParam("MetricList"))
  {
    xCfgINI::stringVx MetricListVS = m_CfgParser.getParamArgs("MetricList");

    for(const std::string& MetricS : MetricListVS)
    {
      if(xString::toUpper(MetricS) == "ALL") { std::fill(m_CalcMetric.begin(), m_CalcMetric.end(), true); break; }
      eMetric Metric = xStrToMetric(MetricS);
      if(Metric != eMetric::UNDEFINED) { m_CalcMetric[(int32)Metric] = true; }
      else { m_ErrorLog += fmt::format("!  MetricList contains not valid entry. Token \"{}\" in not a metric. \n", MetricS); Correct = false; }
    }
  }
  else
  {
    for(int32 m = 0; m < c_MetricsNum; m++) { if(xMetricInfo::Metrics[m].IsDefault) { m_CalcMetric[m] = true; } }
  }
  return Correct;
}
bool xAppQM::readConfigurationCmnIV()
{
  bool Correct = true;
  m_ShftCompPicRange = m_CfgParser.getParam1stArg("SearchRange", xIVPSNR::c_DefaultSearchRange);
  if(m_ShftCompPicRange < 1) { m_ErrorLog += "!  SearchRange value must non-zero\n"; Correct = false; }
  std::string UnnoticeableCoefS  = m_CfgParser.getParam1stArg("UnnoticeableCoef" , xFmtScn::formatFltWeights(xGlobClrDiffPrms      ::c_DefaultUnntcbCoef));
  m_UnnoticeableCoef  = xFmtScn::scanFltWeights(UnnoticeableCoefS );
  return Correct;
}
bool xAppQM::readConfigurationSSIM()
{
  bool Correct = true;

  m_StructSimMode = m_CfgParser.cvtParam1stArg("StructSimMode", xSSIM::c_DefaultStructSimMode, xSSIM::xStrToMode);
  if(m_StructSimMode == xSSIM::eMode::INVALID) { m_ErrorLog += "!  StructSimMode value is not valid\n"; Correct = false; }  
  m_StructSimBrdExt = m_CfgParser.cvtParam1stArg("StructSimBrdExt", eMrgExt::None, xStr2MrgExt);
  if(m_StructSimBrdExt == eMrgExt::INVALID) { m_ErrorLog += "!  StructSimBrdExt value is not valid\n"; Correct = false; }
  m_StructSimStride = m_CfgParser.getParam1stArg("StructSimStride" , xSSIM::c_DefaultStructSimStride);
  m_StructSimWindow = m_CfgParser.getParam1stArg("StructSimWindow", xSSIM::determineWindowSize(m_StructSimMode, xSSIM::c_DefaultStructSimWindow));
  if(m_StructSimStride < 1 || m_StructSimStride > m_StructSimWindow) { m_ErrorLog += "! StructSimStride must be in range 1-StructSimWindow\n"; Correct = false; }
  if(xSSIM::isRegularMode(m_StructSimMode) && m_StructSimWindow != 11) { m_ErrorLog += "! In regular struct sim mode only StructSimWindow==11 is allowed\n"; Correct = false; }

  return Correct;
}
bool xAppQM::readConfigurationValOp()
{
  bool Correct = true;
  //validation --------------------------------------------------------------------------------------------------------
  m_InvalidPelActn   = m_CfgParser.cvtParam1stArg("InvalidPelActn"  , eActn::STOP, xStr2Actn);
  m_NameMismatchActn = m_CfgParser.cvtParam1stArg("NameMismatchActn", eActn::WARN, xStr2Actn);
  //operation ---------------------------------------------------------------------------------------------------------
  m_NumberOfThreads = m_CfgParser.getParam1stArg("NumberOfThreads", -2  );
  m_VerboseLevel    = m_CfgParser.getParam1stArg("VerboseLevel"   , 1   );
  return Correct;
}
bool xAppQM::derriveConfigurationCmn()
{
  m_CalcPSNRs   = getCalcMetric(eMetric::PSNR) || getCalcMetric(eMetric::WSPSNR) || getCalcMetric(eMetric::IVPSNR);
  m_CalcSSIMs   = getCalcMetric(eMetric::SSIM) || getCalcMetric(eMetric::IVSSIM) || getCalcMetric(eMetric::MSSSIM) || getCalcMetric(eMetric::IVMSSSIM);
  m_CalcPVDs    = getCalcMetric(eMetric::PVD);
  m_CalcIVs     = getCalcMetric(eMetric::IVPSNR) || getCalcMetric(eMetric::IVSSIM) || getCalcMetric(eMetric::IVMSSSIM);
  m_CalcMSs     = getCalcMetric(eMetric::MSSSIM) || getCalcMetric(eMetric::IVMSSSIM);
  m_CalcSCP     = m_WriteSCP || m_CalcIVs;
#if X_PMBB_STALLED
  m_CalcSSIMs    = m_CalcSSIMs || getCalcMetric(eMetric::MSIVSSIM);
  m_CalcIVs      = m_CalcIVs   || getCalcMetric(eMetric::MSIVSSIM);
  m_CalcMSs      = m_CalcMSs   || getCalcMetric(eMetric::MSIVSSIM);
  m_CalcSCP      = m_CalcSCP   || getCalcMetric(eMetric::MSIVSSIM);
#endif //X_PMBB_STALLED
#if X_PMBB_EXPERIMENTAL
  m_CalcPVDs     = m_CalcPVDs || getCalcMetric(eMetric::IVPVD);
  m_CalcIVs      = m_CalcIVs  || getCalcMetric(eMetric::IVPVD);
  m_CalcSCP      = m_CalcSCP  || getCalcMetric(eMetric::IVPVD);
#endif //X_PMBB_EXPERIMENTAL
  m_CalcGCD      = m_CalcIVs || m_CalcSCP;

  int32 MarginIV = m_CalcIVs ? xRoundUpToNearestMultiple(m_ShftCompPicRange, 2) : 0;
  int32 MasginSS = m_CalcSSIMs && xSSIM::isRegularMode(m_StructSimMode) ? xRoundUpToNearestMultiple(m_StructSimWindow / 2, 2) : 0;
  m_PicMargin    = xMax(MarginIV, MasginSS);
  m_WindowSize   = 2 * m_ShftCompPicRange + 1;
  m_PrintFrame   = m_VerboseLevel >= 2;
  m_GatherTime   = m_VerboseLevel >= 3;
  m_PrintDebug   = m_VerboseLevel >= 4;

  return true;
}
bool xAppQM::validateConfigurationCmn()
{
  bool Correct = true;

  //SSIM modes validation
#if !X_PMBB_EXPERIMENTAL
  if(m_CalcSSIMs && m_StructSimMode == xSSIM::eMode::RegularGaussianFlt)
  {
    m_ErrorLog += "! The SSIM calculated using canonical mode (RegularGaussianFlt) is numericaly unstable. Please use RegularGaussianInt to use 11x11 Gaussian window. This mode is only available in EXPERIMENTAL builds. \n"; Correct = false;
  }
  if(m_CalcSSIMs && m_StructSimMode == xSSIM::eMode::BlockGaussianInt)
  {
    m_ErrorLog += "! The SSIM calculated using BlockGaussianInt mode is experimental. This mode is only available in EXPERIMENTAL builds.\n"; Correct = false;
  }
#endif
  if(m_UseMask && (m_StructSimMode == xSSIM::eMode::RegularGaussianFlt || m_StructSimMode == xSSIM::eMode::RegularGaussianInt || m_StructSimMode == xSSIM::eMode::BlockGaussianInt))
  {
    m_ErrorLog += "! The SSIM with Mask is only available in BlockAveraged and RegularAveraged modes.\n"; Correct = false;
  }
  if(m_UseMask && xSSIM::isRegularMode(m_StructSimMode) && m_StructSimStride != 1)
  {
    m_ErrorLog += "! SSIM with Mask calculated using regular modes may lead to udefined behaviour if StructSimStride>1 is used.\n"; Correct = false;
  }

  return Correct;
}
void xAppQM::formatConfigurationMetrics(std::string& ConfigInfo)
{
  ConfigInfo += "MetricList        = ";
  bool AnyMetric = false;
  for(int32 m = 0; m < c_MetricsNum; m++) { if(m_CalcMetric[m]) { ConfigInfo += xMetricToStr((eMetric)m) + ", "; AnyMetric = true; } }
  if(AnyMetric) { ConfigInfo.resize(ConfigInfo.size() - 2); } //cut trailing ", "
  else          { ConfigInfo += "(none)"; }
  ConfigInfo += "\n";
}
void xAppQM::formatConfigurationCmnIV(std::string& ConfigInfo)
{
  ConfigInfo += fmt::format("SearchRange       = {}{}\n", m_ShftCompPicRange, m_ShftCompPicRange == xIVPSNR::c_DefaultSearchRange ? "  (default)" : "  (custom)");
  ConfigInfo += fmt::format("UnnoticeableCoef  = {}{}\n", xFmtScn::formatFltWeights(m_UnnoticeableCoef ), m_UnnoticeableCoef  == xGlobClrDiffPrms      ::c_DefaultUnntcbCoef ? "  (default)" : "  (custom)");
}
void xAppQM::formatConfigurationSSIM(std::string& ConfigInfo)
{
  ConfigInfo += fmt::format("StructSimMode     = {}\n", xSSIM::xModeToStr(m_StructSimMode));
  ConfigInfo += fmt::format("StructSimBrdExt   = {}\n", xMrgExt2Str(m_StructSimBrdExt));
  ConfigInfo += fmt::format("StructSimStride   = {}\n", m_StructSimStride);
  ConfigInfo += fmt::format("StructSimWindow   = {}\n", m_StructSimWindow);
}
void xAppQM::formatConfigurationValOp(std::string& ConfigInfo)
{
  //validation 
  ConfigInfo += fmt::format("InvalidPelActn    = {}\n", xActn2Str(m_InvalidPelActn  ));
  ConfigInfo += fmt::format("NameMismatchActn  = {}\n", xActn2Str(m_NameMismatchActn));
  //operation
  ConfigInfo += fmt::format("NumberOfThreads   = {}{}\n", m_NumberOfThreads, m_NumberOfThreads == -1 ? "  (all)" : m_NumberOfThreads == -2 ? "  (auto)" : "");
  ConfigInfo += fmt::format("VerboseLevel      = {}\n"  , m_VerboseLevel  );
}
void xAppQM::formatMetricDescriptions(std::string& ConfigInfo)
{
  std::string MetricDesc;
  for(int32 m = 0; m < c_MetricsNum; m++) { if(m_CalcMetric[m]) { MetricDesc += fmt::format("{:<9} - {}\n", xMetricToStr((eMetric)m), xMetricInfo::Metrics[m].Description); } }
  if(!MetricDesc.empty()) {  ConfigInfo += "Selected metrics:\n" + MetricDesc + "\n"; }
}
std::string xAppQM::formatWarningsCmn()
{
  QMAU_TRACE(3, "");
  std::string Warnings = "";

  //check conformance
  if(m_ShftCompPicRange != xCorrespPixelShiftPrms::c_DefaultSearchRange)
  {
    Warnings += fmt::format("CONFORMANCE WARNING: Software was executed with SearchRange different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for Immersive Video. The default range is DefaultSearchRange={}.\n\n", xCorrespPixelShiftPrms::c_DefaultSearchRange);
  }
  if(m_UnnoticeableCoef != xGlobClrDiffPrms::c_DefaultUnntcbCoef)
  {
    Warnings += fmt::format("CONFORMANCE WARNING: Software was executed with UnnoticeableCoef different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for Immersive Video. The default coeffs are DefaultUnnoticeableCoef={}.\n\n", xFmtScn::formatFltWeights(xGlobClrDiffPrms::c_DefaultUnntcbCoef));
  }
  if(m_StructSimMode != xSSIM::c_DefaultStructSimMode)
  {
    Warnings += fmt::format("CONFORMANCE WARNING: Software was executed with StructSimMode different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for Immersive Video. The default setting is StructSimMode={}.\n\n", xSSIM::xModeToStr(xSSIM::c_DefaultStructSimMode));
  }
  if(m_StructSimStride != xSSIM::c_DefaultStructSimStride)
  {
    Warnings += fmt::format("CONFORMANCE WARNING: Software was executed with StructSimStride different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for Immersive Video. The default setting is StructSimStride={}.\n\n", xSSIM::c_DefaultStructSimStride);
  }
  if(m_StructSimWindow != xSSIM::c_DefaultStructSimWindow)
  {
    Warnings += fmt::format("CONFORMANCE WARNING: Software was executed with StructSimWindow different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for Immersive Video. The default setting is StructSimWindow={}.\n\n", xSSIM::c_DefaultStructSimWindow);
  }

  //SSIM notes
  if((m_StructSimMode != xSSIM::eMode::RegularGaussianFlt && m_StructSimMode != xSSIM::eMode::RegularGaussianInt) || m_StructSimStride != 1)
  {
    Warnings += fmt::format("SSIM ALGORITM NOTTICE: ");
    Warnings += fmt::format("The selected SSIM calculation mode (StructSimMode={}, StructSimStride={}) differs from proposed in original paper (Z. Wang, A.C. Bovik, H.R. Sheikh, E.P. Simoncelli, \"Image quality assessment : from error measurement to structural similarity\", IEEE Trans Image Process, 13 (Apr. 2004), pp. 600 - 613). ", xSSIM::xModeToStr(m_StructSimMode), m_StructSimStride);
    Warnings += fmt::format("By default, the software uses StructSimMode={}, StructSimStride={}, StructSimWindow={} (similar to approach used by FFMPEG). ", xSSIM::xModeToStr(xSSIM::c_DefaultStructSimMode), xSSIM::c_DefaultStructSimStride, xSSIM::c_DefaultStructSimWindow);
    Warnings += fmt::format("This change reduces computational complexity while increasing correlation with MOS. ");
    Warnings += fmt::format("If you want to use the same approach as in original paper, select StructSimMode={}, StructSimStride={}.\n\n\n", xSSIM::xModeToStr(xSSIM::eMode::RegularGaussianFlt), 1);
  }

  //check performance
  if(m_ShftCompPicRange > xCorrespPixelShiftPrms::c_DefaultSearchRange)
  {
    Warnings += fmt::format("PERFORMANCE WARNING: Software was executed with SearchRange wider than default one. This leads to higher computational complexity and longer calculation time. The default range is DefaultSearchRange=%d.\n\n", xCorrespPixelShiftPrms::c_DefaultSearchRange);
  }

  return Warnings;
}


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Multithreading
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void xAppQM::setupMultithreading()
{
  QMAU_TRACE(2, "");
  m_HardwareConcurency  = std::thread::hardware_concurrency();

  int32 PreferredNumberOfThreads = determinePreferredNumThreads();

  m_NumberOfThreadsUsed = 0;
  if(m_NumberOfThreads >=  1) { m_NumberOfThreadsUsed = xMin(m_NumberOfThreads, m_HardwareConcurency); }
  if(m_NumberOfThreads == -1) { m_NumberOfThreadsUsed = m_HardwareConcurency; }
  if(m_NumberOfThreads == -2) { m_NumberOfThreadsUsed = PreferredNumberOfThreads; }
  if(m_NumberOfThreadsUsed > 0)
  {
    const int32 MaxNumTasks = determineMaxNumTasks();
    m_ThreadPool = new xThreadPool;
    m_ThreadPool->create(m_NumberOfThreadsUsed, MaxNumTasks + 1);
    m_TPI.init(m_ThreadPool, MaxNumTasks, MaxNumTasks);
  }
}
void xAppQM::ceaseMultithreading()
{
  QMAU_TRACE(2, "");
  if(m_NumberOfThreadsUsed)
  {
    m_TPI.uninit();
    m_ThreadPool->destroy();
    m_ThreadPool = nullptr;
  }
}
std::string xAppQM::formatMultithreading()
{
  QMAU_TRACE(2, "");
  std::string Info = "";
  Info += fmt::format("Multithreading:\n");
  Info += fmt::format("HardwareConcurency  = {}\n", m_HardwareConcurency );
  Info += fmt::format("NumberOfThreadsUsed = {}\n", m_NumberOfThreadsUsed);
  return Info;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Metric processors
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xAppQM::createMetricProcessors()
{  
  QMAU_TRACE(2, "");
  const int32 PictureWidth  = m_PictureSize.getX();
  const int32 PictureHeight = m_PictureSize.getY();

  if(m_CalcGCD)
  {
    QMAU_TRACE(3, "ProcGCD");
    m_ProcGCD.setUnntcbCoef(m_UnnoticeableCoef);
    m_ProcGCD.bindThrdPoolIntf(&m_TPI);
  }

  if(m_CalcSCP)
  {
    QMAU_TRACE(3, "ProcSCP");
    m_ProcSCP.setSearchRange      (m_ShftCompPicRange );
    m_ProcSCP.setCmpWeightsSearch (m_CmpWeightsSearch );
    m_ProcSCP.setCmpWeightsAverage(m_CmpWeightsAverage);
    m_ProcSCP.bindThrdPoolIntf    (&m_TPI             );
  }

  if(m_CalcPSNRs)
  {
    QMAU_TRACE(3, "ProcPSNR");
    m_ProcPSNR.setSearchRange      (m_ShftCompPicRange );
    m_ProcPSNR.setCmpWeightsSearch (m_CmpWeightsSearch );
    m_ProcPSNR.setCmpWeightsAverage(m_CmpWeightsAverage);
    m_ProcPSNR.setUnntcbCoef       (m_UnnoticeableCoef );
    m_ProcPSNR.bindThrdPoolIntf    (&m_TPI             );
    m_ProcPSNR.initRowBuffers(PictureHeight);
    if(m_IsEquirectangular) { m_ProcPSNR.initWS(true, PictureWidth, PictureHeight, m_BitDepth, m_LonRangeDeg, m_LatRangeDeg); }
  }

  if(m_CalcSSIMs)
  {
    QMAU_TRACE(3, "ProcSSIM");
    m_ProcSSIM.create              (m_PictureSize, m_BitDepth, m_PicMargin, m_CalcMSs, m_UseMask);
    m_ProcSSIM.setSearchRange      (m_ShftCompPicRange );
    m_ProcSSIM.setCmpWeightsSearch (m_CmpWeightsSearch );
    m_ProcSSIM.setCmpWeightsAverage(m_CmpWeightsAverage);
    m_ProcSSIM.setUnntcbCoef       (m_UnnoticeableCoef );
    m_ProcSSIM.setStructSimParams  (m_StructSimMode, m_StructSimBrdExt, m_StructSimWindow, m_StructSimStride);
    m_ProcSSIM.bindThrdPoolIntf    (&m_TPI             );
    m_ProcSSIM.initRowBuffers(PictureHeight);
    if(m_IsEquirectangular) { m_ProcSSIM.initWS(true, PictureWidth, PictureHeight, m_BitDepth, m_LonRangeDeg, m_LatRangeDeg); }
  }

  if(m_CalcPVDs)
  {
    QMAU_TRACE(3, "ProcPVD");
    m_ProcPVD.bindThrdPoolIntf(&m_TPI);
  }

  if(m_PrintDebug)
  {
    if(m_CalcPSNRs) { m_ProcPSNR.setDebugCallbackQAP([this](flt64 R2T, flt64 T2R) { m_LastR2T = R2T; m_LastT2R = T2R; }); }
    if(m_CalcSSIMs) { m_ProcSSIM.setDebugCallbackQAP([this](flt64 R2T, flt64 T2R) { m_LastR2T = R2T; m_LastT2R = T2R; }); }
  }
}
void xAppQM::destroyMetricProcessors()
{
  QMAU_TRACE(2, "");
  if(m_CalcSSIMs)
  {
    m_ProcSSIM.destroy();
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Preproc & metrics
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xAppQM::calcMetricsCmn(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpcMtr)
{
  if(m_CalcGCD) { calcFrameGCD(FrameIdx, ViewIdx); }
  if(m_CalcSCP) { calcFrameSCP(FrameIdx, ViewIdx); }
  if(getCalcMetric(eMetric::MSE     )) { calcFrame_____MSE(FrameIdx, ViewIdx, ClrSpcMtr); }
  if(getCalcMetric(eMetric::PSNR    )) { calcFrame____PSNR(FrameIdx, ViewIdx, ClrSpcMtr); }
  if(getCalcMetric(eMetric::WSPSNR  )) { calcFrame__WSPSNR(FrameIdx, ViewIdx, ClrSpcMtr); }
  if(getCalcMetric(eMetric::IVPSNR  )) { calcFrame__IVPSNR(FrameIdx, ViewIdx, ClrSpcMtr); }
  if(m_StructSimBrdExt != eMrgExt::None) { addStructSimMargs(FrameIdx); }
  if(getCalcMetric(eMetric::SSIM    )) { calcFrame____SSIM(FrameIdx, ViewIdx, ClrSpcMtr); }
  if(getCalcMetric(eMetric::MSSSIM  )) { calcFrame__MSSSIM(FrameIdx, ViewIdx, ClrSpcMtr); }
  if(getCalcMetric(eMetric::IVSSIM  )) { calcFrame__IVSSIM(FrameIdx, ViewIdx, ClrSpcMtr); }
  if(getCalcMetric(eMetric::IVMSSSIM)) { calcFrameIVMSSSIM(FrameIdx, ViewIdx, ClrSpcMtr); }
#if X_PMBB_STALLED
  if(getCalcMetric(eMetric::MSIVSSIM)) { calcFrameMSIVSSIM(FrameIdx, ViewIdx, ClrSpcMtr); }
#endif //X_PMBB_STALLED
  if(getCalcMetric(eMetric::PVD     )) { calcFrame_____PVD(FrameIdx, ViewIdx, ClrSpcMtr); }
#if X_PMBB_EXPERIMENTAL
  if(getCalcMetric(eMetric::IVPVD   )) { calcFrame___IVPVD(FrameIdx, ViewIdx, ClrSpcMtr); }
#endif //X_PMBB_EXPERIMENTAL
}
void xAppQM::preprocessMaskCmn(int32 /*FrameIdx*/)
{
  switch(m_MaskTransMode)
  {
  case eMskTrMd::None:
    if(m_BitDepthM != 8) //make sure that mask is always in range 0-255
    {
      xPixelOps::ShiftRound(m_PicMskInP.getAddr(eCmp::LM), m_PicMskInP.getAddr(eCmp::LM), m_PicMskInP.getStride(), m_PicMskInP.getStride(), m_PicMskInP.getWidth(), m_PicMskInP.getHeight(), 8, m_BitDepthM);
    }
    break;
  case eMskTrMd::Threshold:
  {
    const uint16 Threshold = (uint16)xClipU<int32>(m_MaskTransParam, m_PicMskInP.getMaxPelValue());
    const uint16 MaxValue  = (uint16)xBitDepth2MaxValue(8);
    xPixelOps::Threshold(m_PicMskInP.getAddr(eCmp::LM), m_PicMskInP.getAddr(eCmp::LM), m_PicMskInP.getStride(), m_PicMskInP.getStride(), m_PicMskInP.getWidth(), m_PicMskInP.getHeight(), Threshold, MaxValue);
    break;
  }
  default: break;
  }

  m_NumNonMasked = xPixelOps::CountNonZero(m_PicMskInP.getAddr(eCmp::LM), m_PicMskInP.getStride(), m_PicMskInP.getWidth(), m_PicMskInP.getHeight());
  if(m_PrintDebug) { fmt::print("NNM {}    ", m_NumNonMasked); }
  
  m_PicMskInP.extend();
}
void xAppQM::addStructSimMargs(int32 /*FrameIdx*/)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  for(int32 i = 0; i < c_NumPics; i++) { m_PicInP[i].extend(m_StructSimBrdExt); }
  if(m_CalcSCP)
  {
    for(int32 i = 0; i < c_NumPics; i++) { m_PicSCP[i].extend(m_StructSimBrdExt); }
  }
  if(m_GatherTime) { m_Ticks__Margin += (xTSC() - T); }
}
void xAppQM::calcFrameGCD(int32 FrameIdx, int32 ViewIdx)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  if(m_UseMask) { m_GCD_R2T = m_ProcGCD.CalcGlobalColorDiffM(&m_PicInP[0], &m_PicInP[1], &m_PicMskInP, m_NumNonMasked); }
  else          { m_GCD_R2T = m_ProcGCD.CalcGlobalColorDiff (&m_PicInP[0], &m_PicInP[1]                              ); }
  if(m_PrintDebug) { fmt::print("{} GCD-R2T {} {} {} {}\n", xFormatIdx(FrameIdx, ViewIdx), m_GCD_R2T[0], m_GCD_R2T[1], m_GCD_R2T[2], m_GCD_R2T[3]); }
  if(m_GatherTime) { m_Ticks_____GCD += (xTSC() - T); }
}
void xAppQM::calcFrameSCP(int32 /*FrameIdx*/, int32 /*ViewIdx*/)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  if(m_UseMask) { m_ProcSCP.GenShftCompPicsM(&m_PicSCP[1], &m_PicSCP[0], &m_PicInP[1], &m_PicInP[0], &m_PicMskInP, m_GCD_R2T); }
  else          { m_ProcSCP.GenShftCompPics (&m_PicSCP[1], &m_PicSCP[0], &m_PicInP[1], &m_PicInP[0],               m_GCD_R2T); }
  if(m_GatherTime) { m_Ticks_____SCP += (xTSC() - T); }
}
void xAppQM::calcFrame_____MSE(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  flt64V4 MSE  = xMakeVec4(0.0);  

  if(m_UseMask) { MSE = m_ProcPSNR.calcPicMSEM(&m_PicInP[0], &m_PicInP[1], &m_PicMskInP, m_NumNonMasked); }
  else          { MSE = m_ProcPSNR.calcPicMSE (&m_PicInP[0], &m_PicInP[1]                              ); }
  if(m_GatherTime) { m_MetricData[(int32)eMetric::MSE].addTicks(xTSC() - T); }
  m_MetricData[(int32)eMetric::MSE].setPerCmpMeric(MSE, FrameIdx, ViewIdx, ClrSpc);

  if(m_PrintFrame)
  {
    std::string Log = xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::MSE].formatPerCmpMetric(FrameIdx, ViewIdx, ClrSpc);
    if(m_ExactCmps[0]) { Log += " ExactY"; } if(m_ExactCmps[1]) { Log += " ExactU"; } if(m_ExactCmps[2]) { Log += " ExactV"; }
    Log += "\n";
    Log += xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::MSE].formatPerPicMetric(FrameIdx, ViewIdx, ClrSpc);
    Log += "\n";
    fmt::print("{}", Log);
  }
}
void xAppQM::calcFrame____PSNR(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  flt64V4 PSNR  = xMakeVec4(0.0);

  if(m_UseMask) { PSNR = m_ProcPSNR.calcPicPSNRM(&m_PicInP[0], &m_PicInP[1], &m_PicMskInP, m_NumNonMasked); }
  else          { PSNR = m_ProcPSNR.calcPicPSNR (&m_PicInP[0], &m_PicInP[1]                              ); }
  if(m_GatherTime) { m_MetricData[(int32)eMetric::PSNR].addTicks(xTSC() - T); }

  for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
  { 
    if(m_ExactCmps[CmpIdx])
    {
      PSNR[CmpIdx] = m_ProcPSNR.getFakePSNR(m_PicInP[0].getArea(), m_PicInP[0].getBitDepth());
      m_MetricData[(int32)eMetric::PSNR].setAnyFake(true);
    }
  }
  m_MetricData[(int32)eMetric::PSNR].setPerCmpMeric(PSNR, FrameIdx, ViewIdx, ClrSpc);

  if(m_PrintFrame)
  {
    std::string Log = xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::PSNR].formatPerCmpMetric(FrameIdx, ViewIdx, ClrSpc);
    if(m_ExactCmps[0]) { Log += " ExactY"; } if(m_ExactCmps[1]) { Log += " ExactU"; } if(m_ExactCmps[2]) { Log += " ExactV"; }
    Log += "\n";
    Log += xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::PSNR].formatPerPicMetric(FrameIdx, ViewIdx, ClrSpc);
    Log += "\n";
    Log += xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::PSNR].formatPerPicMSEMetric(FrameIdx, ViewIdx, ClrSpc);
    Log += "\n";
    fmt::print("{}", Log);
  }
}
void xAppQM::calcFrame__WSPSNR(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  flt64V4 WSPSNR = xMakeVec4(0.0);
  if(m_UseMask) { WSPSNR = m_ProcPSNR.calcPicWSPSNRM(&m_PicInP[0], &m_PicInP[1], &m_PicMskInP, m_NumNonMasked); }
  else          { WSPSNR = m_ProcPSNR.calcPicWSPSNR (&m_PicInP[0], &m_PicInP[1]                              ); }
  if(m_GatherTime) { m_MetricData[(int32)eMetric::WSPSNR].addTicks(xTSC() - T); }

  for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
  {
    if(m_ExactCmps[CmpIdx])
    {
      WSPSNR[CmpIdx] = m_ProcPSNR.getFakePSNR(m_PicInP[0].getArea(), m_PicInP[0].getBitDepth());
      m_MetricData[(int32)eMetric::WSPSNR].setAnyFake(true);
    }
  }
  m_MetricData[(int32)eMetric::WSPSNR].setPerCmpMeric(WSPSNR, FrameIdx, ViewIdx, ClrSpc);

  if(m_PrintFrame)
  {
    std::string Log = xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::WSPSNR].formatPerCmpMetric(FrameIdx, ViewIdx, ClrSpc);
    if(m_ExactCmps[0]) { Log += " ExactY"; } if(m_ExactCmps[1]) { Log += " ExactU"; } if(m_ExactCmps[2]) { Log += " ExactV"; }
    Log += "\n";
    Log += xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::WSPSNR].formatPerPicMetric(FrameIdx, ViewIdx, ClrSpc);
    Log += "\n";
    fmt::print("{}", Log);
  }
}
void xAppQM::calcFrame__IVPSNR(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  flt64V2 IVPSNR = { 0.0, 0.0 };
  if(m_UseMask) { IVPSNR = m_ProcPSNR.calcPicIVPSNRM(&m_PicInP[0], &m_PicInP[1], &m_PicSCP[0], &m_PicSCP[1], &m_PicMskInP, m_NumNonMasked); }
  //if(m_UseMask) { IVPSNR = m_ProcPSNR.calcPicIVPSNRM(&m_PicInP[0], &m_PicInP[1], &m_PicMskInP, m_NumNonMasked); }
  else          { IVPSNR = m_ProcPSNR.calcPicIVPSNR (&m_PicInP[0], &m_PicInP[1], &m_PicSCP[0], &m_PicSCP[1])                              ; }
  if(m_GatherTime) { m_MetricData[(int32)eMetric::IVPSNR].addTicks(xTSC() - T); }
  m_MetricData[(int32)eMetric::IVPSNR].setPerPicMeric(IVPSNR[0], FrameIdx, ViewIdx, ClrSpc, IVPSNR[1]);

  if(m_PrintFrame)
  {
    std::string Log = xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::IVPSNR].formatPerPicMetric(FrameIdx, ViewIdx, ClrSpc);
    if(m_PrintDebug) { Log += fmt::format("    R2T {:7.4f}  T2R {:7.4f}", m_LastR2T, m_LastT2R); }
    Log += "\n";
    Log += xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::IVPSNR].formatPerPicMSEMetric(FrameIdx, ViewIdx, ClrSpc);
    fmt::print("{}\n", Log);
  }
}
void xAppQM::calcFrame____SSIM(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  flt64V4 SSIM = xMakeVec4(0.0);

  if(m_UseMask){ SSIM = m_ProcSSIM.calcPicSSIMM(&m_PicInP[0], &m_PicInP[1], &m_PicMskInP); }
  else         { SSIM = m_ProcSSIM.calcPicSSIM (&m_PicInP[0], &m_PicInP[1]              ); }
  if(m_GatherTime) { m_MetricData[(int32)eMetric::SSIM].addTicks(xTSC() - T); }
  m_MetricData[(int32)eMetric::SSIM].setPerCmpMeric(SSIM, FrameIdx, ViewIdx, ClrSpc);

  if(m_PrintFrame)
  { 
    std::string Log = xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::SSIM].formatPerCmpMetric(FrameIdx, ViewIdx, ClrSpc) + "\n";
    Log            += xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::SSIM].formatPerPicMetric(FrameIdx, ViewIdx, ClrSpc) + "\n";
    fmt::print("{}", Log);
  }

  if(m_DebugDump)
  {
    xPlane<uint16> Vis(m_PicInP[0].getSize(), 8, 0);
    m_ProcSSIM.visualizeSSIM(&Vis, &m_PicInP[0], &m_PicInP[1], eCmp::C0);
    xSeq::dumpFrame(&Vis, fmt::format("DUMP_SSIM_{}x{}_8bps.yuv", Vis.getWidth(), Vis.getHeight()), eCrF::CF420, FrameIdx == 0);
  }
}
void xAppQM::calcFrame__MSSSIM(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  flt64V4 MSSSIM = xMakeVec4(0.0);

  if(m_UseMask) { MSSSIM = m_ProcSSIM.calcPicMSSSIMM(&m_PicInP[0], &m_PicInP[1], &m_PicMskInP); }
  else          { MSSSIM = m_ProcSSIM.calcPicMSSSIM (&m_PicInP[0], &m_PicInP[1]              ); }
  if(m_GatherTime) { m_MetricData[(int32)eMetric::MSSSIM].addTicks(xTSC() - T); }
  m_MetricData[(int32)eMetric::MSSSIM].setPerCmpMeric(MSSSIM, FrameIdx, ViewIdx, ClrSpc);

  if(m_PrintFrame)
  {
    std::string Log = xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::MSSSIM].formatPerCmpMetric(FrameIdx, ViewIdx, ClrSpc) + "\n";
    Log            += xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::MSSSIM].formatPerPicMetric(FrameIdx, ViewIdx, ClrSpc) + "\n";
    fmt::print("{}", Log);
  }

}
void xAppQM::calcFrame__IVSSIM(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  flt64 IVSSIM = 0.0;
  if(m_UseMask) { IVSSIM = m_ProcSSIM.calcPicIVSSIMM(&m_PicInP[0], &m_PicInP[1], &m_PicSCP[0], &m_PicSCP[1], &m_PicMskInP); }
  else          { IVSSIM = m_ProcSSIM.calcPicIVSSIM (&m_PicInP[0], &m_PicInP[1], &m_PicSCP[0], &m_PicSCP[1]              ); }
  if(m_GatherTime) { m_MetricData[(int32)eMetric::IVSSIM].addTicks(xTSC() - T); }
  m_MetricData[(int32)eMetric::IVSSIM].setPerPicMeric(IVSSIM, FrameIdx, ViewIdx, ClrSpc);

  if(m_PrintFrame)
  {
    std::string Log = xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::IVSSIM].formatPerPicMetric(FrameIdx, ViewIdx, ClrSpc);
    if(m_PrintDebug) { Log += fmt::format("    R2T {:7.4f}  T2R {:7.4f}", m_LastR2T, m_LastT2R); }
    fmt::print("{}\n", Log);
  }

  if(m_DebugDump)
  {
    xPlane<uint16> Vis(m_PicInP[0].getSize(), 8, 0);
    m_ProcSSIM.visualizeIVSSIM(&Vis, &m_PicInP[0], &m_PicInP[1], &m_PicSCP[0], &m_PicSCP[1], eCmp::C0);
    xSeq::dumpFrame(&Vis, fmt::format("DUMP_IVSSIM_{}x{}_8bps.yuv", Vis.getWidth(), Vis.getHeight()), eCrF::CF420, FrameIdx == 0);
  }
}
void xAppQM::calcFrameIVMSSSIM(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  flt64 IVMSSSIM = 0.0;
  if(m_UseMask) { IVMSSSIM = m_ProcSSIM.calcPicIVMSSSIMM(&m_PicInP[0], &m_PicInP[1], &m_PicSCP[0], &m_PicSCP[1], &m_PicMskInP); }
  else          { IVMSSSIM = m_ProcSSIM.calcPicIVMSSSIM (&m_PicInP[0], &m_PicInP[1], &m_PicSCP[0], &m_PicSCP[1]              ); }
  if(m_GatherTime) { m_MetricData[(int32)eMetric::IVMSSSIM].addTicks(xTSC() - T); }
  m_MetricData[(int32)eMetric::IVMSSSIM].setPerPicMeric(IVMSSSIM, FrameIdx, ViewIdx, ClrSpc);

  if(m_PrintFrame)
  {
    std::string Log = xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::IVMSSSIM].formatPerPicMetric(FrameIdx, ViewIdx, ClrSpc);
    if(m_PrintDebug) { Log += fmt::format("    R2T {:7.4f}  T2R {:7.4f}", m_LastR2T, m_LastT2R); }
    fmt::print("{}\n", Log);
  }
}
#if X_PMBB_STALLED
void xAppQM::calcFrameMSIVSSIM(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  flt64 MSIVSSIM = m_ProcSSIM.calcPicMSIVSSIM(&m_PicInP[0], &m_PicInP[1], &m_PicSCP[0], &m_PicSCP[1]);
  if(m_GatherTime) { m_MetricData[(int32)eMetric::MSIVSSIM].addTicks(xTSC() - T); }
  m_MetricData[(int32)eMetric::MSIVSSIM].setPerPicMeric(MSIVSSIM, FrameIdx, ViewIdx, ClrSpc);
  
  if (m_PrintFrame)
  {
    std::string Log = xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::MSIVSSIM].formatPerPicMetric(FrameIdx, ViewIdx, ClrSpc);
    if (m_PrintDebug) { Log += fmt::format("    R2T {:7.4f}  T2R {:7.4f}", m_LastR2T, m_LastT2R); }
    fmt::print("{}\n", Log);
  }
}
#endif //X_PMBB_STALLED
void xAppQM::calcFrame_____PVD(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  flt64 PVD = 0.0;
  if(m_UseMask) { PVD = m_ProcPVD.calcPicPVDM(&m_PicInP[0], &m_PicInP[1], &m_PicMskInP); }
  else          { PVD = m_ProcPVD.calcPicPVD (&m_PicInP[0], &m_PicInP[1]              ); }
  if(m_GatherTime) { m_MetricData[(int32)eMetric::PVD].addTicks(xTSC() - T); }
  m_MetricData[(int32)eMetric::PVD].setPerPicMeric(PVD, FrameIdx, ViewIdx, ClrSpc);

  if(m_PrintFrame)
  {
    std::string Log = xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::PVD].formatPerPicMetric(FrameIdx, ViewIdx, ClrSpc);
    fmt::print("{}\n", Log);
  }
}
#if X_PMBB_EXPERIMENTAL
void xAppQM::calcFrame___IVPVD(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  flt64 IVPVD = 0.0;
  if(m_UseMask) { IVPVD = m_ProcPVD.calcPicIVPVDM(&m_PicInP[0], &m_PicInP[1], &m_PicSCP[0], &m_PicSCP[1], &m_PicMskInP); }
  else          { IVPVD = m_ProcPVD.calcPicIVPVD (&m_PicInP[0], &m_PicInP[1], &m_PicSCP[0], &m_PicSCP[1]              ); }
  if(m_GatherTime) { m_MetricData[(int32)eMetric::IVPVD].addTicks(xTSC() - T); }
  m_MetricData[(int32)eMetric::IVPVD].setPerPicMeric(IVPVD, FrameIdx, ViewIdx, ClrSpc);

  if(m_PrintFrame)
  {
    std::string Log = xFormatIdx(FrameIdx, ViewIdx) + m_MetricData[(int32)eMetric::IVPVD].formatPerPicMetric(FrameIdx, ViewIdx, ClrSpc);
    fmt::print("{}\n", Log);
  }
}
#endif //X_PMBB_EXPERIMENTAL


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Stats
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
std::string xAppQM::calibrateTimeStamp()
{
  QMAU_TRACE(2, "");
  m_TimeStamp.calibrateTimeStamp();
  return m_TimeStamp.formatCalibration();
}
std::string xAppQM::formatMetricTimeStats(flt64 InvDurationDenominator)
{
  QMAU_TRACE(3, "");
  std::string Result; Result.reserve(xMemory::getBestEffortSizePageBase());

  if(!m_GatherTime) { return Result; }

  tDurationMS AvgDurationPreprocF = tDurationMS((flt64)m_TicksPreprocF * InvDurationDenominator);
  tDurationMS AvgDurationPreprocM = tDurationMS((flt64)m_TicksPreprocM * InvDurationDenominator);
  tDurationMS AvgDuration__Margin = tDurationMS((flt64)m_Ticks__Margin * InvDurationDenominator);
  tDurationMS AvgDuration_____GCD = tDurationMS((flt64)m_Ticks_____GCD * InvDurationDenominator);
  tDurationMS AvgDuration_____SCP = tDurationMS((flt64)m_Ticks_____SCP * InvDurationDenominator);    

  Result += fmt::format("AvgTime     PREPROC-F {:9.2f} ms\n", AvgDurationPreprocF.count());
  Result += fmt::format("AvgTime     PREPROC-M {:9.2f} ms\n", AvgDurationPreprocM.count());
  if(m_CalcGCD) { Result += fmt::format("AvgTime           GCD {:9.2f} ms\n", AvgDuration_____GCD.count()); }
  if(m_CalcSCP) { Result += fmt::format("AvgTime           SCP {:9.2f} ms\n", AvgDuration_____SCP.count()); }
  if(m_StructSimBrdExt != eMrgExt::None) { Result += fmt::format("AvgTime        Margin {:9.2f} ms\n", AvgDuration__Margin.count()); }
  
  for(int32 m = 0; m < c_MetricsNum; m++)
  {
    tDurationMS PreMetricOps = AvgDurationPreprocM + AvgDurationPreprocF;

    xMetricStat& MD = m_MetricData[m];
    if(MD.getEnabled())
    { 
      switch(MD.getMetric())
      {
        case eMetric::     MSE: break;
        case eMetric::    PSNR: break;
        case eMetric::  WSPSNR: break;
        case eMetric::  IVPSNR: PreMetricOps += AvgDuration_____GCD + AvgDuration_____SCP; break;
        case eMetric::    SSIM: PreMetricOps += AvgDuration__Margin; break;
        case eMetric::  MSSSIM: PreMetricOps += AvgDuration__Margin; break;
        case eMetric::  IVSSIM: PreMetricOps += AvgDuration_____GCD + AvgDuration_____SCP + AvgDuration__Margin; break;
        case eMetric::IVMSSSIM: PreMetricOps += AvgDuration_____GCD + AvgDuration_____SCP + AvgDuration__Margin; break;
#if X_PMBB_STALLED
        case eMetric::MSIVSSIM: PreMetricOps += AvgDuration_____GCD + AvgDuration_____SCP; break;
#endif //X_PMBB_STALLED
        case eMetric::  PVD   : break;
#if X_PMBB_EXPERIMENTAL
        case eMetric::IVPVD   : PreMetricOps += AvgDuration_____GCD + AvgDuration_____SCP; break;
#endif //X_PMBB_EXPERIMENTAL
        default: break;
      }

      std::string PreMetricStr = "PREPROC";
      switch(MD.getMetric())
      {
        case eMetric::     MSE: break;
        case eMetric::    PSNR: break;
        case eMetric::  WSPSNR: break;
        case eMetric::  IVPSNR: PreMetricStr += " GCD SCP"       ; break;
        case eMetric::    SSIM: PreMetricStr += " Margin"        ; break;
        case eMetric::  MSSSIM: PreMetricStr += " Margin"        ; break;
        case eMetric::  IVSSIM: PreMetricStr += " GCD SCP Margin"; break;
        case eMetric::IVMSSSIM: PreMetricStr += " GCD SCP Margin"; break;
#if X_PMBB_STALLED
        case eMetric::MSIVSSIM: PreMetricStr += " GCD SCP"       ; break;
#endif //X_PMBB_STALLED
        case eMetric::  PVD   : break;
#if X_PMBB_EXPERIMENTAL
        case eMetric::IVPVD   : PreMetricStr += " GCD SCP"       ; break;
#endif //X_PMBB_EXPERIMENTAL
        default: break;
      }

      Result += MD.formatAvgTime("AvgTime      ", PreMetricOps) + "   (Total includes = [" + PreMetricStr + "]\n";
    }
  }
  return Result;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

std::string xAppQM::xFormatIdx(int32 FrameIdx, int32 ViewIdx) 
{
  std::string Result = fmt::format("Frame {:08d} ", FrameIdx);
  if(m_NumViews > 1)
  {
    if(ViewIdx != NOT_VALID) { Result += fmt::format("View {:03d} ", ViewIdx); }
    else                     { Result +=             "View NA  "             ; }
  }
  return Result;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB