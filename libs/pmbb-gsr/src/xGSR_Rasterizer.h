/*
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefGSR.h"
#include "xProjectedCloud.h"

//portable implementation
#include "xGSR_RasterizerSTD.h"

//AVX implementation
#if X_SIMD_CAN_USE_AVX && __has_include("xGSR_RasterizerAVX.h")
#define X_CAN_USE_AVX 1
#include "xGSR_RasterizerAVX.h"
#else
#define X_CAN_USE_AVX 0
#endif

//AVX512 implementation
#if X_SIMD_CAN_USE_AVX512 && __has_include("xGSR_RasterizerAVX512.h")
#define X_CAN_USE_AVX512 1
#include "xGSR_RasterizerAVX512.h"
#else
#define X_CAN_USE_AVX512 0
#endif

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================

class xRasterizer
{
public:
#if X_CAN_USE_AVX512
  static inline void RasterizeTile(xPicP* DstPic, tPicM* DstMsk, const xProjectedCloud& Projected, const tSplIdxV& SplatIdxsTile, int32V2 TileIdxXY, tTileBuff* TileBuff) { xRasterizerAVX512::RasterizeTile(DstPic, DstMsk, Projected, SplatIdxsTile, TileIdxXY, TileBuff); }
#elif X_CAN_USE_AVX
  static inline void RasterizeTile(xPicP* DstPic, tPicM* DstMsk, const xProjectedCloud& Projected, const tSplIdxV& SplatIdxsTile, int32V2 TileIdxXY, tTileBuff* TileBuff) { xRasterizerAVX   ::RasterizeTile(DstPic, DstMsk, Projected, SplatIdxsTile, TileIdxXY, TileBuff); }
#else
  static inline void RasterizeTile(xPicP* DstPic, tPicM* DstMsk, const xProjectedCloud& Projected, const tSplIdxV& SplatIdxsTile, int32V2 TileIdxXY, tTileBuff* TileBuff) { xRasterizerSTD   ::RasterizeTile(DstPic, DstMsk, Projected, SplatIdxsTile, TileIdxXY, TileBuff); }
#endif

  static inline void RasterizePic(tRstBuf* DstPic, const xProjectedCloud& Projected) { xRasterizerSTD::RasterizePic(DstPic, Projected); }
};

//===============================================================================================================================================================================================================

#undef X_CAN_USE_AVX
#undef X_CAN_USE_AVX512

} //end of namespace PMBB_NAMESPACE::GSR

