/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xDistortionSSE.h"

#if X_SIMD_CAN_USE_SSE

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

int32 xDistortionSSE::CalcSD(const uint16* restrict Org, const uint16* restrict Dist, int32 Area)
{  
  const int32 Area8   = (int32)((uint32)Area & c_MultipleMask8);
  __m128i     SD_V128 = _mm_setzero_si128();

  for(int32 i = 0; i < Area8; i += 8)
  {
    __m128i Org_V128   = _mm_loadu_si128((__m128i*) & Org [i]);
    __m128i Dist_V128  = _mm_loadu_si128((__m128i*) & Dist[i]);
    __m128i Diff_V128  = _mm_sub_epi16     (Org_V128 , Dist_V128);
    __m128i Diff_V128A = _mm_cvtepi16_epi32(Diff_V128);
    __m128i Diff_V128B = _mm_cvtepi16_epi32(_mm_srli_si128(Diff_V128, 8));
    __m128i Sum_V128   = _mm_add_epi32     (Diff_V128A, Diff_V128B);
    SD_V128            = _mm_add_epi32     (SD_V128, Sum_V128);
  } //i
  __m128i Tmp1V = _mm_hadd_epi32(SD_V128, SD_V128);
  __m128i Tmp2V = _mm_hadd_epi32(Tmp1V, Tmp1V);
  int32   SD    = _mm_extract_epi32(Tmp2V, 0);

  for(int32 i = Area8; i < Area; i++) { SD += (int32)Org[i] - (int32)Dist[i]; }
  return SD;
}
int32 xDistortionSSE::CalcSD(const uint16* restrict Org, const uint16* restrict Dist, int32 OStride, int32 DStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask8)==0) //Width%8==0 - fast path without tail
  {
    __m128i SD_V128 = _mm_setzero_si128();
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=8)
      {
        __m128i Org_V128   = _mm_loadu_si128((__m128i*) & Org [x]);
        __m128i Dist_V128  = _mm_loadu_si128((__m128i*) & Dist[x]);
        __m128i Diff_V128  = _mm_sub_epi16     (Org_V128 , Dist_V128);
        __m128i Diff_V128A = _mm_cvtepi16_epi32(Diff_V128);
        __m128i Diff_V128B = _mm_cvtepi16_epi32(_mm_srli_si128(Diff_V128, 8));
        __m128i Sum_V128   = _mm_add_epi32     (Diff_V128A, Diff_V128B);
        SD_V128            = _mm_add_epi32     (SD_V128, Sum_V128);
      } //x
      Org  += OStride;
      Dist += DStride;
    } //y
    __m128i Tmp1V = _mm_hadd_epi32(SD_V128, SD_V128);
    __m128i Tmp2V = _mm_hadd_epi32(Tmp1V, Tmp1V);
    int32   SD    = _mm_extract_epi32(Tmp2V, 0);
    return SD;
  }
  else //any other
  {
    const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8);
    const int32 Width4 = (int32)((uint32)Width & c_MultipleMask4);
    int32   SD      = 0;
    __m128i SD_V128 = _mm_setzero_si128();

    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width8; x+=8)
      {
        __m128i Org_V128   = _mm_loadu_si128((__m128i*) & Org [x]);
        __m128i Dist_V128  = _mm_loadu_si128((__m128i*) & Dist[x]);
        __m128i Diff_V128  = _mm_sub_epi16     (Org_V128 , Dist_V128);
        __m128i Diff_V128A = _mm_cvtepi16_epi32(Diff_V128);
        __m128i Diff_V128B = _mm_cvtepi16_epi32(_mm_srli_si128(Diff_V128, 8));
        __m128i Sum_V128   = _mm_add_epi32     (Diff_V128A, Diff_V128B);
        SD_V128            = _mm_add_epi32     (SD_V128, Sum_V128);
      } //x
      for(int32 x=Width8; x<Width4; x+=4)
      {
        __m128i Org_V128   = _mm_loadl_epi64((__m128i*)&Org [x]);
        __m128i Dist_V128  = _mm_loadl_epi64((__m128i*)&Dist[x]);
        __m128i Diff_V128  = _mm_sub_epi16     (Org_V128, Dist_V128);
        __m128i Diff_V128A = _mm_cvtepi16_epi32(Diff_V128);
        SD_V128 = _mm_add_epi32(SD_V128, Diff_V128A);
      } //x
      for(int32 x=Width4; x<Width; x++)
      {
        SD += (int32)Org[x] - (int32)Dist[x];
      } //x
      Org  += OStride;
      Dist += DStride;
    } //y
    __m128i Tmp1V = _mm_hadd_epi32(SD_V128, SD_V128);
    __m128i Tmp2V = _mm_hadd_epi32(Tmp1V, Tmp1V);
    SD += _mm_extract_epi32(Tmp2V, 0);
    return SD;
  }  
}
uint64 xDistortionSSE::CalcSSD(const uint16* restrict Org, const uint16* restrict Dist, int32 Area)
{  
  const int32 Area8    = (int32)((uint32)Area & c_MultipleMask8);
  __m128i     SSD_V128 = _mm_setzero_si128();

  for(int32 i = 0; i < Area8; i += 8)
  {
    __m128i Org_V128  = _mm_loadu_si128((__m128i*) & Org [i]);
    __m128i Dist_V128 = _mm_loadu_si128((__m128i*) & Dist[i]);
    __m128i Diff_V128 = _mm_sub_epi16     (Org_V128 , Dist_V128);
    __m128i Pow_V128  = _mm_madd_epi16    (Diff_V128, Diff_V128);
    __m128i Pow_V128A = _mm_unpacklo_epi32(Pow_V128 , _mm_setzero_si128());
    __m128i Pow_V128B = _mm_unpackhi_epi32(Pow_V128 , _mm_setzero_si128());
    __m128i Sum_V128  = _mm_add_epi64     (Pow_V128A, Pow_V128B);
    SSD_V128          = _mm_add_epi64     (SSD_V128, Sum_V128);
  }
  uint64 SSD = _mm_extract_epi64(SSD_V128, 0) + _mm_extract_epi64(SSD_V128, 1);

  for(int32 i = Area8; i < Area; i++) { SSD += (uint64)xPow2(((int32)Org[i]) - ((int32)Dist[i])); }
  return SSD;
}
uint64 xDistortionSSE::CalcSSD(const uint16* restrict Org, const uint16* restrict Dist, int32 OStride, int32 DStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask8)==0) //Width%8==0 - fast path without tail
  {
    __m128i SSD_V128 = _mm_setzero_si128();
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=8)
      {
        __m128i Org_V128  = _mm_loadu_si128((__m128i*) & Org [x]);
        __m128i Dist_V128 = _mm_loadu_si128((__m128i*) & Dist[x]);
        __m128i Diff_V128 = _mm_sub_epi16     (Org_V128 , Dist_V128);
        __m128i Pow_V128  = _mm_madd_epi16    (Diff_V128, Diff_V128);
        __m128i Pow_V128A = _mm_unpacklo_epi32(Pow_V128 , _mm_setzero_si128());
        __m128i Pow_V128B = _mm_unpackhi_epi32(Pow_V128 , _mm_setzero_si128());
        __m128i Sum_V128  = _mm_add_epi64     (Pow_V128A, Pow_V128B);
        SSD_V128          = _mm_add_epi64     (SSD_V128, Sum_V128);
      } //x
      Org  += OStride;
      Dist += DStride;
    } //y
    uint64 SSD = _mm_extract_epi64(SSD_V128, 0) + _mm_extract_epi64(SSD_V128, 1);
    return SSD;
  }
  else //any other
  {
    const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8);
    const int32 Width4 = (int32)((uint32)Width & c_MultipleMask4);
    uint64  SSD = 0;
    __m128i SSD_V128 = _mm_setzero_si128();

    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width8; x+=8)
      {
        __m128i Org_V128  = _mm_loadu_si128((__m128i*)&Org [x]);
        __m128i Dist_V128 = _mm_loadu_si128((__m128i*)&Dist[x]);
        __m128i Diff_V128 = _mm_sub_epi16     (Org_V128, Dist_V128);
        __m128i Pow_V128  = _mm_madd_epi16    (Diff_V128, Diff_V128);
        __m128i Pow_V128A = _mm_unpacklo_epi32(Pow_V128, _mm_setzero_si128());
        __m128i Pow_V128B = _mm_unpackhi_epi32(Pow_V128, _mm_setzero_si128());
        __m128i Sum_V128  = _mm_add_epi64     (Pow_V128A, Pow_V128B);
        SSD_V128          = _mm_add_epi64     (SSD_V128, Sum_V128);
      }
      for(int32 x=Width8; x<Width4; x+=4)
      {
        __m128i Org_V128  = _mm_loadl_epi64((__m128i*)&Org [x]);
        __m128i Dist_V128 = _mm_loadl_epi64((__m128i*)&Dist[x]);
        __m128i Diff_V128 = _mm_sub_epi16(Org_V128, Dist_V128);
        __m128i Pow_V128  = _mm_madd_epi16(Diff_V128, Diff_V128);
        Pow_V128 = _mm_unpacklo_epi32(Pow_V128, _mm_setzero_si128());
        SSD_V128 = _mm_add_epi64(SSD_V128, Pow_V128);
      }
      for(int32 x=Width4; x<Width; x++)
      {
        SSD += xPow2((int32)Org[x] - (int32)Dist[x]);
      }
      Org  += OStride;
      Dist += DStride;
    } //y
    SSD += _mm_extract_epi64(SSD_V128, 0) + _mm_extract_epi64(SSD_V128, 1);
    return SSD;
  }  
}

int64 xDistortionSSE::CalcWeightedSD(const uint16* restrict Org, const uint16* restrict Dist, const uint16* restrict Mask, int32 Area)
{
  assert(0); //TODO - NOT TESTED
  const int32 Area8 = (int32)((uint32)Area & c_MultipleMask8);
  __m128i SD_V128 = _mm_setzero_si128();

  for(int32 i = 0; i < Area8; i += 8)
  {
    __m128i Org_V128   = _mm_loadu_si128((__m128i*) & Org [i]);
    __m128i Dist_V128  = _mm_loadu_si128((__m128i*) & Dist[i]);
    __m128i Mask_V128  = _mm_loadu_si128((__m128i*) & Mask[i]);
    __m128i Diff_V128  = _mm_sub_epi16     (Org_V128 , Dist_V128);
    __m128i Diff_V128A = _mm_cvtepi16_epi32(Diff_V128);
    __m128i Diff_V128B = _mm_cvtepi16_epi32(_mm_srli_si128(Diff_V128, 8));
    __m128i Mask_V128A = _mm_unpacklo_epi16(Mask_V128, _mm_setzero_si128());
    __m128i Mask_V128B = _mm_unpackhi_epi16(Mask_V128, _mm_setzero_si128());
    __m128i Wght_V128A = _mm_mullo_epi32   (Diff_V128A, Mask_V128A);
    __m128i Wght_V128B = _mm_mullo_epi32   (Diff_V128B, Mask_V128B);
    __m128i Wght_V128  = _mm_add_epi32     (Wght_V128A, Wght_V128B);
    __m128i Sum_V128A  = _mm_unpacklo_epi32(Wght_V128, _mm_setzero_si128());
    __m128i Sum_V128B  = _mm_unpackhi_epi32(Wght_V128, _mm_setzero_si128());
    __m128i Sum_V128   = _mm_add_epi64     (Sum_V128A, Sum_V128B);
    SD_V128 = _mm_add_epi64(SD_V128, Sum_V128);
  } //i
  int64 SD = _mm_extract_epi64(SD_V128, 0) + _mm_extract_epi64(SD_V128, 1);

  for(int32 i = Area8; i < Area; i++) { SD += ((int32)Org[i] - (int32)Dist[i]) * (int32)Mask[i]; }
  return SD;
}
int64 xDistortionSSE::CalcWeightedSD(const uint16* restrict Org, const uint16* restrict Dist, const uint16* restrict Mask, int32 OStride, int32 DStride, int32 MStride, int32 Width, int32 Height)
{
  assert(0); //TODO - NOT TESTED
  if(((uint32)Width & c_RemainderMask8)==0) //Width%8==0 - fast path without tail
  {
    __m128i SD_V128 = _mm_setzero_si128();
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=8)
      {
        __m128i Org_V128   = _mm_loadu_si128((__m128i*) & Org [x]);
        __m128i Dist_V128  = _mm_loadu_si128((__m128i*) & Dist[x]);
        __m128i Mask_V128  = _mm_loadu_si128((__m128i*) & Mask[x]);
        __m128i Diff_V128  = _mm_sub_epi16     (Org_V128 , Dist_V128);
        __m128i Diff_V128A = _mm_cvtepi16_epi32(Diff_V128);
        __m128i Diff_V128B = _mm_cvtepi16_epi32(_mm_srli_si128(Diff_V128, 8));
        __m128i Mask_V128A = _mm_unpacklo_epi16(Mask_V128, _mm_setzero_si128());
        __m128i Mask_V128B = _mm_unpackhi_epi16(Mask_V128, _mm_setzero_si128());
        __m128i Wght_V128A = _mm_mullo_epi32   (Diff_V128A, Mask_V128A);
        __m128i Wght_V128B = _mm_mullo_epi32   (Diff_V128B, Mask_V128B);
        __m128i Wght_V128  = _mm_add_epi32     (Wght_V128A, Wght_V128B);
        __m128i Sum_V128A  = _mm_unpacklo_epi32(Wght_V128, _mm_setzero_si128());
        __m128i Sum_V128B  = _mm_unpackhi_epi32(Wght_V128, _mm_setzero_si128());
        __m128i Sum_V128   = _mm_add_epi64     (Sum_V128A, Sum_V128B);
        SD_V128 = _mm_add_epi64(SD_V128, Sum_V128);
      } //x
      Org  += OStride;
      Dist += DStride;
      Mask += MStride;
    } //y
    int64 SD = _mm_extract_epi64(SD_V128, 0) + _mm_extract_epi64(SD_V128, 1);
    return SD;
  }
  else //any other
  {
    const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8);
    int64   SD      = 0;
    __m128i SD_V128 = _mm_setzero_si128();

    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width8; x+=8)
      {
        __m128i Org_V128   = _mm_loadu_si128((__m128i*) & Org [x]);
        __m128i Dist_V128  = _mm_loadu_si128((__m128i*) & Dist[x]);
        __m128i Mask_V128  = _mm_loadu_si128((__m128i*) & Mask[x]);
        __m128i Diff_V128  = _mm_sub_epi16     (Org_V128 , Dist_V128);
        __m128i Diff_V128A = _mm_cvtepi16_epi32(Diff_V128);
        __m128i Diff_V128B = _mm_cvtepi16_epi32(_mm_srli_si128(Diff_V128, 8));
        __m128i Mask_V128A = _mm_unpacklo_epi16(Mask_V128, _mm_setzero_si128());
        __m128i Mask_V128B = _mm_unpackhi_epi16(Mask_V128, _mm_setzero_si128());
        __m128i Wght_V128A = _mm_mullo_epi32   (Diff_V128A, Mask_V128A);
        __m128i Wght_V128B = _mm_mullo_epi32   (Diff_V128B, Mask_V128B);
        __m128i Wght_V128  = _mm_add_epi32     (Wght_V128A, Wght_V128B);
        __m128i Sum_V128A  = _mm_unpacklo_epi32(Wght_V128, _mm_setzero_si128());
        __m128i Sum_V128B  = _mm_unpackhi_epi32(Wght_V128, _mm_setzero_si128());
        __m128i Sum_V128   = _mm_add_epi64     (Sum_V128A, Sum_V128B);
        SD_V128 = _mm_add_epi64(SD_V128, Sum_V128);
      } //x
      for(int32 x=Width8; x<Width; x++)
      {
        SD += ((int32)Org[x] - (int32)Dist[x]) * (int32)Mask[x];
      } //x
      Org  += OStride;
      Dist += DStride;
      Mask += MStride;
    } //y
    SD += _mm_extract_epi64(SD_V128, 0) + _mm_extract_epi64(SD_V128, 1);
    return SD;
  }
}
uint64 xDistortionSSE::CalcWeightedSSD(const uint16* restrict Org, const uint16* restrict Dist, const uint16* restrict Mask, int32 Area)
{
  assert(0); //TODO - NOT TESTED
  const int32 Area8    = (int32)((uint32)Area & c_MultipleMask8);
  __m128i     SSD_V128 = _mm_setzero_si128();

  for(int32 i = 0; i < Area8; i += 8)
  {
    __m128i Org_V128   = _mm_loadu_si128((__m128i*) & Org [i]);
    __m128i Dist_V128  = _mm_loadu_si128((__m128i*) & Dist[i]);
    __m128i Mask_V128  = _mm_loadu_si128((__m128i*) & Mask[i]);
    __m128i Diff_V128  = _mm_sub_epi16     (Org_V128 , Dist_V128);
    __m128i Diff_V128A = _mm_cvtepi16_epi32(Diff_V128);
    __m128i Diff_V128B = _mm_cvtepi16_epi32(_mm_srli_si128(Diff_V128, 8));
    __m128i Pow_V128A  = _mm_mullo_epi32   (Diff_V128A, Diff_V128A);
    __m128i Pow_V128B  = _mm_mullo_epi32   (Diff_V128B, Diff_V128B);
    __m128i Mask_V128A = _mm_unpacklo_epi16(Mask_V128, _mm_setzero_si128());
    __m128i Mask_V128B = _mm_unpackhi_epi16(Mask_V128, _mm_setzero_si128());
    __m128i Wght_V128A = _mm_mullo_epi32   (Pow_V128A, Mask_V128A);
    __m128i Wght_V128B = _mm_mullo_epi32   (Pow_V128B, Mask_V128B);
    __m128i Wght_V128  = _mm_add_epi32     (Wght_V128A, Wght_V128B);
    __m128i Sum_V128A  = _mm_unpacklo_epi32(Wght_V128, _mm_setzero_si128());
    __m128i Sum_V128B  = _mm_unpackhi_epi32(Wght_V128, _mm_setzero_si128());
    __m128i Sum_V128   = _mm_add_epi64     (Sum_V128A, Sum_V128B);
    SSD_V128 = _mm_add_epi64(SSD_V128, Sum_V128);
  }
  uint64 SSD = _mm_extract_epi64(SSD_V128, 0) + _mm_extract_epi64(SSD_V128, 1);

  for(int32 i = Area8; i < Area; i++) { SSD += ((uint64)xPow2(((int32)Org[i]) - ((int32)Dist[i]))) * (uint64)Mask[i]; }
  return SSD;
}
uint64 xDistortionSSE::CalcWeightedSSD(const uint16* restrict Org, const uint16* restrict Dist, const uint16* restrict Mask, int32 OStride, int32 DStride, int32 MStride, int32 Width, int32 Height)
{
  assert(0); //TODO - NOT TESTED
  if(((uint32)Width & c_RemainderMask8)==0) //Width%8==0 - fast path without tail
  {
    __m128i SSD_V128 = _mm_setzero_si128();
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=8)
      {
        __m128i Org_V128   = _mm_loadu_si128((__m128i*) & Org [x]);
        __m128i Dist_V128  = _mm_loadu_si128((__m128i*) & Dist[x]);
        __m128i Mask_V128  = _mm_loadu_si128((__m128i*) & Mask[x]);
        __m128i Diff_V128  = _mm_sub_epi16     (Org_V128 , Dist_V128);
        __m128i Diff_V128A = _mm_cvtepi16_epi32(Diff_V128);
        __m128i Diff_V128B = _mm_cvtepi16_epi32(_mm_srli_si128(Diff_V128, 8));
        __m128i Pow_V128A  = _mm_mullo_epi32   (Diff_V128A, Diff_V128A);
        __m128i Pow_V128B  = _mm_mullo_epi32   (Diff_V128B, Diff_V128B);
        __m128i Mask_V128A = _mm_unpacklo_epi16(Mask_V128, _mm_setzero_si128());
        __m128i Mask_V128B = _mm_unpackhi_epi16(Mask_V128, _mm_setzero_si128());
        __m128i Wght_V128A = _mm_mullo_epi32   (Pow_V128A, Mask_V128A);
        __m128i Wght_V128B = _mm_mullo_epi32   (Pow_V128B, Mask_V128B);
        __m128i Wght_V128  = _mm_add_epi32     (Wght_V128A, Wght_V128B);
        __m128i Sum_V128A  = _mm_unpacklo_epi32(Wght_V128, _mm_setzero_si128());
        __m128i Sum_V128B  = _mm_unpackhi_epi32(Wght_V128, _mm_setzero_si128());
        __m128i Sum_V128   = _mm_add_epi64     (Sum_V128A, Sum_V128B);
        SSD_V128 = _mm_add_epi64(SSD_V128, Sum_V128);
      } //x
      Org  += OStride;
      Dist += DStride;
      Mask += MStride;
    } //y
    uint64 SSD = _mm_extract_epi64(SSD_V128, 0) + _mm_extract_epi64(SSD_V128, 1);
    return SSD;
  }
  else //any other
  {
    const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8);
    uint64  SSD = 0;
    __m128i SSD_V128 = _mm_setzero_si128();

    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width8; x+=8)
      {
        __m128i Org_V128   = _mm_loadu_si128((__m128i*) & Org [x]);
        __m128i Dist_V128  = _mm_loadu_si128((__m128i*) & Dist[x]);
        __m128i Mask_V128  = _mm_loadu_si128((__m128i*) & Mask[x]);
        __m128i Diff_V128  = _mm_sub_epi16     (Org_V128 , Dist_V128);
        __m128i Diff_V128A = _mm_cvtepi16_epi32(Diff_V128);
        __m128i Diff_V128B = _mm_cvtepi16_epi32(_mm_srli_si128(Diff_V128, 8));
        __m128i Pow_V128A  = _mm_mullo_epi32   (Diff_V128A, Diff_V128A);
        __m128i Pow_V128B  = _mm_mullo_epi32   (Diff_V128B, Diff_V128B);
        __m128i Mask_V128A = _mm_unpacklo_epi16(Mask_V128, _mm_setzero_si128());
        __m128i Mask_V128B = _mm_unpackhi_epi16(Mask_V128, _mm_setzero_si128());
        __m128i Wght_V128A = _mm_mullo_epi32   (Pow_V128A, Mask_V128A);
        __m128i Wght_V128B = _mm_mullo_epi32   (Pow_V128B, Mask_V128B);
        __m128i Wght_V128  = _mm_add_epi32     (Wght_V128A, Wght_V128B);
        __m128i Sum_V128A  = _mm_unpacklo_epi32(Wght_V128, _mm_setzero_si128());
        __m128i Sum_V128B  = _mm_unpackhi_epi32(Wght_V128, _mm_setzero_si128());
        __m128i Sum_V128   = _mm_add_epi64     (Sum_V128A, Sum_V128B);
        SSD_V128 = _mm_add_epi64(SSD_V128, Sum_V128);
      }
      for(int32 x= Width8; x<Width; x++)
      {
        SSD += ((uint64)xPow2(((int32)Org[x]) - ((int32)Dist[x]))) * (uint64)Mask[x];
      }
      Org  += OStride;
      Dist += DStride;
      Mask += MStride;
    } //y
    SSD += _mm_extract_epi64(SSD_V128, 0) + _mm_extract_epi64(SSD_V128, 1);
    return SSD;
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_SSE
