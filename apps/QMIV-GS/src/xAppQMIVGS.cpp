/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xAppQMIVGS.h"
#include "xProcInfo.h"
#include "xFmtScn.h"
#include "xPixelOps.h"
#include "xColorSpace.h"
#include "xSeqLST.h"
#include "xPly.h"
#include "xFileListUtils.h"
#include "xPicHash.h"
#include "xUtilsAppGS.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

const std::string_view xAppQMIVGS::c_BannerString =
R"PMBBRAWSTRING(
=============================================================================
QMIV-GS software v4.0-rc1  [Quality Merics for Immersive Video - Gaussian Splatting]

Copyright (c) 2020-2026, Poznan University of Technology, All rights reserved.

Developed at Poznan University of Technology, Poznan, Poland
Authors: Jakub Stankowski, Błażej Szydełko, Adrian Dziembowski

=============================================================================

)PMBBRAWSTRING";

const std::string_view xAppQMIVGS::c_HelpStringHead =
R"PMBBRAWSTRING(
=============================================================================
QMIV-GS software v4.0-rc1
)PMBBRAWSTRING";

const std::string_view xAppQMIVGS::c_HelpStringGeneral =
R"PMBBRAWSTRING(
usage::general --------------------------------------------------------------
 -i0   InputFile0         File path - test gaussian splat      (fmtlib pattern with {FrameIdx} token, e.g. "cloudT_{FrameIdx:04d}.ply")
 -i1   InputFile1         File path - reference gaussian splat (fmtlib pattern with {FrameIdx} token, e.g. "cloudR_{FrameIdx:04d}.ply")
 -ps   PictureSize        Resolution of rendered image (WxH)
 -bd   BitDepth           Bit depth of rendered image (optional, default=8, up to 14)
 -sf0  StartFrame0        Start frame 0  (optional, auto=-1, default=-1) 
 -sf1  StartFrame1        Start frame 1  (optional, auto=-1, default=-1) 
 -nf   NumberOfFrames     Number of frames to process (optional, -1=all, default=-1)
 -r    ResultFile         Output file for metric results (optional)

 -s0   SynthFile0         Output YUV base name for rendered pictures from InputFile0 (optional)
 -s1   SynthFile1         Output YUV base name for rendered pictures from InputFile1 (optional)
                          Filename pattern has to contain {W}, {H}, and {ViewIdx} e.g. Render_{W}x{H}_V{ViewIdx:d}_Bbps.yuv
                          Both or files or none at all have to be provided.
 -sfv  SynthFilePerView   Use different files for each target view (optional, default 1)
 -ffs  FileFormatS        File format of synth output (optional, default=RAW) [RAW, PNG, BMP]
 -css  ColorSpaceFileS    Color space of synth output (optional, default=YCbCr)
                          [RGB, BGR, GBR, YCbCr_BT601, YCbCr_SMPTE170M, YCbCr_BT709, YCbCr_SMPTE240M, YCbCr_BT2020]
 -cfs  ChromaFormatFileS  Chroma format for render output (optional, default=420) [400, 420, 422, 444]
 -ptf  PoseTraceFile      Pose trace (optional), one camera pose per frame, the metrics are calculated per pose. Requires CamParamsFile.
                          If longer than the cloud sequence, cloud frames are mirrored back and forth (ping-pong). Formats:
                          1. CSV without a header, one pose per row: X,Y,Z,Yaw,Pitch,Roll (angles in degrees)
                          2. CSV with a header (column names in any order), one pose per row:
                               x,y,z + yaw,pitch,roll (degrees) or qw,qx,qy,qz (quaternion), optionally + fx,fy (focal length in pixels) or zoom (focal length multiplier)
                          3. Viewpoints: first line = number of poses, then one pose per line, 11, 13 or 15 values separated by spaces:
                               11 values: PosX PosY PosZ  TargetX TargetY TargetZ  UpX UpY UpZ  Fx Fy
                               13 values: the 11 above + Width Height
                               15 values: the 11 above + Cx Cy Width Height
                               Pos = camera position, Target = point the camera looks at, Up = up vector, Fx Fy = focal length, Cx Cy = principal point (default: picture center),
                               in pixels of a Width x Height picture (default: the rendered picture)
)PMBBRAWSTRING";

const std::string_view xAppQMIVGS::c_HelpStringMask =
R"PMBBRAWSTRING(
usage::mask_mode ------------------------------------------------------------
 -smm  SynthMaskMode     Use mask derived during synthesis process - based on alpha (default None) [None, Occupancy, Alpha]
 -ofm  OutputFileM       File path - mask (optional, 8 bit 4:2:0 YUV format only, same resolution as PictureSize) )PMBBRAWSTRING";

const std::string_view xAppQMIVGS::c_HelpStringIV =
R"PMBBRAWSTRING(
       CmpWeightsSearchRGB    IV-metric component weights used during search "R:G:B:0"
                              (per component integer weights, default="1:1:1:0", quotes are mandatory)
       CmpWeightsSearchYCbCr  IV-metric component weights used during search "Lm:Cb:Cr:0"
                              (per component integer weights, default="4:1:1:0", quotes are mandatory)
       CmpWeightsAverageRGB   IV-metric component weights used during averaging "R:G:B:0"
                              (per component integer weights, default="1:1:1:0", quotes are mandatory)
       CmpWeightsAverageYCbCr IV-metric component weights used during averaging "Lm:Cb:Cr:0"
                              (per component integer weights, default="4:1:1:0", quotes are mandatory) )PMBBRAWSTRING";

const std::string_view xAppQMIVGS::c_HelpStringExample =
R"PMBBRAWSTRING(-----------------------------------------------------------------------------
Example:
  QMIV-GS -i0 "src_{FrameIdx:04d}.ply" -i1 "dec_{FrameIdx:04d}.ply" -ps 1920x1080 -v 3 -r "r.txt"

=============================================================================
)PMBBRAWSTRING";

//===============================================================================================================================================================================================================

std::string xAppQMIVGS::formatHelp()
{
  std::string Result = "";
  Result += c_HelpStringHead   ;
  Result += c_HelpStringGeneral;
  Result += c_HelpStringMetrics;
  Result += c_HelpStringMask   ;
  Result += c_HelpStringCmnIV  ;
  Result += c_HelpStringIV     ;
  Result += c_HelpStringSSIM   ;
  Result += c_HelpStringValOp  ;
  Result += c_HelpStringExample;
  return Result;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xAppQMIVGS::registerCmdParams()
{
  registerCmdParamsDispatch(); //dispatcher params to be ignored
  //basic io
  m_CfgParser.addCmdParm("i0" , "InputFile0"       , "", "InputFile0"          );
  m_CfgParser.addCmdParm("i1" , "InputFile1"       , "", "InputFile1"          );
  m_CfgParser.addCmdParm("cpf", "CamParamsFile"    , "", "CamParamsFile"       );
  m_CfgParser.addCmdParm("cpm", "CamParamsFormat"  , "", "CamParamsFormat"     );
  m_CfgParser.addCmdParm("cpt", "CamNameTemplate"  , "", "CamNameTemplate"     );
  m_CfgParser.addCmdParm("ptf", "PoseTraceFile"    , "", "PoseTraceFile"       );
  m_CfgParser.addCmdParm("ps" , "PictureSize"      , "", "PictureSize"         );
  m_CfgParser.addCmdParm("bd" , "BitDepth"         , "", "BitDepth"            );
  m_CfgParser.addCmdParm("sf0", "StartFrame0"      , "", "StartFrame0"         );
  m_CfgParser.addCmdParm("sf1", "StartFrame1"      , "", "StartFrame1"         );
  m_CfgParser.addCmdParm("nf" , "NumberOfFrames"   , "", "NumberOfFrames"      );
  m_CfgParser.addCmdParm("r"  , "ResultFile"       , "", "ResultFile"          );
  m_CfgParser.addCmdParm("s0" , "SynthFile0"       , "", "SynthFile0"          );
  m_CfgParser.addCmdParm("s1" , "SynthFile1"       , "", "SynthFile1"          );
  m_CfgParser.addCmdParm("sfv", "SynthFilePerView" , "", "SynthFilePerView"    );
  m_CfgParser.addCmdParm("ffs", "FileFormatS"      , "", "FileFormatS"         );
  m_CfgParser.addCmdParm("css", "ColorSpaceFileS"  , "", "ColorSpaceFileS"     );
  m_CfgParser.addCmdParm("cfs", "ChromaFormatFileS", "", "ChromaFormatFileS"   );
  //metrics
  m_CfgParser.addCmdList("ml" , "MetricList"       , "", "MetricList", ','     );
  //auxiliary io
  m_CfgParser.addCmdParm("cf0", "ShftCompPicFile0" , "", "OutputScpFile0"      );
  m_CfgParser.addCmdParm("cf1", "ShftCompPicFile1" , "", "OutputScpFile1"      );
  //mask io
  m_CfgParser.addCmdParm("smm", "SynthMaskMode"    , "", "SynthMaskMode"       );
  m_CfgParser.addCmdParm("ofm", "OutputFileM"      , "", "InputFileM"          );
  //iv-specific
  registerCmdParamsCmnIV();
  //ssim specific
  registerCmdParamsSSIM();
  //validation & operation
  registerCmdParamsValOp();
}
bool xAppQMIVGS::readConfiguration()
{
  bool Correct = true;

  //basic io ----------------------------------------------------------------------------------------------------------
  m_InputFile[0] = m_CfgParser.getParam1stArg("InputFile0", std::string(""));
  m_InputFile[1] = m_CfgParser.getParam1stArg("InputFile1", std::string(""));
  if(m_InputFile[0].empty()) { m_ErrorLog += "!  InputFile0 is empty\n"; Correct = false; }
  if(m_InputFile[1].empty()) { m_ErrorLog += "!  InputFile1 is empty\n"; Correct = false; }

  m_CamParamsFile   = m_CfgParser.getParam1stArg("CamParamsFile", std::string(""));
  m_CamParamsFormat = m_CfgParser.cvtParam1stArg("CamParamsFormat", FVGS::eCamParamsFormat::PLY, FVGS::xStrToCamParamsFormat);
  m_CamNameTemplate = m_CfgParser.getParam1stArg("CamNameTemplate", std::string("[CamIdStr]"));
  if(m_CamParamsFormat == FVGS::eCamParamsFormat::INVALID) { m_ErrorLog += "!  Invalid CamParamsFormat value\n"; Correct = false; }
  m_PoseTraceFile   = m_CfgParser.getParam1stArg("PoseTraceFile", std::string(""));
  m_CalcPoseTrace   = !m_PoseTraceFile.empty();

  std::string PictureSizeS = m_CfgParser.getParam1stArg("PictureSize", std::string(""));
  m_PictureSize = xFmtScn::scanResolution(PictureSizeS);
  if(m_PictureSize[0] <= 0 || m_PictureSize[1] <= 0) { m_ErrorLog += "!  Invalid PictureSize value\n"; Correct = false; }
  m_BitDepth          = m_CfgParser.getParam1stArg("BitDepth"        , 8 );

  m_StartFrame[0]     = m_CfgParser.getParam1stArg("StartFrame0", NOT_VALID);
  m_StartFrame[1]     = m_CfgParser.getParam1stArg("StartFrame1", NOT_VALID);
  if(m_StartFrame[0] < -1 || m_StartFrame[1] < -1) { m_ErrorLog += "!  StartFrame value cannot be negative\n"; Correct = false; }
  m_NumberOfFrames    = m_CfgParser.getParam1stArg("NumberOfFrames"  , -1);

  m_ResultFile        = m_CfgParser.getParam1stArg("ResultFile"      , std::string(""));
  m_SynthFile[0]      = m_CfgParser.getParam1stArg("SynthFile0"      , std::string(""));
  m_SynthFile[1]      = m_CfgParser.getParam1stArg("SynthFile1"      , std::string(""));
  m_WriteSynth        = !m_SynthFile[0].empty() && !m_SynthFile[1].empty();
  m_SynthFilePerView  = m_CfgParser.getParam1stArg("SynthFilePerView", 1 );
  m_FileFormatS       = m_CfgParser.cvtParam1stArg("FileFormatS"     , eFileFmt::RAW          , xStr2FileFmt  );
  m_ColorSpaceFileS   = m_CfgParser.cvtParam1stArg("ColorSpaceFileS" , eClrSpcApp::YCbCr_BT709, xStr2ClrSpcApp);
  m_ChromaFormatFileS = m_CfgParser.cvtParam1stArg("ChromaFormatFileS", eCrF::CF420            , xStr2CrF      );
  m_WriteSynthYCbCr   = isDefinedYCbCr(m_ColorSpaceFileS);
  m_ReorderWriteRGB   = isRGB(m_ColorSpaceFileS) && m_ColorSpaceFileS != eClrSpcApp::RGB;

  //metrics -----------------------------------------------------------------------------------------------------------
  Correct = Correct && readConfigurationMetric();

  //mask io -----------------------------------------------------------------------------------------------------------
  m_SynthMaskMode = m_CfgParser.cvtParam1stArg("SynthMaskMode", eSynMskMd::None, xStr2SynMskMd);
  m_OutputFileM   = m_CfgParser.getParam1stArg("OutputFileM", std::string(""));
  switch(m_SynthMaskMode)
  {
  case eSynMskMd::None:
    m_BitDepthM      = NOT_VALID;
    m_MaskTransMode  = eMskTrMd::None;
    m_MaskTransParam = NOT_VALID;
    m_UseMask        = false;
    break;
  case eSynMskMd::Occupancy:
    m_BitDepthM      = 8;
    m_MaskTransMode  = eMskTrMd::Threshold;
    m_MaskTransParam = 51; //0.2 * 255;
    m_UseMask        = true;
    break;
  case eSynMskMd::Alpha:
    m_BitDepthM      = 8;
    m_MaskTransMode  = eMskTrMd::None;
    m_MaskTransParam = NOT_VALID;
    m_UseMask        = true;
    break;
  }

  //iv-specific -------------------------------------------------------------------------------------------------------
  Correct = Correct && readConfigurationCmnIV();
  std::string CmpWeightsSearchRGB_S    = m_CfgParser.getParam1stArg("CmpWeightsSearchRGB"   , xFmtScn::formatIntWeights(xCorrespPixelShiftPrms::c_EqualCmpWeights  ));
  std::string CmpWeightsSearchYCbCr_S  = m_CfgParser.getParam1stArg("CmpWeightsSearchYCbCr" , xFmtScn::formatIntWeights(xCorrespPixelShiftPrms::c_DefaultCmpWeights));
  std::string CmpWeightsAverageRGB_S   = m_CfgParser.getParam1stArg("CmpWeightsAverageRGB"  , xFmtScn::formatIntWeights(xCorrespPixelShiftPrms::c_EqualCmpWeights  ));
  std::string CmpWeightsAverageYCbCr_S = m_CfgParser.getParam1stArg("CmpWeightsAverageYCbCr", xFmtScn::formatIntWeights(xCorrespPixelShiftPrms::c_DefaultCmpWeights));
  m_CmpWeightsSearchRGB    = xFmtScn::scanIntWeights(CmpWeightsSearchRGB_S   );
  m_CmpWeightsSearchYCbCr  = xFmtScn::scanIntWeights(CmpWeightsSearchYCbCr_S );
  m_CmpWeightsAverageRGB   = xFmtScn::scanIntWeights(CmpWeightsAverageRGB_S  );
  m_CmpWeightsAverageYCbCr = xFmtScn::scanIntWeights(CmpWeightsAverageYCbCr_S);

  //ssim specific -----------------------------------------------------------------------------------------------------
  Correct = Correct && readConfigurationSSIM();

  //validation & operation --------------------------------------------------------------------------------------------
  Correct = Correct && readConfigurationValOp();

  //derived -----------------------------------------------------------------------------------------------------------
  derriveConfigurationCmn();

  //post-validation ---------------------------------------------------------------------------------------------------
  if(m_BitDepth < 8 || m_BitDepth > 14) 
  {
    m_ErrorLog += "! BitDepth has to be in range [8,14]\n"; Correct = false;
  }
  if(m_ColorSpaceFileS == eClrSpcApp::INVALID)
  {
    m_ErrorLog += "! Invalid ColorSpaceFileS value\n"; Correct = false;
  }
  if(m_WriteSynth && m_ChromaFormatFileS == eCrF::CF420 && ((m_PictureSize.getX() & 0x1) || (m_PictureSize.getY() & 0x1)))
  {
    m_ErrorLog += "! Chroma format 420 requires PictureWidth and PictureHeight to be even\n"; Correct = false;
  }
  if(m_WriteSynth && m_ChromaFormatFileS == eCrF::CF422 && (m_PictureSize.getX() & 0x1))
  {
    m_ErrorLog += "! Chroma format 422 requires PictureWidth to be even\n"; Correct = false;
  }
  if(m_UseMask && m_CalcSSIMs)
  {
    m_ErrorLog += "! Structural Similarity metrics cannot be combined with Mask mode\n"; Correct = false;
  }
  if(m_UseMask && !xSSIM::isRegularMode(m_StructSimMode))
  {
    m_ErrorLog += "! Mask mode requires regular SSIM mode\n"; Correct = false;
  }
  if(m_UseMask && m_StructSimStride != 1)
  {
    m_ErrorLog += "! Mask mode requires StructSimStride=1\n"; Correct = false;
  }
  if(m_UseMask && m_CalcMSs)
  {
    m_ErrorLog += "! MS-SSIM and IV-MS-SSIM does not support mask mode\n"; Correct = false;
  }

  return Correct;
}
std::string xAppQMIVGS::formatConfiguration()
{
  std::string Config; Config.reserve(xMemory::getBestEffortSizePageBase());
  Config += "Run-time configuration:\n";
  Config += fmt::format("InputFile0        = {}\n"  , m_InputFile[0]);
  Config += fmt::format("InputFile1        = {}\n"  , m_InputFile[1]);
  Config += fmt::format("CamParamsFile     = {}\n"  , m_CamParamsFile);
  Config += fmt::format("CamParamsFormat   = {}\n"  , xCamParamsFormatToStr(m_CamParamsFormat));
  Config += fmt::format("CamNameTemplate   = {}\n"  , m_CamNameTemplate);
  Config += fmt::format("PoseTraceFile     = {}\n"  , m_PoseTraceFile.empty() ? "(unused)" : m_PoseTraceFile);
  Config += fmt::format("PictureSize       = {}\n"  , xFmtScn::formatResolution(m_PictureSize) );
  Config += fmt::format("BitDepth          = {}\n"  , m_BitDepth);
  Config += fmt::format("StartFrame0       = {}\n"  , m_StartFrame[0]    );
  Config += fmt::format("StartFrame1       = {}\n"  , m_StartFrame[1]    );
  Config += fmt::format("NumberOfFrames    = {}{}\n", m_NumberOfFrames, m_NumberOfFrames==NOT_VALID ? "  (all)" : "");
  Config += fmt::format("ResultFile        = {}\n"  , m_ResultFile.empty() ? "(unused)" : m_ResultFile);
  Config += fmt::format("SynthFile0        = {}\n"  , m_SynthFile[0]);
  Config += fmt::format("SynthFile1        = {}\n"  , m_SynthFile[1]);
  Config += fmt::format("SynthFilePerView  = {}\n"  , m_SynthFilePerView);
  Config += fmt::format("FileFormatS       = {}\n"  , xFileFmt2Str(m_FileFormatS));
  Config += fmt::format("ColorSpaceFileS   = {}\n"  , xClrSpcApp2Str(m_ColorSpaceFileS));
  Config += fmt::format("ChromaFormatFileS = {}{}\n", xCrF2Str(m_ChromaFormatFileS), !m_WriteSynthYCbCr ? "  (irrelevant)" : "");
  Config += fmt::format("ColorSpaceMetric  = RGB+{}\n", xClrSpcApp2Str(m_ColorSpaceMetric));
  //metrics
  formatConfigurationMetrics(Config);
  //mask io
  Config += fmt::format("SynthMaskMode     = {}\n"  , xSynMskMd2Str(m_SynthMaskMode));
  Config += fmt::format("OutputFileM       = {}{}\n", m_OutputFileM.empty() ? "(unused)" : m_OutputFileM, m_UseMask ? "" : "  (irrelevant)");
  Config += fmt::format("BitDepthM         = {}{}\n", m_BitDepthM, m_UseMask ? "" : "  (irrelevant)");
  Config += fmt::format("MaskTransMode     = {}{}\n", xMskTrMd2Str(m_MaskTransMode), m_UseMask ? "" : "  (irrelevant)");
  Config += fmt::format("MaskTransParam    = {}{}\n", m_MaskTransParam, m_UseMask ? "" : "  (irrelevant)");
  //iv-specific
  formatConfigurationCmnIV(Config);
  Config += fmt::format("CmpWeightsSearchRGB    = {}{}\n", xFmtScn::formatIntWeights(m_CmpWeightsSearchRGB   ), m_CmpWeightsSearchRGB    == xCorrespPixelShiftPrms::c_EqualCmpWeights   ? "  (default)" : "  (custom)");
  Config += fmt::format("CmpWeightsSearchYCbCr  = {}{}\n", xFmtScn::formatIntWeights(m_CmpWeightsSearchYCbCr ), m_CmpWeightsSearchYCbCr  == xCorrespPixelShiftPrms::c_DefaultCmpWeights ? "  (default)" : "  (custom)");
  Config += fmt::format("CmpWeightsAverageRGB   = {}{}\n", xFmtScn::formatIntWeights(m_CmpWeightsAverageRGB  ), m_CmpWeightsAverageRGB   == xCorrespPixelShiftPrms::c_EqualCmpWeights   ? "  (default)" : "  (custom)");
  Config += fmt::format("CmpWeightsAverageYCbCr = {}{}\n", xFmtScn::formatIntWeights(m_CmpWeightsAverageYCbCr), m_CmpWeightsAverageYCbCr == xCorrespPixelShiftPrms::c_DefaultCmpWeights ? "  (default)" : "  (custom)");
  //ssim specific
  formatConfigurationSSIM(Config);
  //validation & operation
  formatConfigurationValOp(Config);
  Config += "\n";
  //derived
  Config += fmt::format("Run-time derived parameters:\n");
  Config += fmt::format("WindowSize        = {}x{}\n", m_WindowSize, m_WindowSize);
  Config += fmt::format("PictureMargin     = {}\n", m_PicMargin);
  Config += fmt::format("UseMask           = {:d}\n", m_UseMask);
  Config += "\n";
  //metric description
  formatMetricDescriptions(Config);

  return Config;
}
eAppRes xAppQMIVGS::validateInputFiles()
{
  QMAU_TRACE(2, "");
  return eAppRes::Good;
}
std::string xAppQMIVGS::formatWarnings()
{ 
  std::string Warnings = "";

  Warnings += formatWarningsCmn();

#if defined(X_PMBB_ARCH_AMD64) && !(X_SIMD_CAN_USE_AVX || X_SIMD_HAS_AVX512)
  Warnings += "REPRODUCIBILITY WARNING: The Gaussian splating renderer uses FMA operations. The older (pre-AVX2) x86-64 architectures does not support FMA instructions. In current version of the software, rendering results can differ (it will be fixed in future releases).\n\n";
#endif

  return Warnings;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int32 xAppQMIVGS::determineMaxNumTasks()
{
  QMAU_TRACE(3, "");
  int32 NumTiles    = GSR::calcNumTiles(m_PictureSize);
  int32 MaxNumTasks = xMax(4096, m_PictureSize.getY(), NumTiles);
  return MaxNumTasks;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

eAppRes xAppQMIVGS::setupSeqs()
{
  QMAU_TRACE(2, "");

  //check if file exists
  for(int32 i = 0; i < c_NumPics; i++)
  {
    bool Exists = xFileListUtils::checkIfFileExists(m_InputFile[i], m_StartFrame[i]);
    if(!Exists) { xErrMsg::printError(fmt::format("ERROR --> InputFile{} does not exist ({})", i, m_InputFile[i])); return eAppRes::Error; }
  }

  //create input sequences 
  for(int32 i = 0; i < c_NumPics; i++) { m_SeqPlyIn[i].create(false, m_StartFrame[i], xSeqImgList::c_DefaultMaxNumFiles); }

  //open input sequences 
  for(int32 i = 0; i < c_NumPics; i++)
  {
    xSeqPic::tResult Result = m_SeqPlyIn[i].openFile(m_InputFile[i], xSeq::eMode::Read);
    if(!Result) { xErrMsg::printError(fmt::format("ERROR --> InputFile opening failure ({}) {}", m_InputFile[i], Result.format())); return eAppRes::Error; }
  }

  //load camera parameters
  if(!m_CamManager.loadRealCams(m_CamParamsFormat, m_CamParamsFile, m_CamNameTemplate, m_PictureSize))
  {
    xErrMsg::printError(fmt::format("ERROR --> failed to load {} camera parameters from CamParamsFile ({})", xCamParamsFormatToStr(m_CamParamsFormat), m_CamParamsFile));
    return eAppRes::Error;
  }
  m_NumViews = m_CamManager.getNumRealCams();
  if(m_VerboseLevel >= 1) { fmt::print("NumViews            = {}\n", m_NumViews); }

  if(m_CalcPoseTrace)
  {
    if(m_CamManager.getNumRealCams() == 0) { xErrMsg::printError("ERROR --> PoseTraceFile requires at least one camera intrinsics loaded via CamParamsFile"); return eAppRes::Error; }

    bool Success = m_CamManager.loadPoseTraceParams(FVGS::xCameraParamsManager<flt64>::c_VirtGroupIdx, m_PoseTraceFile, m_PictureSize);
    if(!Success) { xErrMsg::printError(fmt::format("ERROR --> failed to load PoseTraceFile ({})", m_PoseTraceFile)); return eAppRes::Error; }
    m_NumPoseTraceFrames = m_CamManager.getNumVirtCams();
    if(m_VerboseLevel >= 1) { fmt::print("NumPoseTraceFrames  = {}\n", m_NumPoseTraceFrames); }
  }

  //first frames & num of frames per input file
  int32 DetectedNumOfFrames[c_NumPics] = { 0 };
  int32 DetectedFirstFrame [c_NumPics] = { 0 };
  for(int32 i = 0; i < c_NumPics; i++)
  {
    DetectedNumOfFrames[i] = m_SeqPlyIn[i].getNumOfFrames ();
    DetectedFirstFrame [i] = m_SeqPlyIn[i].getCurrFrameIdx();
    if(m_VerboseLevel >= 1) { fmt::print("DetectedNumFrames{}  = {}\n", i, DetectedNumOfFrames[i]); }
    if(m_VerboseLevel >= 1) { fmt::print("DetectedFirstFrame{} = {}\n", i, DetectedFirstFrame [i]); }
    if(m_StartFrame[i] == NOT_VALID) { m_StartFrame[i] = DetectedFirstFrame[i]; } //auto first frame mode
    if(m_StartFrame[i] < DetectedFirstFrame[i]) { xErrMsg::printError(fmt::format("ERROR --> StartFrame{} < DetectedFirstFrame{} for ({})", i, i, m_InputFile[i])); return eAppRes::Error; }
    if(m_StartFrame[i] >= DetectedNumOfFrames[i] + DetectedFirstFrame[i]) { xErrMsg::printError(fmt::format("ERROR --> StartFrame{} >= DetectedNumOfFrames{} + DetectedFirstFrame{} for ({})", i, i, i, m_InputFile[i])); return eAppRes::Error; }
  }
  //num of frames to process
  int32 MinSeqNumFrames = xMin(DetectedNumOfFrames[0], DetectedNumOfFrames[1]);
  int32 MinSeqRemFrames = xMin(DetectedNumOfFrames[0] + DetectedFirstFrame[0] - m_StartFrame[0], DetectedNumOfFrames[1] + DetectedFirstFrame[1] - m_StartFrame[1]);
  if(m_NumberOfFrames > 0 && m_NumberOfFrames > MinSeqRemFrames) { { xErrMsg::printError(fmt::format("ERROR --> requested NumberOfFrames ({}) > MinSeqRemFrames ({})", m_NumberOfFrames, MinSeqRemFrames)); return eAppRes::Error; } }
  m_NumFrames           = xMin(m_NumberOfFrames > 0 ? m_NumberOfFrames : MinSeqNumFrames, MinSeqRemFrames);
  if(m_VerboseLevel >= 1) { fmt::print("FramesToProcess     = {}\n", m_NumFrames); }
 
  //seek sequences
  for(int32 i = 0; i < c_NumPics; i++)
  { 
    if(m_StartFrame[i] != DetectedFirstFrame[i])
    { 
      xSeqPic::tResult Result = m_SeqPlyIn[i].seekFrame(m_StartFrame[i]);
      if(!Result) { xErrMsg::printError(fmt::format("ERROR --> InputFile{} seeking failure ({}) {}", i, m_InputFile[i], Result.format())); return eAppRes::Error; }
    }
  }

  m_NumRenders = m_CalcPoseTrace ? m_NumPoseTraceFrames : m_NumFrames * m_NumViews;
  if(m_VerboseLevel >= 1) { fmt::print("NumRenders          = {}\n", m_NumRenders); }
  fmt::print("\n\n");

  // rendered sequences
  if(m_WriteSynth)
  {
    const int32 NumViewsToOpen = (m_CalcPoseTrace || !m_SynthFilePerView) ? 1 : m_NumViews;
    for(int32 i = 0; i < c_NumPics; i++)
    {
      m_SeqPicSyn[i].resize(NumViewsToOpen);
      for(int32 v = 0; v < NumViewsToOpen; v++)
      {
        m_SeqPicSyn[i][v] = createSeqByFileFmt(m_FileFormatS, m_PictureSize, m_BitDepth, m_WriteSynthYCbCr ? m_ChromaFormatFileS : eCrF::CF444);
        if(m_SeqPicSyn[i][v] == nullptr) { xErrMsg::printError(fmt::format("ERROR --> unsupported Pic FileFormat ({})", xFileFmt2Str(m_FileFormatS))); return eAppRes::Error; }


        std::string FileName = xFileListUtils::formatNamedPattern(m_SynthFile[i], fmt::arg("W", m_PictureSize.getX()), fmt::arg("H", m_PictureSize.getY()), fmt::arg("ViewIdx", v));
        if(FileName.empty()) { xErrMsg::printError(fmt::format("ERROR --> SynthFile formating failure ({})", m_SynthFile[i])); return eAppRes::Error; }
        bool OpenSuccess = (bool)(m_SeqPicSyn[i][v]->openFile(FileName, xSeq::eMode::Write));
        if(!OpenSuccess) { xErrMsg::printError(fmt::format("ERROR --> SynthFile opening failure ({})", FileName)); return eAppRes::Error; }
        if(m_VerboseLevel >= 1) { fmt::print("SynthFile_I{}_V{} = {}\n", i, v, FileName); }
      }
    }
    if(m_VerboseLevel >= 1) { fmt::print("\n\n"); }
  }

  return eAppRes::Good;
}
eAppRes xAppQMIVGS::ceaseSeqs()
{
  QMAU_TRACE(2, "");

  //input sequences
  for(int32 i = 0; i < c_NumPics; i++)
  {
    m_SeqPlyIn[i].closeFile();
    m_SeqPlyIn[i].destroy  ();
  }
  //rendered sequences
  for(int32 i = 0; i < c_NumPics; i++)
  {
    for(xSeqPic* Seq : m_SeqPicSyn[i]) { Seq->closeFile(); Seq->destroy(); }
    m_SeqPicSyn[i].clear();
  }
  //scp sequences
  if(m_WriteSCP)
  {
    for(int32 i = 0; i < c_NumPics; i++) { m_SeqPicSCP[i].closeFile(); }
    for(int32 i = 0; i < c_NumPics; i++) { m_SeqPicSCP[i].destroy  (); }
  }
  return eAppRes::Good;
}
eAppRes xAppQMIVGS::setupBuffs()
{
  QMAU_TRACE(2, "");
  //input buffers
  for(int32 i = 0; i < c_NumPics; i++) { m_PicInP[i].create(m_PictureSize, m_BitDepth, m_PicMargin); }
  //SCP buffers
  if(m_CalcSCP) { for(int32 i = 0; i < c_NumPics; i++) { m_PicSCP[i].create(m_PictureSize, m_BitDepth, m_PicMargin); } }
  //output buffers
  if(m_WriteSynth)
  { 
    for(int32 i = 0; i < c_NumPics; i++) { m_PicOutP[i].create(m_PictureSize, m_BitDepth, m_PicMargin); }
    if(m_WriteSynthYCbCr || m_ReorderWriteRGB) { m_PicTmpP.create(m_PictureSize, m_BitDepth, m_PicMargin); }
  }
  //mask buffer
  if(m_UseMask)
  { 
    for(int32 i = 0; i < c_NumPics; i++) { m_PlnMskSyn[i] = new GSR::tPicM(m_PictureSize, m_BitDepthM, m_PicMargin); }
    m_PicMskInP.create(m_PictureSize, m_BitDepthM, m_PicMargin);
  }

  return eAppRes::Good;
}
eAppRes xAppQMIVGS::ceaseBuffs()
{
  QMAU_TRACE(2, "");
  //input buffers
  for(int32 i = 0; i < c_NumPics; i++) { m_PicInP[i].destroy(); }
  //SCP buffers
  if(m_CalcSCP) { for(int32 i = 0; i < c_NumPics; i++) { m_PicSCP[i].destroy(); } }
  //output buffers
  if(m_WriteSynth)
  { 
    for(int32 i = 0; i < c_NumPics; i++) { m_PicOutP[i].destroy(); }
    if(m_WriteSynthYCbCr || m_ReorderWriteRGB) { m_PicTmpP.destroy(); }
  }
  //mask buffer
  if(m_UseMask) 
  { 
    for(int32 i = 0; i < c_NumPics; i++) { m_PlnMskSyn[i]->destroy(); m_PlnMskSyn[i] = nullptr; }
    m_PicMskInP.destroy();
  }

  return eAppRes::Good;
}
void xAppQMIVGS::initMetricStorage()
{
  QMAU_TRACE(2, "");

  int32 NumFramesPerMetric = m_CalcPoseTrace ? m_NumPoseTraceFrames : m_NumFrames;
  int32 NumViewsPerMetric  = m_CalcPoseTrace ? 1                    : m_NumViews ;

  for(int32 m = 0; m < c_MetricsNum; m++)
  {
    if(m_CalcMetric[m]) 
    {
      m_MetricData[m].initMetric  ((eMetric)m, NumFramesPerMetric, NumViewsPerMetric);
      m_MetricData[m].initSuffixes(m_UseMask);
      m_MetricData[m].initCmpWeightsAverage(m_CmpWeightsAverageYCbCr, eClrSpcMtr::YCbCr);
      m_MetricData[m].initCmpWeightsAverage(m_CmpWeightsAverageRGB  , eClrSpcMtr::RGB  );
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xAppQMIVGS::createRendererProcessor()
{
  QMAU_TRACE(2, "");
  m_Renderer = new GSR::xRenderer();
  m_Renderer->create(m_PictureSize, m_ThreadPool);
  m_Renderer->setGatherTimeStats(m_GatherTime);
  if(m_GatherHash) { for(int32 i = 0; i < c_NumPics; i++) { m_HashCloud[i].init(); m_HashSynth[i].init(); } }
}
void xAppQMIVGS::destroyRendererProcessor()
{
  QMAU_TRACE(2, "");
  m_Renderer->destroy(); delete m_Renderer; m_Renderer = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

eAppRes xAppQMIVGS::processAllFrames()
{
  QMAU_TRACE(2, "");  
  m_TimeStamp.sampleBeg();

  for(int32 f = 0; f < m_NumFrames; f++)
  {
    eAppRes LoadRes = loadClouds(f); if(LoadRes != eAppRes::Good) { return LoadRes; }
    if(m_GatherHash) { calcHashClouds(f); }

    for(int32 v = 0; v < m_NumViews; v++)
    {
      synthesizePics(f, v);
      if(m_GatherHash) { calcHashSynths(f, v); }
      if(m_WriteSynth) { eAppRes WriteRes = writeSynths(f, v); if(WriteRes != eAppRes::Good) { return WriteRes; } }
      calcMetrics(f, v);
    } //end of loop over views
  } //end of loop over frames

  m_TimeStamp.sampleEnd();
  return eAppRes::Good;
}
eAppRes xAppQMIVGS::processPoseTraces()
{
  QMAU_TRACE(2, "");
  m_TimeStamp.sampleBeg();

  //cache all cloud frames
  for(int32 i = 0; i < c_NumPics; i++) { m_CloudCache[i].create(m_NumFrames); }

  uint64 LoadT = m_GatherTime ? xTSC() : 0;

  for(int32 i = 0; i < c_NumPics; i++)
  {
    xSeqPic::tResult SeekResult = m_SeqPlyIn[i].seekFrame(m_StartFrame[i]);
    if(!SeekResult) { xErrMsg::printError(fmt::format("ERROR --> PoseTrace cloud cache seek error ({})", xFileListUtils::formatNamedPattern(m_InputFile[i], m_StartFrame[i], fmt::arg("FrameIdx", m_StartFrame[i])))); return eAppRes::Error; }
  }

  xSeqPic::tResult ReadResult[c_NumPics] = { eRetv::Success, eRetv::Success };
  for(int32 i = 0; i < c_NumPics; i++)
  {
    m_TPI.storeTask([this, &ReadResult, i](int32 /*ThId*/) { ReadResult[i] = m_CloudCache[i].load(&m_SeqPlyIn[i], m_InputFile[i], m_StartFrame[i], m_PrintDebug); }); 
  }
  m_TPI.executeStoredTasks();
  if(ReadResult[0] != eRetv::Success || ReadResult[1] != eRetv::Success) { return eAppRes::Error; }
  if(m_GatherTime) { m_Ticks____Load += (xTSC() - LoadT); }
  if(m_PrintDebug) { fmt::print("\n"); }

  if(m_GatherHash)
  {
    uint64 HashT = m_GatherTime ? xTSC() : 0;
    for(int32 i = 0; i < c_NumPics; i++) { m_CloudCache[i].hash(m_HashCloud[i]); }
    if(m_GatherTime) { m_Ticks_HashCld += (xTSC() - HashT); }
  }

  const auto& VirtCamIdx = m_CamManager.getVirtCamIdx();

  if(m_VerboseLevel >= 1) { fmt::print("\nProcessing:\n"); }

  for(int32 p = 0; p < m_NumPoseTraceFrames; p++)
  {
    const int32 CloudFrameIdx = xMirrorFrameIdx(p, m_NumFrames);
    const FVGS::xGaussianCloudI* Clouds[c_NumPics] = { m_CloudCache[0].getCloud(CloudFrameIdx), m_CloudCache[1].getCloud(CloudFrameIdx) };

    uint64 SynthT = m_GatherTime ? xTSC() : 0;

    //TODO use synthesizePics here
    const GSR::tCamPar& Cam = m_CamManager.getCamParams(VirtCamIdx[p]);
    for(int32 i = 0; i < c_NumPics; i++)
    {
      if(m_DebugDump) { m_Renderer->preRenderDetailedStats(p, NOT_VALID, i); }
      m_Renderer->renderFrame(&m_PicInP[i], m_PlnMskSyn[i], Clouds[i], Cam);
      if(m_DebugDump) { m_Renderer->postRenderDetailedStats(); }
    }

    if(m_GatherTime) { m_Ticks___Synth += (xTSC() - SynthT); }

    if(m_GatherHash)
    {
      uint64 HashT = m_GatherTime ? xTSC() : 0;
      xXXH128::tHash ImmHash[c_NumPics];
      for(int32 i = 0; i < c_NumPics; i++) { m_TPI.storeTask([this, &ImmHash, i](int32 /*ThId*/) { ImmHash[i] = xPicHash::CalcPicHash(m_PicInP[i]); }); }
      m_TPI.executeStoredTasks();
      if(m_PrintDebug) { fmt::print("PoseTrace {} SynthHash {} {}\n", xFormatIdx(p, NOT_VALID), xXXH128::format(ImmHash[0]), xXXH128::format(ImmHash[1])); }
      for(int32 i = 0; i < c_NumPics; i++) { m_HashSynth[i].update(ImmHash[i].data(), ImmHash[i].size()); }
      if(m_GatherTime) { m_Ticks_HashSyn += (xTSC() - HashT); }
    }

    if(m_WriteSynth) { eAppRes WriteRes = writeSynths(p, 0); if(WriteRes != eAppRes::Good) { return WriteRes; } }
    calcMetrics(p, 0);
  }

  for(int32 i = 0; i < c_NumPics; i++) { m_CloudCache[i].destroy(); }

  m_TimeStamp.sampleEnd();
  return eAppRes::Good;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

eAppRes xAppQMIVGS::loadClouds(int32 f)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;

  xSeqPic::tResult ReadResult[c_NumPics] = { eRetv::Success, eRetv::Success };

  for(int32 i = 0; i < c_NumPics; i++) { m_TPI.storeTask([this, &ReadResult, i](int32 /*ThId*/) { ReadResult[i] = m_SeqPlyIn[i].readGC(&(m_Cloud[i])); }); }
  m_TPI.executeStoredTasks();

  for(int32 i = 0; i < c_NumPics; i++) { if(!ReadResult[i]) { xErrMsg::printError(fmt::format("Frame {:08d} ERROR --> InputFile read error ({}) {}", f, xFileListUtils::formatNamedPattern(m_InputFile[i], m_StartFrame[i] + f, fmt::arg("FrameIdx", m_StartFrame[i] + f)), ReadResult[i].format())); return eAppRes::Error; } }

  if(m_GatherTime) { m_Ticks____Load += (xTSC() - T); }
  return eAppRes::Good;
}
void xAppQMIVGS::calcHashClouds(int32 FrameIdx)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;

  xXXH128::tHash ImmHash[c_NumPics];

  for(int32 i = 0; i < c_NumPics; i++) { m_TPI.storeTask([this, &ImmHash, i](int32 /*ThId*/) { ImmHash[i] = xXXH128::calc((uint8*)(m_Cloud[i].getGaussianObjects()), m_Cloud[i].getNumValidSplats() * sizeof(FVGS::xGaussianObject)); }); }
  m_TPI.executeStoredTasks();

  if(m_PrintDebug) { fmt::print("{} CloudHash {} {}\n", xFormatIdx(FrameIdx, NOT_VALID), xXXH128::format(ImmHash[0]), xXXH128::format(ImmHash[1])); }

  for(int32 i = 0; i < c_NumPics; i++) { m_HashCloud[i].update(ImmHash[i].data(), ImmHash[i].size()); }

  if(m_GatherTime) { m_Ticks_HashCld += (xTSC() - T); }
}
void xAppQMIVGS::synthesizePics(int32 FrameIdx, int32 ViewIdx)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  if(m_PrintDebug) { fmt::print("{} Sythesize\n", xFormatIdx(FrameIdx, ViewIdx)); }

  const int32 CamIdx = m_CamManager.getRealCamIdx()[ViewIdx];
  GSR::tCamPar Cam = m_CamManager.getCamParams(CamIdx);

  for(int32 i = 0; i < c_NumPics; i++)
  {
    if(m_DebugDump) { m_Renderer->preRenderDetailedStats(FrameIdx, ViewIdx, i); }
    m_Renderer->renderFrame(&m_PicInP[i], m_PlnMskSyn[i], &m_Cloud[i], Cam);
    if(m_DebugDump) { m_Renderer->postRenderDetailedStats(); }
  }
  if(m_GatherTime) { m_Ticks___Synth += (xTSC() - T); }
}
void xAppQMIVGS::calcHashSynths(int32 FrameIdx, int32 ViewIdx)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;

  xXXH128::tHash ImmHash[c_NumPics];

  for(int32 i = 0; i < c_NumPics; i++) { m_TPI.storeTask([this, &ImmHash, i](int32 /*ThId*/) { ImmHash[i] = xPicHash::CalcPicHash(m_PicInP[i]); }); }
  m_TPI.executeStoredTasks();

  if(m_PrintDebug) { fmt::print("{} SynthHash {} {}\n", xFormatIdx(FrameIdx, ViewIdx), xXXH128::format(ImmHash[0]), xXXH128::format(ImmHash[1])); }

  for(int32 i = 0; i < c_NumPics; i++) { m_HashSynth[i].update(ImmHash[i].data(), ImmHash[i].size()); }

  if(m_GatherTime) { m_Ticks_HashSyn += (xTSC() - T); }
}
eAppRes xAppQMIVGS::writeSynths(int32 f, int32 v)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;

  const int32 LocalViewIdx = m_SynthFilePerView ? v : 0;
  for(int32 i = 0; i < c_NumPics; i++)
  {
    if(!m_WriteSynthYCbCr)
    {
      if(!m_ReorderWriteRGB)
      {
        xSeqPic::tResult WriteResult = m_SeqPicSyn[i][LocalViewIdx]->writeFrame(&(m_PicInP[i]));
        if(!WriteResult) { xErrMsg::printError(fmt::format("ERROR --> SynthFile write error (Frame={}, File={}) {}", f, m_SynthFile[i], WriteResult.format())); return eAppRes::Error; }
      }
      else
      {
        m_PicTmpP.copy(&(m_PicInP[i]));
        if(m_ColorSpaceFileS == eClrSpcApp::BGR)
        {
          m_PicTmpP.swapComponents(eCmp::C0, eCmp::C2); //BGR --> RGB
        }
        if(m_ColorSpaceFileS == eClrSpcApp::GBR)
        {
          m_PicTmpP.swapComponents(eCmp::C0, eCmp::C1); //RGB --> GRB
          m_PicTmpP.swapComponents(eCmp::C1, eCmp::C2); //GRB --> GBR
        }
        xSeqPic::tResult WriteResult = m_SeqPicSyn[i][LocalViewIdx]->writeFrame(&m_PicTmpP);
        if(!WriteResult) { xErrMsg::printError(fmt::format("ERROR --> SynthFile write error (Frame={}, File={}) {}", f, m_SynthFile[i], WriteResult.format())); return eAppRes::Error; }
      }    
    }
    else
    {
      const eClrSpcLC ColorSpace = xClrSpcAppToClrSpc(m_ColorSpaceFileS);
       xColorSpace::ConvertRGB2YCbCr(m_PicTmpP.getAddr(eCmp::LM), m_PicTmpP.getAddr(eCmp::CB), m_PicTmpP.getAddr(eCmp::CR),
         m_PicInP[i].getAddr(eCmp::R ), m_PicInP[i].getAddr(eCmp::G ), m_PicInP[i].getAddr(eCmp::B ),          
         m_PicTmpP.getStride(), m_PicTmpP.getStride(), m_PicTmpP.getWidth(), m_PicTmpP.getHeight(), m_PicTmpP.getBitDepth(), ColorSpace);
      xSeqPic::tResult WriteResult = m_SeqPicSyn[i][LocalViewIdx]->writeFrame(&m_PicTmpP);
      if(!WriteResult) { xErrMsg::printError(fmt::format("ERROR --> SynthFile write error (Frame={}, File={}) {}", f, m_SynthFile[i], WriteResult.format())); return eAppRes::Error; }
    }
  }

  if(m_GatherTime) { m_TicksWriteSyn += (xTSC() - T); }

  return eAppRes::Good;
}
void xAppQMIVGS::calcMetrics(int32 FrameIdx, int32 ViewIdx)
{
  preprocessMask(FrameIdx);
  //RGB colorspace path
  xSetProcessorsCmpWeights(m_CmpWeightsSearchRGB, m_CmpWeightsAverageRGB);
  preprocessRGB (FrameIdx);
  calcMetricsCmn(FrameIdx, ViewIdx, eClrSpcMtr::RGB);
  //YCbCr colorspace path
  xSetProcessorsCmpWeights(m_CmpWeightsSearchYCbCr, m_CmpWeightsAverageYCbCr);
  preprocessYCbCr(FrameIdx);
  calcMetricsCmn (FrameIdx, ViewIdx, eClrSpcMtr::YCbCr);
}
void xAppQMIVGS::preprocessMask(int32 FrameIdx)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;

  if(m_UseMask)
  {
    xPixelOps::Maximum(m_PicMskInP.getAddr(eCmp::LM), m_PlnMskSyn[0]->getAddr(), m_PlnMskSyn[1]->getAddr(), m_PicMskInP.getStride(), m_PlnMskSyn[0]->getStride(), m_PicMskInP.getWidth(), m_PicMskInP.getHeight());

    preprocessMaskCmn(FrameIdx);
  }

  if(m_GatherTime) { m_TicksPreprocM += (xTSC() - T); }
}
void xAppQMIVGS::preprocessRGB(int32 /*FrameIdx*/)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;
  for(int32 CmpIdx = 0; CmpIdx < m_PicInP[0].getNumCmps(); CmpIdx++)
  {
    m_ExactCmps[CmpIdx] = m_PicInP[0].equalCmp(&m_PicInP[1], (eCmp)CmpIdx);
  }
  if(m_GatherTime) { m_TicksPreprocF += (xTSC() - T); }
}
void xAppQMIVGS::preprocessYCbCr(int32 /*FrameIdx*/)
{
  QMAU_TRACE(3, "");
  uint64 T = m_GatherTime ? xTSC() : 0;

  const eClrSpcLC ColorSpace = xClrSpcAppToClrSpc(m_ColorSpaceMetric);
  for(int32 i = 0; i < c_NumPics; i++) { m_TPI.storeTask([this, i, ColorSpace](int32) { xColorSpace::ConvertRGB2YCbCr(
    m_PicInP[i].getAddr(eCmp::R ), m_PicInP[i].getAddr(eCmp::G ), m_PicInP[i].getAddr(eCmp::B ),
    m_PicInP[i].getAddr(eCmp::LM), m_PicInP[i].getAddr(eCmp::CB), m_PicInP[i].getAddr(eCmp::CR),
    m_PicInP[i].getStride(), m_PicInP[i].getStride(), m_PicInP[i].getWidth(), m_PicInP[i].getHeight(), m_PicInP[i].getBitDepth(), ColorSpace);
  } ); }
  m_TPI.executeStoredTasks();

  for(int32 CmpIdx = 0; CmpIdx < m_PicInP[0].getNumCmps(); CmpIdx++)
  {
    m_ExactCmps[CmpIdx] = m_PicInP[0].equalCmp(&m_PicInP[1], (eCmp)CmpIdx);
  }

  if(m_GatherTime) { m_TicksPreprocF += (xTSC() - T); }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xAppQMIVGS::xSetProcessorsCmpWeights(const int32V4& CmpWeightsSearch, const int32V4& CmpWeightsAverage)
{
  QMAU_TRACE(3, "");
  m_CmpWeightsSearch  = CmpWeightsSearch ;
  m_CmpWeightsAverage = CmpWeightsAverage;

  if(m_CalcSCP)
  {
    m_ProcSCP.setCmpWeightsSearch (m_CmpWeightsSearch );
    m_ProcSCP.setCmpWeightsAverage(m_CmpWeightsAverage);
  }

  if(m_CalcPSNRs)
  {
    m_ProcPSNR.setCmpWeightsSearch (m_CmpWeightsSearch );
    m_ProcPSNR.setCmpWeightsAverage(m_CmpWeightsAverage);
  }

  if(m_CalcSSIMs)
  {
    m_ProcSSIM.setCmpWeightsSearch (m_CmpWeightsSearch );
    m_ProcSSIM.setCmpWeightsAverage(m_CmpWeightsAverage);
  }
}
void xAppQMIVGS::combineFrameStats()
{
  const flt64 InvDurationDenominatorFV = m_CalcPoseTrace ? ((flt64)1.0 / ((flt64)m_NumPoseTraceFrames     * m_TimeStamp.getTicksPerMiliSec()))
                                                        : ((flt64)1.0 / ((flt64)m_NumFrames * m_NumViews * m_TimeStamp.getTicksPerMiliSec()));

  for(int32 m = 0; m < c_MetricsNum; m++)
  {
    xMetricStat& MD = m_MetricData[m];
    if(MD.getEnabled())
    {
      if(m_CalcPoseTrace) { MD.calcAvgMetric(m_NumPoseTraceFrames, 1); }
      else               { MD.calcAvgMetric(m_NumFrames, m_NumViews); }

      if(m_GatherTime) { MD.calcAvgDuration(InvDurationDenominatorFV); }
    }
  }
}
std::string xAppQMIVGS::formatResultsFile()
{
  QMAU_TRACE(2, "");
  std::time_t TimeStamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  std::string Result; Result.reserve(xMemory::getBestEffortSizePageBase());
  
  Result += fmt::format("FILE0  \"{}\"\n", m_InputFile[0]);
  Result += fmt::format("FILE1  \"{}\"\n", m_InputFile[1]);
  //if(m_UseMask) { Result += fmt::format("FILEM  \"{}\"\n", m_InputFileM); }
  if(m_CalcPoseTrace) { Result += fmt::format("POSETRACE \"{}\"  NumPoses {}\n", m_PoseTraceFile, m_NumPoseTraceFrames); }
  Result += fmt::format ("TIME   {:%Y-%m-%d  %H:%M:%S}\n", *std::localtime(&TimeStamp));

  for(int32 m = 0; m < c_MetricsNum; m++)
  {
    xMetricStat& MD = m_MetricData[m];
    if(MD.getEnabled()) { Result += MD.formatAvgMetric("", eClrSpcMtr::RGB  ) + "\n"; }
  }

  for(int32 m = 0; m < c_MetricsNum; m++)
  {
    xMetricStat& MD = m_MetricData[m];
    if(MD.getEnabled()) { Result += MD.formatAvgMetric("", eClrSpcMtr::YCbCr) + "\n"; }
  }

  return Result;
}
std::string xAppQMIVGS::formatResultsStdOut()
{
  QMAU_TRACE(2, "");
  std::string Result; Result.reserve(xMemory::getBestEffortSizePageBase());

  if(m_GatherHash)
  {
    for(int32 i = 0; i < c_NumPics; i++) { Result += fmt::format("HashCloud{} {}\n", i, xXXH128::format(m_HashCloud[i].finish())); }
    for(int32 i = 0; i < c_NumPics; i++) { Result += fmt::format("HashSynth{} {}\n", i, xXXH128::format(m_HashSynth[i].finish())); }
    Result += "\n";
  }

  for(int32 m = 0; m < c_MetricsNum; m++)
  {
    xMetricStat& MD = m_MetricData[m];
    if(MD.getEnabled()) { Result += MD.formatAvgMetric("Average      ", eClrSpcMtr::RGB  ) + "\n"; }
  }

  for(int32 m = 0; m < c_MetricsNum; m++)
  {
    xMetricStat& MD = m_MetricData[m];
    if(MD.getEnabled()) { Result += MD.formatAvgMetric("Average      ", eClrSpcMtr::YCbCr) + "\n"; }
  }

  if(m_GatherTime)
  {
    const flt64 InvDurationDenominatorF  = (flt64)1.0 / ((flt64)m_NumFrames * m_TimeStamp.getTicksPerMiliSec());
    const flt64 InvDurationDenominatorFV = m_CalcPoseTrace ? ((flt64)1.0 / ((flt64)m_NumPoseTraceFrames     * m_TimeStamp.getTicksPerMiliSec()))
                                                           : ((flt64)1.0 / ((flt64)m_NumFrames * m_NumViews * m_TimeStamp.getTicksPerMiliSec()));

    tDurationMS AvgDuration____Load = tDurationMS((flt64)m_Ticks____Load * InvDurationDenominatorF );
    tDurationMS AvgDuration_HashCld = tDurationMS((flt64)m_Ticks_HashCld * InvDurationDenominatorF );
    tDurationMS AvgDuration___Synth = tDurationMS((flt64)m_Ticks___Synth * InvDurationDenominatorFV);
    tDurationMS AvgDuration_HashSyn = tDurationMS((flt64)m_Ticks_HashSyn * InvDurationDenominatorFV);
    tDurationMS AvgDurationWriteSyn = tDurationMS((flt64)m_TicksWriteSyn * InvDurationDenominatorFV);
    Result += "\n";
                          Result += fmt::format("AvgTime          LOAD {:9.2f} ms\n", AvgDuration____Load.count());
    if(m_GatherHash   ) { Result += fmt::format("AvgTime    HASH_CLOUD {:9.2f} ms\n", AvgDuration_HashCld.count()); }
                          Result += fmt::format("AvgTime         SYNTH {:9.2f} ms\n", AvgDuration___Synth.count());
    if(m_GatherHash   ) { Result += fmt::format("AvgTime    HASH_SYNTH {:9.2f} ms\n", AvgDuration_HashSyn.count()); }
    if(m_TicksWriteSyn) { Result += fmt::format("AvgTime   WRITE_SYNTH {:9.2f} ms\n", AvgDurationWriteSyn.count()); }
    Result += formatMetricTimeStats(InvDurationDenominatorFV);
    Result += "\n";
    Result += "RENDERER";
    Result += m_Renderer->formatAndResetStats("  ", m_TimeStamp.getTicksPerMiliSec());
  }
  return Result;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB