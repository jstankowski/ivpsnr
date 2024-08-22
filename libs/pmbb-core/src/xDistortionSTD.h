/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefCORE.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xDistortionSTD
{
public:
  static  int32 CalcSD (const uint16* restrict Tst, const uint16* restrict Ref,                                   int32 Area               );
  static  int32 CalcSD (const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height);
  static uint32 CalcSAD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area                                                 );
  static uint32 CalcSAD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height);
  static uint64 CalcSSD(const uint16* restrict Tst, const uint16* restrict Ref,                                   int32 Area               );
  static uint64 CalcSSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height);

  static  int64 CalcWeightedSD (const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Mask,                                                    int32 Area               );
  static  int64 CalcWeightedSD (const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Mask, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height);
  static uint64 CalcWeightedSSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Mask,                                                    int32 Area               );
  static uint64 CalcWeightedSSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Mask, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
