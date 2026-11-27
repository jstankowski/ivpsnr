/*
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xAppQMIVGSR.h"
#include "xErrMsg.h"
#include "xFmtScn.h"
#include "xMiscUtilsCORE.h"
#include "xSeqLST.h"
#include "xFileListUtils.h"
#include "xPicHash.h"
#include "xColorSpace.h"
#include <thread>
#include "xUtilsAppGS.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

const std::string_view xAppQMIVGSR::c_BannerString =
R"PMBBRAWSTRING(
=============================================================================
QMIV-GS-renderer software v4.0-rc1   [Gaussian-splat renderer/synthesizer]

Copyright (c) 2020-2026, Poznan University of Technology, All rights reserved.

Developed at Poznan University of Technology, Poznan, Poland
Authors: Jakub Stankowski, Błażej Szydełko, Adrian Dziembowski

=============================================================================

)PMBBRAWSTRING";

const std::string_view xAppQMIVGSR::c_HelpStringHead =
R"PMBBRAWSTRING(
=============================================================================
QMIV-GS-renderer software v4.0-rc1
)PMBBRAWSTRING";

const std::string_view xAppQMIVGSR::c_HelpStringGeneral =
R"PMBBRAWSTRING(
usage::general --------------------------------------------------------------
 -i    InputFile         File path - gaussian splat cloud (fmtlib pattern with {FrameIdx} token, e.g. "cloud_{FrameIdx:04d}.ply", or a single static .ply)
 -cpf  CamParamsFile     Camera parameters file/directory
 -cpm  CamParamsFormat   Camera parameters format (optional, default=PLY) [PLY, COLMAP, INI]
 -cpt  CamNameTemplate   Camera name template (optional, only used when CamParamsFormat=INI, default="[CamIdStr]")
 -ptf  PoseTraceFile     Pose trace (optional), one camera pose per frame, replaces the static cameras. Requires CamParamsFile.
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
 -ps   PictureSize       Resolution of rendered image (WxH)
 -bd   BitDepth          Bit depth of rendered image (optional, default=8, up to 14)
 -sf   StartFrame        Start frame (optional, auto=-1, default=-1)
 -nf   NumberOfFrames    Number of frames to process (optional, -1=all, default=-1)
 -s    SynthFile         Output base name for rendered pictures (optional)
                         Filename pattern may contain {W}, {H}, and {ViewIdx} e.g. Render_{W}x{H}_V{ViewIdx:d}.yuv
 -sfv  SynthFilePerView  Use a separate output file for each camera (optional, default=1). Ignored in posetrace mode (always one file).
 -ffs  FileFormatS       File format of rendered output (optional, default=RAW) [RAW, PNG, BMP]
 -css  ColorSpaceFileS   Color space of rendered output (optional, default=YCbCr_BT709)
                         [RGB, BGR, GBR, YCbCr_BT601, YCbCr_SMPTE170M, YCbCr_BT709, YCbCr_SMPTE240M, YCbCr_BT2020]
 -cfs  ChromaFormatFileS Chroma format for rendered output (optional, default=420) [400, 420, 422, 444]
 -gh   GatherHash        Compute reproducibility hashes of the input cloud(s) and rendered output (optional, default=1)
 -nth  NumberOfThreads   Number of worker threads (optional, -1=all, -2=auto, default=-2)
 -v    VerboseLevel      Verbosity level (optional, default=1)

 -c    "config.cfg"      External config file - in INI format (optional)

-----------------------------------------------------------------------------
VerboseLevel:
  0 = final hash values only
  1 = 0 + configuration + detected/processed frame numbers + input/output file listing
  2 = 1 + argc/argv + working directory + build info
  3 = 2 + computing time statistics
  4 = 3 + per-frame debug data (cloud/render hashes, cache loading progress)
)PMBBRAWSTRING";

const std::string_view xAppQMIVGSR::c_HelpStringExample =
R"PMBBRAWSTRING(-----------------------------------------------------------------------------
Example:
  QMIV-GS-renderer -i "cloud_{FrameIdx:04d}.ply" -cpf "cameras.cfg" -cpm INI -ps 1920x1080 -s "Render_{W}x{H}_V{ViewIdx:d}.yuv" -v 2

=============================================================================
)PMBBRAWSTRING";

//===============================================================================================================================================================================================================

std::string xAppQMIVGSR::formatHelp()
{
  std::string Result = "";
  Result += c_HelpStringHead;
  Result += c_HelpStringGeneral;
  Result += c_HelpStringExample;
  return Result;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xAppQMIVGSR::registerCmdParams()
{
  //basic io
  m_CfgParser.addCmdParm("i"  , "InputFile"       , "", "InputFile"       );
  m_CfgParser.addCmdParm("cpf", "CamParamsFile"   , "", "CamParamsFile"   );
  m_CfgParser.addCmdParm("cpm", "CamParamsFormat" , "", "CamParamsFormat" );
  m_CfgParser.addCmdParm("cpt", "CamNameTemplate" , "", "CamNameTemplate" );
  m_CfgParser.addCmdParm("ptf", "PoseTraceFile"   , "", "PoseTraceFile"   );
  m_CfgParser.addCmdParm("ps" , "PictureSize"     , "", "PictureSize"     );
  m_CfgParser.addCmdParm("bd" , "BitDepth"        , "", "BitDepth"        );
  m_CfgParser.addCmdParm("sf" , "StartFrame"      , "", "StartFrame"      );
  m_CfgParser.addCmdParm("nf" , "NumberOfFrames"  , "", "NumberOfFrames"  );
  //output io
  m_CfgParser.addCmdParm("s"  , "SynthFile"        , "", "SynthFile"        );
  m_CfgParser.addCmdParm("sfv", "SynthFilePerView" , "", "SynthFilePerView" );
  m_CfgParser.addCmdParm("ffs", "FileFormatS"      , "", "FileFormatS"      );
  m_CfgParser.addCmdParm("css", "ColorSpaceFileS"  , "", "ColorSpaceFileS"  );
  m_CfgParser.addCmdParm("cfs", "ChromaFormatFileS", "", "ChromaFormatFileS");
  m_CfgParser.addCmdParm("gh" , "GatherHash"      , "", "GatherHash"      );
  //operation
  m_CfgParser.addCmdParm("nth", "NumberOfThreads" , "", "NumberOfThreads" );
  m_CfgParser.addCmdParm("v"  , "VerboseLevel"    , "", "VerboseLevel"    );
}
bool xAppQMIVGSR::loadConfiguration(int argc, const char* argv[])
{
  bool CommandlineResult = m_CfgParser.loadFromCmdln(argc, argv);
  if(!CommandlineResult)
  {
    m_ErrorLog += "! invalid commandline\n";
    m_ErrorLog += m_CfgParser.getParsingLog();
  }
  return CommandlineResult;
}
bool xAppQMIVGSR::readConfiguration()
{
  bool Correct = true;

  //basic io ------------------------------------------------------------------------------------------------------------
  m_InputFile = m_CfgParser.getParam1stArg("InputFile", std::string(""));
  if(m_InputFile.empty()) { m_ErrorLog += "!  InputFile is empty\n"; Correct = false; }

  m_CamParamsFile   = m_CfgParser.getParam1stArg("CamParamsFile", std::string(""));
  m_CamParamsFormat = m_CfgParser.cvtParam1stArg("CamParamsFormat", FVGS::eCamParamsFormat::PLY, FVGS::xStrToCamParamsFormat);
  m_CamNameTemplate = m_CfgParser.getParam1stArg("CamNameTemplate", std::string("[CamIdStr]"));
  if(m_CamParamsFile.empty()) { m_ErrorLog += "!  CamParamsFile is empty\n"; Correct = false; }
  if(m_CamParamsFormat == FVGS::eCamParamsFormat::INVALID) { m_ErrorLog += "!  Invalid CamParamsFormat value\n"; Correct = false; }

  m_PoseTraceFile = m_CfgParser.getParam1stArg("PoseTraceFile", std::string(""));
  m_UsePoseTrace  = !m_PoseTraceFile.empty();

  std::string PictureSizeS = m_CfgParser.getParam1stArg("PictureSize", std::string(""));
  m_PictureSize = xFmtScn::scanResolution(PictureSizeS);
  if(m_PictureSize[0] <= 0 || m_PictureSize[1] <= 0) { m_ErrorLog += "!  Invalid PictureSize value\n"; Correct = false; }
  m_BitDepth       = m_CfgParser.getParam1stArg("BitDepth"      , 8 );
  m_StartFrame     = m_CfgParser.getParam1stArg("StartFrame"    , NOT_VALID);
  if(m_StartFrame < -1) { m_ErrorLog += "!  StartFrame value cannot be negative\n"; Correct = false; }
  m_NumberOfFrames = m_CfgParser.getParam1stArg("NumberOfFrames", -1);

  //output io -----------------------------------------------------------------------------------------------------------
  m_SynthFile         = m_CfgParser.getParam1stArg("SynthFile"       , std::string(""));
  m_WriteSynth        = !m_SynthFile.empty();
  m_SynthFilePerView = m_CfgParser.getParam1stArg("SynthFilePerView", 1);
  m_FileFormatS      = m_CfgParser.cvtParam1stArg("FileFormatS"     , eFileFmt::RAW          , xStr2FileFmt );
  m_ColorSpaceFileS   = m_CfgParser.cvtParam1stArg("ColorSpaceFileS"  , eClrSpcApp::YCbCr_BT709, xStr2ClrSpcApp);
  m_ChromaFormatFileS = m_CfgParser.cvtParam1stArg("ChromaFormatFileS", eCrF::CF420            , xStr2CrF      );
  m_WriteSynthYCbCr   = isDefinedYCbCr(m_ColorSpaceFileS);
  m_ReorderWriteRGB   = isRGB(m_ColorSpaceFileS) && m_ColorSpaceFileS != eClrSpcApp::RGB;

  //render setup ----------------------------------------------------------------------------------------------------------
  m_GatherHash    = m_CfgParser.getParam1stArg("GatherHash"   , 1);

  //operation -------------------------------------------------------------------------------------------------------------
  m_NumberOfThreads = m_CfgParser.getParam1stArg("NumberOfThreads", -2);
  m_VerboseLevel    = m_CfgParser.getParam1stArg("VerboseLevel"   , 1 );
  m_GatherTime      = m_VerboseLevel >= 3;
  m_PrintDebug      = m_VerboseLevel >= 4;

  //post-validation ---------------------------------------------------------------------------------------------------
  if(m_BitDepth < 8 || m_BitDepth > 14) { m_ErrorLog += "! BitDepth has to be in range [8,14]\n"; Correct = false; }
  if(m_FileFormatS == eFileFmt::INVALID) { m_ErrorLog += "! Invalid FileFormatS value\n"; Correct = false; }
  if(m_ColorSpaceFileS == eClrSpcApp::INVALID) { m_ErrorLog += "! Invalid ColorSpaceFileS value\n"; Correct = false; }
  if(m_WriteSynth && m_ChromaFormatFileS == eCrF::CF420 && ((m_PictureSize.getX() & 0x1) || (m_PictureSize.getY() & 0x1)))
  {
    m_ErrorLog += "! Chroma format 420 requires PictureWidth and PictureHeight to be even\n"; Correct = false;
  }
  if(m_WriteSynth && m_ChromaFormatFileS == eCrF::CF422 && (m_PictureSize.getX() & 0x1))
  {
    m_ErrorLog += "! Chroma format 422 requires PictureWidth to be even\n"; Correct = false;
  }
  if(m_UsePoseTrace && m_CamParamsFile.empty()) { m_ErrorLog += "! PoseTraceFile requires CamParamsFile\n"; Correct = false; }

  return Correct;
}
std::string xAppQMIVGSR::formatConfiguration()
{
  std::string Config; Config.reserve(xMemory::getBestEffortSizePageBase());
  Config += "Run-time configuration:\n";
  Config += fmt::format("InputFile         = {}\n"  , m_InputFile);
  Config += fmt::format("CamParamsFile     = {}\n"  , m_CamParamsFile);
  Config += fmt::format("CamParamsFormat   = {}\n"  , FVGS::xCamParamsFormatToStr(m_CamParamsFormat));
  Config += fmt::format("CamNameTemplate   = {}\n"  , m_CamNameTemplate);
  Config += fmt::format("PoseTraceFile     = {}\n"  , m_PoseTraceFile.empty() ? "(unused)" : m_PoseTraceFile);
  Config += fmt::format("PictureSize       = {}\n"  , xFmtScn::formatResolution(m_PictureSize));
  Config += fmt::format("BitDepth          = {}\n"  , m_BitDepth);
  Config += fmt::format("StartFrame        = {}\n"  , m_StartFrame);
  Config += fmt::format("NumberOfFrames    = {}{}\n", m_NumberOfFrames, m_NumberOfFrames==NOT_VALID ? "  (all)" : "");
  Config += fmt::format("SynthFile         = {}\n"  , m_SynthFile.empty() ? "(unused)" : m_SynthFile);
  Config += fmt::format("SynthFilePerView  = {}\n"  , m_SynthFilePerView);
  Config += fmt::format("FileFormatS       = {}\n"  , xFileFmt2Str(m_FileFormatS));
  Config += fmt::format("ColorSpaceFileS   = {}\n"  , xClrSpcApp2Str(m_ColorSpaceFileS));
  Config += fmt::format("ChromaFormatFileS = {}{}\n", xCrF2Str(m_ChromaFormatFileS), !m_WriteSynthYCbCr ? "  (irrelevant)" : "");
  Config += fmt::format("GatherHash        = {}\n"  , m_GatherHash);
  Config += fmt::format("NumberOfThreads   = {}\n"  , m_NumberOfThreads);
  Config += fmt::format("VerboseLevel      = {}\n"  , m_VerboseLevel);
  Config += "\n";
  return Config;
}
eAppRes xAppQMIVGSR::validateInputFiles()
{
  return eAppRes::Good;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int32 xAppQMIVGSR::xDetermineMaxNumTasks()
{
  int32 NumTiles    = GSR::calcNumTiles(m_PictureSize);
  int32 MaxNumTasks = xMax(4096, m_PictureSize.getY(), NumTiles);
  return MaxNumTasks;
}
void xAppQMIVGSR::setupMultithreading()
{
  m_HardwareConcurency  = std::thread::hardware_concurrency();
  m_NumberOfThreadsUsed = 0;
  if(m_NumberOfThreads >=  1) { m_NumberOfThreadsUsed = xMin(m_NumberOfThreads, m_HardwareConcurency); }
  if(m_NumberOfThreads == -1) { m_NumberOfThreadsUsed = m_HardwareConcurency; }
  if(m_NumberOfThreads == -2) { m_NumberOfThreadsUsed = m_HardwareConcurency; }
  if(m_NumberOfThreadsUsed > 0)
  {
    const int32 MaxNumTasks = xDetermineMaxNumTasks();
    m_ThreadPool = new xThreadPool;
    m_ThreadPool->create(m_NumberOfThreadsUsed, MaxNumTasks + 1);
    m_TPI.init(m_ThreadPool, MaxNumTasks, MaxNumTasks);
  }
}
void xAppQMIVGSR::ceaseMultithreading()
{
  if(m_NumberOfThreadsUsed)
  {
    m_TPI.uninit();
    m_ThreadPool->destroy();
    m_ThreadPool = nullptr;
  }
}
std::string xAppQMIVGSR::formatMultithreading()
{
  std::string Info = "";
  Info += fmt::format("Multithreading:\n");
  Info += fmt::format("HardwareConcurency  = {}\n", m_HardwareConcurency );
  Info += fmt::format("NumberOfThreadsUsed = {}\n", m_NumberOfThreadsUsed);
  return Info;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

eAppRes xAppQMIVGSR::setupSeqs()
{
  //create + open input cloud sequence
  m_SeqPlyIn.create(false, m_StartFrame, xSeqImgList::c_DefaultMaxNumFiles);

  xSeqPic::tResult OpenResult = m_SeqPlyIn.openFile(m_InputFile, xSeq::eMode::Read);
  if(!OpenResult) { xErrMsg::printError(fmt::format("ERROR --> InputFile opening failure ({}) {}", m_InputFile, OpenResult.format())); return eAppRes::Error; }

  //load camera parameters
  if(!m_CamManager.loadRealCams(m_CamParamsFormat, m_CamParamsFile, m_CamNameTemplate, m_PictureSize))
  {
    xErrMsg::printError(fmt::format("ERROR --> failed to load {} camera parameters from CamParamsFile ({})", FVGS::xCamParamsFormatToStr(m_CamParamsFormat), m_CamParamsFile));
    return eAppRes::Error;
  }
  m_NumViews = m_CamManager.getNumRealCams();
  if(m_VerboseLevel >= 1) { fmt::print("NumViews\t= {}\n", m_NumViews); }

  //load posetrace, if requested - switches the whole run to posetrace mode
  if(m_UsePoseTrace)
  {
    if(m_CamManager.getNumRealCams() == 0) { xErrMsg::printError("ERROR --> PoseTraceFile requires at least one camera intrinsics loaded via CamParamsFile"); return eAppRes::Error; }
    if(!m_CamManager.loadPoseTraceParams(FVGS::xCameraParamsManager<flt64>::c_VirtGroupIdx, m_PoseTraceFile, m_PictureSize))
    {
      xErrMsg::printError(fmt::format("ERROR --> failed to load PoseTraceFile ({})", m_PoseTraceFile));
      return eAppRes::Error;
    }
    if(m_VerboseLevel >= 1) { fmt::print("PoseTraceFrames  = {}\n", m_CamManager.getNumVirtCams()); }
  }

  //first frame & num of frames
  const int32 DetectedNumOfFrames = m_SeqPlyIn.getNumOfFrames ();
  const int32 DetectedFirstFrame  = m_SeqPlyIn.getCurrFrameIdx();
  if(m_VerboseLevel >= 1) { fmt::print("DetectedNumFrames  = {}\n", DetectedNumOfFrames); }
  if(m_VerboseLevel >= 1) { fmt::print("DetectedFirstFrame = {}\n", DetectedFirstFrame ); }
  if(DetectedNumOfFrames <= 0) { xErrMsg::printError(fmt::format("ERROR --> no frames found in ({})", m_InputFile)); return eAppRes::Error; }
  if(m_StartFrame == NOT_VALID) { m_StartFrame = DetectedFirstFrame; } //auto first frame mode
  if(m_StartFrame < DetectedFirstFrame) { xErrMsg::printError(fmt::format("ERROR --> StartFrame < DetectedFirstFrame for ({})", m_InputFile)); return eAppRes::Error; }
  if(m_StartFrame >= DetectedNumOfFrames + DetectedFirstFrame) { xErrMsg::printError(fmt::format("ERROR --> StartFrame >= DetectedNumOfFrames + DetectedFirstFrame for ({})", m_InputFile)); return eAppRes::Error; }
  const int32 RemFrames = DetectedNumOfFrames + DetectedFirstFrame - m_StartFrame;
  if(m_NumberOfFrames > 0 && m_NumberOfFrames > RemFrames) { xErrMsg::printError(fmt::format("ERROR --> requested NumberOfFrames ({}) > remaining frames ({})", m_NumberOfFrames, RemFrames)); return eAppRes::Error; }
  m_NumFrames = xMin(m_NumberOfFrames > 0 ? m_NumberOfFrames : DetectedNumOfFrames, RemFrames);
  if(m_VerboseLevel >= 1) { fmt::print("FramesToProcess     = {}\n", m_NumFrames); }
  if(m_StartFrame != DetectedFirstFrame)
  {
    xSeqPic::tResult SeekResult = m_SeqPlyIn.seekFrame(m_StartFrame);
    if(!SeekResult) { xErrMsg::printError(fmt::format("ERROR --> InputFile seeking failure ({}) {}", m_InputFile, SeekResult.format())); return eAppRes::Error; }
  }
  m_NumRenders = m_UsePoseTrace ? m_CamManager.getNumVirtCams() : (m_NumFrames * m_NumViews);
  if (m_VerboseLevel >= 1) { fmt::print("NumRenders\t= {}\n", m_NumRenders); }
  if(m_VerboseLevel >= 1)
  {
    fmt::print("Input files:\n");
    for(int32 f = 0; f < m_NumFrames; f++)
    {
      fmt::print("\t{}\n", xFileListUtils::formatNamedPattern(m_InputFile, m_StartFrame + f, fmt::arg("FrameIdx", m_StartFrame + f)));
    }
  }
  //output sequence(s)
  if(m_WriteSynth)
  {
    const int32 NumSlotsToOpen = (m_UsePoseTrace || !m_SynthFilePerView) ? 1 : m_NumViews;
    m_SeqPicSyn.resize(NumSlotsToOpen);
    if(m_VerboseLevel >= 1) { fmt::print("Output files:\n"); }
    for(int32 v = 0; v < NumSlotsToOpen; v++)
    {
      m_SeqPicSyn[v] = createSeqByFileFmt(m_FileFormatS, m_PictureSize, m_BitDepth, m_WriteSynthYCbCr ? m_ChromaFormatFileS : eCrF::CF444);
      if(!m_SeqPicSyn[v]) { xErrMsg::printError(fmt::format("ERROR --> unsupported FileFormatS ({})", xFileFmt2Str(m_FileFormatS))); return eAppRes::Error; }
      std::string FileName = xFileListUtils::formatNamedPattern(m_SynthFile, fmt::arg("W", m_PictureSize.getX()), fmt::arg("H", m_PictureSize.getY()), fmt::arg("ViewIdx", v));
      if(FileName.empty()) { xErrMsg::printError(fmt::format("ERROR --> SynthFile formatting failure ({})", m_SynthFile)); return eAppRes::Error; }
      bool OpenSuccess = (bool)(m_SeqPicSyn[v]->openFile(FileName, xSeq::eMode::Write));
      if(!OpenSuccess) { xErrMsg::printError(fmt::format("ERROR --> SynthFile opening failure ({})", FileName)); return eAppRes::Error; }
      if(m_VerboseLevel >= 1) { fmt::print("\t{}\n", FileName); }
    }
  }

  return eAppRes::Good;
}
eAppRes xAppQMIVGSR::ceaseSeqs()
{
  m_SeqPlyIn.closeFile();
  m_SeqPlyIn.destroy  ();

  for(auto& Seq : m_SeqPicSyn) { Seq->closeFile(); Seq->destroy(); }
  m_SeqPicSyn.clear();

  return eAppRes::Good;
}
eAppRes xAppQMIVGSR::setupBuffs()
{
  m_PicOutP.create(m_PictureSize, m_BitDepth, 0);
  if(m_WriteSynthYCbCr || m_ReorderWriteRGB) { m_PicTmpP.create(m_PictureSize, m_BitDepth, 0); }
  return eAppRes::Good;
}
eAppRes xAppQMIVGSR::ceaseBuffs()
{
  m_PicOutP.destroy();
  if(m_WriteSynthYCbCr || m_ReorderWriteRGB) { m_PicTmpP.destroy(); }
  return eAppRes::Good;
}
std::string xAppQMIVGSR::formatWarnings()
{
  std::string Warnings = "";

#if defined(X_PMBB_ARCH_AMD64) && !(X_SIMD_CAN_USE_AVX || X_SIMD_HAS_AVX512)
  Warnings += "REPRODUCIBILITY WARNING: The Gaussian splating renderer uses FMA operations. The older (pre-AVX2) x86-64 architectures does not support FMA instructions. In current version of the software, rendering results can differ (it will be fixed in future releases).\n\n";
#endif

  return Warnings;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xAppQMIVGSR::createRendererProcessor()
{
  m_Renderer = new GSR::xRenderer();
  m_Renderer->create(m_PictureSize, m_ThreadPool);
  m_Renderer->setGatherTimeStats(m_GatherTime);
  if(m_GatherHash) { m_HashCloud.init(); m_HashSynth.init(); }
}
void xAppQMIVGSR::destroyRendererProcessor()
{
  m_Renderer->destroy(); delete m_Renderer; m_Renderer = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

eAppRes xAppQMIVGSR::xWriteSynthFrame(int32 OutSlotIdx)
{
  uint64 T = m_GatherTime ? xTSC() : 0;

  xPicP* PicToWrite = &m_PicOutP;
  if(!m_WriteSynthYCbCr)
  {
    if(m_ReorderWriteRGB)
    {
      m_PicTmpP.copy(&m_PicOutP);
      if(m_ColorSpaceFileS == eClrSpcApp::BGR)
      {
        m_PicTmpP.swapComponents(eCmp::C0, eCmp::C2); //BGR --> RGB
      }
      if(m_ColorSpaceFileS == eClrSpcApp::GBR)
      {
        m_PicTmpP.swapComponents(eCmp::C0, eCmp::C1); //RGB --> GRB
        m_PicTmpP.swapComponents(eCmp::C1, eCmp::C2); //GRB --> GBR
      }
      PicToWrite = &m_PicTmpP;
    }
  }
  else
  {
    const eClrSpcLC ColorSpace = xClrSpcAppToClrSpc(m_ColorSpaceFileS);
    xColorSpace::ConvertRGB2YCbCr(m_PicTmpP.getAddr(eCmp::LM), m_PicTmpP.getAddr(eCmp::CB), m_PicTmpP.getAddr(eCmp::CR),
      m_PicOutP.getAddr(eCmp::R ), m_PicOutP.getAddr(eCmp::G ), m_PicOutP.getAddr(eCmp::B ),
      m_PicTmpP.getStride(), m_PicTmpP.getStride(), m_PicTmpP.getWidth(), m_PicTmpP.getHeight(), m_PicTmpP.getBitDepth(), ColorSpace);
    PicToWrite = &m_PicTmpP;
  }

  xSeqPic::tResult WriteResult = m_SeqPicSyn[OutSlotIdx]->writeFrame(PicToWrite);
  if(!WriteResult) { xErrMsg::printError(fmt::format("ERROR --> SynthFile write error {}", WriteResult.format())); return eAppRes::Error; }

  if(m_GatherTime) { m_TicksWriteSyn += (xTSC() - T); }
  return eAppRes::Good;
}
eAppRes xAppQMIVGSR::xRenderWriteHashOne(const FVGS::xGaussianCloudI* Cloud, const GSR::tCamPar& Cam, int32 LogFrameIdx, int32 LogViewIdx, int32 OutSlotIdx)
{
  uint64 SynthT = m_GatherTime ? xTSC() : 0;
  if(m_PrintDebug) { m_Renderer->preRenderDetailedStats(LogFrameIdx, LogViewIdx, 0); }
  m_Renderer->renderFrame(&m_PicOutP, nullptr, Cloud, Cam);
  if(m_PrintDebug) { m_Renderer->postRenderDetailedStats(); }
  if(m_GatherTime) { m_Ticks___Synth += (xTSC() - SynthT); }

  if(m_GatherHash)
  {
    uint64 HashT = m_GatherTime ? xTSC() : 0;
    xXXH128::tHash Hash = xPicHash::CalcPicHash(m_PicOutP);
    if(m_PrintDebug)
    {
      std::string Label = m_UsePoseTrace ? ("PoseTrace " + xFormatIdx(LogFrameIdx)) : xFormatIdx(LogFrameIdx, LogViewIdx);
      fmt::print("{}SynthHash {}\n", Label, xXXH128::format(Hash));
    }
    m_HashSynth.update(Hash.data(), Hash.size());
    if(m_GatherTime) { m_Ticks_HashSyn += (xTSC() - HashT); }
  }

  return m_WriteSynth ? xWriteSynthFrame(OutSlotIdx) : eAppRes::Good;
}
xXXH128::tHash xAppQMIVGSR::xHashCloudFrame(const FVGS::xGaussianCloudI& Cloud)
{
  uint64 HashT = m_GatherTime ? xTSC() : 0;
  xXXH128::tHash Hash = CalcCloudHash(&Cloud);
  m_HashCloud.update(Hash.data(), Hash.size());
  if(m_GatherTime) { m_Ticks_HashCld += (xTSC() - HashT); }
  return Hash;
}
eAppRes xAppQMIVGSR::xProcessStaticCams()
{
  const auto& RealCamIdx = m_CamManager.getRealCamIdx();
  for(int32 f = 0; f < m_NumFrames; f++)
  {
    uint64 LoadT = m_GatherTime ? xTSC() : 0;
    xSeqPic::tResult ReadResult = m_SeqPlyIn.readGC(&m_Cloud);
    if(!ReadResult) { xErrMsg::printError(fmt::format("ERROR --> InputFile read error (Frame={}) {}", f, ReadResult.format())); return eAppRes::Error; }
    if(m_GatherTime) { m_Ticks____Load += (xTSC() - LoadT); }

    if(m_GatherHash)
    {
      xXXH128::tHash Hash = xHashCloudFrame(m_Cloud);
      if(m_PrintDebug) { fmt::print("{}CloudHash {}\n", xFormatIdx(f), xXXH128::format(Hash)); }
    }

    for(int32 v = 0; v < m_NumViews; v++)
    {
      const GSR::tCamPar& Cam = m_CamManager.getCamParams(RealCamIdx[v]);
      eAppRes Res = xRenderWriteHashOne(&m_Cloud, Cam, f, v, m_SynthFilePerView ? v : 0);
      if(Res != eAppRes::Good) { return Res; }
    }
  }
  return eAppRes::Good;
}
eAppRes xAppQMIVGSR::xProcessPoseTrace()
{
  //cache all cloud frames
  m_CloudCache.create(m_NumFrames);

  uint64 LoadT = m_GatherTime ? xTSC() : 0;
  xSeqPic::tResult SeekResult = m_SeqPlyIn.seekFrame(m_StartFrame);
  if(!SeekResult) { xErrMsg::printError(fmt::format("ERROR --> InputFile seek error (Frame={}) {}", m_StartFrame, SeekResult.format())); return eAppRes::Error; }
  xSeqPic::tResult ReadResult = m_CloudCache.load(&m_SeqPlyIn, m_InputFile, m_StartFrame, m_PrintDebug);
  if(!ReadResult) { return eAppRes::Error; }
  if(m_GatherTime) { m_Ticks____Load += (xTSC() - LoadT); }

  if(m_GatherHash)
  {
    uint64 HashT = m_GatherTime ? xTSC() : 0;
    m_CloudCache.hash(m_HashCloud);
    if(m_GatherTime) { m_Ticks_HashCld += (xTSC() - HashT); }
  }

  const auto& VirtCamIdx = m_CamManager.getVirtCamIdx();
  for(int32 p = 0; p < (int32)VirtCamIdx.size(); p++)
  {
    const int32 CloudFrameIdx = xMirrorFrameIdx(p, m_NumFrames);
    const GSR::tCamPar& Cam   = m_CamManager.getCamParams(VirtCamIdx[p]);
    eAppRes Res = xRenderWriteHashOne(m_CloudCache.getCloud(CloudFrameIdx), Cam, p, 0, 0);
    if(Res != eAppRes::Good) { return Res; }
  }

  m_CloudCache.destroy();
  return eAppRes::Good;
}
eAppRes xAppQMIVGSR::processAllFrames()
{
  if(m_VerboseLevel >= 1) { fmt::print("\nProcessing:\n"); }
  m_TimeStamp.sampleBeg();
  eAppRes Res = m_UsePoseTrace ? xProcessPoseTrace() : xProcessStaticCams();
  m_TimeStamp.sampleEnd();
  return Res;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

std::string xAppQMIVGSR::calibrateTimeStamp()
{
  m_TimeStamp.calibrateTimeStamp();
  m_InvDurationDenominatorF  = (flt64)1.0 / ((flt64)m_NumFrames  * m_TimeStamp.getTicksPerMiliSec());
  m_InvDurationDenominatorFV = (flt64)1.0 / ((flt64)m_NumRenders * m_TimeStamp.getTicksPerMiliSec());
  return m_TimeStamp.formatCalibration();
}
std::string xAppQMIVGSR::formatResultsStdOut()
{
  std::string Result; Result.reserve(xMemory::getBestEffortSizePageBase());

  if(m_GatherHash)
  {
    Result += fmt::format("HashCloud {}\n"  , xXXH128::format(m_HashCloud.finish()));
    Result += fmt::format("HashSynth {}\n\n", xXXH128::format(m_HashSynth.finish()));
  }

  if(m_GatherTime)
  {
    tDurationMS AvgDuration____Load = tDurationMS((flt64)m_Ticks____Load * m_InvDurationDenominatorF );
    tDurationMS AvgDuration_HashCld = tDurationMS((flt64)m_Ticks_HashCld * m_InvDurationDenominatorF );
    tDurationMS AvgDuration___Synth = tDurationMS((flt64)m_Ticks___Synth * m_InvDurationDenominatorFV);
    tDurationMS AvgDuration_HashSyn = tDurationMS((flt64)m_Ticks_HashSyn * m_InvDurationDenominatorFV);
    tDurationMS AvgDurationWriteSyn = tDurationMS((flt64)m_TicksWriteSyn * m_InvDurationDenominatorFV);
    Result += "\n";
                          Result += fmt::format("AvgTime          LOAD {:9.2f} ms\n", AvgDuration____Load.count());
    if(m_GatherHash   ) { Result += fmt::format("AvgTime    HASH_CLOUD {:9.2f} ms\n", AvgDuration_HashCld.count()); }
                          Result += fmt::format("AvgTime         SYNTH {:9.2f} ms\n", AvgDuration___Synth.count());
    if(m_GatherHash   ) { Result += fmt::format("AvgTime    HASH_SYNTH {:9.2f} ms\n", AvgDuration_HashSyn.count()); }
    if(m_TicksWriteSyn) { Result += fmt::format("AvgTime   WRITE_SYNTH {:9.2f} ms\n", AvgDurationWriteSyn.count()); }
    Result += "\n";
    Result += "RENDERER";
    Result += m_Renderer->formatAndResetStats("  ", m_TimeStamp.getTicksPerMiliSec());
  }
  return Result;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
