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

} //end of namespace PMBB

