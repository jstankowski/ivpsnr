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

//===============================================================================================================================================================================================================

} //end of namespace PMBB
