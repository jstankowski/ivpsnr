/*
    SPDX-FileCopyrightText: 2024-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefIVQM.h"
#include "xPic.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xTestUtilsIVQM
{
public:
  static void addConst(uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 Value);

  static uint32 addBlockNoise(uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, uint32 State);
  static uint64V4 calcPicSSD(const xPicP* Tst, const xPicP* Ref);
  static uint64V4 calcPicSSD(const xPicI* Tst, const xPicI* Ref);
};

//===============================================================================================================================================================================================================

using fCalcDistAsymmetricRowP = std::function<uint64V4(const xPicP*, const xPicP*, const int32, const int32V4&, const int32, const int32V4&)>;
using pCalcDistAsymmetricRowP = uint64V4(*)(const xPicP*, const xPicP*, const int32, const int32V4&, const int32, const int32V4&);

uint64V4 xTestCalcDistAsymmetricPicP(const xPicP* Tst, const xPicP* OrgP, const int32V4& GlobalColorDiff, int32 SearchRange, const int32V4& CmpWeightsSearch, fCalcDistAsymmetricRowP CalcDistAsymmetricRow);

using fCalcDistAsymmetricRowI = std::function<uint64V4(const xPicI*, const xPicI*, const int32, const int32V4&, const int32, const int32V4&)>;
using pCalcDistAsymmetricRowI = uint64V4(*)(const xPicI*, const xPicI*, const int32, const int32V4&, const int32, const int32V4&);

uint64V4 xTestCalcDistAsymmetricPicI(const xPicI* Tst, const xPicI* OrgP, const int32V4& GlobalColorDiff, int32 SearchRange, const int32V4& CmpWeightsSearch, fCalcDistAsymmetricRowI CalcDistAsymmetricRow);

using fGenShftCompRowP = std::function<void(xPicP*, const xPicP*, const xPicP*, const int32, const int32V4&, const int32, const int32V4&)>;
using pGenShftCompRowP = void(*)(xPicP*, const xPicP*, const xPicP*, const int32, const int32V4&, const int32, const int32V4&);

void xTestGenShftCompPicP(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, fGenShftCompRowP GenShftCompRow);

using fGenShftCompRowI = std::function<void(xPicI*, const xPicI*, const xPicI*, const int32, const int32V4&, const int32, const int32V4&)>;
using pGenShftCompRowI = void(*)(xPicI*, const xPicI*, const xPicI*, const int32, const int32V4&, const int32, const int32V4&);

void xTestGenShftCompPicI(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, fGenShftCompRowI GenShftCompRow);

//===============================================================================================================================================================================================================

} //end of namespace PMBB