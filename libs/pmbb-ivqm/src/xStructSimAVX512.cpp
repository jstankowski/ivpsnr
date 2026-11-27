/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xStructSimAVX512.h"
#include "xHelpersSIMD.h"
#include "xStructSimConsts.h"

#if X_SIMD_CAN_USE_AVX512

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
flt64 xStructSimAVX512::CalcRglrFlt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 /*WndSize*/, flt64 C1, flt64 C2, bool CalcL)
{
  //WARNING: This function is provided for reference only. The floating point accumulator makes it numericaly unstable.
  constexpr int32 c_BlockSize = xStructSimConsts::c_FilterSize;

  Tst -= (5 * StrideT + 5);
  Ref -= (5 * StrideR + 5);

  const flt32* Cff = &xStructSimConsts::c_FilterRglrGaussFlt32[0][0];

  __m512d SumR_F64_V  = _mm512_setzero_pd();
  __m512d SumT_F64_V  = _mm512_setzero_pd();
  __m512d SumRR_F64_V = _mm512_setzero_pd();
  __m512d SumTT_F64_V = _mm512_setzero_pd();
  __m512d SumRT_F64_V = _mm512_setzero_pd();

  constexpr uint16 LoadMask = (1 << c_BlockSize) - 1;
  for(int32 y = 0; y < c_BlockSize; y++)
  {    
    __m256i Tst_U16_V = _mm256_mask_loadu_epi16(_mm256_setzero_si256(), LoadMask, Tst);
    __m256i Ref_U16_V = _mm256_mask_loadu_epi16(_mm256_setzero_si256(), LoadMask, Ref);
    __m512f Cff_F32_V = _mm512_mask_loadu_ps   (_mm512_setzero_ps   (), LoadMask, Cff);

    __m512i Tst_I32_V = _mm512_cvtepu16_epi32(Tst_U16_V);
    __m512i Ref_I32_V = _mm512_cvtepu16_epi32(Ref_U16_V);

    __m512d CffA_F64_V = _mm512_cvtps_pd   (_mm512_castps512_ps256   (Cff_F32_V  ));
    __m512d CffB_F64_V = _mm512_cvtps_pd   (_mm512_extractf32x8_ps   (Cff_F32_V,1));
    __m512d TstA_F64_V = _mm512_cvtepi32_pd(_mm512_castsi512_si256   (Tst_I32_V  ));
    __m512d TstB_F64_V = _mm512_cvtepi32_pd(_mm512_extracti64x4_epi64(Tst_I32_V,1));
    __m512d RefA_F64_V = _mm512_cvtepi32_pd(_mm512_castsi512_si256   (Ref_I32_V  ));
    __m512d RefB_F64_V = _mm512_cvtepi32_pd(_mm512_extracti64x4_epi64(Ref_I32_V,1));

    SumT_F64_V = _mm512_add_pd(SumT_F64_V, _mm512_add_pd(_mm512_mul_pd(TstA_F64_V, CffA_F64_V), _mm512_mul_pd(TstB_F64_V, CffB_F64_V))); //SumT  += T*C;
    SumR_F64_V = _mm512_add_pd(SumR_F64_V, _mm512_add_pd(_mm512_mul_pd(RefA_F64_V, CffA_F64_V), _mm512_mul_pd(RefB_F64_V, CffB_F64_V))); //SumR  += R*C;

    __m512d T2CA_F32_V = _mm512_mul_pd(_mm512_mul_pd(TstA_F64_V, TstA_F64_V), CffA_F64_V);
    __m512d T2CB_F32_V = _mm512_mul_pd(_mm512_mul_pd(TstB_F64_V, TstB_F64_V), CffB_F64_V);
    __m512d R2CA_F32_V = _mm512_mul_pd(_mm512_mul_pd(RefA_F64_V, RefA_F64_V), CffA_F64_V);
    __m512d R2CB_F32_V = _mm512_mul_pd(_mm512_mul_pd(RefB_F64_V, RefB_F64_V), CffB_F64_V);
    __m512d TRCA_F32_V = _mm512_mul_pd(_mm512_mul_pd(TstA_F64_V, RefA_F64_V), CffA_F64_V);
    __m512d TRCB_F32_V = _mm512_mul_pd(_mm512_mul_pd(TstB_F64_V, RefB_F64_V), CffB_F64_V);

    SumTT_F64_V = _mm512_add_pd(SumTT_F64_V, _mm512_add_pd(T2CA_F32_V, T2CB_F32_V)); //SumT2 += T*T*C;
    SumRR_F64_V = _mm512_add_pd(SumRR_F64_V, _mm512_add_pd(R2CA_F32_V, R2CB_F32_V)); //SumR2 += R*R*C;
    SumRT_F64_V = _mm512_add_pd(SumRT_F64_V, _mm512_add_pd(TRCA_F32_V, TRCB_F32_V)); //SumRT += R*T*C;
    Ref += StrideR;
    Tst += StrideT;
    Cff += c_BlockSize;
  }  

  flt64 SumR  = xHorVecSum_pd(SumR_F64_V );
  flt64 SumT  = xHorVecSum_pd(SumT_F64_V );
  flt64 SumRR = xHorVecSum_pd(SumRR_F64_V);
  flt64 SumTT = xHorVecSum_pd(SumTT_F64_V);
  flt64 SumRT = xHorVecSum_pd(SumRT_F64_V);

  flt64 AvgR  = SumR ;
  flt64 AvgT  = SumT ;
  flt64 VarR2 = SumRR - xPow2(AvgR);
  flt64 VarT2 = SumTT - xPow2(AvgT);
  flt64 CovRT = SumRT - AvgR*AvgT;

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
flt64 xStructSimAVX512::CalcRglrInt(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 /*WndSize*/, flt64 C1, flt64 C2, bool CalcL)
{
  constexpr int32 c_BlockSize = xStructSimConsts::c_FilterSize;

  Tst -= (5 * StrideT + 5);
  Ref -= (5 * StrideR + 5);

  const int16* Cff = &xStructSimConsts::c_FilterRglrGaussInt[0][0];

  __m512i SumR_I32_V  = _mm512_setzero_si512();
  __m512i SumT_I32_V  = _mm512_setzero_si512();
  __m512i SumRR_I64_V = _mm512_setzero_si512();
  __m512i SumTT_I64_V = _mm512_setzero_si512();
  __m512i SumRT_I64_V = _mm512_setzero_si512();

  constexpr __mmask16 LoadMask = (1 << c_BlockSize) - 1;
  for(int32 y = 0; y < c_BlockSize; y++)
  {    
    __m256i Tst_U16_V = _mm256_mask_loadu_epi16(_mm256_setzero_si256(), LoadMask, (__m256i*)(Tst));
    __m256i Ref_U16_V = _mm256_mask_loadu_epi16(_mm256_setzero_si256(), LoadMask, (__m256i*)(Ref));
    __m256i Cff_U16_V = _mm256_mask_loadu_epi16(_mm256_setzero_si256(), LoadMask, (__m256i*)(Cff));

    SumT_I32_V  = _mm512_add_epi32(SumT_I32_V , _mm512_cvtepi32_epi64(_mm256_madd_epi16(Tst_U16_V, Cff_U16_V))); //SumT  += T;
    SumR_I32_V  = _mm512_add_epi32(SumR_I32_V , _mm512_cvtepi32_epi64(_mm256_madd_epi16(Ref_U16_V, Cff_U16_V))); //SumR  += R;

    __m512i Tst_I32_V = _mm512_cvtepu16_epi32(Tst_U16_V);
    __m512i Ref_I32_V = _mm512_cvtepu16_epi32(Ref_U16_V);
    __m512i Cff_I32_V = _mm512_cvtepu16_epi32(Cff_U16_V);

    __m512i T2_I32_V = _mm512_mullo_epi32(Tst_I32_V, Tst_I32_V);
    __m512i R2_I32_V = _mm512_mullo_epi32(Ref_I32_V, Ref_I32_V);
    __m512i RT_I32_V = _mm512_mullo_epi32(Tst_I32_V, Ref_I32_V);

    __m512i CfA_I64_V = _mm512_cvtepi32_epi64(_mm512_castsi512_si256   (Cff_I32_V  ));
    __m512i CfB_I64_V = _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(Cff_I32_V,1));
    __m512i T2A_I64_V = _mm512_cvtepi32_epi64(_mm512_castsi512_si256   (T2_I32_V  ));
    __m512i T2B_I64_V = _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(T2_I32_V,1));
    __m512i R2A_I64_V = _mm512_cvtepi32_epi64(_mm512_castsi512_si256   (R2_I32_V  ));
    __m512i R2B_I64_V = _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(R2_I32_V,1));
    __m512i RTA_I64_V = _mm512_cvtepi32_epi64(_mm512_castsi512_si256   (RT_I32_V  ));
    __m512i RTB_I64_V = _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(RT_I32_V,1));

    __m512i T2C_I64_V = _mm512_add_epi64(_mm512_mullo_epi64(T2A_I64_V, CfA_I64_V), _mm512_mullo_epi64(T2B_I64_V, CfB_I64_V));
    __m512i R2C_I64_V = _mm512_add_epi64(_mm512_mullo_epi64(R2A_I64_V, CfA_I64_V), _mm512_mullo_epi64(R2B_I64_V, CfB_I64_V));
    __m512i RTC_I64_V = _mm512_add_epi64(_mm512_mullo_epi64(RTA_I64_V, CfA_I64_V), _mm512_mullo_epi64(RTB_I64_V, CfB_I64_V));

    SumTT_I64_V = _mm512_add_epi64(SumTT_I64_V, T2C_I64_V); //SumT2 += T*T;
    SumRR_I64_V = _mm512_add_epi64(SumRR_I64_V, R2C_I64_V); //SumR2 += R*R;
    SumRT_I64_V = _mm512_add_epi64(SumRT_I64_V, RTC_I64_V); //SumRT += R*T;
    Ref += StrideR;
    Tst += StrideT;
    Cff += c_BlockSize;
  }  

  int64 SumR  = xHorVecSumI64_epi64(SumR_I32_V );
  int64 SumT  = xHorVecSumI64_epi64(SumT_I32_V );
  int64 SumRR = xHorVecSumI64_epi64(SumRR_I64_V);
  int64 SumTT = xHorVecSumI64_epi64(SumTT_I64_V);
  int64 SumRT = xHorVecSumI64_epi64(SumRT_I64_V);

  flt64 AvgR  = (flt64)SumR  * xStructSimConsts::c_InvFltrIntMul;
  flt64 AvgT  = (flt64)SumT  * xStructSimConsts::c_InvFltrIntMul;
  flt64 VarR2 = (flt64)SumRR * xStructSimConsts::c_InvFltrIntMul - xPow2(AvgR);
  flt64 VarT2 = (flt64)SumTT * xStructSimConsts::c_InvFltrIntMul - xPow2(AvgT);
  flt64 CovRT = (flt64)SumRT * xStructSimConsts::c_InvFltrIntMul - AvgR*AvgT;

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
flt64 xStructSimAVX512::CalcBlckAvg4(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL)
{
  constexpr int32 c_BlockSize    = 4;
  constexpr int32 c_BlockArea    = c_BlockSize * c_BlockSize;
  constexpr flt64 c_InvBlockArea = (flt64)1.0 / (flt64)c_BlockArea;

  __m128i SumR_I32_V  = _mm_setzero_si128();
  __m128i SumT_I32_V  = _mm_setzero_si128();
  __m128i SumR2_I64_V = _mm_setzero_si128();
  __m128i SumT2_I64_V = _mm_setzero_si128();
  __m128i SumRT_I64_V = _mm_setzero_si128();

  for(int32 y = 0; y < c_BlockSize; y++)
  {
    __m128i Tst_U16_V = _mm_loadl_epi64((__m128i*)(Tst));
    __m128i Ref_U16_V = _mm_loadl_epi64((__m128i*)(Ref));
    SumT_I32_V  = _mm_add_epi32(SumT_I32_V, _mm_unpacklo_epi16(Tst_U16_V, _mm_setzero_si128())); //SumT  += T;    
    SumR_I32_V  = _mm_add_epi32(SumR_I32_V, _mm_unpacklo_epi16(Ref_U16_V, _mm_setzero_si128())); //SumR  += R;    
    SumT2_I64_V = _mm_add_epi64(SumT2_I64_V, _mm_unpacklo_epi32(_mm_madd_epi16(Tst_U16_V, Tst_U16_V), _mm_setzero_si128())); //SumT2 += xPow2(T);    
    SumR2_I64_V = _mm_add_epi64(SumR2_I64_V, _mm_unpacklo_epi32(_mm_madd_epi16(Ref_U16_V, Ref_U16_V), _mm_setzero_si128())); //SumR2 += xPow2(R);    
    SumRT_I64_V = _mm_add_epi64(SumRT_I64_V, _mm_unpacklo_epi32(_mm_madd_epi16(Tst_U16_V, Ref_U16_V), _mm_setzero_si128())); //SumRT += R*T;
    Ref += StrideR;
    Tst += StrideT;
  }

  __m128i TmpSumRandT = _mm_hadd_epi32(_mm_hadd_epi32(SumR_I32_V, SumT_I32_V), _mm_setzero_si128());
  int32 SumR = _mm_extract_epi32(TmpSumRandT, 0);
  int32 SumT = _mm_extract_epi32(TmpSumRandT, 1);

  int64 SumR2 = xHorVecSumI64_epi64(SumR2_I64_V);
  int64 SumT2 = xHorVecSumI64_epi64(SumT2_I64_V);
  int64 SumRT = xHorVecSumI64_epi64(SumRT_I64_V);

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
flt64 xStructSimAVX512::CalcBlckAvg8(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL)
{
  constexpr int32 c_BlockSize    = 8;
  constexpr int32 c_BlockArea    = c_BlockSize * c_BlockSize;
  constexpr flt64 c_InvBlockArea = (flt64)1.0 / (flt64)c_BlockArea;

  __m256i SumR_I32_V  = _mm256_setzero_si256();
  __m256i SumT_I32_V  = _mm256_setzero_si256();
  __m256i SumRR_I64_V = _mm256_setzero_si256();
  __m256i SumTT_I64_V = _mm256_setzero_si256();
  __m256i SumRT_I64_V = _mm256_setzero_si256();

  for(int32 y = 0; y < c_BlockSize; y++)
  {
    __m128i Tst_U16_V = _mm_loadu_si128((__m128i*)Tst);
    __m128i Ref_U16_V = _mm_loadu_si128((__m128i*)Ref);
    SumT_I32_V  = _mm256_add_epi32(SumT_I32_V , _mm256_cvtepu16_epi32(Tst_U16_V)                           ); //SumT  += T;    
    SumR_I32_V  = _mm256_add_epi32(SumR_I32_V , _mm256_cvtepu16_epi32(Ref_U16_V)                           ); //SumR  += R;    
    SumTT_I64_V = _mm256_add_epi64(SumTT_I64_V, _mm256_cvtepi32_epi64(_mm_madd_epi16(Tst_U16_V, Tst_U16_V))); //SumT2 += T*T;    
    SumRR_I64_V = _mm256_add_epi64(SumRR_I64_V, _mm256_cvtepi32_epi64(_mm_madd_epi16(Ref_U16_V, Ref_U16_V))); //SumR2 += R*R;    
    SumRT_I64_V = _mm256_add_epi64(SumRT_I64_V, _mm256_cvtepi32_epi64(_mm_madd_epi16(Ref_U16_V, Tst_U16_V))); //SumRT += R*T;
    Ref += StrideR;
    Tst += StrideT;
  }  

  int32 SumR  = xHorVecSumI32_epi32(SumR_I32_V );
  int32 SumT  = xHorVecSumI32_epi32(SumT_I32_V );
  int64 SumRR = xHorVecSumI64_epi64(SumRR_I64_V);
  int64 SumTT = xHorVecSumI64_epi64(SumTT_I64_V);
  int64 SumRT = xHorVecSumI64_epi64(SumRT_I64_V);

  flt64 AvgR  = (flt64)SumR  * c_InvBlockArea;
  flt64 AvgT  = (flt64)SumT  * c_InvBlockArea;
  flt64 VarR2 = (flt64)SumRR * c_InvBlockArea - xPow2(AvgR);
  flt64 VarT2 = (flt64)SumTT * c_InvBlockArea - xPow2(AvgT);
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
flt64 xStructSimAVX512::CalcBlckAvg11(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL)
{
  constexpr int32 c_BlockSize    = xStructSimConsts::c_FilterSize;
  constexpr int32 c_BlockArea    = c_BlockSize * c_BlockSize;
  constexpr flt64 c_InvBlockArea = (flt64)1.0 / (flt64)c_BlockArea;

  __m512i SumR_I32_V  = _mm512_setzero_si512();
  __m512i SumT_I32_V  = _mm512_setzero_si512();
  __m512i SumRR_I64_V = _mm512_setzero_si512();
  __m512i SumTT_I64_V = _mm512_setzero_si512();
  __m512i SumRT_I64_V = _mm512_setzero_si512();

  constexpr __mmask16 LoadMask = (1 << c_BlockSize) - 1;
  for(int32 y = 0; y < c_BlockSize; y++)
  {    
    __m256i Tst_U16_V = _mm256_mask_loadu_epi16(_mm256_setzero_si256(), LoadMask, (__m256i*)(Tst));
    __m256i Ref_U16_V = _mm256_mask_loadu_epi16(_mm256_setzero_si256(), LoadMask, (__m256i*)(Ref));

    SumT_I32_V  = _mm512_add_epi32(SumT_I32_V , _mm512_cvtepu16_epi32(Tst_U16_V)); //SumT  += T;
    SumR_I32_V  = _mm512_add_epi32(SumR_I32_V , _mm512_cvtepu16_epi32(Ref_U16_V)); //SumR  += R;
    SumTT_I64_V = _mm512_add_epi64(SumTT_I64_V, _mm512_cvtepi32_epi64(_mm256_madd_epi16(Tst_U16_V, Tst_U16_V))); //SumT2 += T*T;
    SumRR_I64_V = _mm512_add_epi64(SumRR_I64_V, _mm512_cvtepi32_epi64(_mm256_madd_epi16(Ref_U16_V, Ref_U16_V))); //SumR2 += R*R;
    SumRT_I64_V = _mm512_add_epi64(SumRT_I64_V, _mm512_cvtepi32_epi64(_mm256_madd_epi16(Ref_U16_V, Tst_U16_V))); //SumRT += R*T;
    Ref += StrideR;
    Tst += StrideT;
  }  

  int32 SumR  = xHorVecSumI32_epi32(SumR_I32_V );
  int32 SumT  = xHorVecSumI32_epi32(SumT_I32_V );
  int64 SumRR = xHorVecSumI64_epi64(SumRR_I64_V);
  int64 SumTT = xHorVecSumI64_epi64(SumTT_I64_V);
  int64 SumRT = xHorVecSumI64_epi64(SumRT_I64_V);

  flt64 AvgR  = (flt64)SumR  * c_InvBlockArea;
  flt64 AvgT  = (flt64)SumT  * c_InvBlockArea;
  flt64 VarR2 = (flt64)SumRR * c_InvBlockArea - xPow2(AvgR);
  flt64 VarT2 = (flt64)SumTT * c_InvBlockArea - xPow2(AvgT);
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
flt64 xStructSimAVX512::CalcBlckAvg16(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL)
{
  constexpr int32 c_BlockSize    = 16;
  constexpr int32 c_BlockArea    = c_BlockSize * c_BlockSize;
  constexpr flt64 c_InvBlockArea = (flt64)1.0 / (flt64)c_BlockArea;

  __m512i SumR_I32_V  = _mm512_setzero_si512();
  __m512i SumT_I32_V  = _mm512_setzero_si512();
  __m512i SumRR_I64_V = _mm512_setzero_si512();
  __m512i SumTT_I64_V = _mm512_setzero_si512();
  __m512i SumRT_I64_V = _mm512_setzero_si512();

  for(int32 y = 0; y < c_BlockSize; y++)
  {    
    __m256i Tst_U16_V = _mm256_loadu_si256((__m256i*)(Tst));
    __m256i Ref_U16_V = _mm256_loadu_si256((__m256i*)(Ref));

    SumT_I32_V  = _mm512_add_epi32(SumT_I32_V , _mm512_cvtepu16_epi32(Tst_U16_V)); //SumT  += T;
    SumR_I32_V  = _mm512_add_epi32(SumR_I32_V , _mm512_cvtepu16_epi32(Ref_U16_V)); //SumR  += R;
    SumTT_I64_V = _mm512_add_epi64(SumTT_I64_V, _mm512_cvtepi32_epi64(_mm256_madd_epi16(Tst_U16_V, Tst_U16_V))); //SumT2 += T*T;
    SumRR_I64_V = _mm512_add_epi64(SumRR_I64_V, _mm512_cvtepi32_epi64(_mm256_madd_epi16(Ref_U16_V, Ref_U16_V))); //SumR2 += R*R;
    SumRT_I64_V = _mm512_add_epi64(SumRT_I64_V, _mm512_cvtepi32_epi64(_mm256_madd_epi16(Ref_U16_V, Tst_U16_V))); //SumRT += R*T;
    Ref += StrideR;
    Tst += StrideT;
  }  

  int32 SumR  = xHorVecSumI32_epi32(SumR_I32_V );
  int32 SumT  = xHorVecSumI32_epi32(SumT_I32_V );
  int64 SumRR = xHorVecSumI64_epi64(SumRR_I64_V);
  int64 SumTT = xHorVecSumI64_epi64(SumTT_I64_V);
  int64 SumRT = xHorVecSumI64_epi64(SumRT_I64_V);

  flt64 AvgR  = (flt64)SumR  * c_InvBlockArea;
  flt64 AvgT  = (flt64)SumT  * c_InvBlockArea;
  flt64 VarR2 = (flt64)SumRR * c_InvBlockArea - xPow2(AvgR);
  flt64 VarT2 = (flt64)SumTT * c_InvBlockArea - xPow2(AvgT);
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
flt64 xStructSimAVX512::CalcBlckAvg32(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL)
{
  constexpr int32 c_BlockSize    = 32;
  constexpr int32 c_BlockArea    = c_BlockSize * c_BlockSize;
  constexpr flt64 c_InvBlockArea = (flt64)1.0 / (flt64)c_BlockArea;

  __m512i SumR_I32_V  = _mm512_setzero_si512();
  __m512i SumT_I32_V  = _mm512_setzero_si512();
  __m512i SumRR_I64_V = _mm512_setzero_si512();
  __m512i SumTT_I64_V = _mm512_setzero_si512();
  __m512i SumRT_I64_V = _mm512_setzero_si512();

  for(int32 y = 0; y < c_BlockSize; y++)
  {
    __m512i Tst_U16_V = _mm512_loadu_si512((__m512i*)(Tst));
    __m512i Ref_U16_V = _mm512_loadu_si512((__m512i*)(Ref));

    SumT_I32_V = _mm512_add_epi32(SumT_I32_V, _mm512_add_epi32(_mm512_cvtepu16_epi32(_mm512_castsi512_si256(Tst_U16_V)), _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(Tst_U16_V, 1)))); //SumT  += T;
    SumR_I32_V = _mm512_add_epi32(SumR_I32_V, _mm512_add_epi32(_mm512_cvtepu16_epi32(_mm512_castsi512_si256(Ref_U16_V)), _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(Ref_U16_V, 1)))); //SumR  += R;
    __m512i TT_I32V = _mm512_madd_epi16(Tst_U16_V, Tst_U16_V);
    SumTT_I64_V = _mm512_add_epi64(SumTT_I64_V, _mm512_add_epi64(_mm512_cvtepi32_epi64(_mm512_castsi512_si256(TT_I32V)), _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(TT_I32V, 1)))); //SumT2 += T*T;
    __m512i RR_I32V = _mm512_madd_epi16(Ref_U16_V, Ref_U16_V);
    SumRR_I64_V = _mm512_add_epi64(SumRR_I64_V, _mm512_add_epi64(_mm512_cvtepi32_epi64(_mm512_castsi512_si256(RR_I32V)), _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(RR_I32V, 1)))); //SumT2 += T*T;
    __m512i RT_I32V = _mm512_madd_epi16(Ref_U16_V, Tst_U16_V);
    SumRT_I64_V = _mm512_add_epi64(SumRT_I64_V, _mm512_add_epi64(_mm512_cvtepi32_epi64(_mm512_castsi512_si256(RT_I32V)), _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(RT_I32V, 1)))); //SumT2 += T*T;
    Ref += StrideR;
    Tst += StrideT;
  }  

  int32 SumR  = xHorVecSumI32_epi32(SumR_I32_V );
  int32 SumT  = xHorVecSumI32_epi32(SumT_I32_V );
  int64 SumRR = xHorVecSumI64_epi64(SumRR_I64_V);
  int64 SumTT = xHorVecSumI64_epi64(SumTT_I64_V);
  int64 SumRT = xHorVecSumI64_epi64(SumRT_I64_V);

  flt64 AvgR  = (flt64)SumR  * c_InvBlockArea;
  flt64 AvgT  = (flt64)SumT  * c_InvBlockArea;
  flt64 VarR2 = (flt64)SumRR * c_InvBlockArea - xPow2(AvgR);
  flt64 VarT2 = (flt64)SumTT * c_InvBlockArea - xPow2(AvgT);
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

void xStructSimAVX512::CalcMultiBlckAvg8S4(flt64* restrict SSIMs, const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL)
{
  constexpr int32 c_BlockSize    = 8;
  constexpr int32 c_BlockArea    = c_BlockSize * c_BlockSize;
  constexpr flt64 c_InvBlockArea = (flt64)1.0 / (flt64)c_BlockArea;

  __m512i Sum2R_I32V  = _mm512_setzero_si512();
  __m512i Sum2T_I32V  = _mm512_setzero_si512();
  __m512i Sum4RR_I64V = _mm512_setzero_si512();
  __m512i Sum4TT_I64V = _mm512_setzero_si512();
  __m512i Sum4RT_I64V = _mm512_setzero_si512();

  const __m512i SelU16 = _mm512_setr_epi16(0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31);

  for(int32 y = 0; y < c_BlockSize; y++)
  {
    __m512i Tst_U16V = _mm512_loadu_si512((__m128i*)Tst);
    __m512i Ref_U16V = _mm512_loadu_si512((__m128i*)Ref);

    __m512i TstPerm_U16V = _mm512_permutex2var_epi16(Tst_U16V, SelU16, _mm512_setzero_si512());
    __m512i RefPerm_U16V = _mm512_permutex2var_epi16(Ref_U16V, SelU16, _mm512_setzero_si512());

    Sum2T_I32V       = _mm512_add_epi32(Sum2T_I32V, _mm512_add_epi32(_mm512_cvtepu16_epi32(_mm512_castsi512_si256(TstPerm_U16V)), _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(TstPerm_U16V, 1)))); //SumT  += T;
    Sum2R_I32V       = _mm512_add_epi32(Sum2R_I32V, _mm512_add_epi32(_mm512_cvtepu16_epi32(_mm512_castsi512_si256(RefPerm_U16V)), _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(RefPerm_U16V, 1)))); //SumR  += R;
    __m512i TT_I32V  = _mm512_madd_epi16(TstPerm_U16V, TstPerm_U16V);
    Sum4TT_I64V      = _mm512_add_epi64(Sum4TT_I64V, _mm512_add_epi64(_mm512_cvtepi32_epi64(_mm512_castsi512_si256(TT_I32V)), _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(TT_I32V, 1)))); //SumT2 += T*T;
    __m512i RR_I32V  = _mm512_madd_epi16(RefPerm_U16V, RefPerm_U16V);
    Sum4RR_I64V      = _mm512_add_epi64(Sum4RR_I64V, _mm512_add_epi64(_mm512_cvtepi32_epi64(_mm512_castsi512_si256(RR_I32V)), _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(RR_I32V, 1)))); //SumT2 += T*T;
    __m512i RT_I32V  = _mm512_madd_epi16(RefPerm_U16V, TstPerm_U16V);
    Sum4RT_I64V      = _mm512_add_epi64(Sum4RT_I64V, _mm512_add_epi64(_mm512_cvtepi32_epi64(_mm512_castsi512_si256(RT_I32V)), _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(RT_I32V, 1)))); //SumT2 += T*T;
    Ref += StrideR;
    Tst += StrideT;
  }

  //I32 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 - R and T
  //B0   x  x  x  x
  //B1         x  x  x  x
  //B2               x  x  x  x
  //B3                     x  x  x  x
  //B4                           x  x  x  x
  //B5                                 x  x  x  x
  //B7                                       x  x  x  x

  __m512i Sum4R_Sum4T_I32V = _mm512_hadd_epi32(Sum2R_I32V, Sum2T_I32V);

  //I32 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
  //B0   x  x                    x  x
  //B1      x  x                    x  x
  //B2         x  x                    x  x
  //B3            x  x                    x  x
  //B4               x  x                    x  x
  //B5                  x  x                    x  x
  //B6                     x  x                    x  x

  __m512i Sum8R_Sum8T_I32V = _mm512_add_epi32(Sum4R_Sum4T_I32V, _mm512_permutex2var_epi32(Sum4R_Sum4T_I32V, _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16), _mm512_setzero_si512()));

  //I64 00 01 02 03 04 05 06 07
  //B0   x  x
  //B1      x  x
  //B2         x  x
  //B3            x  x
  //B4               x  x
  //B5                  x  x
  //B6                     x  x

  __m512i Sum8RR_I64V = _mm512_add_epi64(Sum4RR_I64V, _mm512_permutex2var_epi64(Sum4RR_I64V, _mm512_setr_epi64(1,2,3,4,5,6,7,8), _mm512_setzero_si512()));
  __m512i Sum8TT_I64V = _mm512_add_epi64(Sum4TT_I64V, _mm512_permutex2var_epi64(Sum4TT_I64V, _mm512_setr_epi64(1,2,3,4,5,6,7,8), _mm512_setzero_si512()));
  __m512i Sum8RT_I64V = _mm512_add_epi64(Sum4RT_I64V, _mm512_permutex2var_epi64(Sum4RT_I64V, _mm512_setr_epi64(1,2,3,4,5,6,7,8), _mm512_setzero_si512()));

  //main path
  __m512d SumR_F64V  = _mm512_cvtepi32_pd(_mm512_castsi512_si256   (Sum8R_Sum8T_I32V  ));
  __m512d SumT_F64V  = _mm512_cvtepi32_pd(_mm512_extracti64x4_epi64(Sum8R_Sum8T_I32V,1));
  __m512d SumRR_F64V = _mm512_cvtepi64_pd(Sum8RR_I64V);
  __m512d SumTT_F64V = _mm512_cvtepi64_pd(Sum8TT_I64V);
  __m512d SumRT_F64V = _mm512_cvtepi64_pd(Sum8RT_I64V);

  const __m512d InvBlockArea_F64V = _mm512_set1_pd(c_InvBlockArea);

  __m512d AvgR_F64V     = _mm512_mul_pd(SumR_F64V, InvBlockArea_F64V);
  __m512d AvgT_F64V     = _mm512_mul_pd(SumT_F64V, InvBlockArea_F64V);
  __m512d Pow2AvgR_F64V = _mm512_mul_pd(AvgR_F64V, AvgR_F64V);
  __m512d Pow2AvgT_F64V = _mm512_mul_pd(AvgT_F64V, AvgT_F64V);
  __m512d VarR2_F64V    = _mm512_sub_pd(_mm512_mul_pd(SumRR_F64V, InvBlockArea_F64V), Pow2AvgR_F64V);
  __m512d VarT2_F64V    = _mm512_sub_pd(_mm512_mul_pd(SumTT_F64V, InvBlockArea_F64V), Pow2AvgT_F64V);
  __m512d CovRT_F64V    = _mm512_sub_pd(_mm512_mul_pd(SumRT_F64V, InvBlockArea_F64V), _mm512_mul_pd(AvgR_F64V, AvgT_F64V));

  const __m512d C1_F64V     = _mm512_set1_pd(C1 );
  const __m512d C2_F64V     = _mm512_set1_pd(C2 );
  const __m512d Const2_F64V = _mm512_set1_pd(2.0);

  if(CalcL)
  {
    __m512d L_F64V    = _mm512_div_pd(_mm512_fmadd_pd(Const2_F64V, _mm512_mul_pd(AvgR_F64V, AvgT_F64V), C1_F64V), _mm512_add_pd(_mm512_add_pd(Pow2AvgR_F64V, Pow2AvgT_F64V), C1_F64V));
    __m512d CS_F64V   = _mm512_div_pd(_mm512_fmadd_pd(Const2_F64V, CovRT_F64V, C2_F64V), _mm512_add_pd(_mm512_add_pd(VarR2_F64V, VarT2_F64V), C2_F64V));
    __m512d SSIM_F64V = _mm512_mul_pd(L_F64V, CS_F64V);
    _mm512_mask_storeu_pd(SSIMs, 0x7F, SSIM_F64V);
  }
  else
  {
    __m512d CS_F64V = _mm512_div_pd(_mm512_fmadd_pd(Const2_F64V, CovRT_F64V, C2_F64V), _mm512_add_pd(_mm512_add_pd(VarR2_F64V, VarT2_F64V), C2_F64V));
    _mm512_mask_storeu_pd(SSIMs, 0x7F, CS_F64V);
  }
}
void xStructSimAVX512::CalcMultiBlckAvg8S8(flt64* restrict SSIMs, const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL)
{
  constexpr int32 c_BlockSize    = 8;
  constexpr int32 c_BlockArea    = c_BlockSize * c_BlockSize;
  constexpr flt64 c_InvBlockArea = (flt64)1.0 / (flt64)c_BlockArea;

  __m512i Sum2R_I32V  = _mm512_setzero_si512();
  __m512i Sum2T_I32V  = _mm512_setzero_si512();
  __m512i Sum4RR_I64V = _mm512_setzero_si512();
  __m512i Sum4TT_I64V = _mm512_setzero_si512();
  __m512i Sum4RT_I64V = _mm512_setzero_si512();

  const __m512i SelU16 = _mm512_setr_epi16(0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31);

  for(int32 y = 0; y < c_BlockSize; y++)
  {
    __m512i Tst_U16V = _mm512_loadu_si512((__m128i*)Tst);
    __m512i Ref_U16V = _mm512_loadu_si512((__m128i*)Ref);

    __m512i TstPerm_U16V = _mm512_permutex2var_epi16(Tst_U16V, SelU16, _mm512_setzero_si512());
    __m512i RefPerm_U16V = _mm512_permutex2var_epi16(Ref_U16V, SelU16, _mm512_setzero_si512());

    Sum2T_I32V       = _mm512_add_epi32(Sum2T_I32V, _mm512_add_epi32(_mm512_cvtepu16_epi32(_mm512_castsi512_si256(TstPerm_U16V)), _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(TstPerm_U16V, 1)))); //SumT  += T;
    Sum2R_I32V       = _mm512_add_epi32(Sum2R_I32V, _mm512_add_epi32(_mm512_cvtepu16_epi32(_mm512_castsi512_si256(RefPerm_U16V)), _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(RefPerm_U16V, 1)))); //SumR  += R;
    __m512i TT_I32V  = _mm512_madd_epi16(TstPerm_U16V, TstPerm_U16V);
    Sum4TT_I64V      = _mm512_add_epi64(Sum4TT_I64V, _mm512_add_epi64(_mm512_cvtepi32_epi64(_mm512_castsi512_si256(TT_I32V)), _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(TT_I32V, 1)))); //SumT2 += T*T;
    __m512i RR_I32V  = _mm512_madd_epi16(RefPerm_U16V, RefPerm_U16V);
    Sum4RR_I64V      = _mm512_add_epi64(Sum4RR_I64V, _mm512_add_epi64(_mm512_cvtepi32_epi64(_mm512_castsi512_si256(RR_I32V)), _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(RR_I32V, 1)))); //SumT2 += T*T;
    __m512i RT_I32V  = _mm512_madd_epi16(RefPerm_U16V, TstPerm_U16V);
    Sum4RT_I64V      = _mm512_add_epi64(Sum4RT_I64V, _mm512_add_epi64(_mm512_cvtepi32_epi64(_mm512_castsi512_si256(RT_I32V)), _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(RT_I32V, 1)))); //SumT2 += T*T;
    Ref += StrideR;
    Tst += StrideT;
  }

  //R, T
  //I32 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 - R and T
  //B0   x  x  x  x
  //B1               x  x  x  x
  //B2                           x  x  x  x
  //B3                                       x  x  x  x

  __m512i Sum4R_Sum4T_I32V = _mm512_hadd_epi32(Sum2R_I32V, Sum2T_I32V);

  //I32 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
  //B0   x  x                    x  x
  //B2         x  x                    x  x
  //B4               x  x                    x  x
  //B7                     x  x                    x  x

  __m256i Sum8R_Sum8T_I32V = _mm512_castsi512_si256(_mm512_hadd_epi32(Sum4R_Sum4T_I32V, _mm512_setzero_si512()));

  //I32 00 01 02 03 04 05 06 07
  //B0   x           x           
  //B2      x           x        
  //B4         x           x      
  //B7            x           x   

  //RR, TT, RT
  //I64 00 01 02 03 04 05 06 07
  //B0   x  x
  //B2         x  x
  //B4               x  x
  //B6                     x  x

  __m256i Sum8RR_I64V = _mm512_castsi512_si256(_mm512_hadd_epi64(Sum4RR_I64V, _mm512_setzero_si512()));
  __m256i Sum8TT_I64V = _mm512_castsi512_si256(_mm512_hadd_epi64(Sum4TT_I64V, _mm512_setzero_si512()));
  __m256i Sum8RT_I64V = _mm512_castsi512_si256(_mm512_hadd_epi64(Sum4RT_I64V, _mm512_setzero_si512()));

  //main path
  __m256d SumR_F64V  = _mm256_cvtepi32_pd(_mm256_castsi256_si128  (Sum8R_Sum8T_I32V  ));
  __m256d SumT_F64V  = _mm256_cvtepi32_pd(_mm256_extracti128_si256(Sum8R_Sum8T_I32V,1));
  __m256d SumRR_F64V = _mm256_cvtepi64_pd(Sum8RR_I64V);
  __m256d SumTT_F64V = _mm256_cvtepi64_pd(Sum8TT_I64V);
  __m256d SumRT_F64V = _mm256_cvtepi64_pd(Sum8RT_I64V);

  const __m256d InvBlockArea_F64V = _mm256_set1_pd(c_InvBlockArea);

  __m256d AvgR_F64V     = _mm256_mul_pd(SumR_F64V, InvBlockArea_F64V);
  __m256d AvgT_F64V     = _mm256_mul_pd(SumT_F64V, InvBlockArea_F64V);
  __m256d Pow2AvgR_F64V = _mm256_mul_pd(AvgR_F64V, AvgR_F64V);
  __m256d Pow2AvgT_F64V = _mm256_mul_pd(AvgT_F64V, AvgT_F64V);
  __m256d VarR2_F64V    = _mm256_sub_pd(_mm256_mul_pd(SumRR_F64V, InvBlockArea_F64V), Pow2AvgR_F64V);
  __m256d VarT2_F64V    = _mm256_sub_pd(_mm256_mul_pd(SumTT_F64V, InvBlockArea_F64V), Pow2AvgT_F64V);
  __m256d CovRT_F64V    = _mm256_sub_pd(_mm256_mul_pd(SumRT_F64V, InvBlockArea_F64V), _mm256_mul_pd(AvgR_F64V, AvgT_F64V));

  const __m256d C1_F64V     = _mm256_set1_pd(C1 );
  const __m256d C2_F64V     = _mm256_set1_pd(C2 );
  const __m256d Const2_F64V = _mm256_set1_pd(2.0);

  if(CalcL)
  {
    __m256d L_F64V    = _mm256_div_pd(_mm256_fmadd_pd(Const2_F64V, _mm256_mul_pd(AvgR_F64V, AvgT_F64V), C1_F64V), _mm256_add_pd(_mm256_add_pd(Pow2AvgR_F64V, Pow2AvgT_F64V), C1_F64V));
    __m256d CS_F64V   = _mm256_div_pd(_mm256_fmadd_pd(Const2_F64V, CovRT_F64V, C2_F64V), _mm256_add_pd(_mm256_add_pd(VarR2_F64V, VarT2_F64V), C2_F64V));
    __m256d SSIM_F64V = _mm256_mul_pd(L_F64V, CS_F64V);
    _mm256_storeu_pd(SSIMs, SSIM_F64V);
  }
  else
  {
    __m256d CS_F64V = _mm256_div_pd(_mm256_fmadd_pd(Const2_F64V, CovRT_F64V, C2_F64V), _mm256_add_pd(_mm256_add_pd(VarR2_F64V, VarT2_F64V), C2_F64V));
    _mm256_storeu_pd(SSIMs, CS_F64V);
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_AVX512