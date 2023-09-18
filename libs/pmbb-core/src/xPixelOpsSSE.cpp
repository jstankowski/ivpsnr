/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xPixelOpsSSE.h"

#if X_SIMD_CAN_USE_SSE

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xPixelOpsSSE::Cvt(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask16)==0) //Width%16==0
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        __m128i SrcV  = _mm_loadu_si128((__m128i*)&(Src[x]));
        __m128i DstV1 = _mm_unpacklo_epi8(SrcV, _mm_setzero_si128());
        __m128i DstV2 = _mm_unpackhi_epi8(SrcV, _mm_setzero_si128());
        _mm_storeu_si128 ((__m128i*)(&(Dst[x  ])), DstV1);
        _mm_storeu_si128 ((__m128i*)(&(Dst[x+8])), DstV2);
      }
      Src += SrcStride;
      Dst += DstStride;    
    }
  }
  else
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16);
    const int32 Width8  = (int32)((uint32)Width & c_MultipleMask8 );
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width16; x+=16)
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
void xPixelOpsSSE::Cvt(uint8* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask16)==0) //Width%16==0
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        __m128i SrcV1 = _mm_loadu_si128((__m128i*)&(Src[x  ]));
        __m128i SrcV2 = _mm_loadu_si128((__m128i*)&(Src[x+8]));
        __m128i DstV  = _mm_packus_epi16(SrcV1, SrcV2);
        _mm_storeu_si128 ((__m128i*)(&(Dst[x])), DstV);
      }
      Src += SrcStride;
      Dst += DstStride;    
    }
  }
  else
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16);
    const int32 Width8  = (int32)((uint32)Width & c_MultipleMask8 );
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width16; x+=16)
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
void xPixelOpsSSE::Upsample(uint16* restrict Dst, const uint16* restrict Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  uint16* restrict DstL0 = Dst;
  uint16* restrict DstL1 = Dst + DstStride;  

  if(((uint32)DstWidth & c_RemainderMask16)==0) //Width%16==0
  {
    for(int32 y=0; y<DstHeight; y+=2)
    {
      for(int32 x=0; x<DstWidth; x+=16)
      {
        __m128i SrcV   = _mm_loadu_si128((__m128i*)&Src[x>>1]);
        __m128i LeftV  = _mm_unpacklo_epi16(SrcV, SrcV);
        __m128i RightV = _mm_unpackhi_epi16(SrcV, SrcV);
        _mm_storeu_si128((__m128i*)&DstL0[x  ], LeftV );
        _mm_storeu_si128((__m128i*)&DstL0[x+8], RightV);
        _mm_storeu_si128((__m128i*)&DstL1[x  ], LeftV );
        _mm_storeu_si128((__m128i*)&DstL1[x+8], RightV);
      }
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    }
  }
  else
  {
    const int32 Width16 = (int32)((uint32)DstWidth & c_MultipleMask16);
    const int32 Width8  = (int32)((uint32)DstWidth & c_MultipleMask8 );

    for(int32 y=0; y<DstHeight; y+=2)
    {
      for(int32 x=0; x<Width16; x+=16)
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
void xPixelOpsSSE::CvtUpsample(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  uint16 *restrict DstL0 = Dst;
  uint16 *restrict DstL1 = Dst + DstStride;

  if(((uint32)DstWidth & c_RemainderMask32)==0) //Width%32==0
  {
    for(int32 y=0; y<DstHeight; y+=2)
    {
      for(int32 x=0; x<DstWidth; x+=32)
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
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    }
  }
  else
  {
    const int32 Width32 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask32);
    const int32 Width16 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask16);

    for(int32 y=0; y<DstHeight; y+=2)
    {
      for(int32 x=0; x<Width32; x+=32)
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
bool xPixelOpsSSE::CheckValues(const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth)
{
  if(BitDepth == 16) { return true; }

  const int32   MaxValue  = xBitDepth2MaxValue(BitDepth);
  const __m128i MaxValueV = _mm_set1_epi16((int16)MaxValue);
  
  if(((uint32)Width & c_RemainderMask16) == 0) //Width%16==0 - fast path without tail
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 16)
      {
        __m128i SrcV1  = _mm_loadu_si128((__m128i*)&Src[x  ]);
        __m128i SrcV2  = _mm_loadu_si128((__m128i*)&Src[x+8]);
        __m128i MaskV1 = _mm_cmpgt_epi16(SrcV1, MaxValueV); //0 - <=, 0xFFFF - >
        __m128i MaskV2 = _mm_cmpgt_epi16(SrcV2, MaxValueV); //0 - <=, 0xFFFF - >
        __m128i Masks  = _mm_packs_epi16(MaskV1, MaskV2);
        uint32  Mask   = _mm_movemask_epi8(Masks) & 0xFFFF;
        if(Mask) { return false; }
      }
      Src += SrcStride;
    } //y
  }
  else
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16);
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 16)
      {
        __m128i SrcV1  = _mm_loadu_si128((__m128i*)&Src[x  ]);
        __m128i SrcV2  = _mm_loadu_si128((__m128i*)&Src[x+8]);
        __m128i MaskV1 = _mm_cmpgt_epi16(SrcV1, MaxValueV); //0 - <=, 0xFFFF - >
        __m128i MaskV2 = _mm_cmpgt_epi16(SrcV2, MaxValueV); //0 - <=, 0xFFFF - >
        __m128i Masks  = _mm_packs_epi16(MaskV1, MaskV2);
        uint32  Mask   = _mm_movemask_epi8(Masks) & 0xFFFF;
        if(Mask) { return false; }
      }
      for (int32 x = Width16; x < Width; x++)
      {
        if (Src[x] > MaxValue) { return false; }
      }
      Src += SrcStride;
    } //y
  }

  return true;
}
void xPixelOpsSSE::AOS4fromSOA3(uint16* restrict DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, const uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  const __m128i d = _mm_set1_epi16(ValueD);

  if(((uint32)Width & c_RemainderMask8) == 0) //Width%8==0 - fast path without tail
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 8)
      {
        //load
        __m128i a = _mm_loadu_si128((__m128i*) & SrcA[x]); //load A0-A7
        __m128i b = _mm_loadu_si128((__m128i*) & SrcB[x]); //load B0-B7
        __m128i c = _mm_loadu_si128((__m128i*) & SrcC[x]); //load C0-C7

        //transpose
        __m128i ac_0   = _mm_unpacklo_epi16(a   , c   );
        __m128i ac_1   = _mm_unpackhi_epi16(a   , c   );
        __m128i bd_0   = _mm_unpacklo_epi16(b   , d   );
        __m128i bd_1   = _mm_unpackhi_epi16(b   , d   );
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
      SrcA    += SrcStride;
      SrcB    += SrcStride;
      SrcC    += SrcStride;
      DstABCD += DstStride;
    }
  }
  else
  {
    const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8);
    const int32 Width4 = (int32)((uint32)Width & c_MultipleMask4);

    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width8; x += 8)
      {
        //load
        __m128i a = _mm_loadu_si128((__m128i*) & SrcA[x]); //load A0-A7
        __m128i b = _mm_loadu_si128((__m128i*) & SrcB[x]); //load B0-B7
        __m128i c = _mm_loadu_si128((__m128i*) & SrcC[x]); //load C0-C7

        //transpose
        __m128i ac_0   = _mm_unpacklo_epi16(a   , c   );
        __m128i ac_1   = _mm_unpackhi_epi16(a   , c   );
        __m128i bd_0   = _mm_unpacklo_epi16(b   , d   );
        __m128i bd_1   = _mm_unpackhi_epi16(b   , d   );
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
        __m128i ac_0 = _mm_unpacklo_epi16(a, c);
        __m128i bd_0 = _mm_unpacklo_epi16(b, d);

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
int32 xPixelOpsSSE::CountNonZero(const uint16* Src, int32 SrcStride, int32 Width, int32 Height)
{
  
  const __m128i ZeroV = _mm_setzero_si128();
  const __m128i MaxV  = _mm_set1_epi16((uint16)0xFFFF);
  int32  NumNonZero = 0;

  if(((uint32)Width & (uint32)c_RemainderMask16)==0) //Width%16==0
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        __m128i CoeffsA = _mm_loadu_si128((__m128i*)&Src[x  ]);
        __m128i CoeffsB = _mm_loadu_si128((__m128i*)&Src[x+8]);
        __m128i MasksA  = _mm_cmpeq_epi16(CoeffsA, ZeroV);
        __m128i MasksB  = _mm_cmpeq_epi16(CoeffsB, ZeroV);
        __m128i Masks   = _mm_packs_epi16(MasksA, MasksB);
        uint32 Mask     = (~_mm_movemask_epi8(Masks)) & 0xFFFF;
        uint32 NumOnes  = _mm_popcnt_u32(Mask);
        NumNonZero += NumOnes;
      }
      Src += SrcStride;
    }
  }
  else
  {
    int32 Width16 = (int32)((uint32)Width & c_MultipleMask16);
    int32 Width8  = (int32)((uint32)Width & c_MultipleMask8 );
    int32 Width4  = (int32)((uint32)Width & c_MultipleMask4 );

    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width16; x+=16)
      {
        __m128i CoeffsA = _mm_loadu_si128((__m128i*)&Src[x  ]);
        __m128i CoeffsB = _mm_loadu_si128((__m128i*)&Src[x+8]);
        __m128i MasksA  = _mm_cmpeq_epi16(CoeffsA, ZeroV);
        __m128i MasksB  = _mm_cmpeq_epi16(CoeffsB, ZeroV);
        __m128i Masks   = _mm_packs_epi16(MasksA, MasksB);
        uint32 Mask     = (~_mm_movemask_epi8(Masks)) & 0xFFFF;
        uint32 NumOnes  = _mm_popcnt_u32(Mask);
        NumNonZero += NumOnes;
      }
      for(int32 x=Width16; x<Width8; x+=8)
      {
        __m128i Coeffs  = _mm_loadu_si128((__m128i*)&Src[x  ]);
        __m128i MasksA  = _mm_cmpeq_epi16(Coeffs, ZeroV);
        __m128i Masks   = _mm_packs_epi16(MasksA, MaxV );
        uint32 Mask     = (~_mm_movemask_epi8(Masks)) & 0xFFFF;
        uint32 NumOnes  = _mm_popcnt_u32(Mask);
        NumNonZero += NumOnes;
      }
      for(int32 x=Width8; x<Width4; x+=4)
      {
        __m128i Coeffs  = _mm_loadl_epi64((__m128i*)&Src[x  ]);
        __m128i MasksA  = _mm_cmpeq_epi16(Coeffs, ZeroV);
        __m128i Masks   = _mm_packs_epi16(MasksA, MaxV );
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

#endif //X_SIMD_CAN_USE_SSE
