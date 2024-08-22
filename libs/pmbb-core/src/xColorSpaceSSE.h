/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefCORE.h"

#if X_SIMD_CAN_USE_SSE

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xColorSpaceSSE
{
public:
  static void ConvertRGB2YCbCr_I32(uint16* restrict Y, uint16* restrict U, uint16* restrict V, const uint16* R, const uint16* G, const uint16* B, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc);
  static void ConvertYCbCr2RGB_I32(uint16* restrict R, uint16* restrict G, uint16* restrict B, const uint16* Y, const uint16* U, const uint16* V, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_SSE
