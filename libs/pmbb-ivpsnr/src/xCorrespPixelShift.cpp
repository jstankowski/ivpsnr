/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xCorrespPixelShift.h"
#include <cassert>
#include <numeric>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xBaseIV
//===============================================================================================================================================================================================================

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q planar
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int32V4 xCorrespPixelShift::xCalcDistAsymmetricRow(const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32  Width     = Tst->getWidth();
  const int32  TstStride = Tst->getStride();
  const int32  TstOffset = y * TstStride;

  int32V4 RowDist = { 0, 0, 0, 0 };

  const uint16* TstPtrY = Tst->getAddr(eCmp::LM) + TstOffset;
  const uint16* TstPtrU = Tst->getAddr(eCmp::CB) + TstOffset;
  const uint16* TstPtrV = Tst->getAddr(eCmp::CR) + TstOffset;

  for(int32 x = 0; x < Width; x++)
  {
    const int32V4 CurrTstValue  = int32V4((int32)(TstPtrY[x]), (int32)(TstPtrU[x]), (int32)(TstPtrV[x]), 0) + GlobalColorShift;
    const int32   BestRefOffset = xFindBestPixelWithinBlock(Ref, CurrTstValue, x, y, SearchRange, CmpWeights);

    for(uint32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
      const uint16* RefAddr = Ref->getAddr((eCmp)CmpIdx);
      int32 Diff = CurrTstValue[CmpIdx] - (int32)(RefAddr[BestRefOffset]);
      int32 Dist = xPow2(Diff);
      RowDist[CmpIdx] += Dist;
    }
  }//x

  return RowDist;
}
int32 xCorrespPixelShift::xFindBestPixelWithinBlock(const xPicP* Ref, const int32V4& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 BegY = CenterY - SearchRange;
  const int32 EndY = CenterY + SearchRange;
  const int32 BegX = CenterX - SearchRange;
  const int32 EndX = CenterX + SearchRange;

  const uint16* RefPtrY = Ref->getAddr  (eCmp::LM);
  const uint16* RefPtrU = Ref->getAddr  (eCmp::CB);
  const uint16* RefPtrV = Ref->getAddr  (eCmp::CR);
  const int32   Stride  = Ref->getStride();

  int32 BestError  = std::numeric_limits<int32>::max();
  int32 BestOffset = NOT_VALID;

  for(int32 y = BegY; y <= EndY; y++)
  {
    for(int32 x = BegX; x <= EndX; x++)
    {
      const int32 Offset = y * Stride + x;
      const int32 DistY  = xPow2(TstPel[0] - (int32)(RefPtrY[Offset]));
      const int32 DistU  = xPow2(TstPel[1] - (int32)(RefPtrU[Offset]));
      const int32 DistV  = xPow2(TstPel[2] - (int32)(RefPtrV[Offset]));
      if constexpr (c_UseRuntimeCmpWeights)
      {
        const int32 Error = DistY * CmpWeights[0] + DistU * CmpWeights[1] + DistV * CmpWeights[2];
        if(Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
      else
      {
        const int32 Error = (DistY << 2) + DistU + DistV;
        if (Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
    } //x
  } //y

  return BestOffset;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB