/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefIVQM.h"
#include "xSeq.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

enum class eClrSpcApp : int32
{
  INVALID         = -1,
  //RGBs
  RGB             = 2,
  BGR             = 3,
  GBR             = 4,  
  //generic YCbCr
  YCbCr           = 16,
  //standardized YCbCr
  YCbCr_BT601     = 17,
  YCbCr_SMPTE170M = 18,
  YCbCr_BT709     = 19,
  YCbCr_SMPTE240M = 20,
  YCbCr_BT2020    = 21,
};

eClrSpcApp  xStr2ClrSpcApp    (const std::string& ClrSpc   );
std::string xClrSpcApp2Str    (eClrSpcApp         ClrSpc   );
eClrSpcLC   xClrSpcAppToClrSpc(eClrSpcApp         ClrSpcApp);

static inline bool isRGB         (eClrSpcApp ClrSpc) { return ClrSpc == eClrSpcApp::RGB || ClrSpc == eClrSpcApp::BGR || ClrSpc == eClrSpcApp::GBR; }
static inline bool isYCbCr       (eClrSpcApp ClrSpc) { return (int32)ClrSpc >= (int32)eClrSpcApp::YCbCr; }
static inline bool isDefinedYCbCr(eClrSpcApp ClrSpc) { return (int32)ClrSpc >  (int32)eClrSpcApp::YCbCr; }

//===============================================================================================================================================================================================================

enum class eMskTrMd : int32 //Mask transform modes
{
  INVALID   = -1,
  None      = 0,
  Threshold = 1,
//Log       = 2,
//Sigmoid   = 3,
};

eMskTrMd    xStr2MskTrMd(const std::string& MaskTransformMode);
std::string xMskTrMd2Str(eMskTrMd           MaskTransformMode);

enum class eSynMskMd : int32
{
  INVALID   = -1,
  None      = 0,
  Occupancy = 1,
  Alpha     = 2
};

eSynMskMd   xStr2SynMskMd(const std::string& SynthMaskMode);
std::string xSynMskMd2Str(eSynMskMd          SynthMaskMode);

//===============================================================================================================================================================================================================

bool     checkIfFileExists (eFileFmt FileFormat, std::string FileName, int32 StartFrame);
xSeqPic* createSeqByFileFmt(eFileFmt FileFormat, int32V2 Size, int32 BitDepth, eCrF ChromaFormat);

//===============================================================================================================================================================================================================
// QMAU trace
//===============================================================================================================================================================================================================
static inline void x_QMAU_TRACE_FUN(const std::string& Function, int32 Level, const std::string& Description)
{
  std::string LevelPrefix; for(int32 i = 0; i < (Level); i++) { LevelPrefix += " "; }
  fmt::print("#{}{} --> {}\n", LevelPrefix, Function, Description); std::fflush(stdout);
}

#if X_PMBB_CPP20
#  define QMAU_TRACE(Level, Description) { if(m_VerboseLevel >= 9) { x_QMAU_TRACE_FUN(std::source_location::function_name(), (Level), Description); } }
#else //X_PMBB_CPP20
#  define QMAU_TRACE(Level, Description) { if(m_VerboseLevel >= 9) { x_QMAU_TRACE_FUN(__func__, (Level), Description); } }
#endif //X_PMBB_CPP20

//===============================================================================================================================================================================================================

} //end of namespace PMBB