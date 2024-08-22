/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVQM.h"
#include "xStructSimConsts.h"
#include "xVec.h"
#include <array>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

template <class fltTP> class xStructSim : public xStructSimConsts //Structural Similarity
{
public:
  static constexpr fltTP c_InvFltrArea  = (fltTP)1.0 / (fltTP)c_FilterArea;
  using fltTPV4 = xVec4<fltTP>;

public:
  static fltTP CalcPel(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, const tFltrF& Filter, fltTP C1, fltTP C2); //uses gaussian filter
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#ifndef PMBB_STRUCTSIM_IMPLEMENTATION
extern template class xStructSim<flt32>;
extern template class xStructSim<flt64>;
#endif // !PMBB_STRUCTSIM_IMPLEMENTATION

//===============================================================================================================================================================================================================

} //end of namespace PMBB