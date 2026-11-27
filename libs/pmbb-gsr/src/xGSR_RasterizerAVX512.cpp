/*
    SPDX-FileCopyrightText: 2026 Blazej Szydelko <blazej.szydelko@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xGSR_RasterizerAVX512.h"

#if X_SIMD_CAN_USE_AVX512

#if X_PMBB_GSR_DISABLE_FP_CONTRACT_IN_RASTERIZER
X_PMBB_DISABLE_FP_CONTRACT_BEG;
#endif

#include "xExponentialAVX512.h"
#include "xHelpersFLT.h"
#include <immintrin.h>
#include <algorithm>

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================

static inline __m512 xExp(__m512 x) { return xExponentialAVX512<X_PMBB_GSR_EXP_BOUND_CHECKING, X_PMBB_GSR_EXP_USE_FMA>::SchraudolphExpCorrected(x); }

//===============================================================================================================================================================================================================

void xRasterizerAVX512::RasterizeTile(xPicP* DstPic, tPicM* DstMsk, const xProjectedCloud& Projected, const tSplIdxV& SplatIndicesTile, int32V2 TileIdxXY, tTileBuff* restrict TileBuff)
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

  const __m512 LaneIdx = _mm512_setr_ps(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

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

      // Chunks are aligned to the tile, not to TLx0, so ChunkStart+15 never exceeds c_TileWidth-1 -
      // a chunk may overhang past [TLx0,TLx1], RangeMask below excludes those lanes instead of a scalar tail.
      const int32 ChunkStart0 = TLx0 & ~15;

      for(int32 x = ChunkStart0; x <= TLx1; x += 16)
      {
        const __m512 TLxVec     = _mm512_add_ps(_mm512_set1_ps((flt32)x), LaneIdx);
        const __mmask16 RangeMask = _mm512_cmp_ps_mask(TLxVec, _mm512_set1_ps((flt32)TLx0), _CMP_GE_OQ)
                                   & _mm512_cmp_ps_mask(TLxVec, _mm512_set1_ps((flt32)TLx1), _CMP_LE_OQ);

        const __m512 PxVec     = _mm512_add_ps(_mm512_set1_ps((flt32)PixX0), TLxVec);
        const __m512 PelOffset = c_PelCenter0o5 ? _mm512_set1_ps(0.5f) : _mm512_setzero_ps();
        const __m512 Dx        = _mm512_sub_ps(_mm512_add_ps(PxVec, PelOffset), _mm512_set1_ps(ScreenX));

        const __m512 Power1 = _mm512_mul_ps(_mm512_mul_ps(_mm512_set1_ps(InvCovA), Dx), Dx);
        const __m512 Power2 = _mm512_mul_ps(_mm512_set1_ps(2.0f * icB_Dy), Dx);
        const __m512 Power3 = _mm512_set1_ps(icC_Dy * Dy);
        const __m512 Power  = _mm512_mul_ps(_mm512_add_ps(_mm512_add_ps(Power1, Power2), Power3), _mm512_set1_ps(-0.5f));

        const __mmask16 PowerMask = _mm512_cmp_ps_mask(Power, _mm512_setzero_ps(), _CMP_LE_OQ)
                                   & _mm512_cmp_ps_mask(Power, _mm512_set1_ps(c_PowerThreshold), _CMP_GE_OQ);
        __mmask16 CombinedMask = PowerMask & RangeMask;

        const __m512 Weight = xExp(Power);

        __m512 AlphaEff = _mm512_mul_ps(_mm512_set1_ps(A), Weight);
        AlphaEff = _mm512_min_ps(AlphaEff, _mm512_set1_ps(c_EffectiveAlphaCap));
        const __mmask16 AlphaMask = _mm512_cmp_ps_mask(AlphaEff, _mm512_set1_ps(c_AlphaThreshold), _CMP_GE_OQ);
        CombinedMask = CombinedMask & AlphaMask;

        flt32* TileRowR = &((*TileBuff)[(int32)eCmp::R][TLy][x]);
        flt32* TileRowG = &((*TileBuff)[(int32)eCmp::G][TLy][x]);
        flt32* TileRowB = &((*TileBuff)[(int32)eCmp::B][TLy][x]);
        flt32* TileRowA = &((*TileBuff)[(int32)eCmp::A][TLy][x]);

        const __m512 AccA        = _mm512_loadu_ps(TileRowA);
        const __mmask16 SatMask  = _mm512_cmp_ps_mask(AccA, _mm512_set1_ps(c_AccAlphaSat), _CMP_LT_OQ);
        const __mmask16 FinalMask = CombinedMask & SatMask;

        const __m512 Transmit = _mm512_sub_ps(_mm512_set1_ps(1.0f), AccA);
        const __m512 ContribA = _mm512_maskz_mul_ps(FinalMask, AlphaEff, Transmit); // zero-masked - lanes outside FinalMask contribute 0
        const __m512 ContribR = _mm512_mul_ps(ContribA, _mm512_set1_ps(R));
        const __m512 ContribG = _mm512_mul_ps(ContribA, _mm512_set1_ps(G));
        const __m512 ContribB = _mm512_mul_ps(ContribA, _mm512_set1_ps(B));

        _mm512_storeu_ps(TileRowR, _mm512_add_ps(_mm512_loadu_ps(TileRowR), ContribR));
        _mm512_storeu_ps(TileRowG, _mm512_add_ps(_mm512_loadu_ps(TileRowG), ContribG));
        _mm512_storeu_ps(TileRowB, _mm512_add_ps(_mm512_loadu_ps(TileRowB), ContribB));
        _mm512_storeu_ps(TileRowA, _mm512_add_ps(AccA, ContribA));
      } //end of loop over AABB x
    } //end of loop over AABB y

    if constexpr(c_TileTerminate)
    {
      if(i && ((i & 0x3F) == 0))
      {
        const flt32* PtrTileA = &((*TileBuff)[(int32)eCmp::A][0][0]);
        const __m512 SatVec   = _mm512_set1_ps(c_AccAlphaSat);
        bool Continue = false;
        for(int32 p = 0; p < c_TileArea; p += 16)
        {
          const __m512 AccA = _mm512_loadu_ps(PtrTileA + p);
          if(_mm512_cmp_ps_mask(AccA, SatVec, _CMP_LT_OQ) != 0) { Continue = true; break; }
        }
        if(!Continue) { break; }
      }
    }
  }

  //copy picture from tile to pic
  {
    const int32 DstStride = DstPic->getStride();
    const int32 MaxValI32 = xBitDepth2MaxValue(DstPic->getBitDepth());
    const flt32 MaxValF32 = (flt32)MaxValI32;
    const int32 RowWidth  = PixX1 - PixX0 + 1;

    const __m512  MaxValF32V = _mm512_set1_ps(MaxValF32);
    const __m512  SignMask   = _mm512_set1_ps(-0.0f);
    const __m512  HalfV      = _mm512_set1_ps(0.5f);
    const __m512i ZeroI      = _mm512_setzero_si512();
    const __m512i MaxValI32V = _mm512_set1_epi32(MaxValI32);

    for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
      uint16* restrict DstPtr = DstPic->getAddr((eCmp)CmpIdx);
      for(int32 y = 0; y <= PixY1 - PixY0; y++)
      {
        const int32 PicY = PixY0 + y;
        uint16* restrict DstRowPtr = DstPtr + PicY * DstStride;
        const flt32*     SrcRowPtr = &((*TileBuff)[CmpIdx][y][0]);
        int32 x = 0;
        for(; x + 16 <= RowWidth; x += 16)
        {
          const __m512 SrcVal = _mm512_mul_ps(_mm512_loadu_ps(SrcRowPtr + x), MaxValF32V);
          const __m512 SignedHalf = _mm512_or_ps(HalfV, _mm512_and_ps(SrcVal, SignMask));
          const __m512i Rounded   = _mm512_cvttps_epi32(_mm512_add_ps(SrcVal, SignedHalf));
          const __m512i Clamped   = _mm512_min_epi32(_mm512_max_epi32(Rounded, ZeroI), MaxValI32V);
          const __m256i Packed    = _mm512_cvtusepi32_epi16(Clamped);

          _mm256_storeu_si256((__m256i*)(DstRowPtr + PixX0 + x), Packed);
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

#endif //X_SIMD_CAN_USE_AVX512
