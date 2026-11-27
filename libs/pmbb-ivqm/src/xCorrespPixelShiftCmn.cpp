/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xCorrespPixelShiftCmn.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

xCorrespPixelShiftCmn::eCalcPath xCorrespPixelShiftCmn::xSelectFastestCalculationPath(const int32 BitDepth, const int32V4& CmpWeights, const int32V4& GlobalColorShift)
{
  assert(CmpWeights[0] >= 0 && CmpWeights[0] <= 64 && CmpWeights[1] >= 0 && CmpWeights[1] <= 64 && CmpWeights[2] >= 0 && CmpWeights[2] <= 64);
  if(BitDepth > 14) { return eCalcPath::STD; }

  const int64 MaxPelValue = ((int64)1 << BitDepth) - 1;
  const int64 MaxAbsShift = GlobalColorShift.getMaxAbs();
  assert(MaxAbsShift <= MaxPelValue);

  const int64 MaxAbsDiff = MaxPelValue + MaxAbsShift;           // <= 32766
  const int64 SumWeight  = CmpWeights.getSum();                 // <= 192
  const int64 MaxWeight  = CmpWeights.getMax();                 // <= 64
  const int64 MaxError   = SumWeight * MaxAbsDiff * MaxAbsDiff; // < 2^38
  if(MaxError >= 0xFFFFFFFF             ) { return eCalcPath::STD; }
  if(MaxWeight * MaxAbsDiff <= INT16_MAX) { return eCalcPath::I16; }
  return eCalcPath::U32;
}
int32 xCorrespPixelShiftCmn::xFindBestPixelWithinBlock(const int32V4& TstPel, const xPicP* Ref, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 BegY = CenterY - SearchRange;
  const int32 EndY = CenterY + SearchRange;
  const int32 BegX = CenterX - SearchRange;
  const int32 EndX = CenterX + SearchRange;

  const uint16* RefPtrLm = Ref->getAddr  (eCmp::LM);
  const uint16* RefPtrCb = Ref->getAddr  (eCmp::CB);
  const uint16* RefPtrCr = Ref->getAddr  (eCmp::CR);
  const int32   Stride   = Ref->getStride();

  int64 BestError  = std::numeric_limits<int64>::max();
  int32 BestOffset = NOT_VALID;

  for(int32 y = BegY; y <= EndY; y++)
  {
    for(int32 x = BegX; x <= EndX; x++)
    {
      const int32 Offset = y * Stride + x;
      const int64 DistLm = xPow2<int64>(TstPel[0] - (int32)(RefPtrLm[Offset]));
      const int64 DistCb = xPow2<int64>(TstPel[1] - (int32)(RefPtrCb[Offset]));
      const int64 DistCr = xPow2<int64>(TstPel[2] - (int32)(RefPtrCr[Offset]));
      const int64 Error = DistLm * CmpWeights[0] + DistCb * CmpWeights[1] + DistCr * CmpWeights[2];
      if(Error < BestError) { BestError = Error; BestOffset = Offset; }
    } //x
  } //y

  return BestOffset;
}
void xCorrespPixelShiftCmn::xGenShftCompRowTail(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, const int32 BegX)
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

  for(int32 x = BegX; x < Width; x++)
  {
    const int32V4 CurrTstValue  = int32V4((int32)(TstPtrLm[x]), (int32)(TstPtrCb[x]), (int32)(TstPtrCr[x]), 0) + GlobalColorShift;
    const int32   BestRefOffset = xFindBestPixelWithinBlock(CurrTstValue, Ref, x, y, SearchRange, CmpWeights);
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

//===============================================================================================================================================================================================================

} //end of namespace PMBB