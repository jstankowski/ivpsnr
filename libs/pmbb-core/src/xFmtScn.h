/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefPMBB-CORE.h"
#include "xVec.h"
#include <string>
#include <vector>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xFmtScn
{
public:
  static int32V2 scanResolution(const std::string& ResolutionString); //parse resolution (format {d}x{d} or {d}X{d})
  static int32V4 scanIntWeights(const std::string& CmpWeightsString); //parse vector of 4 nonnegative integers (format {d}:{d}:{d}:{d}), returns {-1, -1, -1, -1} on failure
  static flt32V4 scanFltWeights(const std::string& CmpWeightsString); //parse vector of 4 nonnegative floats   (format {f}:{f}:{f}:{f}), returns {-1, -1, -1, -1} on failure

  static std::string formatResolution(const int32V2 Resolution) { return fmt::sprintf("%dx%d", Resolution.getX(), Resolution.getY()); }
  static std::string formatIntWeights(const int32V4 CmpWeights) { return fmt::sprintf("%d:%d:%d:%d", CmpWeights[0], CmpWeights[1], CmpWeights[2], CmpWeights[3]); }
  static std::string formatFltWeights(const flt32V4 CmpWeights) { return fmt::sprintf("%.2f:%.2f:%.2f:%.2f", CmpWeights[0], CmpWeights[1], CmpWeights[2], CmpWeights[3]); }
};

//===============================================================================================================================================================================================================

class xFileNameScn
{
public:
  static constexpr int32 c_DefChromaFormat = 420;
  static constexpr int32 c_DefBitDepth     = 8;

  //results
  enum class eResult : int32
  {
    Confident,
    Probable,
    Unknown
  };
  using tResI32V2 = std::tuple<int32V2, eResult>;
  using tResI32   = std::tuple<int32  , eResult>;
  using tValRes   = std::tuple<bool, std::string>;

  static tValRes   validateFileParams(const std::string& FilePath, int32V2 PictureSize, int32 BitDepth, int32 ChromaFormat);

  static tResI32V2 determineResolutionFromFilePath  (const std::string& FilePath);
  static tResI32   determineChromaFormatFromFilePath(const std::string& FilePath);
  static tResI32   determineBitDepthFromFilePath    (const std::string& FilePath);

  static tResI32V2 determineResolutionFromString  (const std::string& FileName);
  static tResI32   determineChromaFormatFromString(const std::string& FileName);
  static tResI32   determineBitDepthFromString    (const std::string& FileName);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

