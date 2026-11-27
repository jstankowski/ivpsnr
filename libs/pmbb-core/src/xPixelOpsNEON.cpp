/*
    SPDX-FileCopyrightText: 2025 Patrycja Kaźmierczak <patrycja.kazmierczak@student.put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xPixelOpsNEON.h"

#if X_SIMD_CAN_USE_NEON

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xPixelOpsNEON::Cvt(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask16<uint32>)==0) //ifnotail
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        uint8x16_t SrcV   = vld1q_u8       (&Src[x]);
        
        uint16x8_t DstV1  = vmovl_u8  (vget_low_u8(SrcV));
        uint16x8_t DstV2  = vmovl_high_u8  (SrcV);

        vst1q_u16(&Dst[x  ], DstV1);
        vst1q_u16(&Dst[x+8], DstV2); //kolejnosc???

      }
      Src += SrcStride;
      Dst += DstStride;
    }
  } 
  else
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16<uint32>);
    const int32 Width8  = (int32)((uint32)Width & c_MultipleMask8 <uint32>);
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width16; x+=16)
      {
        uint8x16_t SrcV   = vld1q_u8       (&Src[x]);
        
        uint16x8_t DstV1  = vmovl_u8  (vget_low_u8(SrcV));
        uint16x8_t DstV2  = vmovl_high_u8  (SrcV);

        vst1q_u16(&Dst[x],  DstV1);
        vst1q_u16(&Dst[x+8],DstV2); //kolejnosc ok
      }
      for(int32 x=Width16; x<Width8; x+=8)
      {
        uint8x8_t SrcV    = vld1_u8   (&Src[x]);
        uint16x8_t DstV1 = vmovl_u8 (SrcV);
        vst1q_u16(&Dst[x], DstV1);
      }
      for(int32 x=Width8; x<Width; x++)
      {
        Dst[x] = (uint16)(Src[x]);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }
  return;
}
void xPixelOpsNEON::Cvt(uint8* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  if(((uint32)Width & c_RemainderMask16<uint32>)==0) //ifnotail
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        uint16x8_t SrcV1 = vld1q_u16 (&Src[x]);
        uint16x8_t SrcV2 = vld1q_u16 (&Src[x+8]);        
        uint8x16_t DstV  = vcombine_u8(vqmovn_u16(SrcV1), vqmovn_u16(SrcV2));
        vst1q_u8 (&Dst[x], DstV);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  } 
  else
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16<uint32>);
    const int32 Width8  = (int32)((uint32)Width & c_MultipleMask8 <uint32>);
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width16; x+=16)
      {
        uint16x8_t SrcV1 = vld1q_u16 (&Src[x]);
        uint16x8_t SrcV2 = vld1q_u16 (&Src[x+8]);
        
        uint8x16_t DstV =  vcombine_u8(vqmovn_u16(SrcV1), vqmovn_u16(SrcV2));

        vst1q_u8 (&Dst[x], DstV);

      }
      for(int32 x=Width16; x<Width8; x+=8)
      {
        uint16x8_t SrcV1 = vld1q_u16 (&Src[x]);
        uint8x8_t DstV   = vqmovn_u16(SrcV1);

        vst1_u8 (&Dst[x], DstV);
      }
      for(int32 x=Width8; x<Width; x++)
      {
        Dst[x] = (uint8)xClipU8<uint16>(Src[x]);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }
  return;
}
void xPixelOpsNEON::UpsampleHV(uint16* restrict Dst, const uint16* restrict Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  uint16* restrict DstL0 = Dst;
  uint16* restrict DstL1 = Dst + DstStride;

  if(((uint32)DstWidth & c_RemainderMask16<uint32>) == 0)//notail
  {
    for (int32 y=0; y<DstHeight; y+=2)
    {
      for (int32 x=0; x<DstWidth; x+=16)
      {
        uint16x8_t SrcV = vld1q_u16 (&Src[x>>1]);

        uint16x8x2_t DstV = vzipq_u16 (SrcV, SrcV);

        vst1q_u16 (&DstL0[x], DstV.val[0]);
        vst1q_u16 (&DstL0[x+8], DstV.val[1]);
        vst1q_u16 (&DstL1[x], DstV.val[0]);
        vst1q_u16 (&DstL1[x+8], DstV.val[1]);//kolejniosc????
      }
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    }
  }
  else
  {
    const int32 Width16 = (int32)((uint32)DstWidth & c_MultipleMask16<uint32>);
    const int32 Width8  = (int32)((uint32)DstWidth & c_MultipleMask8 <uint32>);
    for(int32 y=0; y<DstHeight; y+=2)
    {
      for(int32 x=0; x<Width16; x+=16)
      {
        uint16x8_t   SrcV = vld1q_u16 (&Src[x>>1]);
        uint16x8x2_t DstV = vzipq_u16 (SrcV, SrcV);
        vst1q_u16 (&DstL0[x], DstV.val[0]);
        vst1q_u16 (&DstL0[x+8], DstV.val[1]);
        vst1q_u16 (&DstL1[x], DstV.val[0]);
        vst1q_u16 (&DstL1[x+8], DstV.val[1]);
      }
      for(int32 x=Width16; x<Width8; x+=8)
      {
        uint16x4_t SrcV = vld1_u16 (&Src[x>>1]);

        uint16x4x2_t DstV = vzip_u16 (SrcV, SrcV);

        vst1_u16 (&DstL0[x], DstV.val[0]);
        vst1_u16 (&DstL0[x+4], DstV.val[1]);
        vst1_u16 (&DstL1[x], DstV.val[0]);
        vst1_u16 (&DstL1[x+4], DstV.val[1]);
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
  return;
}
void xPixelOpsNEON::DownsampleHV(uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  const uint16* restrict SrcL0 = Src;
  const uint16* restrict SrcL1 = Src + SrcStride;

  if(((uint32)DstWidth & c_RemainderMask8<uint32>) == 0)//notail
  {
    for (int32 y=0; y<DstHeight; y++)
    {
      for (int32 x=0; x<DstWidth; x+=8)
      {
        const int32 SrcX = x<<1;
        uint16x8_t TopLeftV   = vld1q_u16(&SrcL0[SrcX]); //16x8x2 caly top??
        uint16x8_t TopRightV  = vld1q_u16(&SrcL0[SrcX+8]);
        uint16x8_t BottomLeftV  = vld1q_u16(&SrcL1[SrcX]);
        uint16x8_t BottomRightV = vld1q_u16(&SrcL1[SrcX+8]);
        
        uint16x8_t Left = vshrq_n_u16(vaddq_u16(TopLeftV, BottomLeftV),1); //add and half
        uint16x8_t Right = vshrq_n_u16(vaddq_u16(TopRightV, BottomRightV),1);

        uint32x4_t LeftV = vpaddlq_u16(Left);
        uint32x4_t RightV = vpaddlq_u16(Right);
        uint32x4_t HLeftV = vshrq_n_u32(LeftV,1);
        uint32x4_t HRightV = vshrq_n_u32(RightV,1);
        
        uint16x8_t DstV = vcombine_u16(vqmovn_u32(HLeftV), vqmovn_u32(HRightV));
        vst1q_u16 (&Dst[x], DstV);
      }
      SrcL0 += (SrcStride << 1);
      SrcL1 += (SrcStride << 1);
      Dst   += DstStride;
    }
  }
  else
  {
    const int32 Width8  = (int32)((uint32)DstWidth & (uint32)c_MultipleMask8<uint32>);
    const int32 Width4  = (int32)((uint32)DstWidth & (uint32)c_MultipleMask4<uint32>);

    for(int32 y=0; y<DstHeight; y++)
    {
      for(int32 x=0; x<Width8; x+=8)
      {
        const int32 SrcX = x<<1;
        uint16x8_t TopLeftV   = vld1q_u16(&SrcL0[SrcX]); //16x8x2 caly top??
        uint16x8_t TopRightV  = vld1q_u16(&SrcL0[SrcX+8]);
        uint16x8_t BottomLeftV  = vld1q_u16(&SrcL1[SrcX]);
        uint16x8_t BottomRightV = vld1q_u16(&SrcL1[SrcX+8]);
        
        uint16x8_t Left = vshrq_n_u16(vaddq_u16(TopLeftV, BottomLeftV),1); //add and half
        uint16x8_t Right = vshrq_n_u16(vaddq_u16(TopRightV, BottomRightV),1);

        uint32x4_t LeftV = vpaddlq_u16(Left);
        uint32x4_t RightV = vpaddlq_u16(Right);
        uint32x4_t HLeftV = vshrq_n_u32(LeftV,1);
        uint32x4_t HRightV = vshrq_n_u32(RightV,1);
        
        uint16x8_t DstV = vcombine_u16(vqmovn_u32(HLeftV), vqmovn_u32(HRightV));
        vst1q_u16 (&Dst[x], DstV);
      }
      for(int32 x=Width8; x<Width4; x+=4)
      {
        const int32 SrcX = x<<1;
        uint16x8_t TopLeftV   = vld1q_u16(&SrcL0[SrcX]);
        uint16x8_t BottomLeftV  = vld1q_u16(&SrcL1[SrcX]);
        
        uint16x8_t Left = vshrq_n_u16(vaddq_u16(TopLeftV, BottomLeftV),1); //add and half

        uint32x4_t LeftV = vpaddlq_u16(Left); //add pairwise
        uint32x4_t HLeftV = vshrq_n_u32(LeftV,1); //half
        
        uint16x4_t DstV = vqmovn_u32(HLeftV);
        vst1_u16 (&Dst[x], DstV);
      }
      for(int32 x=Width4; x<DstWidth; x++)
      {      
        const int32 SrcX = x<<1;
        int16 D = ((int32)SrcL0[SrcX  ] + (int32)SrcL0[SrcX+1] + (int32)SrcL1[SrcX  ] + (int32)SrcL1[SrcX+1] + 2)>>2;
        Dst[x] = D;
      }
      SrcL0 += (SrcStride << 1);
      SrcL1 += (SrcStride << 1);
      Dst   += DstStride;
    }
  }
  return;
}
void xPixelOpsNEON::CvtUpsampleHV(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  uint16 *restrict DstL0 = Dst;
  uint16 *restrict DstL1 = Dst + DstStride;
  if(((uint32)DstWidth & c_RemainderMask32<uint32>)==0) //Width%32==0
  {
    for(int32 y=0; y<DstHeight; y+=2)
    {
      for(int32 x=0; x<DstWidth; x+=32)
      {
        uint8x16_t SrcV = vld1q_u8(&Src[x>>1]);
        uint16x8_t SrcVL = vmovl_u8(vget_low_u8(SrcV));
        uint16x8_t SrcVH = vmovl_high_u8(SrcV);

        uint16x8x2_t DstVL = vzipq_u16 (SrcVL, SrcVL); //interleaving
        uint16x8x2_t DstVH = vzipq_u16 (SrcVH, SrcVH); 

        vst1q_u16 (&DstL0[x], DstVL.val[0]);
        vst1q_u16 (&DstL0[x+8], DstVL.val[1]);
        vst1q_u16 (&DstL0[x+16], DstVH.val[0]);
        vst1q_u16 (&DstL0[x+24], DstVH.val[1]);
        vst1q_u16 (&DstL1[x], DstVL.val[0]);
        vst1q_u16 (&DstL1[x+8], DstVL.val[1]);
        vst1q_u16 (&DstL1[x+16], DstVH.val[0]);
        vst1q_u16 (&DstL1[x+24], DstVH.val[1]);

      }
      Src   += SrcStride;
      DstL0 += (DstStride << 1);
      DstL1 += (DstStride << 1);
    }
  }
  else
  {
    const int32 Width32 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask32<uint32>);
    const int32 Width16 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask16<uint32>);

    for(int32 y=0; y<DstHeight; y+=2)
    {
      for(int32 x=0; x<Width32; x+=32)
      {
        uint8x16_t SrcV = vld1q_u8(&Src[x>>1]);
        uint16x8_t SrcVL = vmovl_u8(vget_low_u8(SrcV));
        uint16x8_t SrcVH = vmovl_high_u8(SrcV);

        uint16x8x2_t DstVL = vzipq_u16 (SrcVL, SrcVL); //interleaving
        uint16x8x2_t DstVH = vzipq_u16 (SrcVH, SrcVH); 

        vst1q_u16 (&DstL0[x], DstVL.val[0]);
        vst1q_u16 (&DstL0[x+8], DstVL.val[1]);
        vst1q_u16 (&DstL0[x+16], DstVH.val[0]);
        vst1q_u16 (&DstL0[x+24], DstVH.val[1]);
        vst1q_u16 (&DstL1[x], DstVL.val[0]);
        vst1q_u16 (&DstL1[x+8], DstVL.val[1]);
        vst1q_u16 (&DstL1[x+16], DstVH.val[0]);
        vst1q_u16 (&DstL1[x+24], DstVH.val[1]);
      }
      for(int32 x=Width32; x<Width16; x+=16)
      {
        uint8x8_t SrcV = vld1_u8(&Src[x>>1]);
        uint16x8_t SrcVL = vmovl_u8(SrcV);

        uint16x8x2_t DstVL = vzipq_u16 (SrcVL, SrcVL); //interleaving

        vst1q_u16 (&DstL0[x], DstVL.val[0]);
        vst1q_u16 (&DstL0[x+8], DstVL.val[1]);
        vst1q_u16 (&DstL1[x], DstVL.val[0]);
        vst1q_u16 (&DstL1[x+8], DstVL.val[1]);
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
  return;
}
void xPixelOpsNEON::CvtDownsampleHV(uint8* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  const uint16* restrict SrcL0 = Src;
  const uint16* restrict SrcL1 = Src + SrcStride;

  if(((uint32)DstWidth & c_RemainderMask16<uint32>) == 0) //Width%16==0
  {
    for(int32 y = 0; y < DstHeight; y++)
    {
      for(int32 x = 0; x < DstWidth; x += 16)
      {
        const int32 SrcX = x<<1;
        uint16x8_t TopA = vld1q_u16(&SrcL0[SrcX]);
        uint16x8_t TopB = vld1q_u16(&SrcL0[SrcX+8]);
        uint16x8_t TopC = vld1q_u16(&SrcL0[SrcX+16]);
        uint16x8_t TopD = vld1q_u16(&SrcL0[SrcX+24]);

        uint16x8_t BotA = vld1q_u16(&SrcL0[SrcX]);
        uint16x8_t BotB = vld1q_u16(&SrcL0[SrcX+8]);
        uint16x8_t BotC = vld1q_u16(&SrcL0[SrcX+16]);
        uint16x8_t BotD = vld1q_u16(&SrcL0[SrcX+24]);

        uint16x8_t A = vshrq_n_u16(vaddq_u16(TopA, BotA),1); //add and half
        uint16x8_t B = vshrq_n_u16(vaddq_u16(TopB, BotB),1);
        uint16x8_t C = vshrq_n_u16(vaddq_u16(TopC, BotC),1);
        uint16x8_t D = vshrq_n_u16(vaddq_u16(TopD, BotD),1);

        uint32x4_t A32 = vshrq_n_u32(vpaddlq_u16(A),1); //add pairwise and half
        uint32x4_t B32 = vshrq_n_u32(vpaddlq_u16(B),1);
        uint32x4_t C32 = vshrq_n_u32(vpaddlq_u16(C),1);
        uint32x4_t D32 = vshrq_n_u32(vpaddlq_u16(D),1);

        uint16x8_t AB = vcombine_u16(vqmovn_u32(A32), vqmovn_u32(B32));
        uint16x8_t CD = vcombine_u16(vqmovn_u32(C32), vqmovn_u32(D32));

        uint8x16_t DstV = vcombine_u8(vqmovn_u16(AB), vqmovn_u16(CD));

        vst1q_u8 (&Dst[x], DstV);
      }
      SrcL0 += (SrcStride << 1);
      SrcL1 += (SrcStride << 1);
      Dst   += DstStride;
    }
  }
  else
  {
    const int32 Width16 = (int32)((uint32)DstWidth & c_MultipleMask16<uint32>);
    const int32 Width8  = (int32)((uint32)DstWidth & c_MultipleMask8 <uint32>);

    for(int32 y = 0; y < DstHeight; y++)
    {
      for(int32 x = 0; x < Width16; x += 16)
      {
        const int32 SrcX = x<<1;
        uint16x8_t TopA = vld1q_u16(&SrcL0[SrcX]);
        uint16x8_t TopB = vld1q_u16(&SrcL0[SrcX+8]);
        uint16x8_t TopC = vld1q_u16(&SrcL0[SrcX+16]);
        uint16x8_t TopD = vld1q_u16(&SrcL0[SrcX+24]);

        uint16x8_t BotA = vld1q_u16(&SrcL0[SrcX]);
        uint16x8_t BotB = vld1q_u16(&SrcL0[SrcX+8]);
        uint16x8_t BotC = vld1q_u16(&SrcL0[SrcX+16]);
        uint16x8_t BotD = vld1q_u16(&SrcL0[SrcX+24]);

        uint16x8_t A = vshrq_n_u16(vaddq_u16(TopA, BotA),1); //add and half
        uint16x8_t B = vshrq_n_u16(vaddq_u16(TopB, BotB),1);
        uint16x8_t C = vshrq_n_u16(vaddq_u16(TopC, BotC),1);
        uint16x8_t D = vshrq_n_u16(vaddq_u16(TopD, BotD),1);

        uint32x4_t A32 = vshrq_n_u32(vpaddlq_u16(A),1); //add pairwise and half
        uint32x4_t B32 = vshrq_n_u32(vpaddlq_u16(B),1);
        uint32x4_t C32 = vshrq_n_u32(vpaddlq_u16(C),1);
        uint32x4_t D32 = vshrq_n_u32(vpaddlq_u16(D),1);

        uint16x8_t AB = vcombine_u16(vqmovn_u32(A32), vqmovn_u32(B32));
        uint16x8_t CD = vcombine_u16(vqmovn_u32(C32), vqmovn_u32(D32));

        uint8x16_t DstV = vcombine_u8(vqmovn_u16(AB), vqmovn_u16(CD));

        vst1q_u8 (&Dst[x], DstV);
      }
      for(int32 x = Width16; x < Width8; x += 8)
      {
        const int32 SrcX = x<<1;
        uint16x8_t TopA = vld1q_u16(&SrcL0[SrcX]);
        uint16x8_t TopB = vld1q_u16(&SrcL0[SrcX+8]);

        uint16x8_t BotA = vld1q_u16(&SrcL0[SrcX]);
        uint16x8_t BotB = vld1q_u16(&SrcL0[SrcX+8]);

        uint16x8_t A = vshrq_n_u16(vaddq_u16(TopA, BotA),1); //add and half
        uint16x8_t B = vshrq_n_u16(vaddq_u16(TopB, BotB),1);

        uint32x4_t A32 = vshrq_n_u32(vpaddlq_u16(A),1); //add pairwise and half
        uint32x4_t B32 = vshrq_n_u32(vpaddlq_u16(B),1);

        uint16x8_t AB = vcombine_u16(vqmovn_u32(A32), vqmovn_u32(B32));

        uint8x8_t DstV = vqmovn_u16(AB);

        vst1_u8 (&Dst[x], DstV);
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
  return;
}
void xPixelOpsNEON::UpsampleH(uint16* restrict Dst, const uint16* restrict Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  if(((uint32)DstWidth & c_RemainderMask16<uint32>)==0) //Width%16==0
  {
    for(int32 y=0; y<DstHeight; y++)
    {
      for(int32 x=0; x<DstWidth; x+=16)
      {
        uint16x8_t SrcV = vld1q_u16(&Src[x>>1]);
        uint16x8x2_t DstV = vzipq_u16 (SrcV, SrcV);

        vst1q_u16 (&Dst[x], DstV.val[0]);
        vst1q_u16 (&Dst[x+8], DstV.val[1]);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }
  else
  {
    const int32 Width16 = (int32)((uint32)DstWidth & c_MultipleMask16<uint32>);
    const int32 Width8  = (int32)((uint32)DstWidth & c_MultipleMask8 <uint32>);

    for(int32 y=0; y<DstHeight; y++)
    {
      for(int32 x=0; x<Width16; x+=16)
      {
        uint16x8_t SrcV = vld1q_u16(&Src[x>>1]);
        uint16x8x2_t DstV = vzipq_u16 (SrcV, SrcV);

        vst1q_u16 (&Dst[x], DstV.val[0]);
        vst1q_u16 (&Dst[x+8], DstV.val[1]);
      }
      for(int32 x=Width16; x<Width8; x+=8)
      {
        uint16x4_t SrcV = vld1_u16(&Src[x>>1]);
        uint16x4x2_t DstV = vzip_u16 (SrcV, SrcV);

        vst1_u16 (&Dst[x], DstV.val[0]);
        vst1_u16 (&Dst[x+4], DstV.val[1]);
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
  return;
}
void xPixelOpsNEON::DownsampleH(uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  if(((uint32)DstWidth & (uint32)c_RemainderMask8<uint32>)==0) //Width%8==0
  {
    for(int32 y=0; y<DstHeight; y++)
    {
      for(int32 x=0; x<DstWidth; x+=8)
      {
        const int32 SrcX = x<<1;
        uint16x8_t LeftV = vld1q_u16(&Src[SrcX]);
        uint16x8_t RightV = vld1q_u16(&Src[SrcX+8]);
        uint32x4_t Left32 = vshrq_n_u32(vpaddlq_u16(LeftV),1);
        uint32x4_t Right32 = vshrq_n_u32(vpaddlq_u16(RightV),1);  //rounding??

        uint16x8_t DstV = vcombine_u16(vqmovn_u32(Left32), vqmovn_u32(Right32));

        vst1q_u16(&Dst[x], DstV);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }  
  else
  {
    const int32 Width8  = (int32)((uint32)DstWidth & (uint32)c_MultipleMask8<uint32>);
    const int32 Width4  = (int32)((uint32)DstWidth & (uint32)c_MultipleMask4<uint32>);

    for(int32 y=0; y<DstHeight; y++)
    {
      for(int32 x=0; x<Width8; x+=8)
      {
        const int32 SrcX = x<<1;
        uint16x8_t LeftV = vld1q_u16(&Src[SrcX]);
        uint16x8_t RightV = vld1q_u16(&Src[SrcX+8]);
        uint32x4_t Left32 = vshrq_n_u32(vpaddlq_u16(LeftV),1);
        uint32x4_t Right32 = vshrq_n_u32(vpaddlq_u16(RightV),1);

        uint16x8_t DstV = vcombine_u16(vqmovn_u32(Left32), vqmovn_u32(Right32));
        vst1q_u16(&Dst[x], DstV);
      }
      for(int32 x=Width8; x<Width4; x+=4)
      {
        const int32 SrcX = x<<1;
        uint16x4_t LeftV = vld1_u16(&Src[SrcX]);
        uint16x4_t RightV = vld1_u16(&Src[SrcX+4]);
        uint16x4_t DstV = vshr_n_u16(vpadd_u16(LeftV, RightV),1);

        vst1_u16(&Dst[x], DstV);

      }
      for(int32 x=Width4; x<DstWidth; x++)
      {      
        const int32 SrcX = x<<1;
        int16 D = ((int32)Src[SrcX  ] + (int32)Src[SrcX+1] + 1)>>1;
        Dst[x] = D;
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }
  return;
}
void xPixelOpsNEON::CvtUpsampleH(uint16* restrict Dst, const uint8* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  if(((uint32)DstWidth & c_RemainderMask32<uint32>)==0) //Width%32==0
  {
    for(int32 y=0; y<DstHeight; y++)
    {
      for(int32 x=0; x<DstWidth; x+=32)
      {
        uint8x16_t SrcV = vld1q_u8(&Src[x>>1]);
        uint16x8_t SrcVL = vmovl_u8(vget_low_u8(SrcV));
        uint16x8_t SrcVH = vmovl_high_u8(SrcV);

        uint16x8x2_t DstVL = vzipq_u16 (SrcVL, SrcVL);
        uint16x8x2_t DstVH = vzipq_u16 (SrcVH, SrcVH);

        vst1q_u16(&Dst[x],      DstVL.val[0]);
        vst1q_u16(&Dst[x + 8],  DstVL.val[1]);
        vst1q_u16(&Dst[x + 16], DstVH.val[0]);
        vst1q_u16(&Dst[x + 24], DstVH.val[1]);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }
  else
  {
    const int32 Width32 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask32<uint32>);
    const int32 Width16 = (int32)((uint32)DstWidth & (uint32)c_MultipleMask16<uint32>);

    for(int32 y=0; y<DstHeight; y++)
    {
      for(int32 x=0; x<Width32; x+=32)
      {
        uint8x16_t SrcV = vld1q_u8(&Src[x>>1]);
        uint16x8_t SrcVL = vmovl_u8(vget_low_u8(SrcV));
        uint16x8_t SrcVH = vmovl_high_u8(SrcV);

        uint16x8x2_t DstVL = vzipq_u16 (SrcVL, SrcVL);
        uint16x8x2_t DstVH = vzipq_u16 (SrcVH, SrcVH);

        vst1q_u16(&Dst[x],      DstVL.val[0]);
        vst1q_u16(&Dst[x + 8],  DstVL.val[1]);
        vst1q_u16(&Dst[x + 16], DstVH.val[0]);
        vst1q_u16(&Dst[x + 24], DstVH.val[1]);
      }
      for(int32 x=Width32; x<Width16; x+=16)
      {
        uint8x8_t SrcV = vld1_u8(&Src[x>>1]);
        uint16x8_t SrcVL = vmovl_u8(SrcV);

        uint16x8x2_t DstVL = vzipq_u16 (SrcVL, SrcVL);

        vst1q_u16 (&Dst[x], DstVL.val[0]);
        vst1q_u16 (&Dst[x+8], DstVL.val[1]);
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
  return;
}
void xPixelOpsNEON::CvtDownsampleH(uint8* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight)
{
  if(((uint32)DstWidth & (uint32)c_RemainderMask8<uint32>)==0) //Width%8==0
  {
    for(int32 y=0; y<DstHeight; y++)
    {
      for(int32 x=0; x<DstWidth; x+=16)
      {
        const int32 SrcX = x<<1;
        uint16x8_t A = vld1q_u16(&Src[SrcX]);
        uint16x8_t B = vld1q_u16(&Src[SrcX+8]);
        uint16x8_t C = vld1q_u16(&Src[SrcX+16]);
        uint16x8_t D = vld1q_u16(&Src[SrcX+24]);

        uint32x4_t A32 = vshrq_n_u32(vpaddlq_u16(A),1); //add pairwise and half
        uint32x4_t B32 = vshrq_n_u32(vpaddlq_u16(B),1);
        uint32x4_t C32 = vshrq_n_u32(vpaddlq_u16(C),1);
        uint32x4_t D32 = vshrq_n_u32(vpaddlq_u16(D),1);

        uint16x8_t AB = vcombine_u16(vqmovn_u32(A32), vqmovn_u32(B32));
        uint16x8_t CD = vcombine_u16(vqmovn_u32(C32), vqmovn_u32(D32));

        uint8x16_t DstV = vcombine_u8(vqmovn_u16(AB), vqmovn_u16(CD));

        vst1q_u8 (&Dst[x], DstV);
      }
      Src += SrcStride;
      Dst += DstStride;
    }
  }  
  else
  {
    const int32 Width16 = (int32)((uint32)DstWidth & c_MultipleMask16<uint32>);
    const int32 Width8  = (int32)((uint32)DstWidth & c_MultipleMask8 <uint32>);

    for(int32 y = 0; y < DstHeight; y++)
    {
      for(int32 x = 0; x < Width16; x += 16)
      {
        const int32 SrcX = x<<1;
        uint16x8_t A = vld1q_u16(&Src[SrcX]);
        uint16x8_t B = vld1q_u16(&Src[SrcX+8]);
        uint16x8_t C = vld1q_u16(&Src[SrcX+16]);
        uint16x8_t D = vld1q_u16(&Src[SrcX+24]);

        uint32x4_t A32 = vshrq_n_u32(vpaddlq_u16(A),1); //add pairwise and half
        uint32x4_t B32 = vshrq_n_u32(vpaddlq_u16(B),1);
        uint32x4_t C32 = vshrq_n_u32(vpaddlq_u16(C),1);
        uint32x4_t D32 = vshrq_n_u32(vpaddlq_u16(D),1);

        uint16x8_t AB = vcombine_u16(vqmovn_u32(A32), vqmovn_u32(B32));
        uint16x8_t CD = vcombine_u16(vqmovn_u32(C32), vqmovn_u32(D32));

        uint8x16_t DstV = vcombine_u8(vqmovn_u16(AB), vqmovn_u16(CD));

        vst1q_u8 (&Dst[x], DstV);
      }
      for(int32 x = Width16; x < Width8; x += 8)
      {
        const int32 SrcX = x<<1;
        uint16x8_t A = vld1q_u16(&Src[SrcX]);
        uint16x8_t B = vld1q_u16(&Src[SrcX+8]);

        uint32x4_t A32 = vshrq_n_u32(vpaddlq_u16(A),1); //add pairwise and half
        uint32x4_t B32 = vshrq_n_u32(vpaddlq_u16(B),1);

        uint16x8_t AB = vcombine_u16(vqmovn_u32(A32), vqmovn_u32(B32));

        uint8x8_t DstV = vqmovn_u16(AB);

        vst1_u8 (&Dst[x], DstV);
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
  return;
}
bool xPixelOpsNEON::CheckIfInRange(const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth)
{
  if(BitDepth == 16) { return true; }

  const int32   MaxValue  = xBitDepth2MaxValue(BitDepth);
  const int16x8_t MaxValueV = vdupq_n_s16((int16)MaxValue);
  
  if(((uint32)Width & c_RemainderMask16<uint32>) == 0) //Width%16==0 - fast path without tail
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 16)
      {
        uint16x8_t SrcV1 = vld1q_u16(&Src[x]);
        uint16x8_t SrcV2 = vld1q_u16(&Src[x+8]);

        uint16x8_t CmpV1 = vcgtq_u16(SrcV1, vreinterpretq_u16_s16(MaxValueV));
        uint16x8_t CmpV2 = vcgtq_u16(SrcV2, vreinterpretq_u16_s16(MaxValueV));

        uint16x8_t CombinedCmp = vorrq_u16(CmpV1, CmpV2);

        uint8 result = vmaxvq_u16(CombinedCmp);

        if (result) { return false; }
      }
      Src += SrcStride;
    } //y
  }
  else
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16<uint32>);
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width16; x += 16)
      {
        uint16x8_t SrcV1 = vld1q_u16(&Src[x]);
        uint16x8_t SrcV2 = vld1q_u16(&Src[x+8]);

        uint16x8_t CmpV1 = vcgtq_u16(SrcV1, vreinterpretq_u16_s16(MaxValueV));
        uint16x8_t CmpV2 = vcgtq_u16(SrcV2, vreinterpretq_u16_s16(MaxValueV));

        uint16x8_t CombinedCmp = vorrq_u16(CmpV1, CmpV2);

        uint8 result = vmaxvq_u16(CombinedCmp);

        if (result) { return false; }
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
void xPixelOpsNEON::AOS4fromSOA3(uint16* restrict DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, const uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  const uint16x8_t d = vdupq_n_u16(ValueD);

  if(((uint32)Width & c_RemainderMask8<uint32>) == 0) //Width%8==0 - fast path without tail
  {
    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x += 8)
      {
        //load
        uint16x8_t a = vld1q_u16(& SrcA[x]); //load A0-A7
        uint16x8_t b = vld1q_u16(& SrcB[x]); //load B0-B7
        uint16x8_t c = vld1q_u16(& SrcC[x]); //load C0-C7

        //transpose
        uint16x8x2_t ac = vzipq_u16(a,c);
        uint16x8x2_t bd = vzipq_u16(b,d);

        uint16x8x2_t abcd0 = vzipq_u16 (ac.val[0], bd.val[0]);
        uint16x8x2_t abcd1 = vzipq_u16 (ac.val[1], bd.val[1]);

        //save
        vst1q_u16(&DstABCD[(x << 2) +  0], abcd0.val[0]);
        vst1q_u16(&DstABCD[(x << 2) +  8], abcd0.val[1]);
        vst1q_u16(&DstABCD[(x << 2) + 16], abcd1.val[0]);
        vst1q_u16(&DstABCD[(x << 2) + 24], abcd1.val[1]);
      }
      SrcA    += SrcStride;
      SrcB    += SrcStride;
      SrcC    += SrcStride;
      DstABCD += DstStride;
    }
  }
  else
  {
    const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8<uint32>);
    const int32 Width4 = (int32)((uint32)Width & c_MultipleMask4<uint32>);

    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width8; x += 8)
      {
        //load
        uint16x8_t a = vld1q_u16(& SrcA[x]); //load A0-A7
        uint16x8_t b = vld1q_u16(& SrcB[x]); //load B0-B7
        uint16x8_t c = vld1q_u16(& SrcC[x]); //load C0-C7

        //transpose
        uint16x8x2_t ac = vzipq_u16(a,c);
        uint16x8x2_t bd = vzipq_u16(b,d);

        uint16x8x2_t abcd0 = vzipq_u16 (ac.val[0], bd.val[0]);
        uint16x8x2_t abcd1 = vzipq_u16 (ac.val[1], bd.val[1]);

        //save
        vst1q_u16(&DstABCD[(x << 2) +  0], abcd0.val[0]);
        vst1q_u16(&DstABCD[(x << 2) +  8], abcd0.val[1]);
        vst1q_u16(&DstABCD[(x << 2) + 16], abcd1.val[0]);
        vst1q_u16(&DstABCD[(x << 2) + 24], abcd1.val[1]);
      }
      for(int32 x = Width8; x < Width4; x += 4)
      {
        //load
        uint16x4_t a = vld1_u16(& SrcA[x]); //load A0-A3
        uint16x4_t b = vld1_u16(& SrcB[x]); //load B0-B3
        uint16x4_t c = vld1_u16(& SrcC[x]); //load C0-C3

        //transpose
        uint16x4x2_t ac = vzip_u16(a,c);
        uint16x4x2_t bd = vzip_u16(b, vget_low_u16(d));
        uint16x4x2_t abcd0 = vzip_u16 (ac.val[0], bd.val[0]);
        uint16x4x2_t abcd1 = vzip_u16 (ac.val[1], bd.val[1]);

        //save
        vst1_u16(&DstABCD[(x << 2) +  0], abcd0.val[0]);
        vst1_u16(&DstABCD[(x << 2) +  4], abcd0.val[1]);
        vst1_u16(&DstABCD[(x << 2) +  8], abcd1.val[0]);
        vst1_u16(&DstABCD[(x << 2) + 12], abcd1.val[1]);
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
  return;
}
void xPixelOpsNEON::SOA3fromAOS4(uint16* restrict DstA, uint16* restrict DstB, uint16* restrict DstC, const uint16* SrcABCD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  if(((uint32)Width & (uint32)c_RemainderMask8<uint32>)==0) //Width%8==0
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=8)
      {
        //load x4
        uint16x8x4_t abcd = vld4q_u16(&SrcABCD[(x << 2)]);

        //store every 4th
        vst1q_u16(&DstA[x], abcd.val[0]); // A0-A7
        vst1q_u16(&DstB[x], abcd.val[1]); // B0-B7  
        vst1q_u16(&DstC[x], abcd.val[2]); // C0-C7

        /*
        //load
        uint16x8x2_t abcd0 = vld1q_u16_x2(&SrcABCD[(x<<2)]);
        uint16x8x2_t abcd1 = vld1q_u16_x2(&SrcABCD[(x<<2)+16]);
        //transpose
        uint16x8_t aabbccdd00 = vzip1q_u16(abcd0.val[0], abcd0.val[1]);
        uint16x8_t aabbccdd01 = vzip2q_u16(abcd0.val[0], abcd0.val[1]);
        uint16x8_t aabbccdd10 = vzip1q_u16(abcd1.val[0], abcd1.val[1]);
        uint16x8_t aabbccdd11 = vzip2q_u16(abcd1.val[0], abcd1.val[1]);
        uint16x8x2_t a4b4xc4d40 = vzipq_u16(aabbccdd00, aabbccdd01);
        uint16x8x2_t a4b4xc4d41 = vzipq_u16(aabbccdd10, aabbccdd11);
        uint16x8_t ax8 = vzip1q_u16(a4b4xc4d40.val[0], a4b4xc4d41.val[0]);
        uint16x8_t bx8 = vzip2q_u16(a4b4xc4d40.val[0], a4b4xc4d41.val[0]);
        uint16x8_t cx8 = vzip1q_u16(a4b4xc4d40.val[1], a4b4xc4d41.val[1]);
        //save
        vst1q_u16(&DstA[x], ax8);
        vst1q_u16(&DstB[x], bx8);
        vst1q_u16(&DstC[x], cx8);
        */
      }
      SrcABCD += SrcStride;
      DstA    += DstStride;
      DstB    += DstStride;
      DstC    += DstStride;
    }
  }
  else
  {
    int32 Width8  = (int32)((uint32)Width & (uint32)c_MultipleMask8<uint32>);
    int32 Width4  = (int32)((uint32)Width & (uint32)c_MultipleMask4<uint32>);

    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width8; x+=8)
      {
        //load
        uint16x8x4_t abcd = vld4q_u16(&SrcABCD[(x << 2)]);
        //store
        vst1q_u16(&DstA[x], abcd.val[0]); // A0-A7
        vst1q_u16(&DstB[x], abcd.val[1]); // B0-B7  
        vst1q_u16(&DstC[x], abcd.val[2]); // C0-C7

      }
      for(int32 x=Width8; x<Width4; x+=4)
      {
        //load
        uint16x4x4_t abcd = vld4_u16(&SrcABCD[(x << 2)]);
        //store
        vst1_u16(&DstA[x], abcd.val[0]);
        vst1_u16(&DstB[x], abcd.val[1]);
        vst1_u16(&DstC[x], abcd.val[2]);

      }
      for(int32 x=Width4; x<Width; x++)
      {      
        uint16 a = SrcABCD[(x<<2)+0];
        uint16 b = SrcABCD[(x<<2)+1];
        uint16 c = SrcABCD[(x<<2)+2];
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
int32 xPixelOpsNEON::CountNonZero(const uint16* Src, int32 SrcStride, int32 Width, int32 Height)
{
  int32_t NumNonZero = 0;

  if(((uint32)Width & (uint32)c_RemainderMask16<uint32>)==0)
  {
    for (int32_t y = 0; y < Height; ++y)
    {
      for (int32_t x = 0; x < Width; x += 16)
      {
        uint16x8_t SrcV1 = vld1q_u16(&Src[x]);
        uint16x8_t SrcV2 = vld1q_u16(&Src[x+8]);

        uint16x8_t CmpV1 = vtstq_u16(SrcV1, SrcV1);
        uint16x8_t CmpV2 = vtstq_u16(SrcV2, SrcV2);
        CmpV1 = vandq_u16(CmpV1, vdupq_n_u16(1));
        CmpV2 = vandq_u16(CmpV2, vdupq_n_u16(1));

        NumNonZero += vaddlvq_u16(CmpV1);
        NumNonZero += vaddlvq_u16(CmpV2);
      }
      Src += SrcStride;
    }
  }
  else
  {
    int32 Width16 = (int32)((uint32)Width & c_MultipleMask16<uint32>);
    int32 Width8  = (int32)((uint32)Width & c_MultipleMask8 <uint32>);
    int32 Width4  = (int32)((uint32)Width & c_MultipleMask4 <uint32>);

    for (int32_t y = 0; y < Height; ++y)
    {
      for (int32_t x = 0; x < Width16; x += 16)
      {
        uint16x8_t SrcV1 = vld1q_u16(&Src[x]);
        uint16x8_t SrcV2 = vld1q_u16(&Src[x+8]);
        uint16x8_t CmpV1 = vtstq_u16(SrcV1, SrcV1);
        uint16x8_t CmpV2 = vtstq_u16(SrcV2, SrcV2);
        CmpV1 = vandq_u16(CmpV1, vdupq_n_u16(1));
        CmpV2 = vandq_u16(CmpV2, vdupq_n_u16(1));

        NumNonZero += vaddlvq_u16(CmpV1);
        NumNonZero += vaddlvq_u16(CmpV2);
      }
      for (int32_t x = Width16; x < Width8; x += 8)
      {
        uint16x8_t SrcV1 = vld1q_u16(&Src[x]);
        uint16x8_t CmpV1 = vtstq_u16(SrcV1, SrcV1);
        CmpV1 = vandq_u16(CmpV1, vdupq_n_u16(1));

        NumNonZero += vaddlvq_u16(CmpV1);
      }
      for (int32_t x = Width8; x < Width4; x += 4)
      {
        uint16x4_t SrcV1 = vld1_u16(&Src[x]);
        uint16x4_t CmpV1 = vtst_u16(SrcV1, SrcV1);
        CmpV1 = vand_u16(CmpV1, vdup_n_u16(1));

        NumNonZero += vaddlv_u16(CmpV1);
      }
      for (int32_t x = Width4; x < Width; ++x)
      {
        if(Src[x]!=0) { NumNonZero++; }
      }

      Src += SrcStride;
    }
  }
  return NumNonZero;
}
bool xPixelOpsNEON::CompareEqual(const uint16* Tst, const uint16* Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  if (((uint32)Width & c_RemainderMask16<uint32>) == 0) // Width % 16 == 0
  {
    for (int32 y = 0; y < Height; ++y)
    {
      for (int32 x = 0; x < Width; x += 16)
      {
        uint16x8_t TstV1 = vld1q_u16(&Tst[x]);
        uint16x8_t TstV2 = vld1q_u16(&Tst[x + 8]);
        uint16x8_t RefV1 = vld1q_u16(&Ref[x]);
        uint16x8_t RefV2 = vld1q_u16(&Ref[x + 8]);

        uint16x8_t CmpV1 = vceqq_u16(TstV1, RefV1);
        uint16x8_t CmpV2 = vceqq_u16(TstV2, RefV2);

        if(vminvq_u16(CmpV1) != 0xFFFF || vminvq_u16(CmpV2) != 0xFFFF) { return false; }
      }
      Tst += TstStride;
      Ref += RefStride;
    }
  }
  else
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16<uint32>);
    for (int32 y = 0; y < Height; ++y)
    {
      for (int32 x = 0; x < Width16; x += 16)
      {
        uint16x8_t TstV1 = vld1q_u16(&Tst[x]);
        uint16x8_t TstV2 = vld1q_u16(&Tst[x + 8]);
        uint16x8_t RefV1 = vld1q_u16(&Ref[x]);
        uint16x8_t RefV2 = vld1q_u16(&Ref[x + 8]);

        uint16x8_t CmpV1 = vceqq_u16(TstV1, RefV1);
        uint16x8_t CmpV2 = vceqq_u16(TstV2, RefV2);

        if(vminvq_u16(CmpV1) != 0xFFFF || vminvq_u16(CmpV2) != 0xFFFF) { return false; }
      }
      for (int32 x = Width16; x < Width; ++x)
      {
        if(Tst[x] != Ref[x]) { return false; }
      }

      Tst += TstStride;
      Ref += RefStride;
    }
  }
  return true;
}
//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_NEON
