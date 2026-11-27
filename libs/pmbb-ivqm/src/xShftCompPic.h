/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefIVQM.h"
#include "xCorrespPixelShift.h"
#include "xCorrespPixelShiftPrms.h"
#include "xCorrespPixelShiftCmn.h"
#include "xMultiThreaded.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// (corresponding pixel) Shift Compensated Picture
//===============================================================================================================================================================================================================

class xShftCompPic : public xCorrespPixelShiftPrms, public xMultiThreaded
{
protected:
  using tCP = xCorrespPixelShiftCmn::eCalcPath;
  tCP m_CalcPath = tCP::STD;

public:
  //Corresponding Pixel Shift
  void GenShftCompPics (xPicP* DstRef, xPicP* DstTst, const xPicP* SrcRef, const xPicP* SrcTst,                   const int32V4& GlobalColorDiffRef2Tst);
  void GenShftCompPics (xPicI* DstRef, xPicI* DstTst, const xPicI* SrcRef, const xPicI* SrcTst,                   const int32V4& GlobalColorDiffRef2Tst);
  void GenShftCompPicsM(xPicP* DstRef, xPicP* DstTst, const xPicP* SrcRef, const xPicP* SrcTst, const xPicP* Msk, const int32V4& GlobalColorDiffRef2Tst);
  void GenShftCompPicsM(xPicI* DstRef, xPicI* DstTst, const xPicI* SrcRef, const xPicI* SrcTst, const xPicP* Msk, const int32V4& GlobalColorDiffRef2Tst);

protected:
  void xGenShftCompPic (xPicP* DstRef, const xPicP* Ref, const xPicP* Tst,                   const int32V4& GlobalColorShift);
  void xGenShftCompPic (xPicI* DstRef, const xPicI* Ref, const xPicI* Tst,                   const int32V4& GlobalColorShift);
  void xGenShftCompPicM(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const xPicP* Msk, const int32V4& GlobalColorShift);
  void xGenShftCompPicM(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32V4& GlobalColorShift);

  static void xGenShftCompRng (xPicP* DstRef, const xPicP* Ref, const xPicP* Tst,                   const int32 BegY, const int32 EndY, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, tCP CalcPath);
  static void xGenShftCompRng (xPicI* DstRef, const xPicI* Ref, const xPicI* Tst,                   const int32 BegY, const int32 EndY, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
  static void xGenShftCompRngM(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const xPicP* Msk, const int32 BegY, const int32 EndY, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
  static void xGenShftCompRngM(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32 BegY, const int32 EndY, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB