/*
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xCommonDefGSR.h"

#if X_PMBB_GSR_DISABLE_FP_CONTRACT_IN_RASTERIZER
X_PMBB_DISABLE_FP_CONTRACT_BEG;
#endif

#include "xGSR_RasterizerSTD.h"
#include "xHelpersFLT.h"
#include "xExponential.h"
#include <cmath>
#include <algorithm>

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================

static inline flt32 xExp(flt32 x) { return xExponential<X_PMBB_GSR_EXP_BOUND_CHECKING, X_PMBB_GSR_EXP_USE_FMA>::SchraudolphExpCorrected(x); }
static inline flt64 xExp(flt64 x) { return xExponential<X_PMBB_GSR_EXP_BOUND_CHECKING, X_PMBB_GSR_EXP_USE_FMA>::SchraudolphExpCorrected(x); }

//===============================================================================================================================================================================================================

void xRasterizerSTD::RasterizeTile(xPicP* DstPic, tPicM* DstMsk, const xProjectedCloud& Projected, const tSplIdxV& SplatIndicesTile, int32V2 TileIdxXY, tTileBuff* restrict TileBuff)
{
  const int32 NumSplats = (int32)SplatIndicesTile.size();
  if(NumSplats == 0) { return; }

  const int32 PicWidth  = DstPic->getWidth ();
  const int32 PicHeight = DstPic->getHeight();

  // Pixel bounds of this tile (clamped to image borders)
  const int32 PixX0 = TileIdxXY.getX() * c_TileWidth ;
  const int32 PixY0 = TileIdxXY.getY() * c_TileHeight;
  const int32 PixX1 = std::min(PixX0 + c_TileWidth  - 1, PicWidth  - 1);
  const int32 PixY1 = std::min(PixY0 + c_TileHeight - 1, PicHeight - 1);

  //clear local tile cache
  memset(TileBuff, 0, sizeof(tTileBuff));

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

    // Rasterize: for each pixel in the intersection
    for(int32 Py = Y0; Py <= Y1; Py++)
    {
      const int32  TLy     = Py - PixY0; // local tile row
      const flt32  Dy      = c_PelCenter0o5 ? ((flt32)Py + 0.5f) - ScreenY : (flt32)Py - ScreenY; // MGM: pixel centre at +0.5
      const flt32  icC_Dy  = InvCovC * Dy;
      const flt32  icB_Dy  = InvCovB * Dy;

      for(int32 Px = X0; Px <= X1; Px++)
      {
        const int32 TLx = Px - PixX0; // local tile col

        // Early exit if tile pixel already saturated (accumulated alpha >= c_AccAlphaSat)
        if((*TileBuff)[(int32)eCmp::A][TLy][TLx] >= c_AccAlphaSat) { continue; }

        // EWA quadratic form: d^T * Sigma^{-1} * d  where d = (Dx, Dy) = icA*Dx^2 + 2*icB*Dx*Dy + icC*Dy^2
        // EWA weight: same quadratic form as MGM (mathematically equivalent to MGM's sigma)
        const flt32 Dx     = c_PelCenter0o5 ? ((flt32)Px + 0.5f) - ScreenX : (flt32)Px - ScreenX; // MGM: pixel centre at +0.5
        const flt32 Power1 = InvCovA * Dx * Dx;
        const flt32 Power2 = 2.0f * icB_Dy * Dx;
        const flt32 Power3 = icC_Dy * Dy;
        const flt32 Power  = -0.5f * (Power1 + Power2 + Power3);

        // Guard against positive/degenerate values (same as MGM's sigma < 0 check)
        if(Power > 0.0f || Power < c_PowerThreshold) { continue; }

        const flt32 Weight   = xExp(Power);
        const flt32 AlphaEff = std::min(A * Weight, c_EffectiveAlphaCap); // MGM: cap effective alpha at 0.999

        if(AlphaEff < c_AlphaThreshold) { continue; }

        // Alpha compositing: front-to-back blending
        const flt32 Transmit = (1.0f - (*TileBuff)[(int32)eCmp::A][TLy][TLx]);
        const flt32 ContribA = AlphaEff * Transmit;
        const flt32 ContribR = ContribA * R;
        const flt32 ContribG = ContribA * G;
        const flt32 ContribB = ContribA * B;

        (*TileBuff)[(int32)eCmp::R][TLy][TLx] += ContribR;
        (*TileBuff)[(int32)eCmp::G][TLy][TLx] += ContribG;
        (*TileBuff)[(int32)eCmp::B][TLy][TLx] += ContribB;
        (*TileBuff)[(int32)eCmp::A][TLy][TLx] += ContribA;
      } //end of loop over AABB x
    } //end of loop over AABB y

    if constexpr(c_TileTerminate)
    {
      if(i && ((i & 0x3F) == 0)) //every 63
      {
        bool Continue = false;
        const flt32* PtrTileA = &((*TileBuff)[(int32)eCmp::A][0][0]);
        for(int32 p = 0; p < c_TileArea; p++)
        {
          if(PtrTileA[p] < c_AccAlphaSat) { Continue = true; break; }
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

    for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
      uint16* restrict DstPtr = DstPic->getAddr((eCmp)CmpIdx);
      for(int32 y = 0; y <= PixY1 - PixY0; y++)
      {
        const int32 PicY = PixY0 + y;
        uint16* restrict DstRowPtr = DstPtr + PicY * DstStride;
        for(int32 x = 0; x <= PixX1 - PixX0; x++)
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
void xRasterizerSTD::RasterizePic(tRstBuf* DstPic, const xProjectedCloud& Projected)
{
  flt32* restrict DstPtrR   = DstPic->getAddr  (eCmp::R);
  flt32* restrict DstPtrG   = DstPic->getAddr  (eCmp::G);
  flt32* restrict DstPtrB   = DstPic->getAddr  (eCmp::B);
  flt32* restrict DstPtrA   = DstPic->getAddr  (eCmp::A);
  const int32     DstStride = DstPic->getStride();

  for(int32 i=0; i< Projected.getNumValidSplats(); i++)
  {
    const int32 Idx = Projected.getSortedIdx()[i];

    const xProjectedCloud::xBoundingBox& BoundingBox = Projected.getBoundingBox()[Idx];
    const xProjectedCloud::xSplatData&   SplatData   = Projected.getSplatData  ()[Idx];

    for(int32 Py = BoundingBox.MinY; Py <= BoundingBox.MaxY; Py++)
    {
      const flt32  Dy     = ((flt32)Py + 0.5f) - SplatData.ScreenY;  // mpeg: pixel centre at +0.5
      const flt32  icC_Dy = SplatData.InvCovC * Dy;
      const flt32  icB_Dy = SplatData.InvCovB * Dy;

      flt32* RowR = DstPtrR + Py * DstStride;
      flt32* RowG = DstPtrG + Py * DstStride;
      flt32* RowB = DstPtrB + Py * DstStride;
      flt32* RowA = DstPtrA + Py * DstStride;

      for(int32 Px = BoundingBox.MinX; Px <= BoundingBox.MaxX; Px++)
      {
        // Early exit if pixel already saturated (accumulated alpha >= 0.9999)
        if(RowA[Px] >= c_AccAlphaSat) { continue; }

        // EWA weight: same quadratic form as PMBB (mathematically equivalent to mpeg's sigma)
        const flt32 Dx     = ((flt32)Px + 0.5f) - SplatData.ScreenX;  // mpeg: pixel centre at +0.5
        const flt32 Power1 = SplatData.InvCovA * Dx * Dx;
        const flt32 Power2 = 2.0f * icB_Dy * Dx;
        const flt32 Power3 = icC_Dy * Dy;
        const flt32 Power  = -0.5f * (Power1 + Power2 + Power3);

        // Guard against positive/degenerate values (same as mpeg's sigma < 0 check)
        if(Power > 0.0f || Power < c_PowerThreshold) { continue; }

        const flt32 Weight   = xExp(Power);
        const flt32 AlphaEff = std::min(SplatData.A * Weight, c_EffectiveAlphaCap); // MGM: cap effective alpha at 0.999

        if(AlphaEff < c_AlphaThreshold) { continue; }

        // Front-to-back alpha compositing
        const flt32 Transmit = (1.0f - RowA[Px]);
        const flt32 ContribA = AlphaEff * Transmit;
        const flt32 ContribR = ContribA * SplatData.R;
        const flt32 ContribG = ContribA * SplatData.G;
        const flt32 ContribB = ContribA * SplatData.B;

        RowR[Px] += ContribR;
        RowG[Px] += ContribG;
        RowB[Px] += ContribB;
        RowA[Px] += ContribA;
      }
    }
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB::GSR
