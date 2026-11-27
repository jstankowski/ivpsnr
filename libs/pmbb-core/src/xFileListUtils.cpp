/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xFileListUtils.h"
#include "xFile.h"
#include "xErrMsg.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xFileListUtils
//===============================================================================================================================================================================================================
bool xFileListUtils::checkIfFileExists(const std::string& FileNamePattern, int32 FirstFrameHint)
{
  eFmtSts FormatStatus = analyzeFormatString(FileNamePattern);
  if     (FormatStatus == eFmtSts::NonFmt   ) { return xFile::exists(FileNamePattern); }
  else if(FormatStatus == eFmtSts::Malformed) { xErrMsg::printError(fmt::format("ERROR --> FileNamePattern ({}) is malformed", FileNamePattern)); return false; }

  int32 DetectBeg = FirstFrameHint != NOT_VALID ? FirstFrameHint : 0;
  int32 DetectEnd = DetectBeg + c_DefaultMaxFirstFrame;

  for(int32 i = DetectBeg; i <= DetectEnd; i++)
  {
    std::string FileNameI = formatNamedPattern(FileNamePattern, i, fmt::arg("FrameIdx", i));
    if(!FileNameI.empty() && xFile::exists(FileNameI))
    {
      return true;
    }
  }

  return false;
}
xFileListUtils::eFmtSts xFileListUtils::analyzeFormatString(std::string_view s)
{
  size_t Depth     = 0    ;
  bool   HasFormat = false;

  for(size_t i = 0; i < s.size(); i++)
  {
    if(s[i] == '{')
    {
      if(Depth == 0)
      {
        // Check if it's an escaped literal '{'
        if(i + 1 < s.size() && s[i + 1] == '{') { i++; } // Skip the second '{'
        else{ Depth = 1; HasFormat = true; }
      }
      else { Depth++; } // Inside a format specifier, '{' opens a nested field (e.g., runtime width/precision)
    }
    else if(s[i] == '}')
    {
      if(Depth == 0)
      {
        // Check if it's an escaped literal '}'
        if(i + 1 < s.size() && s[i + 1] == '}') { i++; } // Skip the second '}'
        else { return eFmtSts::Malformed; } // Closing brace with no opening brace -> Malformed
      }
      else { Depth--; } // Closes the current format specifier level
    }
  }

  // If the loop finished but a brace is left unclosed -> Malformed
  if(Depth != 0) { return eFmtSts::Malformed; }

  return HasFormat ? eFmtSts::HasFmt : eFmtSts::NonFmt;
}
xFileListUtils::tIntRes xFileListUtils::detectFirstFrame(const std::string& FileNamePattern, std::function<xInOutResult(const std::string&)>FileVerify, int32 FirstFrameHint)
{
  int32 DetectBeg  = FirstFrameHint != NOT_VALID ? FirstFrameHint : 0;
  int32 DetectEnd  = DetectBeg + c_DefaultMaxFirstFrame;
  int32 StartFrame = NOT_VALID;

  for(int32 i = DetectBeg; i <= DetectEnd; i++)
  {
    std::string FileNameI = formatNamedPattern(FileNamePattern, i, fmt::arg("FrameIdx", i));
    if(!FileNameI.empty() && xFile::exists(FileNameI))
    {
      StartFrame = i;
      if(FileVerify)
      {
        xInOutResult Result = FileVerify(FileNameI);
        if(!Result) { return { NOT_VALID, Result }; }
      }
      break;
    }
  }
  if(StartFrame == NOT_VALID) { return { NOT_VALID, {eRetv::Error, "First Idx=(0 or 1) not found" } }; }
  return { StartFrame, eRetv::Success };
}
xFileListUtils::tIntRes xFileListUtils::detectNumFrames(const std::string& FileNamePattern, int32 FirstFileIdx, int32 MaxNumFiles)
{
  int32 NumFrames = 0;
  for(int32 i = FirstFileIdx; i < MaxNumFiles; i++) //start form 0 or 1 TODO
  {
    std::string FrameFileName = formatNamedPattern(FileNamePattern, i, fmt::arg("FrameIdx", i));
    if(xFile::exists(FrameFileName)) { NumFrames++; }
    else { break; }
  }
  if(NumFrames == 0) { return { NOT_VALID, eRetv::Error   }; }
  else               { return { NumFrames, eRetv::Success }; }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
