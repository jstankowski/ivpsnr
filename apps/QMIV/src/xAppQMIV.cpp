/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xAppQMIV.h"
#include "xProcInfo.h"
#include "xFmtScn.h"
#include "xPixelOps.h"
#include "xColorSpace.h"
#include "xSeqLST.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

const std::string_view xAppQMIV::c_BannerString =
R"PMBBRAWSTRING(
=============================================================================
QMIV software v4.0  [Quality Merics for Immersive Video]

Copyright (c) 2020-2026, Poznan University of Technology, All rights reserved.

Developed at Poznan University of Technology, Poznan, Poland
Authors: Jakub Stankowski, Adrian Dziembowski

The IV-PSNR metric is described in following paper:
A. Dziembowski, D. Mieloch, J. Stankowski and A. Grzelka, "IV-PSNR - The Objective Quality Metric for Immersive Video Applications," in IEEE Transactions on Circuits and Systems for Video Technology, vol. 32, no. 11, pp. 7575-7591, Nov. 2022, doi: 10.1109/TCSVT.2022.3179575.
https://doi.org/10.1109/TCSVT.2022.3179575

The IV-SSIM metric is described in following paper:
A. Dziembowski, W. Nowak, J. Stankowski, "IV-SSIM - The Structural Similarity Metric for Immersive Video", Applied Sciences, Vol. 14, No. 16, Aug 2024, doi: 10.3390/app14167090.
https://doi.org/10.3390/app14167090

=============================================================================

)PMBBRAWSTRING";


const std::string_view xAppQMIV::c_HelpStringHead = 
R"PMBBRAWSTRING(
=============================================================================
QMIV software v4.0
)PMBBRAWSTRING";

const std::string_view xAppQMIV::c_HelpStringGeneral = 
R"PMBBRAWSTRING(
usage::general --------------------------------------------------------------
 -i0   InputFile0         File path - input sequence 0
 -i1   InputFile1         File path - input sequence 1
 -ff   FileFormat         Format of input sequence (optional, default=RAW) [RAW, PNG, BMP]
 -ps   PictureSize        Size of input sequences (WxH)
 -pw   PictureWidth       Width of input sequences 
 -ph   PictureHeight      Height of input sequences
 -pf   PictureFormat      Picture format as defined by FFMPEG pix_fmt i.e. yuv420p10le
 -bd   BitDepth           Bit depth     (optional, default=8, up to 14) 
 -cf   ChromaFormat       Chroma format (optional, default=420) [420, 422, 444]
 -sf0  StartFrame0        Start frame 0 (optional, auto=-1, default=-1) 
 -sf1  StartFrame1        Start frame 1 (optional, auto=-1, default=-1) 
 -nf   NumberOfFrames     Number of frames to be processed (optional, all=-1, default=-1)
 -r    ResultFile         Output file path for printing result(s) (optional)

PictureSize parameter can be used interchangeably with PictureWidth, PictureHeight pair. If PictureSize parameter is present the PictureWidth and PictureHeight arguments are ignored.
PictureFormat parameter can be used interchangeably with BitDepth, ChromaFormat pair. If PictureFormat parameter is present the BitDepth and, ChromaFormat arguments are ignored.
)PMBBRAWSTRING";

const std::string_view xAppQMIV::c_HelpStringMask =
R"PMBBRAWSTRING(
usage::mask_mode ------------------------------------------------------------
 -ifm  InputFileM         File path - mask         (optional, same resolution as InputFile0 and InputFile1)
 -ffm  FileFormat         Format of input sequence (optional, default=RAW) [RAW, PNG, BMP]
 -bdm  BitDepthM          Bit depth for mask       (optional, default=BitDepth, up to 16)
 -cfm  ChromaFormatM      Chroma format for mask   (optional, default=ChromaFormat) [400, 420, 422, 444]
 -sfm  StartFrameM        Start frame M            (optional, default=0)
 -mtm  MaskTransMode      Mask preprocessing mode  (optional) [None, Threshold]
 -mtp  MaskTransParam     Mask preprocessing parameter (optional, default=128) )PMBBRAWSTRING";

const std::string_view xAppQMIV::c_HelpStringERP = 
R"PMBBRAWSTRING(
usage::equirectangular ------------------------------------------------------
 -erp  Equirectangular    Equirectangular input sequence (flag, default disabled)
 -lor  LonRangeDeg        Range for ERP in degrees - Longitudinal (optional, default=360)
 -lar  LatRangeDeg        Range for ERP in degrees - Lateral      (optional, default=180) )PMBBRAWSTRING";

const std::string_view xAppQMIV::c_HelpStringColor = 
R"PMBBRAWSTRING(
usage::colorspace_parameters ------------------------------------------------
 -csi  ColorSpaceInput    Color space of input file             (optional, default=YCbCr)
 -csm  ColorSpaceMetric   Color space used to calculate metrics (optional, default=ColorSpaceInput)
                          If ColorSpaceInput!=ColorSpaceMetric the software performs on-demand conversion
                          (RGB-->YCbCr or YCbCr-->RGB). Conversion requires specific YCbCr color space parameters.
                          [RGB, BGR, GBR, YCbCr, YCbCr_BT601, YCbCr_SMPTE170M, YCbCr_BT709, YCbCr_SMPTE240M, YCbCr_BT2020] )PMBBRAWSTRING";

const std::string_view xAppQMIV::c_HelpStringIV = 
R"PMBBRAWSTRING(
 -cws  CmpWeightsSearch   IV-metric component weights used during search "Lm:Cb:Cr:0" or "R:G:B:0"
                          (per component integer weights, default="4:1:1:0", quotes are mandatory)
 -cwa  CmpWeightsAverage  IV-metric component weights used during averaging "Lm:Cb:Cr:0" or "R:G:B:0"
                          (per component integer weights, default="4:1:1:0", quotes are mandatory) )PMBBRAWSTRING";

const std::string_view xAppQMIV::c_HelpStringExample =
R"PMBBRAWSTRING(-----------------------------------------------------------------------------
Example - commandline parameters:
  QMIV -i0 "A.yuv" -i1 "B.yuv" -ps 2048x1088 -bd 10 -cf 420 -v 3 -r "r.txt"

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

std::string xAppQMIV::formatHelp()
{
  std::string Result = "";
  Result += c_HelpStringHead   ;
  Result += c_HelpStringGeneral;
  Result += c_HelpStringMetrics;
  Result += c_HelpStringMask   ;
  Result += c_HelpStringERP    ;
  Result += c_HelpStringColor  ;
  Result += c_HelpStringCmnIV  ;
  Result += c_HelpStringIV     ;
  Result += c_HelpStringSSIM   ;
  Result += c_HelpStringValOp  ;
  Result += c_HelpStringExample;
  return Result;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xAppQMIV::registerCmdParams()
{
  registerCmdParamsDispatch(); //dispatcher params to be ignored
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
  m_CfgParser.addCmdParm("sf0", "StartFrame0"      , "", "StartFrame0"         );
  m_CfgParser.addCmdParm("sf1", "StartFrame1"      , "", "StartFrame1"         );
  m_CfgParser.addCmdParm("nf" , "NumberOfFrames"   , "", "NumberOfFrames"      );
  m_CfgParser.addCmdParm("r"  , "ResultFile"       , "", "ResultFile"          );
  //metrics
  m_CfgParser.addCmdList("ml" , "MetricList"       , "", "MetricList", ','     );
  //auxliary io
  m_CfgParser.addCmdParm("cp0", "ShftCompPicFile0" , "", "OutputScpFile0"      );
  m_CfgParser.addCmdParm("cp1", "ShftCompPicFile1" , "", "OutputScpFile1"      );
  //mask io
  m_CfgParser.addCmdParm("ifm", "InputFileM"       , "", "InputFileM"          );
  m_CfgParser.addCmdParm("ffm", "FileFormat"       , "", "FileFormatM"         );
  m_CfgParser.addCmdParm("bdm", "BitDepthM"        , "", "BitDepthM"           );
  m_CfgParser.addCmdParm("cfm", "ChromaFormatM"    , "", "ChromaFormatM"       );
  m_CfgParser.addCmdParm("sfm", "StartFrameM"      , "", "StartFrameM"         );
  m_CfgParser.addCmdParm("mtm", "MaskTransMode"    , "", "MaskTransMode"       );
  m_CfgParser.addCmdParm("mtp", "MaskTransParam"   , "", "MaskTransParam"      );
  //erp
  m_CfgParser.addCmdFlag("erp", "Equirectangular"  , "", "Equirectangular", "1");
  m_CfgParser.addCmdParm("lor", "LonRangeDeg"      , "", "LonRangeDeg"         );
  m_CfgParser.addCmdParm("lar", "LatRangeDeg"      , "", "LatRangeDeg"         );
  //colorspace
  m_CfgParser.addCmdParm("csi", "ColorSpaceInput"  , "", "ColorSpaceInput"     );
  m_CfgParser.addCmdParm("csm", "ColorSpaceMetric" , "", "ColorSpaceMetric"    );
  //iv-specific
  registerCmdParamsCmnIV();
  m_CfgParser.addCmdParm("cws", "CmpWeightsSearch" , "", "CmpWeightsSearch"    );
  m_CfgParser.addCmdParm("cwa", "CmpWeightsAverage", "", "CmpWeightsAverage"   );
  //ssim specific
  registerCmdParamsSSIM(); 
  //validation & operation
  registerCmdParamsValOp(); 
}
bool xAppQMIV::readConfiguration()
{
  bool Correct = true;

  //basic io ----------------------------------------------------------------------------------------------------------
  m_InputFile[0] = m_CfgParser.getParam1stArg("InputFile0", std::string(""));
  m_InputFile[1] = m_CfgParser.getParam1stArg("InputFile1", std::string(""));
  if(m_InputFile[0].empty()) { m_ErrorLog += "!  InputFile0 is empty\n"; Correct = false; }
  if(m_InputFile[1].empty()) { m_ErrorLog += "!  InputFile1 is empty\n"; Correct = false; }
  
  m_FileFormat    = m_CfgParser.cvtParam1stArg("FileFormat", eFileFmt::RAW, xStr2FileFmt);
  if(m_FileFormat == eFileFmt::INVALID) { m_ErrorLog += "!  FileFormat is invalid\n"; Correct = false; }
  m_FileFormatRGB = m_FileFormat == eFileFmt::BMP || m_FileFormat == eFileFmt::PNG;
    
  if(m_CfgParser.findParam("PictureSize"))
  {
    std::string PictureSizeS = m_CfgParser.getParam1stArg("PictureSize", std::string(""));
    m_PictureSize = xFmtScn::scanResolution(PictureSizeS);
    if(m_PictureSize[0] <= 0 || m_PictureSize[1] <= 0) { m_ErrorLog += "!  Invalid PictureSize value\n"; Correct = false; }
  }
  else
  {
    int32 PictureWidth  = m_CfgParser.getParam1stArg("PictureWidth" , NOT_VALID);
    int32 PictureHeight = m_CfgParser.getParam1stArg("PictureHeight", NOT_VALID);
    m_PictureSize.set(PictureWidth, PictureHeight);
    if(PictureWidth  <= 0) { m_ErrorLog += "!  Invalid PictureWidth value\n" ; Correct = false; }
    if(PictureHeight <= 0) { m_ErrorLog += "!  Invalid PictureHeight value\n"; Correct = false; }
  }

  if(m_CfgParser.findParam("PictureFormat"))
  {
    std::string PictureFormatS = m_CfgParser.getParam1stArg("PictureFormat", std::string(""));
    eImgTp ImageType;
    std::tie(ImageType, m_ChromaFormat, m_BitDepth) = xFmtScn::scanPixelFormat(PictureFormatS);
    if(ImageType != eImgTp::YCbCr       ) { m_ErrorLog += "!  Invalid or unsuported ImageType value derrived from PictureFormat\n"; Correct = false; }
    if(m_BitDepth < 8 || m_BitDepth > 14) { m_ErrorLog += "!  Invalid or unsuported BitDepth value derrived from PictureFormat\n"; Correct = false; }
    if(m_ChromaFormat == eCrF::INVALID  ) { m_ErrorLog += "!  Invalid or unsuported ChromaFormat value derrived from PictureFormat\n"; Correct = false; }
  }
  else
  {
    m_BitDepth     = m_CfgParser.getParam1stArg("BitDepth"    , 8);
    m_ChromaFormat = m_CfgParser.cvtParam1stArg("ChromaFormat", eCrF::CF420, xStr2CrF);
    if(m_BitDepth < 8 || m_BitDepth > 14) { m_ErrorLog += "!  Invalid or unsuported BitDepth value\n"; Correct = false; }
    if(m_ChromaFormat == eCrF::INVALID  ) { m_ErrorLog += "!  Invalid or unsuported ChromaFormat value\n"; Correct = false; }
  }

  m_StartFrame[0]      = m_CfgParser.getParam1stArg("StartFrame0", NOT_VALID);
  m_StartFrame[1]      = m_CfgParser.getParam1stArg("StartFrame1", NOT_VALID);
  if(m_StartFrame[0] < -1 || m_StartFrame[1] < -1) { m_ErrorLog += "!  StartFrame value cannot be negative\n"; Correct = false; }
  m_NumberOfFrames     = m_CfgParser.getParam1stArg("NumberOfFrames", -1); 

  m_ShftCompPicFile[0] = m_CfgParser.getParam1stArg("ShftCompPicFile0", std::string(""));
  m_ShftCompPicFile[1] = m_CfgParser.getParam1stArg("ShftCompPicFile1", std::string(""));
  m_WriteSCP           = !m_ShftCompPicFile[0].empty() && !m_ShftCompPicFile[1].empty();
  m_ResultFile         = m_CfgParser.getParam1stArg("ResultFile" , std::string(""));

  //metrics -----------------------------------------------------------------------------------------------------------
  Correct = Correct && readConfigurationMetric();

  //mask io -----------------------------------------------------------------------------------------------------------
  m_InputFileM     = m_CfgParser.getParam1stArg("InputFileM"    , std::string("")             );
  m_FileFormatM    = m_CfgParser.cvtParam1stArg("FileFormatM"   , eFileFmt::RAW , xStr2FileFmt);
  m_BitDepthM      = m_CfgParser.getParam1stArg("BitDepthM"     , m_BitDepth                  );
  m_ChromaFormatM  = m_CfgParser.cvtParam1stArg("ChromaFormatM" , m_ChromaFormat, xStr2CrF    );
  m_StartFrameM    = m_CfgParser.getParam1stArg("StartFrameM"   , NOT_VALID                   );
  m_MaskTransMode  = m_CfgParser.cvtParam1stArg("MaskTransMode" , eMskTrMd::None, xStr2MskTrMd);
  m_MaskTransParam = m_CfgParser.getParam1stArg("MaskTransParam", 128                         );

  if(m_FileFormatM == eFileFmt::INVALID  ) { m_ErrorLog += "!  FileFormatM is invalid\n"                   ; Correct = false; }
  if(m_BitDepthM < 8 || m_BitDepthM > 14 ) { m_ErrorLog += "!  Invalid or unsuported BitDepthM value\n"    ; Correct = false; }
  if(m_ChromaFormatM == eCrF::INVALID    ) { m_ErrorLog += "!  Invalid or unsuported ChromaFormatM value\n"; Correct = false; }
  if(m_StartFrameM < -1                  ) { m_ErrorLog += "!  StartFrameM value cannot be negative\n"     ; Correct = false; }
  if(m_MaskTransMode == eMskTrMd::INVALID) { m_ErrorLog += "!  Invalid or unsuported MaskTransMode value\n"; Correct = false; }  
  m_UseMask = !m_InputFileM.empty();

  //erp ---------------------------------------------------------------------------------------------------------------
  m_IsEquirectangular  = m_CfgParser.getParam1stArg("Equirectangular", false);
  m_LonRangeDeg        = m_CfgParser.getParam1stArg("LonRangeDeg"    , 360  );
  m_LatRangeDeg        = m_CfgParser.getParam1stArg("LatRangeDeg"    , 180  );

  //colorspace --------------------------------------------------------------------------------------------------------
  const eClrSpcApp DefaultColorSpace = m_FileFormatRGB ? eClrSpcApp::RGB : eClrSpcApp::YCbCr;
  m_ColorSpaceInput    = m_CfgParser.cvtParam1stArg("ColorSpaceInput" , DefaultColorSpace, xStr2ClrSpcApp);
  m_ColorSpaceMetric   = m_CfgParser.cvtParam1stArg("ColorSpaceMetric", DefaultColorSpace, xStr2ClrSpcApp);
  if(m_ColorSpaceInput != m_ColorSpaceMetric)
  {
    if(isYCbCr(m_ColorSpaceInput) && isYCbCr(m_ColorSpaceMetric)) { m_ErrorLog += fmt::format("!  YCbCr to YCbCr conversion is not supported.\n"); Correct = false; }
    if(m_ColorSpaceInput != m_ColorSpaceMetric && (m_ColorSpaceInput == eClrSpcApp::YCbCr || m_ColorSpaceMetric == eClrSpcApp::YCbCr)) { m_ErrorLog += fmt::format("!  Generic YCbCr cannot be used for colorspace conversion.\n"); Correct = false; }
  }
  m_CvtYCbCr2RGB = isDefinedYCbCr(m_ColorSpaceInput) && isRGB(m_ColorSpaceMetric);
  m_CvtRGB2YCbCr = isRGB(m_ColorSpaceInput) && isDefinedYCbCr(m_ColorSpaceMetric);
  m_ReorderRGB   = isRGB(m_ColorSpaceInput) && m_ColorSpaceInput != eClrSpcApp::RGB && m_ColorSpaceMetric == eClrSpcApp::RGB;
  m_InputRGB     = isRGB(m_ColorSpaceInput);

  //iv-specific -------------------------------------------------------------------------------------------------------
  Correct = Correct && readConfigurationCmnIV(); 
  std::string CmpWeightsSearchS  = m_CfgParser.getParam1stArg("CmpWeightsSearch", xFmtScn::formatIntWeights(xCorrespPixelShiftPrms::c_DefaultCmpWeights));
  std::string CmpWeightsAverageS = m_CfgParser.getParam1stArg("CmpWeightsAverage", xFmtScn::formatIntWeights(xCorrespPixelShiftPrms::c_DefaultCmpWeights));
  m_CmpWeightsSearch  = xFmtScn::scanIntWeights(CmpWeightsSearchS);
  m_CmpWeightsAverage = xFmtScn::scanIntWeights(CmpWeightsAverageS);

  //ssim specific -----------------------------------------------------------------------------------------------------
  Correct = Correct && readConfigurationSSIM();

  //validation & operation --------------------------------------------------------------------------------------------
  Correct = Correct && readConfigurationValOp(); 

  //derrived ----------------------------------------------------------------------------------------------------------  
  derriveConfigurationCmn();

  //post-validation ---------------------------------------------------------------------------------------------------
  if(m_FileFormat == eFileFmt::RAW && m_ChromaFormat == eCrF::CF420 && ((m_PictureSize.getX() & 0x1) || (m_PictureSize.getY() & 0x1)))
  {
    m_ErrorLog += "! Croma format 420 requires PictureWidth and PictureHeight to be even\n"; Correct = false;
  }
  if(m_FileFormat == eFileFmt::RAW && m_ChromaFormat == eCrF::CF422 && (m_PictureSize.getX() & 0x1))
  {
    m_ErrorLog += "! Croma format 422 requires PictureWidth to be even\n"; Correct = false;
  }
  if(m_FileFormat != eFileFmt::RAW && m_ColorSpaceInput != eClrSpcApp::RGB)
  { 
    m_ErrorLog += fmt::format("! Input FileFormat={} contains data in RGB color space whitch conflicts with defined ColorSpaceInput={}\n", xFileFmt2Str(m_FileFormat), xClrSpcApp2Str(m_ColorSpaceInput)); Correct = false;
  }
  if(m_FileFormat != eFileFmt::RAW && m_BitDepth != 8)
  { 
    m_ErrorLog += fmt::format("! Input FileFormat={} contains 8-bit per pixel data whitch conflicts with defined BitDepth={}\n", xFileFmt2Str(m_FileFormat), m_BitDepth); Correct = false;
  }
  Correct = Correct && validateConfigurationCmn();
  
  return Correct;
}
std::string xAppQMIV::formatConfiguration()
{
  std::string Config; Config.reserve(xMemory::getBestEffortSizePageBase());
  //basic io
  Config += "Run-time configuration:\n";
  Config += fmt::format("InputFile0        = {}\n"  , m_InputFile[0]);
  Config += fmt::format("InputFile1        = {}\n"  , m_InputFile[1]);
  Config += fmt::format("FileFormat        = {}\n"  , xFileFmt2Str(m_FileFormat));
  Config += fmt::format("PictureSize       = {}\n"  , xFmtScn::formatResolution(m_PictureSize) );
  Config += fmt::format("BitDepth          = {}\n"  , m_BitDepth);
  Config += fmt::format("ChromaFormat      = {}{}\n", xCrF2Str(m_ChromaFormat), m_InputRGB ? "  (irrelevant)" : "");
  Config += fmt::format("StartFrame0       = {}{}\n", m_StartFrame[0], m_StartFrame[0]==NOT_VALID ? "  (auto)" : "");
  Config += fmt::format("StartFrame1       = {}{}\n", m_StartFrame[1], m_StartFrame[1]==NOT_VALID ? "  (auto)" : "");
  Config += fmt::format("NumberOfFrames    = {}{}\n", m_NumberOfFrames, m_NumberOfFrames==NOT_VALID ? "  (all)" : "");
  Config += fmt::format("ShftCompPicFile0  = {}\n"  , m_ShftCompPicFile[0].empty() ? "(unused)" : m_ShftCompPicFile[0]);
  Config += fmt::format("ShftCompPicFile1  = {}\n"  , m_ShftCompPicFile[1].empty() ? "(unused)" : m_ShftCompPicFile[1]);
  Config += fmt::format("ResultFile        = {}\n"  , m_ResultFile.empty() ? "(unused)" : m_ResultFile);
  //metrics
  formatConfigurationMetrics(Config);
  //mask io
  Config += fmt::format("InputFileM        = {}\n", m_InputFileM.empty() ? "(unused)" : m_InputFileM);
  Config += fmt::format("FileFormatM       = {}()\n", xFileFmt2Str(m_FileFormatM), m_UseMask ? "" : "  (irrelevant)");
  Config += fmt::format("BitDepthM         = {}{}\n", m_BitDepthM                , m_UseMask ? "" : "  (irrelevant)");
  Config += fmt::format("ChromaFormatM     = {}{}\n", xCrF2Str(m_ChromaFormatM)  , m_UseMask ? "" : "  (irrelevant)");
  Config += fmt::format("StartFrameM       = {}{}\n", m_StartFrameM, m_StartFrameM == NOT_VALID ? "  (auto)" : "");
  Config += fmt::format("MaskTransMode     = {}{}\n", xMskTrMd2Str(m_MaskTransMode), m_UseMask ? "" : "  (irrelevant)");
  Config += fmt::format("MaskTransParam    = {}{}\n", m_MaskTransParam             , m_UseMask ? "" : "  (irrelevant)");
  //erp
  Config += fmt::format("Equirectangular   = {:d}\n", m_IsEquirectangular);
  Config += fmt::format("LonRangeDeg       = {}{}\n", m_LonRangeDeg, m_IsEquirectangular ? "" : "  (irrelevant)");
  Config += fmt::format("LatRangeDeg       = {}{}\n", m_LatRangeDeg, m_IsEquirectangular ? "" : "  (irrelevant)");
  //colorspace
  Config += fmt::format("ColorSpaceInput   = {}{}\n", xClrSpcApp2Str(m_ColorSpaceInput ), m_CvtRGB2YCbCr || m_CvtYCbCr2RGB || m_ReorderRGB ? "" : "  (irrelevant)");
  Config += fmt::format("ColorSpaceMetric  = {}{}\n", xClrSpcApp2Str(m_ColorSpaceMetric), m_CvtRGB2YCbCr || m_CvtYCbCr2RGB || m_ReorderRGB ? "" : "  (irrelevant)");
  //iv-specific
  formatConfigurationCmnIV(Config);
  Config += fmt::format("CmpWeightsSearch  = {}{}\n", xFmtScn::formatIntWeights(m_CmpWeightsSearch ), m_CmpWeightsSearch  == xCorrespPixelShiftPrms::c_DefaultCmpWeights ? "  (default)" : "  (custom)");
  Config += fmt::format("CmpWeightsAverage = {}{}\n", xFmtScn::formatIntWeights(m_CmpWeightsAverage), m_CmpWeightsAverage == xCorrespPixelShiftPrms::c_DefaultCmpWeights ? "  (default)" : "  (custom)");
  //ssim specific
  formatConfigurationSSIM (Config); 
  //validation & operation
  formatConfigurationValOp(Config); 
  Config += "\n";
  //derrived
  Config += fmt::format("Run-time derrived parameters:\n");
  Config += fmt::format("WindowSize        = {}x{}\n", m_WindowSize, m_WindowSize);
  Config += fmt::format("PictureMargin     = {}\n", m_PicMargin);
  Config += fmt::format("UseMask           = {:d}\n", m_UseMask);
  Config += "\n";
  //metric description
  formatMetricDescriptions(Config);

  return Config;
}
eAppRes xAppQMIV::validateInputFiles()
{
  QMAU_TRACE(2, "");
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
      const auto [ValidM, MessageM] = xFileNameScn::validateFileParams(m_InputFileM, m_PictureSize, m_BitDepthM, m_ChromaFormatM);
      if(!ValidM) { m_ErrorLog += MessageM; AnyError = true; }
    }
  }

  if(AnyError) { return m_NameMismatchActn == eActn::STOP ? eAppRes::Error : eAppRes::Warning; }
  return eAppRes::Good;
}
std::string xAppQMIV::formatWarnings()
{ 
  QMAU_TRACE(2, "");
  std::string Warnings = "";

  if(m_CmpWeightsSearch != xCorrespPixelShiftPrms::c_DefaultCmpWeights)
  {
    Warnings += fmt::format("CONFORMANCE WARNING: Software was executed with CmpWeightsSearch different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for Immersive Video. The default weights are DefaultCmpWeights={}.\n\n", xFmtScn::formatIntWeights(xCorrespPixelShiftPrms::c_DefaultCmpWeights));
  }
  if(m_CmpWeightsAverage != xCorrespPixelShiftPrms::c_DefaultCmpWeights)
  {
    Warnings += fmt::format("CONFORMANCE WARNING: Software was executed with CmpWeightsAverage different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for Immersive Video. The default weights are DefaultCmpWeights={}.\n\n", xFmtScn::formatIntWeights(xCorrespPixelShiftPrms::c_DefaultCmpWeights));
  }

  Warnings += formatWarningsCmn();

  return Warnings;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int32 xAppQMIV::determinePreferredNumThreads()
{
  int32 PreferedNumberOfThreads = 8;
  if(m_CalcSSIMs && xSSIM::isRegularMode(m_StructSimMode) && m_StructSimStride < 4)
  {
    PreferedNumberOfThreads = m_HardwareConcurency;
  }
  if((m_CalcSSIMs || m_CalcIVs) && (int64)m_PictureSize.getX() * (int64)m_PictureSize.getX() >= 4096 * 4096) 
  {
    PreferedNumberOfThreads = m_HardwareConcurency;
  }
  return PreferedNumberOfThreads;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

eAppRes xAppQMIV::setupSeqs()
{
  QMAU_TRACE(2, "");
    
  //check if file exists
  for(int32 i = 0; i < c_NumPics; i++)
  {
    bool Exists = checkIfFileExists(m_FileFormat, m_InputFile[i], m_StartFrame[i]);
    if(!Exists) { xErrMsg::printError(fmt::format("ERROR --> InputFile{} does not exist ({})", i, m_InputFile[i])); return eAppRes::Error; }
  }
  if(m_UseMask)
  {
    bool Exists = checkIfFileExists(m_FileFormatM, m_InputFileM, m_StartFrameM);
    if(!Exists) { xErrMsg::printError(fmt::format("ERROR --> InputFile{} does not exist ({})", "M", m_InputFileM)); return eAppRes::Error; }
  }

  //file size
  if(m_FileFormat == eFileFmt::RAW              && m_VerboseLevel >= 1) { for(int32 i=0; i<c_NumPics; i++) { fmt::print("SizeOfInputFile{}    = {}\n", i, xFile::size(m_InputFile[i])); } }
  if(m_UseMask && m_FileFormat == eFileFmt::RAW && m_VerboseLevel >= 1) { fmt::print("SizeOfInputFile{}    = {}\n", "M", xFile::size(m_InputFileM)); }  

  //create input sequences 
  for(int32 i = 0; i < c_NumPics; i++)
  {
    m_SeqPicIn[i] = createSeqByFileFmt(m_FileFormat, m_PictureSize, m_BitDepth, m_ChromaFormat);
    if(m_SeqPicIn[i] == nullptr) { xErrMsg::printError(fmt::format("ERROR --> unsupported Pic FileFormat ({})", xFileFmt2Str(m_FileFormat))); return eAppRes::Error; }
  }
  if(m_UseMask)
  {
    m_SeqMskIn = createSeqByFileFmt(m_FileFormatM, m_PictureSize, m_BitDepthM, m_ChromaFormatM);
    if(m_SeqMskIn == nullptr) { xErrMsg::printError(fmt::format("ERROR --> unsupported Mask FileFormat ({})", xFileFmt2Str(m_FileFormatM))); return eAppRes::Error; }
  }

  //open input sequences 
  for(int32 i = 0; i < c_NumPics; i++)
  {
    xSeqPic::tResult Result = m_SeqPicIn[i]->openFile(m_InputFile[i], xSeq::eMode::Read);
    if(!Result) { xErrMsg::printError(fmt::format("ERROR --> InputFile opening failure ({}) {}", m_InputFile[i], Result.format())); return eAppRes::Error; }
  }
  if(m_UseMask)
  {
    xSeqPic::tResult Result = m_SeqMskIn->openFile(m_InputFileM, xSeq::eMode::Read);
    if(!Result) { xErrMsg::printError(fmt::format("ERROR --> InputFile opening failure ({}) {}", m_InputFileM, Result.format())); return eAppRes::Error; }
  }

  //no camera list so 1 view
  m_NumViews = 1;

  //first frames & num of frames per input file
  int32 DetectedNumOfFrames[c_NumPics] = { 0 };
  int32 DetectedFirstFrame [c_NumPics] = { 0 };
  for(int32 i = 0; i < c_NumPics; i++)
  {
    DetectedNumOfFrames[i] = m_SeqPicIn[i]->getNumOfFrames ();
    DetectedFirstFrame [i] = m_SeqPicIn[i]->getCurrFrameIdx();
    if(m_VerboseLevel >= 1) { fmt::print("DetectedNumFrames{}  = {}\n", i, DetectedNumOfFrames[i]); }
    if(m_VerboseLevel >= 1) { fmt::print("DetectedFirstFrame{} = {}\n", i, DetectedFirstFrame [i]); }
    if(m_StartFrame[i] == NOT_VALID) { m_StartFrame[i] = DetectedFirstFrame[i]; } //auto first frame mode
    if(m_StartFrame[i] < DetectedFirstFrame[i]) { xErrMsg::printError(fmt::format("ERROR --> StartFrame{} < DetectedFirstFrame{} for ({})", i, i, m_InputFile[i])); return eAppRes::Error; }
    if(m_StartFrame[i] >= DetectedNumOfFrames[i] + DetectedFirstFrame[i]) { xErrMsg::printError(fmt::format("ERROR --> StartFrame{} >= DetectedNumOfFrames{} + DetectedFirstFrame{} for ({})", i, i, i, m_InputFile[i])); return eAppRes::Error; }
  }
  int32 DetectedNumOfFramesM = std::numeric_limits<int32>::max();
  int32 DetectedFirstFrameM  = 0;
  if(m_UseMask)
  {
    DetectedNumOfFramesM = m_SeqMskIn->getNumOfFrames ();
    DetectedFirstFrameM  = m_SeqMskIn->getCurrFrameIdx();
    if(m_VerboseLevel >= 1) { fmt::print("DetectedNumFrames{}  = {}\n", "M", DetectedNumOfFramesM); }
    if(m_VerboseLevel >= 1) { fmt::print("DetectedFirstFrame{} = {}\n", "M", DetectedFirstFrameM ); }
    if(m_StartFrameM == NOT_VALID) { m_StartFrameM = DetectedFirstFrameM; } //auto first frame mode
    if(m_StartFrameM < DetectedFirstFrameM) { xErrMsg::printError(fmt::format("ERROR --> StartFrame{} < DetectedFirstFrame{} for ({})", "M", "M", m_InputFileM)); return eAppRes::Error; }
    if(m_StartFrameM >= DetectedNumOfFramesM + DetectedFirstFrameM) { xErrMsg::printError(fmt::format("ERROR --> StartFrame{} >= DetectedNumOfFrames{} + DetectedFirstFrame{} for ({})", "M", "M", "M", m_InputFileM)); return eAppRes::Error; }
  }

  //num of frames to process
  int32 MinSeqNumFrames = xMin(DetectedNumOfFrames[0], DetectedNumOfFrames[1]);
  int32 MinSeqRemFrames = xMin(DetectedNumOfFrames[0] + DetectedFirstFrame[0] - m_StartFrame[0], DetectedNumOfFrames[1] + DetectedFirstFrame[1] - m_StartFrame[1]);
  if(m_UseMask)
  {
    MinSeqNumFrames = xMin(MinSeqNumFrames, DetectedNumOfFramesM                );
    MinSeqRemFrames = xMin(MinSeqRemFrames, DetectedNumOfFramesM + DetectedFirstFrameM - m_StartFrameM);
  }
  if(m_NumberOfFrames > 0 && m_NumberOfFrames > MinSeqRemFrames) { { xErrMsg::printError(fmt::format("ERROR --> requested NumberOfFrames ({}) > MinSeqRemFrames ({})", m_NumberOfFrames, MinSeqRemFrames)); return eAppRes::Error; } }
  m_NumFrames = xMin(m_NumberOfFrames > 0 ? m_NumberOfFrames : MinSeqNumFrames, MinSeqRemFrames);
  if(m_VerboseLevel >= 1) { fmt::print("FramesToProcess     = {}\n", m_NumFrames); }
  fmt::print("\n");
 
  //seek sequences
  for(int32 i = 0; i < c_NumPics; i++)
  { 
    if(m_StartFrame[i] != DetectedFirstFrame[i])
    { 
      xSeqPic::tResult Result = m_SeqPicIn[i]->seekFrame(m_StartFrame[i]);
      if(!Result) { xErrMsg::printError(fmt::format("ERROR --> InputFile{} seeking failure ({}) {}", i, m_InputFile[i], Result.format())); return eAppRes::Error; }
    }
  }
  if(m_UseMask && m_StartFrameM != DetectedFirstFrameM)
  {
    xSeqPic::tResult Result = m_SeqMskIn->seekFrame(m_StartFrameM);
    if(!Result) { xErrMsg::printError(fmt::format("ERROR --> InputFile{} seeking failure ({}) {}", "M", m_InputFileM, Result.format())); return eAppRes::Error; }
  }

  //scp sequences
  if(m_WriteSCP)
  { 
    for(int32 i = 0; i < c_NumPics; i++)
    {
      m_SeqPicSCP[i].create(m_PictureSize, m_BitDepth, eCrF::CF444);
      bool OpenSucces = (bool)(m_SeqPicSCP[i].openFile(m_ShftCompPicFile[i], xSeq::eMode::Write));
      if(!OpenSucces) { xErrMsg::printError(fmt::format("ERROR --> OutputFile opening failure ({})", m_ShftCompPicFile[i])); return eAppRes::Error; }
    }
  }

  return eAppRes::Good;
}
eAppRes xAppQMIV::ceaseSeqs()
{
  QMAU_TRACE(2, "");
  //input sequences
  for(int32 i = 0; i < c_NumPics; i++) { m_SeqPicIn[i]->closeFile(); }
  for(int32 i = 0; i < c_NumPics; i++) { m_SeqPicIn[i]->destroy(); m_SeqPicIn[i] = nullptr; }
  //scp sequences
  if(m_WriteSCP)
  {
    for(int32 i = 0; i < c_NumPics; i++) { m_SeqPicSCP[i].closeFile(); }
    for(int32 i = 0; i < c_NumPics; i++) { m_SeqPicSCP[i].destroy  (); }
  }
  //mask sequences
  if(m_UseMask)
  {
    m_SeqMskIn->closeFile(); m_SeqMskIn->destroy(); m_SeqMskIn = nullptr;
  }
  return eAppRes::Good;
}
eAppRes xAppQMIV::setupBuffs()
{
  QMAU_TRACE(2, "");
  //input buffers
  for(int32 i = 0; i < c_NumPics; i++) { m_PicInP[i].create(m_PictureSize, m_BitDepth, m_PicMargin); }
  //SCP buffers
  if(m_CalcSCP) { for(int32 i = 0; i < c_NumPics; i++) { m_PicSCP[i].create(m_PictureSize, m_BitDepth, m_PicMargin); } }
  //output buffers
  if(m_WriteSCP) { for(int32 i = 0; i < c_NumPics; i++) { m_PicOutP[i].create(m_PictureSize, m_BitDepth, m_PicMargin); } }
  //mask buffer
  if(m_UseMask) { m_PicMskInP.create(m_PictureSize, m_BitDepthM, m_PicMargin); }

  return eAppRes::Good;
}
eAppRes xAppQMIV::ceaseBuffs()
{
  QMAU_TRACE(2, "");
  //input buffers
  for(int32 i = 0; i < c_NumPics; i++) { m_PicInP[i].destroy(); }
  //SCP buffers
  if(m_CalcSCP)
  {
    for(int32 i = 0; i < c_NumPics; i++) { m_PicSCP[i].destroy(); }
  }
  //output buffers
  if(m_WriteSCP) { for(int32 i = 0; i < c_NumPics; i++) { m_PicOutP[i].destroy(); } }
  //mask buffer
  if(m_UseMask) { m_PicMskInP.destroy(); }

  return eAppRes::Good;
}
void xAppQMIV::initMetricStorage()
{
  QMAU_TRACE(2, "");

  for(int32 m = 0; m < c_MetricsNum; m++)
  {
    if(m_CalcMetric[m]) 
    {
      m_MetricData[m].initMetric  ((eMetric)m, m_NumFrames, 1);
      m_MetricData[m].initSuffixes(m_UseMask);
      m_MetricData[m].initCmpWeightsAverage(m_CmpWeightsAverage, eClrSpcMtr::YCbCr);
      m_MetricData[m].initCmpWeightsAverage(m_CmpWeightsAverage, eClrSpcMtr::RGB  );
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

eAppRes xAppQMIV::processAllFrames()
{
  QMAU_TRACE(2, "");
  const eClrSpcMtr ClrSpcMtr = xClrSpcAppToClrSpcMtr(m_ColorSpaceMetric);
  m_TimeStamp.sampleBeg();

  for(int32 f = 0; f < m_NumFrames; f++)
  {
    eAppRes LoadRes       = loadFrames    (f); if(LoadRes       != eAppRes::Good) { return LoadRes      ; }    
    eAppRes ValidationRes = validateFrames(f); if(ValidationRes != eAppRes::Good) { return ValidationRes; }
    preprocessFrames(f);
    preprocessMask  (f);
    calcMetricsCmn  (f, 0, ClrSpcMtr);
  } //end of loop over frames

  m_TimeStamp.sampleEnd();
  return eAppRes::Good;
}

eAppRes xAppQMIV::loadFrames(int32 f)
{
  QMAU_TRACE(3, "loadFrames");
  uint64 T = m_GatherTime ? xTSC() : 0;
  xSeqPic::tResult ReadResult[c_NumPics] = { eRetv::Success, eRetv::Success };
  xSeqPic::tResult ReadResultM = eRetv::Success;

  for(int32 i = 0; i < c_NumPics; i++) { m_TPI.storeTask([this, &ReadResult, i](int32 /*ThId*/) { ReadResult[i] = m_SeqPicIn[i]->readFrame(&(m_PicInP[i])); }); }
  if(m_UseMask) { m_TPI.storeTask([this, &ReadResultM](int32 /*ThId*/) { ReadResultM = m_SeqMskIn->readFrame(&(m_PicMskInP)); }); }
  m_TPI.executeStoredTasks();

  static constexpr std::string_view ErrorForm = "Frame{:08d} ERROR --> InputFile{} read error ({}) for file frame idx {} --- {}";
  for(int32 i = 0; i < c_NumPics; i++) { if(!ReadResult[i]) { xErrMsg::printError(fmt::format(ErrorForm, f, i  , m_InputFile[i], m_SeqPicIn[i]->getCurrFrameIdx(), ReadResult[i].format())); return eAppRes::Error; } }
  if(m_UseMask)                        { if(!ReadResultM  ) { xErrMsg::printError(fmt::format(ErrorForm, f, "M", m_InputFileM  , m_SeqMskIn   ->getCurrFrameIdx(), ReadResultM  .format())); return eAppRes::Error; } }

  if(m_GatherTime) { m_Ticks____Load += (xTSC() - T); }
  return eAppRes::Good;
}
eAppRes xAppQMIV::validateFrames(int32 f)
{
  QMAU_TRACE(3, "");
  if(m_InvalidPelActn == eActn::SKIP) { return eAppRes::Good; }
  uint64 T = m_GatherTime ? xTSC() : 0;
  bool CheckPassed[c_NumPics] = { true, true };
  bool CheckPassedM = true;
  for(int32 i = 0; i < c_NumPics; i++) { m_TPI.storeTask([this, &CheckPassed, i](int32) { CheckPassed[i] = m_PicInP[i].check(m_InputFile[i]); }); };
  if(m_UseMask                       ) { m_TPI.storeTask([this, &CheckPassedM  ](int32) { CheckPassedM   = m_PicMskInP.check(m_InputFileM  ); }); };
  m_TPI.executeStoredTasks();

  if(m_InvalidPelActn == eActn::CNCL)
  {
    for(int32 i = 0; i < c_NumPics; i++) { if(!CheckPassed[i]) { m_PicInP[i].conceal(); } }
    if(m_UseMask                       ) { if(!CheckPassedM  ) { m_PicMskInP.conceal(); } }
  }

  if(m_InvalidPelActn==eActn::STOP)
  {
    for(int32 i = 0; i < c_NumPics; i++) { if(!CheckPassed[i]) { xErrMsg::printError(fmt::format("Frame {:08d} ERROR --> InputFile contains invalid values ({})", f, m_InputFile[i])); return eAppRes::Error; } }
    if(m_UseMask                       ) { if(!CheckPassedM  ) { xErrMsg::printError(fmt::format("Frame {:08d} ERROR --> InputFile contains invalid values ({})", f, m_InputFileM  )); return eAppRes::Error; } }
  }

  if(m_GatherTime) { m_TicksValidate += (xTSC() - T); }
  return eAppRes::Good;
}
void xAppQMIV::preprocessFrames(int32 /**/)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  if(m_CvtYCbCr2RGB)
  {
    eClrSpcLC ColorSpace = xClrSpcAppToClrSpc(m_ColorSpaceInput);
    for(int32 i = 0; i < c_NumPics; i++) { m_TPI.storeTask([this, i, ColorSpace](int32) { xColorSpace::ConvertYCbCr2RGB(
      m_PicInP[i].getAddr(eCmp::R ), m_PicInP[i].getAddr(eCmp::G ), m_PicInP[i].getAddr(eCmp::B ),
      m_PicInP[i].getAddr(eCmp::LM), m_PicInP[i].getAddr(eCmp::CB), m_PicInP[i].getAddr(eCmp::CR),
      m_PicInP[i].getStride(), m_PicInP[i].getStride(), m_PicInP[i].getWidth(), m_PicInP[i].getHeight(), m_PicInP[i].getBitDepth(), ColorSpace);
    } ); }
    m_TPI.executeStoredTasks();
  }

  if(m_CvtRGB2YCbCr)
  {
    eClrSpcLC ColorSpace = xClrSpcAppToClrSpc(m_ColorSpaceMetric);
    for(int32 i = 0; i < c_NumPics; i++) { m_TPI.storeTask([this, i, ColorSpace](int32) { xColorSpace::ConvertRGB2YCbCr(
      m_PicInP[i].getAddr(eCmp::LM), m_PicInP[i].getAddr(eCmp::CB), m_PicInP[i].getAddr(eCmp::CR),
      m_PicInP[i].getAddr(eCmp::R ), m_PicInP[i].getAddr(eCmp::G ), m_PicInP[i].getAddr(eCmp::B ),      
      m_PicInP[i].getStride(), m_PicInP[i].getStride(), m_PicInP[i].getWidth(), m_PicInP[i].getHeight(), m_PicInP[i].getBitDepth(), ColorSpace);
    } ); }
    m_TPI.executeStoredTasks();
  }

  if(m_ReorderRGB)
  {
    for(int32 i = 0; i < c_NumPics; i++)
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

  for(int32 CmpIdx = 0; CmpIdx < m_PicInP[0].getNumCmps(); CmpIdx++)
  {
    m_ExactCmps[CmpIdx] = m_PicInP[0].equalCmp(&m_PicInP[1], (eCmp)CmpIdx);
  }

  for(int32 i = 0; i < c_NumPics; i++) { m_TPI.storeTask([this, i](int32) { m_PicInP[i].extend(); }); }
  m_TPI.executeStoredTasks();

  if(m_GatherTime) { m_TicksPreprocF += (xTSC() - T); }
}
void xAppQMIV::preprocessMask(int32 FrameIdx)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;

  if(m_UseMask) { preprocessMaskCmn(FrameIdx); }

  if(m_GatherTime) { m_TicksPreprocM += (xTSC() - T); }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xAppQMIV::combineFrameStats()
{
  const flt64 InvDurationDenominatorF = (flt64)1.0 / ((flt64)m_NumFrames * m_TimeStamp.getTicksPerMiliSec());

  for(int32 m = 0; m < c_MetricsNum; m++)
  {
    xMetricStat& MD = m_MetricData[m];
    if(MD.getEnabled())
    {
      MD.calcAvgMetric(m_NumFrames, 1);
      if(m_GatherTime) { MD.calcAvgDuration(InvDurationDenominatorF); }
    }
  }
}
std::string xAppQMIV::formatResultsFile()
{
  QMAU_TRACE(2, "");
  const eClrSpcMtr ClrSpcMtr = xClrSpcAppToClrSpcMtr(m_ColorSpaceMetric);

  std::time_t TimeStamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  std::string Result; Result.reserve(xMemory::getBestEffortSizePageBase());
  
  Result += fmt::format("FILE0  \"{}\"\n", m_InputFile[0]);
  Result += fmt::format("FILE1  \"{}\"\n", m_InputFile[1]);
  if(m_UseMask) { Result += fmt::format("FILEM  \"{}\"\n", m_InputFileM); }
  Result += fmt::format ("TIME   {:%Y-%m-%d  %H:%M:%S}\n", *std::localtime(&TimeStamp));

  for(int32 m = 0; m < c_MetricsNum; m++)
  {
    xMetricStat& MD = m_MetricData[m];
    if(MD.getEnabled()) { Result += MD.formatAvgMetric("", ClrSpcMtr) + "\n"; }
  }

  return Result;
}
std::string xAppQMIV::formatResultsStdOut()
{
  QMAU_TRACE(2, "");
  const eClrSpcMtr ClrSpcMtr = xClrSpcAppToClrSpcMtr(m_ColorSpaceMetric);

  std::string Result; Result.reserve(xMemory::getBestEffortSizePageBase());

  for(int32 m = 0; m < c_MetricsNum; m++)
  {
    xMetricStat& MD = m_MetricData[m];
    if(MD.getEnabled()) { Result += MD.formatAvgMetric("Average      ", ClrSpcMtr) + "\n"; }
  }

  if(m_GatherTime)
  {
    const flt64 InvDurationDenominatorF = (flt64)1.0 / ((flt64)m_NumFrames * m_TimeStamp.getTicksPerMiliSec());
    tDurationMS AvgDuration____Load = tDurationMS((flt64)m_Ticks____Load * InvDurationDenominatorF);
    tDurationMS AvgDurationValidate = tDurationMS((flt64)m_TicksValidate * InvDurationDenominatorF);
    Result += "\n";
    Result += fmt::format("AvgTime          LOAD {:9.2f} ms\n", AvgDuration____Load.count());
    Result += fmt::format("AvgTime      VALIDATE {:9.2f} ms\n", AvgDurationValidate.count());
    Result += formatMetricTimeStats(InvDurationDenominatorF);
  }

  return Result;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB