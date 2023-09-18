/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xFile.h"
#include <filesystem>

namespace PMBB_BASE {

//===============================================================================================================================================================================================================
// xFile
//===============================================================================================================================================================================================================

bool xFile::exists(const std::string& FilePath)
{
  std::error_code EC;
  bool Exists = std::filesystem::exists(FilePath, EC);
  if(EC) { fmt::printf("ERROR " + EC.message()); return false; }
  return Exists;
}
int64 xFile::size(const std::string& FilePath)
{
  std::error_code EC;
  uint64 FileSize = std::filesystem::file_size(FilePath, EC);
  if(EC) { fmt::printf("ERROR " + EC.message()); return NOT_VALID; }
  return (int64)FileSize;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
