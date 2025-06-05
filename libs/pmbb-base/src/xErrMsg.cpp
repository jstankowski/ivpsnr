/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xErrMsg.h"

namespace PMBB_BASE {

//===============================================================================================================================================================================================================

void xErrMsg::printError(tCSR ErrorMessage, tCSR HelpString)
{
  fmt::fprintf(stdout, ErrorMessage + "\n");
  fmt::fprintf(stderr, ErrorMessage + "\n");
  if(!HelpString.empty()) { fmt::fprintf(stdout, HelpString + "\n"); }
  std::fflush(stdout);
  std::fflush(stderr);
}
void xErrMsg::printError(tCSR ErrorMessage, tCSV& HelpString)
{
  fmt::fprintf(stdout, ErrorMessage + "\n");
  fmt::fprintf(stderr, ErrorMessage + "\n");
  if(!HelpString.empty()) { fmt::fprintf(stdout, "%s\n", HelpString); }
  std::fflush(stdout);
  std::fflush(stderr);
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
