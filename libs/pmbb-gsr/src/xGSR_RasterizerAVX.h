/*
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefGSR.h"
#include "xProjectedCloud.h"

#if X_SIMD_CAN_USE_AVX

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================

static_assert(c_TileWidth >= 8, "xRasterizerAVX requires TileWidth >= 8");

class xRasterizerAVX
{
public:
  static void RasterizeTile(xPicP* DstPic, tPicM* DstMsk, const xProjectedCloud& Projected, const tSplIdxV& SplatIndicesTile, int32V2 TileIdxXY, tTileBuff* restrict TileBuff);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB_NAMESPACE::GSR

#endif //X_SIMD_CAN_USE_AVX
