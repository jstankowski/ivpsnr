/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xDistortionAVX512.h"
#include "xHelpersSIMD.h"

#if X_SIMD_CAN_USE_AVX512

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

int32 xDistortionAVX512::CalcSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{
  const int32 Area32  = (int32)((uint32)Area & c_MultipleMask32);
  __m512i     SD_V512 = _mm512_setzero_si512();

  for (int32 i = 0; i < Area32; i += 32)
  {
    __m512i Tst_V512   = _mm512_loadu_si512((__m512i*)&Tst[i]);
    __m512i Ref_V512   = _mm512_loadu_si512((__m512i*)&Ref[i]);
    __m512i Diff_V512  = _mm512_sub_epi16     (Tst_V512, Ref_V512);
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

  for (int32 i = Area32; i < Area; i++) { SD += (int32)Tst[i] - (int32)Ref[i]; }
  return SD;
}
int32 xDistortionAVX512::CalcSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  __m512i SD_V512 = _mm512_setzero_si512();
  if(((uint32)Width & c_RemainderMask32) == 0) //Width%32==0 - fast path without tail
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
    const int32 Width32 = (int32)((uint32)Width & c_MultipleMask32);
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
      const uint32    Remainder32 = (uint32)Width & c_RemainderMask32;
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
  int32   SD       = _mm_extract_epi32(Tmp2V, 0);
  return SD;
}
uint32 xDistortionAVX512::CalcSAD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{
  const int32 Area32  = (int32)((uint32)Area & c_MultipleMask32);
  __m512i SAD_I32_V512 = _mm512_setzero_si512();

  for (int32 i = 0; i < Area32; i += 32)
  {
    __m512i Tst_U16_V512   = _mm512_loadu_si512((__m512i*)&Tst[i]);
    __m512i Ref_U16_V512   = _mm512_loadu_si512((__m512i*)&Ref[i]);
    __m512i Tst_I32_V512A  = _mm512_unpacklo_epi16(Tst_U16_V512, _mm512_setzero_si512());
    __m512i Tst_I32_V512B  = _mm512_unpackhi_epi16(Tst_U16_V512, _mm512_setzero_si512());
    __m512i Ref_I32_V512A  = _mm512_unpacklo_epi16(Ref_U16_V512, _mm512_setzero_si512());
    __m512i Ref_I32_V512B  = _mm512_unpackhi_epi16(Ref_U16_V512, _mm512_setzero_si512());
    __m512i AbsD_I32_V512A = _mm512_abs_epi32(_mm512_sub_epi32(Tst_I32_V512A, Ref_I32_V512A));
    __m512i AbsD_I32_V512B = _mm512_abs_epi32(_mm512_sub_epi32(Tst_I32_V512B, Ref_I32_V512B));
    __m512i SumA_I32V512   = _mm512_add_epi32(AbsD_I32_V512A, AbsD_I32_V512B);
    SAD_I32_V512 = _mm512_add_epi32(SAD_I32_V512, SumA_I32V512);
  } //i
  uint32 SAD = xHorVecSum_epi32(SAD_I32_V512);

  for (int32 i = Area32; i < Area; i++) { SAD += (uint32)xAbs(((int32)Tst[i]) - ((int32)Ref[i])); }
  return SAD;
}
uint32 xDistortionAVX512::CalcSAD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  __m512i SAD_I32_V512 = _mm512_setzero_si512();
  if(((uint32)Width & c_RemainderMask32) == 0) //Width%32==0 - fast path without tail
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
    const int32 Width32 = (int32)((uint32)Width & c_MultipleMask32);
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
      const uint32    Remainder32 = (uint32)Width & c_RemainderMask32;
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
  
  uint32 SAD = xHorVecSum_epi32(SAD_I32_V512);
  return SAD;
}
uint64 xDistortionAVX512::CalcSSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{
  const int32 Area32   = (int32)((uint32)Area & c_MultipleMask32);
  __m512i     SSD_V512 = _mm512_setzero_si512();

  for(int32 i = 0; i < Area32; i += 32)
  {
    __m512i Tst_V512  = _mm512_loadu_si512((__m512i*) & Tst[i]);
    __m512i Ref_V512  = _mm512_loadu_si512((__m512i*) & Ref[i]);
    __m512i Diff_V512 = _mm512_sub_epi16     (Tst_V512 , Ref_V512);
    __m512i Pow_V512  = _mm512_madd_epi16    (Diff_V512, Diff_V512);
    __m512i Pow_V512A = _mm512_unpacklo_epi32(Pow_V512 , _mm512_setzero_si512());
    __m512i Pow_V512B = _mm512_unpackhi_epi32(Pow_V512 , _mm512_setzero_si512());
    __m512i Sum_V512  = _mm512_add_epi64     (Pow_V512A, Pow_V512B);
    SSD_V512          = _mm512_add_epi64     (SSD_V512, Sum_V512);
  }
  uint64 SSD = xHorVecSum_epi64(SSD_V512);

  for(int32 i = Area32; i < Area; i++) { SSD += (uint64)xPow2(((int32)Tst[i]) - ((int32)Ref[i])); }
  return SSD;
}
uint64 xDistortionAVX512::CalcSSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  __m512i SSD_V512 = _mm512_setzero_si512();
  if(((uint32)Width & c_RemainderMask32)==0) //Width%32==0 - fast path without tail
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=32)
      {
        __m512i Tst_V512  = _mm512_loadu_si512((__m512i*) & Tst[x]);
        __m512i Ref_V512  = _mm512_loadu_si512((__m512i*) & Ref[x]);
        __m512i Diff_V512 = _mm512_sub_epi16     (Tst_V512 , Ref_V512);
        __m512i Pow_V512  = _mm512_madd_epi16    (Diff_V512, Diff_V512);
        __m512i Pow_V512A = _mm512_unpacklo_epi32(Pow_V512 , _mm512_setzero_si512());
        __m512i Pow_V512B = _mm512_unpackhi_epi32(Pow_V512 , _mm512_setzero_si512());
        __m512i Sum_V512  = _mm512_add_epi64     (Pow_V512A, Pow_V512B);
        SSD_V512          = _mm512_add_epi64     (SSD_V512, Sum_V512);
      } //x
      Tst  += TstStride;
      Ref += RefStride;
    } //y
  }
  else //any other
  {
    const int32  Width32 = (int32)((uint32)Width & c_MultipleMask32);
    for(int32 y=0; y<Height; y++)
    {
      for (int32 x = 0; x < Width32; x += 32)
      {
        __m512i Tst_V512  = _mm512_loadu_si512((__m512i*) & Tst[x]);
        __m512i Ref_V512  = _mm512_loadu_si512((__m512i*) & Ref[x]);
        __m512i Diff_V512 = _mm512_sub_epi16     (Tst_V512, Ref_V512);
        __m512i Pow_V512  = _mm512_madd_epi16    (Diff_V512, Diff_V512);
        __m512i Pow_V512A = _mm512_unpacklo_epi32(Pow_V512, _mm512_setzero_si512());
        __m512i Pow_V512B = _mm512_unpackhi_epi32(Pow_V512, _mm512_setzero_si512());
        __m512i Sum_V512  = _mm512_add_epi64     (Pow_V512A, Pow_V512B);
        SSD_V512 = _mm512_add_epi64(SSD_V512, Sum_V512);
      } //x
      const uint32    Remainder32 = (uint32)Width & c_RemainderMask32;
      const __mmask32 Mask        = ((uint32)1 << Remainder32) - 1;
      __m512i Tst_V512  = _mm512_maskz_loadu_epi16(Mask, (__m512i*) & Tst[Width32]);
      __m512i Ref_V512  = _mm512_maskz_loadu_epi16(Mask, (__m512i*) & Ref[Width32]);
      __m512i Diff_V512 = _mm512_sub_epi16     (Tst_V512, Ref_V512);
      __m512i Pow_V512  = _mm512_madd_epi16    (Diff_V512, Diff_V512);
      __m512i Pow_V512A = _mm512_unpacklo_epi32(Pow_V512, _mm512_setzero_si512());
      __m512i Pow_V512B = _mm512_unpackhi_epi32(Pow_V512, _mm512_setzero_si512());
      __m512i Sum_V512  = _mm512_add_epi64     (Pow_V512A, Pow_V512B);
      SSD_V512 = _mm512_add_epi64(SSD_V512, Sum_V512);
      Tst  += TstStride;
      Ref += RefStride;
    } //y
  }
  uint64 SSD = xHorVecSum_epi64(SSD_V512);
  return SSD;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_AVX512
