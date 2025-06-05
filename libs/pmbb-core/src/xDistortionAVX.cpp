/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
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
    int64 SD = xHorVecSum_epi64(SD_I64_V256);
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
    SD += xHorVecSum_epi64(SD_I64_V128);
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
    int64 SD = xHorVecSum_epi64(SD_I64_V256);
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
    SD += xHorVecSum_epi64(SD_I64_V128);
    return SD;
  }
}

uint64 xDistortionAVX::CalcSAD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
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
    uint64 SAD = xHorVecSum_epi32(SAD_I32_V256);
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
    SAD += xHorVecSum_epi32(SAD_I32_V128);
    return SAD;
  } //any other
}
uint64 xDistortionAVX::CalcSSD14(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  //up to 14 bit input
  if(((uint32)Width & c_RemainderMask16<uint32>)==0) //Width%16==0 - fast path without tail
  {
    __m256i SSD_V256 = _mm256_setzero_si256();
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        __m256i Tst_V256  = _mm256_loadu_si256((__m256i*) & Tst[x]);
        __m256i Ref_V256  = _mm256_loadu_si256((__m256i*) & Ref[x]);
        __m256i Diff_V256 = _mm256_sub_epi16     (Tst_V256 , Ref_V256);
        __m256i Pow_V256  = _mm256_madd_epi16    (Diff_V256, Diff_V256);
        __m256i Pow_V256A = _mm256_unpacklo_epi32(Pow_V256 , _mm256_setzero_si256());
        __m256i Pow_V256B = _mm256_unpackhi_epi32(Pow_V256 , _mm256_setzero_si256());
        __m256i Sum_V256  = _mm256_add_epi64     (Pow_V256A, Pow_V256B);
        SSD_V256          = _mm256_add_epi64     (SSD_V256, Sum_V256);
      } //x
      Tst  += TstStride;
      Ref += RefStride;
    } //y
    __m128i SSD_V128A = _mm256_extracti128_si256(SSD_V256, 1);
    __m128i SSD_V128B = _mm256_castsi256_si128  (SSD_V256);
    __m128i SSD_V128  = _mm_add_epi64           (SSD_V128A, SSD_V128B);
    uint64 SSD        = _mm_extract_epi64(SSD_V128, 0) + _mm_extract_epi64(SSD_V128, 1);
    return SSD;
  }
  else //any other
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16<uint32>);
    const int32 Width8  = (int32)((uint32)Width & c_MultipleMask8<uint32> );
    const int32 Width4  = (int32)((uint32)Width & c_MultipleMask4<uint32> );
    uint64  SSD = 0;
    __m256i SSD_V256 = _mm256_setzero_si256();
    __m128i SSD_V128 = _mm_setzero_si128   ();

    for(int32 y=0; y<Height; y++)
    {
      for (int32 x = 0; x < Width16; x += 16)
      {
        __m256i Tst_V256  = _mm256_loadu_si256((__m256i*) & Tst[x]);
        __m256i Ref_V256  = _mm256_loadu_si256((__m256i*) & Ref[x]);
        __m256i Diff_V256 = _mm256_sub_epi16     (Tst_V256, Ref_V256);
        __m256i Pow_V256  = _mm256_madd_epi16    (Diff_V256, Diff_V256);
        __m256i Pow_V256A = _mm256_unpacklo_epi32(Pow_V256, _mm256_setzero_si256());
        __m256i Pow_V256B = _mm256_unpackhi_epi32(Pow_V256, _mm256_setzero_si256());
        __m256i Sum_V256  = _mm256_add_epi64     (Pow_V256A, Pow_V256B);
        SSD_V256 = _mm256_add_epi64(SSD_V256, Sum_V256);
      } //x
      for(int32 x=Width16; x<Width8; x+=8)
      {
        __m128i Tst_V128  = _mm_loadu_si128((__m128i*)&Tst [x]);
        __m128i Ref_V128  = _mm_loadu_si128((__m128i*)&Ref[x]);
        __m128i Diff_V128 = _mm_sub_epi16     (Tst_V128, Ref_V128);
        __m128i Pow_V128  = _mm_madd_epi16    (Diff_V128, Diff_V128);
        __m128i Pow_V128A = _mm_unpacklo_epi32(Pow_V128, _mm_setzero_si128());
        __m128i Pow_V128B = _mm_unpackhi_epi32(Pow_V128, _mm_setzero_si128());
        __m128i Sum_V128  = _mm_add_epi64(Pow_V128A, Pow_V128B);
        SSD_V128 = _mm_add_epi64(SSD_V128, Sum_V128);
      } //x
      for(int32 x=Width8; x<Width4; x+=4)
      {
        __m128i Tst_V128  = _mm_loadl_epi64((__m128i*)&Tst[x]);
        __m128i Ref_V128  = _mm_loadl_epi64((__m128i*)&Ref[x]);
        __m128i Diff_V128 = _mm_sub_epi16  (Tst_V128, Ref_V128);
        __m128i Pow_V128  = _mm_madd_epi16 (Diff_V128, Diff_V128);
        Pow_V128 = _mm_unpacklo_epi32(Pow_V128, _mm_setzero_si128());
        SSD_V128 = _mm_add_epi64(SSD_V128, Pow_V128);
      } //x
      for(int32 x=Width4; x<Width; x++)
      {
        SSD += xPow2((int32)Tst[x] - (int32)Ref[x]);
      } //x
      Tst  += TstStride;
      Ref += RefStride;
    } //y
    __m128i SSD_V128A = _mm256_extracti128_si256(SSD_V256, 1);
    __m128i SSD_V128B = _mm256_castsi256_si128(SSD_V256);
    SSD_V128 = _mm_add_epi64(SSD_V128, _mm_add_epi64(SSD_V128A, SSD_V128B));
    SSD += _mm_extract_epi64(SSD_V128, 0) + _mm_extract_epi64(SSD_V128, 1);
    return SSD;
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_AVX
