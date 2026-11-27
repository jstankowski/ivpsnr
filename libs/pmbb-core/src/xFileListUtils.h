/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefCORE.h"
#include "xInOutResult.h"
#include "xErrMsg.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xFileListUtils
{
public:
  static constexpr int32 c_DefaultMaxNumFiles   = std::numeric_limits<int32>::max() - 100;
  static constexpr int32 c_DefaultMaxFirstFrame = 1000;

  enum class eFmtSts
  {
    UNKNOWN  ,
    HasFmt   ,
    NonFmt   ,
    Malformed,
  };

  using  tIntRes = std::pair<int32, xInOutResult>;

  static bool checkIfFileExists(const std::string& FileNamePattern, int32 FirstFrameHint = NOT_VALID);

  static eFmtSts analyzeFormatString(std::string_view s);
  static tIntRes detectFirstFrame(const std::string& FileNamePattern, std::function<xInOutResult(const std::string&)>FileVerify, int32 FirstFrameHint = NOT_VALID);
  static tIntRes detectNumFrames(const std::string& FileNamePattern, int32 FirstFileIdx, int32 MaxNumFiles);

  static inline std::string formatFileName(const std::string& FileNamePattern, int32 FileIdx) { return fmt::format(fmt::runtime(FileNamePattern), FileIdx); }

  template<typename... Args>
  static inline std::string formatNamedPattern(const std::string& FileNamePattern, Args&&... NamedArgs)
  {
    try { return fmt::format(fmt::runtime(FileNamePattern), std::forward<Args>(NamedArgs)...); }
    catch(const fmt::format_error& Err) { xErrMsg::printError(fmt::format("ERROR --> FileNamePattern ({}) is malformed = {}", FileNamePattern, Err.what())); return ""; }
  }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
