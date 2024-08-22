/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xWeightedSpherically.h"
#include "xMathUtils.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xWeightedSpherically::initWS(bool UseWS, int32 /*Width*/, int32 Height, int32 /*BitDepth*/, int32 /*LonRangeDeg*/, int32 LatRangeDeg)
{
  m_UseWS = UseWS;

  if(m_UseWS)
  {
    m_EquirectangularWeights.resize(Height);
    const flt64 EquirectangularHeight = 180.0 * (flt64)Height / (flt64)LatRangeDeg;
    const flt64 EquirectangularOffset = (EquirectangularHeight - Height) / 2.0;
    for(int32 h = 0; h < Height; h++)
    {
      m_EquirectangularWeights[h] = cos((h + EquirectangularOffset - (EquirectangularHeight / 2 - 0.5)) * xc_Pi<flt64> / EquirectangularHeight);
    }
    flt64 SumEquirectangularWeights = xKBNS::Accumulate(m_EquirectangularWeights);
    m_DistortionCorrection = Height / SumEquirectangularWeights;
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB