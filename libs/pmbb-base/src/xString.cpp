/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xString.h"
#include <charconv>

namespace PMBB_BASE {

//===============================================================================================================================================================================================================

std::string xString::replaceFirst(const std::string& Source, const std::string& Token, const std::string& ReplaceTo)
{
  std::string Result = Source;
  uintSize CurrentPos = Result.find(Token);
  if(CurrentPos != std::string::npos) { Result.replace(CurrentPos, Token.length(), ReplaceTo); }
  return Result;
}
std::string xString::replaceLast(const std::string& Source, const std::string& Token, const std::string& ReplaceTo)
{
  std::string Result = Source;
  uintSize CurrentPos = Result.rfind(Token);
  if(CurrentPos != std::string::npos) { Result.replace(CurrentPos, Token.length(), ReplaceTo); }
  return Result;
}
std::string xString::replaceAll(const std::string& Source, const std::string& Token, const std::string& ReplaceTo)
{
  std::string Result = Source;
  uintSize CurrentPos = 0;
  while((CurrentPos = Result.find(Token, CurrentPos)) != std::string::npos)
  {
    Result.replace(CurrentPos, Token.length(), ReplaceTo);
    CurrentPos += ReplaceTo.length();
  }
  return Result;
}

std::vector<std::string> xString::split(const std::string& String, const char Delimiter)
{
  std::vector<std::string> SubStrings;
  size_t Beg = 0, End = 0;
  while((Beg = String.find_first_not_of(Delimiter, End)) != std::string::npos)
  {
    End = String.find(Delimiter, Beg);
    if(End == std::string::npos) { End = String.length(); }
    std::string_view Unit(String.data() + Beg, End - Beg);
    std::string_view Striped = stripL(stripR(Unit));
    SubStrings.push_back(std::string{ Striped });
  }
  return SubStrings;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
