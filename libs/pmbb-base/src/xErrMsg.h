/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefBASE.h"

namespace PMBB_BASE {

//===============================================================================================================================================================================================================

class xErrMsg
{
public:
  using tCSR = const std::string&;
  using tCSV = const std::string_view;

public:
  static void printError(tCSR ErrorMessage, tCSR  HelpString = std::string());
  static void printError(tCSR ErrorMessage, tCSV& HelpString);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

