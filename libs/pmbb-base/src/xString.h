/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefPMBB-BASE.h"
#include <string>
#include <sstream>
#include <vector>

namespace PMBB_BASE {

//===============================================================================================================================================================================================================

class xString
{
public:
  using tCSR = const std::string&;

public:
  static inline bool IsAlpha   (int32 c) { return ((c>='a' && c<='z') || (c>='A' && c<='Z')); }  
  static inline bool IsNumeric (int32 c) { return ( c>='0' && c<='9'); }
  static inline bool IsAlphaNum(int32 c) { return (IsAlpha(c) || IsNumeric(c)); }
  static inline bool IsBlank   (int32 c) { return (c==' '  || c=='\a' || c=='\b' || c=='\t' || c=='\f'); }
  static inline bool IsEndl    (int32 c) { return (c=='\n' || c=='\r' || c=='\v'); }
  static inline bool IsSpace   (int32 c) { return (c==' '  || c=='\t' || c=='\n' || c=='\v' || c=='\f' || c=='\r'); }

  static inline std::string toUpper(const std::string Src) { std::string Dst; std::transform(Src.cbegin(), Src.cend(), std::back_inserter(Dst), [](auto C) { return (char)std::toupper(C); }); return Dst; }
  static inline std::string toLower(const std::string Src) { std::string Dst; std::transform(Src.cbegin(), Src.cend(), std::back_inserter(Dst), [](auto C) { return (char)std::tolower(C); }); return Dst; }
  
  static inline std::string toUpper(const std::string_view Src) { std::string Dst; std::transform(Src.cbegin(), Src.cend(), std::back_inserter(Dst), [](auto C) { return (char)std::toupper(C); }); return Dst; }
  static inline std::string toLower(const std::string_view Src) { std::string Dst; std::transform(Src.cbegin(), Src.cend(), std::back_inserter(Dst), [](auto C) { return (char)std::tolower(C); }); return Dst; }


  static std::string replaceFirst(const std::string& Source, const std::string& Token, const std::string& ReplaceTo);
  static std::string replaceLast (const std::string& Source, const std::string& Token, const std::string& ReplaceTo);
  static std::string replaceAll  (const std::string& Source, const std::string& Token, const std::string& ReplaceTo);

  static inline std::string_view stripL(std::string_view S) { while(S.length() > 0 && IsSpace(S.front())) { S.remove_prefix(1); } return S; }
  static inline std::string_view stripR(std::string_view S) { while(S.length() > 0 && IsSpace(S.back ())) { S.remove_suffix(1); } return S; }  

  static inline void trimL(std::string& s) { s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {  return !IsSpace(ch); })); }
  static inline void trimR(std::string& s) { s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !IsSpace(ch); }).base(), s.end()); }

  static std::vector<std::string> split(const std::string& String, const char Delimiter);

  template <class XXX> static XXX StrToXXX(tCSR Str, XXX Default)
  {
    if(Str.length() > 0) { std::istringstream InStringStream(Str, std::istringstream::in); InStringStream >> Default; }
    return Default;
  }
  template <class XXX> static std::vector<XXX> VecOfStringToVecOfXXX(const std::vector<std::string>& VecStr, XXX Default)
  {
    std::vector<XXX> VecXXX;
    std::transform(VecStr.cbegin(), VecStr.cend(), std::back_inserter(VecXXX), [&](tCSR Str) -> XXX { return StrToXXX<XXX>(Str, Default); });
    return VecXXX;
  }
  template <class XXX> static std::vector<std::string> VecOfXxxToVecOfString(const std::vector<XXX>& VecXXX)
  {
    std::vector<std::string> VecStr;
    std::transform(VecStr.cbegin(), VecStr.cend(), std::back_inserter(VecXXX), [&](XXX X) -> std::string { return std::to_string(X); });
    return VecXXX;
  }


};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

