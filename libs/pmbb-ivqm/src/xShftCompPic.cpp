/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xShftCompPic.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xShftCompPic::GenShftCompPics(xPicP* DstRef, xPicP* DstTst, const xPicP* SrcRef, const xPicP* SrcTst, const int32V4& GlobalColorDiffRef2Tst, const int32 SearchRange, const int32V4& CmpWeights, tThPI* TPI)
{
  assert(DstRef != nullptr && DstTst != nullptr && SrcRef != nullptr && SrcTst != nullptr);
  assert(DstRef->isCompatible(DstTst) && DstRef->isCompatible(SrcRef) && DstRef->isCompatible(SrcTst));

  const int32V4 GlobalColorDiffTst2Ref = -GlobalColorDiffRef2Tst;

  xGenShftCompPic(DstRef, SrcRef, SrcTst, GlobalColorDiffRef2Tst, SearchRange, CmpWeights, TPI); //TODO check GlobalColorDiffRef2Tst
  xGenShftCompPic(DstTst, SrcTst, SrcRef, GlobalColorDiffTst2Ref, SearchRange, CmpWeights, TPI);
}
void xShftCompPic::GenShftCompPics(xPicI* DstRef, xPicI* DstTst, const xPicI* SrcRef, const xPicI* SrcTst, const int32V4& GlobalColorDiffRef2Tst, const int32 SearchRange, const int32V4& CmpWeights, tThPI* TPI)
{
  assert(DstRef != nullptr && DstTst != nullptr && SrcRef != nullptr && SrcTst != nullptr);
  assert(DstRef->isCompatible(DstTst) && DstRef->isCompatible(SrcRef) && DstRef->isCompatible(SrcTst));

  const int32V4 GlobalColorDiffTst2Ref = -GlobalColorDiffRef2Tst;

  xGenShftCompPic(DstRef, SrcRef, SrcTst, GlobalColorDiffRef2Tst, SearchRange, CmpWeights, TPI); //TODO check GlobalColorDiffRef2Tst
  xGenShftCompPic(DstTst, SrcTst, SrcRef, GlobalColorDiffTst2Ref, SearchRange, CmpWeights, TPI);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xShftCompPic::xGenShftCompPic(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, tThPI* TPI)
{
  const int32 Height = Ref->getHeight();

  if(TPI != nullptr && TPI->isActive())
  {
    for(int32 y = 0; y < Height; y++)
    {
      TPI->addWaitingTask([&DstRef, &Tst, &Ref, &GlobalColorShift, &SearchRange, &CmpWeights, y](int32 /*ThreadIdx*/) { xGenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); });
    }
    TPI->waitUntilTasksFinished(Height);
  }
  else
  {
    for(int32 y = 0; y < Height; y++) { xGenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
  }
}
void xShftCompPic::xGenShftCompPic(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, tThPI* TPI)
{
  const int32 Height = Ref->getHeight();

  if(TPI != nullptr && TPI->isActive())
  {
    for(int32 y = 0; y < Height; y++)
    {
      TPI->addWaitingTask([&DstRef, &Tst, &Ref, &GlobalColorShift, &SearchRange, &CmpWeights, y](int32 /*ThreadIdx*/) { xGenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); });
    }
    TPI->waitUntilTasksFinished(Height);
  }
  else
  {
    for(int32 y = 0; y < Height; y++) { xGenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
  }
}
void xShftCompPic::xGenShftCompRow(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32  Width     = Tst->getWidth ();
  const int32  TstStride = Tst->getStride();
  const int32  TstOffset = y * TstStride;
  const int32  MaxValue  = DstRef->getMaxPelValue();

  const uint16*    TstPtrLm = Tst   ->getAddr(eCmp::LM) + TstOffset;
  const uint16*    TstPtrCb = Tst   ->getAddr(eCmp::CB) + TstOffset;
  const uint16*    TstPtrCr = Tst   ->getAddr(eCmp::CR) + TstOffset;
  uint16* restrict DstPtrLm = DstRef->getAddr(eCmp::LM) + TstOffset;
  uint16* restrict DstPtrCb = DstRef->getAddr(eCmp::CB) + TstOffset;
  uint16* restrict DstPtrCr = DstRef->getAddr(eCmp::CR) + TstOffset;

  for(int32 x = 0; x < Width; x++)
  {
    const int32V4 CurrTstValue  = int32V4((int32)(TstPtrLm[x]), (int32)(TstPtrCb[x]), (int32)(TstPtrCr[x]), 0) + GlobalColorShift;
    const int32   BestRefOffset = xCorrespPixelShift::xFindBestPixelWithinBlock(CurrTstValue, Ref, x, y, SearchRange, CmpWeights);
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
void xShftCompPic::xGenShftCompRow(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
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
    const int32    BestRefOffset = xCorrespPixelShiftSTD::FindBestPixelWithinBlock(CurrTstValue, Ref, x, y, SearchRange, CmpWeights);
    const int32V4  RefValue      = (int32V4)(Ref->getAddr()[BestRefOffset]);
    const uint16V4 DstValue      = (uint16V4)((RefValue - GlobalColorShift).getClipU(MaxValue));
    DstPtr[x] = DstValue;
  }//x
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB