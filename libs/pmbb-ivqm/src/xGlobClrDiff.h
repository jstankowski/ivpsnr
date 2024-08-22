/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVQM.h"
#include "xPic.h"
#include "xThreadPool.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// Global Color Difference
//===============================================================================================================================================================================================================

class xGlobClrDiffPrms
{
public:
  static constexpr flt32V4 c_DefaultUnntcbCoef = { 0.01f, 0.01f, 0.01f, 0.0f };
protected:
  flt32V4 m_CmpUnntcbCoef = c_DefaultUnntcbCoef;
public:
  void  setUnntcbCoef(const flt32V4& UnntcbCoef) { m_CmpUnntcbCoef = UnntcbCoef; }
};

//===============================================================================================================================================================================================================

class xGlobClrDiff
{
public:
  static int32V4 CalcGlobalColorDiff (const xPicP* Tst, const xPicP* Ref,                   const flt32V4& CmpUnntcbCoef,                           tThPI* TPI = nullptr);
  static int32V4 CalcGlobalColorDiffM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const flt32V4& CmpUnntcbCoef, const int32 NumNonMasked, tThPI* TPI = nullptr);
};

//===============================================================================================================================================================================================================

class xGlobClrDiffProc : public xGlobClrDiffPrms, public xMultiThreaded
{
public:
  inline int32V4 CalcGlobalColorDiff (const xPicP* Tst, const xPicP* Ref                                            ) { return xGlobClrDiff::CalcGlobalColorDiff (Ref, Tst,      m_CmpUnntcbCoef,               &m_ThPI); }
  inline int32V4 CalcGlobalColorDiffM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32 NumNonMasked) { return xGlobClrDiff::CalcGlobalColorDiffM(Ref, Tst, Msk, m_CmpUnntcbCoef, NumNonMasked, &m_ThPI); }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
