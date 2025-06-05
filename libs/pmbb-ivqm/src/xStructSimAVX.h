/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVQM.h"
#include "xStructSimSTD.h"

#if X_SIMD_CAN_USE_AVX

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xStructSimAVX
{
public:
  static flt64 CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 BlockSize, flt64 C1, flt64 C2, bool CalcL)
  {
    if (BlockSize == 8 )
    { 
      return CalcBlckAvg8 (Tst, Ref, StrideT, StrideR, C1, C2, CalcL);
    }
    else if(BlockSize == 16 || BlockSize == 32)
    { 
      return CalcBlckAvgM16(Tst, Ref, StrideT, StrideR, BlockSize, C1, C2, CalcL);
    }
    else 
    { 
      return xStructSimSTD::CalcBlckAvg(Tst, Ref, StrideT, StrideR, BlockSize, C1, C2, CalcL);
    }
  }

  static flt64 CalcBlckAvg8  (const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR,                  flt64 C1, flt64 C2, bool CalcL);
  static flt64 CalcBlckAvgM16(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 BlockSize, flt64 C1, flt64 C2, bool CalcL);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_AVX
