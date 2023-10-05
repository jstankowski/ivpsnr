/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xFmtScn.h"
#include "xString.h"
#include <filesystem>
#include <charconv>
#include <regex>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

int32V2 xFmtScn::scanResolution(const std::string& ResolutionString)
{
  if(ResolutionString.empty() || ResolutionString.length() < 3) {return { NOT_VALID, NOT_VALID }; }

  std::string_view ResolutionView = ResolutionString;
  ResolutionView = xString::stripR(xString::stripL(ResolutionView));

  if(ResolutionView.empty() || ResolutionView.length() < 3) {return { NOT_VALID, NOT_VALID }; }

  uintPtr NumX = std::count_if(ResolutionView.begin(), ResolutionView.end(), [](char i) { return (i == 'x' || i == 'X'); });
  if(NumX != 1) { return { NOT_VALID, NOT_VALID }; }

  const uintSize Pos = xMin(ResolutionView.find('x'), ResolutionView.find('X'));
  const uintSize Len = ResolutionView.length();
  if(Pos == 0 || Pos == Len-1) { return { NOT_VALID, NOT_VALID }; }

  std::string_view WidthView  = ResolutionView.substr(0, Pos);
  std::string_view HeightView = ResolutionView.substr(Pos+1);
  
  //Parsing
  int32 Width  = NOT_VALID;
  int32 Height = NOT_VALID;
  std::from_chars(WidthView .data(), WidthView .data() + WidthView .length(), Width );
  std::from_chars(HeightView.data(), HeightView.data() + HeightView.length(), Height);

  return { Width, Height };
}
int32V4 xFmtScn::scanIntWeights(const std::string& CmpWeightsString) //parse vector of 4 nonnegative integers (format {d}:{d}:{d}:{d}), returns {-1, -1, -1, -1} on failure
{
  if (CmpWeightsString.empty() || CmpWeightsString.length() < 7) { return xMakeVec4(NOT_VALID); }

  std::string_view CmpWeightsView = CmpWeightsString;
  CmpWeightsView = xString::stripR(xString::stripL(CmpWeightsView));

  if (CmpWeightsView.empty() || CmpWeightsView.length() < 7) { return xMakeVec4(NOT_VALID); }

  uintPtr NumX = std::count_if(CmpWeightsView.begin(), CmpWeightsView.end(), [](char i) { return (i == ':'); });
  if (NumX != 3) { return xMakeVec4(NOT_VALID); }

  const uintSize Pos0 = 0;
  const uintSize Pos1 = CmpWeightsView.find(':', Pos0  );
  const uintSize Pos2 = CmpWeightsView.find(':', Pos1+1);
  const uintSize Pos3 = CmpWeightsView.find(':', Pos2+1);
  const uintSize Pos4 = CmpWeightsView.length();

  if(Pos1 == std::string_view::npos || Pos2 == std::string_view::npos || Pos3 == std::string_view::npos) { return xMakeVec4(NOT_VALID); }
  if(Pos1 <= Pos0 || Pos2 <= Pos1+1 || Pos3 <= Pos2+1 || Pos3 == Pos4) { return xMakeVec4(NOT_VALID); }

  std::string_view WeightLmView = CmpWeightsView.substr(0     , Pos1       );
  std::string_view WeightCbView = CmpWeightsView.substr(Pos1+1, Pos2-Pos1-1);
  std::string_view WeightCrView = CmpWeightsView.substr(Pos2+1, Pos3-Pos2-1);
  std::string_view WeightXxView = CmpWeightsView.substr(Pos3+1, Pos4-Pos3-1);
  
  //Parsing
  int32 WeightLm = NOT_VALID; std::from_chars(WeightLmView.data(), WeightLmView.data() + WeightLmView.length(), WeightLm);
  int32 WeightCb = NOT_VALID; std::from_chars(WeightCbView.data(), WeightCbView.data() + WeightCbView.length(), WeightCb);
  int32 WeightCr = NOT_VALID; std::from_chars(WeightCrView.data(), WeightCrView.data() + WeightCrView.length(), WeightCr);
  int32 WeightXx = NOT_VALID; std::from_chars(WeightXxView.data(), WeightXxView.data() + WeightXxView.length(), WeightXx);
  
  return { WeightLm, WeightCb, WeightCr, WeightXx };
}
flt32V4 xFmtScn::scanFltWeights(const std::string& CmpWeightsString) //parse vector of 4 integers (format {d}:{d}:{d}:{d})
{
  if (CmpWeightsString.empty() || CmpWeightsString.length() < 7) { return xMakeVec4<flt32>(NOT_VALID); }

  std::string_view CmpWeightsView = CmpWeightsString;
  CmpWeightsView = xString::stripR(xString::stripL(CmpWeightsView));

  if (CmpWeightsView.empty() || CmpWeightsView.length() < 7) { return xMakeVec4<flt32>(NOT_VALID); }

  uintPtr NumX = std::count_if(CmpWeightsView.begin(), CmpWeightsView.end(), [](char i) { return (i == ':'); });
  if (NumX != 3) { return xMakeVec4<flt32>(NOT_VALID); }

  const uintSize Pos0 = 0;
  const uintSize Pos1 = CmpWeightsView.find(':', Pos0  );
  const uintSize Pos2 = CmpWeightsView.find(':', Pos1+1);
  const uintSize Pos3 = CmpWeightsView.find(':', Pos2+1);
  const uintSize Pos4 = CmpWeightsView.length();

  if(Pos1 == std::string_view::npos || Pos2 == std::string_view::npos || Pos3 == std::string_view::npos) { return xMakeVec4<flt32>(NOT_VALID); }
  if(Pos1 <= Pos0 || Pos2 <= Pos1+1 || Pos3 <= Pos2+1 || Pos3 == Pos4) { return xMakeVec4<flt32>(NOT_VALID); }

  std::string_view WeightLmView = CmpWeightsView.substr(0     , Pos1       );
  std::string_view WeightCbView = CmpWeightsView.substr(Pos1+1, Pos2-Pos1-1);
  std::string_view WeightCrView = CmpWeightsView.substr(Pos2+1, Pos3-Pos2-1);
  std::string_view WeightXxView = CmpWeightsView.substr(Pos3+1, Pos4-Pos3-1);
  
  //Parsing
#if defined(_MSC_VER)
  flt32 WeightLm = NOT_VALID; std::from_chars(WeightLmView.data(), WeightLmView.data() + WeightLmView.length(), WeightLm);
  flt32 WeightCb = NOT_VALID; std::from_chars(WeightCbView.data(), WeightCbView.data() + WeightCbView.length(), WeightCb);
  flt32 WeightCr = NOT_VALID; std::from_chars(WeightCrView.data(), WeightCrView.data() + WeightCrView.length(), WeightCr);
  flt32 WeightXx = NOT_VALID; std::from_chars(WeightXxView.data(), WeightXxView.data() + WeightXxView.length(), WeightXx);
#else //don't use std::from_chars for float, since gcc (or rather libstdc++) is a bit retarded in this matter
  flt32 WeightLm = std::stof(std::string(WeightLmView));
  flt32 WeightCb = std::stof(std::string(WeightCbView));
  flt32 WeightCr = std::stof(std::string(WeightCrView));
  flt32 WeightXx = std::stof(std::string(WeightXxView));
#endif
  
  return { WeightLm, WeightCb, WeightCr, WeightXx };
}

//===============================================================================================================================================================================================================

xFileNameScn::tValRes xFileNameScn::validateFileParams(const std::string& FilePath, int32V2 PictureSize, int32 BitDepth, int32 ChromaFormat)
{
  const auto [DetPS, ResPS] = determineResolutionFromFilePath  (FilePath);
  const auto [DetBD, ResBD] = determineBitDepthFromFilePath    (FilePath);
  const auto [DetCF, ResCF] = determineChromaFormatFromFilePath(FilePath); 

  bool        Correct = true;
  std::string Message = "";

  if(ResPS == eResult::Confident && DetPS != PictureSize)
  {
    Message += fmt::format("Detected PictureSize={}x{} does not mach declared PictureSize={}x{} in file FilePath={}\n", DetPS[0], DetPS[1], PictureSize[0], PictureSize[1], FilePath);
    Correct = false;
  }

  if(ResBD == eResult::Confident && DetBD != BitDepth)
  {
    Message += fmt::format("Detected BitDepth={} does not mach declared BitDepth={} in file FilePath={}\n", DetBD, BitDepth, FilePath);
    Correct = false;
  }

  if(ResCF == eResult::Confident && DetCF != ChromaFormat)
  {
    Message += fmt::format("Detected ChromaFormat={} does not mach declared ChromaFormat={} in file FilePath={}\n", DetCF, ChromaFormat, FilePath);
    Correct = false;
  }

  return { Correct, Message };
}
xFileNameScn::tResI32V2 xFileNameScn::determineResolutionFromFilePath(const std::string& FilePath)
{
  std::string FileName   = std::filesystem::path(FilePath).filename().string();
  tResI32V2   FileResult = determineResolutionFromString(FileName);
  if(std::get<1>(FileResult) == eResult::Confident) { return FileResult; }

  std::string ParrentPath   = std::filesystem::path(FilePath).parent_path().string();
  tResI32V2   ParrentResult = determineResolutionFromString(ParrentPath);
  if(std::get<1>(ParrentResult) == eResult::Confident) { return ParrentResult; }

  return std::make_tuple(xMakeVec2(NOT_VALID), eResult::Unknown);
}
xFileNameScn::tResI32 xFileNameScn::determineChromaFormatFromFilePath(const std::string& FilePath)
{
  std::string FileName   = std::filesystem::path(FilePath).filename().string();
  tResI32     FileResult = determineChromaFormatFromString(FileName);
  if(std::get<1>(FileResult) == eResult::Confident) { return FileResult; }

  std::string ParrentPath   = std::filesystem::path(FilePath).parent_path().string();
  tResI32     ParrentResult = determineChromaFormatFromString(ParrentPath);
  if(std::get<1>(ParrentResult) == eResult::Confident) { return ParrentResult; }

  if(std::get<1>(FileResult   ) == eResult::Probable) { return FileResult   ; }
  if(std::get<1>(ParrentResult) == eResult::Probable) { return ParrentResult; }

  return std::make_tuple(NOT_VALID, eResult::Unknown);
}
xFileNameScn::tResI32 xFileNameScn::determineBitDepthFromFilePath(const std::string& FilePath)
{
  std::string FileName   = std::filesystem::path(FilePath).filename().string();
  tResI32     FileResult = determineBitDepthFromString(FileName);
  if(std::get<1>(FileResult) == eResult::Confident) { return FileResult; }

  std::string ParrentPath   = std::filesystem::path(FilePath).parent_path().string();
  tResI32     ParrentResult = determineBitDepthFromString(ParrentPath);
  if(std::get<1>(ParrentResult) == eResult::Confident) { return ParrentResult; }

  if(std::get<1>(FileResult   ) == eResult::Probable) { return FileResult   ; }
  if(std::get<1>(ParrentResult) == eResult::Probable) { return ParrentResult; }

  return std::make_tuple(NOT_VALID, eResult::Unknown);
}

xFileNameScn::tResI32V2 xFileNameScn::determineResolutionFromString(const std::string& FileName)
{
  tResI32V2 Result = std::make_tuple(xMakeVec2(NOT_VALID), eResult::Unknown);
  if(FileName.empty() || FileName.length()<3) return Result;

  //seeking resolution pattern - looking for "x" in filename
  int32 Len = (int32)FileName.length();

  for(int32 Pos=Len-1; Pos>0; Pos--)
  {		
    if(FileName[Pos] == 'x' || FileName[Pos] == 'X')
    {
      int32 Beg = Pos;
      int32 End = Pos;

      //Looking for beginning and end of resolution pattern
      for(  ; Beg>=0;  Beg--) if(!isdigit(FileName[Beg-1])) break;
      for(  ; End<Len; End++) if(!isdigit(FileName[End+1])) break;

      //Test
      if(Beg == Pos || End == Pos) continue;

      //Parsing
      const std::string ResolutionStr = FileName.substr(Beg, End - Beg + 1);
      int32V2 Resolution = xFmtScn::scanResolution(ResolutionStr);
      if(Resolution[0]!=NOT_VALID && Resolution[1]!=NOT_VALID)
      {
        Result = std::make_tuple(Resolution, eResult::Confident);
        return Result;
      }
    }		
  }
  return Result;
}
xFileNameScn::tResI32 xFileNameScn::determineChromaFormatFromString(const std::string& FileName)
{
  tResI32 Result = std::make_tuple(NOT_VALID, eResult::Unknown);
  if(FileName.empty() || FileName.length() < 3) return Result;
  std::smatch Match;

  {
    std::regex yuvFFFpDEE("yuv(420|422|444)p(9|10|12|14|16)(le|be)");
    bool Found = std::regex_search(FileName, Match, yuvFFFpDEE);
    if(Found)
    {
      int32 ChromaFormat = NOT_VALID; std::from_chars(Match[1].str().c_str(), Match[1].str().c_str() + Match[1].str().length(), ChromaFormat);
      if(ChromaFormat != NOT_VALID) { return std::make_tuple(ChromaFormat, eResult::Confident); }
    }
  }

  {
    std::regex grayDEE("gray(9|10|12|14|16)(le|be)");
    //std::smatch Match;
    bool Found = std::regex_search(FileName, Match, grayDEE);
    if(Found) { return std::make_tuple(400, eResult::Confident); }
  }

  {
    std::regex yuvFFFp("(?:cf|CF)(400|420|422|444)_");
    //std::smatch Match;
    bool Found = std::regex_search(FileName, Match, yuvFFFp);
    if(Found)
    {
      int32 ChromaFormat = NOT_VALID; std::from_chars(Match[1].str().c_str(), Match[1].str().c_str() + Match[1].str().length(), ChromaFormat);
      if(ChromaFormat != NOT_VALID) { return std::make_tuple(ChromaFormat, eResult::Confident); }
    }
  }

  {
    std::regex cfFFF("yuv(420|422|444)p");
    //std::smatch Match;
    bool Found = std::regex_search(FileName, Match, cfFFF);
    if(Found)
    {
      int32 ChromaFormat = NOT_VALID; std::from_chars(Match[1].str().c_str(), Match[1].str().c_str() + Match[1].str().length(), ChromaFormat);
      if(ChromaFormat != NOT_VALID) { return std::make_tuple(ChromaFormat, eResult::Confident); }
    }
  }

  return std::make_tuple(c_DefChromaFormat, eResult::Probable);
}
xFileNameScn::tResI32 xFileNameScn::determineBitDepthFromString(const std::string& FileName)
{
  tResI32 Result = std::make_tuple(NOT_VALID, eResult::Unknown);
  if(FileName.empty() || FileName.length() < 3) return Result;
  std::smatch Match;

  //seeking for FFMPEG`s pixfmt-like token
  {
    std::regex yuvFFFpDEE("yuv(420|422|444)p(9|10|12|14|16)(le|be)");
    bool Found = std::regex_search(FileName, Match, yuvFFFpDEE);
    if(Found)
    {
      int32 BitDepth = NOT_VALID; std::from_chars(Match[2].str().c_str(), Match[2].str().c_str() + Match[2].str().length(), BitDepth);
      if(BitDepth != NOT_VALID) { return std::make_tuple(BitDepth, eResult::Confident); } 
    }
  }

  {
    std::regex grayDEE("gray(9|10|12|14|16)(le|be)");
    bool Found = std::regex_search(FileName, Match, grayDEE);
    if(Found)
    {
      int32 BitDepth = NOT_VALID; std::from_chars(Match[1].str().c_str(), Match[1].str().c_str() + Match[1].str().length(), BitDepth);
      if(BitDepth != NOT_VALID) { return std::make_tuple(BitDepth, eResult::Confident); }
    }
  }

  {
    std::regex yuvFFFpDEE("_([0-9]{1,})bps");
    bool Found = std::regex_search(FileName, Match, yuvFFFpDEE);
    if(Found)
    {
      int32 BitDepth = NOT_VALID; std::from_chars(Match[1].str().c_str(), Match[1].str().c_str() + Match[1].str().length(), BitDepth);
      if(BitDepth != NOT_VALID) { return std::make_tuple(BitDepth, eResult::Confident); }
    }
  }

  {
    std::regex cfFFF("yuv(420|422|444)p");
    bool Found = std::regex_search(FileName, Match, cfFFF);
    if(Found) { return std::make_tuple(8, eResult::Confident); }
  }    

  return std::make_tuple(c_DefBitDepth, eResult::Probable);
}


//===============================================================================================================================================================================================================

} //end of namespace PMBB
