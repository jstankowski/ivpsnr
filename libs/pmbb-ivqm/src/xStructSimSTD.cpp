/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xStructSimSTD.h"
#include "xStructSimConsts.h"
#include "xKBNS.h"

#define SSIM_PRINT_INTERNAL 0

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Regular (11x11) structural similarity
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
flt64 xStructSimSTD::CalcRglrFlt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 /*WndSize*/, flt64 C1, flt64 C2, bool CalcL)
{
  //WARNING: This function is provided for reference only. The floating point accumulator makes it numericaly unstable.
  using tFlt = flt64;

  constexpr int32 FilterRange = xStructSimConsts::c_FilterRange;

  tFlt SumR = 0, SumT = 0, SumR2 = 0, SumT2 = 0, SumRT = 0;
  
  for(int32 dy = -FilterRange; dy <= FilterRange; dy++)
  {
    for(int32 dx = -FilterRange; dx <= FilterRange; dx++)
    {
      tFlt R = Ref[dy * StrideR + dx];
      tFlt T = Tst[dy * StrideT + dx];
      tFlt C = xStructSimConsts::c_FilterRglrGaussFlt32[dy + FilterRange][dx + FilterRange];
      SumR  += R        * C;
      SumT  += T        * C;
      SumR2 += xPow2(R) * C;
      SumT2 += xPow2(T) * C;
      SumRT += R*T      * C;
    }
  }

  tFlt AvgR  = SumR ;
  tFlt AvgT  = SumT ;
  tFlt VarR2 = SumR2 - xPow2(AvgR);
  tFlt VarT2 = SumT2 - xPow2(AvgT);
  tFlt CovRT = SumRT - AvgR*AvgT;

  //if(VarR2 < 0 || VarT2 < 0 || CovRT < 0) { return std::numeric_limits<flt64>::quiet_NaN(); }

#if SSIM_PRINT_INTERNAL
  fmt::print("CalcRglrFlt ");
  fmt::print("AvgR = {} ", AvgR );
  fmt::print("AvgT = {} ", AvgT );
  fmt::print("VarR2= {} ", VarR2);
  fmt::print("VarT2= {} ", VarT2);
  fmt::print("CovRT= {} ", CovRT);
  tFlt C3         = C2 / 2.0;
  tFlt DevR2      = sqrt(VarR2);
  tFlt DevT2      = sqrt(VarT2);
  tFlt Luminance  = (2 * AvgR  * AvgT  + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1);
  tFlt Contrast   = (2 * DevR2 * DevT2 + C2) / (VarR2       + VarT2       + C2);
  tFlt Similarity = (CovRT             + C3) / (DevR2       * DevT2       + C3);
  tFlt SSIMo      = Luminance * Contrast * Similarity;
  fmt::print("Luminance  = {} ", Luminance );
  fmt::print("Contrast   = {} ", Contrast  );
  fmt::print("Similarity = {} ", Similarity);
  fmt::print("SSIMo      = {} ", SSIMo     );
#endif //SSIM_PRINT_INTERNAL

  if (CalcL)
  {
    tFlt L    = (2 * AvgR * AvgT + (tFlt)C1) / (xPow2(AvgR) + xPow2(AvgT) + (tFlt)C1); //"Luminance"
    tFlt CS   = (2 * CovRT       + (tFlt)C2) / (VarR2       + VarT2       + (tFlt)C2); //"Contrast"*"Similarity"
    tFlt SSIM = L * CS;
#if SSIM_PRINT_INTERNAL
    fmt::print("L={} CS={} SSIM= {}\n", L, CS, SSIM);
#endif //SSIM_PRINT_INTERNAL
    return SSIM;
  }
  else
  {
    tFlt CS = (2 * CovRT + (tFlt)C2) / (VarR2 + VarT2 + (tFlt)C2); //"Contrast"*"Similarity"
#if SSIM_PRINT_INTERNAL
    fmt::print("CS={}\n", CS);
#endif //SSIM_PRINT_INTERNAL
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

#if SSIM_PRINT_INTERNAL
  fmt::print("CalcRglrInt ");
  fmt::print("AvgR = {} ", AvgR);
  fmt::print("AvgT = {} ", AvgT);
  fmt::print("VarR2= {} ", VarR2);
  fmt::print("VarT2= {} ", VarT2);
  fmt::print("CovRT= {} ", CovRT);
  flt64 C3         = C2 / 2.0;
  flt64 DevR2      = sqrt(VarR2);
  flt64 DevT2      = sqrt(VarT2);
  flt64 Luminance  = (2 * AvgR  * AvgT  + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1);
  flt64 Contrast   = (2 * DevR2 * DevT2 + C2) / (VarR2       + VarT2       + C2);
  flt64 Similarity = (CovRT             + C3) / (DevR2       * DevT2       + C3);
  flt64 SSIMo      = Luminance * Contrast * Similarity;
  fmt::print("Luminance  = {} ", Luminance);
  fmt::print("Contrast   = {} ", Contrast);
  fmt::print("Similarity = {} ", Similarity);
  fmt::print("SSIMo      = {} ", SSIMo);
#endif //SSIM_PRINT_INTERNAL

  if (CalcL)
  {
    flt64 L    = (2 * AvgR * AvgT + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1); //"Luminance"
    flt64 CS   = (2 * CovRT       + C2) / (VarR2       + VarT2       + C2); //"Contrast"*"Similarity"
    flt64 SSIM = L * CS;
#if SSIM_PRINT_INTERNAL
    fmt::print("L={} CS={} SSIM= {}\n", L, CS, SSIM);
#endif //SSIM_PRINT_INTERNAL
    return SSIM;
  }
  else
  {
    flt64 CS = (2 * CovRT + C2) / (VarR2 + VarT2 + C2); //"Contrast"*"Similarity"
#if SSIM_PRINT_INTERNAL
    fmt::print("CS={}\n", CS);
#endif //SSIM_PRINT_INTERNAL
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

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Regular (11x11) structural similarity - with mask
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
tSSIMM xStructSimSTD::CalcRglrIntM(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 StrideT, int32 StrideR, int32 StrideM, int32 /*WndSize*/, flt64 C1, flt64 C2, bool CalcL)
{  
  //WARNING: This function is provided for reference only. It fails for BitDepth > 8 due to catastrophic cancelation while calculating VarR2 or VarT2 or CovRT.
  constexpr int32 FilterRange = xStructSimConsts::c_FilterRange;

  if(Msk[0] == 0) { return { std::numeric_limits<flt64>::quiet_NaN(), 0.0 }; } //do not return SSIM for central pixel if it is masked-out

  int64 SumR = 0, SumT = 0, SumR2 = 0, SumT2 = 0, SumRT = 0, SumCM = 0, SumM = 0, Num = 0;

  for(int32 dy = -FilterRange; dy <= FilterRange; dy++)
  {
    for(int32 dx = -FilterRange; dx <= FilterRange; dx++)
    {
      const int32 M  = Msk[dy * StrideM + dx]; //assume - Msk values are in range 0-255
      if(M == 0) { continue; }
      const int32 C  = xStructSimConsts::c_FilterRglrGaussInt[dy + FilterRange][dx + FilterRange];
      const int32 R  = Ref[dy * StrideR + dx];
      const int32 T  = Tst[dy * StrideT + dx];
      const int32 CM = C * M;
      SumR  += R               * CM;
      SumT  += T               * CM;
      SumR2 += xPow2<int64>(R) * CM;
      SumT2 += xPow2<int64>(T) * CM;
      SumRT += (int64)(R*T)    * CM;
      SumCM +=                   CM;
      SumM  += M;
      Num++;
    }
  }

  if(Num <= 2) { return { std::numeric_limits<flt64>::quiet_NaN(), 0.0 }; } //do not return SSIM for less than 3 pixels

  flt64 Weight = SumM * xStructSimConsts::c_InvRegMaskMul;

  flt64 InvCM = (flt64)1.0 / ((flt64)SumCM); 
  flt64 AvgR  = ((flt64)SumR  * InvCM);
  flt64 AvgT  = ((flt64)SumT  * InvCM);
  flt64 VarR2 = ((flt64)SumR2 * InvCM) - xPow2(AvgR);
  flt64 VarT2 = ((flt64)SumT2 * InvCM) - xPow2(AvgT);
  flt64 CovRT = ((flt64)SumRT * InvCM) - AvgR*AvgT;

  if (CalcL)
  {
    flt64 L    = (2 * AvgR * AvgT + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1); //"Luminance"
    flt64 CS   = (2 * CovRT       + C2) / (VarR2       + VarT2       + C2); //"Contrast"*"Similarity"
    flt64 SSIM = L * CS;
    return { SSIM, Weight };
  }
  else
  {
    flt64 CS = (2 * CovRT + C2) / (VarR2 + VarT2 + C2); //"Contrast"*"Similarity"
    return { CS, Weight };
  }
}
tSSIMM xStructSimSTD::CalcRglrAvgM(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 StrideT, int32 StrideR, int32 StrideM, int32 /*WndSize*/, flt64 C1, flt64 C2, bool CalcL)
{
  constexpr int32 FilterRange = xStructSimConsts::c_FilterRange;

  if(Msk[0] == 0) { return { std::numeric_limits<flt64>::quiet_NaN(), 0.0 }; } //do not return SSIM for central pixel if it is masked-out

  int64 SumR = 0, SumT = 0, SumR2 = 0, SumT2 = 0, SumRT = 0, SumM = 0, Num = 0;

  for(int32 dy = -FilterRange; dy <= FilterRange; dy++)
  {
    for(int32 dx = -FilterRange; dx <= FilterRange; dx++)
    {
      const int32 M = Msk[dy * StrideM + dx]; //assume - Msk values are in range 0-255
      if(M == 0) { continue; }
      const int32 R = Ref[dy * StrideR + dx];
      const int32 T = Tst[dy * StrideT + dx];
      SumR  += R               * M;
      SumT  += T               * M;
      SumR2 += xPow2<int64>(R) * M;
      SumT2 += xPow2<int64>(T) * M;
      SumRT += (int64)(R*T)    * M;
      SumM  += M;
      Num++;
    }
  }

  if(Num <= 2) { return { std::numeric_limits<flt64>::quiet_NaN(), 0.0 }; } //do not return SSIM for less than 3 pixels

  flt64 Weight = SumM * xStructSimConsts::c_InvRegMaskMul;

  flt64 InvM  = (flt64)1.0 / ((flt64)SumM);
  flt64 AvgR  = (flt64)SumR  * InvM;
  flt64 AvgT  = (flt64)SumT  * InvM;
  flt64 VarR2 = (flt64)SumR2 * InvM - xPow2(AvgR);
  flt64 VarT2 = (flt64)SumT2 * InvM - xPow2(AvgT);
  flt64 CovRT = (flt64)SumRT * InvM - AvgR*AvgT;
  
  if (CalcL)
  {
    flt64 L    = (2 * AvgR * AvgT + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1); //"Luminance"
    flt64 CS   = (2 * CovRT       + C2) / (VarR2       + VarT2       + C2); //"Contrast"*"Similarity"
    flt64 SSIM = L * CS;
    return { SSIM, Weight };
  }
  else
  {
    flt64 CS = (2 * CovRT + C2) / (VarR2 + VarT2 + C2); //"Contrast"*"Similarity"
    return { CS, Weight };
  }
}
tSSIMM xStructSimSTD::CalcBlckAvgM(const uint16* Tst, const uint16* Ref, const uint16* Msk, int32 StrideT, int32 StrideR, int32 StrideM, int32 WndSize, flt64 C1, flt64 C2, bool CalcL)
{

  int64 SumR = 0, SumT = 0, SumR2 = 0, SumT2 = 0, SumRT = 0, SumM = 0, Num = 0;

  for(int32 y = 0; y < WndSize; y++)
  {
    for(int32 x = 0; x < WndSize; x++)
    {
      const int32 M = Msk[x]; //assume - Msk values are in range 0-255
      if(M == 0) { continue; }
      const int32 R = Ref[x];
      const int32 T = Tst[x];
      SumR  += R           *M;
      SumT  += T           *M;
      SumR2 += (int64)(R*R)*M;
      SumT2 += (int64)(T*T)*M;
      SumRT += (int64)(R*T)*M;
      SumM  += M;
      Num++;

    }
    Ref += StrideR;
    Tst += StrideT;
    Msk += StrideM;
  }

  if(Num <= 2) { return { std::numeric_limits<flt64>::quiet_NaN(), 0.0 }; } //do not return SSIM for less than 3 pixels

  const int32 BlockArea        = WndSize * WndSize;
  const int32 BlkAvgMaskMul    = BlockArea * xStructSimConsts::c_MaskMaxVal;
  const flt64 InvBlkAvgMaskMul = (flt64)1.0 / (flt64)BlkAvgMaskMul;
  flt64 Weight = SumM * InvBlkAvgMaskMul;

  flt64 InvM  = (flt64)1.0 / ((flt64)SumM);
  flt64 AvgR  = (flt64)SumR  * InvM;
  flt64 AvgT  = (flt64)SumT  * InvM;
  flt64 VarR2 = (flt64)SumR2 * InvM - xPow2(AvgR);
  flt64 VarT2 = (flt64)SumT2 * InvM - xPow2(AvgT);
  flt64 CovRT = (flt64)SumRT * InvM - AvgR*AvgT;

  if (CalcL)
  {
    flt64 L    = (2 * AvgR * AvgT + C1) / (xPow2(AvgR) + xPow2(AvgT) + C1); //"Luminance"
    flt64 CS   = (2 * CovRT       + C2) / (VarR2       + VarT2       + C2); //"Contrast"*"Similarity"
    flt64 SSIM = L * CS;
    return { SSIM, Weight };
  }
  else
  {
    flt64 CS = (2 * CovRT + C2) / (VarR2 + VarT2 + C2); //"Contrast"*"Similarity"
    return { CS, Weight };
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
