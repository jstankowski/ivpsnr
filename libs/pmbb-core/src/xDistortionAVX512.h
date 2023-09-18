/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefPMBB-CORE.h"

#if X_SIMD_CAN_USE_AVX512

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xDistortionAVX512
{
public:
  //SD, SSD
  static  int32 CalcSD (const uint16* restrict Org, const uint16* restrict Dist,                               int32 Area               );
  static  int32 CalcSD (const uint16* restrict Org, const uint16* restrict Dist, int32 OStride, int32 DStride, int32 Width, int32 Height);
  static uint64 CalcSSD(const uint16* restrict Org, const uint16* restrict Dist,                               int32 Area               );
  static uint64 CalcSSD(const uint16* restrict Org, const uint16* restrict Dist, int32 OStride, int32 DStride, int32 Width, int32 Height);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_AVX512
