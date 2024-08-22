/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xPSNR.h"
#include "xWeightedSpherically.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xWSPSNR : public xPSNR, public xWeightedSpherically
{
protected:
  bool m_LegacyPeakValue8bitEmulation = false;

public:
  void  setLegacyWS8bit(bool LegacyPeakValue8bitEmulation) { m_LegacyPeakValue8bitEmulation = LegacyPeakValue8bitEmulation; }

  flt64V4 calcPicWSPSNR  (const xPicP* Tst, const xPicP* Ref);
  flt64V4 calcPicWSPSNRM (const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, int32 NumNonMasked = NOT_VALID);

protected:
  flt64 xCalcCmpWSPSNR (const xPicP* Tst, const xPicP* Ref,                                             eCmp CmpId);
  flt64 xCalcCmpWSPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32 NumNonMasked, eCmp CmpId);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB