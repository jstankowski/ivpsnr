/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefPMBB-BASE.h"

namespace PMBB_BASE {

//===============================================================================================================================================================================================================

class xFile
{
public:
  static bool  exists(const std::string& FilePath);
  static int64 size  (const std::string& FilePath);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
