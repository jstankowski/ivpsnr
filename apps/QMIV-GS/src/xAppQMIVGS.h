/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "xFile.h"
#include "xStream.h"
#include "xSeq.h"
#include "xCfgINI.h"
#include "xFmtScn.h"
#include "xMemory.h"
#include "xMiscUtilsCORE.h"
#include "xKBNS.h"
#include "xShftCompPic.h"
#include "xTimeUtils.h"
#include "xPly.h"
#include <math.h>
#include <fstream>
#include <time.h>
#include <limits>
#include <numeric>
#include <cassert>
#include <thread>
#include <filesystem>
#include "fmt/chrono.h"
#include "xUtilsAppQM.h"
#include "xMetricsAppQM.h"
#include "xAppQM_Common.h"
#include "xCommonDefGSR.h"
#include "xGSR_Renderer.h"
#include "xCameraParamsManager.h"
#include "xXXH.h"
#include "xCloudCache.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xAppQMIVGS : public xAppQM
{
public:
  static const std::string_view c_BannerString     ;
  static const std::string_view c_HelpStringHead   ;
  static const std::string_view c_HelpStringGeneral;
  static const std::string_view c_HelpStringMask   ;
  static const std::string_view c_HelpStringIV     ;
  static const std::string_view c_HelpStringExample;
  tStr formatHelp();

public:
  //basic io
  tStr        m_SynthFile[c_NumPics];
  bool        m_SynthFilePerView  = true;
  eFileFmt    m_FileFormatS       = eFileFmt::RAW;  
  eClrSpcApp  m_ColorSpaceFileS   = eClrSpcApp::YCbCr_BT709;
  eCrF        m_ChromaFormatFileS = eCrF::CF420;
  eClrSpcApp  m_ColorSpaceMetric  = eClrSpcApp::YCbCr_BT709;
  //mask io
  eSynMskMd   m_SynthMaskMode = eSynMskMd::INVALID;
  std::string m_OutputFileM;  
  //iv-specific
  int32V4     m_CmpWeightsSearchRGB   ;
  int32V4     m_CmpWeightsSearchYCbCr ;
  int32V4     m_CmpWeightsAverageRGB  ;
  int32V4     m_CmpWeightsAverageYCbCr;
  //derived
  bool        m_GatherHash      = true;
  bool        m_WriteSynth      = false;
  bool        m_WriteSynthYCbCr = false;
  bool        m_ReorderWriteRGB = false;

protected:
  //sequences
  std::array<FVGS::xSeqPlyList, c_NumPics> m_SeqPlyIn; //0=Tst,1=Ref
  std::vector<xSeqPic*>                    m_SeqPicSyn[c_NumPics];
  // buffers
  FVGS::xGaussianCloudI m_Cloud    [c_NumPics];
  GSR::tPicM*           m_PlnMskSyn[c_NumPics] = { nullptr, nullptr };
  // cloud cache
  xCloudCache           m_CloudCache[c_NumPics];

protected:
  //processing data
  int32 m_NumRenders         = 0; 
  int32 m_NumPoseTraceFrames = 0;
  
  //cam params
  tStr                              m_CamParamsFile;
  FVGS::eCamParamsFormat            m_CamParamsFormat = FVGS::eCamParamsFormat::PLY;
  tStr                              m_CamNameTemplate = "[CamIdStr]"; //only used when m_CamParamsFormat == INI
  FVGS::xCameraParamsManager<flt64> m_CamManager;
  tStr                              m_PoseTraceFile;
  bool                              m_CalcPoseTrace = false;

  //processors
  GSR::xRenderer* m_Renderer = nullptr;
  xPicP m_PicTmpP;
  //hash states
  xXXH128 m_HashCloud[c_NumPics];
  xXXH128 m_HashSynth[c_NumPics];

  //metrics data & stats
  uint64 m_Ticks___Synth = 0;
  uint64 m_Ticks_HashCld = 0;
  uint64 m_Ticks_HashSyn = 0;
  uint64 m_TicksWriteSyn = 0;

public:
  void    registerCmdParams  ();
  bool    readConfiguration  ();
  tStr    formatConfiguration();
  eAppRes validateInputFiles ();
  tStr    formatWarnings     ();

  virtual int32 determinePreferredNumThreads() final { return m_HardwareConcurency; };
  virtual int32 determineMaxNumTasks        () final;

  eAppRes     setupSeqs ();
  eAppRes     ceaseSeqs ();
  eAppRes     setupBuffs();
  eAppRes     ceaseBuffs();

  void        initMetricStorage();

  //processors
  void createRendererProcessor ();
  void destroyRendererProcessor();

  eAppRes processAllFrames (); //regular mode
  eAppRes processPoseTraces(); //pose trace mode

  eAppRes loadClouds     (int32 FrameIdx);
  void    calcHashClouds (int32 FrameIdx);
  void    synthesizePics (int32 FrameIdx, int32 ViewIdx);
  void    calcHashSynths (int32 FrameIdx, int32 ViewIdx);
  eAppRes writeSynths    (int32 FrameIdx, int32 ViewIdx);
  void    calcMetrics    (int32 FrameIdx, int32 ViewIdx);
  void    preprocessMask (int32 FrameIdx);
  void    preprocessRGB  (int32 FrameIdx);
  void    preprocessYCbCr(int32 FrameIdx);
  
  void combineFrameStats  ();
  tStr formatResultsStdOut();
  tStr formatResultsFile  ();

  bool getCalcPoseTrace() const { return m_CalcPoseTrace; }

protected:
  void xSetProcessorsCmpWeights(const int32V4& CmpWeightsSearch, const int32V4& CmpWeightsAverage);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB