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
void xShftCompPic::GenShftCompPicsM(xPicI* DstRef, xPicI* DstTst, const xPicI* SrcRef, const xPicI* SrcTst, const xPicP* Msk, const int32V4& GlobalColorDiffRef2Tst, const int32 SearchRange, const int32V4& CmpWeights, tThPI* TPI)
{
  assert(DstRef != nullptr && DstTst != nullptr && SrcRef != nullptr && SrcTst != nullptr);
  assert(DstRef->isCompatible(DstTst) && DstRef->isCompatible(SrcRef) && DstRef->isCompatible(SrcTst));
  assert(DstRef->isSameSize(Msk));

  const int32V4 GlobalColorDiffTst2Ref = -GlobalColorDiffRef2Tst;

  xGenShftCompPicM(DstRef, SrcRef, SrcTst, Msk, GlobalColorDiffRef2Tst, SearchRange, CmpWeights, TPI); //TODO check GlobalColorDiffRef2Tst
  xGenShftCompPicM(DstTst, SrcTst, SrcRef, Msk, GlobalColorDiffTst2Ref, SearchRange, CmpWeights, TPI);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xShftCompPic::xGenShftCompPic(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, tThPI* TPI)
{
  const int32 Height = Ref->getHeight();

  if(TPI != nullptr && TPI->isActive())
  {
    for(int32 y = 0; y < Height; y += xMultiThreaded::c_NumRowsInRng)
    {
      TPI->storeTask([&DstRef, &Tst, &Ref, &GlobalColorShift, &SearchRange, &CmpWeights, y, Height](int32 /*ThreadIdx*/) { xGenShftCompRng(DstRef, Ref, Tst, y, xMin(y + xMultiThreaded::c_NumRowsInRng, Height), GlobalColorShift, SearchRange, CmpWeights); });
    }
    TPI->executeStoredTasks();
  }
  else
  {
    xGenShftCompRng(DstRef, Ref, Tst, 0, Height, GlobalColorShift, SearchRange, CmpWeights);
  }
}
void xShftCompPic::xGenShftCompPic(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, tThPI* TPI)
{
  const int32 Height = Ref->getHeight();

  if(TPI != nullptr && TPI->isActive())
  {
    for(int32 y = 0; y < Height; y += xMultiThreaded::c_NumRowsInRng)
    {
      TPI->storeTask([&DstRef, &Tst, &Ref, &GlobalColorShift, &SearchRange, &CmpWeights, y, Height](int32 /*ThreadIdx*/) { xGenShftCompRng(DstRef, Ref, Tst, y, xMin(y + xMultiThreaded::c_NumRowsInRng, Height), GlobalColorShift, SearchRange, CmpWeights); });
    }
    TPI->executeStoredTasks();
  }
  else
  {
    xGenShftCompRng(DstRef, Ref, Tst, 0, Height, GlobalColorShift, SearchRange, CmpWeights);
  }
}
void xShftCompPic::xGenShftCompPicM(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, tThPI* TPI)
{
  const int32 Height = Ref->getHeight();

  if(TPI != nullptr && TPI->isActive())
  {
    for(int32 y = 0; y < Height; y += xMultiThreaded::c_NumRowsInRng)
    {
      TPI->storeTask([&DstRef, &Tst, &Ref, &Msk, &GlobalColorShift, &SearchRange, &CmpWeights, y, Height](int32 /*ThreadIdx*/) { xGenShftCompRngM(DstRef, Ref, Tst, Msk, y, xMin(y + xMultiThreaded::c_NumRowsInRng, Height), GlobalColorShift, SearchRange, CmpWeights); });
    }
    TPI->executeStoredTasks();
  }
  else
  {
    xGenShftCompRngM(DstRef, Ref, Tst, Msk, 0, Height, GlobalColorShift, SearchRange, CmpWeights);
  }
}
void xShftCompPic::xGenShftCompRng(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 BegY, const int32 EndY, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  for(int32 y = BegY; y < EndY; y++)
  {
    xCorrespPixelShift::GenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights);
  }
}
void xShftCompPic::xGenShftCompRng(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const int32 BegY, const int32 EndY, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  for(int32 y = BegY; y < EndY; y++)
  {
    xCorrespPixelShift::GenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights);
  }
}
void xShftCompPic::xGenShftCompRngM(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32 BegY, const int32 EndY, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  for(int32 y = BegY; y < EndY; y++)
  {
    xCorrespPixelShift::GenShftCompRowM(DstRef, Ref, Tst, Msk, y, GlobalColorShift, SearchRange, CmpWeights);
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB