/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefPMBB-CORE.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xKBNS //Kahan Babuska Neumaier Sumation
{
protected:
  flt64 s = 0.0;
  flt64 c = 0.0;

public:
  inline void init() { s = 0.0; c = 0.0; }

  inline void acc(flt64 v)
  {
    flt64 t = s + v;
    if(xAbs(s) >= xAbs(v)) { c += ((s - t) + v); }
    else                   { c += ((v - t) + s); }
    s = t;
  }

  inline flt64 get() const { return s + c; }
};

class xMathUtils
{
public:
  static flt64 KahanBabuskaNeumaierSumation(const flt64* x, const int32 n);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB