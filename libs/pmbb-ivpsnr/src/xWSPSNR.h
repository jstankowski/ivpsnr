/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xPSNR.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xWSPSNR : public xPSNR
{
protected:
  bool                m_UseWS                        = false;
  bool                m_LegacyPeakValue8bitEmulation = false;
  std::vector<flt64 > m_EquirectangularWeights;
  flt64               m_DistortionCorrection = 1.0;

  std::vector<uint64> m_RowDistortions[4];
  std::vector<flt64 > m_RowErrors     [4];

public:
  void  init           (int32 Height);
  void  setLegacyWS8bit(bool LegacyPeakValue8bitEmulation) { m_LegacyPeakValue8bitEmulation = LegacyPeakValue8bitEmulation; }
  void  initWS         (bool UseWS, int32 Width, int32 Height, int32 BitDepth, int32 LonRangeDeg = 360, int32 LatRangeDeg = 180);

  tRes4 calcPicWSPSNR  (const xPicP* Tst, const xPicP* Ref                  );
  tRes4 calcPicWSPSNRM (const xPicP* Tst, const xPicP* Ref, const xPicP* Msk);

protected:
  tRes1 calcCmpWSPSNR (const xPicP* Tst, const xPicP* Ref,                                             eCmp CmpId);
  tRes1 calcCmpWSPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32 NumNonMasked, eCmp CmpId);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB