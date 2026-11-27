/*
    SPDX-FileCopyrightText: 2025 Patrycja Kaźmierczak <patrycja.kazmierczak@student.put.poznan.pl>
    SPDX-FileCopyrightText: 2015-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xDistortionNEON.h"
#include "xHelpersSIMD.h"

#if X_SIMD_CAN_USE_NEON

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
int64 xDistortionNEON::CalcSD14(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  int32x4_t SD_I32V = vdupq_n_s32(0);
  if(((uint32)Width & c_RemainderMask16<uint32>)==0) 
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        uint16x8_t TstA_U16V  = vld1q_u16(Tst + x    );
        uint16x8_t RefA_U16V  = vld1q_u16(Ref + x    );
        uint16x8_t TstB_U16V  = vld1q_u16(Tst + x + 8);
        uint16x8_t RefB_U16V  = vld1q_u16(Ref + x + 8);
        int16x8_t  DiffA_I16V = vsubq_s16(vreinterpretq_s16_u16(TstA_U16V), vreinterpretq_s16_u16(RefA_U16V));
        int16x8_t  DiffB_I16V = vsubq_s16(vreinterpretq_s16_u16(TstB_U16V), vreinterpretq_s16_u16(RefB_U16V));
        int32x4_t  Diff1_I32V = vaddl_s16     (vget_low_s16(DiffA_I16V), vget_low_s16(DiffB_I16V));
        int32x4_t  Diff2_I32V = vaddl_high_s16(             DiffA_I16V ,              DiffB_I16V );
        int32x4_t  Diff_I32_V = vaddq_s32(Diff1_I32V, Diff2_I32V);
        SD_I32V               = vaddq_s32(SD_I32V, Diff_I32_V);
      }//x
      Tst += TstStride;
      Ref += RefStride;
    }//y
    int64 SD = vaddvq_s32(SD_I32V);
    return SD;
  }
  else
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16<uint32>);
    int64 SD = 0;
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width16; x+=16)
      {
        uint16x8_t TstA_U16V  = vld1q_u16(Tst + x    );
        uint16x8_t RefA_U16V  = vld1q_u16(Ref + x    );
        uint16x8_t TstB_U16V  = vld1q_u16(Tst + x + 8);
        uint16x8_t RefB_U16V  = vld1q_u16(Ref + x + 8);
        int16x8_t  DiffA_I16V = vsubq_s16(vreinterpretq_s16_u16(TstA_U16V), vreinterpretq_s16_u16(RefA_U16V));
        int16x8_t  DiffB_I16V = vsubq_s16(vreinterpretq_s16_u16(TstB_U16V), vreinterpretq_s16_u16(RefB_U16V));
        int32x4_t  Diff1_I32V = vaddl_s16     (vget_low_s16(DiffA_I16V), vget_low_s16(DiffB_I16V));
        int32x4_t  Diff2_I32V = vaddl_high_s16(             DiffA_I16V ,              DiffB_I16V );
        int32x4_t  Diff_I32_V = vaddq_s32(Diff1_I32V, Diff2_I32V);
        SD_I32V               = vaddq_s32(SD_I32V, Diff_I32_V);
      }//8x
      for(int32 x=Width16; x<Width; x++)
      {
        SD += (int32)Tst[x] - (int32)Ref[x];
      }//x
      Tst += TstStride;
      Ref += RefStride;
    }//y
    SD += vaddvq_s32(SD_I32V);
    return SD;
  }
}
int64 xDistortionNEON::CalcSD16(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  int32x4_t  SD_V128 = vdupq_n_s32(0);
  if(((uint32)Width & c_RemainderMask8<uint32>)==0) //Width%8==0 - fast path without tail
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=8)
      {
        uint16x8_t Tst_V128   = vld1q_u16(&Tst[x]);
        uint16x8_t Ref_V128   = vld1q_u16(&Ref[x]);
        int32x4_t  Diffl_V128 = vsubq_s32(vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Tst_V128))), vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Ref_V128)))); //sub 0-3 to uint32
        int32x4_t  Diffh_V128 = vsubq_s32(vreinterpretq_s32_u32(vmovl_high_u16(Tst_V128)), vreinterpretq_s32_u32(vmovl_high_u16(Ref_V128))); //sub 4-7
        int32x4_t  Sum_V128   = vaddq_s32(Diffl_V128, Diffh_V128);
        SD_V128               = vaddq_s32(SD_V128,    Sum_V128);
      }//x
      Tst += TstStride;
      Ref += RefStride;
    }//y
    int64 SD = vaddvq_s32(SD_V128);
    return SD;
  }
  else
  {
    const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8<uint32>);
    const int32 Width4 = (int32)((uint32)Width & c_MultipleMask4<uint32>);
    int64 SD = 0;
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width8; x+=8)
      {
        uint16x8_t Tst_V128   = vld1q_u16(&Tst[x]);
        uint16x8_t Ref_V128   = vld1q_u16(&Ref[x]);
        int32x4_t  Diffl_V128 = vsubq_s32(vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Tst_V128))), vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Ref_V128)))); //sub 0-3 to uint32
        int32x4_t  Diffh_V128 = vsubq_s32(vreinterpretq_s32_u32(vmovl_high_u16(Tst_V128)), vreinterpretq_s32_u32(vmovl_high_u16(Ref_V128))); //sub 4-7
        int32x4_t  Sum_V128   = vaddq_s32(Diffl_V128, Diffh_V128);
        SD_V128               = vaddq_s32(SD_V128,    Sum_V128);
      }//8x
      for(int32 x=Width8; x<Width4; x+=4)
      {
        uint16x4_t Tst_V64   = vld1_u16(&Tst[x]);
        uint16x4_t Ref_V64   = vld1_u16(&Ref[x]);
        int32x4_t  Tst_V128  = vreinterpretq_s32_u32(vmovl_u16(Tst_V64));
        int32x4_t  Ref_V128  = vreinterpretq_s32_u32(vmovl_u16(Ref_V64));
        int32x4_t  Diff_V128 = vsubq_s32(Tst_V128, Ref_V128);
        SD_V128              = vaddq_s32(SD_V128, Diff_V128);
      }//4x
      for(int32 x=Width4; x<Width; x++)
      {
        SD += (int32)Tst[x] - (int32)Ref[x];
      }//x
      Tst += TstStride;
      Ref += RefStride;
    }//y
    SD += vaddvq_s32(SD_V128);
    return SD;
  }
}
uint64 xDistortionNEON::CalcSAD16(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  uint32x4_t  SAD_V = vdupq_n_u32(0);
  if(((uint32)Width & c_RemainderMask8<uint32>)==0) //Width%8==0 - fast path without tail
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=8)
      {
        uint16x8_t Tst_V128   = vld1q_u16(&Tst[x]);
        uint16x8_t Ref_V128   = vld1q_u16(&Ref[x]);
        SAD_V = vabal_u16(SAD_V, vget_low_u16(Tst_V128), vget_low_u16(Ref_V128));
        SAD_V = vabal_high_u16(SAD_V, Tst_V128, Ref_V128);
      }//x
      Tst += TstStride;
      Ref += RefStride;
    }//y
    uint64 SAD = vaddvq_u32(SAD_V);
    return SAD;
  }
  else
  {
    const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8<uint32>);
    const int32 Width4 = (int32)((uint32)Width & c_MultipleMask4<uint32>);
    uint64 SAD = 0;
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width8; x+=8)
      {
        uint16x8_t Tst_V128   = vld1q_u16       (&Tst[x]);
        uint16x8_t Ref_V128   = vld1q_u16       (&Ref[x]);

        SAD_V = vabal_u16(SAD_V, vget_low_u16(Tst_V128), vget_low_u16(Ref_V128));
        SAD_V = vabal_high_u16(SAD_V, Tst_V128, Ref_V128);
      }//8x
      for(int32 x=Width8; x<Width4; x+=4)
      {
        uint16x4_t Tst_V128   = vld1_u16       (&Tst[x]);
        uint16x4_t Ref_V128   = vld1_u16       (&Ref[x]);

        SAD_V = vabal_u16(SAD_V, Tst_V128, Ref_V128);
      }//4x
      for(int32 x=Width4; x<Width; x++)
      {
        SAD += (uint32)xAbs(((int32)Tst[x]) - ((int32)Ref[x]));
      }//x
      Tst += TstStride;
      Ref += RefStride;
    }//y
    SAD += vaddvq_u32(SAD_V);
    return SAD;
  }
}
uint64 xDistortionNEON::CalcSSD14(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  uint64x2_t SSD_U64V = vdupq_n_u64(0);
  if(((uint32)Width & c_RemainderMask16<uint32>)==0) 
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        uint16x8_t TstA_U16V  = vld1q_u16(Tst + x    );
        uint16x8_t RefA_U16V  = vld1q_u16(Ref + x    );
        uint16x8_t TstB_U16V  = vld1q_u16(Tst + x + 8);
        uint16x8_t RefB_U16V  = vld1q_u16(Ref + x + 8);
        int16x8_t  DiffA_I16V = vsubq_s16(vreinterpretq_s16_u16(TstA_U16V), vreinterpretq_s16_u16(RefA_U16V));
        int16x8_t  DiffB_I16V = vsubq_s16(vreinterpretq_s16_u16(TstB_U16V), vreinterpretq_s16_u16(RefB_U16V));
        int32x4_t  PowA_I32V  = vmlal_high_s16(vmull_s16(vget_low_s16(DiffA_I16V), vget_low_s16(DiffA_I16V)), DiffA_I16V, DiffA_I16V);
        int32x4_t  PowB_I32V  = vmlal_high_s16(vmull_s16(vget_low_s16(DiffB_I16V), vget_low_s16(DiffB_I16V)), DiffB_I16V, DiffB_I16V);
        int64x2_t  Pow1_I64   = vaddl_s32     (vget_low_s32(PowA_I32V), vget_low_s32(PowB_I32V));
        int64x2_t  Pow2_I64   = vaddl_high_s32(             PowA_I32V ,              PowB_I32V );
        int64x2_t  Pow_I64V   = vaddq_s64(Pow1_I64, Pow2_I64);
        SSD_U64V = vaddq_u64(SSD_U64V, vreinterpretq_u64_s64(Pow_I64V));    
      }//x
      Tst += TstStride;
      Ref += RefStride;
    }//y
    uint64 SSD = vaddvq_u64(SSD_U64V);
    return SSD;
  }
  else
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16<uint32>);
    uint64 SSD = 0;
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width16; x+=16)
      {
        uint16x8_t TstA_U16V  = vld1q_u16(Tst + x    );
        uint16x8_t RefA_U16V  = vld1q_u16(Ref + x    );
        uint16x8_t TstB_U16V  = vld1q_u16(Tst + x + 8);
        uint16x8_t RefB_U16V  = vld1q_u16(Ref + x + 8);
        int16x8_t  DiffA_I16V = vsubq_s16(vreinterpretq_s16_u16(TstA_U16V), vreinterpretq_s16_u16(RefA_U16V));
        int16x8_t  DiffB_I16V = vsubq_s16(vreinterpretq_s16_u16(TstB_U16V), vreinterpretq_s16_u16(RefB_U16V));
        int32x4_t  PowA_I32V  = vmlal_high_s16(vmull_s16(vget_low_s16(DiffA_I16V), vget_low_s16(DiffA_I16V)), DiffA_I16V, DiffA_I16V);
        int32x4_t  PowB_I32V  = vmlal_high_s16(vmull_s16(vget_low_s16(DiffB_I16V), vget_low_s16(DiffB_I16V)), DiffB_I16V, DiffB_I16V);
        int64x2_t  Pow1_I64   = vaddl_s32     (vget_low_s32(PowA_I32V), vget_low_s32(PowB_I32V));
        int64x2_t  Pow2_I64   = vaddl_high_s32(             PowA_I32V ,              PowB_I32V );
        int64x2_t  Pow_I64V   = vaddq_s64(Pow1_I64, Pow2_I64);
        SSD_U64V = vaddq_u64(SSD_U64V, vreinterpretq_u64_s64(Pow_I64V));    
      }
      for(int32 x=Width16; x<Width; x++)
      {
        SSD += (uint64)xPow2(((int32)Tst[x]) - ((int32)Ref[x]));
      }//x
      Tst += TstStride;
      Ref += RefStride;
    }//y
    SSD += vaddvq_u64(SSD_U64V);
    return SSD;
  }
}
uint64 xDistortionNEON::CalcSSD16(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  uint64x2_t SSD_V = vdupq_n_u64(0);
  if(((uint32)Width & c_RemainderMask8<uint32>)==0) //Width%8==0 - fast path without tail
  {
    for(int32 y=0; y<Height; y++)
    {
      int64x2_t RowSSD_V = vdupq_n_s64(0);
      for(int32 x=0; x<Width; x+=8)
      {
        uint16x8_t Tst_V    = vld1q_u16     (&Tst[x]);
        uint16x8_t Ref_V    = vld1q_u16     (&Ref[x]);
        int32x4_t Diffl_V128  = vsubq_s32  (vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Tst_V))), vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Ref_V)))); 
        int32x4_t Diffh_V128  = vsubq_s32  (vreinterpretq_s32_u32(vmovl_high_u16(Tst_V)), vreinterpretq_s32_u32(vmovl_high_u16(Ref_V)));
        
        //multiply accumulate and widen
        RowSSD_V = vmlal_s32      (RowSSD_V, vget_low_s32(Diffl_V128),vget_low_s32(Diffl_V128));
        RowSSD_V = vmlal_high_s32 (RowSSD_V, Diffl_V128, Diffl_V128);
        RowSSD_V = vmlal_s32      (RowSSD_V, vget_low_s32(Diffh_V128),vget_low_s32(Diffh_V128));
        RowSSD_V = vmlal_high_s32 (RowSSD_V, Diffh_V128, Diffh_V128);        
      }//x
      SSD_V = vaddq_u64(SSD_V, vreinterpretq_u64_s64(RowSSD_V));
      Tst += TstStride;
      Ref += RefStride;
    }//y
    uint64 SSD = vaddvq_u64(SSD_V);
    return SSD;
  }
  else
  {
    const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8<uint32>);
    const int32 Width4 = (int32)((uint32)Width & c_MultipleMask4<uint32>);
    uint64 SSD = 0;
    for(int32 y=0; y<Height; y++)
    {
      int64x2_t RowSSD_V = vdupq_n_s64(0);
      for(int32 x=0; x<Width8; x+=8)
      {
        uint16x8_t Tst_V    = vld1q_u16     (&Tst[x]);
        uint16x8_t Ref_V    = vld1q_u16     (&Ref[x]);
        int32x4_t Diffl_V128  = vsubq_s32  (vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Tst_V))), vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(Ref_V)))); 
        int32x4_t Diffh_V128  = vsubq_s32  (vreinterpretq_s32_u32(vmovl_high_u16(Tst_V)), vreinterpretq_s32_u32(vmovl_high_u16(Ref_V)));
        
        //multiply accumulate and widen
        RowSSD_V = vmlal_s32      (RowSSD_V, vget_low_s32(Diffl_V128),vget_low_s32(Diffl_V128));
        RowSSD_V = vmlal_high_s32 (RowSSD_V, Diffl_V128, Diffl_V128);
        RowSSD_V = vmlal_s32      (RowSSD_V, vget_low_s32(Diffh_V128),vget_low_s32(Diffh_V128));
        RowSSD_V = vmlal_high_s32 (RowSSD_V, Diffh_V128, Diffh_V128);
      }//8x
      for(int32 x=Width8; x<Width4; x+=4)
      {
        uint16x4_t Tst_V    = vld1_u16     (&Tst[x]);
        uint16x4_t Ref_V    = vld1_u16     (&Ref[x]);
        int32x4_t Diffl_V128  = vsubq_s32 (vreinterpretq_s32_u32(vmovl_u16(Tst_V)), vreinterpretq_s32_u32(vmovl_u16(Ref_V))); 
        
        //multiply accumulate and widen
        RowSSD_V = vmlal_s32      (RowSSD_V, vget_low_s32(Diffl_V128), vget_low_s32(Diffl_V128));
        RowSSD_V = vmlal_high_s32 (RowSSD_V, Diffl_V128, Diffl_V128);
      }//4x
      for(int32 x=Width4; x<Width; x++)
      {
        SSD += (uint64)xPow2(((int32)Tst[x]) - ((int32)Ref[x]));
      }//x
      SSD_V = vaddq_u64(SSD_V, vreinterpretq_u64_s64(RowSSD_V));
      Tst += TstStride;
      Ref += RefStride;
    }//y
    SSD += vaddvq_u64(SSD_V);
    return SSD;
  }
}
tSDSSD xDistortionNEON::CalcSSS14(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  int32x4_t  SD_I32V  = vdupq_n_s32(0);
  uint64x2_t SSD_U64V = vdupq_n_u64(0);
  if(((uint32)Width & c_RemainderMask16<uint32>)==0) 
  {
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width; x+=16)
      {
        uint16x8_t TstA_U16V  = vld1q_u16(Tst + x    );
        uint16x8_t RefA_U16V  = vld1q_u16(Ref + x    );
        uint16x8_t TstB_U16V  = vld1q_u16(Tst + x + 8);
        uint16x8_t RefB_U16V  = vld1q_u16(Ref + x + 8);
        int16x8_t  DiffA_I16V = vsubq_s16(vreinterpretq_s16_u16(TstA_U16V), vreinterpretq_s16_u16(RefA_U16V));
        int16x8_t  DiffB_I16V = vsubq_s16(vreinterpretq_s16_u16(TstB_U16V), vreinterpretq_s16_u16(RefB_U16V));
        int32x4_t  Diff1_I32V = vaddl_s16     (vget_low_s16(DiffA_I16V), vget_low_s16(DiffB_I16V));
        int32x4_t  Diff2_I32V = vaddl_high_s16(             DiffA_I16V ,              DiffB_I16V );
        int32x4_t  Diff_I32_V = vaddq_s32(Diff1_I32V, Diff2_I32V);
        SD_I32V               = vaddq_s32(SD_I32V, Diff_I32_V);
        int32x4_t  PowA_I32V  = vmlal_high_s16(vmull_s16(vget_low_s16(DiffA_I16V), vget_low_s16(DiffA_I16V)), DiffA_I16V, DiffA_I16V);
        int32x4_t  PowB_I32V  = vmlal_high_s16(vmull_s16(vget_low_s16(DiffB_I16V), vget_low_s16(DiffB_I16V)), DiffB_I16V, DiffB_I16V);
        int64x2_t  Pow1_I64   = vaddl_s32     (vget_low_s32(PowA_I32V), vget_low_s32(PowB_I32V));
        int64x2_t  Pow2_I64   = vaddl_high_s32(             PowA_I32V ,              PowB_I32V );
        int64x2_t  Pow_I64V   = vaddq_s64(Pow1_I64, Pow2_I64);
        SSD_U64V = vaddq_u64(SSD_U64V, vreinterpretq_u64_s64(Pow_I64V)); 
      }//x
      Tst += TstStride;
      Ref += RefStride;
    }//y
    int64  SD  = vaddvq_s32(SD_I32V);
    uint64 SSD = vaddvq_u64(SSD_U64V);
    return { SD, SSD };
  }
  else
  {
    const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16<uint32>);
    int64  SD  = 0;
    uint64 SSD = 0;
    for(int32 y=0; y<Height; y++)
    {
      for(int32 x=0; x<Width16; x+=16)
      {
        uint16x8_t TstA_U16V  = vld1q_u16(Tst + x    );
        uint16x8_t RefA_U16V  = vld1q_u16(Ref + x    );
        uint16x8_t TstB_U16V  = vld1q_u16(Tst + x + 8);
        uint16x8_t RefB_U16V  = vld1q_u16(Ref + x + 8);
        int16x8_t  DiffA_I16V = vsubq_s16(vreinterpretq_s16_u16(TstA_U16V), vreinterpretq_s16_u16(RefA_U16V));
        int16x8_t  DiffB_I16V = vsubq_s16(vreinterpretq_s16_u16(TstB_U16V), vreinterpretq_s16_u16(RefB_U16V));
        int32x4_t  Diff1_I32V = vaddl_s16     (vget_low_s16(DiffA_I16V), vget_low_s16(DiffB_I16V));
        int32x4_t  Diff2_I32V = vaddl_high_s16(             DiffA_I16V ,              DiffB_I16V );
        int32x4_t  Diff_I32_V = vaddq_s32(Diff1_I32V, Diff2_I32V);
        SD_I32V               = vaddq_s32(SD_I32V, Diff_I32_V);
        int32x4_t  PowA_I32V  = vmlal_high_s16(vmull_s16(vget_low_s16(DiffA_I16V), vget_low_s16(DiffA_I16V)), DiffA_I16V, DiffA_I16V);
        int32x4_t  PowB_I32V  = vmlal_high_s16(vmull_s16(vget_low_s16(DiffB_I16V), vget_low_s16(DiffB_I16V)), DiffB_I16V, DiffB_I16V);
        int64x2_t  Pow1_I64   = vaddl_s32     (vget_low_s32(PowA_I32V), vget_low_s32(PowB_I32V));
        int64x2_t  Pow2_I64   = vaddl_high_s32(             PowA_I32V ,              PowB_I32V );
        int64x2_t  Pow_I64V   = vaddq_s64(Pow1_I64, Pow2_I64);
        SSD_U64V = vaddq_u64(SSD_U64V, vreinterpretq_u64_s64(Pow_I64V)); 
      }
      for(int32 x=Width16; x<Width; x++)
      {
        int32 Diff = (int32)Tst[x] - (int32)Ref[x];
        SD  += Diff;
        SSD += xPow2(Diff);
      }//x
      Tst += TstStride;
      Ref += RefStride;
    }//y
    SD  += vaddvq_s32(SD_I32V );
    SSD += vaddvq_u64(SSD_U64V);
    return { SD, SSD };
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_NEON
