/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefCORE.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

using tSDSSD = std::tuple<int64, uint64>;

class xDistortionSTD
{
public:
  static  int64 CalcSD16 (const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height);
  static uint64 CalcSAD16(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height);
  static uint64 CalcSSD16(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height);
  static tSDSSD CalcSSS16(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height);

  static inline  int64 CalcSD (const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 ) { return CalcSD16 (Tst, Ref, TstStride, RefStride, Width, Height); }
  static inline uint64 CalcSAD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 ) { return CalcSAD16(Tst, Ref, TstStride, RefStride, Width, Height); }
  static inline uint64 CalcSSD(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 ) { return CalcSSD16(Tst, Ref, TstStride, RefStride, Width, Height); }
  static inline tSDSSD CalcSSS(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height, int32 ) { return CalcSSS16(Tst, Ref, TstStride, RefStride, Width, Height); }

  //masked modes
  static  int64 CalcMaskSD (const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Msk, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height);
  static uint64 CalcMaskSSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Msk, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height);
  static tSDSSD CalcMaskSSS(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Msk, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height);

  //weightewd modes
  static  int64 CalcWghtSD (const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Wgh, int32 TstStride, int32 RefStride, int32 WghStride, int32 Width, int32 Height);
  static uint64 CalcWghtSSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Wgh, int32 TstStride, int32 RefStride, int32 WghStride, int32 Width, int32 Height);
  static tSDSSD CalcWghtSSS(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Wgh, int32 TstStride, int32 RefStride, int32 WghStride, int32 Width, int32 Height);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
