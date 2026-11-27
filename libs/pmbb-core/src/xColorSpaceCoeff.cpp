/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define PMBB_xColorSpaceCoeff_IMPLEMENTATION
#include "xColorSpaceCoeff.h"
#include "xMemory.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xColorSpaceCoeffCommon
//===============================================================================================================================================================================================================
std::string xColorSpaceCoeffCommon::formatColorSpaceCoeff(tMatCoeffsF32 Coeff, const std::vector<std::string>& SrcCmp, const std::vector<std::string>& DstCmp)
{
  std::string Result; Result.reserve(xMemory::getBestEffortSizePageBase());
  for(int32 i = 0; i < 3; i++)
  {
    for(int32 j = 0; j < 3; j++)
    {
      Result += fmt::format("{:2s}({:2s}) = {:< 10f}   ", DstCmp[i], SrcCmp[j], Coeff[i][j]);
    }
    Result += "\n";
  }
  return Result;
}
std::string xColorSpaceCoeffCommon::formatColorSpaceCoeff(tMatCoeffsI32 Coeff, const std::vector<std::string>& SrcCmp, const std::vector<std::string>& DstCmp)
{
  std::string Result; Result.reserve(xMemory::getBestEffortSizePageBase());
  for(int32 i = 0; i < 3; i++)
  {
    for(int32 j = 0; j < 3; j++)
    {
      Result += fmt::format("{:2s}({:2s}) = {:< 8d}   ", DstCmp[i], SrcCmp[j], Coeff[i][j]);
    }
    Result += "\n";
  }
  return Result;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
