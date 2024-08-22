/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefIVQM.h"
#include "xCorrespPixelShift.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// (corresponding pixel) Shift Compensated Picture
//===============================================================================================================================================================================================================

class xShftCompPic 
{
public:
  static void GenShftCompPics(xPicP* DstRef, xPicP* DstTst, const xPicP* SrcRef, const xPicP* SrcTst, const int32V4& GlobalColorDiffRef2Tst, const int32 SearchRange, const int32V4& CmpWeights, tThPI* TPI = nullptr);
  static void GenShftCompPics(xPicI* DstRef, xPicI* DstTst, const xPicI* SrcRef, const xPicI* SrcTst, const int32V4& GlobalColorDiffRef2Tst, const int32 SearchRange, const int32V4& CmpWeights, tThPI* TPI = nullptr);

protected:
  static void xGenShftCompPic(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, tThPI* TPI);
  static void xGenShftCompPic(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, tThPI* TPI);

  static void xGenShftCompRow(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
  static void xGenShftCompRow(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights);
};

//===============================================================================================================================================================================================================

class xShftCompPicProc : public xCorrespPixelShiftPrms, public xMultiThreaded
{
public:
  inline void GenShftCompPics(xPicP* DstRef, xPicP* DstTst, const xPicP* SrcRef, const xPicP* SrcTst, const int32V4& GlobalColorDiffRef2Tst)
  {
    xShftCompPic::GenShftCompPics(DstRef, DstTst, SrcRef, SrcTst, GlobalColorDiffRef2Tst, m_SearchRange, m_CmpWeightsSearch, &m_ThPI);
  }
  inline void GenShftCompPics(xPicI* DstRef, xPicI* DstTst, const xPicI* SrcRef, const xPicI* SrcTst, const int32V4& GlobalColorDiffRef2Tst)
  {
    xShftCompPic::GenShftCompPics(DstRef, DstTst, SrcRef, SrcTst, GlobalColorDiffRef2Tst, m_SearchRange, m_CmpWeightsSearch, &m_ThPI);
  }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB