/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xDistortionAVX.h"
#include "xHelpersSIMD.h"

#if X_SIMD_CAN_USE_AVX

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

int64 xDistortionAVX::CalcSD14(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask16<uint32>)==0) //Width%16==0 - fast path without tail
  {
    __m256i SD_I64V = _mm256_setzero_si256();
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        __m256i Tst_U16V   = _mm256_loadu_si256((__m256i*)&Tst[x]);
        __m256i Ref_U16V   = _mm256_loadu_si256((__m256i*)&Ref[x]);
        __m256i Diff_I16V  = _mm256_sub_epi16     (Tst_U16V, Ref_U16V);
        __m256i Diff_I32VA = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(Diff_I16V, 1));
        __m256i Diff_I32VB = _mm256_cvtepi16_epi32(_mm256_castsi256_si128  (Diff_I16V   ));
        __m256i SumD_I32V  = _mm256_add_epi32     (Diff_I32VA, Diff_I32VB);
        __m256i SumD_I64VA = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(SumD_I32V, 1));
        __m256i SumD_I64VB = _mm256_cvtepi32_epi64(_mm256_castsi256_si128  (SumD_I32V   ));
        __m256i SumD_I64V  = _mm256_add_epi64     (SumD_I64VA, SumD_I64VB);
        SD_I64V            = _mm256_add_epi64     (SD_I64V, SumD_I64V);
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y
    int64 SD = xHorVecSumI64_epi64(SD_I64V);
    return SD;
  }
  else //any other
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16<uint32>);
    const int32 Width8  = (int32)((uint32)Width & c_MultipleMask8 <uint32>);
    const int32 Width4  = (int32)((uint32)Width & c_MultipleMask4 <uint32>);
    int64   SD          = 0;
    __m256i SD_I64_V256 = _mm256_setzero_si256();
    __m128i SD_I64_V128 = _mm_setzero_si128   ();

    for(int32 y=0; y<Height; y++)
    {
      for (int32 x = 0; x < Width16; x += 16)
      {
        __m256i Tst_U16V   = _mm256_loadu_si256((__m256i*)&Tst[x]);
        __m256i Ref_U16V   = _mm256_loadu_si256((__m256i*)&Ref[x]);
        __m256i Diff_I16V  = _mm256_sub_epi16     (Tst_U16V, Ref_U16V);
        __m256i Diff_I32VA = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(Diff_I16V, 1));
        __m256i Diff_I32VB = _mm256_cvtepi16_epi32(_mm256_castsi256_si128  (Diff_I16V   ));
        __m256i SumD_I32V  = _mm256_add_epi32     (Diff_I32VA, Diff_I32VB);
        __m256i SumD_I64VA = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(SumD_I32V, 1));
        __m256i SumD_I64VB = _mm256_cvtepi32_epi64(_mm256_castsi256_si128  (SumD_I32V   ));
        __m256i SumD_I64V  = _mm256_add_epi64     (SumD_I64VA, SumD_I64VB);
        SD_I64_V256        = _mm256_add_epi64     (SD_I64_V256, SumD_I64V);
      } //x
      for(int32 x=Width16; x<Width8; x+=8)
      {
        __m128i Tst_U16V  = _mm_loadu_si128((__m128i*) & Tst[x]);
        __m128i Ref_U16V  = _mm_loadu_si128((__m128i*) & Ref[x]);
        __m128i Diff_I16V = _mm_sub_epi16     (Tst_U16V, Ref_U16V);
        __m128i Diff_I32A = _mm_cvtepi16_epi32(Diff_I16V);
        __m128i Diff_I32B = _mm_cvtepi16_epi32(_mm_srli_si128(Diff_I16V, 8));
        __m128i SumD_I32V = _mm_add_epi32     (Diff_I32A, Diff_I32B);
        __m128i SumD_I64A = _mm_cvtepi32_epi64(SumD_I32V);
        __m128i SumD_I64B = _mm_cvtepi32_epi64(_mm_srli_si128(SumD_I32V, 8));
        __m128i SumD_I64V = _mm_add_epi64     (SumD_I64A, SumD_I64B);
        SD_I64_V128       = _mm_add_epi64     (SD_I64_V128, SumD_I64V);
      } //x
      for(int32 x=Width8; x<Width4; x+=4)
      {
        __m128i Tst_U16V  = _mm_loadl_epi64((__m128i*)&Tst[x]);
        __m128i Ref_U16V  = _mm_loadl_epi64((__m128i*)&Ref[x]);
        __m128i Diff_I16V = _mm_sub_epi16     (Tst_U16V, Ref_U16V);
        __m128i Diff_I32A = _mm_cvtepi16_epi32(Diff_I16V);
        __m128i SumD_I64A = _mm_cvtepi32_epi64(Diff_I32A);
        __m128i SumD_I64B = _mm_cvtepi32_epi64(_mm_srli_si128(Diff_I32A, 8));
        __m128i SumD_I64V = _mm_add_epi64     (SumD_I64A, SumD_I64B);
        SD_I64_V128       = _mm_add_epi64     (SD_I64_V128, SumD_I64V);
      } //x
      for(int32 x=Width4; x<Width; x++)
      {
        SD += (int32)Tst[x] - (int32)Ref[x];
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y

    __m128i SumSD_I64_V128 = _mm_add_epi64(_mm256_castsi256_si128(SD_I64_V256), _mm256_extracti128_si256(SD_I64_V256, 1));
    SD += xHorVecSumI64_epi64(_mm_add_epi64(SD_I64_V128, SumSD_I64_V128));
    return SD;
  }
}
int64 xDistortionAVX::CalcSD16(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask16<uint32>)==0) //Width%16==0 - fast path without tail
  {
    __m256i SD_I64_V256 = _mm256_setzero_si256();
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        __m256i Tst_U16_V256   = _mm256_loadu_si256((__m256i*)&Tst[x]);
        __m256i Ref_U16_V256   = _mm256_loadu_si256((__m256i*)&Ref[x]);
        __m256i Tst_I32_V256A  = _mm256_unpacklo_epi16(Tst_U16_V256, _mm256_setzero_si256());
        __m256i Tst_I32_V256B  = _mm256_unpackhi_epi16(Tst_U16_V256, _mm256_setzero_si256());
        __m256i Ref_I32_V256A  = _mm256_unpacklo_epi16(Ref_U16_V256, _mm256_setzero_si256());
        __m256i Ref_I32_V256B  = _mm256_unpackhi_epi16(Ref_U16_V256, _mm256_setzero_si256());
        __m256i SumD_I32_V256  = _mm256_add_epi32(_mm256_sub_epi32(Tst_I32_V256A, Ref_I32_V256A), _mm256_sub_epi32(Tst_I32_V256B, Ref_I32_V256B));
        __m256i SumD_I64_V256A = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(SumD_I32_V256, 1));
        __m256i SumD_I64_V256B = _mm256_cvtepi32_epi64(_mm256_castsi256_si128(SumD_I32_V256));
        __m256i SumD_I64_V256  = _mm256_add_epi64(SumD_I64_V256A, SumD_I64_V256B);
        SD_I64_V256            = _mm256_add_epi64(SD_I64_V256, SumD_I64_V256);
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y
    int64 SD = xHorVecSumI64_epi64(SD_I64_V256);
    return SD;
  }
  else //any other
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16<uint32>);
    const int32 Width8  = (int32)((uint32)Width & c_MultipleMask8 <uint32>);
    const int32 Width4  = (int32)((uint32)Width & c_MultipleMask4 <uint32>);
    int64   SD          = 0;
    __m256i SD_I64_V256 = _mm256_setzero_si256();
    __m128i SD_I64_V128 = _mm_setzero_si128   ();

    for(int32 y=0; y<Height; y++)
    {
      for (int32 x = 0; x < Width16; x += 16)
      {
        __m256i Tst_U16_V256   = _mm256_loadu_si256((__m256i*)&Tst[x]);
        __m256i Ref_U16_V256   = _mm256_loadu_si256((__m256i*)&Ref[x]);
        __m256i Tst_I32_V256A  = _mm256_unpacklo_epi16(Tst_U16_V256, _mm256_setzero_si256());
        __m256i Tst_I32_V256B  = _mm256_unpackhi_epi16(Tst_U16_V256, _mm256_setzero_si256());
        __m256i Ref_I32_V256A  = _mm256_unpacklo_epi16(Ref_U16_V256, _mm256_setzero_si256());
        __m256i Ref_I32_V256B  = _mm256_unpackhi_epi16(Ref_U16_V256, _mm256_setzero_si256());
        __m256i SumD_I32_V256  = _mm256_add_epi32(_mm256_sub_epi32(Tst_I32_V256A, Ref_I32_V256A), _mm256_sub_epi32(Tst_I32_V256B, Ref_I32_V256B));
        __m256i SumD_I64_V256A = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(SumD_I32_V256, 1));
        __m256i SumD_I64_V256B = _mm256_cvtepi32_epi64(_mm256_castsi256_si128(SumD_I32_V256));
        __m256i SumD_I64_V256  = _mm256_add_epi64(SumD_I64_V256A, SumD_I64_V256B);
        SD_I64_V256            = _mm256_add_epi64(SD_I64_V256, SumD_I64_V256);
      } //x
      for(int32 x=Width16; x<Width8; x+=8)
      {
        __m128i Tst_U16_V128   = _mm_loadu_si128((__m128i*)(Tst+x));
        __m128i Ref_U16_V128   = _mm_loadu_si128((__m128i*)(Ref+x));
        __m128i Tst_I32_V128A  = _mm_unpacklo_epi16(Tst_U16_V128, _mm_setzero_si128());
        __m128i Tst_I32_V128B  = _mm_unpackhi_epi16(Tst_U16_V128, _mm_setzero_si128());
        __m128i Ref_I32_V128A  = _mm_unpacklo_epi16(Ref_U16_V128, _mm_setzero_si128());
        __m128i Ref_I32_V128B  = _mm_unpackhi_epi16(Ref_U16_V128, _mm_setzero_si128());
        __m128i SumD_I32_V128  = _mm_add_epi32(_mm_sub_epi32(Tst_I32_V128A, Ref_I32_V128A), _mm_sub_epi32(Tst_I32_V128B, Ref_I32_V128B));
        __m128i SumD_I64_V128A = _mm_cvtepi32_epi64(SumD_I32_V128);
        __m128i SumD_I64_V128B = _mm_cvtepi32_epi64(_mm_srli_si128(SumD_I32_V128, 8));
        __m128i SumD_I64_V128  = _mm_add_epi64(SumD_I64_V128A, SumD_I64_V128B);
        SD_I64_V128            = _mm_add_epi64(SD_I64_V128, SumD_I64_V128);
      } //x
      for(int32 x=Width8; x<Width4; x+=4)
      {
        __m128i Tst_U16_V128   = _mm_loadl_epi64((__m128i*)(Tst+x));
        __m128i Ref_U16_V128   = _mm_loadl_epi64((__m128i*)(Ref+x));
        __m128i Tst_I32_V128   = _mm_unpacklo_epi16(Tst_U16_V128, _mm_setzero_si128());
        __m128i Ref_I32_V128   = _mm_unpacklo_epi16(Ref_U16_V128, _mm_setzero_si128());
        __m128i SumD_I32_V128  = _mm_sub_epi32(Tst_I32_V128, Ref_I32_V128);
        __m128i SumD_I64_V128A = _mm_cvtepi32_epi64(SumD_I32_V128);
        __m128i SumD_I64_V128B = _mm_cvtepi32_epi64(_mm_srli_si128(SumD_I32_V128, 8));
        __m128i SumD_I64_V128  = _mm_add_epi64(SumD_I64_V128A, SumD_I64_V128B);
        SD_I64_V128            = _mm_add_epi64(SD_I64_V128, SumD_I64_V128);
      } //x
      for(int32 x=Width4; x<Width; x++)
      {
        SD += (int32)Tst[x] - (int32)Ref[x];
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y

    __m128i SD_I64_V128A = _mm256_extracti128_si256(SD_I64_V256, 1);
    __m128i SD_I64_V128B = _mm256_castsi256_si128  (SD_I64_V256);
    SD_I64_V128 = _mm_add_epi64(SD_I64_V128, _mm_add_epi64(SD_I64_V128A, SD_I64_V128B));
    SD += xHorVecSumI64_epi64(SD_I64_V128);
    return SD;
  }
}
uint64 xDistortionAVX::CalcSAD16(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask16<uint32>)==0) //Width%16==0 - fast path without tail
  {
    __m256i SAD_I32_V256 = _mm256_setzero_si256();
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
         __m256i Tst_U16_V256   = _mm256_loadu_si256((__m256i*)&Tst[x]);
         __m256i Ref_U16_V256   = _mm256_loadu_si256((__m256i*)&Ref[x]);
         __m256i Tst_I32_V256A  = _mm256_unpacklo_epi16(Tst_U16_V256, _mm256_setzero_si256());
         __m256i Tst_I32_V256B  = _mm256_unpackhi_epi16(Tst_U16_V256, _mm256_setzero_si256());
         __m256i Ref_I32_V256A  = _mm256_unpacklo_epi16(Ref_U16_V256, _mm256_setzero_si256());
         __m256i Ref_I32_V256B  = _mm256_unpackhi_epi16(Ref_U16_V256, _mm256_setzero_si256());
         __m256i AbsD_I32_V256A = _mm256_abs_epi32(_mm256_sub_epi32(Tst_I32_V256A, Ref_I32_V256A));
         __m256i AbsD_I32_V256B = _mm256_abs_epi32(_mm256_sub_epi32(Tst_I32_V256B, Ref_I32_V256B));
         __m256i SumA_I32_V256  = _mm256_add_epi32(AbsD_I32_V256A, AbsD_I32_V256B);
         SAD_I32_V256 = _mm256_add_epi32(SAD_I32_V256, SumA_I32_V256);
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y
    uint64 SAD = xHorVecSumI32_epi32(SAD_I32_V256);
    return SAD;
  }
  else //any other
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16<uint32>);
    const int32 Width8  = (int32)((uint32)Width & c_MultipleMask8<uint32> );
    const int32 Width4  = (int32)((uint32)Width & c_MultipleMask4<uint32> );
    uint64  SAD          = 0;
    __m256i SAD_I32_V256 = _mm256_setzero_si256();
    __m128i SAD_I32_V128 = _mm_setzero_si128   ();

    for(int32 y=0; y<Height; y++)
    {
      for (int32 x = 0; x < Width16; x += 16)
      {
         __m256i Tst_U16_V256   = _mm256_loadu_si256((__m256i*)&Tst[x]);
         __m256i Ref_U16_V256   = _mm256_loadu_si256((__m256i*)&Ref[x]);
         __m256i Tst_I32_V256A  = _mm256_unpacklo_epi16(Tst_U16_V256, _mm256_setzero_si256());
         __m256i Tst_I32_V256B  = _mm256_unpackhi_epi16(Tst_U16_V256, _mm256_setzero_si256());
         __m256i Ref_I32_V256A  = _mm256_unpacklo_epi16(Ref_U16_V256, _mm256_setzero_si256());
         __m256i Ref_I32_V256B  = _mm256_unpackhi_epi16(Ref_U16_V256, _mm256_setzero_si256());
         __m256i AbsD_I32_V256A = _mm256_abs_epi32(_mm256_sub_epi32(Tst_I32_V256A, Ref_I32_V256A));
         __m256i AbsD_I32_V256B = _mm256_abs_epi32(_mm256_sub_epi32(Tst_I32_V256B, Ref_I32_V256B));
         __m256i SumA_I32_V256  = _mm256_add_epi32(AbsD_I32_V256A, AbsD_I32_V256B);
         SAD_I32_V256 = _mm256_add_epi32(SAD_I32_V256, SumA_I32_V256);
      } //x
      for(int32 x=Width16; x<Width8; x+=8)
      {
         __m128i Tst_U16_V128   = _mm_loadu_si128((__m128i*)&Tst[x]);
         __m128i Ref_U16_V128   = _mm_loadu_si128((__m128i*)&Ref[x]);
         __m128i Tst_I32_V128A  = _mm_unpacklo_epi16(Tst_U16_V128, _mm_setzero_si128());
         __m128i Tst_I32_V128B  = _mm_unpackhi_epi16(Tst_U16_V128, _mm_setzero_si128());
         __m128i Ref_I32_V128A  = _mm_unpacklo_epi16(Ref_U16_V128, _mm_setzero_si128());
         __m128i Ref_I32_V128B  = _mm_unpackhi_epi16(Ref_U16_V128, _mm_setzero_si128());
         __m128i AbsD_I32_V128A = _mm_abs_epi32(_mm_sub_epi32(Tst_I32_V128A, Ref_I32_V128A));
         __m128i AbsD_I32_V128B = _mm_abs_epi32(_mm_sub_epi32(Tst_I32_V128B, Ref_I32_V128B));
         __m128i SumA_I32_V128  = _mm_add_epi32(AbsD_I32_V128A, AbsD_I32_V128B);
         SAD_I32_V128           = _mm_add_epi32(SAD_I32_V128, SumA_I32_V128);
      } //x
      for(int32 x=Width8; x<Width4; x+=4)
      {
         __m128i Tst_U16_V128   = _mm_loadl_epi64((__m128i*)&Tst[x]);
         __m128i Ref_U16_V128   = _mm_loadl_epi64((__m128i*)&Ref[x]);
         __m128i Tst_I32_V128   = _mm_unpacklo_epi16(Tst_U16_V128, _mm_setzero_si128());
         __m128i Ref_I32_V128   = _mm_unpacklo_epi16(Ref_U16_V128, _mm_setzero_si128());
         __m128i AbsD_I32_V128  = _mm_abs_epi32(_mm_sub_epi32(Tst_I32_V128, Ref_I32_V128));
         SAD_I32_V128           = _mm_add_epi32(SAD_I32_V128, AbsD_I32_V128);
      } //x
      for(int32 x=Width4; x<Width; x++)
      {
        SAD += (uint32)xAbs(((int32)Tst[x]) - ((int32)Ref[x]));
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y

    __m128i SAD_I32_V128A = _mm256_extracti128_si256(SAD_I32_V256, 1);
    __m128i SAD_I32_V128B = _mm256_castsi256_si128  (SAD_I32_V256);
    SAD_I32_V128 = _mm_add_epi32(SAD_I32_V128, _mm_add_epi32(SAD_I32_V128A, SAD_I32_V128B));
    SAD += xHorVecSumI32_epi32(SAD_I32_V128);
    return SAD;
  } //any other
}
uint64 xDistortionAVX::CalcSSD14(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask16<uint32>)==0) //Width%16==0 - fast path without tail
  {
    __m256i SSD_U64_V256 = _mm256_setzero_si256();
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        __m256i Tst_U16V   = _mm256_loadu_si256((__m256i*) & Tst[x]);
        __m256i Ref_U16V   = _mm256_loadu_si256((__m256i*) & Ref[x]);
        __m256i Diff_I16V  = _mm256_sub_epi16     (Tst_U16V, Ref_U16V);
        __m256i PowD_U32V  = _mm256_madd_epi16    (Diff_I16V, Diff_I16V);
        __m256i PowD_U64VA = _mm256_unpacklo_epi32(PowD_U32V, _mm256_setzero_si256());
        __m256i PowD_U64VB = _mm256_unpackhi_epi32(PowD_U32V, _mm256_setzero_si256());
        __m256i SumD_U64V  = _mm256_add_epi64     (PowD_U64VA, PowD_U64VB);
        SSD_U64_V256       = _mm256_add_epi64     (SSD_U64_V256, SumD_U64V);
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y
    uint64 SSD = xHorVecSumI64_epi64(SSD_U64_V256);
    return SSD;
  }
  else //any other
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16<uint32>);
    const int32 Width8  = (int32)((uint32)Width & c_MultipleMask8<uint32> );
    const int32 Width4  = (int32)((uint32)Width & c_MultipleMask4<uint32> );
    uint64  SSD          = 0;
    __m256i SSD_U64_V256 = _mm256_setzero_si256();
    __m128i SSD_U64_V128 = _mm_setzero_si128   ();

    for(int32 y=0; y<Height; y++)
    {
      for (int32 x = 0; x < Width16; x += 16)
      {
        __m256i Tst_U16V   = _mm256_loadu_si256((__m256i*) & Tst[x]);
        __m256i Ref_U16V   = _mm256_loadu_si256((__m256i*) & Ref[x]);
        __m256i Diff_I16V  = _mm256_sub_epi16     (Tst_U16V, Ref_U16V);
        __m256i PowD_U32V  = _mm256_madd_epi16    (Diff_I16V, Diff_I16V);
        __m256i PowD_U64VA = _mm256_unpacklo_epi32(PowD_U32V, _mm256_setzero_si256());
        __m256i PowD_U64VB = _mm256_unpackhi_epi32(PowD_U32V, _mm256_setzero_si256());
        __m256i SumD_U64V  = _mm256_add_epi64     (PowD_U64VA, PowD_U64VB);
        SSD_U64_V256       = _mm256_add_epi64     (SSD_U64_V256, SumD_U64V);
      } //x
      for(int32 x=Width16; x<Width8; x+=8)
      {
        __m128i Tst_U16V   = _mm_loadu_si128((__m128i*) & Tst[x]);
        __m128i Ref_U16V   = _mm_loadu_si128((__m128i*) & Ref[x]);
        __m128i Diff_I16V  = _mm_sub_epi16     (Tst_U16V, Ref_U16V);
        __m128i PowD_U32V  = _mm_madd_epi16    (Diff_I16V, Diff_I16V);
        __m128i PowD_U64VA = _mm_unpacklo_epi32(PowD_U32V, _mm_setzero_si128());
        __m128i PowD_U64VB = _mm_unpackhi_epi32(PowD_U32V, _mm_setzero_si128());
        __m128i SumD_U64V  = _mm_add_epi64     (PowD_U64VA, PowD_U64VB);
        SSD_U64_V128       = _mm_add_epi64     (SSD_U64_V128, SumD_U64V);
      } //x
      for(int32 x=Width8; x<Width4; x+=4)
      {
        __m128i Tst_U16V   = _mm_loadl_epi64((__m128i*)&Tst[x]);
        __m128i Ref_U16V   = _mm_loadl_epi64((__m128i*)&Ref[x]);
        __m128i Diff_I16V  = _mm_sub_epi16     (Tst_U16V, Ref_U16V);
        __m128i PowD_U32V  = _mm_madd_epi16    (Diff_I16V, Diff_I16V);
        __m128i PowD_U64VA = _mm_unpacklo_epi32(PowD_U32V, _mm_setzero_si128());
        SSD_U64_V128       = _mm_add_epi64     (SSD_U64_V128, PowD_U64VA);
      } //x
      for(int32 x=Width4; x<Width; x++)
      {
        SSD += xPow2((int32)Tst[x] - (int32)Ref[x]);
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y

    __m128i SumSSD_I64_V128 = _mm_add_epi64(_mm256_castsi256_si128(SSD_U64_V256), _mm256_extracti128_si256(SSD_U64_V256, 1));
    SSD += xHorVecSumI64_epi64(_mm_add_epi64(SSD_U64_V128, SumSSD_I64_V128));
    return SSD;
  }
}
tSDSSD xDistortionAVX::CalcSSS14(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask16<uint32>)==0) //Width%16==0 - fast path without tail
  {
    __m256i SD_I64_V256  = _mm256_setzero_si256();
    __m256i SSD_U64_V256 = _mm256_setzero_si256();
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        __m256i Tst_U16V   = _mm256_loadu_si256((__m256i*) & Tst[x]);
        __m256i Ref_U16V   = _mm256_loadu_si256((__m256i*) & Ref[x]);
        __m256i Diff_I16V  = _mm256_sub_epi16     (Tst_U16V, Ref_U16V);
        __m256i Diff_I32VA = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(Diff_I16V, 1));
        __m256i Diff_I32VB = _mm256_cvtepi16_epi32(_mm256_castsi256_si128  (Diff_I16V   ));
        __m256i SumD_I32V  = _mm256_add_epi32     (Diff_I32VA, Diff_I32VB);
        __m256i SumD_I64VA = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(SumD_I32V, 1));
        __m256i SumD_I64VB = _mm256_cvtepi32_epi64(_mm256_castsi256_si128  (SumD_I32V   ));
        __m256i SumD_I64V  = _mm256_add_epi64     (SumD_I64VA, SumD_I64VB);
        SD_I64_V256        = _mm256_add_epi64     (SD_I64_V256, SumD_I64V);
        __m256i PowD_U32V  = _mm256_madd_epi16    (Diff_I16V, Diff_I16V);
        __m256i PowD_U64VA = _mm256_unpacklo_epi32(PowD_U32V, _mm256_setzero_si256());
        __m256i PowD_U64VB = _mm256_unpackhi_epi32(PowD_U32V, _mm256_setzero_si256());
        __m256i SumD_U64V  = _mm256_add_epi64     (PowD_U64VA, PowD_U64VB);
        SSD_U64_V256       = _mm256_add_epi64     (SSD_U64_V256, SumD_U64V);
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y
    int64  SD  = xHorVecSumI64_epi64(SD_I64_V256 );
    uint64 SSD = xHorVecSumI64_epi64(SSD_U64_V256);
    return { SD, SSD };
  }
  else //any other
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16<uint32>);
    const int32 Width8  = (int32)((uint32)Width & c_MultipleMask8<uint32> );
    const int32 Width4  = (int32)((uint32)Width & c_MultipleMask4<uint32> );
    int64   SD           = 0;
    __m256i SD_I64_V256  = _mm256_setzero_si256();
    __m128i SD_I64_V128  = _mm_setzero_si128   ();
    uint64  SSD          = 0;
    __m256i SSD_U64_V256 = _mm256_setzero_si256();
    __m128i SSD_U64_V128 = _mm_setzero_si128   ();

    for(int32 y=0; y<Height; y++)
    {
      for (int32 x = 0; x < Width16; x += 16)
      {
        __m256i Tst_U16V   = _mm256_loadu_si256((__m256i*) & Tst[x]);
        __m256i Ref_U16V   = _mm256_loadu_si256((__m256i*) & Ref[x]);
        __m256i Diff_I16V  = _mm256_sub_epi16     (Tst_U16V, Ref_U16V);
        __m256i Diff_I32VA = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(Diff_I16V, 1));
        __m256i Diff_I32VB = _mm256_cvtepi16_epi32(_mm256_castsi256_si128  (Diff_I16V   ));
        __m256i SumD_I32V  = _mm256_add_epi32     (Diff_I32VA, Diff_I32VB);
        __m256i SumD_I64VA = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(SumD_I32V, 1));
        __m256i SumD_I64VB = _mm256_cvtepi32_epi64(_mm256_castsi256_si128  (SumD_I32V   ));
        __m256i SumD_I64V  = _mm256_add_epi64     (SumD_I64VA, SumD_I64VB);
        SD_I64_V256        = _mm256_add_epi64     (SD_I64_V256, SumD_I64V);
        __m256i PowD_U32V  = _mm256_madd_epi16    (Diff_I16V, Diff_I16V);
        __m256i PowD_U64VA = _mm256_unpacklo_epi32(PowD_U32V, _mm256_setzero_si256());
        __m256i PowD_U64VB = _mm256_unpackhi_epi32(PowD_U32V, _mm256_setzero_si256());
        __m256i SumD_U64V  = _mm256_add_epi64     (PowD_U64VA, PowD_U64VB);
        SSD_U64_V256       = _mm256_add_epi64     (SSD_U64_V256, SumD_U64V);
      } //x
      for(int32 x=Width16; x<Width8; x+=8)
      {
        __m128i Tst_U16V  = _mm_loadu_si128((__m128i*) & Tst[x]);
        __m128i Ref_U16V  = _mm_loadu_si128((__m128i*) & Ref[x]);
        __m128i Diff_I16V = _mm_sub_epi16     (Tst_U16V, Ref_U16V);
        __m128i Diff_I32A = _mm_cvtepi16_epi32(Diff_I16V);
        __m128i Diff_I32B = _mm_cvtepi16_epi32(_mm_srli_si128(Diff_I16V, 8));
        __m128i Sum_I32V  = _mm_add_epi32     (Diff_I32A, Diff_I32B);
        __m128i Sum_I64A  = _mm_cvtepi32_epi64(Sum_I32V);
        __m128i Sum_I64B  = _mm_cvtepi32_epi64(_mm_srli_si128(Sum_I32V, 8));
        __m128i Sum_I64V  = _mm_add_epi64     (Sum_I64A, Sum_I64B);
        SD_I64_V128       = _mm_add_epi64     (SD_I64_V128, Sum_I64V);
        __m128i Pow_U32V  = _mm_madd_epi16    (Diff_I16V, Diff_I16V);
        __m128i Pow_U64VA = _mm_unpacklo_epi32(Pow_U32V, _mm_setzero_si128());
        __m128i Pow_U64VB = _mm_unpackhi_epi32(Pow_U32V, _mm_setzero_si128());
        __m128i Sum_U64V  = _mm_add_epi64     (Pow_U64VA, Pow_U64VB);
        SSD_U64_V128      = _mm_add_epi64     (SSD_U64_V128, Sum_U64V);
      } //x
      for(int32 x=Width8; x<Width4; x+=4)
      {
        __m128i Tst_U16V  = _mm_loadl_epi64((__m128i*)&Tst[x]);
        __m128i Ref_U16V  = _mm_loadl_epi64((__m128i*)&Ref[x]);
        __m128i Diff_I16V = _mm_sub_epi16     (Tst_U16V, Ref_U16V);
        __m128i Diff_I32A = _mm_cvtepi16_epi32(Diff_I16V);
        __m128i Sum_I64A  = _mm_cvtepi32_epi64(Diff_I32A);
        __m128i Sum_I64B  = _mm_cvtepi32_epi64(_mm_srli_si128(Diff_I32A, 8));
        __m128i Sum_I64V  = _mm_add_epi64     (Sum_I64A, Sum_I64B);
        SD_I64_V128       = _mm_add_epi64     (SD_I64_V128, Sum_I64V);
        __m128i Pow_U32V  = _mm_madd_epi16    (Diff_I16V, Diff_I16V);
        __m128i Pow_U64VA = _mm_unpacklo_epi32(Pow_U32V, _mm_setzero_si128());
        SSD_U64_V128      = _mm_add_epi64     (SSD_U64_V128, Pow_U64VA);
      } //x
      for(int32 x=Width4; x<Width; x++)
      {
        int32 Diff = (int32)Tst[x] - (int32)Ref[x];
        SD  += Diff;
        SSD += xPow2(Diff);
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y

    __m128i SumSD_I64_V128 = _mm_add_epi64(_mm256_castsi256_si128(SD_I64_V256), _mm256_extracti128_si256(SD_I64_V256, 1));
    SD += xHorVecSumI64_epi64(_mm_add_epi64(SD_I64_V128, SumSD_I64_V128));
    __m128i SumSSD_I64_V128 = _mm_add_epi64(_mm256_castsi256_si128(SSD_U64_V256), _mm256_extracti128_si256(SSD_U64_V256, 1));
    SSD += xHorVecSumI64_epi64(_mm_add_epi64(SSD_U64_V128, SumSSD_I64_V128));
    return { SD, SSD };
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_AVX
