/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xStructSimSTD.h"
#include "xStructSimConsts.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Regular (11x11) structural similarity
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
flt64 xStructSimSTD::CalcRglrFlt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 /*WndSize*/, flt64 C1, flt64 C2, bool CalcL)
{
  constexpr int32 FilterRange = xStructSimConsts::c_FilterRange;

  flt64 SumR = 0, SumT = 0, SumR2 = 0, SumT2 = 0, SumRT = 0;
  
  for(int32 dy = -FilterRange; dy <= FilterRange; dy++)
  {
    for(int32 dx = -FilterRange; dx <= FilterRange; dx++)
    {
      flt64 R = Ref[dy * StrideR + dx];
      flt64 T = Tst[dy * StrideT + dx];
      flt64 C = xStructSimConsts::c_FilterRglrGaussFlt[dy + FilterRange][dx + FilterRange];
      SumR  += R        * C;
      SumT  += T        * C;
      SumR2 += xPow2(R) * C;
      SumT2 += xPow2(T) * C;
      SumRT += R*T      * C;
    }
  }  

  flt64 AvgR  = SumR;
  flt64 AvgT  = SumT;
  flt64 VarR2 = SumR2 - xPow2(AvgR);
  flt64 VarT2 = SumT2 - xPow2(AvgT);
  flt64 CovRT = SumRT - AvgR*AvgT;

  // flt64 C3         = C2 / 2.0;
  // flt64 DevR2      = sqrt(VarR2);
  // flt64 DevT2      = sqrt(VarT2);
  // flt64 Luminance  = (2 * AvgR  * AvgT  + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1);
  // flt64 Contrast   = (2 * DevR2 * DevT2 + C2) / (VarR2       + VarT2       + C2);
  // flt64 Similarity = (CovRT             + C3) / (DevR2       * DevT2       + C3);
  // flt64 SSIM       = Luminance * Contrast * Similarity;

  if (CalcL)
  {
    flt64 L    = (2 * AvgR * AvgT + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1); //"Luminance"
    flt64 CS   = (2 * CovRT       + C2) / (VarR2       + VarT2       + C2); //"Contrast"*"Similarity"
    flt64 SSIM = L * CS;
    return SSIM;
  }
  else
  {
    flt64 CS = (2 * CovRT + C2) / (VarR2 + VarT2 + C2); //"Contrast"*"Similarity"
    return CS;
  }
}
flt64 xStructSimSTD::CalcRglrInt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 /*WndSize*/, flt64 C1, flt64 C2, bool CalcL)
{
  constexpr int32 FilterRange = xStructSimConsts::c_FilterRange;

  int64 SumR = 0, SumT = 0, SumR2 = 0, SumT2 = 0, SumRT = 0;

  for(int32 dy = -FilterRange; dy <= FilterRange; dy++)
  {
    for(int32 dx = -FilterRange; dx <= FilterRange; dx++)
    {
      int32 R = Ref[dy * StrideR + dx];
      int32 T = Tst[dy * StrideT + dx];
      int32 C = xStructSimConsts::c_FilterRglrGaussInt[dy + FilterRange][dx + FilterRange];
      SumR  += R                 * C;
      SumT  += T                 * C;
      SumR2 += xPow2<int64>(R)   * C;
      SumT2 += xPow2<int64>(T)   * C;
      SumRT += (int64)R*(int64)T * C;
    }
  }  

  flt64 AvgR  = ((flt64)SumR  * xStructSimConsts::c_InvFltrIntMul);
  flt64 AvgT  = ((flt64)SumT  * xStructSimConsts::c_InvFltrIntMul);
  flt64 VarR2 = ((flt64)SumR2 * xStructSimConsts::c_InvFltrIntMul) - xPow2(AvgR);
  flt64 VarT2 = ((flt64)SumT2 * xStructSimConsts::c_InvFltrIntMul) - xPow2(AvgT);
  flt64 CovRT = ((flt64)SumRT * xStructSimConsts::c_InvFltrIntMul) - AvgR*AvgT;

  if (CalcL)
  {
    flt64 L    = (2 * AvgR * AvgT + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1); //"Luminance"
    flt64 CS   = (2 * CovRT       + C2) / (VarR2       + VarT2       + C2); //"Contrast"*"Similarity"
    flt64 SSIM = L * CS;
    return SSIM;
  }
  else
  {
    flt64 CS = (2 * CovRT + C2) / (VarR2 + VarT2 + C2); //"Contrast"*"Similarity"
    return CS;
  }
}
flt64 xStructSimSTD::CalcRglrAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 /*WndSize*/, flt64 C1, flt64 C2, bool CalcL)
{
  constexpr int32 FilterRange = xStructSimConsts::c_FilterRange;

  int64 SumR = 0, SumT = 0, SumR2 = 0, SumT2 = 0, SumRT = 0;

  for(int32 dy = -FilterRange; dy <= FilterRange; dy++)
  {
    for(int32 dx = -FilterRange; dx <= FilterRange; dx++)
    {
      int32 R = Ref[dy * StrideR + dx];
      int32 T = Tst[dy * StrideT + dx];
      SumR  += R;
      SumT  += T;
      SumR2 += xPow2(R);
      SumT2 += xPow2(T);
      SumRT += R*T;
    }
  }  

  flt64 AvgR  = (flt64)SumR  * xStructSimConsts::c_InvFltrArea;
  flt64 AvgT  = (flt64)SumT  * xStructSimConsts::c_InvFltrArea;
  flt64 VarR2 = (flt64)SumR2 * xStructSimConsts::c_InvFltrArea - xPow2(AvgR);
  flt64 VarT2 = (flt64)SumT2 * xStructSimConsts::c_InvFltrArea - xPow2(AvgT);
  flt64 CovRT = (flt64)SumRT * xStructSimConsts::c_InvFltrArea - AvgR*AvgT;
  
  if (CalcL)
  {
    flt64 L    = (2 * AvgR * AvgT + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1); //"Luminance"
    flt64 CS   = (2 * CovRT       + C2) / (VarR2       + VarT2       + C2); //"Contrast"*"Similarity"
    flt64 SSIM = L * CS;
    return SSIM;
  }
  else
  {
    flt64 CS = (2 * CovRT + C2) / (VarR2 + VarT2 + C2); //"Contrast"*"Similarity"
    return CS;
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Regular (11x11) structural similarity - with mask
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#if X_PMBB_EXPERIMENTAL
flt64 xStructSimSTD::CalcRglrFltM(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 StrideT, int32 StrideR, int32 StrideM, int32 /*WndSize*/, flt64 C1, flt64 C2, bool CalcL)
{
  assert(*Msk != 0);
  constexpr int32 FilterRange = xStructSimConsts::c_FilterRange;

  flt64 SumR = 0, SumT = 0, SumR2 = 0, SumT2 = 0, SumRT = 0, SumC = 0.0;
  
  for(int32 dy = -FilterRange; dy <= FilterRange; dy++)
  {
    for(int32 dx = -FilterRange; dx <= FilterRange; dx++)
    {
      if(Msk[dy * StrideM + dx] == 0) { continue; }
      flt64  R = Ref[dy * StrideR + dx];
      flt64  T = Tst[dy * StrideT + dx];
      flt64  C = xStructSimConsts::c_FilterRglrGaussFlt[dy + FilterRange][dx + FilterRange];
      SumR  += R        * C;
      SumT  += T        * C;
      SumR2 += xPow2(R) * C;
      SumT2 += xPow2(T) * C;
      SumRT += R*T      * C;
      SumC  +=            C;
    }
  }

  flt64 InvC  = 1.0 / SumC;
  flt64 AvgR  = SumR  * InvC;
  flt64 AvgT  = SumT  * InvC;
  flt64 VarR2 = SumR2 * InvC - xPow2(AvgR);
  flt64 VarT2 = SumT2 * InvC - xPow2(AvgT);
  flt64 CovRT = SumRT * InvC - AvgR*AvgT;

  if (CalcL)
  {
    flt64 L    = (2 * AvgR * AvgT + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1); //"Luminance"
    flt64 CS   = (2 * CovRT       + C2) / (VarR2       + VarT2       + C2); //"Contrast"*"Similarity"
    flt64 SSIM = L * CS;
    return SSIM;
  }
  else
  {
    flt64 CS = (2 * CovRT + C2) / (VarR2 + VarT2 + C2); //"Contrast"*"Similarity"
    return CS;
  }
}
flt64 xStructSimSTD::CalcRglrIntM(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 StrideT, int32 StrideR, int32 StrideM, int32 /*WndSize*/, flt64 C1, flt64 C2, bool CalcL)
{
  constexpr int32 FilterRange = xStructSimConsts::c_FilterRange;

  int64 SumR = 0, SumT = 0, SumR2 = 0, SumT2 = 0, SumRT = 0, SumC = 0;

  for(int32 dy = -FilterRange; dy <= FilterRange; dy++)
  {
    for(int32 dx = -FilterRange; dx <= FilterRange; dx++)
    {
      if(Msk[dy * StrideM + dx] == 0) { continue; }
      int32 R = Ref[dy * StrideR + dx];
      int32 T = Tst[dy * StrideT + dx];
      int32 C = xStructSimConsts::c_FilterRglrGaussInt[dy + FilterRange][dx + FilterRange];
      SumR  += R                 * C;
      SumT  += T                 * C;
      SumR2 += xPow2<int64>(R)   * C;
      SumT2 += xPow2<int64>(T)   * C;
      SumRT += (int64)R*(int64)T * C;
      SumC  +=                     C;
    }
  }  

  assert(0);

  flt64 InvC  = (flt64)1.0 / ((flt64)xStructSimConsts::c_FltrIntMul );
 
  flt64 AvgR  = ((flt64)SumR  * xStructSimConsts::c_InvFltrIntMul);
  flt64 AvgT  = ((flt64)SumT  * xStructSimConsts::c_InvFltrIntMul);
  flt64 VarR2 = ((flt64)SumR2 * xStructSimConsts::c_InvFltrIntMul) - xPow2(AvgR);
  flt64 VarT2 = ((flt64)SumT2 * xStructSimConsts::c_InvFltrIntMul) - xPow2(AvgT);
  flt64 CovRT = ((flt64)SumRT * xStructSimConsts::c_InvFltrIntMul) - AvgR*AvgT;

  if (CalcL)
  {
    flt64 L    = (2 * AvgR * AvgT + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1); //"Luminance"
    flt64 CS   = (2 * CovRT       + C2) / (VarR2       + VarT2       + C2); //"Contrast"*"Similarity"
    flt64 SSIM = L * CS;
    return SSIM;
  }
  else
  {
    flt64 CS = (2 * CovRT + C2) / (VarR2 + VarT2 + C2); //"Contrast"*"Similarity"
    return CS;
  }
}
flt64 xStructSimSTD::CalcRglrAvgM(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 StrideT, int32 StrideR, int32 StrideM, int32 /*WndSize*/, flt64 C1, flt64 C2, bool CalcL)
{
  constexpr int32 FilterRange = xStructSimConsts::c_FilterRange;

  int64 SumR = 0, SumT = 0, SumR2 = 0, SumT2 = 0, SumRT = 0;
  int32 Num = 0;

  for(int32 dy = -FilterRange; dy <= FilterRange; dy++)
  {
    for(int32 dx = -FilterRange; dx <= FilterRange; dx++)
    {
      if(Msk[dy * StrideM + dx] == 0) { continue; }
      int32 R = Ref[dy * StrideR + dx];
      int32 T = Tst[dy * StrideT + dx];
      SumR  += R;
      SumT  += T;
      SumR2 += xPow2(R);
      SumT2 += xPow2(T);
      SumRT += R*T;
      Num++;
    }
  }

  flt64 InvN  = 1.0 / (flt64)Num;
  flt64 AvgR  = (flt64)SumR  * InvN;
  flt64 AvgT  = (flt64)SumT  * InvN;
  flt64 VarR2 = (flt64)SumR2 * InvN - xPow2(AvgR);
  flt64 VarT2 = (flt64)SumT2 * InvN - xPow2(AvgT);
  flt64 CovRT = (flt64)SumRT * InvN - AvgR*AvgT;
  
  if (CalcL)
  {
    flt64 L    = (2 * AvgR * AvgT + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1); //"Luminance"
    flt64 CS   = (2 * CovRT       + C2) / (VarR2       + VarT2       + C2); //"Contrast"*"Similarity"
    flt64 SSIM = L * CS;
    return SSIM;
  }
  else
  {
    flt64 CS = (2 * CovRT + C2) / (VarR2 + VarT2 + C2); //"Contrast"*"Similarity"
    return CS;
  }
}
#else //X_PMBB_EXPERIMENTAL
flt64 xStructSimSTD::CalcRglrFltM(const uint16*, const uint16*, const uint16*, int32, int32, int32, int32, flt64, flt64, bool)
{
  assert(0);
  return std::numeric_limits<flt64>::signaling_NaN();
}
flt64 xStructSimSTD::CalcRglrIntM(const uint16*, const uint16*, const uint16*, int32, int32, int32, int32, flt64, flt64, bool)
{
  assert(0);
  return std::numeric_limits<flt64>::signaling_NaN();
}
flt64 xStructSimSTD::CalcRglrAvgM(const uint16*, const uint16*, const uint16*, int32, int32, int32, int32, flt64, flt64, bool)
{
  assert(0);
  return std::numeric_limits<flt64>::signaling_NaN();
}
#endif //X_PMBB_EXPERIMENTAL

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Block based (8x8, 16x16) structural similarity
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
flt64 xStructSimSTD::CalcBlckInt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL)
{
  int64 SumR = 0, SumT = 0, SumR2 = 0, SumT2 = 0, SumRT = 0;

  if(WndSize == 8)
  {
    for(int32 y = 0; y < WndSize; y++)
    {
      for(int32 x = 0; x < WndSize; x++)
      {
        int32 R = Ref[x];
        int32 T = Tst[x];
        int64 C = xStructSimConsts::c_FilterBlckGaussInt8[y][x];
        SumR  += R     * C;
        SumT  += T     * C;
        SumR2 += R * R * C;
        SumT2 += T * T * C;
        SumRT += R * T * C;
      }
      Ref += StrideR;
      Tst += StrideT;
    }
  }
  else if(WndSize == 16)
  {
    for(int32 y = 0; y < WndSize; y++)
    {
      for(int32 x = 0; x < WndSize; x++)
      {
        int32 R = Ref[x];
        int32 T = Tst[x];
        int64 C = xStructSimConsts::c_FilterBlckGaussInt16[y][x];
        SumR  += R     * C;
        SumT  += T     * C;
        SumR2 += R * R * C;
        SumT2 += T * T * C;
        SumRT += R * T * C;
      }
      Ref += StrideR;
      Tst += StrideT;
    }
  }
  else
  {
    assert(0);
    return std::numeric_limits<flt64>::quiet_NaN();
  }

  flt64 AvgR  = ((flt64)SumR  * xStructSimConsts::c_InvFltrIntMul);
  flt64 AvgT  = ((flt64)SumT  * xStructSimConsts::c_InvFltrIntMul);
  flt64 VarR2 = ((flt64)SumR2 * xStructSimConsts::c_InvFltrIntMul) - xPow2(AvgR);
  flt64 VarT2 = ((flt64)SumT2 * xStructSimConsts::c_InvFltrIntMul) - xPow2(AvgT);
  flt64 CovRT = ((flt64)SumRT * xStructSimConsts::c_InvFltrIntMul) - AvgR*AvgT;

  if (CalcL)
  {
    flt64 L    = (2 * AvgR * AvgT + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1); //"Luminance"
    flt64 CS   = (2 * CovRT       + C2) / (VarR2       + VarT2       + C2); //"Contrast"*"Similarity"
    flt64 SSIM = L * CS;
    return SSIM;
  }
  else
  {
    flt64 CS = (2 * CovRT + C2) / (VarR2 + VarT2 + C2); //"Contrast"*"Similarity"
    return CS;
  }
}
flt64 xStructSimSTD::CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL)
{
  const int32 c_BlockArea    = WndSize * WndSize;
  const flt64 c_InvBlockArea = (flt64)1.0 / (flt64)c_BlockArea;

  int64 SumR = 0, SumT = 0, SumR2 = 0, SumT2 = 0, SumRT = 0;

  for(int32 y = 0; y < WndSize; y++)
  {
    for(int32 x = 0; x < WndSize; x++)
    {
      int32 R = Ref[x];
      int32 T = Tst[x];
      SumR  += R  ;
      SumT  += T  ;
      SumR2 += R*R;
      SumT2 += T*T;
      SumRT += R*T;
    }
    Ref += StrideR;
    Tst += StrideT;
  }

  flt64 AvgR  = (flt64)SumR  * c_InvBlockArea;
  flt64 AvgT  = (flt64)SumT  * c_InvBlockArea;
  flt64 VarR2 = (flt64)SumR2 * c_InvBlockArea - xPow2(AvgR);
  flt64 VarT2 = (flt64)SumT2 * c_InvBlockArea - xPow2(AvgT);
  flt64 CovRT = (flt64)SumRT * c_InvBlockArea - AvgR*AvgT;

  if (CalcL)
  {
    flt64 L    = (2 * AvgR * AvgT + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1); //"Luminance"
    flt64 CS   = (2 * CovRT       + C2) / (VarR2       + VarT2       + C2); //"Contrast"*"Similarity"
    flt64 SSIM = L * CS;
    return SSIM;
  }
  else
  {
    flt64 CS = (2 * CovRT + C2) / (VarR2 + VarT2 + C2); //"Contrast"*"Similarity"
    return CS;
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
