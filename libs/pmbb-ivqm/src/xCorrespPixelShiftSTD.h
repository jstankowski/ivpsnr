/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVQM.h"
#include "xPic.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xCorrespPixelShiftSTD
{
public:
  static constexpr bool c_UseRuntimeCmpWeights = xc_USE_RUNTIME_CMPWEIGHTS;

  //asymetric Q interleaved
  static uint64V4 CalcDistAsymmetricRow    (const xPicI* Tst, const xPicI* Ref, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
  static int32    FindBestPixelWithinBlock (const int32V4& TstPel, const xPicI* Ref, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights);
  
  static uint64V4 CalcDistAsymmetricRowM   (const xPicI* Tst, const xPicI* Ref, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
  static int32    FindBestPixelWithinBlockM(const int32V4& TstPel, const xPicI* Ref, const xPicP* Msk, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB