/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "xAppQM_Common.h"
#include "xFile.h"
#include "xSeq.h"
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
#include "xUtilsAppQM.h"
#include "xMetricsAppQM.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xAppQMIV : public xAppQM
{
public:
  static const std::string_view c_BannerString     ;
  static const std::string_view c_HelpStringHead   ;
  static const std::string_view c_HelpStringGeneral;
  static const std::string_view c_HelpStringMask   ;
  static const std::string_view c_HelpStringERP    ;
  static const std::string_view c_HelpStringColor  ;
  static const std::string_view c_HelpStringIV     ;
  static const std::string_view c_HelpStringExample;
  tStr formatHelp();

public:
  //basic io
  eFileFmt     m_FileFormat;
  //mask io
  std::string  m_InputFileM   ;
  eFileFmt     m_FileFormatM  ;
  eCrF         m_ChromaFormatM;
  int32        m_StartFrameM  ;
  //colorspace
  eClrSpcApp  m_ColorSpaceInput  = eClrSpcApp::INVALID;
  eClrSpcApp  m_ColorSpaceMetric = eClrSpcApp::INVALID;
  //derrived
  bool        m_FileFormatRGB;
  bool        m_CvtYCbCr2RGB ;
  bool        m_CvtRGB2YCbCr ;
  bool        m_ReorderRGB   ;
  bool        m_InputRGB     ;  

protected:
  //sequences
  std::array<xSeqPic*, c_NumPics> m_SeqPicIn ; //0=Tst,1=Ref

public:
  void    registerCmdParams   ();
  bool    readConfiguration   ();
  tStr    formatConfiguration ();
  eAppRes validateInputFiles  ();
  tStr    formatWarnings      ();

  virtual int32 determinePreferredNumThreads() final;
  virtual int32 determineMaxNumTasks        () final { return m_PictureSize.getY() + 1; }

  eAppRes setupSeqs ();
  eAppRes ceaseSeqs ();
  eAppRes setupBuffs();
  eAppRes ceaseBuffs();

  void    initMetricStorage();

  eAppRes processAllFrames ();

  eAppRes loadFrames      (int32 FrameIdx);
  eAppRes validateFrames  (int32 FrameIdx);
  void    preprocessFrames(int32 FrameIdx);
  void    preprocessMask  (int32 FrameIdx);

  void combineFrameStats  ();
  tStr formatResultsStdOut();
  tStr formatResultsFile  ();
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB