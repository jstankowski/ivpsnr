/*
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xMiscUtilsBASE.h"

namespace PMBB_BASE {

//===============================================================================================================================================================================================================

std::string xMiscUtilsBASE::formatCompileTimeSetup()
{
  std::string Str;
  Str += "Build components configuration:\n";
  Str += fmt::format("Build-with-EXPERIMENTAL = {:d}\n", X_PMBB_EXPERIMENTAL);
  Str += fmt::format("Build-with-STALLED      = {:d}\n", X_PMBB_STALLED     );
  Str += fmt::format("Build-with-BROKEN       = {:d}\n", X_PMBB_BROKEN      );
  return Str;
}
std::string xMiscUtilsBASE::formatBuildInfo()
{
  std::string Str;
  Str += "Build and target configuration:\n";
  Str += fmt::format("TARGET_OS_NAME   = {}\n", X_PMBB_OPERATING_SYSTEM_NAME);
  Str += fmt::format("TARGET_ARCH_NAME = {}\n", X_PMBB_ARCH_NAME            );
  Str += fmt::format("COMPILER_NAME    = {}\n", X_PMBB_COMPILER_NAME        );
  Str += fmt::format("COMPILER_VERSION = {}\n", X_PMBB_COMPILER_VER         );
  Str += fmt::format("CPP_VERSION      = {}\n", X_PMBB_CPUSPLUS_VER         );
  Str += fmt::format("BUILD_TIME       = {} {}\n", __DATE__, __TIME__       );
  return Str;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
