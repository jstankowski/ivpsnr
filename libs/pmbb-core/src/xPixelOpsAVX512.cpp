/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xPixelOpsAVX512.h"

#if X_SIMD_CAN_USE_AVX512

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xPixelOpsAVX512::Cvt(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  const __m512i PermCtlV = _mm512_setr_epi64(0, 4, 1, 5, 2, 6, 3, 7);
  if(((uint32)Width & c_RemainderMask64) == 0) //Width%64==0
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 64)
      {
        __m512i SrcVt = _mm512_loadu_si512((__m512i*)&(Src[x]));
        __m512i SrcV  = _mm512_permutexvar_epi64(PermCtlV, SrcVt); //fix AVX per lane mess
        __m512i DstV1 = _mm512_unpacklo_epi8(SrcV, _mm512_setzero_si512());
        __m512i DstV2 = _mm512_unpackhi_epi8(SrcV, _mm512_setzero_si512());
        _mm512_storeu_si512((__m512i*)(&(Dst[x   ])), DstV1);
        _mm512_storeu_si512((__m512i*)(&(Dst[x+32])), DstV2);
      } //x
      Src += SrcStride;
      Dst += DstStride;
    } //y
  }
  else
  {
    const int32  Width64     = (int32)((uint32)Width & c_MultipleMask64);
    const uint64 Remainder64 = (uint32)Width & c_RemainderMask64;
    const uint64 MaskL       = ((uint64)1 << Remainder64) - 1;
    const uint32 MaskS1      = (uint32)(MaskL & 0xFFFFFFFF);
    const uint32 MaskS2      = (uint32)(MaskL>>32);      

    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width64; x += 64)
      {
        __m512i SrcVt = _mm512_loadu_si512((__m512i*)&(Src[x]));
        __m512i SrcV  = _mm512_permutexvar_epi64(PermCtlV, SrcVt); //fix AVX per lane mess
        __m512i DstV1 = _mm512_unpacklo_epi8(SrcV, _mm512_setzero_si512());
        __m512i DstV2 = _mm512_unpackhi_epi8(SrcV, _mm512_setzero_si512());
        _mm512_storeu_si512((__m512i*)(&(Dst[x     ])), DstV1);
        _mm512_storeu_si512((__m512i*)(&(Dst[x + 32])), DstV2);
      } //x
      __m512i SrcVt = _mm512_maskz_loadu_epi8(MaskL, (__m512i*)&Src[Width64]);
      __m512i SrcV  = _mm512_permutexvar_epi64(PermCtlV, SrcVt); //fix AVX per lane mess
      __m512i DstV1 = _mm512_unpacklo_epi8(SrcV, _mm512_setzero_si512());
      __m512i DstV2 = _mm512_unpackhi_epi8(SrcV, _mm512_setzero_si512());
                   _mm512_mask_storeu_epi16((__m512i*)(&(Dst[Width64   ])), MaskS1, DstV1);
      if(MaskS2) { _mm512_mask_storeu_epi16((__m512i*)(&(Dst[Width64+32])), MaskS2, DstV2); }
      Src += SrcStride;
      Dst += DstStride;
    } //y
  } 
}
void xPixelOpsAVX512::Cvt(uint8* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  const __m512i PermCtlV = _mm512_setr_epi64(0, 2, 4, 6, 1, 3, 5, 7);
  if(((uint32)Width & c_RemainderMask64) == 0) //Width%64==0
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 64)
      {
        __m512i SrcV1 = _mm512_loadu_si512((__m512i*)&(Src[x   ]));
        __m512i SrcV2 = _mm512_loadu_si512((__m512i*)&(Src[x+32]));
        __m512i DstVt = _mm512_packus_epi16(SrcV1, SrcV2);
        __m512i DstV  = _mm512_permutexvar_epi64(PermCtlV, DstVt); //fix AVX per lane mess
        _mm512_storeu_si512((__m512i*)(&(Dst[x])), DstV);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }
  else
  {
    const int32 Width64 = (int32)((uint32)Width & c_MultipleMask64);
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width64; x += 64)
      {
        __m512i SrcV1 = _mm512_loadu_si512((__m512i*)&(Src[x   ]));
        __m512i SrcV2 = _mm512_loadu_si512((__m512i*)&(Src[x+32]));
        __m512i DstVt = _mm512_packus_epi16(SrcV1, SrcV2);
        __m512i DstV  = _mm512_permutexvar_epi64(PermCtlV, DstVt); //fix AVX per lane mess
        _mm512_storeu_si512((__m512i*)(&(Dst[x])), DstV);
      } //x
      const uint64 Remainder64 = (uint32)Width & c_RemainderMask64;
      const uint64 Mask        = ((uint64)1 << Remainder64) - 1;
      const uint32 Mask1       = (uint32)(Mask & 0xFFFFFFFF);
      const uint32 Mask2       = (uint32)(Mask>>32);      
      __m512i SrcV1 = _mm512_maskz_loadu_epi16(Mask1, (__m512i*)&(Src[Width64   ]));
      __m512i SrcV2 = _mm512_maskz_loadu_epi16(Mask2, (__m512i*)&(Src[Width64+32]));
      __m512i DstVt = _mm512_packus_epi16(SrcV1, SrcV2);
      __m512i DstV  = _mm512_permutexvar_epi64(PermCtlV, DstVt); //fix AVX per lane mess
      _mm512_mask_storeu_epi8((__m512i*)&(Dst[Width64]), Mask, DstV);
      Src += SrcStride;
      Dst += DstStride;
    } //y
  }
}
void xPixelOpsAVX512::UpsampleHV(uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  const __m512i PermCtlV = _mm512_setr_epi64(0, 4, 1, 5, 2, 6, 3, 7);
  uint16* restrict DstL0 = Dst;
  uint16* restrict DstL1 = Dst + DstStride;  

  if(((uint32)DstWidth & c_RemainderMask64)==0) //Width%64==0
  {
    for(int32 y=0; y<DstHeight; y+=2)
    {
      for(int32 x=0; x<DstWidth; x+=64)
      {
        __m512i SrcVt  = _mm512_loadu_si512((__m512i*)&Src[x>>1]);
        __m512i SrcV   = _mm512_permutexvar_epi64(PermCtlV, SrcVt); //fix AVX512 per lane mess
        __m512i LeftV  = _mm512_unpacklo_epi16(SrcV, SrcV);
        __m512i RightV = _mm512_unpackhi_epi16(SrcV, SrcV);
        _mm512_storeu_si512((__m512i*)&DstL0[x   ], LeftV );
        _mm512_storeu_si512((__m512i*)&DstL0[x+32], RightV);
        _mm512_storeu_si512((__m512i*)&DstL1[x   ], LeftV );
        _mm512_storeu_si512((__m512i*)&DstL1[x+32], RightV);
      } //x
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    } //y
  }
  else
  {
    const int32  Width64     = (int32)((uint32)DstWidth & c_MultipleMask64);
    const uint32 Remainder32 = (uint32)(DstWidth>>1) & c_RemainderMask32;
    const uint32 MaskL       = ((uint32)1 << Remainder32) - 1;
    const uint64 Remainder64 = (uint32)DstWidth & c_RemainderMask64;
    const uint64 MaskS       = ((uint64)1 << Remainder64) - 1;
    const uint32 MaskS1      = (uint32)(MaskS & 0xFFFFFFFF);
    const uint32 MaskS2      = (uint32)(MaskS >>32);

    for(int32 y = 0; y < DstHeight; y += 2)
    {
      for(int32 x = 0; x < Width64; x += 64)
      {
        __m512i SrcVt  = _mm512_loadu_si512((__m512i*)&Src[x>>1]);
        __m512i SrcV   = _mm512_permutexvar_epi64(PermCtlV, SrcVt); //fix AVX per lane mess
        __m512i LeftV  = _mm512_unpacklo_epi16(SrcV, SrcV);
        __m512i RightV = _mm512_unpackhi_epi16(SrcV, SrcV);
        _mm512_storeu_si512((__m512i*)&DstL0[x   ], LeftV );
        _mm512_storeu_si512((__m512i*)&DstL0[x+32], RightV);
        _mm512_storeu_si512((__m512i*)&DstL1[x   ], LeftV );
        _mm512_storeu_si512((__m512i*)&DstL1[x+32], RightV);
      } //x
      __m512i SrcVt  = _mm512_maskz_loadu_epi16(MaskL, (__m512i*)&Src[Width64>>1]);
      __m512i SrcV   = _mm512_permutexvar_epi64(PermCtlV, SrcVt); //fix AVX per lane mess
      __m512i LeftV  = _mm512_unpacklo_epi16(SrcV, SrcV);
      __m512i RightV = _mm512_unpackhi_epi16(SrcV, SrcV);
                   _mm512_mask_storeu_epi16((__m512i*)&DstL0[Width64   ], MaskS1, LeftV );
      if(MaskS2) { _mm512_mask_storeu_epi16((__m512i*)&DstL0[Width64+32], MaskS2, RightV); }
                   _mm512_mask_storeu_epi16((__m512i*)&DstL1[Width64   ], MaskS1, LeftV );
      if(MaskS2) { _mm512_mask_storeu_epi16((__m512i*)&DstL1[Width64+32], MaskS2, RightV); }
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    } //y
  }
}
void xPixelOpsAVX512::DownsampleHV(uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  const __m512i PermCtlV  = _mm512_setr_epi64(0, 2, 4, 6, 1, 3, 5, 7);
  const __m512i One_U16_V = _mm512_set1_epi16((int16)1);
  const __m512i Two_I32_V = _mm512_set1_epi32((int32)2);
  const uint16* SrcL0     = Src;
  const uint16* SrcL1     = Src + SrcStride;

  if(((uint32)DstWidth & (uint32)c_RemainderMask32) == 0) //Width%32==0
  {
    for(int32 y = 0; y < DstHeight; y++)
    {
      for(int32 x = 0; x < DstWidth; x += 32)
      {
        const int32 SrcX = x << 1;
        __m512i TopLeft_U16_V     = _mm512_loadu_si512((__m512i*)&SrcL0[SrcX   ]);
        __m512i TopRight_U16_V    = _mm512_loadu_si512((__m512i*)&SrcL0[SrcX+32]);
        __m512i BottomLeft_U16_V  = _mm512_loadu_si512((__m512i*)&SrcL1[SrcX   ]);
        __m512i BottomRight_U16_V = _mm512_loadu_si512((__m512i*)&SrcL1[SrcX+32]);
        __m512i Left_U16_V        = _mm512_add_epi16(TopLeft_U16_V , BottomLeft_U16_V );
        __m512i Right_U16_V       = _mm512_add_epi16(TopRight_U16_V, BottomRight_U16_V);
        __m512i Left_I32_V        = _mm512_srai_epi32(_mm512_add_epi32(_mm512_madd_epi16(Left_U16_V , One_U16_V), Two_I32_V), 2);
        __m512i Right_I32_V       = _mm512_srai_epi32(_mm512_add_epi32(_mm512_madd_epi16(Right_U16_V, One_U16_V), Two_I32_V), 2);
        __m512i Out_U16_V         = _mm512_packus_epi32(Left_I32_V, Right_I32_V);
        __m512i OutP_U16_V        = _mm512_permutexvar_epi64(PermCtlV, Out_U16_V); //fix AVX512 per lane mess
        _mm512_storeu_si512((__m512i*)&Dst[x], OutP_U16_V);
      } //x
      SrcL0 += (SrcStride << 1);
      SrcL1 += (SrcStride << 1);
      Dst   += DstStride;
    } //y
  }
  else
  {
    const int32  SrcWidth    = (DstWidth << 1);
    const int32  Width32     = (int32)((uint32)DstWidth & c_MultipleMask32);
    const int32  Width64     = (int32)((uint32)SrcWidth & c_MultipleMask64);
    const uint32 Remainder32 = (uint32)DstWidth & c_RemainderMask32;
    const uint32 MaskS       = ((uint32)1 << Remainder32) - 1;
    const uint64 Remainder64 = (uint32)SrcWidth & c_RemainderMask64;
    const uint64 MaskL       = ((uint64)1 << Remainder64) - 1;
    const uint32 MaskL1      = (uint32)(MaskL & 0xFFFFFFFF);
    const uint32 MaskL2      = (uint32)(MaskL >>32);

    for(int32 y = 0; y < DstHeight; y++)
    {
      for(int32 x = 0; x < Width32; x += 32)
      {
        const int32 SrcX = x << 1;
        __m512i TopLeft_U16_V     = _mm512_loadu_si512((__m512i*)&SrcL0[SrcX   ]);
        __m512i TopRight_U16_V    = _mm512_loadu_si512((__m512i*)&SrcL0[SrcX+32]);
        __m512i BottomLeft_U16_V  = _mm512_loadu_si512((__m512i*)&SrcL1[SrcX   ]);
        __m512i BottomRight_U16_V = _mm512_loadu_si512((__m512i*)&SrcL1[SrcX+32]);
        __m512i Left_U16_V        = _mm512_add_epi16(TopLeft_U16_V , BottomLeft_U16_V );
        __m512i Right_U16_V       = _mm512_add_epi16(TopRight_U16_V, BottomRight_U16_V);
        __m512i Left_I32_V        = _mm512_srai_epi32(_mm512_add_epi32(_mm512_madd_epi16(Left_U16_V , One_U16_V), Two_I32_V), 2);
        __m512i Right_I32_V       = _mm512_srai_epi32(_mm512_add_epi32(_mm512_madd_epi16(Right_U16_V, One_U16_V), Two_I32_V), 2);
        __m512i Out_U16_V         = _mm512_packus_epi32(Left_I32_V, Right_I32_V);
        __m512i OutP_U16_V        = _mm512_permutexvar_epi64(PermCtlV, Out_U16_V); //fix AVX512 per lane mess
        _mm512_storeu_si512((__m512i*)&Dst[x], OutP_U16_V);
      } //x
      __m512i TopLeft_U16_V     =          _mm512_maskz_loadu_epi16(MaskL1, (__m512i*)&SrcL0[Width64    ])                         ;
      __m512i TopRight_U16_V    = MaskL2 ? _mm512_maskz_loadu_epi16(MaskL2, (__m512i*)&SrcL0[Width64 +32]) : _mm512_setzero_si512();
      __m512i BottomLeft_U16_V  =          _mm512_maskz_loadu_epi16(MaskL1, (__m512i*)&SrcL1[Width64    ])                         ;
      __m512i BottomRight_U16_V = MaskL2 ? _mm512_maskz_loadu_epi16(MaskL2, (__m512i*)&SrcL1[Width64 +32]) : _mm512_setzero_si512();
      __m512i Left_U16_V        = _mm512_add_epi16(TopLeft_U16_V , BottomLeft_U16_V );
      __m512i Right_U16_V       = _mm512_add_epi16(TopRight_U16_V, BottomRight_U16_V);
      __m512i Left_I32_V        = _mm512_srai_epi32(_mm512_add_epi32(_mm512_madd_epi16(Left_U16_V , One_U16_V), Two_I32_V), 2);
      __m512i Right_I32_V       = _mm512_srai_epi32(_mm512_add_epi32(_mm512_madd_epi16(Right_U16_V, One_U16_V), Two_I32_V), 2);
      __m512i Out_U16_V         = _mm512_packus_epi32(Left_I32_V, Right_I32_V);
      __m512i OutP_U16_V        = _mm512_permutexvar_epi64(PermCtlV, Out_U16_V); //fix AVX512 per lane mess
      _mm512_mask_storeu_epi16((__m512i*)&Dst[Width32], MaskS, OutP_U16_V);
      SrcL0 += (SrcStride << 1);
      SrcL1 += (SrcStride << 1);
      Dst   += DstStride;
    } //y
  }
}
void xPixelOpsAVX512::CvtUpsampleHV(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  const __m512i PermCtlV = _mm512_setr_epi64(0, 4, 1, 5, 2, 6, 3, 7);
  uint16* restrict DstL0 = Dst;
  uint16* restrict DstL1 = Dst + DstStride;  

  if(((uint32)DstWidth & c_RemainderMask128)==0) //Width%128==0
  {
    for(int32 y=0; y<DstHeight; y+=2)
    {
      for(int32 x=0; x<DstWidth; x+=128)
      {
        __m512i SrcVt  = _mm512_loadu_si512((__m512i*)&Src[x>>1]);
        __m512i SrcV   = _mm512_permutexvar_epi64(PermCtlV, SrcVt); //fix AVX per lane mess
        __m512i SrcV1t = _mm512_unpacklo_epi8    (SrcV, _mm512_setzero_si512());
        __m512i SrcV2t = _mm512_unpackhi_epi8    (SrcV, _mm512_setzero_si512());
        __m512i SrcV1  = _mm512_permutexvar_epi64(PermCtlV, SrcV1t); //fix AVX per lane mess
        __m512i SrcV2  = _mm512_permutexvar_epi64(PermCtlV, SrcV2t); //fix AVX per lane mess
        __m512i DstV1  = _mm512_unpacklo_epi16   (SrcV1, SrcV1);
        __m512i DstV2  = _mm512_unpackhi_epi16   (SrcV1, SrcV1);
        __m512i DstV3  = _mm512_unpacklo_epi16   (SrcV2, SrcV2);
        __m512i DstV4  = _mm512_unpackhi_epi16   (SrcV2, SrcV2);
        _mm512_storeu_si512((__m512i*)&DstL0[x     ], DstV1);
        _mm512_storeu_si512((__m512i*)&DstL0[x + 32], DstV2);
        _mm512_storeu_si512((__m512i*)&DstL0[x + 64], DstV3);
        _mm512_storeu_si512((__m512i*)&DstL0[x + 96], DstV4);
        _mm512_storeu_si512((__m512i*)&DstL1[x     ], DstV1);
        _mm512_storeu_si512((__m512i*)&DstL1[x + 32], DstV2);
        _mm512_storeu_si512((__m512i*)&DstL1[x + 64], DstV3);
        _mm512_storeu_si512((__m512i*)&DstL1[x + 96], DstV4);
      } //x
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    } //y
  }
  else
  {
    const int32  Width128    = (int32)((uint32)DstWidth & c_MultipleMask128);
    const uint32 Remainder64 = (uint32)(DstWidth>>1) & c_RemainderMask64;
    const uint64 MaskL       = ((uint64)1 << Remainder64) - 1;

    const uint64 Remainder128 = (uint32)DstWidth & c_RemainderMask128;
    const uint64 MaskS1t      = Remainder128 > 64 ? (uint64)0xFFFFFFFFFFFFFFFF : ((uint64)1 << (Remainder128)) - 1;
    const uint64 MaskS2t      = Remainder128 < 64 ? 0 : ((uint64)1 << (Remainder128 - 64)) - 1;
    const uint32 MaskS1       = (uint32)(MaskS1t & 0xFFFFFFFF);
    const uint32 MaskS2       = (uint32)(MaskS1t >>32);
    const uint32 MaskS3       = (uint32)(MaskS2t & 0xFFFFFFFF);
    const uint32 MaskS4       = (uint32)(MaskS2t >> 32);

    for(int32 y = 0; y < DstHeight; y += 2)
    {
      for(int32 x = 0; x < Width128; x += 128)
      {
        __m512i SrcVt  = _mm512_loadu_si512((__m512i*)&Src[x>>1]);
        __m512i SrcV   = _mm512_permutexvar_epi64(PermCtlV, SrcVt); //fix AVX per lane mess
        __m512i SrcV1t = _mm512_unpacklo_epi8    (SrcV, _mm512_setzero_si512());
        __m512i SrcV2t = _mm512_unpackhi_epi8    (SrcV, _mm512_setzero_si512());
        __m512i SrcV1  = _mm512_permutexvar_epi64(PermCtlV, SrcV1t); //fix AVX per lane mess
        __m512i SrcV2  = _mm512_permutexvar_epi64(PermCtlV, SrcV2t); //fix AVX per lane mess
        __m512i DstV1  = _mm512_unpacklo_epi16   (SrcV1, SrcV1);
        __m512i DstV2  = _mm512_unpackhi_epi16   (SrcV1, SrcV1);
        __m512i DstV3  = _mm512_unpacklo_epi16   (SrcV2, SrcV2);
        __m512i DstV4  = _mm512_unpackhi_epi16   (SrcV2, SrcV2);
        _mm512_storeu_si512((__m512i*)&DstL0[x     ], DstV1);
        _mm512_storeu_si512((__m512i*)&DstL0[x + 32], DstV2);
        _mm512_storeu_si512((__m512i*)&DstL0[x + 64], DstV3);
        _mm512_storeu_si512((__m512i*)&DstL0[x + 96], DstV4);
        _mm512_storeu_si512((__m512i*)&DstL1[x     ], DstV1);
        _mm512_storeu_si512((__m512i*)&DstL1[x + 32], DstV2);
        _mm512_storeu_si512((__m512i*)&DstL1[x + 64], DstV3);
        _mm512_storeu_si512((__m512i*)&DstL1[x + 96], DstV4);
      } //x
      __m512i SrcVt  = _mm512_maskz_loadu_epi8(MaskL, (__m512i*)&Src[Width128>>1]);
      __m512i SrcV   = _mm512_permutexvar_epi64(PermCtlV, SrcVt); //fix AVX per lane mess
      __m512i SrcV1t = _mm512_unpacklo_epi8    (SrcV, _mm512_setzero_si512());
      __m512i SrcV2t = _mm512_unpackhi_epi8    (SrcV, _mm512_setzero_si512());
      __m512i SrcV1  = _mm512_permutexvar_epi64(PermCtlV, SrcV1t); //fix AVX per lane mess
      __m512i SrcV2  = _mm512_permutexvar_epi64(PermCtlV, SrcV2t); //fix AVX per lane mess
      __m512i DstV1  = _mm512_unpacklo_epi16   (SrcV1, SrcV1);
      __m512i DstV2  = _mm512_unpackhi_epi16   (SrcV1, SrcV1);
      __m512i DstV3  = _mm512_unpacklo_epi16   (SrcV2, SrcV2);
      __m512i DstV4  = _mm512_unpackhi_epi16   (SrcV2, SrcV2);
                   _mm512_mask_storeu_epi16((__m512i*)&DstL0[Width128     ], MaskS1, DstV1);
      if(MaskS2) { _mm512_mask_storeu_epi16((__m512i*)&DstL0[Width128 + 32], MaskS2, DstV2); }
      if(MaskS3) { _mm512_mask_storeu_epi16((__m512i*)&DstL0[Width128 + 64], MaskS3, DstV3); }
      if(MaskS4) { _mm512_mask_storeu_epi16((__m512i*)&DstL0[Width128 + 96], MaskS4, DstV4); }
                   _mm512_mask_storeu_epi16((__m512i*)&DstL1[Width128     ], MaskS1, DstV1);
      if(MaskS2) { _mm512_mask_storeu_epi16((__m512i*)&DstL1[Width128 + 32], MaskS2, DstV2); }
      if(MaskS3) { _mm512_mask_storeu_epi16((__m512i*)&DstL1[Width128 + 64], MaskS3, DstV3); }
      if(MaskS4) { _mm512_mask_storeu_epi16((__m512i*)&DstL1[Width128 + 96], MaskS4, DstV4); }
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    } //y
  }
}
bool xPixelOpsAVX512::CheckIfInRange(const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth)
{
  if(BitDepth == 16) { return true; }

  const int32   MaxValue = xBitDepth2MaxValue(BitDepth);
  const __m512i MaxValueV = _mm512_set1_epi16((int16)MaxValue);

  if(((uint32)Width & c_RemainderMask32) == 0) //Width%32==0
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 32)
      {
        __m512i SrcV = _mm512_loadu_si512((__m512i*)&Src[x   ]);
        uint32  Cmp  = _mm512_cmpgt_epi16_mask(SrcV, MaxValueV);
        if(Cmp) { return false; }
      } //x
      Src += SrcStride;
    } //y
  }
  else
  {
    const int32  Width32     = (int32)((uint32)Width & c_MultipleMask32);
    const uint32 Remainder32 = (uint32)(Width) & c_RemainderMask32;
    const uint32 MaskL       = ((uint32)1 << Remainder32) - 1;

    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width32; x += 32)
      {
        __m512i SrcV = _mm512_loadu_si512((__m512i*)&Src[x]);
        uint32  Cmp  = _mm512_cmpgt_epi16_mask(SrcV, MaxValueV);
        if(Cmp) { return false; }
      } //x      
      __m512i SrcV = _mm512_maskz_loadu_epi16(MaskL, (__m512i*)&Src[Width32]);
      uint32  Cmp  = _mm512_cmpgt_epi16_mask(SrcV, MaxValueV);
      if(Cmp) { return false; }
      Src += SrcStride;
    } //y
  }
  return true;
}
void xPixelOpsAVX512::AOS4fromSOA3(uint16* restrict DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, const uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  const __m512i d        = _mm512_set1_epi16(ValueD);
  const __m512i PermCtlV = _mm512_setr_epi32(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15);

  if(((uint32)Width & c_RemainderMask32) == 0) //Width%32==0
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 32)
      {
        //load
        __m512i a = _mm512_loadu_si512((__m512i*)&SrcA[x]); //load A0-A31
        __m512i b = _mm512_loadu_si512((__m512i*)&SrcB[x]); //load B0-B31
        __m512i c = _mm512_loadu_si512((__m512i*)&SrcC[x]); //load C0-C31

        __m512i ap = _mm512_permutexvar_epi32(PermCtlV, a); //fix AVX per lane mess (this time before "unpack") 
        __m512i bp = _mm512_permutexvar_epi32(PermCtlV, b); //fix AVX per lane mess (this time before "unpack")
        __m512i cp = _mm512_permutexvar_epi32(PermCtlV, c); //fix AVX per lane mess (this time before "unpack")

        //transpose
        __m512i ac_0   = _mm512_unpacklo_epi16(ap  , cp  );
        __m512i ac_1   = _mm512_unpackhi_epi16(ap  , cp  );
        __m512i bd_0   = _mm512_unpacklo_epi16(bp  , d   );
        __m512i bd_1   = _mm512_unpackhi_epi16(bp  , d   );
        __m512i abcd_0 = _mm512_unpacklo_epi16(ac_0, bd_0);
        __m512i abcd_1 = _mm512_unpackhi_epi16(ac_0, bd_0);
        __m512i abcd_2 = _mm512_unpacklo_epi16(ac_1, bd_1);
        __m512i abcd_3 = _mm512_unpackhi_epi16(ac_1, bd_1);

        //save
        _mm512_storeu_si512((__m512i*)&DstABCD[(x << 2) +  0], abcd_0);
        _mm512_storeu_si512((__m512i*)&DstABCD[(x << 2) + 32], abcd_1);
        _mm512_storeu_si512((__m512i*)&DstABCD[(x << 2) + 64], abcd_2);
        _mm512_storeu_si512((__m512i*)&DstABCD[(x << 2) + 96], abcd_3);
      } //x
      SrcA    += SrcStride;
      SrcB    += SrcStride;
      SrcC    += SrcStride;
      DstABCD += DstStride;
    } //y
  }
  else
  {
    const int32  Width32     = (int32)((uint32)Width & c_MultipleMask32);
    const uint32 Remainder32 = (uint32)(Width) & c_RemainderMask32;
    const uint32 MaskL       = ((uint32)1 << Remainder32) - 1;

    const uint64 Remainder128 = (uint32)(Width<<2) & c_RemainderMask128;
    const uint64 MaskS1t      = Remainder128 > 64 ? (uint64)0xFFFFFFFFFFFFFFFF : ((uint64)1 << (Remainder128)) - 1;
    const uint64 MaskS2t      = Remainder128 < 64 ? 0 : ((uint64)1 << (Remainder128 - 64)) - 1;
    const uint32 MaskS1       = (uint32)(MaskS1t & 0xFFFFFFFF);
    const uint32 MaskS2       = (uint32)(MaskS1t >>32);
    const uint32 MaskS3       = (uint32)(MaskS2t & 0xFFFFFFFF);
    const uint32 MaskS4       = (uint32)(MaskS2t >> 32);

    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width32; x += 32)
      {
        //load
        __m512i a = _mm512_loadu_si512((__m512i*)&SrcA[x]); //load A0-A31
        __m512i b = _mm512_loadu_si512((__m512i*)&SrcB[x]); //load B0-B31
        __m512i c = _mm512_loadu_si512((__m512i*)&SrcC[x]); //load C0-C31

        __m512i ap = _mm512_permutexvar_epi32(PermCtlV, a); //fix AVX per lane mess (this time before "unpack") 
        __m512i bp = _mm512_permutexvar_epi32(PermCtlV, b); //fix AVX per lane mess (this time before "unpack")
        __m512i cp = _mm512_permutexvar_epi32(PermCtlV, c); //fix AVX per lane mess (this time before "unpack")

        //transpose
        __m512i ac_0   = _mm512_unpacklo_epi16(ap  , cp  );
        __m512i ac_1   = _mm512_unpackhi_epi16(ap  , cp  );
        __m512i bd_0   = _mm512_unpacklo_epi16(bp  , d   );
        __m512i bd_1   = _mm512_unpackhi_epi16(bp  , d   );
        __m512i abcd_0 = _mm512_unpacklo_epi16(ac_0, bd_0);
        __m512i abcd_1 = _mm512_unpackhi_epi16(ac_0, bd_0);
        __m512i abcd_2 = _mm512_unpacklo_epi16(ac_1, bd_1);
        __m512i abcd_3 = _mm512_unpackhi_epi16(ac_1, bd_1);

        //save
        _mm512_storeu_si512((__m512i*)&DstABCD[(x << 2) +  0], abcd_0);
        _mm512_storeu_si512((__m512i*)&DstABCD[(x << 2) + 32], abcd_1);
        _mm512_storeu_si512((__m512i*)&DstABCD[(x << 2) + 64], abcd_2);
        _mm512_storeu_si512((__m512i*)&DstABCD[(x << 2) + 96], abcd_3);
      } //x
      //load
      __m512i a = _mm512_maskz_loadu_epi16(MaskL, (__m512i*)&SrcA[Width32]); //load A0-A31
      __m512i b = _mm512_maskz_loadu_epi16(MaskL, (__m512i*)&SrcB[Width32]); //load B0-B31
      __m512i c = _mm512_maskz_loadu_epi16(MaskL, (__m512i*)&SrcC[Width32]); //load C0-C31

      __m512i ap = _mm512_permutexvar_epi32(PermCtlV, a); //fix AVX per lane mess (this time before "unpack") 
      __m512i bp = _mm512_permutexvar_epi32(PermCtlV, b); //fix AVX per lane mess (this time before "unpack")
      __m512i cp = _mm512_permutexvar_epi32(PermCtlV, c); //fix AVX per lane mess (this time before "unpack")

      //transpose
      __m512i ac_0   = _mm512_unpacklo_epi16(ap  , cp  );
      __m512i ac_1   = _mm512_unpackhi_epi16(ap  , cp  );
      __m512i bd_0   = _mm512_unpacklo_epi16(bp  , d   );
      __m512i bd_1   = _mm512_unpackhi_epi16(bp  , d   );
      __m512i abcd_0 = _mm512_unpacklo_epi16(ac_0, bd_0);
      __m512i abcd_1 = _mm512_unpackhi_epi16(ac_0, bd_0);
      __m512i abcd_2 = _mm512_unpacklo_epi16(ac_1, bd_1);
      __m512i abcd_3 = _mm512_unpackhi_epi16(ac_1, bd_1);

      //save
                   _mm512_mask_storeu_epi16((__m512i*)&DstABCD[(Width32 << 2) +  0], MaskS1, abcd_0);
      if(MaskS2) { _mm512_mask_storeu_epi16((__m512i*)&DstABCD[(Width32 << 2) + 32], MaskS2, abcd_1); }
      if(MaskS3) { _mm512_mask_storeu_epi16((__m512i*)&DstABCD[(Width32 << 2) + 64], MaskS3, abcd_2); }
      if(MaskS4) { _mm512_mask_storeu_epi16((__m512i*)&DstABCD[(Width32 << 2) + 96], MaskS4, abcd_3); }
      SrcA    += SrcStride;
      SrcB    += SrcStride;
      SrcC    += SrcStride;
      DstABCD += DstStride;
    } //y
  }
}
void xPixelOpsAVX512::SOA3fromAOS4(uint16* restrict DstA, uint16* restrict DstB, uint16* restrict DstC, const uint16* SrcABCD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  const __m512i PermCtlV = _mm512_setr_epi32(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15);

  if(((uint32)Width & (uint32)c_RemainderMask32) == 0) //Width%32==0
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 32)
      {
        //load
        __m512i abcd_U16_V0 = _mm512_loadu_si512((__m512i*) & SrcABCD[(x << 2) + 0 ]);
        __m512i abcd_U16_V1 = _mm512_loadu_si512((__m512i*) & SrcABCD[(x << 2) + 32]);
        __m512i abcd_U16_V2 = _mm512_loadu_si512((__m512i*) & SrcABCD[(x << 2) + 64]);
        __m512i abcd_U16_V3 = _mm512_loadu_si512((__m512i*) & SrcABCD[(x << 2) + 96]);

        //transpose
        __m512i ac_U16_V0 = _mm512_unpacklo_epi16(abcd_U16_V0, abcd_U16_V1);
        __m512i ac_U16_V1 = _mm512_unpackhi_epi16(abcd_U16_V0, abcd_U16_V1);
        __m512i bd_U16_V0 = _mm512_unpacklo_epi16(abcd_U16_V2, abcd_U16_V3);
        __m512i bd_U16_V1 = _mm512_unpackhi_epi16(abcd_U16_V2, abcd_U16_V3);

        __m512i a_U16_V0 = _mm512_unpacklo_epi16(ac_U16_V0, ac_U16_V1);
        __m512i b_U16_V0 = _mm512_unpackhi_epi16(ac_U16_V0, ac_U16_V1);
        __m512i c_U16_V0 = _mm512_unpacklo_epi16(bd_U16_V0, bd_U16_V1);
        __m512i d_U16_V0 = _mm512_unpackhi_epi16(bd_U16_V0, bd_U16_V1);

        __m512i a_U16_V = _mm512_unpacklo_epi64(a_U16_V0, c_U16_V0);
        __m512i b_U16_V = _mm512_unpackhi_epi64(a_U16_V0, c_U16_V0);
        __m512i c_U16_V = _mm512_unpacklo_epi64(b_U16_V0, d_U16_V0);

        __m512i a_U16_VP = _mm512_permutexvar_epi32(PermCtlV, a_U16_V); //fix AVX512 per lane mess (this time before "unpack")
        __m512i b_U16_VP = _mm512_permutexvar_epi32(PermCtlV, b_U16_V); //fix AVX512 per lane mess (this time before "unpack")
        __m512i c_U16_VP = _mm512_permutexvar_epi32(PermCtlV, c_U16_V); //fix AVX512 per lane mess (this time before "unpack")

        //save
        _mm512_storeu_si512((__m512i*)&DstA[x], a_U16_VP);
        _mm512_storeu_si512((__m512i*)&DstB[x], b_U16_VP);
        _mm512_storeu_si512((__m512i*)&DstC[x], c_U16_VP);
      } //x
      SrcABCD += SrcStride;
      DstA    += DstStride;
      DstB    += DstStride;
      DstC    += DstStride;
    } //y
  }
  else
  {
    const int32  Width32     = (int32)((uint32)Width & c_MultipleMask32);
    const uint32 Remainder32 = (uint32)(Width) & c_RemainderMask32;
    const uint32 MaskS       = ((uint32)1 << Remainder32) - 1;

    const uint64 Remainder128 = (uint32)(Width<<2) & c_RemainderMask128;
    const uint64 MaskL1t      = Remainder128 > 64 ? (uint64)0xFFFFFFFFFFFFFFFF : ((uint64)1 << (Remainder128)) - 1;
    const uint64 MaskL2t      = Remainder128 < 64 ? 0 : ((uint64)1 << (Remainder128 - 64)) - 1;
    const uint32 MaskL1       = (uint32)(MaskL1t & 0xFFFFFFFF);
    const uint32 MaskL2       = (uint32)(MaskL1t >>32);
    const uint32 MaskL3       = (uint32)(MaskL2t & 0xFFFFFFFF);
    const uint32 MaskL4       = (uint32)(MaskL2t >> 32);

    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width32; x += 32)
      {
        //load
        __m512i abcd_U16_V0 = _mm512_loadu_si512((__m512i*) & SrcABCD[(x << 2) + 0 ]);
        __m512i abcd_U16_V1 = _mm512_loadu_si512((__m512i*) & SrcABCD[(x << 2) + 32]);
        __m512i abcd_U16_V2 = _mm512_loadu_si512((__m512i*) & SrcABCD[(x << 2) + 64]);
        __m512i abcd_U16_V3 = _mm512_loadu_si512((__m512i*) & SrcABCD[(x << 2) + 96]);

        //transpose
        __m512i ac_U16_V0 = _mm512_unpacklo_epi16(abcd_U16_V0, abcd_U16_V1);
        __m512i ac_U16_V1 = _mm512_unpackhi_epi16(abcd_U16_V0, abcd_U16_V1);
        __m512i bd_U16_V0 = _mm512_unpacklo_epi16(abcd_U16_V2, abcd_U16_V3);
        __m512i bd_U16_V1 = _mm512_unpackhi_epi16(abcd_U16_V2, abcd_U16_V3);

        __m512i a_U16_V0 = _mm512_unpacklo_epi16(ac_U16_V0, ac_U16_V1);
        __m512i b_U16_V0 = _mm512_unpackhi_epi16(ac_U16_V0, ac_U16_V1);
        __m512i c_U16_V0 = _mm512_unpacklo_epi16(bd_U16_V0, bd_U16_V1);
        __m512i d_U16_V0 = _mm512_unpackhi_epi16(bd_U16_V0, bd_U16_V1);

        __m512i a_U16_V = _mm512_unpacklo_epi64(a_U16_V0, c_U16_V0);
        __m512i b_U16_V = _mm512_unpackhi_epi64(a_U16_V0, c_U16_V0);
        __m512i c_U16_V = _mm512_unpacklo_epi64(b_U16_V0, d_U16_V0);

        __m512i a_U16_VP = _mm512_permutexvar_epi32(PermCtlV, a_U16_V); //fix AVX512 per lane mess (this time before "unpack")
        __m512i b_U16_VP = _mm512_permutexvar_epi32(PermCtlV, b_U16_V); //fix AVX512 per lane mess (this time before "unpack")
        __m512i c_U16_VP = _mm512_permutexvar_epi32(PermCtlV, c_U16_V); //fix AVX512 per lane mess (this time before "unpack")

        //save
        _mm512_storeu_si512((__m512i*)&DstA[x], a_U16_VP);
        _mm512_storeu_si512((__m512i*)&DstB[x], b_U16_VP);
        _mm512_storeu_si512((__m512i*)&DstC[x], c_U16_VP);
      } //x

      //load
      __m512i abcd_U16_V0 =          _mm512_maskz_loadu_epi16(MaskL1, (__m512i*) & SrcABCD[(Width32 << 2) + 0 ])                         ;
      __m512i abcd_U16_V1 = MaskL2 ? _mm512_maskz_loadu_epi16(MaskL2, (__m512i*) & SrcABCD[(Width32 << 2) + 32]) : _mm512_setzero_si512();
      __m512i abcd_U16_V2 = MaskL3 ? _mm512_maskz_loadu_epi16(MaskL3, (__m512i*) & SrcABCD[(Width32 << 2) + 64]) : _mm512_setzero_si512();
      __m512i abcd_U16_V3 = MaskL4 ? _mm512_maskz_loadu_epi16(MaskL4, (__m512i*) & SrcABCD[(Width32 << 2) + 96]) : _mm512_setzero_si512();

      //transpose
      __m512i ac_U16_V0 = _mm512_unpacklo_epi16(abcd_U16_V0, abcd_U16_V1);
      __m512i ac_U16_V1 = _mm512_unpackhi_epi16(abcd_U16_V0, abcd_U16_V1);
      __m512i bd_U16_V0 = _mm512_unpacklo_epi16(abcd_U16_V2, abcd_U16_V3);
      __m512i bd_U16_V1 = _mm512_unpackhi_epi16(abcd_U16_V2, abcd_U16_V3);

      __m512i a_U16_V0 = _mm512_unpacklo_epi16(ac_U16_V0, ac_U16_V1);
      __m512i b_U16_V0 = _mm512_unpackhi_epi16(ac_U16_V0, ac_U16_V1);
      __m512i c_U16_V0 = _mm512_unpacklo_epi16(bd_U16_V0, bd_U16_V1);
      __m512i d_U16_V0 = _mm512_unpackhi_epi16(bd_U16_V0, bd_U16_V1);

      __m512i a_U16_V = _mm512_unpacklo_epi64(a_U16_V0, c_U16_V0);
      __m512i b_U16_V = _mm512_unpackhi_epi64(a_U16_V0, c_U16_V0);
      __m512i c_U16_V = _mm512_unpacklo_epi64(b_U16_V0, d_U16_V0);

      __m512i a_U16_VP = _mm512_permutexvar_epi32(PermCtlV, a_U16_V); //fix AVX512 per lane mess (this time before "unpack")
      __m512i b_U16_VP = _mm512_permutexvar_epi32(PermCtlV, b_U16_V); //fix AVX512 per lane mess (this time before "unpack")
      __m512i c_U16_VP = _mm512_permutexvar_epi32(PermCtlV, c_U16_V); //fix AVX512 per lane mess (this time before "unpack")

      //save
      _mm512_mask_storeu_epi16((__m512i*)&DstA[Width32], MaskS, a_U16_VP);
      _mm512_mask_storeu_epi16((__m512i*)&DstB[Width32], MaskS, b_U16_VP);
      _mm512_mask_storeu_epi16((__m512i*)&DstC[Width32], MaskS, c_U16_VP);

      SrcABCD += SrcStride;
      DstA    += DstStride;
      DstB    += DstStride;
      DstC    += DstStride;
    } //y
  }
}
int32 xPixelOpsAVX512::CountNonZero(const uint16* Src, int32 SrcStride, int32 Width, int32 Height)
{
  int32 NumNonZero = 0;

  if(((uint32)Width & (uint32)c_RemainderMask32)==0) //Width%32==0
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=32)
      {
        __m512i SrcV  = _mm512_loadu_si512((__m512i*)&Src[x  ]);
        uint32  Cmp   = ~_mm512_cmpeq_epi16_mask(SrcV, _mm512_setzero_si512());
        uint32  NumNZ = _mm_popcnt_u32(Cmp);
        NumNonZero += NumNZ;
      } //x
      Src += SrcStride;
    } //y
  }
  else
  {
    const int32  Width32     = (int32)((uint32)Width & c_MultipleMask32);
    const uint32 Remainder32 = (uint32)(Width) & c_RemainderMask32;
    const uint32 MaskL       = ((uint32)1 << Remainder32) - 1;

    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width32; x += 32)
      {
        __m512i SrcV  = _mm512_loadu_si512((__m512i*)&Src[x]);
        uint32  Cmp   = ~_mm512_cmpeq_epi16_mask(SrcV, _mm512_setzero_si512());
        uint32  NumNZ = _mm_popcnt_u32(Cmp);
        NumNonZero += NumNZ;
      } //x
      __m512i SrcV  = _mm512_maskz_loadu_epi16(MaskL, (__m512i*)&Src[Width32]);
      uint32  Cmp   = ~_mm512_cmpeq_epi16_mask(SrcV, _mm512_setzero_si512());
      uint32  NumNZ = _mm_popcnt_u32(Cmp);
      NumNonZero += NumNZ;
      Src += SrcStride;
    } //y
  }

  return NumNonZero;
}
bool xPixelOpsAVX512::CompareEqual(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask32) == 0) //Width%32==0
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 32)
      {
        __m512i Tst_U16_V = _mm512_loadu_si512((__m512i*)&Tst[x]);
        __m512i Ref_U16_V = _mm512_loadu_si512((__m512i*)&Ref[x]);
        uint32  EqMask    = _mm512_cmpeq_epi16_mask(Tst_U16_V, Ref_U16_V);
        if(EqMask != 0xFFFFFFFF) { return false; }
      } //x
      Tst += TstStride;
      Ref += RefStride;
    } //y
  }
  else
  {
    const int32  Width32     = (int32)((uint32)Width & c_MultipleMask32);
    const uint32 Remainder32 = (uint32)(Width) & c_RemainderMask32;
    const uint32 MaskL       = ((uint32)1 << Remainder32) - 1;

    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width32; x += 32)
      {
        __m512i Tst_U16_V = _mm512_loadu_si512((__m512i*)&Tst[x]);
        __m512i Ref_U16_V = _mm512_loadu_si512((__m512i*)&Ref[x]);
        uint32  EqMask    = _mm512_cmpeq_epi16_mask(Tst_U16_V, Ref_U16_V);
        if(EqMask != 0xFFFFFFFF) { return false; }
      } //x      
      __m512i Tst_U16_V = _mm512_maskz_loadu_epi16(MaskL, (__m512i*)&Tst[Width32]);
      __m512i Ref_U16_V = _mm512_maskz_loadu_epi16(MaskL, (__m512i*)&Ref[Width32]);
      uint32  EqMask    = _mm512_cmpeq_epi16_mask(Tst_U16_V, Ref_U16_V);
      if(EqMask != 0xFFFFFFFF) { return false; }
      Tst += TstStride;
      Ref += RefStride;
    } //y
  }
  return true;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_AVX512
