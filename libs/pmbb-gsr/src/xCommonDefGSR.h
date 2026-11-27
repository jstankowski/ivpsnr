/*
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>    
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefCORE.h"
#include "xGaussianCloud.h"
#include "xCameraParams.h"
#include "xBfrPic.h"
#include "xPlane.h"
#include "xString.h"
#include <string_view>
#include <cstdint>

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================
// Compile time settings
//===============================================================================================================================================================================================================
#define X_PMBB_GSR_DISABLE_FP_CONTRACT_IN_PROJECTOR  1
#define X_PMBB_GSR_DISABLE_FP_CONTRACT_IN_RASTERIZER 1
#define X_PMBB_GSR_EXP_BOUND_CHECKING                0
#define X_PMBB_GSR_EXP_USE_FMA                       1
#define X_PMBB_PROJECTOR_FORCE_STD                   1 //built and tested, disabled by default: ~6% measured speedup, not worth the FP-rounding differences vs STD it introduces downstream. Bottleneck is memory layout, not arithmetic.


using tProjInternalFloatType = flt32;

//===============================================================================================================================================================================================================
// Constants
//===============================================================================================================================================================================================================
static constexpr int32 c_Log2TileWidth    = 5; // log2(TileWidth) 
static constexpr int32 c_Log2TileHeight   = 5; // log2(TileHeight)
static constexpr int32 c_TileWidth        = 1<< c_Log2TileWidth ;
static constexpr int32 c_TileHeight       = 1<< c_Log2TileHeight;
static constexpr int32 c_TileArea         = c_TileWidth * c_TileHeight;
static constexpr int32 c_DefProjBatchSize = 512;
//projection/rasterization constants
static constexpr flt32 c_CullSigmas        = 3.33f;         // ellipse radius in sigma units for culling/AABB
static constexpr flt32 c_AlphaThreshold    = 1.0f / 255.0f; // minimum effective alpha to splat
static constexpr flt32 c_PowerThreshold    = -5.545f;       // ln(c_AlphaThreshold); skip exp() when weight * alpha_max < threshold
static constexpr flt32 c_AccAlphaSat       = 0.9999f;       // accumulated alpha saturation (early exit)
static constexpr flt32 c_CovRegularize     = 0.3f;          // regularization added to diagonal of Sigma_2D
static constexpr flt32 c_EffectiveAlphaCap = 0.999f;        // MGM cap effective alpha cap - 0.999
static constexpr flt32 c_PlaneNear         = 0.01f;
static constexpr flt32 c_PlaneFar          = 1.0e10f;
static constexpr flt32 c_DepthOffset       = 0.02f;         // culling and sorting use the clip space z of a frustum projection (zNear = 0.01) = depth - 2 * zNear
//projection/rasterization parameters
static constexpr bool  c_ProjClamping   = true;
static constexpr bool  c_PelCenter0o5   = true;
static constexpr bool  c_TileTerminate  = true; //early tile termination based on c_AccAlphaSat

//===============================================================================================================================================================================================================
// Types
//===============================================================================================================================================================================================================
using tGaussI   = FVGS::xGaussianCloudI;
using tCamPar   = FVGS::xCameraParams<flt64>;
using tRstBuf   = xBfrPicP<flt32, 4>; //rasterizer buffer
using tValidV   = std::vector<uint8>;
using tSplIdxV  = std::vector<int32>;
using tPicM     = xPlane<uint16>;

using tTileBuff   = std::array<std::array<std::array<flt32, c_TileWidth>, c_TileHeight>, 4>; //[RGBA][y][x]
using tTileBuffPV = std::vector<tTileBuff*>;

using tTileSplatIdxV = std::vector<int32>;
using tTileSplatIdxM = std::vector<tTileSplatIdxV>;

//===============================================================================================================================================================================================================
// Utils
//===============================================================================================================================================================================================================
static inline int32 calcNumTiles(int32V2 PictureSize)
{
  const int32 NumTilesX = (PictureSize.getX() + c_TileWidth  - 1) >> c_Log2TileWidth ;
  const int32 NumTilesY = (PictureSize.getY() + c_TileHeight - 1) >> c_Log2TileHeight;
  return NumTilesX * NumTilesY;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB_NAMESPACE::GSR
