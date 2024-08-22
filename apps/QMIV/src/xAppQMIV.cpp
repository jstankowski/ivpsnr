/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xAppQMIV.h"
#include "xProcInfo.h"
#include "xFmtScn.h"
#include "xPixelOps.h"
#include "xColorSpace.h"
#include "xSeqPNG.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

const std::string_view xAppQMIV::c_BannerString =
R"PMBBRAWSTRING(
=============================================================================
IVPSNR software v7.0

Copyright (c) 2020-2024, Jakub Stankowski & Adrian Dziembowski, All rights reserved.

Developed at Poznan University of Technology, Poznan, Poland
Authors: Jakub Stankowski, Adrian Dziembowski

The IV-PSNR metric is described in following paper:
A. Dziembowski, D. Mieloch, J. Stankowski and A. Grzelka, "IV-PSNR - The Objective Quality Metric for Immersive Video Applications," in IEEE Transactions on Circuits and Systems for Video Technology, vol. 32, no. 11, pp. 7575-7591, Nov. 2022, doi: 10.1109/TCSVT.2022.3179575.
https://doi.org/10.1109/TCSVT.2022.3179575

=============================================================================

)PMBBRAWSTRING";

const std::string_view xAppQMIV::c_HelpString =
R"PMBBRAWSTRING(
=============================================================================
IVPSNR software v7.0

 Cmd | ParamName        | Description

usage::general --------------------------------------------------------------
 -i0   InputFile0         File path - input sequence 0
 -i1   InputFile1         File path - input sequence 1
 -ff   FileFormat         Format of input sequence (optional, default=RAW) [RAW, PNG]
 -ps   PictureSize        Size of input sequences (WxH)
 -pw   PictureWidth       Width of input sequences 
 -ph   PictureHeight      Height of input sequences
 -pf   PictureFormat      Picture format as defined by FFMPEG pix_fmt i.e. yuv420p10le
 -bd   BitDepth           Bit depth     (optional, default=8, up to 14) 
 -cf   ChromaFormat       Chroma format (optional, default=420) [420, 422, 444]
 -s0   StartFrame0        Start frame 0  (optional, default=0) 
 -s1   StartFrame1        Start frame 1  (optional, default=0) 
 -nf   NumberOfFrames     Number of frames to be processed (optional, all=-1, default=-1)
 -r    ResultFile         Output file path for printing result(s) (optional)
 -ml   MetricList         List of quality metrics to be calculated, must be coma separated,
                          quotes are required. "All" enables all available metrics.
                          [PSNR, WSPSNR, IVPSNR, SSIM, IVSSIM]
                          (optional, default="PSNR, WSPSNR, IVPSNR, IVSSIM")       

PictureSize parameter can be used interchangeably with PictureWidth, PictureHeight pair. If PictureSize parameter is present the PictureWidth and PictureHeight arguments are ignored.
PictureFormat parameter can be used interchangeably with BitDepth, ChromaFormat pair. If PictureFormat parameter is present the BitDepth and, ChromaFormat arguments are ignored.

usage::mask_mode ------------------------------------------------------------
 -im   InputFileM         File path - mask       (optional, same resolution as InputFile0 and InputFile1)
 -bdm  BitDepthM          Bit depth for mask     (optional, default=BitDepth, up to 16)
 -cfm  ChromaFormatM      Chroma format for mask (optional, default=ChromaFormat) [400, 420, 422, 444]

usage::equirectangular ------------------------------------------------------
 -erp  Equirectangular    Equirectangular input sequence (flag, default disabled)
 -lor  LonRangeDeg        Range for ERP in degrees - Longitudinal (optional, default=360)
 -lar  LatRangeDeg        Range for ERP in degrees - Lateral      (optional, default=180)

usage::colorspace_parameters ------------------------------------------------
 -csi  ColorSpaceInput    Color space of input file             (optional, default=YCbCr)
 -csm  ColorSpaceMetric   Color space used to calculate metrics (optional, default=ColorSpaceInput)
                          If ColorSpaceInput!=ColorSpaceMetric the software performs on-demand conversion
                          (RGB-->YCbCr or YCbCr-->RGB). Conversion requires specific YCbCr color space parameters.
                          [RGB, BGR, GBR, YCbCr, YCbCr_BT601, YCbCr_SMPTE170M, YCbCr_BT709, YCbCr_SMPTE240M, YCbCr_BT2020]

usage::IV_specific ----------------------------------------------------------
 -sr   SearchRange        IV-metric search range around center point
                          (optional, default=2 --> 5x5)
 -cws  CmpWeightsSearch   IV-metric component weights used during search
                          ("Lm:Cb:Cr:0" or "R:G:B:0" - per component integer weights,
                          default="4:1:1:0", quotes are mandatory, requires USE_RUNTIME_CMPWEIGHTS=1)
 -cwa  CmpWeightsAverage  IV-metric component weights used during averaging
                          ("Lm:Cb:Cr:0" or "R:G:B:0" - per component integer weights,
                          default="4:1:1:0", quotes are mandatory)
 -unc  UnnoticeableCoef   IV-metric unnoticeable color difference threshold coeff
                          ("Lm:Cb:Cr:0" or "R:G:B:0" - per component coeff,
                          default="0.01:0.01:0.01:0", quotes are mandatory)

usage::valiation ------------------------------------------------------------
 -ipa  InvalidPelActn     Select action taken if invalid pixel value is detected 
                          (optional, default=STOP) [SKIP = disable pixel value checking,
                          WARN = print warning and ignore, STOP = stop execution,
                          CNCL = try to conceal by clipping to bit depth range]
 -nma  NameMismatchActn   Select action taken if parameters derived from filename are different
                          than provided as input parameters. Checks resolution, bit depth
                          and chroma format. (optional, default=WARN) [SKIP = disable checking,
                          WARN = print warning and ignore, STOP = stop execution]

usage::software_operation ---------------------------------------------------
 -nth  NumberOfThreads    Number of worker threads (optional, default=-2,
                          suggested ~8 for IVPSNR, all physical cores for SSIM)
                          [-1 = all available threads, -2 = reasonable auto]
 -ilp  InterleavedPic     Use additional image buffer with interleaved layout for IV-PSNR 
                          (improves performance at a cost of increased memory usage
                          optional, default=1)
 -v    VerboseLevel       Verbose level (optional, default=1)

 -c    "config.cfg"       External config file - in INI format (optional)

-----------------------------------------------------------------------------
VerboseLevel:
  0 = final (average) metric values only
  1 = 0 + configuration + detected frame numbers
  2 = 1 + argc/argv + frame level metric values
  3 = 2 + computing time (could slightly slow down computations)
  4 = 3 + IV specific debug data (GlobalColorShift, R2T+T2R, NumNonMasked)
  9 = stdout flood 

-----------------------------------------------------------------------------
Example - commandline parameters:
  IVPSNR -i0 "A.yuv" -i1 "B.yuv" -ps 2048x1088 -bd 10 -cf 420 -v 3 -r "r.txt"

-----------------------------------------------------------------------------
Example - config file:
  InputFile0      = "A.yuv"
  InputFile1      = "B.yuv"
  PictureWidth    = 2048
  PictureHeight   = 1088
  BitDepth        = 10
  ChromaFormat    = 420
  VerboseLevel    = 3
  ResultFile      = "results.txt"


=============================================================================
)PMBBRAWSTRING";

//===============================================================================================================================================================================================================

void xAppQMIV::registerCmdParams()
{
  //dispatcher params to be ignored
  m_CfgParser.addCmdFake("", "DispatchForceMFL");
  m_CfgParser.addCmdFake("", "DispatchVerbose" );
  //basic io
  m_CfgParser.addCmdParm("i0" , "InputFile0"       , "", "InputFile0"          );
  m_CfgParser.addCmdParm("i1" , "InputFile1"       , "", "InputFile1"          );  
  m_CfgParser.addCmdParm("ff" , "FileFormat"       , "", "FileFormat"          );
  m_CfgParser.addCmdParm("ps" , "PictureSize"      , "", "PictureSize"         );
  m_CfgParser.addCmdParm("pw" , "PictureWidth"     , "", "PictureWidth"        );
  m_CfgParser.addCmdParm("ph" , "PictureHeight"    , "", "PictureHeight"       );
  m_CfgParser.addCmdParm("pf" , "PictureFormat"    , "", "PictureFormat"       );
  m_CfgParser.addCmdParm("bd" , "BitDepth"         , "", "BitDepth"            );
  m_CfgParser.addCmdParm("cf" , "ChromaFormat"     , "", "ChromaFormat"        );
  m_CfgParser.addCmdParm("s0" , "StartFrame0"      , "", "StartFrame0"         );
  m_CfgParser.addCmdParm("s1" , "StartFrame1"      , "", "StartFrame1"         );
  m_CfgParser.addCmdParm("nf" , "NumberOfFrames"   , "", "NumberOfFrames"      );
  m_CfgParser.addCmdParm("r"  , "ResultFile"       , "", "ResultFile"          );
  m_CfgParser.addCmdList("ml" , "MetricList"       , "", "MetricList", ','     );
  //mask io
  m_CfgParser.addCmdParm("im" , "InputFileM"       , "", "InputFileM"          );
  m_CfgParser.addCmdParm("bdm", "BitDepthM"        , "", "BitDepthM"           );
  m_CfgParser.addCmdParm("cfm", "ChromaFormatM"    , "", "ChromaFormatM"       );
  //erp
  m_CfgParser.addCmdFlag("erp", "Equirectangular"  , "", "Equirectangular", "1");
  m_CfgParser.addCmdParm("lor", "LonRangeDeg"      , "", "LonRangeDeg"         );
  m_CfgParser.addCmdParm("lar", "LatRangeDeg"      , "", "LatRangeDeg"         );
  //colorspace
  m_CfgParser.addCmdParm("csi", "ColorSpaceInput"  , "", "ColorSpaceInput"     );
  m_CfgParser.addCmdParm("csm", "ColorSpaceMetric" , "", "ColorSpaceMetric"    );
  //iv-specific
  m_CfgParser.addCmdParm("sr" , "SearchRange"      , "", "SearchRange"         );
  m_CfgParser.addCmdParm("cws", "CmpWeightsSearch" , "", "CmpWeightsSearch"    );
  m_CfgParser.addCmdParm("cwa", "CmpWeightsAverage", "", "CmpWeightsAverage"   );
  m_CfgParser.addCmdParm("unc", "UnnoticeableCoef" , "", "UnnoticeableCoef"    );
  //validation 
  m_CfgParser.addCmdParm("ipa", "InvalidPelActn"   , "", "InvalidPelActn"      );
  m_CfgParser.addCmdParm("nma", "NameMismatchActn" , "", "NameMismatchActn"    );
  //operation
  m_CfgParser.addCmdParm("nth", "NumberOfThreads"  , "", "NumberOfThreads"     );
  m_CfgParser.addCmdParm("ilp", "InterleavedPic"   , "", "InterleavedPic"      );
  m_CfgParser.addCmdParm("v"  , "VerboseLevel"     , "", "VerboseLevel"        );  
}
bool xAppQMIV::loadConfiguration(int argc, const char* argv[])
{
  bool CommandlineResult = m_CfgParser.loadFromCmdln(argc, argv);
  if(!CommandlineResult) 
  { 
    m_ErrorLog += "! invalid commandline\n";
    m_ErrorLog += m_CfgParser.getParsingLog();
  }
  return CommandlineResult;
}
bool xAppQMIV::readConfiguration()
{
  bool AnyError = false;

  //basic io ----------------------------------------------------------------------------------------------------------
  m_InputFile[0] = m_CfgParser.getParam1stArg("InputFile0", std::string(""));
  m_InputFile[1] = m_CfgParser.getParam1stArg("InputFile1", std::string(""));
  if(m_InputFile[0].empty()) { m_ErrorLog += "!  InputFile0 is empty\n"; AnyError = true; }
  if(m_InputFile[1].empty()) { m_ErrorLog += "!  InputFile1 is empty\n"; AnyError = true; }
  m_FileFormat   = m_CfgParser.cvtParam1stArg("FileFormat", eFileFmt::RAW, xStr2FileFmt);

  if(m_CfgParser.findParam("PictureSize"))
  {
    std::string PictureSizeS = m_CfgParser.getParam1stArg("PictureSize", std::string(""));
    m_PictureSize = xFmtScn::scanResolution(PictureSizeS);
    if(m_PictureSize[0] <= 0 || m_PictureSize[1] <= 0) { m_ErrorLog += "!  Invalid PictureSize value\n"; AnyError = true; }
  }
  else
  {
    int32 PictureWidth  = m_CfgParser.getParam1stArg("PictureWidth" , NOT_VALID);
    int32 PictureHeight = m_CfgParser.getParam1stArg("PictureHeight", NOT_VALID);
    m_PictureSize.set(PictureWidth, PictureHeight);
    if(PictureWidth  <= 0) { m_ErrorLog += "!  Invalid PictureWidth value\n" ; AnyError = true; }
    if(PictureHeight <= 0) { m_ErrorLog += "!  Invalid PictureHeight value\n"; AnyError = true; }
  }

  if(m_CfgParser.findParam("PictureFormat"))
  {
    std::string PictureFormatS = m_CfgParser.getParam1stArg("PictureFormat", std::string(""));
    eImgTp ImageType;
    std::tie(ImageType, m_ChromaFormat, m_BitDepth) = xFmtScn::scanPixelFormat(PictureFormatS);
    if(ImageType != eImgTp::YCbCr       ) { m_ErrorLog += "!  Invalid or unsuported ImageType value derrived from PictureFormat\n"; AnyError = true; }
    if(m_BitDepth < 8 || m_BitDepth > 14) { m_ErrorLog += "!  Invalid or unsuported BitDepth value derrived from PictureFormat\n"; AnyError = true; }
    if(m_ChromaFormat == eCrF::INVALID  ) { m_ErrorLog += "!  Invalid or unsuported ChromaFormat value derrived from PictureFormat\n"; AnyError = true; }
  }
  else
  {
    m_BitDepth     = m_CfgParser.getParam1stArg("BitDepth"    , 8);
    m_ChromaFormat = m_CfgParser.cvtParam1stArg("ChromaFormat", eCrF::CF420, xStr2CrF);
    if(m_BitDepth < 8 || m_BitDepth > 14) { m_ErrorLog += "!  Invalid or unsuported BitDepth value\n"; AnyError = true; }
    if(m_ChromaFormat == eCrF::INVALID  ) { m_ErrorLog += "!  Invalid or unsuported ChromaFormat value\n"; AnyError = true; }
  }

  m_StartFrame[0]      = m_CfgParser.getParam1stArg("StartFrame0", 0);
  m_StartFrame[1]      = m_CfgParser.getParam1stArg("StartFrame1", 0);
  if(m_StartFrame[0] < 0 || m_StartFrame[1] < 0) { m_ErrorLog += "!  StartFrame value cannot be negative\n"; AnyError = true; }

  m_NumberOfFrames     = m_CfgParser.getParam1stArg("NumberOfFrames", -1); 

  m_ResultFile         = m_CfgParser.getParam1stArg("ResultFile" , std::string(""));

  if(m_CfgParser.findParam("MetricList"))
  {
    xCfgINI::stringVx MetricListVS = m_CfgParser.getParamArgs("MetricList");

    for(const std::string& MetricS : MetricListVS)
    {
      if(xString::toUpper(MetricS) == "ALL") { std::fill(m_CalcMetric.begin(), m_CalcMetric.end(), true); break; }
      eMetric Metric = xStrToMetric(MetricS);
      if(Metric != eMetric::UNDEFINED) { m_CalcMetric[(int32)Metric] = true; }
      else { m_ErrorLog += fmt::format("!  MetricList contains not valid entry. Token \"{}\" in not a metric. \n", MetricS); AnyError = true; }
    }
  }
  else
  {
    m_CalcMetric[(int32)eMetric::PSNR  ] = true;
    m_CalcMetric[(int32)eMetric::WSPSNR] = true;
    m_CalcMetric[(int32)eMetric::IVPSNR] = true;
    m_CalcMetric[(int32)eMetric::IVSSIM] = true;
  }

  //mask io -----------------------------------------------------------------------------------------------------------
  m_InputFile[2]       = m_CfgParser.getParam1stArg("InputFileM"   , std::string(""));
  m_BitDepthM          = m_CfgParser.getParam1stArg("BitDepthM"    , m_BitDepth     );
  m_ChromaFormatM      = m_CfgParser.cvtParam1stArg("ChromaFormatM", m_ChromaFormat, xStr2CrF);
  if(m_BitDepthM < 8 || m_BitDepthM > 14) { m_ErrorLog += "!  Invalid or unsuported BitDepthM value\n"; AnyError = true; }
  if(m_ChromaFormat == eCrF::INVALID    ) { m_ErrorLog += "!  Invalid or unsuported ChromaFormatM value\n"; AnyError = true; }

  //erp ---------------------------------------------------------------------------------------------------------------
  m_IsEquirectangular  = m_CfgParser.getParam1stArg("Equirectangular", false          );
  m_LonRangeDeg        = m_CfgParser.getParam1stArg("LonRangeDeg"    , 360            );
  m_LatRangeDeg        = m_CfgParser.getParam1stArg("LatRangeDeg"    , 180            );

  //colorspace --------------------------------------------------------------------------------------------------------
  m_ColorSpaceInput    = m_CfgParser.cvtParam1stArg("ColorSpaceInput" , eClrSpcApp::YCbCr, xStr2ClrSpcApp);
  m_ColorSpaceMetric   = m_CfgParser.cvtParam1stArg("ColorSpaceMetric", eClrSpcApp::YCbCr, xStr2ClrSpcApp);

  if(m_ColorSpaceInput != m_ColorSpaceMetric)
  {
    if(isYCbCr(m_ColorSpaceInput) && isYCbCr(m_ColorSpaceMetric)) { m_ErrorLog += fmt::format("!  YCbCr to YCbCr conversion is not supported.\n"); AnyError = true; }
    if(m_ColorSpaceInput != m_ColorSpaceMetric && (m_ColorSpaceInput == eClrSpcApp::YCbCr || m_ColorSpaceMetric == eClrSpcApp::YCbCr)) { m_ErrorLog += fmt::format("!  Generic YCbCr cannot be used for colorspace conversion.\n"); AnyError = true; }
  }

  //iv-specific -------------------------------------------------------------------------------------------------------
  m_SearchRange = m_CfgParser.getParam1stArg("SearchRange", xIVPSNR::c_DefaultSearchRange);
  if(m_SearchRange < 1) { m_ErrorLog += "!  SearchRange value must non-zero\n"; AnyError = true; }

  std::string CmpWeightsSearchS  = m_CfgParser.getParam1stArg("CmpWeightsSearch" , xFmtScn::formatIntWeights(xCorrespPixelShiftPrms::c_DefaultCmpWeights));
  std::string CmpWeightsAverageS = m_CfgParser.getParam1stArg("CmpWeightsAverage", xFmtScn::formatIntWeights(xCorrespPixelShiftPrms::c_DefaultCmpWeights));
  std::string UnnoticeableCoefS  = m_CfgParser.getParam1stArg("UnnoticeableCoef" , xFmtScn::formatFltWeights(xGlobClrDiffPrms      ::c_DefaultUnntcbCoef));
  m_CmpWeightsSearch  = xFmtScn::scanIntWeights(CmpWeightsSearchS );
  m_CmpWeightsAverage = xFmtScn::scanIntWeights(CmpWeightsAverageS);
  m_UnnoticeableCoef  = xFmtScn::scanFltWeights(UnnoticeableCoefS );

  //validation --------------------------------------------------------------------------------------------------------
  std::string InvalidPelActnS   = m_CfgParser.getParam1stArg("InvalidPelActn"  , "STOP");
  std::string NameMismatchActnS = m_CfgParser.getParam1stArg("NameMismatchActn", "WARN");
  m_InvalidPelActn   = xStr2Actn(InvalidPelActnS  );
  m_NameMismatchActn = xStr2Actn(NameMismatchActnS);

  //operation ---------------------------------------------------------------------------------------------------------
  m_NumberOfThreads = m_CfgParser.getParam1stArg("NumberOfThreads", -2  );
  m_InterleavedPic  = m_CfgParser.getParam1stArg("InterleavedPic" , true);
  m_VerboseLevel    = m_CfgParser.getParam1stArg("VerboseLevel"   , 1   );

  //derrived ----------------------------------------------------------------------------------------------------------
  m_UseMask      = !m_InputFile[2].empty();
  m_UsePicI      = m_InterleavedPic && getCalcMetric(eMetric::IVPSNR);
  m_NumInputsCur = !m_UseMask ? 2 : 3;
  m_CvtYCbCr2RGB = isDefinedYCbCr(m_ColorSpaceInput) && isRGB(m_ColorSpaceMetric);
  m_CvtRGB2YCbCr = isRGB(m_ColorSpaceInput) && isDefinedYCbCr(m_ColorSpaceMetric);
  m_ReorderRGB   = isRGB(m_ColorSpaceInput) && m_ColorSpaceInput != eClrSpcApp::RGB && m_ColorSpaceMetric == eClrSpcApp::RGB;
  m_CalcPSNRs    = getCalcMetric(eMetric::PSNR) || getCalcMetric(eMetric::WSPSNR) || getCalcMetric(eMetric::IVPSNR);
  m_CalcSSIMs    = getCalcMetric(eMetric::SSIM) || getCalcMetric(eMetric::IVSSIM);
  m_CalcIVs      = getCalcMetric(eMetric::IVPSNR) || getCalcMetric(eMetric::IVSSIM);
  m_CalcSCP      = getCalcMetric(eMetric::IVSSIM);
  m_CalcGCD      = m_CalcIVs || m_CalcSCP;
  m_PicMargin    = xRoundUpToNearestMultiple(m_SearchRange, 2);
  m_WindowSize   = 2 * m_SearchRange + 1;
  m_PrintFrame   = m_VerboseLevel >= 2;
  m_GatherTime   = m_VerboseLevel >= 3;
  m_PrintDebug   = m_VerboseLevel >= 4;

  //post-validation ---------------------------------------------------------------------------------------------------
  if(m_UseMask && m_CalcSSIMs) { m_ErrorLog += "! Structural Similarity metrics cannot be combined with Mask mode\n"; AnyError = true; }

  return !AnyError;
}
std::string xAppQMIV::formatConfiguration()
{
  std::string Config; Config.reserve(xMemory::c_MemSizePageBase);  
  //basic io
  Config += "Run-time configuration:\n";
  Config += fmt::format("InputFile0        = {}\n"  , m_InputFile[0]);
  Config += fmt::format("InputFile1        = {}\n"  , m_InputFile[1]);
  Config += fmt::format("FileFormat        = {}\n"  , xFileFmt2Str(m_FileFormat));
  Config += fmt::format("PictureSize       = {}\n"  , xFmtScn::formatResolution(m_PictureSize) );
  Config += fmt::format("BitDepth          = {}\n"  , m_BitDepth);
  Config += fmt::format("ChromaFormat      = {}\n"  , xCrF2Str(m_ChromaFormat));
  Config += fmt::format("StartFrame0       = {}\n"  , m_StartFrame[0]    );
  Config += fmt::format("StartFrame1       = {}\n"  , m_StartFrame[1]    );
  Config += fmt::format("NumberOfFrames    = {}{}\n", m_NumberOfFrames, m_NumberOfFrames==NOT_VALID ? "  (all)" : "");
  Config += fmt::format("ResultFile        = {}\n"  , m_ResultFile.empty() ? "(unused)" : m_ResultFile);
  Config += "MetricList        = ";
  for(int32 m = 0; m < c_MetricsNum; m++) { if(m_CalcMetric[m]) { Config += xMetricToStr((eMetric)m) + ", "; } }
  Config.resize(Config.size() - 2); //cut trailing ", "
  Config += "\n";
  //mask io
  Config += fmt::format("InputFileM        = {}\n"  , m_InputFile[2].empty() ? "(unused)" : m_InputFile[2]);
  Config += fmt::format("BitDepthM         = {}{}\n", m_BitDepthM              , m_UseMask ? "" : "  (irrelevant)");
  Config += fmt::format("ChromaFormatM     = {}{}\n", xCrF2Str(m_ChromaFormatM), m_UseMask ? "" : "  (irrelevant)");
  //erp
  Config += fmt::format("Equirectangular   = {:d}\n", m_IsEquirectangular);
  Config += fmt::format("LonRangeDeg       = {}{}\n", m_LonRangeDeg, m_IsEquirectangular ? "" : "  (irrelevant)");
  Config += fmt::format("LatRangeDeg       = {}{}\n", m_LatRangeDeg, m_IsEquirectangular ? "" : "  (irrelevant)");
  //colorspace
  Config += fmt::format("ColorSpaceInput   = {}{}\n", xClrSpcApp2Str(m_ColorSpaceInput ), m_CvtRGB2YCbCr || m_CvtYCbCr2RGB || m_ReorderRGB ? "" : "  (irrelevant)");
  Config += fmt::format("ColorSpaceMetric  = {}{}\n", xClrSpcApp2Str(m_ColorSpaceMetric), m_CvtRGB2YCbCr || m_CvtYCbCr2RGB || m_ReorderRGB ? "" : "  (irrelevant)");
  //iv-specific
  Config += fmt::format("SearchRange       = {}{}\n", m_SearchRange, m_SearchRange == xIVPSNR::c_DefaultSearchRange ? "  (default)" : "  (custom)");
  Config += fmt::format("CmpWeightsSearch  = {}{}\n", xFmtScn::formatIntWeights(m_CmpWeightsSearch ), m_CmpWeightsSearch  == xCorrespPixelShiftPrms::c_DefaultCmpWeights ? "  (default)" : "  (custom)");
  Config += fmt::format("CmpWeightsAverage = {}{}\n", xFmtScn::formatIntWeights(m_CmpWeightsAverage), m_CmpWeightsAverage == xCorrespPixelShiftPrms::c_DefaultCmpWeights ? "  (default)" : "  (custom)");
  Config += fmt::format("UnnoticeableCoef  = {}{}\n", xFmtScn::formatFltWeights(m_UnnoticeableCoef ), m_UnnoticeableCoef  == xGlobClrDiffPrms      ::c_DefaultUnntcbCoef ? "  (default)" : "  (custom)");
  //validation 
  Config += fmt::format("InvalidPelActn    = {}\n", xActn2Str(m_InvalidPelActn  ));
  Config += fmt::format("NameMismatchActn  = {}\n", xActn2Str(m_NameMismatchActn));
  //operation
  Config += fmt::format("NumberOfThreads   = {}{}\n", m_NumberOfThreads, m_NumberOfThreads == -1 ? "  (all)" : m_NumberOfThreads == -2 ? "  (auto)" : "");
  Config += fmt::format("InterleavedPic    = {:d}\n", m_InterleavedPic);
  Config += fmt::format("VerboseLevel      = {}\n"  , m_VerboseLevel  );
  Config += "\n";
  //derrived
  Config += fmt::format("Run-time derrived parameters:\n");
  Config += fmt::format("WindowSize        = {}x{}\n", m_WindowSize, m_WindowSize);
  Config += fmt::format("PictureMargin     = {}\n", m_PicMargin);
  Config += fmt::format("UseMask           = {:d}\n", m_UseMask);
  Config += "\n";
  //metric description
  Config += fmt::format("Selected metrics:\n");
  for(int32 m = 0; m < c_MetricsNum; m++) { if(m_CalcMetric[m]) { Config += fmt::format("{:<9} - {}\n", xMetricToStr((eMetric)m), xMetricInfo::Description[m]); } }
  Config += "\n";

  return Config;
}
eRes xAppQMIV::validateInputFiles()
{
  bool AnyError = false;

  if(m_NameMismatchActn == eActn::WARN || m_NameMismatchActn == eActn::STOP)
  {
    for(int32 i = 0; i < 2; i++)
    {
      const auto [ValidI, MessageI] = xFileNameScn::validateFileParams(m_InputFile[i], m_PictureSize, m_BitDepth, m_ChromaFormat);
      if(!ValidI) { m_ErrorLog += MessageI; AnyError = true; }
    }
    if(m_UseMask)
    {
      const auto [ValidM, MessageM] = xFileNameScn::validateFileParams(m_InputFile[2], m_PictureSize, m_BitDepthM, m_ChromaFormatM);
      if(!ValidM) { m_ErrorLog += MessageM; AnyError = true; }
    }
  }

  if(AnyError) { return m_NameMismatchActn == eActn::STOP ? eRes::Error : eRes::Warning; }
  return eRes::Good;
}
std::string xAppQMIV::formatWarnings()
{
  std::string Warnings = "";

  //check weights
  if constexpr (!xc_USE_RUNTIME_CMPWEIGHTS)
  {
    Warnings += fmt::format("CONFIGURATION WARNING: Software was build with USE_RUNTIME_CMPWEIGHTS option disabled. Config parameter CmpWeightsSearch will be ignored. Default component weights will be used. The default weights are {}.\n\n", xFmtScn::formatIntWeights(xCorrespPixelShiftPrms::c_DefaultCmpWeights));
  }

  //check conformance
  if(m_SearchRange != xCorrespPixelShiftPrms::c_DefaultSearchRange)
  {
    Warnings += fmt::format("CONFORMANCE WARNING: Software was executed with SearchRange different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for immersive video. The default range is DefaultSearchRange={}.\n\n", xCorrespPixelShiftPrms::c_DefaultSearchRange);
  }
  if(xc_USE_RUNTIME_CMPWEIGHTS && m_CmpWeightsSearch != xCorrespPixelShiftPrms::c_DefaultCmpWeights)
  {
    Warnings += fmt::format("CONFORMANCE WARNING: Software was executed with CmpWeightsSearch different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for immersive video. The default weights are DefaultCmpWeights={}.\n\n", xFmtScn::formatIntWeights(xCorrespPixelShiftPrms::c_DefaultCmpWeights));
  }
  if(xc_USE_RUNTIME_CMPWEIGHTS && m_CmpWeightsAverage != xCorrespPixelShiftPrms::c_DefaultCmpWeights)
  {
    Warnings += fmt::format("CONFORMANCE WARNING: Software was executed with CmpWeightsAverage different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for immersive video. The default weights are DefaultCmpWeights={}.\n\n", xFmtScn::formatIntWeights(xCorrespPixelShiftPrms::c_DefaultCmpWeights));
  }
  if(m_UnnoticeableCoef != xGlobClrDiffPrms::c_DefaultUnntcbCoef)
  {
    Warnings += fmt::format("CONFORMANCE WARNING: Software was executed with UnnoticeableCoef different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for immersive video. The default coeffs are DefaultUnnoticeableCoef={}.\n\n", xFmtScn::formatFltWeights(xGlobClrDiffPrms::c_DefaultUnntcbCoef));
  }

  //check performance
  if(m_SearchRange > xCorrespPixelShiftPrms::c_DefaultSearchRange)
  {
    Warnings += fmt::format("PERFORMANCE WARNING: Software was executed with SearchRange wider than default one. This leads to higher computational complexity and longer calculation time. The default range is DefaultSearchRange=%d.\n\n", xCorrespPixelShiftPrms::c_DefaultSearchRange);
  }
  if(xc_USE_RUNTIME_CMPWEIGHTS && m_CmpWeightsSearch == xCorrespPixelShiftPrms::c_DefaultCmpWeights && !X_CORRESPPIXELSHIFT_CAN_USE_SSE)
  {
    Warnings += fmt::format("PERFORMANCE WARNING: Software was build with USE_RUNTIME_CMPWEIGHTS option enabled and default weights was selected. To speed up computation of IV-PSNR with default weights - dissable USE_RUNTIME_CMPWEIGHTS option.\n\n");
  }

  return Warnings;
}
void xAppQMIV::setupMultithreading()
{
  m_HardwareConcurency  = std::thread::hardware_concurrency();
  m_NumberOfThreadsUsed = 0;
  if(m_NumberOfThreads >=  1) { m_NumberOfThreadsUsed = xMin(m_NumberOfThreads, m_HardwareConcurency); }
  if(m_NumberOfThreads == -1) { m_NumberOfThreadsUsed = m_HardwareConcurency; }
  if(m_NumberOfThreads == -2) { m_NumberOfThreadsUsed = m_CalcSSIMs ? m_HardwareConcurency : xMin(8, m_HardwareConcurency); }
  if(m_NumberOfThreadsUsed > 0)
  {
    m_ThreadPool = new xThreadPool;
    m_ThreadPool->create(m_NumberOfThreadsUsed, m_PictureSize.getY() + 1);
    m_TPI.init(m_ThreadPool, 4, 4);
  }
}
void xAppQMIV::ceaseMultithreading()
{
  if(m_NumberOfThreadsUsed)
  {
    m_TPI.uininit();
    m_ThreadPool->destroy();
    m_ThreadPool = nullptr;
  }
}
std::string xAppQMIV::formatMultithreading()
{
  std::string Info = "";
  Info += fmt::format("Multithreading:\n");
  Info += fmt::format("HardwareConcurency  = {}\n", m_HardwareConcurency );
  Info += fmt::format("NumberOfThreadsUsed = {}\n", m_NumberOfThreadsUsed);
  return Info;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

eRes xAppQMIV::setupSeqAndBuffs()
{
  const char  FID[NumInputsMax] = { '0', '1', 'M' };
  const int32 BDs[NumInputsMax] = { m_BitDepth    , m_BitDepth    , m_BitDepthM     };
  const eCrF  CFs[NumInputsMax] = { m_ChromaFormat, m_ChromaFormat, m_ChromaFormatM };
    
  //check if file exists
  if(m_FileFormat == eFileFmt::RAW)
  {
    for(int32 i = 0; i < m_NumInputsCur; i++)
    {
      if(!xFile::exists(m_InputFile[i])) { xCfgINI::printError(fmt::format("ERROR --> InputFile{} does not exist ({})", FID[i], m_InputFile[i])); return eRes::Error; }
    }
  }
  else if(m_FileFormat == eFileFmt::PNG)
  {
    for(int32 i = 0; i < m_NumInputsCur; i++)
    {
      std::string InputFile0 = fmt::format(m_InputFile[i], 0);
      std::string InputFile1 = fmt::format(m_InputFile[i], 1);
      if(!xFile::exists(InputFile0) && !xFile::exists(InputFile1)) { xCfgINI::printError(fmt::format("ERROR --> InputFile{} does not exist ({}) [Checked {} {}]", FID[i], m_InputFile[i], InputFile0, InputFile1)); return eRes::Error; }
    }
  }

  //file size
  if(m_FileFormat == eFileFmt::RAW)
  {
    int64 SizeOfInputFile[NumInputsMax] = { 0 };
    for(int32 i = 0; i < m_NumInputsCur; i++)
    {
      SizeOfInputFile[i] = xFile::size(m_InputFile[i]);
      if(m_VerboseLevel >= 1) { fmt::print("SizeOfInputFile{} = {}\n", FID[i], SizeOfInputFile[i]); }
    }
  }

  //create input sequences 
  switch(m_FileFormat)
  {
  case eFileFmt::RAW: for(int32 i = 0; i < m_NumInputsCur; i++) { m_SeqIn[i] = new xSeq   (m_PictureSize, BDs[i], CFs[i]); } break;
  case eFileFmt::PNG: for(int32 i = 0; i < m_NumInputsCur; i++) { m_SeqIn[i] = new xSeqPNG(m_PictureSize, uint16_max    ); } break;
  default: xCfgINI::printError(fmt::format("ERROR --> unsupported FileFormat ({})", xFileFmt2Str(m_FileFormat))); return eRes::Error;
  }

  //open input sequences 
  for(int32 i = 0; i < m_NumInputsCur; i++)
  {
    xSeqBase::tResult Result = m_SeqIn[i]->openFile(m_InputFile[i], xSeq::eMode::Read);
    if(!Result) { xCfgINI::printError(fmt::format("ERROR --> InputFile opening failure ({}) {}", m_InputFile[i], Result.format())); return eRes::Error; }
  }

  //num of frames per input file
  int32 NumOfFrames[NumInputsMax] = { 0 };
  for(int32 i = 0; i < m_NumInputsCur; i++)
  {
    NumOfFrames[i] = m_SeqIn[i]->getNumOfFrames();
    if(m_VerboseLevel >= 1) { fmt::print("DetectedFrames{}  = {}\n", i, NumOfFrames[i]); }
    if(m_StartFrame[i] >= NumOfFrames[i]) { xCfgINI::printError(fmt::format("ERROR --> StartFrame{} >= DetectedFrames{} for ({})", FID[i], FID[i], m_InputFile[i])); return eRes::Error; }
  }

  //num of frames to process
  int32 MinSeqNumFrames = xMin(NumOfFrames[0], NumOfFrames[1]);
  int32 MinSeqRemFrames = xMin(NumOfFrames[0] - m_StartFrame[0], NumOfFrames[1] - m_StartFrame[1]);
  m_NumFrames           = xMin(m_NumberOfFrames > 0 ? m_NumberOfFrames : MinSeqNumFrames, MinSeqRemFrames);
  int32 FirstFrame[NumInputsMax] = { 0 };
  for(int32 i = 0; i < 2; i++) { FirstFrame[i] = xMin(m_StartFrame[i], NumOfFrames[i] - 1); }
  if(m_VerboseLevel >= 1) { fmt::print("FramesToProcess  = {}\n", m_NumFrames); }
  fmt::print("\n");

  if(m_UseMask && (m_NumFrames > NumOfFrames[2])) { xCfgINI::printError(fmt::format("ERROR --> FramesToProcess > NumOfFramesM")); return eRes::Error; }
  
  //seeek sequences 
  for(int32 i = 0; i < m_NumInputsCur; i++)
  { 
    if(FirstFrame[i] != 0) 
    { 
      xSeqBase::tResult Result = m_SeqIn[i]->seekFrame(FirstFrame[i]);
      if(!Result) { xCfgINI::printError(fmt::format("ERROR --> InputFile seeking failure ({}) {}", m_InputFile[i], Result.format())); return eRes::Error; }
    }
  }

  //input buffers
  for(int32 i = 0; i < m_NumInputsCur; i++) { m_PicInP[i].create(m_PictureSize, BDs[i], m_PicMargin); }
  if(m_UsePicI) { for(int32 i = 0; i < NumInputsSeq; i++) { m_PicInI[i].create(m_PictureSize, m_BitDepth, m_PicMargin); } }

  //SCP buffers
  if(m_CalcGCD) { for(int32 i = 0; i < NumInputsSeq; i++) { m_PicSCP[i].create(m_PictureSize, m_BitDepth, m_PicMargin); } }

  return eRes::Good;
}
eRes xAppQMIV::ceaseSeqAndBuffs()
{
  //input sequences 
  for(int32 i = 0; i < m_NumInputsCur; i++) { m_SeqIn[i]->closeFile(); }
  for(int32 i = 0; i < m_NumInputsCur; i++) { m_SeqIn[i]->destroy(); m_SeqIn[i] = nullptr; }
  //input buffers
  for(int32 i = 0; i < m_NumInputsCur; i++) { m_PicInP[i].destroy  (); }
  if(m_UsePicI) { for(int32 i = 0; i < NumInputsSeq; i++) { m_PicInI[i].destroy(); } }
  //SCP buffers
  if(m_CalcGCD) { for(int32 i = 0; i < NumInputsSeq; i++) { m_PicSCP[i].destroy(); } }
  return eRes::Good;
}
void xAppQMIV::createProcessors()
{  
  const int32 PictureWidth  = m_PictureSize.getX();
  const int32 PictureHeight = m_PictureSize.getY();

  if(m_CalcGCD)
  {
    m_ProcGCD.setUnntcbCoef(m_UnnoticeableCoef);
    if(m_NumberOfThreadsUsed > 0) { m_ProcGCD.initThreadPool(m_ThreadPool, PictureHeight + 1); }
  }

  if(m_CalcSCP)
  {
    m_ProcSCP.setSearchRange      (m_SearchRange      );
    m_ProcSCP.setCmpWeightsSearch (m_CmpWeightsSearch );
    m_ProcSCP.setCmpWeightsAverage(m_CmpWeightsAverage);
    if(m_NumberOfThreadsUsed > 0) { m_ProcSCP.initThreadPool(m_ThreadPool, PictureHeight + 1); }
  }

  if(m_CalcPSNRs)
  {
    m_ProcPSNR.setSearchRange      (m_SearchRange      );
    m_ProcPSNR.setCmpWeightsSearch (m_CmpWeightsSearch );
    m_ProcPSNR.setCmpWeightsAverage(m_CmpWeightsAverage);
    m_ProcPSNR.setUnntcbCoef       (m_UnnoticeableCoef );
    if(m_NumberOfThreadsUsed > 0) { m_ProcPSNR.initThreadPool(m_ThreadPool, PictureHeight + 1); }
    m_ProcPSNR.initRowBuffers(PictureHeight);
    if(m_IsEquirectangular) { m_ProcPSNR.initWS(true, PictureWidth, PictureHeight, m_BitDepth, m_LonRangeDeg, m_LatRangeDeg); }
  }

  if(m_CalcSSIMs)
  {
    m_ProcSSIM.create(m_PictureSize, m_BitDepth, m_PicMargin);
    m_ProcSSIM.setSearchRange      (m_SearchRange      );
    m_ProcSSIM.setCmpWeightsSearch (m_CmpWeightsSearch );
    m_ProcSSIM.setCmpWeightsAverage(m_CmpWeightsAverage);
    m_ProcSSIM.setUnntcbCoef       (m_UnnoticeableCoef );
    if(m_NumberOfThreadsUsed > 0) { m_ProcSSIM.initThreadPool(m_ThreadPool, PictureHeight + 1); }
    m_ProcSSIM.initRowBuffers(PictureHeight);
    if(m_IsEquirectangular) { m_ProcPSNR.initWS(true, PictureWidth, PictureHeight, m_BitDepth, m_LonRangeDeg, m_LatRangeDeg); }
  }

  for(int32 m = 0; m < c_MetricsNum; m++)
  {
    if(m_CalcMetric[m]) 
    {
      m_MetricData[m].initMetric  ((eMetric)m, m_NumFrames);
      m_MetricData[m].initSuffixes(m_UseMask, isRGB(m_ColorSpaceMetric));
      m_MetricData[m].initCmpWeightsAverage(m_CmpWeightsAverage);
    }
  }
  
  if(m_PrintDebug)
  {
    if(m_CalcPSNRs) { m_ProcPSNR.setDebugCallbackQAP([this](flt64 R2T, flt64 T2R) { m_LastR2T = R2T; m_LastT2R = T2R; }); }
    if(m_CalcSSIMs) { m_ProcSSIM.setDebugCallbackQAP([this](flt64 R2T, flt64 T2R) { m_LastR2T = R2T; m_LastT2R = T2R; }); }
  }
}
void xAppQMIV::destroyProcessors()
{
  if(m_CalcSSIMs)
  {
    m_ProcSSIM.destroy();
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

eRes xAppQMIV::processAllFrames()
{
  m_ProcBegTime  = tClock::now();
  m_ProcBegTicks = xTSC();

  for(int32 f = 0; f < m_NumFrames; f++)
  {
    if(m_PrintDebug) { fmt::print("Frame {:08d}  ", f); }

    uint64 T0 = m_GatherTime ? xTSC() : 0;

    //reading
    std::vector<xSeqBase::tResult> ReadResult(m_NumInputsCur, xSeqBase::eRetv::Success);
    for(int32 i = 0; i < m_NumInputsCur; i++) { m_TPI.addWaitingTask([this, &ReadResult, i](int32 /*ThId*/) { ReadResult[i] = m_SeqIn[i]->readFrame(&(m_PicInP[i])); }); }
    m_TPI.waitUntilTasksFinished(m_NumInputsCur);
    for(int32 i = 0; i < m_NumInputsCur; i++) { if(!ReadResult[i]) { xCfgINI::printError(fmt::format("ERROR --> InputFile read error ({}) {}", m_InputFile[i], ReadResult[i].format())); return eRes::Error; } }
    
    uint64 T1 = m_GatherTime ? xTSC() : 0;

    //validation
    if(m_InvalidPelActn != eActn::SKIP) 
    { 
      eRes ValidationRes = validateFrames(f);
      if(ValidationRes != eRes::Good) { return eRes::Error; }
    }

    uint64 T2 = m_GatherTime ? xTSC() : 0;

    preprocessFrames(f); //preprocessing

    uint64 T3 = m_GatherTime ? xTSC() : 0;
    
    if(m_CalcGCD)
    {
      if(m_UseMask) { m_GCD_R2T = m_ProcGCD.CalcGlobalColorDiffM(&m_PicInP[0], &m_PicInP[1], &m_PicInP[2], m_NumNonMasked); }
      else          { m_GCD_R2T = m_ProcGCD.CalcGlobalColorDiff (&m_PicInP[0], &m_PicInP[1]                              ); }
      if(m_PrintDebug) { fmt::print("GCD-R2T {} {} {} {}    ", m_GCD_R2T[0], m_GCD_R2T[1], m_GCD_R2T[2], m_GCD_R2T[3]); }
    }
    if(m_PrintDebug) { fmt::print("\n"); }

    uint64 T4 = m_GatherTime ? xTSC() : 0;

    if(m_CalcSCP) { m_ProcSCP.GenShftCompPics(&m_PicSCP[1], &m_PicSCP[0], &m_PicInP[1], &m_PicInP[0], m_GCD_R2T); }

    uint64 T5 = m_GatherTime ? xTSC() : 0;

    if(getCalcMetric(eMetric::    PSNR)) { calcFrame____PSNR(f); }

    uint64 T6 = m_GatherTime ? xTSC() : 0;

    if(getCalcMetric(eMetric::  WSPSNR)) { calcFrame__WSPSNR(f); }

    uint64 T7 = m_GatherTime ? xTSC() : 0;

    if(getCalcMetric(eMetric::  IVPSNR)) { calcFrame__IVPSNR(f); }

    uint64 T8 = m_GatherTime ? xTSC() : 0;

    if(getCalcMetric(eMetric::    SSIM)) { calcFrame____SSIM(f); }

    uint64 T9 = m_GatherTime ? xTSC() : 0;

    if(getCalcMetric(eMetric::  IVSSIM)) { calcFrame__IVSSIM(f); }

    uint64 T10 = m_GatherTime ? xTSC() : 0;

    if(m_GatherTime)
    {
      m_Ticks____Load += (T1 - T0);
      m_TicksValidate += (T2 - T1);
      m_Ticks_Preproc += (T3 - T2);
      m_Ticks_____GCD += (T4 - T3);
      m_Ticks_____SCP += (T5 - T4);
      m_MetricData[(int32)eMetric::    PSNR].addTicks(T6  - T5 );
      m_MetricData[(int32)eMetric::  WSPSNR].addTicks(T7  - T6 );
      m_MetricData[(int32)eMetric::  IVPSNR].addTicks(T8  - T7 );
      m_MetricData[(int32)eMetric::    SSIM].addTicks(T9  - T8 );
      m_MetricData[(int32)eMetric::  IVSSIM].addTicks(T10 - T9 );
    }
  } //end of loop over frames

  m_ProcEndTime  = tClock::now();
  m_ProcEndTicks = xTSC();

  return eRes::Good;
}

eRes xAppQMIV::validateFrames(int32 /**/)
{
  std::vector<bool> CheckOK(m_NumInputsCur, true);
  for(int32 i = 0; i < m_NumInputsCur; i++) { m_TPI.addWaitingTask([this, &CheckOK, i](int32) { CheckOK[i] = m_PicInP[i].check(m_InputFile[i]); } ); }
  m_TPI.waitUntilTasksFinished(m_NumInputsCur);

  if(m_InvalidPelActn == eActn::CNCL)
  {
    for(int32 i = 0; i < m_NumInputsCur; i++) { if(!CheckOK[i]) { m_PicInP[i].conceal(); } }
  }

  if(m_InvalidPelActn==eActn::STOP)
  {
    for(int32 i = 0; i < m_NumInputsCur; i++) { if(!CheckOK[i]) { xCfgINI::printError(fmt::format("ERROR --> InputFile contains invalid values ({})", m_InputFile[i])); return eRes::Error; } }
  }

  return eRes::Good;
}
void xAppQMIV::preprocessFrames(int32 /**/)
{
  for(int32 CmpIdx = 0; CmpIdx < m_PicInP[0].getNumCmps(); CmpIdx++)
  {
    m_ExactCmps[CmpIdx] = m_PicInP[0].equalCmp(&m_PicInP[1], (eCmp)CmpIdx);
  }

  if(m_CvtYCbCr2RGB)
  {
    eClrSpcLC ColorSpace = xClrSpcAppToClrSpc(m_ColorSpaceInput);
    for(int32 i = 0; i < NumInputsSeq; i++) { m_TPI.addWaitingTask([this, i, ColorSpace](int32) { xColorSpace::ConvertYCbCr2RGB(
      m_PicInP[i].getAddr(eCmp::R ), m_PicInP[i].getAddr(eCmp::G ), m_PicInP[i].getAddr(eCmp::B ),
      m_PicInP[i].getAddr(eCmp::LM), m_PicInP[i].getAddr(eCmp::CB), m_PicInP[i].getAddr(eCmp::CR),
      m_PicInP[i].getStride(), m_PicInP[i].getStride(), m_PicInP[i].getWidth(), m_PicInP[i].getHeight(), m_PicInP[i].getBitDepth(), ColorSpace);
    } ); }
    m_TPI.waitUntilTasksFinished(NumInputsSeq);
  }

  if(m_CvtRGB2YCbCr)
  {
    eClrSpcLC ColorSpace = xClrSpcAppToClrSpc(m_ColorSpaceMetric);
    for(int32 i = 0; i < NumInputsSeq; i++) { m_TPI.addWaitingTask([this, i, ColorSpace](int32) { xColorSpace::ConvertRGB2YCbCr(
      m_PicInP[i].getAddr(eCmp::LM), m_PicInP[i].getAddr(eCmp::CB), m_PicInP[i].getAddr(eCmp::CR),
      m_PicInP[i].getAddr(eCmp::R ), m_PicInP[i].getAddr(eCmp::G ), m_PicInP[i].getAddr(eCmp::B ),      
      m_PicInP[i].getStride(), m_PicInP[i].getStride(), m_PicInP[i].getWidth(), m_PicInP[i].getHeight(), m_PicInP[i].getBitDepth(), ColorSpace);
    } ); }
    m_TPI.waitUntilTasksFinished(NumInputsSeq);
  }

  if(m_ReorderRGB)
  {
    for(int32 i = 0; i < NumInputsSeq; i++)
    {
      if(m_ColorSpaceInput == eClrSpcApp::BGR)
      {
        m_PicInP[i].swapComponents(eCmp::C0, eCmp::C2); //BGR --> RGB 
      }
      if(m_ColorSpaceInput == eClrSpcApp::GBR)
      {
        m_PicInP[i].swapComponents(eCmp::C0, eCmp::C1); //GBR --> BGR
        m_PicInP[i].swapComponents(eCmp::C0, eCmp::C2); //BGR --> RGB
      }
    }
  }

  for(int32 i = 0; i < m_NumInputsCur; i++) { m_TPI.addWaitingTask([this, i](int32) { m_PicInP[i].extend(); } ); }
  m_TPI.waitUntilTasksFinished(m_NumInputsCur);

  if(m_UsePicI)
  {
    for(int32 i = 0; i < NumInputsSeq; i++) { m_TPI.addWaitingTask([this, i](int32) { m_PicInI[i].rearrangeFromPlanar(&m_PicInP[i]); } ); }
    m_TPI.waitUntilTasksFinished(NumInputsSeq);
  }

  if(m_UseMask)
  {
    m_NumNonMasked = xPixelOps::CountNonZero(m_PicInP[2].getAddr(eCmp::LM), m_PicInP[2].getStride(), m_PicInP[2].getWidth(), m_PicInP[2].getHeight());
    if(m_PrintDebug) { fmt::print("NNM {}    ", m_NumNonMasked); }
  }  
}
void xAppQMIV::calcFrame____PSNR(int32 FrameIdx)
{
  flt64V4 PSNR  = xMakeVec4(0.0  );
  if(m_UseMask) { PSNR = m_ProcPSNR.calcPicPSNRM(&m_PicInP[0], &m_PicInP[1], &m_PicInP[2], m_NumNonMasked); }
  else          { PSNR = m_ProcPSNR.calcPicPSNR (&m_PicInP[0], &m_PicInP[1]                              ); }

  for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
  { 
    if(m_ExactCmps[CmpIdx])
    {
      PSNR[CmpIdx] = m_ProcPSNR.getFakePSNR(m_PicInP[0].getArea(), m_PicInP[0].getBitDepth());
      m_MetricData[(int32)eMetric::PSNR].setAnyFake(true);
    }
  }
  m_MetricData[(int32)eMetric::PSNR].setPerCmpMeric(PSNR, FrameIdx);

  if(m_PrintFrame)
  {
    std::string Log = fmt::format("Frame {:08d} ", FrameIdx) + m_MetricData[(int32)eMetric::PSNR].formatPerCmpMetric(FrameIdx);
    if(m_ExactCmps[0]) { Log += " ExactY"; } if(m_ExactCmps[1]) { Log += " ExactU"; } if(m_ExactCmps[2]) { Log += " ExactV"; }
    Log += "\n";
    Log += fmt::format("Frame {:08d} ", FrameIdx) + m_MetricData[(int32)eMetric::PSNR].formatPerPicMetric(FrameIdx);
    Log += "\n";
    fmt::print(Log);
  }
}
void xAppQMIV::calcFrame__WSPSNR(int32 FrameIdx)
{
  flt64V4 WSPSNR = xMakeVec4(0.0  );

  if(m_UseMask) { WSPSNR = m_ProcPSNR.calcPicWSPSNRM(&m_PicInP[0], &m_PicInP[1], &m_PicInP[2], m_NumNonMasked); }
  else          { WSPSNR = m_ProcPSNR.calcPicWSPSNR (&m_PicInP[0], &m_PicInP[1]                              ); }

  for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
  {
    if(m_ExactCmps[CmpIdx])
    {
      WSPSNR[CmpIdx] = m_ProcPSNR.getFakePSNR(m_PicInP[0].getArea(), m_PicInP[0].getBitDepth());
      m_MetricData[(int32)eMetric::WSPSNR].setAnyFake(true);
    }
  }
  m_MetricData[(int32)eMetric::WSPSNR].setPerCmpMeric(WSPSNR, FrameIdx);

  if(m_PrintFrame)
  {
    std::string Log = fmt::format("Frame {:08d} ", FrameIdx) + m_MetricData[(int32)eMetric::WSPSNR].formatPerCmpMetric(FrameIdx);
    if(m_ExactCmps[0]) { Log += " ExactY"; } if(m_ExactCmps[1]) { Log += " ExactU"; } if(m_ExactCmps[2]) { Log += " ExactV"; }
    Log += "\n";
    Log += fmt::format("Frame {:08d} ", FrameIdx) + m_MetricData[(int32)eMetric::WSPSNR].formatPerPicMetric(FrameIdx);
    Log += "\n";
    fmt::print(Log);
  }
}
void xAppQMIV::calcFrame__IVPSNR(int32 FrameIdx)
{
  flt64 IVPSNR = 0.0;
  if(m_UseMask)
  {
    IVPSNR = m_ProcPSNR.calcPicIVPSNRM(&m_PicInI[0], &m_PicInI[1], &m_PicInP[2], m_NumNonMasked, m_GCD_R2T);
  }
  else
  {
    if  (m_InterleavedPic) { IVPSNR = m_ProcPSNR.calcPicIVPSNR(&m_PicInI[0], &m_PicInI[1], m_GCD_R2T); }
    else                   { IVPSNR = m_ProcPSNR.calcPicIVPSNR(&m_PicInP[0], &m_PicInP[1], m_GCD_R2T); }
  }
  m_MetricData[(int32)eMetric::IVPSNR].setPerPicMeric(IVPSNR, FrameIdx);

  if(m_PrintFrame)
  {
    std::string Log = fmt::format("Frame {:08d} ", FrameIdx) + m_MetricData[(int32)eMetric::IVPSNR].formatPerPicMetric(FrameIdx);
    if(m_PrintDebug) { Log += fmt::format("    R2T {:7.4f}  T2R {:7.4f}", m_LastR2T, m_LastT2R); }
    fmt::print(Log + "\n");
  }
}
void xAppQMIV::calcFrame____SSIM(int32 FrameIdx)
{
  flt64V4 SSIM = m_ProcSSIM.calcPicSSIM(&m_PicInP[0], &m_PicInP[1]);
  m_MetricData[(int32)eMetric::SSIM].setPerCmpMeric(SSIM, FrameIdx);

  if(m_PrintFrame)
  { 
    fmt::print("Frame {:08d} {}\n", FrameIdx, m_MetricData[(int32)eMetric::SSIM].formatPerCmpMetric(FrameIdx));
    fmt::print("Frame {:08d} {}\n", FrameIdx, m_MetricData[(int32)eMetric::SSIM].formatPerPicMetric(FrameIdx));
  }
}
void xAppQMIV::calcFrame__IVSSIM(int32 FrameIdx)
{
  flt64 IVSSIM = m_ProcSSIM.calcPicIVSSIM(&m_PicInP[0], &m_PicInP[1], &m_PicSCP[0], &m_PicSCP[1]);
  m_MetricData[(int32)eMetric::IVSSIM].setPerPicMeric(IVSSIM, FrameIdx);

  if(m_PrintFrame)
  {
    std::string Log = fmt::format("Frame {:08d} ", FrameIdx) + m_MetricData[(int32)eMetric::IVSSIM].formatPerPicMetric(FrameIdx);
    if(m_PrintDebug) { Log += fmt::format("    R2T {:7.4f}  T2R {:7.4f}", m_LastR2T, m_LastT2R); }
    fmt::print(Log + "\n");
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

std::string xAppQMIV::calibrateTimeStamp()
{
  tDuration TotalProcTime  = m_ProcEndTime  - m_ProcBegTime ;
  uint64    TotalProcTicks = m_ProcEndTicks - m_ProcBegTicks;

  flt64 TicksPerMicroSec = (flt64)TotalProcTicks / std::chrono::duration_cast<tDurationUS>(TotalProcTime).count();
  flt64 TicksPerMiliSec  = (flt64)TotalProcTicks / std::chrono::duration_cast<tDurationMS>(TotalProcTime).count();
  flt64 TicksPerSec      = (flt64)TotalProcTicks / std::chrono::duration_cast<tDurationS >(TotalProcTime).count();

  std::string Result = fmt::format("CalibratedTicksPerSec = {:.0f} ({:.3f}MHz)\n", TicksPerSec, TicksPerMicroSec);

  m_InvDurationDenominator = (flt64)1.0 / ((flt64)m_NumFrames * TicksPerMiliSec);

  return Result;
}
void xAppQMIV::combineFrameStats()
{
  for(int32 m = 0; m < c_MetricsNum; m++)
  {
    xMetricStat& MD = m_MetricData[m];
    if(MD.getEnabled())
    { 
      MD.calcAvgMetric(m_NumFrames);
      if(m_GatherTime) { MD.calcAvgDuration(m_InvDurationDenominator); }
    }
  }
}
std::string xAppQMIV::formatResultsFile()
{
  std::time_t TimeStamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  std::string Result; Result.reserve(xMemory::c_MemSizePageBase);
  
  Result += fmt::format("FILE0  \"{}\"\n", m_InputFile[0]);
  Result += fmt::format("FILE1  \"{}\"\n", m_InputFile[1]);
  if(m_UseMask) { Result += fmt::format("FILEM  \"{}\"\n", m_InputFile[2]); }
  Result += fmt::format ("TIME   {:%Y-%m-%d  %H:%M:%S}\n", fmt::localtime(TimeStamp));

  for(int32 m = 0; m < c_MetricsNum; m++)
  {
    xMetricStat& MD = m_MetricData[m];
    if(MD.getEnabled()) { Result += MD.formatAvgMetric("") + "\n"; }
  }

  return Result;
}
std::string xAppQMIV::formatResultsStdOut()
{
  std::string Result; Result.reserve(xMemory::c_MemSizePageBase);

  for(int32 m = 0; m < c_MetricsNum; m++)
  {
    xMetricStat& MD = m_MetricData[m];
    if(MD.getEnabled()) { Result += MD.formatAvgMetric("Average      ") + "\n"; }
  }

  if(m_GatherTime)
  {
    tDurationMS AvgDuration____Load = tDurationMS((flt64)m_Ticks____Load * m_InvDurationDenominator);
    tDurationMS AvgDurationValidate = tDurationMS((flt64)m_TicksValidate * m_InvDurationDenominator);
    tDurationMS AvgDuration_Preproc = tDurationMS((flt64)m_Ticks_Preproc * m_InvDurationDenominator);
    tDurationMS AvgDuration_____GCD = tDurationMS((flt64)m_Ticks_____GCD * m_InvDurationDenominator);
    tDurationMS AvgDuration_____SCP = tDurationMS((flt64)m_Ticks_____SCP * m_InvDurationDenominator);

    Result += "\n";
    Result += fmt::format("AvgTime          LOAD {:9.2f} ms\n", AvgDuration____Load.count());
    Result += fmt::format("AvgTime      VALIDATE {:9.2f} ms\n", AvgDurationValidate.count());
    Result += fmt::format("AvgTime       PREPROC {:9.2f} ms\n", AvgDuration_Preproc.count());
    if(m_CalcGCD) { Result += fmt::format("AvgTime           GCD {:9.2f} ms\n", AvgDuration_____GCD.count()); }
    if(m_CalcSCP) { Result += fmt::format("AvgTime           SCP {:9.2f} ms\n", AvgDuration_____SCP.count()); }

    for(int32 m = 0; m < c_MetricsNum; m++)
    {
      xMetricStat& MD = m_MetricData[m];
      if(MD.getEnabled())
      { 
        tDurationMS PreMetricOps = AvgDuration_Preproc;
        switch(MD.getMetric())
        {
          case eMetric::    PSNR: break;
          case eMetric::  WSPSNR: break;
          case eMetric::  IVPSNR: PreMetricOps += AvgDuration_____GCD; break;
          case eMetric::    SSIM: break;
          case eMetric::  IVSSIM: PreMetricOps += AvgDuration_____GCD + AvgDuration_____SCP; break;
          default: break;
        }

        Result += MD.formatAvgTime("AvgTime      ", PreMetricOps) + "\n";
      }
    }
  }
  return Result;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB