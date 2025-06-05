/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xCorrespPixelShiftSTD.h"
#include "xCorrespPixelShiftPrms.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xCorrespPixelShiftSTD
//===============================================================================================================================================================================================================

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q planar
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint64V4 xCorrespPixelShiftSTD::CalcDistAsymmetricRow(const xPicP* Tst, const xPicP* Ref, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32  Width     = Tst->getWidth ();
  const int32  TstStride = Tst->getStride();
  const int32  TstOffset = y * TstStride;

  uint64V4 RowDist = { 0, 0, 0, 0 };

  const uint16* TstPtrLm = Tst->getAddr(eCmp::LM) + TstOffset;
  const uint16* TstPtrCb = Tst->getAddr(eCmp::CB) + TstOffset;
  const uint16* TstPtrCr = Tst->getAddr(eCmp::CR) + TstOffset;

  for(int32 x = 0; x < Width; x++)
  {
    const int32V4 CurrTstValue  = int32V4((int32)(TstPtrLm[x]), (int32)(TstPtrCb[x]), (int32)(TstPtrCr[x]), 0) + GlobalColorShift;
    const int32   BestRefOffset = FindBestPixelWithinBlock(CurrTstValue, Ref, x, y, SearchRange, CmpWeights);

    for(uint32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
      const uint16* RefAddr = Ref->getAddr((eCmp)CmpIdx);
      int32 Diff = CurrTstValue[CmpIdx] - (int32)(RefAddr[BestRefOffset]);  //TODO - xc_CLIP_CURR_TST_RANGE
      int32 Dist = xPow2(Diff);
      RowDist[CmpIdx] += Dist;
    }
  }//x

  return RowDist;
}
int32 xCorrespPixelShiftSTD::FindBestPixelWithinBlock(const int32V4& TstPel, const xPicP* Ref, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 BegY = CenterY - SearchRange;
  const int32 EndY = CenterY + SearchRange;
  const int32 BegX = CenterX - SearchRange;
  const int32 EndX = CenterX + SearchRange;

  const uint16* RefPtrLm = Ref->getAddr  (eCmp::LM);
  const uint16* RefPtrCb = Ref->getAddr  (eCmp::CB);
  const uint16* RefPtrCr = Ref->getAddr  (eCmp::CR);
  const int32   Stride   = Ref->getStride();

  int32 BestError  = std::numeric_limits<int32>::max();
  int32 BestOffset = NOT_VALID;

  for(int32 y = BegY; y <= EndY; y++)
  {
    for(int32 x = BegX; x <= EndX; x++)
    {
      const int32 Offset = y * Stride + x;
      const int32 DistLm = xPow2(TstPel[0] - (int32)(RefPtrLm[Offset]));
      const int32 DistCb = xPow2(TstPel[1] - (int32)(RefPtrCb[Offset]));
      const int32 DistCr = xPow2(TstPel[2] - (int32)(RefPtrCr[Offset]));
      if constexpr (xCorrespPixelShiftPrms::c_UseRuntimeCmpWeights)
      {
        const int32 Error = DistLm * CmpWeights[0] + DistCb * CmpWeights[1] + DistCr * CmpWeights[2];
        if(Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
      else
      {
        const int32 Error = (DistLm << 2) + DistCb + DistCr;
        if (Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
    } //x
  } //y

  return BestOffset;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q interleaved
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint64V4 xCorrespPixelShiftSTD::CalcDistAsymmetricRow(const xPicI* Tst, const xPicI* Ref, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  assert(Tst->isCompatible(Ref));

  const int32  Width     = Tst->getWidth ();
  const int32  TstStride = Tst->getStride();
  const int32  TstOffset = y * TstStride;

  uint64V4 RowDist = { 0, 0, 0, 0 };

  const uint16V4* TstPtr  = Tst->getAddr() + TstOffset;
        
  for(int32 x = 0; x < Width; x++)
  {
    const int32V4 CurrTstValue  = (int32V4)(TstPtr[x]) + GlobalColorShift;
    const int32   BestRefOffset = FindBestPixelWithinBlock(CurrTstValue, Ref, x, y, SearchRange, CmpWeights);
    const int32V4 Diff = CurrTstValue - (int32V4)(Ref->getAddr()[BestRefOffset]); //TODO - xc_CLIP_CURR_TST_RANGE
    const int32V4 Dist = Diff.getVecPow2();
    RowDist += (uint64V4)Dist;
  }//x

  return RowDist;
}
int32 xCorrespPixelShiftSTD::FindBestPixelWithinBlock(const int32V4& TstPel, const xPicI* Ref, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights)
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
uint64V4 xCorrespPixelShiftSTD::CalcDistAsymmetricRowM(const xPicI* Tst, const xPicI* Ref, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  assert(Tst->isCompatible(Ref));

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
    const int32   BestRefOffset = FindBestPixelWithinBlockM(CurrTstValue, Ref, Msk, x, y, SearchRange, CmpWeights);
    const int32V4 Diff = CurrTstValue - (int32V4)(Ref->getAddr()[BestRefOffset]); //TODO - xc_CLIP_CURR_TST_RANGE
    const int32V4 Dist = Diff.getVecPow2();
    RowDist += ((uint64V4)Dist) * CurrMskValue;
  } //x

  return RowDist;
}
int32 xCorrespPixelShiftSTD::FindBestPixelWithinBlockM(const int32V4& TstPel, const xPicI* Ref, const xPicP* Msk, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights)
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

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// shift-compensated picture generation
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void xCorrespPixelShiftSTD::GenShftCompRow(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 Width     = Tst->getWidth ();
  const int32 TstStride = Tst->getStride();
  const int32 TstOffset = y * TstStride;
  const int32 MaxValue  = DstRef->getMaxPelValue();

  const uint16*    TstPtrLm = Tst   ->getAddr(eCmp::LM) + TstOffset;
  const uint16*    TstPtrCb = Tst   ->getAddr(eCmp::CB) + TstOffset;
  const uint16*    TstPtrCr = Tst   ->getAddr(eCmp::CR) + TstOffset;
  uint16* restrict DstPtrLm = DstRef->getAddr(eCmp::LM) + TstOffset;
  uint16* restrict DstPtrCb = DstRef->getAddr(eCmp::CB) + TstOffset;
  uint16* restrict DstPtrCr = DstRef->getAddr(eCmp::CR) + TstOffset;

  for(int32 x = 0; x < Width; x++)
  {
    const int32V4 CurrTstValue  = int32V4((int32)(TstPtrLm[x]), (int32)(TstPtrCb[x]), (int32)(TstPtrCr[x]), 0) + GlobalColorShift;
    const int32   BestRefOffset = FindBestPixelWithinBlock(CurrTstValue, Ref, x, y, SearchRange, CmpWeights);
    int32  RefLm = Ref->getAddr(eCmp::LM)[BestRefOffset];
    int32  RefCb = Ref->getAddr(eCmp::CB)[BestRefOffset];
    int32  RefCr = Ref->getAddr(eCmp::CR)[BestRefOffset];
    uint16 DstLm = (uint16)xClipU(RefLm - GlobalColorShift[0], MaxValue);
    uint16 DstCb = (uint16)xClipU(RefCb - GlobalColorShift[1], MaxValue);
    uint16 DstCr = (uint16)xClipU(RefCr - GlobalColorShift[2], MaxValue);
    DstPtrLm[x] = DstLm;
    DstPtrCb[x] = DstCb;
    DstPtrCr[x] = DstCr;
  }//x
}
void xCorrespPixelShiftSTD::GenShftCompRow(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32   Width     = Tst->getWidth ();
  const int32   TstStride = Tst->getStride();
  const int32   TstOffset = y * TstStride;
  const int32V4 MaxValue  = xMakeVec4<int32>(DstRef->getMaxPelValue());

  const uint16V4*    TstPtr = Tst   ->getAddr() + TstOffset;
  uint16V4* restrict DstPtr = DstRef->getAddr() + TstOffset;

  for(int32 x = 0; x < Width; x++)
  {
    const int32V4  CurrTstValue  = (int32V4)(TstPtr[x]) + GlobalColorShift;
    const int32    BestRefOffset = FindBestPixelWithinBlock(CurrTstValue, Ref, x, y, SearchRange, CmpWeights);
    const int32V4  RefValue      = (int32V4)(Ref->getAddr()[BestRefOffset]);
    const uint16V4 DstValue      = (uint16V4)((RefValue - GlobalColorShift).getClipU(MaxValue));
    DstPtr[x] = DstValue;
  }//x
}
void xCorrespPixelShiftSTD::GenShftCompRowM(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32   Width     = Tst->getWidth ();
  const int32   TstStride = Tst->getStride();
  const int32   TstOffset = y * TstStride;
  const int32   MskStride = Msk->getStride();
  const int32   MskOffset = y * MskStride;
  const int32V4 MaxValue  = xMakeVec4<int32>(DstRef->getMaxPelValue());

  const uint16V4*    TstPtr = Tst   ->getAddr(        ) + TstOffset;
  const uint16*      MskPtr = Msk   ->getAddr(eCmp::LM) + MskOffset;
  uint16V4* restrict DstPtr = DstRef->getAddr(        ) + TstOffset;

  for(int32 x = 0; x < Width; x++)
  {
    if(MskPtr[x] == 0) { continue; } //skip masked pixels
    const int32V4  CurrTstValue  = (int32V4)(TstPtr[x]) + GlobalColorShift;
    const int32    BestRefOffset = FindBestPixelWithinBlockM(CurrTstValue, Ref, Msk, x, y, SearchRange, CmpWeights);
    const int32V4  RefValue      = (int32V4)(Ref->getAddr()[BestRefOffset]);
    const uint16V4 DstValue      = (uint16V4)((RefValue - GlobalColorShift).getClipU(MaxValue));
    DstPtr[x] = DstValue;
  }//x

}

//===============================================================================================================================================================================================================

} //end of namespace PMBB