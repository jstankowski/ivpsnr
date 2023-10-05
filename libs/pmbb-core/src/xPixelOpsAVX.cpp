/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xPixelOpsAVX.h"

#if X_SIMD_CAN_USE_AVX

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xPixelOpsAVX::Cvt(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask32)==0) //Width%32==0
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=32)
      {
        __m256i SrcVt = _mm256_loadu_si256((__m256i*)&(Src[x]));
        __m256i SrcV  = _mm256_permute4x64_epi64(SrcVt, 0xD8); //fix AVX per lane mess
        __m256i DstV1 = _mm256_unpacklo_epi8(SrcV, _mm256_setzero_si256());
        __m256i DstV2 = _mm256_unpackhi_epi8(SrcV, _mm256_setzero_si256());
        _mm256_storeu_si256 ((__m256i*)(&(Dst[x   ])), DstV1);
        _mm256_storeu_si256 ((__m256i*)(&(Dst[x+16])), DstV2);
      }
      Src += SrcStride;
      Dst += DstStride;    
    }
  }
  else
  {
    const int32 Width32 = (int32)((uint32)Width & c_MultipleMask32);
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16);
    const int32 Width8  = (int32)((uint32)Width & c_MultipleMask8 );
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width32; x+=32)
      {
        __m256i SrcVt = _mm256_loadu_si256((__m256i*)&(Src[x]));
        __m256i SrcV  = _mm256_permute4x64_epi64(SrcVt, 0xD8); //fix AVX per lane mess
        __m256i DstV1 = _mm256_unpacklo_epi8(SrcV, _mm256_setzero_si256());
        __m256i DstV2 = _mm256_unpackhi_epi8(SrcV, _mm256_setzero_si256());
        _mm256_storeu_si256 ((__m256i*)(&(Dst[x   ])), DstV1);
        _mm256_storeu_si256 ((__m256i*)(&(Dst[x+16])), DstV2);
      }
      for(int32 x=Width32; x<Width16; x+=16)
      {
        __m128i SrcV  = _mm_loadu_si128((__m128i*)&(Src[x]));
        __m128i DstV1 = _mm_unpacklo_epi8(SrcV, _mm_setzero_si128());
        __m128i DstV2 = _mm_unpackhi_epi8(SrcV, _mm_setzero_si128());
        _mm_storeu_si128 ((__m128i*)(&(Dst[x  ])), DstV1);
        _mm_storeu_si128 ((__m128i*)(&(Dst[x+8])), DstV2);
      }
      for(int32 x=Width16; x<Width8; x+=8)
      {
        __m128i SrcVh = _mm_loadl_epi64((__m128i*)&(Src[x]));
        __m128i DstV1 = _mm_unpacklo_epi8(SrcVh, _mm_setzero_si128());
        _mm_storeu_si128 ((__m128i*)(&(Dst[x  ])), DstV1);
      }
      for(int32 x=Width8 ; x<Width; x++)
      {
        Dst[x] = (uint16)(Src[x]);
      }
      Src += SrcStride;
      Dst += DstStride;    
    }
  }
}
void xPixelOpsAVX::Cvt(uint8* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask32)==0) //Width%32==0
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=32)
      {
        __m256i SrcV1 = _mm256_loadu_si256((__m256i*)&(Src[x   ]));
        __m256i SrcV2 = _mm256_loadu_si256((__m256i*)&(Src[x+16]));
        __m256i DstVt = _mm256_packus_epi16(SrcV1, SrcV2);
        __m256i DstV  = _mm256_permute4x64_epi64(DstVt, 0xD8); //fix AVX per lane mess
        _mm256_storeu_si256 ((__m256i*)(&(Dst[x])), DstV);
      }
      Src += SrcStride;
      Dst += DstStride;    
    }
  }
  else
  {
    const int32 Width32 = (int32)((uint32)Width & c_MultipleMask32);
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16);
    const int32 Width8  = (int32)((uint32)Width & c_MultipleMask8 );
    for(int32 y=0; y<Height; y++)
    {
      for (int32 x=0; x<Width32; x+=32)
      {
        __m256i SrcV1 = _mm256_loadu_si256((__m256i*)&(Src[x   ]));
        __m256i SrcV2 = _mm256_loadu_si256((__m256i*)&(Src[x+16]));
        __m256i DstVt = _mm256_packus_epi16(SrcV1, SrcV2);
        __m256i DstV  = _mm256_permute4x64_epi64(DstVt, 0xD8); //fix AVX per lane mess
        _mm256_storeu_si256 ((__m256i*)(&(Dst[x])), DstV);
      }
      for(int32 x=Width32; x<Width16; x+=16)
      {
        __m128i SrcV1 = _mm_loadu_si128((__m128i*)&(Src[x  ]));
        __m128i SrcV2 = _mm_loadu_si128((__m128i*)&(Src[x+8]));
        __m128i DstV  = _mm_packus_epi16(SrcV1, SrcV2);
        _mm_storeu_si128 ((__m128i*)(&(Dst[x])), DstV);
      }
      for(int32 x=Width16; x<Width8; x+=8)
      {
        __m128i SrcV1 = _mm_loadu_si128((__m128i*)&(Src[x  ]));
        __m128i DstV  = _mm_packus_epi16(SrcV1, SrcV1);
        _mm_storel_epi64 ((__m128i*)(&(Dst[x])), DstV);
      }
      for(int32 x = Width8; x < Width; x++)
      {
        Dst[x] = (uint8)xClipU8<uint16>(Src[x]);
      }
      Src += SrcStride;
      Dst += DstStride;    
    }
  }
}
void xPixelOpsAVX::UpsampleHV(uint16* restrict Dst, const uint16* restrict Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  uint16* restrict DstL0 = Dst;
  uint16* restrict DstL1 = Dst + DstStride;  

  if(((uint32)DstWidth & c_RemainderMask32)==0) //Width%32==0
  {
    for(int32 y=0; y<DstHeight; y+=2)
    {
      for(int32 x=0; x<DstWidth; x+=32)
      {
        __m256i SrcVt  = _mm256_loadu_si256((__m256i*)&Src[x>>1]);
        __m256i SrcV   = _mm256_permute4x64_epi64(SrcVt, 0xD8); //fix AVX per lane mess
        __m256i LeftV  = _mm256_unpacklo_epi16(SrcV, SrcV);
        __m256i RightV = _mm256_unpackhi_epi16(SrcV, SrcV);
        _mm256_storeu_si256((__m256i*)&DstL0[x   ], LeftV );
        _mm256_storeu_si256((__m256i*)&DstL0[x+16], RightV);
        _mm256_storeu_si256((__m256i*)&DstL1[x   ], LeftV );
        _mm256_storeu_si256((__m256i*)&DstL1[x+16], RightV);
      }
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    }
  }
  else
  {
    const int32 Width32 = (int32)((uint32)DstWidth & c_MultipleMask32);
    const int32 Width16 = (int32)((uint32)DstWidth & c_MultipleMask16);
    const int32 Width8  = (int32)((uint32)DstWidth & c_MultipleMask8 );

    for(int32 y=0; y<DstHeight; y+=2)
    {
      for (int32 x=0; x<Width32; x+=32)
      {
        __m256i SrcVt  = _mm256_loadu_si256((__m256i*)&Src[x>>1]);
        __m256i SrcV   = _mm256_permute4x64_epi64(SrcVt, 0xD8); //fix AVX per lane mess
        __m256i LeftV  = _mm256_unpacklo_epi16(SrcV, SrcV);
        __m256i RightV = _mm256_unpackhi_epi16(SrcV, SrcV);
        _mm256_storeu_si256((__m256i*)&DstL0[x   ], LeftV );
        _mm256_storeu_si256((__m256i*)&DstL0[x+16], RightV);
        _mm256_storeu_si256((__m256i*)&DstL1[x   ], LeftV );
        _mm256_storeu_si256((__m256i*)&DstL1[x+16], RightV);
      }
      for(int32 x=Width32; x<Width16; x+=16)
      {
        __m128i SrcV   = _mm_loadu_si128((__m128i*)&Src[x>>1]);
        __m128i LeftV  = _mm_unpacklo_epi16(SrcV, SrcV);
        __m128i RightV = _mm_unpackhi_epi16(SrcV, SrcV);
        _mm_storeu_si128((__m128i*)&DstL0[x  ], LeftV );
        _mm_storeu_si128((__m128i*)&DstL0[x+8], RightV);
        _mm_storeu_si128((__m128i*)&DstL1[x  ], LeftV );
        _mm_storeu_si128((__m128i*)&DstL1[x+8], RightV);
      }
      for(int32 x=Width16; x<Width8; x+=8)
      {
        __m128i SrcV  = _mm_loadl_epi64((__m128i*)&Src[x>>1]);
        __m128i LeftV = _mm_unpacklo_epi16(SrcV, SrcV);
        _mm_storeu_si128((__m128i*)&DstL0[x  ], LeftV );
        _mm_storeu_si128((__m128i*)&DstL1[x  ], LeftV );
      }
      for(int32 x=Width8; x<DstWidth; x+=2)
      {
        const uint16 S = Src[x>>1];
        DstL0[x  ] = S;
        DstL0[x+1] = S;
        DstL1[x  ] = S;
        DstL1[x+1] = S;
      }
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    }
  }
}
void xPixelOpsAVX::DownsampleHV(uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  __m256i Two_I32_V = _mm256_set1_epi32((int32)2);

  const uint16* SrcL0 = Src;
  const uint16* SrcL1 = Src + SrcStride;

  if(((uint32)DstWidth & (uint32)c_RemainderMask16)==0) //Width%16==0
  {
    for(int32 y=0; y<DstHeight; y++)
    {
      for(int32 x=0; x<DstWidth; x+=16)
      {
        const int32 SrcX = x<<1;
        __m256i TopLeft_U16_V     = _mm256_loadu_si256((__m256i*)&SrcL0[SrcX   ]);
        __m256i TopRight_U16_V    = _mm256_loadu_si256((__m256i*)&SrcL0[SrcX+16]);
        __m256i BottomLeft_U16_V  = _mm256_loadu_si256((__m256i*)&SrcL1[SrcX   ]);
        __m256i BottomRight_U16_V = _mm256_loadu_si256((__m256i*)&SrcL1[SrcX+16]);
        __m256i Left_U16_V        = _mm256_add_epi16(TopLeft_U16_V , BottomLeft_U16_V );
        __m256i Right_U16_V       = _mm256_add_epi16(TopRight_U16_V, BottomRight_U16_V);
        __m256i Left_I32_V        = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(Left_U16_V , _mm256_set1_epi16(1)), Two_I32_V), 2);
        __m256i Right_I32_V       = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(Right_U16_V, _mm256_set1_epi16(1)), Two_I32_V), 2);
        __m256i Out_U16_V         = _mm256_permute4x64_epi64(_mm256_packus_epi32(Left_I32_V, Right_I32_V), 0xD8);
        _mm256_storeu_si256((__m256i*)&Dst[x], Out_U16_V);
      }
      SrcL0 += (SrcStride << 1);
      SrcL1 += (SrcStride << 1);
      Dst   += DstStride;
    }
  } 
  else
  {
    const int32 Width16 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask16);
    const int32 Width8  = (int32)((uint32)DstWidth & (uint32)c_MultipleMask8 );
    const int32 Width4  = (int32)((uint32)DstWidth & (uint32)c_MultipleMask4 );

    for(int32 y = 0; y < DstHeight; y++)
    {
      for(int32 x = 0; x < Width16; x += 16)
      {
        const int32 SrcX = x<<1;
        __m256i TopLeft_U16_V     = _mm256_loadu_si256((__m256i*)&SrcL0[SrcX   ]);
        __m256i TopRight_U16_V    = _mm256_loadu_si256((__m256i*)&SrcL0[SrcX+16]);
        __m256i BottomLeft_U16_V  = _mm256_loadu_si256((__m256i*)&SrcL1[SrcX   ]);
        __m256i BottomRight_U16_V = _mm256_loadu_si256((__m256i*)&SrcL1[SrcX+16]);
        __m256i Left_U16_V        = _mm256_add_epi16(TopLeft_U16_V , BottomLeft_U16_V );
        __m256i Right_U16_V       = _mm256_add_epi16(TopRight_U16_V, BottomRight_U16_V);
        __m256i Left_I32_V        = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(Left_U16_V , _mm256_set1_epi16(1)), Two_I32_V), 2);
        __m256i Right_I32_V       = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(Right_U16_V, _mm256_set1_epi16(1)), Two_I32_V), 2);
        __m256i Out_U16_V         = _mm256_permute4x64_epi64(_mm256_packus_epi32(Left_I32_V, Right_I32_V), 0xD8);
        _mm256_storeu_si256((__m256i*)&Dst[x], Out_U16_V);
      }
      for(int32 x = Width16; x < Width8; x += 8)
      {
        const int32 SrcX = x<<1;
        __m128i TopLeft_U16_V     = _mm_loadu_si128((__m128i*)&SrcL0[SrcX  ]);
        __m128i TopRight_U16_V    = _mm_loadu_si128((__m128i*)&SrcL0[SrcX+8]);
        __m128i BottomLeft_U16_V  = _mm_loadu_si128((__m128i*)&SrcL1[SrcX  ]);
        __m128i BottomRight_U16_V = _mm_loadu_si128((__m128i*)&SrcL1[SrcX+8]);
        __m128i Left_U16_V        = _mm_add_epi16(TopLeft_U16_V , BottomLeft_U16_V );
        __m128i Right_U16_V       = _mm_add_epi16(TopRight_U16_V, BottomRight_U16_V);
        __m128i Left_I32_V        = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(Left_U16_V , _mm_set1_epi16(1)), _mm256_castsi256_si128(Two_I32_V)), 2);
        __m128i Right_I32_V       = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(Right_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(Two_I32_V)), 2);
        __m128i Out_U16_V         = _mm_packus_epi32(Left_I32_V, Right_I32_V);
        _mm_storeu_si128 ((__m128i*)&Dst[x], Out_U16_V);
      }
      for(int32 x = Width8; x < Width4; x += 4)
      {
        const int32 SrcX = x<<1;
        __m128i TopLeft_U16_V     = _mm_loadu_si128((__m128i*)&SrcL0[SrcX  ]);
        __m128i BottomLeft_U16_V  = _mm_loadu_si128((__m128i*)&SrcL1[SrcX  ]);
        __m128i Left_U16_V        = _mm_add_epi16(TopLeft_U16_V, BottomLeft_U16_V);
        __m128i Left_I32_V        = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(Left_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(Two_I32_V)), 2);
        __m128i Out_U16_V         = _mm_packus_epi32(Left_I32_V, _mm_setzero_si128());
        _mm_storel_epi64 ((__m128i*)&Dst[x], Out_U16_V);

      }
      for(int32 x = Width4; x < DstWidth; x++)
      {
        const int32 SrcX = x << 1;
        int16 D = ((int32)SrcL0[SrcX] + (int32)SrcL0[SrcX + 1] + (int32)SrcL1[SrcX] + (int32)SrcL1[SrcX + 1] + 2) >> 2;
        Dst[x] = D;
      }
      SrcL0 += (SrcStride << 1);
      SrcL1 += (SrcStride << 1);
      Dst   += DstStride;
    }
  }
}
void xPixelOpsAVX::CvtUpsampleHV(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  //TODO - reduce number of permutations
  uint16 *restrict DstL0 = Dst;
  uint16 *restrict DstL1 = Dst + DstStride;

  if(((uint32)DstWidth & c_RemainderMask64)==0) //Width%64==0
  {
    for(int32 y=0; y<DstHeight; y+=2)
    {
      for(int32 x=0; x<DstWidth; x+=64)
      {
        __m256i SrcVt  = _mm256_loadu_si256      ((__m256i*)&Src[x>>1]);
        __m256i SrcV   = _mm256_permute4x64_epi64(SrcVt, 0xD8); //fix AVX per lane mess
        __m256i SrcV1t = _mm256_unpacklo_epi8    (SrcV, _mm256_setzero_si256());
        __m256i SrcV2t = _mm256_unpackhi_epi8    (SrcV, _mm256_setzero_si256());
        __m256i SrcV1  = _mm256_permute4x64_epi64(SrcV1t, 0xD8); //fix AVX per lane mess
        __m256i SrcV2  = _mm256_permute4x64_epi64(SrcV2t, 0xD8); //fix AVX per lane mess
        __m256i DstV1  = _mm256_unpacklo_epi16   (SrcV1, SrcV1);
        __m256i DstV2  = _mm256_unpackhi_epi16   (SrcV1, SrcV1);
        __m256i DstV3  = _mm256_unpacklo_epi16   (SrcV2, SrcV2);
        __m256i DstV4  = _mm256_unpackhi_epi16   (SrcV2, SrcV2);
        _mm256_storeu_si256((__m256i*) & DstL0[x     ], DstV1);
        _mm256_storeu_si256((__m256i*) & DstL0[x + 16], DstV2);
        _mm256_storeu_si256((__m256i*) & DstL0[x + 32], DstV3);
        _mm256_storeu_si256((__m256i*) & DstL0[x + 48], DstV4);
        _mm256_storeu_si256((__m256i*) & DstL1[x     ], DstV1);
        _mm256_storeu_si256((__m256i*) & DstL1[x + 16], DstV2);
        _mm256_storeu_si256((__m256i*) & DstL1[x + 32], DstV3);
        _mm256_storeu_si256((__m256i*) & DstL1[x + 48], DstV4);
      }
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    }
  }
  else
  {
    const int32 Width64 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask64);
    const int32 Width32 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask32);
    const int32 Width16 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask16);

    for(int32 y=0; y<DstHeight; y+=2)
    {
      for(int32 x=0; x<Width64; x+=64)
      {
        __m256i SrcVt  = _mm256_loadu_si256      ((__m256i*)&Src[x>>1]);
        __m256i SrcV   = _mm256_permute4x64_epi64(SrcVt, 0xD8); //fix AVX per lane mess
        __m256i SrcV1t = _mm256_unpacklo_epi8    (SrcV, _mm256_setzero_si256());
        __m256i SrcV2t = _mm256_unpackhi_epi8    (SrcV, _mm256_setzero_si256());
        __m256i SrcV1  = _mm256_permute4x64_epi64(SrcV1t, 0xD8); //fix AVX per lane mess
        __m256i SrcV2  = _mm256_permute4x64_epi64(SrcV2t, 0xD8); //fix AVX per lane mess
        __m256i DstV1  = _mm256_unpacklo_epi16   (SrcV1, SrcV1);
        __m256i DstV2  = _mm256_unpackhi_epi16   (SrcV1, SrcV1);
        __m256i DstV3  = _mm256_unpacklo_epi16   (SrcV2, SrcV2);
        __m256i DstV4  = _mm256_unpackhi_epi16   (SrcV2, SrcV2);
        _mm256_storeu_si256((__m256i*) & DstL0[x     ], DstV1);
        _mm256_storeu_si256((__m256i*) & DstL0[x + 16], DstV2);
        _mm256_storeu_si256((__m256i*) & DstL0[x + 32], DstV3);
        _mm256_storeu_si256((__m256i*) & DstL0[x + 48], DstV4);
        _mm256_storeu_si256((__m256i*) & DstL1[x     ], DstV1);
        _mm256_storeu_si256((__m256i*) & DstL1[x + 16], DstV2);
        _mm256_storeu_si256((__m256i*) & DstL1[x + 32], DstV3);
        _mm256_storeu_si256((__m256i*) & DstL1[x + 48], DstV4);
      }
      for(int32 x=Width64; x<Width32; x+=32)
      {
        __m128i SrcV  = _mm_loadu_si128   ((__m128i*)&Src[x>>1]);
        __m128i SrcV1 = _mm_unpacklo_epi8 (SrcV, _mm_setzero_si128());
        __m128i SrcV2 = _mm_unpackhi_epi8 (SrcV, _mm_setzero_si128());
        __m128i DstV1 = _mm_unpacklo_epi16(SrcV1, SrcV1);
        __m128i DstV2 = _mm_unpackhi_epi16(SrcV1, SrcV1);
        __m128i DstV3 = _mm_unpacklo_epi16(SrcV2, SrcV2);
        __m128i DstV4 = _mm_unpackhi_epi16(SrcV2, SrcV2);
        _mm_storeu_si128((__m128i*) & DstL0[x     ], DstV1);
        _mm_storeu_si128((__m128i*) & DstL0[x +  8], DstV2);
        _mm_storeu_si128((__m128i*) & DstL0[x + 16], DstV3);
        _mm_storeu_si128((__m128i*) & DstL0[x + 24], DstV4);
        _mm_storeu_si128((__m128i*) & DstL1[x     ], DstV1);
        _mm_storeu_si128((__m128i*) & DstL1[x +  8], DstV2);
        _mm_storeu_si128((__m128i*) & DstL1[x + 16], DstV3);
        _mm_storeu_si128((__m128i*) & DstL1[x + 24], DstV4);
      }
      for(int32 x=Width32; x<Width16; x+=16)
      {
        __m128i SrcVh = _mm_loadl_epi64((__m128i*)&Src[x>>1]);
        __m128i SrcV1 = _mm_unpacklo_epi8 (SrcVh, _mm_setzero_si128());
        __m128i DstV1 = _mm_unpacklo_epi16(SrcV1, SrcV1);
        __m128i DstV2 = _mm_unpackhi_epi16(SrcV1, SrcV1);
        _mm_storeu_si128((__m128i*) & DstL0[x    ], DstV1);
        _mm_storeu_si128((__m128i*) & DstL0[x + 8], DstV2);
        _mm_storeu_si128((__m128i*) & DstL1[x    ], DstV1);
        _mm_storeu_si128((__m128i*) & DstL1[x + 8], DstV2);
      }
      for(int32 x=Width16; x<DstWidth; x+=2)
      {
        int16 S = Src[x>>1];
        DstL0[x  ] = S;
        DstL0[x+1] = S;
        DstL1[x  ] = S;
        DstL1[x+1] = S;
      }
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    }
  }
}
void xPixelOpsAVX::CvtDownsampleHV(uint8* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  __m256i Two_I32_V = _mm256_set1_epi32((int32)2);

  const uint16* SrcL0 = Src;
  const uint16* SrcL1 = Src + SrcStride;

  if(((uint32)DstWidth & c_RemainderMask32) == 0) //Width%32==0
  {
    for(int32 y = 0; y < DstHeight; y++)
    {
      for(int32 x = 0; x < DstWidth; x += 32)
      {
        const int32 SrcX = x<<1;
        __m256i TopA_U16_V = _mm256_loadu_si256((__m256i*)&SrcL0[SrcX     ]);
        __m256i TopB_U16_V = _mm256_loadu_si256((__m256i*)&SrcL0[SrcX + 16]);
        __m256i TopC_U16_V = _mm256_loadu_si256((__m256i*)&SrcL0[SrcX + 32]);
        __m256i TopD_U16_V = _mm256_loadu_si256((__m256i*)&SrcL0[SrcX + 48]);
        __m256i BotA_U16_V = _mm256_loadu_si256((__m256i*)&SrcL1[SrcX     ]);
        __m256i BotB_U16_V = _mm256_loadu_si256((__m256i*)&SrcL1[SrcX + 16]);
        __m256i BotC_U16_V = _mm256_loadu_si256((__m256i*)&SrcL1[SrcX + 32]);
        __m256i BotD_U16_V = _mm256_loadu_si256((__m256i*)&SrcL1[SrcX + 48]);
        __m256i A_U16_V    = _mm256_add_epi16(TopA_U16_V, BotA_U16_V);
        __m256i B_U16_V    = _mm256_add_epi16(TopB_U16_V, BotB_U16_V);
        __m256i C_U16_V    = _mm256_add_epi16(TopC_U16_V, BotC_U16_V);
        __m256i D_U16_V    = _mm256_add_epi16(TopD_U16_V, BotD_U16_V);
        __m256i A_I32_V    = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(A_U16_V, _mm256_set1_epi16(1)), Two_I32_V), 2);
        __m256i B_I32_V    = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(B_U16_V, _mm256_set1_epi16(1)), Two_I32_V), 2);
        __m256i C_I32_V    = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(C_U16_V, _mm256_set1_epi16(1)), Two_I32_V), 2);
        __m256i D_I32_V    = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(D_U16_V, _mm256_set1_epi16(1)), Two_I32_V), 2);
        __m256i AB_U16_V   = _mm256_permute4x64_epi64(_mm256_packus_epi32(A_I32_V, B_I32_V), 0xD8);
        __m256i CD_U16_V   = _mm256_permute4x64_epi64(_mm256_packus_epi32(C_I32_V, D_I32_V), 0xD8);
        __m256i Dst_U8_V   = _mm256_permute4x64_epi64(_mm256_packus_epi16(AB_U16_V, CD_U16_V), 0xD8);
        _mm256_storeu_si256((__m256i*)(&(Dst[x])), Dst_U8_V);
      }
      SrcL0 += (SrcStride << 1);
      SrcL1 += (SrcStride << 1);
      Dst   += DstStride;
    }
  }
  else
  {
    const int32 Width32 = (int32)((uint32)DstWidth & c_MultipleMask32);
    const int32 Width16 = (int32)((uint32)DstWidth & c_MultipleMask16);
    const int32 Width8  = (int32)((uint32)DstWidth & c_MultipleMask8 );

    for(int32 y = 0; y < DstHeight; y++)
    {
      for(int32 x = 0; x < Width32; x += 32)
      {
        const int32 SrcX = x<<1;
        __m256i TopA_U16_V = _mm256_loadu_si256((__m256i*)&SrcL0[SrcX     ]);
        __m256i TopB_U16_V = _mm256_loadu_si256((__m256i*)&SrcL0[SrcX + 16]);
        __m256i TopC_U16_V = _mm256_loadu_si256((__m256i*)&SrcL0[SrcX + 32]);
        __m256i TopD_U16_V = _mm256_loadu_si256((__m256i*)&SrcL0[SrcX + 48]);
        __m256i BotA_U16_V = _mm256_loadu_si256((__m256i*)&SrcL1[SrcX     ]);
        __m256i BotB_U16_V = _mm256_loadu_si256((__m256i*)&SrcL1[SrcX + 16]);
        __m256i BotC_U16_V = _mm256_loadu_si256((__m256i*)&SrcL1[SrcX + 32]);
        __m256i BotD_U16_V = _mm256_loadu_si256((__m256i*)&SrcL1[SrcX + 48]);
        __m256i A_U16_V    = _mm256_add_epi16(TopA_U16_V, BotA_U16_V);
        __m256i B_U16_V    = _mm256_add_epi16(TopB_U16_V, BotB_U16_V);
        __m256i C_U16_V    = _mm256_add_epi16(TopC_U16_V, BotC_U16_V);
        __m256i D_U16_V    = _mm256_add_epi16(TopD_U16_V, BotD_U16_V);
        __m256i A_I32_V    = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(A_U16_V, _mm256_set1_epi16(1)), Two_I32_V), 2);
        __m256i B_I32_V    = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(B_U16_V, _mm256_set1_epi16(1)), Two_I32_V), 2);
        __m256i C_I32_V    = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(C_U16_V, _mm256_set1_epi16(1)), Two_I32_V), 2);
        __m256i D_I32_V    = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(D_U16_V, _mm256_set1_epi16(1)), Two_I32_V), 2);
        __m256i AB_U16_V   = _mm256_permute4x64_epi64(_mm256_packus_epi32(A_I32_V, B_I32_V), 0xD8);
        __m256i CD_U16_V   = _mm256_permute4x64_epi64(_mm256_packus_epi32(C_I32_V, D_I32_V), 0xD8);
        __m256i Dst_U8_V   = _mm256_permute4x64_epi64(_mm256_packus_epi16(AB_U16_V, CD_U16_V), 0xD8);
        _mm256_storeu_si256((__m256i*)(&(Dst[x])), Dst_U8_V);
      }
      for(int32 x = Width32; x < Width16; x += 16)
      {
        const int32 SrcX = x<<1;
        __m128i TopA_U16_V = _mm_loadu_si128((__m128i*)&SrcL0[SrcX     ]);
        __m128i TopB_U16_V = _mm_loadu_si128((__m128i*)&SrcL0[SrcX + 8 ]);
        __m128i TopC_U16_V = _mm_loadu_si128((__m128i*)&SrcL0[SrcX + 16]);
        __m128i TopD_U16_V = _mm_loadu_si128((__m128i*)&SrcL0[SrcX + 24]);
        __m128i BotA_U16_V = _mm_loadu_si128((__m128i*)&SrcL1[SrcX     ]);
        __m128i BotB_U16_V = _mm_loadu_si128((__m128i*)&SrcL1[SrcX + 8 ]);
        __m128i BotC_U16_V = _mm_loadu_si128((__m128i*)&SrcL1[SrcX + 16]);
        __m128i BotD_U16_V = _mm_loadu_si128((__m128i*)&SrcL1[SrcX + 24]);
        __m128i A_U16_V    = _mm_add_epi16(TopA_U16_V, BotA_U16_V);
        __m128i B_U16_V    = _mm_add_epi16(TopB_U16_V, BotB_U16_V);
        __m128i C_U16_V    = _mm_add_epi16(TopC_U16_V, BotC_U16_V);
        __m128i D_U16_V    = _mm_add_epi16(TopD_U16_V, BotD_U16_V);
        __m128i A_I32_V    = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(A_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(Two_I32_V)), 2);
        __m128i B_I32_V    = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(B_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(Two_I32_V)), 2);
        __m128i C_I32_V    = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(C_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(Two_I32_V)), 2);
        __m128i D_I32_V    = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(D_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(Two_I32_V)), 2);
        __m128i AB_U16_V   = _mm_packus_epi32(A_I32_V, B_I32_V);
        __m128i CD_U16_V   = _mm_packus_epi32(C_I32_V, D_I32_V);
        __m128i Dst_U8_V   = _mm_packus_epi16(AB_U16_V, CD_U16_V);
        _mm_storeu_si128((__m128i*)(&(Dst[x])), Dst_U8_V);
      }
      for(int32 x = Width16; x < Width8; x += 8)
      {
        const int32 SrcX = x<<1;
        __m128i TopA_U16_V = _mm_loadu_si128((__m128i*)&SrcL0[SrcX     ]);
        __m128i TopB_U16_V = _mm_loadu_si128((__m128i*)&SrcL0[SrcX + 8 ]);
        __m128i BotA_U16_V = _mm_loadu_si128((__m128i*)&SrcL1[SrcX     ]);
        __m128i BotB_U16_V = _mm_loadu_si128((__m128i*)&SrcL1[SrcX + 8 ]);
        __m128i A_U16_V    = _mm_add_epi16(TopA_U16_V, BotA_U16_V);
        __m128i B_U16_V    = _mm_add_epi16(TopB_U16_V, BotB_U16_V);
        __m128i A_I32_V    = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(A_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(Two_I32_V)), 2);
        __m128i B_I32_V    = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(B_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(Two_I32_V)), 2);
        __m128i AB_U16_V   = _mm_packus_epi32(A_I32_V, B_I32_V);
        __m128i Dst_U8_V   = _mm_packus_epi16(AB_U16_V, _mm_setzero_si128());
        _mm_storel_epi64((__m128i*)(&(Dst[x])), Dst_U8_V);
      }
      for(int32 x = Width8; x < DstWidth; x++)
      {
        const int32 SrcX = x << 1;
        int32 D = ((int32)SrcL0[SrcX] + (int32)SrcL0[SrcX + 1] + (int32)SrcL1[SrcX] + (int32)SrcL1[SrcX + 1] + 2) >> 2;
        Dst[x] = (uint8)xClip<int32>(D, 0, 255);
      }
      SrcL0 += (SrcStride << 1);
      SrcL1 += (SrcStride << 1);
      Dst   += DstStride;
    }

  }
}
void xPixelOpsAVX::UpsampleH(uint16* restrict Dst, const uint16* restrict Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  if(((uint32)DstWidth & c_RemainderMask32)==0) //Width%32==0
  {
    for(int32 y=0; y<DstHeight; y++)
    {
      for(int32 x=0; x<DstWidth; x+=32)
      {
        __m256i SrcVt  = _mm256_loadu_si256((__m256i*)&Src[x>>1]);
        __m256i SrcV   = _mm256_permute4x64_epi64(SrcVt, 0xD8); //fix AVX per lane mess
        __m256i LeftV  = _mm256_unpacklo_epi16(SrcV, SrcV);
        __m256i RightV = _mm256_unpackhi_epi16(SrcV, SrcV);
        _mm256_storeu_si256((__m256i*)&Dst[x   ], LeftV );
        _mm256_storeu_si256((__m256i*)&Dst[x+16], RightV);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }
  else
  {
    const int32 Width32 = (int32)((uint32)DstWidth & c_MultipleMask32);
    const int32 Width16 = (int32)((uint32)DstWidth & c_MultipleMask16);
    const int32 Width8  = (int32)((uint32)DstWidth & c_MultipleMask8 );

    for(int32 y=0; y<DstHeight; y++)
    {
      for (int32 x=0; x<Width32; x+=32)
      {
        __m256i SrcVt  = _mm256_loadu_si256((__m256i*)&Src[x>>1]);
        __m256i SrcV   = _mm256_permute4x64_epi64(SrcVt, 0xD8); //fix AVX per lane mess
        __m256i LeftV  = _mm256_unpacklo_epi16(SrcV, SrcV);
        __m256i RightV = _mm256_unpackhi_epi16(SrcV, SrcV);
        _mm256_storeu_si256((__m256i*)&Dst[x   ], LeftV );
        _mm256_storeu_si256((__m256i*)&Dst[x+16], RightV);
      }
      for(int32 x=Width32; x<Width16; x+=16)
      {
        __m128i SrcV   = _mm_loadu_si128((__m128i*)&Src[x>>1]);
        __m128i LeftV  = _mm_unpacklo_epi16(SrcV, SrcV);
        __m128i RightV = _mm_unpackhi_epi16(SrcV, SrcV);
        _mm_storeu_si128((__m128i*)&Dst[x  ], LeftV );
        _mm_storeu_si128((__m128i*)&Dst[x+8], RightV);
      }
      for(int32 x=Width16; x<Width8; x+=8)
      {
        __m128i SrcV  = _mm_loadl_epi64((__m128i*)&Src[x>>1]);
        __m128i LeftV = _mm_unpacklo_epi16(SrcV, SrcV);
        _mm_storeu_si128((__m128i*)&Dst[x  ], LeftV );
      }
      for(int32 x=Width8; x<DstWidth; x+=2)
      {
        const uint16 S = Src[x>>1];
        Dst[x  ] = S;
        Dst[x+1] = S;
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }
}
void xPixelOpsAVX::DownsampleH(uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  __m256i One_I32_V = _mm256_set1_epi32((int32)1);

  if(((uint32)DstWidth & (uint32)c_RemainderMask16)==0) //Width%16==0
  {
    for(int32 y=0; y<DstHeight; y++)
    {
      for(int32 x=0; x<DstWidth; x+=16)
      {
        const int32 SrcX = x<<1;
        __m256i Left_U16_V  = _mm256_loadu_si256((__m256i*)&Src[SrcX   ]);
        __m256i Right_U16_V = _mm256_loadu_si256((__m256i*)&Src[SrcX+16]);
        __m256i Left_I32_V  = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(Left_U16_V , _mm256_set1_epi16(1)), One_I32_V), 1);
        __m256i Right_I32_V = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(Right_U16_V, _mm256_set1_epi16(1)), One_I32_V), 1);
        __m256i Out_U16_V   = _mm256_permute4x64_epi64(_mm256_packus_epi32(Left_I32_V, Right_I32_V), 0xD8);
        _mm256_storeu_si256((__m256i*)&Dst[x], Out_U16_V);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  } 
  else
  {
    const int32 Width16 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask16);
    const int32 Width8  = (int32)((uint32)DstWidth & (uint32)c_MultipleMask8 );
    const int32 Width4  = (int32)((uint32)DstWidth & (uint32)c_MultipleMask4 );

    for(int32 y = 0; y < DstHeight; y++)
    {
      for(int32 x = 0; x < Width16; x += 16)
      {
        const int32 SrcX = x<<1;
        __m256i Left_U16_V  = _mm256_loadu_si256((__m256i*)&Src[SrcX   ]);
        __m256i Right_U16_V = _mm256_loadu_si256((__m256i*)&Src[SrcX+16]);
        __m256i Left_I32_V  = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(Left_U16_V , _mm256_set1_epi16(1)), One_I32_V), 1);
        __m256i Right_I32_V = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(Right_U16_V, _mm256_set1_epi16(1)), One_I32_V), 1);
        __m256i Out_U16_V   = _mm256_permute4x64_epi64(_mm256_packus_epi32(Left_I32_V, Right_I32_V), 0xD8);
        _mm256_storeu_si256((__m256i*)&Dst[x], Out_U16_V);
      }
      for(int32 x = Width16; x < Width8; x += 8)
      {
        const int32 SrcX = x<<1;
        __m128i Left_U16_V  = _mm_loadu_si128((__m128i*)&Src[SrcX  ]);
        __m128i Right_U16_V = _mm_loadu_si128((__m128i*)&Src[SrcX+8]);
        __m128i Left_I32_V  = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(Left_U16_V , _mm_set1_epi16(1)), _mm256_castsi256_si128(One_I32_V)), 1);
        __m128i Right_I32_V = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(Right_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(One_I32_V)), 1);
        __m128i Out_U16_V   = _mm_packus_epi32(Left_I32_V, Right_I32_V);
        _mm_storeu_si128 ((__m128i*)&Dst[x], Out_U16_V);
      }
      for(int32 x = Width8; x < Width4; x += 4)
      {
        const int32 SrcX = x<<1;
        __m128i Left_U16_V = _mm_loadu_si128((__m128i*)&Src[SrcX  ]);
        __m128i Left_I32_V = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(Left_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(One_I32_V)), 1);
        __m128i Out_U16_V  = _mm_packus_epi32(Left_I32_V, _mm_setzero_si128());
        _mm_storel_epi64 ((__m128i*)&Dst[x], Out_U16_V);

      }
      for(int32 x = Width4; x < DstWidth; x++)
      {
        const int32 SrcX = x << 1;
        int16 D = ((int32)Src[SrcX] + (int32)Src[SrcX + 1] + 1) >> 1;
        Dst[x] = D;
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }
}
void xPixelOpsAVX::CvtUpsampleH(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  if(((uint32)DstWidth & c_RemainderMask64)==0) //Width%64==0
  {
    for(int32 y=0; y<DstHeight; y++)
    {
      for(int32 x=0; x<DstWidth; x+=64)
      {
        __m256i SrcVt  = _mm256_loadu_si256      ((__m256i*)&Src[x>>1]);
        __m256i SrcV   = _mm256_permute4x64_epi64(SrcVt, 0xD8); //fix AVX per lane mess
        __m256i SrcV1t = _mm256_unpacklo_epi8    (SrcV, _mm256_setzero_si256());
        __m256i SrcV2t = _mm256_unpackhi_epi8    (SrcV, _mm256_setzero_si256());
        __m256i SrcV1  = _mm256_permute4x64_epi64(SrcV1t, 0xD8); //fix AVX per lane mess
        __m256i SrcV2  = _mm256_permute4x64_epi64(SrcV2t, 0xD8); //fix AVX per lane mess
        __m256i DstV1  = _mm256_unpacklo_epi16   (SrcV1, SrcV1);
        __m256i DstV2  = _mm256_unpackhi_epi16   (SrcV1, SrcV1);
        __m256i DstV3  = _mm256_unpacklo_epi16   (SrcV2, SrcV2);
        __m256i DstV4  = _mm256_unpackhi_epi16   (SrcV2, SrcV2);
        _mm256_storeu_si256((__m256i*) & Dst[x     ], DstV1);
        _mm256_storeu_si256((__m256i*) & Dst[x + 16], DstV2);
        _mm256_storeu_si256((__m256i*) & Dst[x + 32], DstV3);
        _mm256_storeu_si256((__m256i*) & Dst[x + 48], DstV4);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }
  else
  {
    const int32 Width64 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask64);
    const int32 Width32 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask32);
    const int32 Width16 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask16);

    for(int32 y=0; y<DstHeight; y++)
    {
      for(int32 x=0; x<Width64; x+=64)
      {
        __m256i SrcVt  = _mm256_loadu_si256      ((__m256i*)&Src[x>>1]);
        __m256i SrcV   = _mm256_permute4x64_epi64(SrcVt, 0xD8); //fix AVX per lane mess
        __m256i SrcV1t = _mm256_unpacklo_epi8    (SrcV, _mm256_setzero_si256());
        __m256i SrcV2t = _mm256_unpackhi_epi8    (SrcV, _mm256_setzero_si256());
        __m256i SrcV1  = _mm256_permute4x64_epi64(SrcV1t, 0xD8); //fix AVX per lane mess
        __m256i SrcV2  = _mm256_permute4x64_epi64(SrcV2t, 0xD8); //fix AVX per lane mess
        __m256i DstV1  = _mm256_unpacklo_epi16   (SrcV1, SrcV1);
        __m256i DstV2  = _mm256_unpackhi_epi16   (SrcV1, SrcV1);
        __m256i DstV3  = _mm256_unpacklo_epi16   (SrcV2, SrcV2);
        __m256i DstV4  = _mm256_unpackhi_epi16   (SrcV2, SrcV2);
        _mm256_storeu_si256((__m256i*) & Dst[x     ], DstV1);
        _mm256_storeu_si256((__m256i*) & Dst[x + 16], DstV2);
        _mm256_storeu_si256((__m256i*) & Dst[x + 32], DstV3);
        _mm256_storeu_si256((__m256i*) & Dst[x + 48], DstV4);
      }
      for(int32 x=Width64; x<Width32; x+=32)
      {
        __m128i SrcV  = _mm_loadu_si128   ((__m128i*)&Src[x>>1]);
        __m128i SrcV1 = _mm_unpacklo_epi8 (SrcV, _mm_setzero_si128());
        __m128i SrcV2 = _mm_unpackhi_epi8 (SrcV, _mm_setzero_si128());
        __m128i DstV1 = _mm_unpacklo_epi16(SrcV1, SrcV1);
        __m128i DstV2 = _mm_unpackhi_epi16(SrcV1, SrcV1);
        __m128i DstV3 = _mm_unpacklo_epi16(SrcV2, SrcV2);
        __m128i DstV4 = _mm_unpackhi_epi16(SrcV2, SrcV2);
        _mm_storeu_si128((__m128i*) & Dst[x     ], DstV1);
        _mm_storeu_si128((__m128i*) & Dst[x +  8], DstV2);
        _mm_storeu_si128((__m128i*) & Dst[x + 16], DstV3);
        _mm_storeu_si128((__m128i*) & Dst[x + 24], DstV4);
      }
      for(int32 x=Width32; x<Width16; x+=16)
      {
        __m128i SrcVh = _mm_loadl_epi64((__m128i*)&Src[x>>1]);
        __m128i SrcV1 = _mm_unpacklo_epi8 (SrcVh, _mm_setzero_si128());
        __m128i DstV1 = _mm_unpacklo_epi16(SrcV1, SrcV1);
        __m128i DstV2 = _mm_unpackhi_epi16(SrcV1, SrcV1);
        _mm_storeu_si128((__m128i*) & Dst[x    ], DstV1);
        _mm_storeu_si128((__m128i*) & Dst[x + 8], DstV2);
      }
      for(int32 x=Width16; x<DstWidth; x+=2)
      {
        int16 S = Src[x>>1];
        Dst[x  ] = S;
        Dst[x+1] = S;
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }
}
void xPixelOpsAVX::CvtDownsampleH(uint8* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  __m256i One_I32_V = _mm256_set1_epi32((int32)1);


  if(((uint32)DstWidth & c_RemainderMask32) == 0) //Width%32==0
  {
    for(int32 y = 0; y < DstHeight; y++)
    {
      for(int32 x = 0; x < DstWidth; x += 32)
      {
        const int32 SrcX = x<<1;
        __m256i A_U16_V  = _mm256_loadu_si256((__m256i*)&Src[SrcX     ]);
        __m256i B_U16_V  = _mm256_loadu_si256((__m256i*)&Src[SrcX + 16]);
        __m256i C_U16_V  = _mm256_loadu_si256((__m256i*)&Src[SrcX + 32]);
        __m256i D_U16_V  = _mm256_loadu_si256((__m256i*)&Src[SrcX + 48]);
        __m256i A_I32_V  = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(A_U16_V, _mm256_set1_epi16(1)), One_I32_V), 1);
        __m256i B_I32_V  = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(B_U16_V, _mm256_set1_epi16(1)), One_I32_V), 1);
        __m256i C_I32_V  = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(C_U16_V, _mm256_set1_epi16(1)), One_I32_V), 1);
        __m256i D_I32_V  = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(D_U16_V, _mm256_set1_epi16(1)), One_I32_V), 1);
        __m256i AB_U16_V = _mm256_permute4x64_epi64(_mm256_packus_epi32(A_I32_V, B_I32_V), 0xD8);
        __m256i CD_U16_V = _mm256_permute4x64_epi64(_mm256_packus_epi32(C_I32_V, D_I32_V), 0xD8);
        __m256i Dst_U8_V = _mm256_permute4x64_epi64(_mm256_packus_epi16(AB_U16_V, CD_U16_V), 0xD8);
        _mm256_storeu_si256((__m256i*)(&(Dst[x])), Dst_U8_V);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }
  else
  {
    const int32 Width32 = (int32)((uint32)DstWidth & c_MultipleMask32);
    const int32 Width16 = (int32)((uint32)DstWidth & c_MultipleMask16);
    const int32 Width8  = (int32)((uint32)DstWidth & c_MultipleMask8 );

    for(int32 y = 0; y < DstHeight; y++)
    {
      for(int32 x = 0; x < Width32; x += 32)
      {
        const int32 SrcX = x<<1;
        __m256i A_U16_V  = _mm256_loadu_si256((__m256i*)&Src[SrcX     ]);
        __m256i B_U16_V  = _mm256_loadu_si256((__m256i*)&Src[SrcX + 16]);
        __m256i C_U16_V  = _mm256_loadu_si256((__m256i*)&Src[SrcX + 32]);
        __m256i D_U16_V  = _mm256_loadu_si256((__m256i*)&Src[SrcX + 48]);
        __m256i A_I32_V  = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(A_U16_V, _mm256_set1_epi16(1)), One_I32_V), 1);
        __m256i B_I32_V  = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(B_U16_V, _mm256_set1_epi16(1)), One_I32_V), 1);
        __m256i C_I32_V  = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(C_U16_V, _mm256_set1_epi16(1)), One_I32_V), 1);
        __m256i D_I32_V  = _mm256_srai_epi32(_mm256_add_epi32(_mm256_madd_epi16(D_U16_V, _mm256_set1_epi16(1)), One_I32_V), 1);
        __m256i AB_U16_V = _mm256_permute4x64_epi64(_mm256_packus_epi32(A_I32_V, B_I32_V), 0xD8);
        __m256i CD_U16_V = _mm256_permute4x64_epi64(_mm256_packus_epi32(C_I32_V, D_I32_V), 0xD8);
        __m256i Dst_U8_V = _mm256_permute4x64_epi64(_mm256_packus_epi16(AB_U16_V, CD_U16_V), 0xD8);
        _mm256_storeu_si256((__m256i*)(&(Dst[x])), Dst_U8_V);
      }
      for(int32 x = Width32; x < Width16; x += 16)
      {
        const int32 SrcX = x<<1;
        __m128i A_U16_V  = _mm_loadu_si128((__m128i*)&Src[SrcX     ]);
        __m128i B_U16_V  = _mm_loadu_si128((__m128i*)&Src[SrcX + 8 ]);
        __m128i C_U16_V  = _mm_loadu_si128((__m128i*)&Src[SrcX + 16]);
        __m128i D_U16_V  = _mm_loadu_si128((__m128i*)&Src[SrcX + 24]);
        __m128i A_I32_V  = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(A_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(One_I32_V)), 1);
        __m128i B_I32_V  = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(B_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(One_I32_V)), 1);
        __m128i C_I32_V  = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(C_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(One_I32_V)), 1);
        __m128i D_I32_V  = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(D_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(One_I32_V)), 1);
        __m128i AB_U16_V = _mm_packus_epi32(A_I32_V, B_I32_V);
        __m128i CD_U16_V = _mm_packus_epi32(C_I32_V, D_I32_V);
        __m128i Dst_U8_V = _mm_packus_epi16(AB_U16_V, CD_U16_V);
        _mm_storeu_si128((__m128i*)(&(Dst[x])), Dst_U8_V);
      }
      for(int32 x = Width16; x < Width8; x += 8)
      {
        const int32 SrcX = x<<1;
        __m128i A_U16_V  = _mm_loadu_si128((__m128i*)&Src[SrcX     ]);
        __m128i B_U16_V  = _mm_loadu_si128((__m128i*)&Src[SrcX + 8 ]);
        __m128i A_I32_V  = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(A_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(One_I32_V)), 1);
        __m128i B_I32_V  = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(B_U16_V, _mm_set1_epi16(1)), _mm256_castsi256_si128(One_I32_V)), 1);
        __m128i AB_U16_V = _mm_packus_epi32(A_I32_V, B_I32_V);
        __m128i Dst_U8_V = _mm_packus_epi16(AB_U16_V, _mm_setzero_si128());
        _mm_storel_epi64((__m128i*)(&(Dst[x])), Dst_U8_V);
      }
      for(int32 x = Width8; x < DstWidth; x++)
      {
        const int32 SrcX = x << 1;
        int32 D = ((int32)Src[SrcX] + (int32)Src[SrcX + 1] + 1) >> 1;
        Dst[x] = (uint8)xClip<int32>(D, 0, 255);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }
}
bool xPixelOpsAVX::CheckValues(const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth)
{
  if(BitDepth == 16) { return true; }

  const int32   MaxValue  = xBitDepth2MaxValue(BitDepth);
  const __m256i MaxValueV = _mm256_set1_epi16((int16)MaxValue);

  if(((uint32)Width & c_RemainderMask32) == 0) //Width%32==0
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 32)
      {
        __m256i SrcV1  = _mm256_loadu_si256((__m256i*)&Src[x   ]);
        __m256i SrcV2  = _mm256_loadu_si256((__m256i*)&Src[x+16]);
        __m256i MaskV1 = _mm256_cmpgt_epi16(SrcV1, MaxValueV); //0 - <=, 0xFFFF - >
        __m256i MaskV2 = _mm256_cmpgt_epi16(SrcV2, MaxValueV); //0 - <=, 0xFFFF - >
        __m256i Masks  = _mm256_packs_epi16(MaskV1, MaskV2);
        uint32  Mask   = _mm256_movemask_epi8(Masks);
        if(Mask) { return false; }
      }
      Src += SrcStride;
    } //y
  }
  else
  {
    const int32 Width32 = (int32)((uint32)Width & c_MultipleMask32);
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 32)
      {
        __m256i SrcV1  = _mm256_loadu_si256((__m256i*)&Src[x   ]);
        __m256i SrcV2  = _mm256_loadu_si256((__m256i*)&Src[x+16]);
        __m256i MaskV1 = _mm256_cmpgt_epi16(SrcV1, MaxValueV); //0 - <=, 0xFFFF - >
        __m256i MaskV2 = _mm256_cmpgt_epi16(SrcV2, MaxValueV); //0 - <=, 0xFFFF - >
        __m256i Masks  = _mm256_packs_epi16(MaskV1, MaskV2);
        uint32  Mask   = _mm256_movemask_epi8(Masks);
        if(Mask) { return false; }
      }
      for (int32 x = Width32; x < Width; x++)
      {
        if (Src[x] > MaxValue) { return false; }
      }
      Src += SrcStride;
    } //y
  }

  return true;
}
void xPixelOpsAVX::AOS4fromSOA3(uint16* restrict DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, const uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  const __m256i d = _mm256_set1_epi16(ValueD);

  if(((uint32)Width & c_RemainderMask16) == 0) //Width%16==0
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 16)
      {
        //load
        __m256i a = _mm256_loadu_si256((__m256i*) & SrcA[x]); //load A0-A7
        __m256i b = _mm256_loadu_si256((__m256i*) & SrcB[x]); //load B0-B7
        __m256i c = _mm256_loadu_si256((__m256i*) & SrcC[x]); //load C0-C7

        //transpose
        __m256i ac_0    = _mm256_unpacklo_epi16(a   , c   );
        __m256i ac_1    = _mm256_unpackhi_epi16(a   , c   );
        __m256i bd_0    = _mm256_unpacklo_epi16(b   , d   );
        __m256i bd_1    = _mm256_unpackhi_epi16(b   , d   );
        __m256i abcd_0t = _mm256_unpacklo_epi16(ac_0, bd_0);
        __m256i abcd_1t = _mm256_unpackhi_epi16(ac_0, bd_0);
        __m256i abcd_2t = _mm256_unpacklo_epi16(ac_1, bd_1);
        __m256i abcd_3t = _mm256_unpackhi_epi16(ac_1, bd_1);

        //fix AVX per lane mess
        __m256i abcd_0  = _mm256_permute2x128_si256(abcd_0t, abcd_1t, 0x20); //fix AVX per lane mess
        __m256i abcd_1  = _mm256_permute2x128_si256(abcd_2t, abcd_3t, 0x20); //fix AVX per lane mess
        __m256i abcd_2  = _mm256_permute2x128_si256(abcd_0t, abcd_1t, 0x31); //fix AVX per lane mess
        __m256i abcd_3  = _mm256_permute2x128_si256(abcd_2t, abcd_3t, 0x31); //fix AVX per lane mess

        //save
        _mm256_storeu_si256((__m256i*) & DstABCD[(x << 2) +  0], abcd_0);
        _mm256_storeu_si256((__m256i*) & DstABCD[(x << 2) + 16], abcd_1);
        _mm256_storeu_si256((__m256i*) & DstABCD[(x << 2) + 32], abcd_2);
        _mm256_storeu_si256((__m256i*) & DstABCD[(x << 2) + 48], abcd_3);
      }
      SrcA    += SrcStride;
      SrcB    += SrcStride;
      SrcC    += SrcStride;
      DstABCD += DstStride;
    }
  }
  else
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16);
    const int32 Width8  = (int32)((uint32)Width & c_MultipleMask8 );
    const int32 Width4  = (int32)((uint32)Width & c_MultipleMask4 );

    for(int32 y = 0; y < Height; y++)
    {
      for (int32 x = 0; x < Width16; x += 16)
      {
        //load
        __m256i a = _mm256_loadu_si256((__m256i*) & SrcA[x]); //load A0-A7
        __m256i b = _mm256_loadu_si256((__m256i*) & SrcB[x]); //load B0-B7
        __m256i c = _mm256_loadu_si256((__m256i*) & SrcC[x]); //load C0-C7

        //transpose
        __m256i ac_0    = _mm256_unpacklo_epi16(a   , c   );
        __m256i ac_1    = _mm256_unpackhi_epi16(a   , c   );
        __m256i bd_0    = _mm256_unpacklo_epi16(b   , d   );
        __m256i bd_1    = _mm256_unpackhi_epi16(b   , d   );
        __m256i abcd_0t = _mm256_unpacklo_epi16(ac_0, bd_0);
        __m256i abcd_1t = _mm256_unpackhi_epi16(ac_0, bd_0);
        __m256i abcd_2t = _mm256_unpacklo_epi16(ac_1, bd_1);
        __m256i abcd_3t = _mm256_unpackhi_epi16(ac_1, bd_1);

        //fix AVX per lane mess
        __m256i abcd_0  = _mm256_permute2x128_si256(abcd_0t, abcd_1t, 0x20);
        __m256i abcd_1  = _mm256_permute2x128_si256(abcd_2t, abcd_3t, 0x20);
        __m256i abcd_2  = _mm256_permute2x128_si256(abcd_0t, abcd_1t, 0x31);
        __m256i abcd_3  = _mm256_permute2x128_si256(abcd_2t, abcd_3t, 0x31);

        //save
        _mm256_storeu_si256((__m256i*) & DstABCD[(x << 2) +  0], abcd_0);
        _mm256_storeu_si256((__m256i*) & DstABCD[(x << 2) + 16], abcd_1);
        _mm256_storeu_si256((__m256i*) & DstABCD[(x << 2) + 32], abcd_2);
        _mm256_storeu_si256((__m256i*) & DstABCD[(x << 2) + 48], abcd_3);
      }
      for(int32 x = Width16; x < Width8; x += 8)
      {
        //load
        __m128i a = _mm_loadu_si128((__m128i*) & SrcA[x]); //load A0-A7
        __m128i b = _mm_loadu_si128((__m128i*) & SrcB[x]); //load B0-B7
        __m128i c = _mm_loadu_si128((__m128i*) & SrcC[x]); //load C0-C7

        //transpose
        __m128i ac_0   = _mm_unpacklo_epi16(a   , c   );
        __m128i ac_1   = _mm_unpackhi_epi16(a   , c   );
        __m128i bd_0   = _mm_unpacklo_epi16(b   , _mm256_castsi256_si128(d));
        __m128i bd_1   = _mm_unpackhi_epi16(b   , _mm256_castsi256_si128(d));
        __m128i abcd_0 = _mm_unpacklo_epi16(ac_0, bd_0);
        __m128i abcd_1 = _mm_unpackhi_epi16(ac_0, bd_0);
        __m128i abcd_2 = _mm_unpacklo_epi16(ac_1, bd_1);
        __m128i abcd_3 = _mm_unpackhi_epi16(ac_1, bd_1);

        //save
        _mm_storeu_si128((__m128i*) & DstABCD[(x << 2) +  0], abcd_0);
        _mm_storeu_si128((__m128i*) & DstABCD[(x << 2) +  8], abcd_1);
        _mm_storeu_si128((__m128i*) & DstABCD[(x << 2) + 16], abcd_2);
        _mm_storeu_si128((__m128i*) & DstABCD[(x << 2) + 24], abcd_3);
      }
      for(int32 x = Width8; x < Width4; x += 4)
      {
        //load
        __m128i a = _mm_loadl_epi64((__m128i*) & SrcA[x]); //load R0-R7
        __m128i b = _mm_loadl_epi64((__m128i*) & SrcB[x]); //load G0-G7
        __m128i c = _mm_loadl_epi64((__m128i*) & SrcC[x]); //load B0-B7

        //transpose
        __m128i ac_0   = _mm_unpacklo_epi16(a   , c   );
        __m128i bd_0   = _mm_unpacklo_epi16(b   , _mm256_castsi256_si128(d));
        __m128i abcd_0 = _mm_unpacklo_epi16(ac_0, bd_0);
        __m128i abcd_1 = _mm_unpackhi_epi16(ac_0, bd_0);

        //save
        _mm_storeu_si128((__m128i*) & DstABCD[(x << 2) + 0], abcd_0);
        _mm_storeu_si128((__m128i*) & DstABCD[(x << 2) + 8], abcd_1);
      }
      for(int32 x = Width4; x < Width; x++)
      {
        DstABCD[(x << 2) + 0] = SrcA[x];
        DstABCD[(x << 2) + 1] = SrcB[x];
        DstABCD[(x << 2) + 2] = SrcC[x];
        DstABCD[(x << 2) + 3] = ValueD;
      }
      SrcA    += SrcStride;
      SrcB    += SrcStride;
      SrcC    += SrcStride;
      DstABCD += DstStride;
    }
  }
}
void xPixelOpsAVX::SOA3fromAOS4(uint16* restrict DstA, uint16* restrict DstB, uint16* restrict DstC, const uint16* SrcABCD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
    if(((uint32)Width & (uint32)c_RemainderMask16)==0) //Width%16==0
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        //load
        __m256i abcd_0 = _mm256_loadu_si256((__m256i*)&SrcABCD[(x<<2)+ 0]);
        __m256i abcd_1 = _mm256_loadu_si256((__m256i*)&SrcABCD[(x<<2)+16]);
        __m256i abcd_2 = _mm256_loadu_si256((__m256i*)&SrcABCD[(x<<2)+32]);
        __m256i abcd_3 = _mm256_loadu_si256((__m256i*)&SrcABCD[(x<<2)+48]);

        //transpose
        __m256i abcd_0a = _mm256_permute2x128_si256(abcd_0, abcd_2, 0x20);
        __m256i abcd_1a = _mm256_permute2x128_si256(abcd_0, abcd_2, 0x31);
        __m256i abcd_2a = _mm256_permute2x128_si256(abcd_1, abcd_3, 0x20);
        __m256i abcd_3a = _mm256_permute2x128_si256(abcd_1, abcd_3, 0x31);

        __m256i ac_0 = _mm256_unpacklo_epi16(abcd_0a, abcd_1a);
        __m256i ac_1 = _mm256_unpackhi_epi16(abcd_0a, abcd_1a);
        __m256i bd_0 = _mm256_unpacklo_epi16(abcd_2a, abcd_3a);
        __m256i bd_1 = _mm256_unpackhi_epi16(abcd_2a, abcd_3a);

        __m256i a_0 = _mm256_unpacklo_epi16(ac_0, ac_1);
        __m256i b_0 = _mm256_unpackhi_epi16(ac_0, ac_1);
        __m256i c_0 = _mm256_unpacklo_epi16(bd_0, bd_1);
        __m256i d_0 = _mm256_unpackhi_epi16(bd_0, bd_1);

        __m256i a = _mm256_unpacklo_epi64(a_0, c_0);
        __m256i b = _mm256_unpackhi_epi64(a_0, c_0);
        __m256i c = _mm256_unpacklo_epi64(b_0, d_0);

        //save
        _mm256_storeu_si256((__m256i*)&DstA[x], a);
        _mm256_storeu_si256((__m256i*)&DstB[x], b);
        _mm256_storeu_si256((__m256i*)&DstC[x], c);
      }
      SrcABCD += SrcStride;
      DstA    += DstStride;
      DstB    += DstStride;
      DstC    += DstStride;
    }
  }
  else
  {
    int32 Width16 = (int32)((uint32)Width & (uint32)c_MultipleMask16);
    int32 Width8  = (int32)((uint32)Width & (uint32)c_MultipleMask8);
    int32 Width4  = (int32)((uint32)Width & (uint32)c_MultipleMask4);

    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width16; x+=16)
      {
        //load
        __m256i abcd_0 = _mm256_loadu_si256((__m256i*)&SrcABCD[(x<<2)+ 0]);
        __m256i abcd_1 = _mm256_loadu_si256((__m256i*)&SrcABCD[(x<<2)+16]);
        __m256i abcd_2 = _mm256_loadu_si256((__m256i*)&SrcABCD[(x<<2)+32]);
        __m256i abcd_3 = _mm256_loadu_si256((__m256i*)&SrcABCD[(x<<2)+48]);

        //transpose
        __m256i abcd_0a = _mm256_permute2x128_si256(abcd_0, abcd_2, 0x20);
        __m256i abcd_1a = _mm256_permute2x128_si256(abcd_0, abcd_2, 0x31);
        __m256i abcd_2a = _mm256_permute2x128_si256(abcd_1, abcd_3, 0x20);
        __m256i abcd_3a = _mm256_permute2x128_si256(abcd_1, abcd_3, 0x31);

        __m256i ac_0 = _mm256_unpacklo_epi16(abcd_0a, abcd_1a);
        __m256i ac_1 = _mm256_unpackhi_epi16(abcd_0a, abcd_1a);
        __m256i bd_0 = _mm256_unpacklo_epi16(abcd_2a, abcd_3a);
        __m256i bd_1 = _mm256_unpackhi_epi16(abcd_2a, abcd_3a);

        __m256i a_0 = _mm256_unpacklo_epi16(ac_0, ac_1);
        __m256i b_0 = _mm256_unpackhi_epi16(ac_0, ac_1);
        __m256i c_0 = _mm256_unpacklo_epi16(bd_0, bd_1);
        __m256i d_0 = _mm256_unpackhi_epi16(bd_0, bd_1);

        __m256i a = _mm256_unpacklo_epi64(a_0, c_0);
        __m256i b = _mm256_unpackhi_epi64(a_0, c_0);
        __m256i c = _mm256_unpacklo_epi64(b_0, d_0);

        //save
        _mm256_storeu_si256((__m256i*)&DstA[x], a);
        _mm256_storeu_si256((__m256i*)&DstB[x], b);
        _mm256_storeu_si256((__m256i*)&DstC[x], c);
      }
      for(int32 x=Width16; x<Width8; x+=8)
      {
        //load
        __m128i abcd_0 = _mm_loadu_si128((__m128i*)&SrcABCD[(x<<2)+ 0]);
        __m128i abcd_1 = _mm_loadu_si128((__m128i*)&SrcABCD[(x<<2)+ 8]);
        __m128i abcd_2 = _mm_loadu_si128((__m128i*)&SrcABCD[(x<<2)+16]);
        __m128i abcd_3 = _mm_loadu_si128((__m128i*)&SrcABCD[(x<<2)+24]);

        //transpose
        __m128i ac_0 = _mm_unpacklo_epi16(abcd_0, abcd_1);
        __m128i ac_1 = _mm_unpackhi_epi16(abcd_0, abcd_1);
        __m128i bd_0 = _mm_unpacklo_epi16(abcd_2, abcd_3);
        __m128i bd_1 = _mm_unpackhi_epi16(abcd_2, abcd_3);

        __m128i a_0 = _mm_unpacklo_epi16(ac_0, ac_1);
        __m128i b_0 = _mm_unpackhi_epi16(ac_0, ac_1);
        __m128i c_0 = _mm_unpacklo_epi16(bd_0, bd_1);
        __m128i d_0 = _mm_unpackhi_epi16(bd_0, bd_1);

        __m128i a = _mm_unpacklo_epi64(a_0, c_0);
        __m128i b = _mm_unpackhi_epi64(a_0, c_0);
        __m128i c = _mm_unpacklo_epi64(b_0, d_0);

        //save
        _mm_storeu_si128((__m128i*)&DstA[x], a);
        _mm_storeu_si128((__m128i*)&DstB[x], b);
        _mm_storeu_si128((__m128i*)&DstC[x], c);
      }
      for(int32 x=Width8; x<Width4; x+=4)
      {
        //load
        __m128i abcd_0 = _mm_loadu_si128((__m128i*)&SrcABCD[(x<<2)+ 0]);
        __m128i abcd_1 = _mm_loadu_si128((__m128i*)&SrcABCD[(x<<2)+ 8]);

        //transpose
        __m128i aabbccdd_02 = _mm_unpacklo_epi16(abcd_0, abcd_1);
        __m128i aabbccdd_13 = _mm_unpackhi_epi16(abcd_0, abcd_1);

        __m128i aaaabbbb = _mm_unpacklo_epi16(aabbccdd_02, aabbccdd_13);
        __m128i ccccdddd = _mm_unpackhi_epi16(aabbccdd_02, aabbccdd_13);

        __m128i a = aaaabbbb;
        __m128i b = _mm_unpackhi_epi64(aaaabbbb, aaaabbbb);
        __m128i c = ccccdddd;

        //save
        _mm_storel_epi64((__m128i*)&DstA[x], a);
        _mm_storel_epi64((__m128i*)&DstB[x], b);
        _mm_storel_epi64((__m128i*)&DstC[x], c);
      }
      for(int32 x=Width4; x<Width; x++)
      {      
        int16 a = SrcABCD[(x<<2)+0];
        int16 b = SrcABCD[(x<<2)+1];
        int16 c = SrcABCD[(x<<2)+2];
        DstA[x] = a;
        DstB[x] = b;
        DstC[x] = c;
      }
      SrcABCD += SrcStride;
      DstA    += DstStride;
      DstB    += DstStride;
      DstC    += DstStride;
    }
  }
}
int32 xPixelOpsAVX::CountNonZero(const uint16* Src, int32 SrcStride, int32 Width, int32 Height)
{
  const __m256i ZeroV = _mm256_setzero_si256();
  const __m256i MaxV  = _mm256_set1_epi16((uint16)0xFFFF);
  int32 NumNonZero = 0;

  if(((uint32)Width & (uint32)c_RemainderMask32)==0) //Width%32==0
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=32)
      {
        __m256i CoeffsA = _mm256_loadu_si256((__m256i*)&Src[x   ]);
        __m256i CoeffsB = _mm256_loadu_si256((__m256i*)&Src[x+16]);
        __m256i MasksA  = _mm256_cmpeq_epi16(CoeffsA, ZeroV);
        __m256i MasksB  = _mm256_cmpeq_epi16(CoeffsB, ZeroV);
        __m256i Masks   = _mm256_packs_epi16(MasksA, MasksB);
        uint32 Mask     = (~_mm256_movemask_epi8(Masks)) & 0xFFFFFFFF;
        uint32 NumOnes  = _mm_popcnt_u32(Mask);
        NumNonZero += NumOnes;
      }
      Src += SrcStride;
    }
  }
  else
  {
    int32 Width32 = (int32)((uint32)Width & (uint32)c_MultipleMask32);
    int32 Width16 = (int32)((uint32)Width & (uint32)c_MultipleMask16);
    int32 Width8  = (int32)((uint32)Width & (uint32)c_MultipleMask8);
    int32 Width4  = (int32)((uint32)Width & (uint32)c_MultipleMask4);

    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width32; x+=32)
      {
        __m256i CoeffsA = _mm256_loadu_si256((__m256i*)&Src[x   ]);
        __m256i CoeffsB = _mm256_loadu_si256((__m256i*)&Src[x+16]);
        __m256i MasksA  = _mm256_cmpeq_epi16(CoeffsA, ZeroV);
        __m256i MasksB  = _mm256_cmpeq_epi16(CoeffsB, ZeroV);
        __m256i Masks   = _mm256_packs_epi16(MasksA, MasksB);
        uint32 Mask     = (~_mm256_movemask_epi8(Masks)) & 0xFFFFFFFF;
        uint32 NumOnes  = _mm_popcnt_u32(Mask);
        NumNonZero += NumOnes;
      }
      for(int32 x=Width32; x<Width16; x+=16)
      {
        __m256i Coeffs  = _mm256_loadu_si256((__m256i*)&Src[x  ]);
        __m256i MasksA  = _mm256_cmpeq_epi16(Coeffs, ZeroV);
        __m256i Masks   = _mm256_packs_epi16(MasksA, MaxV);
        uint32 Mask     = (~_mm256_movemask_epi8(Masks)) & 0xFFFFFFFF;
        uint32 NumOnes  = _mm_popcnt_u32(Mask);
        NumNonZero += NumOnes;
      }
      for(int32 x=Width16; x<Width8; x+=8)
      {
        __m128i Coeffs  = _mm_loadu_si128((__m128i*)&Src[x  ]);
        __m128i MasksA  = _mm_cmpeq_epi16(Coeffs, _mm256_castsi256_si128(ZeroV));
        __m128i Masks   = _mm_packs_epi16(MasksA, _mm256_castsi256_si128(MaxV ));
        uint32 Mask     = (~_mm_movemask_epi8(Masks)) & 0xFFFF;
        uint32 NumOnes  = _mm_popcnt_u32(Mask);
        NumNonZero += NumOnes;      }
      for(int32 x=Width8; x<Width4; x+=4)
      {
        __m128i Coeffs  = _mm_loadl_epi64((__m128i*)&Src[x  ]);
        __m128i MasksA  = _mm_cmpeq_epi16(Coeffs, _mm256_castsi256_si128(ZeroV));
        __m128i Masks   = _mm_packs_epi16(MasksA, _mm256_castsi256_si128(MaxV ));
        uint32 Mask     = (~_mm_movemask_epi8(Masks)) & 0xFFFF;
        uint32 NumOnes  = _mm_popcnt_u32(Mask);
        NumNonZero += NumOnes;
      }
      for(int32 x=Width4; x<Width; x++)
      {      
        if(Src[x]!=0) { NumNonZero++; }
      }
      Src += SrcStride;
    }
  }

  return NumNonZero;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_AVX
