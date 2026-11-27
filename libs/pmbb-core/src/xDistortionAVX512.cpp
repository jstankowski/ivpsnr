/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xDistortionAVX512.h"
#include "xHelpersSIMD.h"

#if X_SIMD_CAN_USE_AVX512

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

int64 xDistortionAVX512::CalcSD14(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  __m512i SD_V512 = _mm512_setzero_si512();
  if(((uint32)Width & c_RemainderMask32<uint32>) == 0) //Width%32==0 - fast path without tail
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 32)
      {
        __m512i Tst_V512   = _mm512_loadu_si512((__m512i*)&Tst[x]);
        __m512i Ref_V512   = _mm512_loadu_si512((__m512i*)&Ref[x]);
        __m512i Diff_V512  = _mm512_sub_epi16     (Tst_V512, Ref_V512);
        __m512i Diff_V512A = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(Diff_V512, 1));
        __m512i Diff_V512B = _mm512_cvtepi16_epi32(_mm512_castsi512_si256   (Diff_V512   ));
        __m512i Sum_V512   = _mm512_add_epi32     (Diff_V512A, Diff_V512B);
        SD_V512 = _mm512_add_epi32(SD_V512, Sum_V512);
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y
  }
  else //any other
  {
    const int32 Width32 = (int32)((uint32)Width & c_MultipleMask32<uint32>);
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width32; x += 32)
      {
        __m512i Tst_V512   = _mm512_loadu_si512((__m512i*)&Tst[x]);
        __m512i Ref_V512   = _mm512_loadu_si512((__m512i*)&Ref[x]);
        __m512i Diff_V512  = _mm512_sub_epi16     (Tst_V512, Ref_V512);
        __m512i Diff_V512A = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(Diff_V512, 1));
        __m512i Diff_V512B = _mm512_cvtepi16_epi32(_mm512_castsi512_si256   (Diff_V512   ));
        __m512i Sum_V512   = _mm512_add_epi32     (Diff_V512A, Diff_V512B);
        SD_V512 = _mm512_add_epi32(SD_V512, Sum_V512);
      } //x
      const uint32    Remainder32 = (uint32)Width & c_RemainderMask32<uint32>;
      const __mmask32 Mask        = ((uint32)1 << Remainder32) - 1;
      __m512i Tst_V512   = _mm512_maskz_loadu_epi16(Mask, (__m512i*)&Tst [Width32]);
      __m512i Ref_V512  = _mm512_maskz_loadu_epi16(Mask, (__m512i*)&Ref[Width32]);
      __m512i Diff_V512  = _mm512_sub_epi16     (Tst_V512, Ref_V512);
      __m512i Diff_V512A = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(Diff_V512, 1));
      __m512i Diff_V512B = _mm512_cvtepi16_epi32(_mm512_castsi512_si256   (Diff_V512   ));
      __m512i Sum_V512   = _mm512_add_epi32     (Diff_V512A, Diff_V512B);
      SD_V512 = _mm512_add_epi32(SD_V512, Sum_V512);
      Tst += TstStride;
      Ref += RefStride;
    } //y
  } //any other
  
  __m256i SD_V256A = _mm512_extracti64x4_epi64(SD_V512, 1);
  __m256i SD_V256B = _mm512_castsi512_si256   (SD_V512   );
  __m256i SD_V256  = _mm256_add_epi32         (SD_V256A, SD_V256B);
  __m128i SD_V128A = _mm256_extracti128_si256 (SD_V256, 1);
  __m128i SD_V128B = _mm256_castsi256_si128   (SD_V256   );
  __m128i SD_V128  = _mm_add_epi32            (SD_V128A, SD_V128B);
  __m128i Tmp1V    = _mm_hadd_epi32           (SD_V128, SD_V128);
  __m128i Tmp2V    = _mm_hadd_epi32           (Tmp1V, Tmp1V);
  int64   SD       = _mm_extract_epi32(Tmp2V, 0);
  return SD;
}
int64 xDistortionAVX512::CalcSD16(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
    __m512i SD_I64_V512 = _mm512_setzero_si512();
  if(((uint32)Width & c_RemainderMask32<uint32>) == 0) //Width%32==0 - fast path without tail
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 32)
      {
        __m512i Tst_U16_V512   = _mm512_loadu_si512((__m512i*)(Tst+x));
        __m512i Ref_U16_V512   = _mm512_loadu_si512((__m512i*)(Ref+x));
        __m512i Tst_I32_V512A  = _mm512_unpacklo_epi16(Tst_U16_V512, _mm512_setzero_si512());
        __m512i Tst_I32_V512B  = _mm512_unpackhi_epi16(Tst_U16_V512, _mm512_setzero_si512());
        __m512i Ref_I32_V512A  = _mm512_unpacklo_epi16(Ref_U16_V512, _mm512_setzero_si512());
        __m512i Ref_I32_V512B  = _mm512_unpackhi_epi16(Ref_U16_V512, _mm512_setzero_si512());
        __m512i SumD_I32_V512  = _mm512_add_epi32(_mm512_sub_epi32(Tst_I32_V512A, Ref_I32_V512A), _mm512_sub_epi32(Tst_I32_V512B, Ref_I32_V512B));
        __m512i SumD_I64_V512A = _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(SumD_I32_V512, 1));
        __m512i SumD_I64_V512B = _mm512_cvtepi32_epi64(_mm512_castsi512_si256(SumD_I32_V512));
        __m512i SumD_I64_V512  = _mm512_add_epi64(SumD_I64_V512A, SumD_I64_V512B);
        SD_I64_V512            = _mm512_add_epi64(SD_I64_V512, SumD_I64_V512);
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y
  }
  else //any other
  {
    const int32 Width32 = (int32)((uint32)Width & c_MultipleMask32<uint32>);
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width32; x += 32)
      {
        __m512i Tst_U16_V512   = _mm512_loadu_si512((__m512i*)(Tst+x));
        __m512i Ref_U16_V512   = _mm512_loadu_si512((__m512i*)(Ref+x));
        __m512i Tst_I32_V512A  = _mm512_unpacklo_epi16(Tst_U16_V512, _mm512_setzero_si512());
        __m512i Tst_I32_V512B  = _mm512_unpackhi_epi16(Tst_U16_V512, _mm512_setzero_si512());
        __m512i Ref_I32_V512A  = _mm512_unpacklo_epi16(Ref_U16_V512, _mm512_setzero_si512());
        __m512i Ref_I32_V512B  = _mm512_unpackhi_epi16(Ref_U16_V512, _mm512_setzero_si512());
        __m512i SumD_I32_V512  = _mm512_add_epi32(_mm512_sub_epi32(Tst_I32_V512A, Ref_I32_V512A), _mm512_sub_epi32(Tst_I32_V512B, Ref_I32_V512B));
        __m512i SumD_I64_V512A = _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(SumD_I32_V512, 1));
        __m512i SumD_I64_V512B = _mm512_cvtepi32_epi64(_mm512_castsi512_si256(SumD_I32_V512));
        __m512i SumD_I64_V512  = _mm512_add_epi64(SumD_I64_V512A, SumD_I64_V512B);
        SD_I64_V512            = _mm512_add_epi64(SD_I64_V512, SumD_I64_V512);
      } //x
      const uint32    Remainder32 = (uint32)Width & c_RemainderMask32<uint32>;
      const __mmask32 Mask        = ((uint32)1 << Remainder32) - 1;
      __m512i Tst_U16_V512   = _mm512_maskz_loadu_epi16(Mask, (__m512i*)(Tst+Width32));
      __m512i Ref_U16_V512   = _mm512_maskz_loadu_epi16(Mask, (__m512i*)(Ref+Width32));
      __m512i Tst_I32_V512A  = _mm512_unpacklo_epi16(Tst_U16_V512, _mm512_setzero_si512());
      __m512i Tst_I32_V512B  = _mm512_unpackhi_epi16(Tst_U16_V512, _mm512_setzero_si512());
      __m512i Ref_I32_V512A  = _mm512_unpacklo_epi16(Ref_U16_V512, _mm512_setzero_si512());
      __m512i Ref_I32_V512B  = _mm512_unpackhi_epi16(Ref_U16_V512, _mm512_setzero_si512());
      __m512i SumD_I32_V512  = _mm512_add_epi32(_mm512_sub_epi32(Tst_I32_V512A, Ref_I32_V512A), _mm512_sub_epi32(Tst_I32_V512B, Ref_I32_V512B));
      __m512i SumD_I64_V512A = _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(SumD_I32_V512, 1));
      __m512i SumD_I64_V512B = _mm512_cvtepi32_epi64(_mm512_castsi512_si256(SumD_I32_V512));
      __m512i SumD_I64_V512  = _mm512_add_epi64(SumD_I64_V512A, SumD_I64_V512B);
      SD_I64_V512            = _mm512_add_epi64(SD_I64_V512, SumD_I64_V512);
      Tst += TstStride;
      Ref += RefStride;
    } //y
  } //any other
  
  int64 SD = xHorVecSumI64_epi64(SD_I64_V512);
  return SD;
}
uint64 xDistortionAVX512::CalcSAD16(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  __m512i SAD_I32_V512 = _mm512_setzero_si512();
  if(((uint32)Width & c_RemainderMask32<uint32>) == 0) //Width%32==0 - fast path without tail
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 32)
      {
        __m512i Tst_U16_V512   = _mm512_loadu_si512((__m512i*)&Tst[x]);
        __m512i Ref_U16_V512   = _mm512_loadu_si512((__m512i*)&Ref[x]);
        __m512i Tst_I32_V512A  = _mm512_unpacklo_epi16(Tst_U16_V512, _mm512_setzero_si512());
        __m512i Tst_I32_V512B  = _mm512_unpackhi_epi16(Tst_U16_V512, _mm512_setzero_si512());
        __m512i Ref_I32_V512A  = _mm512_unpacklo_epi16(Ref_U16_V512, _mm512_setzero_si512());
        __m512i Ref_I32_V512B  = _mm512_unpackhi_epi16(Ref_U16_V512, _mm512_setzero_si512());
        __m512i AbsD_I32_V512A = _mm512_abs_epi32(_mm512_sub_epi32(Tst_I32_V512A, Ref_I32_V512A));
        __m512i AbsD_I32_V512B = _mm512_abs_epi32(_mm512_sub_epi32(Tst_I32_V512B, Ref_I32_V512B));
        __m512i SumA_I32V512   = _mm512_add_epi32(AbsD_I32_V512A, AbsD_I32_V512B);
        SAD_I32_V512 = _mm512_add_epi32(SAD_I32_V512, SumA_I32V512);
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y
  }
  else //any other
  {
    const int32 Width32 = (int32)((uint32)Width & c_MultipleMask32<uint32>);
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width32; x += 32)
      {
        __m512i Tst_U16_V512   = _mm512_loadu_si512((__m512i*)&Tst[x]);
        __m512i Ref_U16_V512   = _mm512_loadu_si512((__m512i*)&Ref[x]);
        __m512i Tst_I32_V512A  = _mm512_unpacklo_epi16(Tst_U16_V512, _mm512_setzero_si512());
        __m512i Tst_I32_V512B  = _mm512_unpackhi_epi16(Tst_U16_V512, _mm512_setzero_si512());
        __m512i Ref_I32_V512A  = _mm512_unpacklo_epi16(Ref_U16_V512, _mm512_setzero_si512());
        __m512i Ref_I32_V512B  = _mm512_unpackhi_epi16(Ref_U16_V512, _mm512_setzero_si512());
        __m512i AbsD_I32_V512A = _mm512_abs_epi32(_mm512_sub_epi32(Tst_I32_V512A, Ref_I32_V512A));
        __m512i AbsD_I32_V512B = _mm512_abs_epi32(_mm512_sub_epi32(Tst_I32_V512B, Ref_I32_V512B));
        __m512i SumA_I32V512   = _mm512_add_epi32(AbsD_I32_V512A, AbsD_I32_V512B);
        SAD_I32_V512 = _mm512_add_epi32(SAD_I32_V512, SumA_I32V512);
      } //x
      const uint32    Remainder32 = (uint32)Width & c_RemainderMask32<uint32>;
      const __mmask32 Mask        = ((uint32)1 << Remainder32) - 1;
      __m512i Tst_U16_V512   = _mm512_maskz_loadu_epi16(Mask, (__m512i*)&Tst [Width32]);
      __m512i Ref_U16_V512   = _mm512_maskz_loadu_epi16(Mask, (__m512i*)&Ref[Width32]);
      __m512i Tst_I32_V512A  = _mm512_unpacklo_epi16(Tst_U16_V512, _mm512_setzero_si512());
      __m512i Tst_I32_V512B  = _mm512_unpackhi_epi16(Tst_U16_V512, _mm512_setzero_si512());
      __m512i Ref_I32_V512A  = _mm512_unpacklo_epi16(Ref_U16_V512, _mm512_setzero_si512());
      __m512i Ref_I32_V512B  = _mm512_unpackhi_epi16(Ref_U16_V512, _mm512_setzero_si512());
      __m512i AbsD_I32_V512A = _mm512_abs_epi32(_mm512_sub_epi32(Tst_I32_V512A, Ref_I32_V512A));
      __m512i AbsD_I32_V512B = _mm512_abs_epi32(_mm512_sub_epi32(Tst_I32_V512B, Ref_I32_V512B));
      __m512i SumA_I32V512   = _mm512_add_epi32(AbsD_I32_V512A, AbsD_I32_V512B);
      SAD_I32_V512 = _mm512_add_epi32(SAD_I32_V512, SumA_I32V512);
      Tst += TstStride;
      Ref += RefStride;
    } //y
  } //any other
  
  uint64 SAD = xHorVecSumI32_epi32(SAD_I32_V512);
  return SAD;
}
uint64 xDistortionAVX512::CalcSSD14(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  __m512i SSD_I64V = _mm512_setzero_si512();
  if(((uint32)Width & c_RemainderMask32<uint32>)==0) //Width%32==0 - fast path without tail
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=32)
      {
        __m512i Tst_U16V  = _mm512_loadu_si512((__m512i*) & Tst[x]);
        __m512i Ref_U16V  = _mm512_loadu_si512((__m512i*) & Ref[x]);
        __m512i Diff_I16V = _mm512_sub_epi16     (Tst_U16V , Ref_U16V);
        __m512i Pow_I32V  = _mm512_madd_epi16    (Diff_I16V, Diff_I16V);
        __m512i Pow_I64VA = _mm512_unpacklo_epi32(Pow_I32V, _mm512_setzero_si512());
        __m512i Pow_I64VB = _mm512_unpackhi_epi32(Pow_I32V, _mm512_setzero_si512());
        __m512i Sum_I64V  = _mm512_add_epi64     (Pow_I64VA, Pow_I64VB);
        SSD_I64V          = _mm512_add_epi64     (SSD_I64V, Sum_I64V);
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y
  }
  else //any other
  {
    const int32  Width32 = (int32)((uint32)Width & c_MultipleMask32<uint32>);
    for(int32 y=0; y<Height; y++)
    {
      for (int32 x = 0; x < Width32; x += 32)
      {
        __m512i Tst_U16V  = _mm512_loadu_si512((__m512i*) & Tst[x]);
        __m512i Ref_U16V  = _mm512_loadu_si512((__m512i*) & Ref[x]);
        __m512i Diff_I16V = _mm512_sub_epi16     (Tst_U16V , Ref_U16V);
        __m512i Pow_I32V  = _mm512_madd_epi16    (Diff_I16V, Diff_I16V);
        __m512i Pow_I64VA = _mm512_unpacklo_epi32(Pow_I32V, _mm512_setzero_si512());
        __m512i Pow_I64VB = _mm512_unpackhi_epi32(Pow_I32V, _mm512_setzero_si512());
        __m512i Sum_I64V  = _mm512_add_epi64     (Pow_I64VA, Pow_I64VB);
        SSD_I64V          = _mm512_add_epi64     (SSD_I64V, Sum_I64V);
      } //x
      const uint32    Remainder32 = (uint32)Width & c_RemainderMask32<uint32>;
      const __mmask32 Mask        = ((uint32)1 << Remainder32) - 1;
      __m512i Tst_U16V  = _mm512_maskz_loadu_epi16(Mask, (__m512i*) & Tst[Width32]);
      __m512i Ref_U16V  = _mm512_maskz_loadu_epi16(Mask, (__m512i*) & Ref[Width32]);
      __m512i Diff_I16V = _mm512_sub_epi16     (Tst_U16V , Ref_U16V);
      __m512i Pow_I32V  = _mm512_madd_epi16    (Diff_I16V, Diff_I16V);
      __m512i Pow_I64VA = _mm512_unpacklo_epi32(Pow_I32V, _mm512_setzero_si512());
      __m512i Pow_I64VB = _mm512_unpackhi_epi32(Pow_I32V, _mm512_setzero_si512());
      __m512i Sum_I64V  = _mm512_add_epi64     (Pow_I64VA, Pow_I64VB);
      SSD_I64V          = _mm512_add_epi64     (SSD_I64V, Sum_I64V);
      Tst += TstStride;
      Ref += RefStride;
    } //y
  }
  uint64 SSD = xHorVecSumI64_epi64(SSD_I64V);
  return SSD;
}
tSDSSD xDistortionAVX512::CalcSSS14(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  __m512i SD_I32V  = _mm512_setzero_si512();
  __m512i SSD_I64V = _mm512_setzero_si512();
  if(((uint32)Width & c_RemainderMask32<uint32>)==0) //Width%32==0 - fast path without tail
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=32)
      {
        __m512i Tst_U16V   = _mm512_loadu_si512((__m512i*) & Tst[x]);
        __m512i Ref_U16V   = _mm512_loadu_si512((__m512i*) & Ref[x]);
        __m512i Diff_I16V  = _mm512_sub_epi16     (Tst_U16V , Ref_U16V);
        __m512i Diff_I32VA = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(Diff_I16V, 1));
        __m512i Diff_I32VB = _mm512_cvtepi16_epi32(_mm512_castsi512_si256   (Diff_I16V));
        __m512i Sum_I32V   = _mm512_add_epi32     (Diff_I32VA, Diff_I32VB);
        SD_I32V            = _mm512_add_epi32     (SD_I32V, Sum_I32V);
        __m512i Pow_I32V   = _mm512_madd_epi16    (Diff_I16V, Diff_I16V);
        __m512i Pow_I64VA  = _mm512_unpacklo_epi32(Pow_I32V, _mm512_setzero_si512());
        __m512i Pow_I64VB  = _mm512_unpackhi_epi32(Pow_I32V, _mm512_setzero_si512());
        __m512i Sum_I64V   = _mm512_add_epi64     (Pow_I64VA, Pow_I64VB);
        SSD_I64V           = _mm512_add_epi64     (SSD_I64V, Sum_I64V);
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y
  }
  else //any other
  {
    const int32  Width32 = (int32)((uint32)Width & c_MultipleMask32<uint32>);
    for(int32 y=0; y<Height; y++)
    {
      for (int32 x = 0; x < Width32; x += 32)
      {
        __m512i Tst_U16V   = _mm512_loadu_si512((__m512i*) & Tst[x]);
        __m512i Ref_U16V   = _mm512_loadu_si512((__m512i*) & Ref[x]);
        __m512i Diff_I16V  = _mm512_sub_epi16     (Tst_U16V , Ref_U16V);
        __m512i Diff_I32VA = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(Diff_I16V, 1));
        __m512i Diff_I32VB = _mm512_cvtepi16_epi32(_mm512_castsi512_si256   (Diff_I16V));
        __m512i Sum_I32V   = _mm512_add_epi32     (Diff_I32VA, Diff_I32VB);
        SD_I32V            = _mm512_add_epi32     (SD_I32V, Sum_I32V);
        __m512i Pow_I32V   = _mm512_madd_epi16    (Diff_I16V, Diff_I16V);
        __m512i Pow_I64VA  = _mm512_unpacklo_epi32(Pow_I32V, _mm512_setzero_si512());
        __m512i Pow_I64VB  = _mm512_unpackhi_epi32(Pow_I32V, _mm512_setzero_si512());
        __m512i Sum_I64V   = _mm512_add_epi64     (Pow_I64VA, Pow_I64VB);
        SSD_I64V           = _mm512_add_epi64     (SSD_I64V, Sum_I64V);
      } //x
      const uint32    Remainder32 = (uint32)Width & c_RemainderMask32<uint32>;
      const __mmask32 Mask        = ((uint32)1 << Remainder32) - 1;
      __m512i Tst_U16V   = _mm512_maskz_loadu_epi16(Mask, (__m512i*) & Tst[Width32]);
      __m512i Ref_U16V   = _mm512_maskz_loadu_epi16(Mask, (__m512i*) & Ref[Width32]);
      __m512i Diff_I16V  = _mm512_sub_epi16     (Tst_U16V , Ref_U16V);
      __m512i Diff_I32VA = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(Diff_I16V, 1));
      __m512i Diff_I32VB = _mm512_cvtepi16_epi32(_mm512_castsi512_si256   (Diff_I16V));
      __m512i Sum_I32V   = _mm512_add_epi32     (Diff_I32VA, Diff_I32VB);
      SD_I32V            = _mm512_add_epi32     (SD_I32V, Sum_I32V);
      __m512i Pow_I32V   = _mm512_madd_epi16    (Diff_I16V, Diff_I16V);
      __m512i Pow_I64VA  = _mm512_unpacklo_epi32(Pow_I32V, _mm512_setzero_si512());
      __m512i Pow_I64VB  = _mm512_unpackhi_epi32(Pow_I32V, _mm512_setzero_si512());
      __m512i Sum_I64V   = _mm512_add_epi64     (Pow_I64VA, Pow_I64VB);
      SSD_I64V           = _mm512_add_epi64     (SSD_I64V, Sum_I64V);
      Tst += TstStride;
      Ref += RefStride;
    } //y
  }
  int64  SD  = xHorVecSumI64_epi32(SD_I32V );
  uint64 SSD = xHorVecSumI64_epi64(SSD_I64V);
  return { SD, SSD };
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_AVX512
