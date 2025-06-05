/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVQM.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xStructSimSTD
{
public:
  //Regular Structural Similarity
  static flt64 CalcRglrFlt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL); //uses gaussian window
  static flt64 CalcRglrInt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL); //uses gaussian window
  static flt64 CalcRglrAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL); //uses averaging

  //Regular Structural Similarity - with mask
  static flt64 CalcRglrFltM(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 StrideT, int32 StrideR, int32 StrideM, int32 WndSize, flt64 C1, flt64 C2, bool CalcL); //uses gaussian window - with mask
  static flt64 CalcRglrIntM(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 StrideT, int32 StrideR, int32 StrideM, int32 WndSize, flt64 C1, flt64 C2, bool CalcL); //uses gaussian window - with mask
  static flt64 CalcRglrAvgM(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 StrideT, int32 StrideR, int32 StrideM, int32 WndSize, flt64 C1, flt64 C2, bool CalcL); //uses averaging       - with mask

  //Block Structural Similarity
  static flt64 CalcBlckInt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL); //uses gaussian window
  static flt64 CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL); //uses averaging 
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB