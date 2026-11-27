/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xUtilsAppQM.h"
#include "xString.h"
#include "xSeqLST.h"
#include "xFile.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

eClrSpcApp xStr2ClrSpcApp(const std::string& ClrSpc)
{
  std::string ClrSpcU = xString::toUpper(ClrSpc);
  return ClrSpc=="RGB"             ? eClrSpcApp::RGB             :
         ClrSpc=="BGR"             ? eClrSpcApp::BGR             :
         ClrSpc=="GBR"             ? eClrSpcApp::GBR             :
         ClrSpc=="YCbCr"           ? eClrSpcApp::YCbCr           :
         ClrSpc=="YCbCr_BT601"     ? eClrSpcApp::YCbCr_BT601     :
         ClrSpc=="YCbCr_SMPTE170M" ? eClrSpcApp::YCbCr_SMPTE170M :
         ClrSpc=="YCbCr_BT709"     ? eClrSpcApp::YCbCr_BT709     :
         ClrSpc=="YCbCr_SMPTE240M" ? eClrSpcApp::YCbCr_SMPTE240M :
         ClrSpc=="YCbCr_BT2020"    ? eClrSpcApp::YCbCr_BT2020    :
                                     eClrSpcApp::INVALID;
}
std::string xClrSpcApp2Str(eClrSpcApp ClrSpc)
{
  switch(ClrSpc)
  {
   case eClrSpcApp::RGB             : return "RGB"             ;
   case eClrSpcApp::BGR             : return "BGR"             ;
   case eClrSpcApp::GBR             : return "GBR"             ;
   case eClrSpcApp::YCbCr           : return "YCbCr"           ;
   case eClrSpcApp::YCbCr_BT601     : return "YCbCr_BT601"     ;
   case eClrSpcApp::YCbCr_SMPTE170M : return "YCbCr_SMPTE170M" ;
   case eClrSpcApp::YCbCr_BT709     : return "YCbCr_BT709"     ;
   case eClrSpcApp::YCbCr_SMPTE240M : return "YCbCr_SMPTE240M" ;
   case eClrSpcApp::YCbCr_BT2020    : return "YCbCr_BT2020"    ;
   default                          : return "INVALID"         ;
  }
}
eClrSpcLC xClrSpcAppToClrSpc(eClrSpcApp ClrSpcApp)
{
  switch(ClrSpcApp)
  {
  case eClrSpcApp::YCbCr_BT601    : return eClrSpcLC::BT601    ;
  case eClrSpcApp::YCbCr_SMPTE170M: return eClrSpcLC::SMPTE170M;
  case eClrSpcApp::YCbCr_BT709    : return eClrSpcLC::BT709    ;
  case eClrSpcApp::YCbCr_SMPTE240M: return eClrSpcLC::SMPTE240M;
  case eClrSpcApp::YCbCr_BT2020   : return eClrSpcLC::BT2020   ;
  default                         : return eClrSpcLC::INVALID  ; 
  }
}

//===============================================================================================================================================================================================================

eMskTrMd xStr2MskTrMd(const std::string& MaskTransformMode)
{
  std::string MTMU = xString::toUpper(MaskTransformMode);
  return MTMU == "NONE"      ? eMskTrMd::None      :
         MTMU == "THRESHOLD" ? eMskTrMd::Threshold :
//       MTMU == "LOG"       ? eMskTrMd::Log       :
//       MTMU == "SIGMOID"   ? eMskTrMd::Sigmoid   :
                               eMskTrMd::INVALID   ;
}	
std::string xMskTrMd2Str(eMskTrMd MaskTransformMode)
{
  switch(MaskTransformMode)
  {
  case eMskTrMd::None     : return "None"     ;
  case eMskTrMd::Threshold: return "Threshold";
//case eMskTrMd::Log      : return "Log"      ;
//case eMskTrMd::Sigmoid  : return "Sigmoid"  ;
  default: return "INVALID";
  }
}

eSynMskMd xStr2SynMskMd(const std::string& SynthMaskMode)
{
  std::string SMML = xString::toLower(SynthMaskMode);
  return SMML == "none"      ? eSynMskMd::None      :
         SMML == "occupancy" ? eSynMskMd::Occupancy :
         SMML == "alpha"     ? eSynMskMd::Alpha     :
                               eSynMskMd::INVALID   ;
}
std::string xSynMskMd2Str(eSynMskMd SynthMaskMode)
{
  switch(SynthMaskMode)
  {
  case eSynMskMd::None     : return "None"     ;
  case eSynMskMd::Occupancy: return "Occupancy";
  case eSynMskMd::Alpha    : return "Alpha"    ;
  default: return "INVALID";
  }
}

//===============================================================================================================================================================================================================

bool checkIfFileExists(eFileFmt FileFormat, std::string FileName, int32 StartFrame)
{
  if(FileFormat == eFileFmt::RAW) { return xFile::exists                    (FileName            ); }
  else                            { return xFileListUtils::checkIfFileExists(FileName, StartFrame); }
}
xSeqPic* createSeqByFileFmt(eFileFmt FileFormat, int32V2 Size, int32 BitDepth, eCrF ChromaFormat)
{
  switch(FileFormat)
  {
  case eFileFmt::RAW: return new xSeq   (Size, BitDepth, ChromaFormat);
  case eFileFmt::PNG: return new xSeqPNG(Size, false, NOT_VALID, xSeqImgList::c_DefaultMaxNumFiles);
  case eFileFmt::BMP: return new xSeqBMP(Size, false, NOT_VALID, xSeqImgList::c_DefaultMaxNumFiles);
  default: return nullptr;
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB