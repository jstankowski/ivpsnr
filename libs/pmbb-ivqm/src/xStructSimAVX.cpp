/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xStructSimAVX.h"
#include "xHelpersSIMD.h"
#include "xStructSimConsts.h"

#if X_SIMD_CAN_USE_AVX

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

flt64 xStructSimAVX::CalcBlckAvg4(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL)
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
flt64 xStructSimAVX::CalcBlckAvg8(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL)
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
flt64 xStructSimAVX::CalcBlckAvg11(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, flt64 C1, flt64 C2, bool CalcL)
{
  constexpr int32 c_BlockSize    = xStructSimConsts::c_FilterSize;
  constexpr int32 c_BlockArea    = c_BlockSize * c_BlockSize;
  constexpr flt64 c_InvBlockArea = (flt64)1.0 / (flt64)c_BlockArea;

  const __m256i ZeroingMask = _mm256_setr_epi16(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0);

  __m256i SumR_I32_V  = _mm256_setzero_si256();
  __m256i SumT_I32_V  = _mm256_setzero_si256();
  __m256i SumRR_I64_V = _mm256_setzero_si256();
  __m256i SumTT_I64_V = _mm256_setzero_si256();
  __m256i SumRT_I64_V = _mm256_setzero_si256();

  for(int32 y = 0; y < c_BlockSize; y++)
  {
    __m256i Tst_U16_V = _mm256_loadu_si256((__m256i*)(Tst));
    __m256i Ref_U16_V = _mm256_loadu_si256((__m256i*)(Ref));

    //zero out last 5 lanes
    Tst_U16_V = _mm256_and_si256(Tst_U16_V, ZeroingMask);
    Ref_U16_V = _mm256_and_si256(Ref_U16_V, ZeroingMask);

    SumT_I32_V = _mm256_add_epi32(SumT_I32_V, _mm256_add_epi32(_mm256_cvtepu16_epi32(_mm256_castsi256_si128(Tst_U16_V)), _mm256_cvtepu16_epi32(_mm256_extracti128_si256(Tst_U16_V, 1)))); //SumT  += T;
    SumR_I32_V = _mm256_add_epi32(SumR_I32_V, _mm256_add_epi32(_mm256_cvtepu16_epi32(_mm256_castsi256_si128(Ref_U16_V)), _mm256_cvtepu16_epi32(_mm256_extracti128_si256(Ref_U16_V, 1)))); //SumR  += R;
    __m256i TT_I32V = _mm256_madd_epi16(Tst_U16_V, Tst_U16_V);
    SumTT_I64_V = _mm256_add_epi64(SumTT_I64_V, _mm256_add_epi64(_mm256_cvtepi32_epi64(_mm256_castsi256_si128(TT_I32V)), _mm256_cvtepi32_epi64(_mm256_extracti128_si256(TT_I32V, 1)))); //SumT2 += T*T;
    __m256i RR_I32V = _mm256_madd_epi16(Ref_U16_V, Ref_U16_V);
    SumRR_I64_V = _mm256_add_epi64(SumRR_I64_V, _mm256_add_epi64(_mm256_cvtepi32_epi64(_mm256_castsi256_si128(RR_I32V)), _mm256_cvtepi32_epi64(_mm256_extracti128_si256(RR_I32V, 1)))); //SumT2 += T*T;
    __m256i RT_I32V = _mm256_madd_epi16(Ref_U16_V, Tst_U16_V);
    SumRT_I64_V = _mm256_add_epi64(SumRT_I64_V, _mm256_add_epi64(_mm256_cvtepi32_epi64(_mm256_castsi256_si128(RT_I32V)), _mm256_cvtepi32_epi64(_mm256_extracti128_si256(RT_I32V, 1)))); //SumT2 += T*T;
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
flt64 xStructSimAVX::CalcBlckAvgM16(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 BlockSize, flt64 C1, flt64 C2, bool CalcL)
{
  assert(BlockSize == 8 || BlockSize == 16 || BlockSize == 32);
  const int32 c_BlockArea    = BlockSize * BlockSize;
  const flt64 c_InvBlockArea = (flt64)1.0 / (flt64)c_BlockArea;

  __m256i SumR_I32_V  = _mm256_setzero_si256();
  __m256i SumT_I32_V  = _mm256_setzero_si256();
  __m256i SumRR_I64_V = _mm256_setzero_si256();
  __m256i SumTT_I64_V = _mm256_setzero_si256();
  __m256i SumRT_I64_V = _mm256_setzero_si256();

  for(int32 y = 0; y < BlockSize; y++)
  {
    for(int32 x = 0; x < BlockSize; x += 16)
    {
      __m256i Tst_U16_V = _mm256_loadu_si256((__m256i*)(Tst+x));
      __m256i Ref_U16_V = _mm256_loadu_si256((__m256i*)(Ref+x));

      SumT_I32_V = _mm256_add_epi32(SumT_I32_V, _mm256_add_epi32(_mm256_cvtepu16_epi32(_mm256_castsi256_si128(Tst_U16_V)), _mm256_cvtepu16_epi32(_mm256_extracti128_si256(Tst_U16_V, 1)))); //SumT  += T;
      SumR_I32_V = _mm256_add_epi32(SumR_I32_V, _mm256_add_epi32(_mm256_cvtepu16_epi32(_mm256_castsi256_si128(Ref_U16_V)), _mm256_cvtepu16_epi32(_mm256_extracti128_si256(Ref_U16_V, 1)))); //SumR  += R;
      __m256i TT_I32V = _mm256_madd_epi16(Tst_U16_V, Tst_U16_V);
      SumTT_I64_V = _mm256_add_epi64(SumTT_I64_V, _mm256_add_epi64(_mm256_cvtepi32_epi64(_mm256_castsi256_si128(TT_I32V)), _mm256_cvtepi32_epi64(_mm256_extracti128_si256(TT_I32V, 1)))); //SumT2 += T*T;
      __m256i RR_I32V = _mm256_madd_epi16(Ref_U16_V, Ref_U16_V);
      SumRR_I64_V = _mm256_add_epi64(SumRR_I64_V, _mm256_add_epi64(_mm256_cvtepi32_epi64(_mm256_castsi256_si128(RR_I32V)), _mm256_cvtepi32_epi64(_mm256_extracti128_si256(RR_I32V, 1)))); //SumT2 += T*T;
      __m256i RT_I32V = _mm256_madd_epi16(Ref_U16_V, Tst_U16_V);
      SumRT_I64_V = _mm256_add_epi64(SumRT_I64_V, _mm256_add_epi64(_mm256_cvtepi32_epi64(_mm256_castsi256_si128(RT_I32V)), _mm256_cvtepi32_epi64(_mm256_extracti128_si256(RT_I32V, 1)))); //SumT2 += T*T;
    }
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

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_AVX