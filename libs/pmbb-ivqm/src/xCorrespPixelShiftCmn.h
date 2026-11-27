/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVQM.h"
#include "xPic.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xCorrespPixelShiftCmn
{
public:
  // I16 - fastest but significantly limited by overflow possibility
  // U32 - fast, with less limitation, overfow still possible
  // STD - portable, always works

  //Note: CmpWeights in range 0 - 64, abs(GlobalColorShift) <= MaxPelValue 

  enum class eCalcPath : int32 { STD, I16, U32 };

  static eCalcPath xSelectFastestCalculationPath(const int32 BitDepth, const int32V4& CmpWeights, const int32V4& GlobalColorShift);

protected:
  static int32 xFindBestPixelWithinBlock(const int32V4& TstPel, const xPicP* Ref, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights);
  
  static void  xGenShftCompRowTail(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, const int32 BegX);


};

//===============================================================================================================================================================================================================

} //end of namespace PMBB