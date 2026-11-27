/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xShftCompPic.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xShftCompPic::GenShftCompPics(xPicP* DstRef, xPicP* DstTst, const xPicP* SrcRef, const xPicP* SrcTst, const int32V4& GlobalColorDiffRef2Tst)
{
  assert(DstRef != nullptr && DstTst != nullptr && SrcRef != nullptr && SrcTst != nullptr);
  assert(DstRef->isCompatible(DstTst) && DstRef->isCompatible(SrcRef) && DstRef->isCompatible(SrcTst));

  m_CalcPath = xCorrespPixelShiftCmn::xSelectFastestCalculationPath(SrcRef->getBitDepth(), m_CmpWeightsSearch, GlobalColorDiffRef2Tst);

  const int32V4 GlobalColorDiffTst2Ref = -GlobalColorDiffRef2Tst;
  xGenShftCompPic(DstRef, SrcRef, SrcTst, GlobalColorDiffRef2Tst); //TODO check GlobalColorDiffRef2Tst
  xGenShftCompPic(DstTst, SrcTst, SrcRef, GlobalColorDiffTst2Ref);
}
void xShftCompPic::GenShftCompPics(xPicI* DstRef, xPicI* DstTst, const xPicI* SrcRef, const xPicI* SrcTst, const int32V4& GlobalColorDiffRef2Tst)
{
  assert(DstRef != nullptr && DstTst != nullptr && SrcRef != nullptr && SrcTst != nullptr);
  assert(DstRef->isCompatible(DstTst) && DstRef->isCompatible(SrcRef) && DstRef->isCompatible(SrcTst));

  const int32V4 GlobalColorDiffTst2Ref = -GlobalColorDiffRef2Tst;
  xGenShftCompPic(DstRef, SrcRef, SrcTst, GlobalColorDiffRef2Tst); //TODO check GlobalColorDiffRef2Tst
  xGenShftCompPic(DstTst, SrcTst, SrcRef, GlobalColorDiffTst2Ref);
}
void xShftCompPic::GenShftCompPicsM(xPicP* DstRef, xPicP* DstTst, const xPicP* SrcRef, const xPicP* SrcTst, const xPicP* Msk, const int32V4& GlobalColorDiffRef2Tst)
{
  assert(DstRef != nullptr && DstTst != nullptr && SrcRef != nullptr && SrcTst != nullptr);
  assert(DstRef->isCompatible(DstTst) && DstRef->isCompatible(SrcRef) && DstRef->isCompatible(SrcTst));
  assert(DstRef->isSameSize(Msk));

  const int32V4 GlobalColorDiffTst2Ref = -GlobalColorDiffRef2Tst;
  xGenShftCompPicM(DstRef, SrcRef, SrcTst, Msk, GlobalColorDiffRef2Tst); //TODO check GlobalColorDiffRef2Tst
  xGenShftCompPicM(DstTst, SrcTst, SrcRef, Msk, GlobalColorDiffTst2Ref);
}
void xShftCompPic::GenShftCompPicsM(xPicI* DstRef, xPicI* DstTst, const xPicI* SrcRef, const xPicI* SrcTst, const xPicP* Msk, const int32V4& GlobalColorDiffRef2Tst)
{
  assert(DstRef != nullptr && DstTst != nullptr && SrcRef != nullptr && SrcTst != nullptr);
  assert(DstRef->isCompatible(DstTst) && DstRef->isCompatible(SrcRef) && DstRef->isCompatible(SrcTst));
  assert(DstRef->isSameSize(Msk));

  const int32V4 GlobalColorDiffTst2Ref = -GlobalColorDiffRef2Tst;
  xGenShftCompPicM(DstRef, SrcRef, SrcTst, Msk, GlobalColorDiffRef2Tst); //TODO check GlobalColorDiffRef2Tst
  xGenShftCompPicM(DstTst, SrcTst, SrcRef, Msk, GlobalColorDiffTst2Ref);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xShftCompPic::xGenShftCompPic(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32V4& GlobalColorShift)
{
  const int32 Height = Ref->getHeight();

  if(m_ThPI != nullptr && m_ThPI->isActive())
  {
    for(int32 y = 0; y < Height; y += xMultiThreaded::c_NumRowsInRng)
    {
      m_ThPI->storeTask([this, &DstRef, &Tst, &Ref, &GlobalColorShift, y, Height](int32 /*ThreadIdx*/)
        { 
          xGenShftCompRng(DstRef, Ref, Tst, y, xMin(y + xMultiThreaded::c_NumRowsInRng, Height), GlobalColorShift, m_SearchRange, m_CmpWeightsSearch, m_CalcPath);
        });
    }
    m_ThPI->executeStoredTasks();
  }
  else
  {
    xGenShftCompRng(DstRef, Ref, Tst, 0, Height, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch, m_CalcPath);
  }
}
void xShftCompPic::xGenShftCompPic(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const int32V4& GlobalColorShift)
{
  const int32 Height = Ref->getHeight();

  if(m_ThPI != nullptr && m_ThPI->isActive())
  {
    for(int32 y = 0; y < Height; y += xMultiThreaded::c_NumRowsInRng)
    {
      m_ThPI->storeTask([this, &DstRef, &Tst, &Ref, &GlobalColorShift, y, Height](int32 /*ThreadIdx*/)
        {
          xGenShftCompRng(DstRef, Ref, Tst, y, xMin(y + xMultiThreaded::c_NumRowsInRng, Height), GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
        });
    }
    m_ThPI->executeStoredTasks();
  }
  else
  {
    xGenShftCompRng(DstRef, Ref, Tst, 0, Height, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
  }
}
void xShftCompPic::xGenShftCompPicM(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const xPicP* Msk, const int32V4& GlobalColorShift)
{
  const int32 Height = Ref->getHeight();

  if(m_ThPI != nullptr && m_ThPI->isActive())
  {
    for(int32 y = 0; y < Height; y += xMultiThreaded::c_NumRowsInRng)
    {
      m_ThPI->storeTask([this, &DstRef, &Tst, &Ref, &Msk, &GlobalColorShift, y, Height](int32 /*ThreadIdx*/)
        {
          xGenShftCompRngM(DstRef, Ref, Tst, Msk, y, xMin(y + xMultiThreaded::c_NumRowsInRng, Height), GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
        });
    }
    m_ThPI->executeStoredTasks();
  }
  else
  {
    xGenShftCompRngM(DstRef, Ref, Tst, Msk, 0, Height, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
  }
}
void xShftCompPic::xGenShftCompPicM(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32V4& GlobalColorShift)
{
  const int32 Height = Ref->getHeight();

  if(m_ThPI != nullptr && m_ThPI->isActive())
  {
    for(int32 y = 0; y < Height; y += xMultiThreaded::c_NumRowsInRng)
    {
      m_ThPI->storeTask([this, &DstRef, &Tst, &Ref, &Msk, &GlobalColorShift, y, Height](int32 /*ThreadIdx*/)
        {
          xGenShftCompRngM(DstRef, Ref, Tst, Msk, y, xMin(y + xMultiThreaded::c_NumRowsInRng, Height), GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
        });
    }
    m_ThPI->executeStoredTasks();
  }
  else
  {
    xGenShftCompRngM(DstRef, Ref, Tst, Msk, 0, Height, GlobalColorShift, m_SearchRange, m_CmpWeightsSearch);
  }
}
void xShftCompPic::xGenShftCompRng(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 BegY, const int32 EndY, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, tCP CalcPath)
{
  switch(CalcPath)
  {
  case tCP::I16: for(int32 y = BegY; y < EndY; y++) { xCorrespPixelShift::GenShftCompRowI16(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }; break;
  case tCP::U32: for(int32 y = BegY; y < EndY; y++) { xCorrespPixelShift::GenShftCompRowU32(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }; break;
  case tCP::STD: for(int32 y = BegY; y < EndY; y++) { xCorrespPixelShift::GenShftCompRowSTD(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }; break;
  }  
}
void xShftCompPic::xGenShftCompRng(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const int32 BegY, const int32 EndY, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  for(int32 y = BegY; y < EndY; y++) { xCorrespPixelShift::GenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights); }
}
void xShftCompPic::xGenShftCompRngM(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const xPicP* Msk, const int32 BegY, const int32 EndY, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  for(int32 y = BegY; y < EndY; y++) { xCorrespPixelShift::GenShftCompRowM(DstRef, Ref, Tst, Msk, y, GlobalColorShift, SearchRange, CmpWeights); }
}
void xShftCompPic::xGenShftCompRngM(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32 BegY, const int32 EndY, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  for(int32 y = BegY; y < EndY; y++) { xCorrespPixelShift::GenShftCompRowM(DstRef, Ref, Tst, Msk, y, GlobalColorShift, SearchRange, CmpWeights); }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB