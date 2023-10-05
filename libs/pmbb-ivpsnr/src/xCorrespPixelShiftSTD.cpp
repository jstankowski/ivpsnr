/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xCorrespPixelShiftSTD.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xCorrespPixelShiftSTD
//===============================================================================================================================================================================================================

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q interleaved
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int32V4 xCorrespPixelShiftSTD::CalcDistAsymmetricRow(const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32  Width     = Tst->getWidth ();
  const int32  TstStride = Tst->getStride();
  const int32  TstOffset = y * TstStride;

  int32V4 RowDist = { 0, 0, 0, 0 };

  const uint16V4* TstPtr  = Tst->getAddr() + TstOffset;
        
  for(int32 x = 0; x < Width; x++)
  {
    const int32V4 CurrTstValue  = (int32V4)(TstPtr[x]) + GlobalColorShift;
    const int32   BestRefOffset = FindBestPixelWithinBlock(Ref, CurrTstValue, x, y, SearchRange, CmpWeights);
    const int32V4 Diff = CurrTstValue - (int32V4)(Ref->getAddr()[BestRefOffset]);
    const int32V4 Dist = Diff.getVecPow2();
    RowDist += Dist;
  }//x

  return RowDist;
}
int32 xCorrespPixelShiftSTD::FindBestPixelWithinBlock(const xPicI* Ref, const int32V4& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 BegY = CenterY - SearchRange;
  const int32 EndY = CenterY + SearchRange;
  const int32 BegX = CenterX - SearchRange;
  const int32 EndX = CenterX + SearchRange;

  const uint16V4* RefPtr = Ref->getAddr  ();
  const int32     Stride = Ref->getStride();

  int32 BestError  = std::numeric_limits<int32>::max();
  int32 BestOffset = NOT_VALID;

  for(int32 y = BegY; y <= EndY; y++)
  {
    for(int32 x = BegX; x <= EndX; x++)
    {
      const int32   Offset = y * Stride + x;
      const int32V4 RefPel = (int32V4)(RefPtr[Offset]);
      const int32V4 Dist   = (TstPel - RefPel).getVecPow2();
      if constexpr (c_UseRuntimeCmpWeights)
      {
        const int32 Error = (Dist * CmpWeights).getSum();
        if (Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
      else
      {
        const int32 Error = (Dist[0] << 2) + Dist[1] + Dist[2];
        if (Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
    } //x
  } //y

  return BestOffset;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q interleaved - with mask
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint64V4 xCorrespPixelShiftSTD::CalcDistAsymmetricRowM(const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32  Width     = Tst->getWidth ();
  const int32  TstStride = Tst->getStride();
  const int32  TstOffset = y * TstStride;
  const int32  MskStride = Msk->getStride();
  const int32  MskOffset = y * MskStride;

  uint64V4 RowDist = { 0, 0, 0, 0 };

  const uint16V4* TstPtr = Tst->getAddr(        ) + TstOffset;
  const uint16*   MskPtr = Msk->getAddr(eCmp::LM) + MskOffset;
        
  for(int32 x = 0; x < Width; x++)
  {
    const int32   CurrMskValue  = (int32)MskPtr[x];
    if(CurrMskValue == 0) { continue; } //skip masked pixels
    const int32V4 CurrTstValue  = (int32V4)(TstPtr[x]) + GlobalColorShift;
    const int32   BestRefOffset = FindBestPixelWithinBlockM(Ref, CurrTstValue, Msk, x, y, SearchRange, CmpWeights);
    const int32V4 Diff = CurrTstValue - (int32V4)(Ref->getAddr()[BestRefOffset]);
    const int32V4 Dist = Diff.getVecPow2();
    RowDist += ((uint64V4)Dist) * CurrMskValue;
  } //x

  return RowDist;
}
int32 xCorrespPixelShiftSTD::FindBestPixelWithinBlockM(const xPicI* Ref, const int32V4& TstPel, const xPicP* Msk, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 BegY = CenterY - SearchRange;
  const int32 EndY = CenterY + SearchRange;
  const int32 BegX = CenterX - SearchRange;
  const int32 EndX = CenterX + SearchRange;

  const uint16V4* RefPtr = Ref->getAddr  ();
  const uint16*   MskPtr = Msk->getAddr  (eCmp::LM);
  const int32     Stride = Ref->getStride();

  int32 BestError  = std::numeric_limits<int32>::max();
  int32 BestOffset = NOT_VALID;

  for(int32 y = BegY; y <= EndY; y++)
  {
    for(int32 x = BegX; x <= EndX; x++)
    {
      const int32   Offset = y * Stride + x;
      if(MskPtr[Offset] == 0) { continue; }
      const int32V4 RefPel = (int32V4)(RefPtr[Offset]);
      const int32V4 Dist   = (TstPel - RefPel).getVecPow2();
      if constexpr (c_UseRuntimeCmpWeights)
      {
        const int32 Error = (Dist * CmpWeights).getSum();
        if (Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
      else
      {
        const int32 Error = (Dist[0] << 2) + Dist[1] + Dist[2];
        if (Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
    } //x
  } //y

  return BestOffset;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB