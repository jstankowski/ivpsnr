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
void xPixelOpsAVX::Upsample(uint16* restrict Dst, const uint16* restrict Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
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
void xPixelOpsAVX::CvtUpsample(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
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
