/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVQM.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xWeightedSpherically
{
protected:
  bool               m_UseWS = false;
  std::vector<flt64> m_EquirectangularWeights;
  flt64              m_DistortionCorrection = 1.0;

public:
  void  initWS(bool UseWS, int32 Width, int32 Height, int32 BitDepth, int32 LonRangeDeg = 360, int32 LatRangeDeg = 180);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB