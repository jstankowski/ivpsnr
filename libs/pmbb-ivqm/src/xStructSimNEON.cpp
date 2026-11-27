/*
    SPDX-FileCopyrightText: 2025 Patrycja Kaźmierczak <patrycja.kazmierczak@student.put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xStructSimNEON.h"
#include "xStructSim.h"
#include "xHelpersSIMD.h"

#if X_SIMD_CAN_USE_NEON

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

flt64 xStructSimNEON::CalcBlckAvg(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 BlockSize, flt64 C1, flt64 C2, bool CalcL)
{
  if(BlockSize != 8 && BlockSize != 16 && BlockSize != 32) { return xStructSimSTD::CalcBlckAvg(Tst, Ref, StrideT, StrideR, BlockSize, C1, C2, CalcL); }
  const int32 c_BlockArea    = BlockSize * BlockSize;
  const flt64 c_InvBlockArea = (flt64)1.0 / (flt64)c_BlockArea;

  int32x4_t SumR_V  = vdupq_n_s32(0);
  int32x4_t SumT_V  = vdupq_n_s32(0);
  uint64x2_t SumR2_V = vdupq_n_u64(0);
  uint64x2_t SumT2_V = vdupq_n_u64(0);
  uint64x2_t SumRT_V = vdupq_n_u64(0);

  for(int32 y = 0; y < BlockSize; y++)
  {
    for(int32 x = 0; x < BlockSize; x+=8)
    {
      uint16x8_t Tst_V = vld1q_u16(Tst+x);
      uint16x8_t Ref_V = vld1q_u16(Ref+x);
      //SumT  += T;
      SumT_V = vaddq_s32(SumT_V, vpaddlq_s16(vreinterpretq_s16_u16(Tst_V)));
      //SumR  += R;
      SumR_V = vaddq_s32(SumR_V, vpaddlq_s16(vreinterpretq_s16_u16(Ref_V)));
      //SumT2 += xPow2(T);
      SumT2_V = vmlal_u32(SumT2_V, vget_low_u32 (vmovl_u16(vget_low_u16 (Tst_V))), vget_low_u32 (vmovl_u16(vget_low_u16 (Tst_V))));
      SumT2_V = vmlal_u32(SumT2_V, vget_high_u32(vmovl_u16(vget_low_u16 (Tst_V))), vget_high_u32(vmovl_u16(vget_low_u16 (Tst_V))));
      SumT2_V = vmlal_u32(SumT2_V, vget_low_u32 (vmovl_u16(vget_high_u16(Tst_V))), vget_low_u32 (vmovl_u16(vget_high_u16(Tst_V))));
      SumT2_V = vmlal_u32(SumT2_V, vget_high_u32(vmovl_u16(vget_high_u16(Tst_V))), vget_high_u32(vmovl_u16(vget_high_u16(Tst_V))));
      //SumR2 += xPow2(R);
      SumR2_V = vmlal_u32(SumR2_V, vget_low_u32(vmovl_u16 (vget_low_u16 (Ref_V))), vget_low_u32 (vmovl_u16(vget_low_u16 (Ref_V))));
      SumR2_V = vmlal_u32(SumR2_V, vget_high_u32(vmovl_u16(vget_low_u16 (Ref_V))), vget_high_u32(vmovl_u16(vget_low_u16 (Ref_V))));
      SumR2_V = vmlal_u32(SumR2_V, vget_low_u32(vmovl_u16 (vget_high_u16(Ref_V))), vget_low_u32 (vmovl_u16(vget_high_u16(Ref_V))));
      SumR2_V = vmlal_u32(SumR2_V, vget_high_u32(vmovl_u16(vget_high_u16(Ref_V))), vget_high_u32(vmovl_u16(vget_high_u16(Ref_V))));

      //SumRT += R*T;
      SumRT_V = vmlal_u32(SumRT_V, vget_low_u32(vmovl_u16(vget_low_u16  (Tst_V))), vget_low_u32 (vmovl_u16(vget_low_u16 (Ref_V))));
      SumRT_V = vmlal_u32(SumRT_V, vget_high_u32(vmovl_u16(vget_low_u16 (Tst_V))), vget_high_u32(vmovl_u16(vget_low_u16 (Ref_V))));
      SumRT_V = vmlal_u32(SumRT_V, vget_low_u32(vmovl_u16(vget_high_u16 (Tst_V))), vget_low_u32 (vmovl_u16(vget_high_u16(Ref_V))));
      SumRT_V = vmlal_u32(SumRT_V, vget_high_u32(vmovl_u16(vget_high_u16(Tst_V))), vget_high_u32(vmovl_u16(vget_high_u16(Ref_V))));
    }    
    Ref += StrideR;
    Tst += StrideT;
  }

  //int32x4_t TmpSumRandT = vaddvq_s32(vaddvq_s32(SumR_V, SumT_V), vdupq_n_s32(0)());
  int32 SumR = vaddvq_s32(SumR_V);
  int32 SumT = vaddvq_s32(SumT_V);

  int64 SumR2 = vaddvq_s64(vreinterpretq_s64_u64(SumR2_V)); 
  int64 SumT2 = vaddvq_s64(vreinterpretq_s64_u64(SumT2_V));
  int64 SumRT = vaddvq_s64(vreinterpretq_s64_u64(SumRT_V));

  flt64 AvgR  = (flt64)SumR  * c_InvBlockArea;
  flt64 AvgT  = (flt64)SumT  * c_InvBlockArea;
  flt64 VarR2 = (flt64)SumR2 * c_InvBlockArea - xPow2(AvgR);
  flt64 VarT2 = (flt64)SumT2 * c_InvBlockArea - xPow2(AvgT);
  flt64 CovRT = (flt64)SumRT * c_InvBlockArea - AvgR*AvgT;

  if(CalcL)
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

#endif //X_SIMD_CAN_USE_NEON