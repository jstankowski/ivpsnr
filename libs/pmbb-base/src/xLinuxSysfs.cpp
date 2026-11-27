/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xLinuxSysfs.h"
#include <charconv>
#include <filesystem>
#include <fstream>

#ifdef X_PMBB_OPERATING_SYSTEM_LINUX

namespace PMBB_BASE {

//===============================================================================================================================================================================================================

xLinuxSysfs::int32V xLinuxSysfs::xParseKernelList(const std::string& KernelList, const int32_t NumUnits)
{
  std::vector<int32_t> Result; Result.reserve(NumUnits);

  std::string_view KernelListView(KernelList.c_str(), KernelList.size());

  size_t Beg = 0, End = 0;
  while((Beg = KernelListView.find_first_not_of(',', End)) != std::string::npos)
  {
    End = KernelListView.find(',', Beg);
    std::string_view SubString = KernelListView.substr(Beg, End - Beg);
    size_t HyphenPos = SubString.find_first_of('-');
    if(HyphenPos == std::string::npos)
    {
      int32_t Value = 0;
      std::from_chars(SubString.data(), SubString.data() + SubString.size(), Value);
      Result.push_back(Value);
    }
    else
    {
      int32_t First = 0, Last = 0;
      std::from_chars(SubString.data(), SubString.data() + HyphenPos, First);
      std::from_chars(SubString.data() + HyphenPos + 1, SubString.data() + SubString.size(), Last);
      for(int32_t Value = First; Value <= Last; Value++) { Result.push_back(Value); }
    }
  }

  return Result;
}
xLinuxSysfs::int32V xLinuxSysfs::xReadListFromSysFsFile(const std::string& FilePath)
{
  std::ifstream FileStream = std::ifstream(FilePath);
  if(!FileStream.is_open() || !FileStream.good()) { return int32V(); }

  std::string FileContent;
  std::getline(std::ifstream(FilePath), FileContent, '\0');
  if(FileContent.empty()) { return int32V(); }

  return xParseKernelList(FileContent);
}
int32_t xLinuxSysfs::xReadIntFromSysFsFile(const std::string& FilePath, int32_t FallbackValue)
{ 
  std::ifstream FileStream = std::ifstream(FilePath);
  if(!FileStream.is_open() || !FileStream.good()) { return FallbackValue; }

  std::string FileContent;
  std::getline(FileStream, FileContent, '\0');
  if(FileContent.empty()) { return FallbackValue; }

  int32_t Value = 0;
  std::from_chars_result Result = std::from_chars(FileContent.data(), FileContent.data() + FileContent.size(), Value);
  if(Result.ec == std::errc()) { return Value; }
  return FallbackValue;
}
uint64_t xLinuxSysfs::xReadHex64FromSysFsFile(const std::string& FilePath, uint64_t FallbackValue)
{
  std::ifstream FileStream = std::ifstream(FilePath);
  if(!FileStream.is_open() || !FileStream.good()) { return FallbackValue; }

  std::string FileContent;
  std::getline(FileStream, FileContent, '\0');
  if(FileContent.empty()) { return FallbackValue; }

  uint64_t Value = 0;
  std::string FileContent16 = FileContent.substr(2);
  std::from_chars_result Result = std::from_chars(FileContent16.data(), FileContent16.data() + FileContent16.size(), Value, 16);
  if(Result.ec == std::errc()) { return Value; }
  return FallbackValue;
}
bool xLinuxSysfs::xFileExists(const std::string& FilePath)
{
  std::error_code EC;
  bool Exists = std::filesystem::exists(FilePath, EC);
  if(EC) { fmt::print("ERROR {}", EC.message()); return false; }
  return Exists;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_PMBB_OPERATING_SYSTEM_LINUX

