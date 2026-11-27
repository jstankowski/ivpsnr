/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xGlobClrDiff.h"
#include "xDistortion.h"
#include "xHelpersFLT.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// special rounding routines
//===============================================================================================================================================================================================================
template <class XXX> static inline int32V4 xRoundVecFltToI32(const xVec4<XXX>& FltV);
inline int32V4 xRoundVecF32ToI32(const xVec4<flt32>& FltV) { return { xRoundF32ToI32(FltV[0]), xRoundF32ToI32(FltV[1]), xRoundF32ToI32(FltV[2]), xRoundF32ToI32(FltV[3]) }; }
inline int32V4 xRoundVecF64ToI32(const xVec4<flt64>& FltV) { return { xRoundF64ToI32(FltV[0]), xRoundF64ToI32(FltV[1]), xRoundF64ToI32(FltV[2]), xRoundF64ToI32(FltV[3]) }; }

//===============================================================================================================================================================================================================
// Global Color Difference - xGlobClrDiff
//===============================================================================================================================================================================================================

int32V4 xGlobClrDiff::CalcGlobalColorDiff(const xPicP* Tst, const xPicP* Ref, const flt32V4& CmpUnntcbCoef, tThPI* TPI)
{
  assert(Ref->isCompatible(Tst));

  const int32   NumCmps  = Ref->getNumCmps();
  const int32   MaxValue = Ref->getMaxPelValue();
  const int32V4 MaxDiff  = xRoundVecF32ToI32(CmpUnntcbCoef * (flt32)MaxValue);
  const flt64   Area     = Ref->getArea();

  int64V4 SumColorDiff = xMakeVec4<int64>(0);
  if(TPI && TPI->isActive())
  {
    for(int32 CmpIdx = 0; CmpIdx < NumCmps; CmpIdx++)
    {
      TPI->storeTask([&SumColorDiff, &Tst, &Ref, CmpIdx](int32 /*ThreadIdx*/)
        { SumColorDiff[CmpIdx] = xDistortion::CalcSD(Tst->getAddr((eCmp)CmpIdx), Ref->getAddr((eCmp)CmpIdx), Tst->getStride(), Ref->getStride(), Ref->getWidth(), Ref->getHeight(), Ref->getBitDepth()); }
      );
    }
    TPI->executeStoredTasks();
  }
  else
  {
    for(int32 CmpIdx = 0; CmpIdx < NumCmps; CmpIdx++)
    {
      SumColorDiff[CmpIdx] = xDistortion::CalcSD(Tst->getAddr((eCmp)CmpIdx), Ref->getAddr((eCmp)CmpIdx), Tst->getStride(), Ref->getStride(), Ref->getWidth(), Ref->getHeight(), Ref->getBitDepth());
    }
  }

  flt64V4 AvgColorDiff     = (flt64V4)SumColorDiff / Area;
  int32V4 GlobalColorShift = xRoundVecF64ToI32(AvgColorDiff);
  GlobalColorShift.modClip(-MaxDiff, MaxDiff);

  return GlobalColorShift;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int32V4 xGlobClrDiff::CalcGlobalColorDiffM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const flt32V4& CmpUnntcbCoef, const int32 NumNonMasked, tThPI* TPI)
{
  const int32   NumCmps  = Ref->getNumCmps();
  const int32   MaxValue = Ref->getMaxPelValue();
  const int32V4 MaxDiff  = xRoundVecF32ToI32(CmpUnntcbCoef * (flt32)MaxValue);

  int64V4 SumColorDiff = xMakeVec4<int64>(0);

  if(TPI && TPI->isActive())
  {
    for(int32 CmpIdx = 0; CmpIdx < NumCmps; CmpIdx++)
    {
      TPI->storeTask([&SumColorDiff, &Tst, &Ref, &Msk, CmpIdx](int32 /*ThreadIdx*/)
        { SumColorDiff[CmpIdx] = xDistortion::CalcWghtSD(Tst->getAddr((eCmp)CmpIdx), Ref->getAddr((eCmp)CmpIdx), Msk->getAddr(eCmp::LM), Tst->getStride(), Ref->getStride(), Msk->getStride(), Ref->getWidth(), Ref->getHeight()); }
      );
    }
    TPI->executeStoredTasks();
  }
  else
  {
    for(int32 CmpIdx = 0; CmpIdx < NumCmps; CmpIdx++)
    {
      SumColorDiff[CmpIdx] = xDistortion::CalcWghtSD(Tst->getAddr((eCmp)CmpIdx), Ref->getAddr((eCmp)CmpIdx), Msk->getAddr(eCmp::LM), Tst->getStride(), Ref->getStride(), Msk->getStride(), Ref->getWidth(), Ref->getHeight());
    }
  }

  flt64V4 AvgColorDiff     = (flt64V4)SumColorDiff / (flt64)((int64)NumNonMasked * (int64)(Msk->getMaxPelValue()));
  int32V4 GlobalColorShift = xRoundVecF64ToI32(AvgColorDiff);
  GlobalColorShift.modClip(-MaxDiff, MaxDiff);

  return GlobalColorShift;
}

//===============================================================================================================================================================================================================


} //end of namespace PMBB