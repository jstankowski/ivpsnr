/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xMiscUtilsPMBB-CORE.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

std::string xMiscUtilsCORE::formatCompileTimeSetup()
{
  std::string Str;
  Str += "Compile-time configuration:\n";
  Str += fmt::format("USE_SIMD               = {:d}\n", USE_SIMD);
  if(USE_SIMD)
  {
    Str += fmt::format("SIMD_CAN_USE_SSE       = {:d}\n", X_SIMD_CAN_USE_SSE);
    Str += fmt::format("SIMD_CAN_USE_AVX       = {:d}\n", X_SIMD_CAN_USE_AVX);
    Str += fmt::format("SIMD_CAN_USE_AVX512    = {:d}\n", X_SIMD_CAN_USE_AVX512);
  }
  return Str;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
