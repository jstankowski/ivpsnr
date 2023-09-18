/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xDistortionAVX512.h"

#if X_SIMD_CAN_USE_AVX512

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

int32 xDistortionAVX512::CalcSD(const uint16* restrict Org, const uint16* restrict Dist, int32 Area)
{
  const int32 Area32  = (int32)((uint32)Area & c_MultipleMask32);
  __m512i     SD_V512 = _mm512_setzero_si512();

  for (int32 i = 0; i < Area32; i += 32)
  {
    __m512i Org_V512   = _mm512_loadu_si512((__m512i*)&Org [i]);
    __m512i Dist_V512  = _mm512_loadu_si512((__m512i*)&Dist[i]);
    __m512i Diff_V512  = _mm512_sub_epi16     (Org_V512, Dist_V512);
    __m512i Diff_V512A = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(Diff_V512, 1));
    __m512i Diff_V512B = _mm512_cvtepi16_epi32(_mm512_castsi512_si256   (Diff_V512   ));
    __m512i Sum_V512   = _mm512_add_epi32     (Diff_V512A, Diff_V512B);
    SD_V512 = _mm512_add_epi32(SD_V512, Sum_V512);
  } //i

  __m256i SD_V256A = _mm512_extracti64x4_epi64(SD_V512, 1);
  __m256i SD_V256B = _mm512_castsi512_si256   (SD_V512   );
  __m256i SD_V256  = _mm256_add_epi32         (SD_V256A, SD_V256B);
  __m128i SD_V128A = _mm256_extracti128_si256 (SD_V256, 1);
  __m128i SD_V128B = _mm256_castsi256_si128   (SD_V256   );
  __m128i SD_V128  = _mm_add_epi32            (SD_V128A, SD_V128B);
  __m128i Tmp1V    = _mm_hadd_epi32           (SD_V128, SD_V128);
  __m128i Tmp2V    = _mm_hadd_epi32           (Tmp1V, Tmp1V);
  int32   SD       = _mm_extract_epi32(Tmp2V, 0);

  for (int32 i = Area32; i < Area; i++) { SD += (int32)Org[i] - (int32)Dist[i]; }
  return SD;
}
int32 xDistortionAVX512::CalcSD(const uint16* restrict Org, const uint16* restrict Dist, int32 OStride, int32 DStride, int32 Width, int32 Height)
{
  __m512i SD_V512 = _mm512_setzero_si512();
  if(((uint32)Width & c_RemainderMask32) == 0) //Width%32==0 - fast path without tail
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 32)
      {
        __m512i Org_V512   = _mm512_loadu_si512((__m512i*)&Org [x]);
        __m512i Dist_V512  = _mm512_loadu_si512((__m512i*)&Dist[x]);
        __m512i Diff_V512  = _mm512_sub_epi16     (Org_V512, Dist_V512);
        __m512i Diff_V512A = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(Diff_V512, 1));
        __m512i Diff_V512B = _mm512_cvtepi16_epi32(_mm512_castsi512_si256   (Diff_V512   ));
        __m512i Sum_V512   = _mm512_add_epi32     (Diff_V512A, Diff_V512B);
        SD_V512 = _mm512_add_epi32(SD_V512, Sum_V512);
      } //x
      Org += OStride;
      Dist += DStride;
    } //y
  }
  else //any other
  {
    const int32 Width32 = (int32)((uint32)Width & c_MultipleMask32);
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width32; x += 32)
      {
        __m512i Org_V512   = _mm512_loadu_si512((__m512i*)&Org [x]);
        __m512i Dist_V512  = _mm512_loadu_si512((__m512i*)&Dist[x]);
        __m512i Diff_V512  = _mm512_sub_epi16     (Org_V512, Dist_V512);
        __m512i Diff_V512A = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(Diff_V512, 1));
        __m512i Diff_V512B = _mm512_cvtepi16_epi32(_mm512_castsi512_si256   (Diff_V512   ));
        __m512i Sum_V512   = _mm512_add_epi32     (Diff_V512A, Diff_V512B);
        SD_V512 = _mm512_add_epi32(SD_V512, Sum_V512);
      } //x
      const uint32    Remainder32 = (uint32)Width & c_RemainderMask32;
      const __mmask32 Mask        = ((uint32)1 << Remainder32) - 1;
      __m512i Org_V512   = _mm512_maskz_loadu_epi16(Mask, (__m512i*)&Org [Width32]);
      __m512i Dist_V512  = _mm512_maskz_loadu_epi16(Mask, (__m512i*)&Dist[Width32]);
      __m512i Diff_V512  = _mm512_sub_epi16     (Org_V512, Dist_V512);
      __m512i Diff_V512A = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(Diff_V512, 1));
      __m512i Diff_V512B = _mm512_cvtepi16_epi32(_mm512_castsi512_si256   (Diff_V512   ));
      __m512i Sum_V512   = _mm512_add_epi32     (Diff_V512A, Diff_V512B);
      SD_V512 = _mm512_add_epi32(SD_V512, Sum_V512);
      Org += OStride;
      Dist += DStride;
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
  int32   SD       = _mm_extract_epi32(Tmp2V, 0);
  return SD;
}
uint64 xDistortionAVX512::CalcSSD(const uint16* restrict Org, const uint16* restrict Dist, int32 Area)
{
  const int32 Area32   = (int32)((uint32)Area & c_MultipleMask32);
  __m512i     SSD_V512 = _mm512_setzero_si512();

  for(int32 i = 0; i < Area32; i += 32)
  {
    __m512i Org_V512  = _mm512_loadu_si512((__m512i*) & Org [i]);
    __m512i Dist_V512 = _mm512_loadu_si512((__m512i*) & Dist[i]);
    __m512i Diff_V512 = _mm512_sub_epi16     (Org_V512 , Dist_V512);
    __m512i Pow_V512  = _mm512_madd_epi16    (Diff_V512, Diff_V512);
    __m512i Pow_V512A = _mm512_unpacklo_epi32(Pow_V512 , _mm512_setzero_si512());
    __m512i Pow_V512B = _mm512_unpackhi_epi32(Pow_V512 , _mm512_setzero_si512());
    __m512i Sum_V512  = _mm512_add_epi64     (Pow_V512A, Pow_V512B);
    SSD_V512          = _mm512_add_epi64     (SSD_V512, Sum_V512);
  }

  __m256i SSD_V256A = _mm512_extracti64x4_epi64(SSD_V512, 1);
  __m256i SSD_V256B = _mm512_castsi512_si256   (SSD_V512);
  __m256i SSD_V256  = _mm256_add_epi64         (SSD_V256A, SSD_V256B);
  __m128i SSD_V128A = _mm256_extracti128_si256 (SSD_V256, 1);
  __m128i SSD_V128B = _mm256_castsi256_si128   (SSD_V256);
  __m128i SSD_V128  = _mm_add_epi64            (SSD_V128A, SSD_V128B);
  uint64  SSD       = _mm_extract_epi64(SSD_V128, 0) + _mm_extract_epi64(SSD_V128, 1);

  for(int32 i = Area32; i < Area; i++) { SSD += (uint64)xPow2(((int32)Org[i]) - ((int32)Dist[i])); }
  return SSD;
}
uint64 xDistortionAVX512::CalcSSD(const uint16* restrict Org, const uint16* restrict Dist, int32 OStride, int32 DStride, int32 Width, int32 Height)
{
  __m512i SSD_V512 = _mm512_setzero_si512();
  if(((uint32)Width & c_RemainderMask32)==0) //Width%32==0 - fast path without tail
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=32)
      {
        __m512i Org_V512  = _mm512_loadu_si512((__m512i*) & Org [x]);
        __m512i Dist_V512 = _mm512_loadu_si512((__m512i*) & Dist[x]);
        __m512i Diff_V512 = _mm512_sub_epi16     (Org_V512 , Dist_V512);
        __m512i Pow_V512  = _mm512_madd_epi16    (Diff_V512, Diff_V512);
        __m512i Pow_V512A = _mm512_unpacklo_epi32(Pow_V512 , _mm512_setzero_si512());
        __m512i Pow_V512B = _mm512_unpackhi_epi32(Pow_V512 , _mm512_setzero_si512());
        __m512i Sum_V512  = _mm512_add_epi64     (Pow_V512A, Pow_V512B);
        SSD_V512          = _mm512_add_epi64     (SSD_V512, Sum_V512);
      } //x
      Org  += OStride;
      Dist += DStride;
    } //y
  }
  else //any other
  {
    const int32  Width32 = (int32)((uint32)Width & c_MultipleMask32);
    for(int32 y=0; y<Height; y++)
    {
      for (int32 x = 0; x < Width32; x += 32)
      {
        __m512i Org_V512  = _mm512_loadu_si512((__m512i*) & Org [x]);
        __m512i Dist_V512 = _mm512_loadu_si512((__m512i*) & Dist[x]);
        __m512i Diff_V512 = _mm512_sub_epi16     (Org_V512, Dist_V512);
        __m512i Pow_V512  = _mm512_madd_epi16    (Diff_V512, Diff_V512);
        __m512i Pow_V512A = _mm512_unpacklo_epi32(Pow_V512, _mm512_setzero_si512());
        __m512i Pow_V512B = _mm512_unpackhi_epi32(Pow_V512, _mm512_setzero_si512());
        __m512i Sum_V512  = _mm512_add_epi64     (Pow_V512A, Pow_V512B);
        SSD_V512 = _mm512_add_epi64(SSD_V512, Sum_V512);
      } //x
      const uint32    Remainder32 = (uint32)Width & c_RemainderMask32;
      const __mmask32 Mask        = ((uint32)1 << Remainder32) - 1;
      __m512i Org_V512   = _mm512_maskz_loadu_epi16(Mask, (__m512i*) & Org [Width32]);
      __m512i Dist_V512  = _mm512_maskz_loadu_epi16(Mask, (__m512i*) & Dist[Width32]);
      __m512i Diff_V512 = _mm512_sub_epi16     (Org_V512, Dist_V512);
      __m512i Pow_V512  = _mm512_madd_epi16    (Diff_V512, Diff_V512);
      __m512i Pow_V512A = _mm512_unpacklo_epi32(Pow_V512, _mm512_setzero_si512());
      __m512i Pow_V512B = _mm512_unpackhi_epi32(Pow_V512, _mm512_setzero_si512());
      __m512i Sum_V512  = _mm512_add_epi64     (Pow_V512A, Pow_V512B);
      SSD_V512 = _mm512_add_epi64(SSD_V512, Sum_V512);
      Org  += OStride;
      Dist += DStride;
    } //y
  }

  __m256i SSD_V256A = _mm512_extracti64x4_epi64(SSD_V512, 1);
  __m256i SSD_V256B = _mm512_castsi512_si256   (SSD_V512);
  __m256i SSD_V256  = _mm256_add_epi64         (SSD_V256A, SSD_V256B);
  __m128i SSD_V128A = _mm256_extracti128_si256 (SSD_V256, 1);
  __m128i SSD_V128B = _mm256_castsi256_si128   (SSD_V256);
  __m128i SSD_V128  = _mm_add_epi64            (SSD_V128A, SSD_V128B);
  uint64 SSD        = _mm_extract_epi64(SSD_V128, 0) + _mm_extract_epi64(SSD_V128, 1);
  return SSD;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_AVX512
