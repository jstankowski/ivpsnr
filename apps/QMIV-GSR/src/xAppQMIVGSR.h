/*
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "xFile.h"
#include "xStream.h"
#include "xSeq.h"
#include "xCfgINI.h"
#include "xMemory.h"
#include "xMiscUtilsCORE.h"
#include "xTimeUtils.h"
#include "xThreadPool.h"
#include "xPly.h"
#include "xPic.h"
#include "xCommonDefGSR.h"
#include "xGSR_Renderer.h"
#include "xCameraParamsManager.h"
#include "xXXH.h"
#include "xUtilsAppQM.h"
#include "xCloudCache.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xAppQMIVGSR
{
public:
  static const std::string_view c_BannerString;
  static const std::string_view c_HelpStringHead;
  static const std::string_view c_HelpStringGeneral;
  static const std::string_view c_HelpStringExample;
  std::string formatHelp();

protected:
  xCfgINI::xParser m_CfgParser;
  std::string      m_ErrorLog;

public:
  //basic io
  std::string      m_InputFile;
  std::string      m_CamParamsFile;
  FVGS::eCamParamsFormat m_CamParamsFormat = FVGS::eCamParamsFormat::PLY;
  std::string      m_CamNameTemplate = "[CamIdStr]";
  std::string      m_PoseTraceFile;
  int32V2          m_PictureSize;
  int32            m_BitDepth = 8;
  int32            m_StartFrame = 0;
  int32            m_NumberOfFrames = -1;
  //output io
  std::string      m_SynthFile;
  bool             m_SynthFilePerView = true;
  eFileFmt         m_FileFormatS = eFileFmt::RAW;
  eClrSpcApp       m_ColorSpaceFileS = eClrSpcApp::YCbCr_BT709;
  eCrF             m_ChromaFormatFileS = eCrF::CF420;
  //render setup
  bool             m_GatherHash = false;
  //operation
  int32            m_NumberOfThreads = -2;
  int32            m_VerboseLevel = 1;

  //derived
  bool  m_UsePoseTrace     = false;
  bool  m_WriteSynth       = false;
  bool  m_WriteSynthYCbCr  = false;
  bool  m_ReorderWriteRGB  = false;
  bool  m_GatherTime       = false;
  bool  m_PrintDebug       = false;

protected:
  //multithreading
  int32        m_HardwareConcurency  = 0;
  int32        m_NumberOfThreadsUsed = 0;
  xThreadPool* m_ThreadPool = nullptr;
  tThPI        m_TPI;

  //processing data
  int32 m_NumFrames  = 0;
  int32 m_NumViews   = 0;
  int32 m_NumRenders = 0;

  //sequences & buffers
  FVGS::xSeqPlyList     m_SeqPlyIn;
  FVGS::xGaussianCloudI m_Cloud;
  xCloudCache           m_CloudCache;
  std::vector<xSeqPic*> m_SeqPicSyn;
  xPicP                 m_PicOutP;
  xPicP                 m_PicTmpP;

  //cam params
  FVGS::xCameraParamsManager<flt64> m_CamManager;

  //renderer
  GSR::xRenderer* m_Renderer = nullptr;

  //hashing
  xXXH128 m_HashCloud;
  xXXH128 m_HashSynth;

  //stats & timers
  xTimeStamp m_TimeStamp;
  uint64 m_Ticks____Load = 0;
  uint64 m_Ticks_HashCld = 0;
  uint64 m_Ticks___Synth = 0;
  uint64 m_Ticks_HashSyn = 0;
  uint64 m_TicksWriteSyn = 0;
  flt64  m_InvDurationDenominatorF  = 0;
  flt64  m_InvDurationDenominatorFV = 0;

public:
  void        registerCmdParams  ();
  bool        loadConfiguration  (int argc, const char* argv[]);
  bool        readConfiguration  ();
  std::string formatConfiguration();
  eAppRes     validateInputFiles ();
  std::string formatWarnings     ();

  void    setupMultithreading ();
  void    ceaseMultithreading ();
  std::string formatMultithreading();

  eAppRes setupSeqs (); eAppRes ceaseSeqs ();
  eAppRes setupBuffs(); eAppRes ceaseBuffs();

  void    createRendererProcessor ();
  void    destroyRendererProcessor();

  eAppRes processAllFrames();

  std::string calibrateTimeStamp ();
  std::string formatResultsStdOut();

protected:
  std::string xFormatIdx(int32 FrameIdx, int32 ViewIdx) { return fmt::format("Frame {:08d} ", FrameIdx) + (m_NumViews > 1 ? fmt::format("View {:03d} ", ViewIdx) : ""); }
  std::string xFormatIdx(int32 FrameIdx) { return fmt::format("Frame {:08d} ", FrameIdx); }

  xXXH128::tHash xHashCloudFrame(const FVGS::xGaussianCloudI& Cloud);

  eAppRes xProcessStaticCams ();
  eAppRes xProcessPoseTrace  ();
  eAppRes xRenderWriteHashOne(const FVGS::xGaussianCloudI* Cloud, const GSR::tCamPar& Cam, int32 LogFrameIdx, int32 LogViewIdx, int32 OutSlotIdx);
  eAppRes xWriteSynthFrame(int32 OutSlotIdx);
  int32   xDetermineMaxNumTasks();

public:
  const std::string& getErrorLog    () const { return m_ErrorLog; }
  int32              getVerboseLevel() const { return m_VerboseLevel; }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
