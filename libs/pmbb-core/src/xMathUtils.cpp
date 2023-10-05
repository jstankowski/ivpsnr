/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xMathUtils.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

flt64 xMathUtils::KahanBabuskaNeumaierSumation(const flt64* x, const uintSize n)
{
  if(n == 0) { return 0; }

  flt64 s = x[0];
  flt64 c = 0;
  for(uintSize i = 1; i < n; i++)
  {
    flt64 t = s + x[i];
    if(xAbs(s) >= xAbs(x[i])) { c += ((s - t) + x[i]); }
    else                      { c += ((x[i] - t) + s); }
    s = t;
  }
  return s + c;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB