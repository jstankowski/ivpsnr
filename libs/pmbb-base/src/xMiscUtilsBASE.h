/*
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefBASE.h"

namespace PMBB_BASE {

//===============================================================================================================================================================================================================

class xMiscUtilsBASE
{
public:
  static std::string formatCompileTimeSetup();
  static std::string formatBuildInfo       ();
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

