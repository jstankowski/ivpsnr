/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefBASE.h"
#include <vector>
#include <map>

#ifdef X_PMBB_OPERATING_SYSTEM_LINUX

namespace PMBB_BASE {

//===============================================================================================================================================================================================================

class xLinuxSysfs
{
public:
  using int32V = std::vector<int32_t>;

  static int32V   xParseKernelList(const std::string& KernelList, const int32_t NumUnits = 1);
  static int32V   xReadListFromSysFsFile(const std::string& FilePath);
  static int32_t  xReadIntFromSysFsFile(const std::string& FilePath, int32_t FallbackValue = NOT_VALID);
  static uint64_t xReadHex64FromSysFsFile(const std::string& FilePath, uint64_t FallbackValue = 0);
  static bool     xFileExists(const std::string& FilePath);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_PMBB_OPERATING_SYSTEM_LINUX

