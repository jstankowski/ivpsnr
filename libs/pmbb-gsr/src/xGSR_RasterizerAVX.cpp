/*
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xGSR_RasterizerAVX.h"

#if X_SIMD_CAN_USE_AVX

#if X_PMBB_GSR_DISABLE_FP_CONTRACT_IN_RASTERIZER
X_PMBB_DISABLE_FP_CONTRACT_BEG;
#endif

#include "xExponentialAVX.h"
#include "xHelpersFLT.h"
#include <immintrin.h>
#include <algorithm>

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================

static inline __m256 xExp(__m256 x) { return xExponentialAVX<X_PMBB_GSR_EXP_BOUND_CHECKING, X_PMBB_GSR_EXP_USE_FMA>::SchraudolphExpCorrected(x); }

//===============================================================================================================================================================================================================

void xRasterizerAVX::RasterizeTile(xPicP* DstPic, tPicM* DstMsk, const xProjectedCloud& Projected, const tSplIdxV& SplatIndicesTile, int32V2 TileIdxXY, tTileBuff* restrict TileBuff)
{
  const int32 NumSplats = (int32)SplatIndicesTile.size();
  if(NumSplats == 0) { return; }

  const int32 PicWidth  = DstPic->getWidth ();
  const int32 PicHeight = DstPic->getHeight();

  const int32 PixX0 = TileIdxXY.getX() * c_TileWidth ;
  const int32 PixY0 = TileIdxXY.getY() * c_TileHeight;
  const int32 PixX1 = std::min(PixX0 + c_TileWidth  - 1, PicWidth  - 1);
  const int32 PixY1 = std::min(PixY0 + c_TileHeight - 1, PicHeight - 1);

  //clear local tile cache
  memset(TileBuff, 0, sizeof(tTileBuff));

  const __m256 LaneIdx = _mm256_setr_ps(0, 1, 2, 3, 4, 5, 6, 7);

  //Iterate Gaussians in front-to-back order
  for(int32 i = 0; i < NumSplats; i++)
  {
    const int32 Idx = SplatIndicesTile[i];

    // Intersect Gaussian AABB with tile pixel bounds
    const xProjectedCloud::xBoundingBox& BoundingBox = Projected.getBoundingBox()[Idx];
    const int32 X0 = std::max((int32)BoundingBox.MinX, PixX0);
    const int32 Y0 = std::max((int32)BoundingBox.MinY, PixY0);
    const int32 X1 = std::min((int32)BoundingBox.MaxX, PixX1);
    const int32 Y1 = std::min((int32)BoundingBox.MaxY, PixY1);
    if(X0 > X1 || Y0 > Y1) { continue; }

    // Gather splat data
    const xProjectedCloud::xSplatData& SplatData = Projected.getSplatData()[Idx];
    const flt32 ScreenX = SplatData.ScreenX;
    const flt32 ScreenY = SplatData.ScreenY;
    const flt32 InvCovA = SplatData.InvCovA;
    const flt32 InvCovB = SplatData.InvCovB;
    const flt32 InvCovC = SplatData.InvCovC;
    const flt32 R       = SplatData.R      ;
    const flt32 G       = SplatData.G      ;
    const flt32 B       = SplatData.B      ;
    const flt32 A       = SplatData.A      ;

    for(int32 Py = Y0; Py <= Y1; Py++)
    {
      const int32 TLy    = Py - PixY0;
      const flt32 Dy     = c_PelCenter0o5 ? ((flt32)Py + 0.5f) - ScreenY : (flt32)Py - ScreenY;
      const flt32 icC_Dy = InvCovC * Dy;
      const flt32 icB_Dy = InvCovB * Dy;

      const int32 TLx0 = X0 - PixX0;
      const int32 TLx1 = X1 - PixX0;

      // Chunks are aligned to the tile, not to TLx0, so ChunkStart+7 never exceeds c_TileWidth-1 -
      // a chunk may overhang past [TLx0,TLx1], RangeMask below excludes those lanes instead of a scalar tail.
      const int32 ChunkStart0 = TLx0 & ~7;

      for(int32 x = ChunkStart0; x <= TLx1; x += 8)
      {
        const __m256 TLxVec    = _mm256_add_ps(_mm256_set1_ps((flt32)x), LaneIdx);
        const __m256 RangeMask = _mm256_and_ps(_mm256_cmp_ps(TLxVec, _mm256_set1_ps((flt32)TLx0), _CMP_GE_OQ),
                                                _mm256_cmp_ps(TLxVec, _mm256_set1_ps((flt32)TLx1), _CMP_LE_OQ));

        const __m256 PxVec     = _mm256_add_ps(_mm256_set1_ps((flt32)PixX0), TLxVec);
        const __m256 PelOffset = c_PelCenter0o5 ? _mm256_set1_ps(0.5f) : _mm256_setzero_ps();
        const __m256 Dx        = _mm256_sub_ps(_mm256_add_ps(PxVec, PelOffset), _mm256_set1_ps(ScreenX));

        const __m256 Power1 = _mm256_mul_ps(_mm256_mul_ps(_mm256_set1_ps(InvCovA), Dx), Dx);
        const __m256 Power2 = _mm256_mul_ps(_mm256_set1_ps(2.0f * icB_Dy), Dx);
        const __m256 Power3 = _mm256_set1_ps(icC_Dy * Dy);
        const __m256 Power  = _mm256_mul_ps(_mm256_add_ps(_mm256_add_ps(Power1, Power2), Power3), _mm256_set1_ps(-0.5f));

        const __m256 PowerMask = _mm256_and_ps(_mm256_cmp_ps(Power, _mm256_setzero_ps(), _CMP_LE_OQ),
                                                _mm256_cmp_ps(Power, _mm256_set1_ps(c_PowerThreshold), _CMP_GE_OQ));
        __m256 CombinedMask = _mm256_and_ps(PowerMask, RangeMask);

        const __m256 Weight = xExp(Power);

        __m256 AlphaEff = _mm256_mul_ps(_mm256_set1_ps(A), Weight);
        AlphaEff = _mm256_min_ps(AlphaEff, _mm256_set1_ps(c_EffectiveAlphaCap));
        const __m256 AlphaMask = _mm256_cmp_ps(AlphaEff, _mm256_set1_ps(c_AlphaThreshold), _CMP_GE_OQ);
        CombinedMask = _mm256_and_ps(CombinedMask, AlphaMask);

        flt32* TileRowR = &((*TileBuff)[(int32)eCmp::R][TLy][x]);
        flt32* TileRowG = &((*TileBuff)[(int32)eCmp::G][TLy][x]);
        flt32* TileRowB = &((*TileBuff)[(int32)eCmp::B][TLy][x]);
        flt32* TileRowA = &((*TileBuff)[(int32)eCmp::A][TLy][x]);

        const __m256 AccA      = _mm256_loadu_ps(TileRowA);
        const __m256 SatMask   = _mm256_cmp_ps(AccA, _mm256_set1_ps(c_AccAlphaSat), _CMP_LT_OQ);
        const __m256 FinalMask = _mm256_and_ps(CombinedMask, SatMask);

        const __m256 Transmit  = _mm256_sub_ps(_mm256_set1_ps(1.0f), AccA);
        const __m256 ContribA  = _mm256_and_ps(_mm256_mul_ps(AlphaEff, Transmit), FinalMask); // mask before accumulating
        const __m256 ContribR  = _mm256_mul_ps(ContribA, _mm256_set1_ps(R));
        const __m256 ContribG  = _mm256_mul_ps(ContribA, _mm256_set1_ps(G));
        const __m256 ContribB  = _mm256_mul_ps(ContribA, _mm256_set1_ps(B));

        _mm256_storeu_ps(TileRowR, _mm256_add_ps(_mm256_loadu_ps(TileRowR), ContribR));
        _mm256_storeu_ps(TileRowG, _mm256_add_ps(_mm256_loadu_ps(TileRowG), ContribG));
        _mm256_storeu_ps(TileRowB, _mm256_add_ps(_mm256_loadu_ps(TileRowB), ContribB));
        _mm256_storeu_ps(TileRowA, _mm256_add_ps(AccA, ContribA));
      } //end of loop over AABB x
    } //end of loop over AABB y

    if constexpr(c_TileTerminate)
    {
      if(i && ((i & 0x3F) == 0))
      {
        const flt32* PtrTileA = &((*TileBuff)[(int32)eCmp::A][0][0]);
        const __m256 SatVec = _mm256_set1_ps(c_AccAlphaSat);
        bool Continue = false;
        for(int32 p = 0; p < c_TileArea; p += 8)
        {
          const __m256 AccA = _mm256_loadu_ps(PtrTileA + p);
          if(_mm256_movemask_ps(_mm256_cmp_ps(AccA, SatVec, _CMP_LT_OQ)) != 0) { Continue = true; break; }
        }
        if(!Continue) { break; }
      }
    }
  } //end of loop over input splats

  //copy picture from tile to pic
  {
    const int32 DstStride = DstPic->getStride();
    const int32 MaxValI32 = xBitDepth2MaxValue(DstPic->getBitDepth());
    const flt32 MaxValF32 = (flt32)MaxValI32;
    const int32 RowWidth  = PixX1 - PixX0 + 1;

    const __m256  MaxValF32V = _mm256_set1_ps(MaxValF32);
    const __m256  SignMask   = _mm256_set1_ps(-0.0f);
    const __m256  HalfV      = _mm256_set1_ps(0.5f);
    const __m256i ZeroI      = _mm256_setzero_si256();
    const __m256i MaxValI32V = _mm256_set1_epi32(MaxValI32);

    for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
      uint16* restrict DstPtr = DstPic->getAddr((eCmp)CmpIdx);
      for(int32 y = 0; y <= PixY1 - PixY0; y++)
      {
        const int32 PicY = PixY0 + y;
        uint16* restrict DstRowPtr = DstPtr + PicY * DstStride;
        const flt32*     SrcRowPtr = &((*TileBuff)[CmpIdx][y][0]);

        int32 x = 0;
        for(; x + 8 <= RowWidth; x += 8)
        {
          const __m256 SrcVal = _mm256_mul_ps(_mm256_loadu_ps(SrcRowPtr + x), MaxValF32V);
          const __m256 SignedHalf = _mm256_or_ps(HalfV, _mm256_and_ps(SrcVal, SignMask)); //TODO - use "round" functionf from SIMD helpers
          const __m256i Rounded   = _mm256_cvttps_epi32(_mm256_add_ps(SrcVal, SignedHalf));
          const __m256i Clamped   = _mm256_min_epi32(_mm256_max_epi32(Rounded, ZeroI), MaxValI32V);

          const __m128i Lo     = _mm256_castsi256_si128(Clamped);
          const __m128i Hi     = _mm256_extracti128_si256(Clamped, 1);
          const __m128i Packed = _mm_packus_epi32(Lo, Hi);

          _mm_storeu_si128((__m128i*)(DstRowPtr + PixX0 + x), Packed);
        }
        for(; x < RowWidth; x++)
        {
          const int32 PicX = PixX0 + x;
          flt32  SrcVal = (*TileBuff)[CmpIdx][y][x];
          uint16 DstVal = (uint16)xClip(xRoundF32ToI32(SrcVal * MaxValF32), 0, MaxValI32);
          DstRowPtr[PicX] = DstVal;
        }
      }
    }
  }

  //copy mask from tile to pic
  // TODO - vectorize it
  if(DstMsk != nullptr)
  {
    const int32 DstStride = DstMsk->getStride();
    const int32 MaxValI32 = xBitDepth2MaxValue(DstMsk->getBitDepth());
    const flt32 MaxValF32 = (flt32)MaxValI32;

    uint16* restrict DstPtr = DstMsk->getAddr();
    for(int32 y = 0; y <= PixY1 - PixY0; y++)
    {
      const int32 PicY = PixY0 + y;
      uint16* restrict DstRowPtr = DstPtr + PicY * DstStride;
      for(int32 x = 0; x <= PixX1 - PixX0; x++)
      {
        const int32 PicX = PixX0 + x;
        flt32  SrcVal = (*TileBuff)[3][y][x];
        uint16 DstVal = (uint16)xClip(xRoundF32ToI32(SrcVal * MaxValF32), 0, MaxValI32);
        DstRowPtr[PicX] = DstVal;
      }
    }
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB_NAMESPACE::GSR

#endif //X_SIMD_CAN_USE_AVX
