/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define PMBB_STRUCTSIM_IMPLEMENTATION
#include "xStructSim.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xStructSim
//===============================================================================================================================================================================================================

template <class fltTP> fltTP xStructSim<fltTP>::CalcPel(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, const tFltrF& Filter, fltTP C1, fltTP C2)
{
  fltTP SumR = 0, SumT = 0, SumR2 = 0, SumT2 = 0, SumRT = 0;

  for(int32 dy = -c_FilterRange; dy < c_FilterRange; dy++)
  {
    for(int32 dx = -c_FilterRange; dx < c_FilterRange; dx++)
    {
      fltTP R = Ref[dy * StrideR + dx];
      fltTP T = Tst[dy * StrideT + dx];
      fltTP C = Filter[dy + c_FilterRange][dx + c_FilterRange];
      SumR  += R        * C;
      SumT  += T        * C;
      SumR2 += xPow2(R) * C;
      SumT2 += xPow2(T) * C;
      SumRT += R*T      * C;
    }
  }  

  fltTP AvgR  = SumR;
  fltTP AvgT  = SumT;
  fltTP VarR2 = SumR2 - xPow2(AvgR);
  fltTP VarT2 = SumT2 - xPow2(AvgT);
  fltTP CovRT = SumRT - AvgR*AvgT;

  fltTP L    = (2 * AvgR * AvgT + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1); //"Luminance"
  fltTP CS   = (2 * CovRT       + C2) / (VarR2       + VarT2       + C2); //"Contrast"*"Similarity"
  fltTP SSIM = L * CS;
  return SSIM;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template class xStructSim<flt32>;
template class xStructSim<flt64>;

//===============================================================================================================================================================================================================

} //end of namespace PMBB