/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVQM.h"
#include "xVec.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// Corresponding Pixel Shift
//===============================================================================================================================================================================================================

class xCorrespPixelShiftPrms
{
public:
  static constexpr bool    c_UseRuntimeCmpWeights = xc_USE_RUNTIME_CMPWEIGHTS;
  static constexpr int32   c_DefaultSearchRange   = 2;
  static constexpr int32V4 c_DefaultCmpWeights    = { 4, 1, 1, 0 };
  static constexpr int32V4 c_EqualCmpWeights      = { 1, 1, 1, 0 };

protected:
  int32   m_SearchRange       = c_DefaultSearchRange;
  int32V4 m_CmpWeightsAverage = c_DefaultCmpWeights;
  int32V4 m_CmpWeightsSearch  = c_DefaultCmpWeights;

public:
  void  setSearchRange      (const int32    SearchRange      ) { m_SearchRange       = SearchRange      ; }
  void  setCmpWeightsSearch (const int32V4& CmpWeightsSearch ) { m_CmpWeightsSearch  = CmpWeightsSearch ; }
  void  setCmpWeightsAverage(const int32V4& CmpWeightsAverage) { m_CmpWeightsAverage = CmpWeightsAverage; }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
