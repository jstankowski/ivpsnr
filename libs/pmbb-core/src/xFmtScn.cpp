/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xFmtScn.h"
#include "xString.h"
#include <charconv>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

int32V2 xFmtScn::scanResolution(const std::string& ResolutionString)
{
  if(ResolutionString.empty() || ResolutionString.length() < 3) {return { NOT_VALID, NOT_VALID }; }

  std::string_view ResolutionView = ResolutionString;
  ResolutionView = xString::stripR(xString::stripL(ResolutionView));

  if(ResolutionView.empty() || ResolutionView.length() < 3) {return { NOT_VALID, NOT_VALID }; }

  uintPtr NumX = std::count_if(ResolutionView.begin(), ResolutionView.end(), [](char i) { return (i == 'x' || i == 'X'); });
  if(NumX != 1) { return { NOT_VALID, NOT_VALID }; }

  const uintSize Pos = xMin(ResolutionView.find('x'), ResolutionView.find('X'));
  const uintSize Len = ResolutionView.length();
  if(Pos == 0 || Pos == Len-1) { return { NOT_VALID, NOT_VALID }; }

  std::string_view WidthView  = ResolutionView.substr(0, Pos);
  std::string_view HeightView = ResolutionView.substr(Pos+1);
  
  //Parsing
  int32 Width  = NOT_VALID;
  int32 Height = NOT_VALID;
  std::from_chars(WidthView .data(), WidthView .data() + WidthView .length(), Width );
  std::from_chars(HeightView.data(), HeightView.data() + HeightView.length(), Height);

  return { Width, Height };
}
int32V4 xFmtScn::scanIntWeights(const std::string& CmpWeightsString) //parse vector of 4 nonnegative integers (format {d}:{d}:{d}:{d}), returns {-1, -1, -1, -1} on failure
{
  if (CmpWeightsString.empty() || CmpWeightsString.length() < 7) { return xMakeVec4(NOT_VALID); }

  std::string_view CmpWeightsView = CmpWeightsString;
  CmpWeightsView = xString::stripR(xString::stripL(CmpWeightsView));

  if (CmpWeightsView.empty() || CmpWeightsView.length() < 7) { return xMakeVec4(NOT_VALID); }

  uintPtr NumX = std::count_if(CmpWeightsView.begin(), CmpWeightsView.end(), [](char i) { return (i == ':'); });
  if (NumX != 3) { return xMakeVec4(NOT_VALID); }

  const uintSize Pos0 = 0;
  const uintSize Pos1 = CmpWeightsView.find(':', Pos0  );
  const uintSize Pos2 = CmpWeightsView.find(':', Pos1+1);
  const uintSize Pos3 = CmpWeightsView.find(':', Pos2+1);
  const uintSize Pos4 = CmpWeightsView.length();

  if(Pos1 == std::string_view::npos || Pos2 == std::string_view::npos || Pos3 == std::string_view::npos) { return xMakeVec4(NOT_VALID); }
  if(Pos1 <= Pos0 || Pos2 <= Pos1+1 || Pos3 <= Pos2+1 || Pos3 == Pos4) { return xMakeVec4(NOT_VALID); }

  std::string_view WeightLmView = CmpWeightsView.substr(0     , Pos1       );
  std::string_view WeightCbView = CmpWeightsView.substr(Pos1+1, Pos2-Pos1-1);
  std::string_view WeightCrView = CmpWeightsView.substr(Pos2+1, Pos3-Pos2-1);
  std::string_view WeightXxView = CmpWeightsView.substr(Pos3+1, Pos4-Pos3-1);
  
  //Parsing
  int32 WeightLm = NOT_VALID; std::from_chars(WeightLmView.data(), WeightLmView.data() + WeightLmView.length(), WeightLm);
  int32 WeightCb = NOT_VALID; std::from_chars(WeightCbView.data(), WeightCbView.data() + WeightCbView.length(), WeightCb);
  int32 WeightCr = NOT_VALID; std::from_chars(WeightCrView.data(), WeightCrView.data() + WeightCrView.length(), WeightCr);
  int32 WeightXx = NOT_VALID; std::from_chars(WeightXxView.data(), WeightXxView.data() + WeightXxView.length(), WeightXx);
  
  return { WeightLm, WeightCb, WeightCr, WeightXx };
}
flt32V4 xFmtScn::scanFltWeights(const std::string& CmpWeightsString) //parse vector of 4 integers (format {d}:{d}:{d}:{d})
{
  if (CmpWeightsString.empty() || CmpWeightsString.length() < 7) { return xMakeVec4<flt32>(NOT_VALID); }

  std::string_view CmpWeightsView = CmpWeightsString;
  CmpWeightsView = xString::stripR(xString::stripL(CmpWeightsView));

  if (CmpWeightsView.empty() || CmpWeightsView.length() < 7) { return xMakeVec4<flt32>(NOT_VALID); }

  uintPtr NumX = std::count_if(CmpWeightsView.begin(), CmpWeightsView.end(), [](char i) { return (i == ':'); });
  if (NumX != 3) { return xMakeVec4<flt32>(NOT_VALID); }

  const uintSize Pos0 = 0;
  const uintSize Pos1 = CmpWeightsView.find(':', Pos0  );
  const uintSize Pos2 = CmpWeightsView.find(':', Pos1+1);
  const uintSize Pos3 = CmpWeightsView.find(':', Pos2+1);
  const uintSize Pos4 = CmpWeightsView.length();

  if(Pos1 == std::string_view::npos || Pos2 == std::string_view::npos || Pos3 == std::string_view::npos) { return xMakeVec4<flt32>(NOT_VALID); }
  if(Pos1 <= Pos0 || Pos2 <= Pos1+1 || Pos3 <= Pos2+1 || Pos3 == Pos4) { return xMakeVec4<flt32>(NOT_VALID); }

  std::string_view WeightLmView = CmpWeightsView.substr(0     , Pos1       );
  std::string_view WeightCbView = CmpWeightsView.substr(Pos1+1, Pos2-Pos1-1);
  std::string_view WeightCrView = CmpWeightsView.substr(Pos2+1, Pos3-Pos2-1);
  std::string_view WeightXxView = CmpWeightsView.substr(Pos3+1, Pos4-Pos3-1);
  
  //Parsing
#if defined(_MSC_VER)
  flt32 WeightLm = NOT_VALID; std::from_chars(WeightLmView.data(), WeightLmView.data() + WeightLmView.length(), WeightLm);
  flt32 WeightCb = NOT_VALID; std::from_chars(WeightCbView.data(), WeightCbView.data() + WeightCbView.length(), WeightCb);
  flt32 WeightCr = NOT_VALID; std::from_chars(WeightCrView.data(), WeightCrView.data() + WeightCrView.length(), WeightCr);
  flt32 WeightXx = NOT_VALID; std::from_chars(WeightXxView.data(), WeightXxView.data() + WeightXxView.length(), WeightXx);
#else //don't use std::from_chars for float, since gcc (or rather libstdc++) is a bit retarded in this matter
  flt32 WeightLm = std::stof(std::string(WeightLmView));
  flt32 WeightCb = std::stof(std::string(WeightCbView));
  flt32 WeightCr = std::stof(std::string(WeightCrView));
  flt32 WeightXx = std::stof(std::string(WeightXxView));
#endif
  
  return { WeightLm, WeightCb, WeightCr, WeightXx };
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
