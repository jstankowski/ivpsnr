/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xGlobalColorShift.h"
#include "xDistortion.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// global color shift
//===============================================================================================================================================================================================================

int32V4 xGlobalColorShift::CalcGlobalColorShift(const xPicP* Ref, const xPicP* Tst, const flt32V4& CmpUnntcbCoef, tThPI* ThreadPoolIf)
{
  const int32   MaxValue = Ref->getMaxPelValue();
  const int32V4 MaxDiff  = xRoundFltToInt32(CmpUnntcbCoef * (flt32)MaxValue);

  flt64V4 AvgColorDiff = { 0, 0, 0, 0 };

  if(ThreadPoolIf && ThreadPoolIf->isActive())
  {
    for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
      ThreadPoolIf->addWaitingTask([&AvgColorDiff, &Tst, &Ref, CmpIdx](int32 /*ThreadIdx*/)
        { AvgColorDiff[CmpIdx] = xCalcAvgColorDiff(Ref->getAddr((eCmp)CmpIdx), Tst->getAddr((eCmp)CmpIdx), Ref->getStride(), Tst->getStride(), Ref->getWidth(), Ref->getHeight()); }
      );
    }
    ThreadPoolIf->waitUntilTasksFinished(3);
  }
  else
  {
    for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
      AvgColorDiff[CmpIdx] = xCalcAvgColorDiff(Ref->getAddr((eCmp)CmpIdx), Tst->getAddr((eCmp)CmpIdx), Ref->getStride(), Tst->getStride(), Ref->getWidth(), Ref->getHeight());
    }
  }

  int32V4 GlobalColorShift = xRoundFltToInt32(AvgColorDiff);
  GlobalColorShift.modClip(-MaxDiff, MaxDiff);

  return GlobalColorShift;
}
flt64 xGlobalColorShift::xCalcAvgColorDiff(const uint16* RefPtr, const uint16* TstPtr, const int32 RefStride, const int32 TstStride, const int32 Width, const int32 Height)
{
  int32 SumColorDiff = xDistortion::CalcSD(RefPtr, TstPtr, RefStride, TstStride, Width, Height);
  int32 Area         = Width * Height;
  flt64 AvgColorDiff = (flt64)SumColorDiff / (flt64)Area;
  return AvgColorDiff;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int32V4 xGlobalColorShift::CalcGlobalColorShiftM(const xPicP* Ref, const xPicP* Tst, const xPicP* Msk, const flt32V4& CmpUnntcbCoef, const int32 NumNonMasked, tThPI* ThreadPoolIf)
{
  const int32   MaxValue = Ref->getMaxPelValue();
  const int32V4 MaxDiff  = xRoundFltToInt32(CmpUnntcbCoef * (flt32)MaxValue);

  int64V4 SumColorDiff = xMakeVec4<int64>(0);

  if(ThreadPoolIf && ThreadPoolIf->isActive())
  {
    for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
      ThreadPoolIf->addWaitingTask([&SumColorDiff, &Tst, &Ref, &Msk, CmpIdx](int32 /*ThreadIdx*/)
        { SumColorDiff[CmpIdx] = xCalcSumColorDiffM(Ref->getAddr((eCmp)CmpIdx), Tst->getAddr((eCmp)CmpIdx), Msk->getAddr(eCmp::LM), Ref->getStride(), Tst->getStride(), Msk->getStride(), Ref->getWidth(), Ref->getHeight()); }
      );
    }
    ThreadPoolIf->waitUntilTasksFinished(3);
  }
  else
  {
    for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
      SumColorDiff[CmpIdx] = xCalcSumColorDiffM(Ref->getAddr((eCmp)CmpIdx), Tst->getAddr((eCmp)CmpIdx), Msk->getAddr(eCmp::LM), Ref->getStride(), Tst->getStride(), Msk->getStride(), Ref->getWidth(), Ref->getHeight());
    }
  }

  flt64V4 AvgColorDiff     = (flt64V4)SumColorDiff / (flt64)((int64)NumNonMasked * (int64)(Msk->getMaxPelValue()));
  int32V4 GlobalColorShift = xRoundFltToInt32(AvgColorDiff);
  GlobalColorShift.modClip(-MaxDiff, MaxDiff);

  return GlobalColorShift;
}
int64 xGlobalColorShift::xCalcSumColorDiffM(const uint16* RefPtr, const uint16* TstPtr, const uint16* MskPtr, const int32 RefStride, const int32 TstStride, const int32 MskStride, const int32 Width, const int32 Height)
{
  int64 SumColorDiff = xDistortion::CalcWeightedSD(RefPtr, TstPtr, MskPtr, RefStride, TstStride, MskStride, Width, Height);
  return SumColorDiff;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB