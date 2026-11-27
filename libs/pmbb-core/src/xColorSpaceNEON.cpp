/*
    SPDX-FileCopyrightText: 2025 Patrycja Kaźmierczak <patrycja.kazmierczak@student.put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xColorSpaceNEON.h"
#include "xColorSpaceCoeff.h"

#if X_SIMD_CAN_USE_NEON

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xColorSpaceNEON::ConvertRGB2YCbCr_I32(uint16* restrict Y, uint16* restrict U, uint16* restrict V, const uint16* R, const uint16* G, const uint16* B, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
{
  const int32 Y_R = xColorSpaceCoeffYCbCr::c_RGB2YCbCr_I32[(int32)ClrSpc][0][0];
  const int32 Y_G = xColorSpaceCoeffYCbCr::c_RGB2YCbCr_I32[(int32)ClrSpc][0][1];
  const int32 Y_B = xColorSpaceCoeffYCbCr::c_RGB2YCbCr_I32[(int32)ClrSpc][0][2];
  const int32 U_R = xColorSpaceCoeffYCbCr::c_RGB2YCbCr_I32[(int32)ClrSpc][1][0];
  const int32 U_G = xColorSpaceCoeffYCbCr::c_RGB2YCbCr_I32[(int32)ClrSpc][1][1];
//const int32 U_B = xColorSpaceCoeffYCbCr::c_RGB2YCbCr_I32[(int32)ClrSpc][1][2]; //is always 0.5
//const int32 V_R = xColorSpaceCoeffYCbCr::c_RGB2YCbCr_I32[(int32)ClrSpc][2][0]; //is always 0.5
  const int32 V_G = xColorSpaceCoeffYCbCr::c_RGB2YCbCr_I32[(int32)ClrSpc][2][1];
  const int32 V_B = xColorSpaceCoeffYCbCr::c_RGB2YCbCr_I32[(int32)ClrSpc][2][2];

  constexpr int32  Add = xColorSpaceCoeffYCbCr::c_Add;
  constexpr uint32 Shr = xColorSpaceCoeffYCbCr::c_Precision;
  constexpr uint32 Shl = Shr - 1;
  const     int32  Mid = (int32)xBitDepth2MidValue(BitDepth);
  const     int32  Max = (int32)xBitDepth2MaxValue(BitDepth);

  const int32x4_t Add_I32_V  =  vdupq_n_s32(Add);
  const int32x4_t Mid_I32_V  =  vdupq_n_s32(Mid);
  const uint16x8_t Max_U16_V =  vdupq_n_u16((uint16)Max);

  const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8<uint32>);
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width8; x += 8)
    {
      //load
      uint16x8_t r_U16_V = vld1q_u16((R + x));
      uint16x8_t g_U16_V = vld1q_u16((G + x));
      uint16x8_t b_U16_V = vld1q_u16((B + x));

      //convert uint16 to int32
      int32x4_t r_I32_V0 = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(r_U16_V)));
      int32x4_t r_I32_V1 = vreinterpretq_s32_u32(vmovl_high_u16(r_U16_V));
      int32x4_t g_I32_V0 = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(g_U16_V)));
      int32x4_t g_I32_V1 = vreinterpretq_s32_u32(vmovl_high_u16(g_U16_V));
      int32x4_t b_I32_V0 = vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(b_U16_V)));
      int32x4_t b_I32_V1 = vreinterpretq_s32_u32(vmovl_high_u16(b_U16_V));

      //convert RGB --> YCbCr
      //ty = (int32)round(Y_R*r + Y_G*g + Y_B*b);
      int32x4_t yRr_V0 = vmulq_n_s32(r_I32_V0, Y_R);
      int32x4_t yGg_V0 = vmulq_n_s32(g_I32_V0, Y_G);
      int32x4_t yBb_V0 = vmulq_n_s32(b_I32_V0, Y_B);
      int32x4_t y_I32_V0 = vrshrq_n_s32(vaddq_s32(vaddq_s32(yRr_V0, yGg_V0), vaddq_s32(yBb_V0, Add_I32_V)), Shr);

      int32x4_t yRr_V1 = vmulq_n_s32(r_I32_V1, Y_R);
      int32x4_t yGg_V1 = vmulq_n_s32(g_I32_V1, Y_G);
      int32x4_t yBb_V1 = vmulq_n_s32(b_I32_V1, Y_B);
      int32x4_t y_I32_V1 = vrshrq_n_s32(vaddq_s32(vaddq_s32(yRr_V1, yGg_V1), vaddq_s32(yBb_V1, Add_I32_V)), Shr);
      
      //tu = (int32)round(U_R*r + U_G*g + U_B*b);
      int32x4_t uRr_V0 = vmulq_n_s32(r_I32_V0, U_R);
      int32x4_t uGg_V0 = vmulq_n_s32(g_I32_V0, U_G);
      int32x4_t uBb_V0 = vqshlq_n_s32(b_I32_V0, Shl); //shift
      int32x4_t u_I32_V0 = vrshrq_n_s32(vaddq_s32(vaddq_s32(uRr_V0, uGg_V0), vaddq_s32(uBb_V0, Add_I32_V)), Shr);

      int32x4_t uRr_V1 = vmulq_n_s32(r_I32_V1, U_R);
      int32x4_t uGg_V1 = vmulq_n_s32(g_I32_V1, U_G);
      int32x4_t uBb_V1 = vqshlq_n_s32(b_I32_V1, Shl); //shift
      int32x4_t u_I32_V1 = vrshrq_n_s32(vaddq_s32(vaddq_s32(uRr_V1, uGg_V1), vaddq_s32(uBb_V1, Add_I32_V)), Shr);
      
      //tv = (int32)round(V_R*r + V_G*g + V_B*b);
      int32x4_t vRr_V0 = vqshlq_n_s32(r_I32_V0, Shl); //shift
      int32x4_t vGg_V0 = vmulq_n_s32(g_I32_V0, V_G);
      int32x4_t vBb_V0 = vmulq_n_s32(b_I32_V0, V_B);
      int32x4_t v_I32_V0 = vrshrq_n_s32(vaddq_s32(vaddq_s32(vRr_V0, vGg_V0), vaddq_s32(vBb_V0, Add_I32_V)), Shr);

      int32x4_t vRr_V1 = vqshlq_n_s32(r_I32_V1, Shl); //shift
      int32x4_t vGg_V1 = vmulq_n_s32(g_I32_V1, V_G);
      int32x4_t vBb_V1 = vmulq_n_s32(b_I32_V1, V_B); 
      int32x4_t v_I32_V1 = vrshrq_n_s32(vaddq_s32(vaddq_s32(vRr_V1, vGg_V1), vaddq_s32(vBb_V1, Add_I32_V)), Shr);

      //change data format (and apply chroma offset) + clip to range 0-Max
      uint16x8_t y_U16_V = vqmovn_high_u32(vqmovn_u32(vreinterpretq_u32_s32(y_I32_V0)), vreinterpretq_u32_s32(y_I32_V1));
      uint16x8_t u_U16_V = vqmovn_high_u32(vqmovn_u32(vreinterpretq_u32_s32(vaddq_s32(u_I32_V0, Mid_I32_V))), vreinterpretq_u32_s32(vaddq_s32(u_I32_V1, Mid_I32_V)));
      uint16x8_t v_U16_V = vqmovn_high_u32(vqmovn_u32(vreinterpretq_u32_s32(vaddq_s32(v_I32_V0, Mid_I32_V))), vreinterpretq_u32_s32(vaddq_s32(v_I32_V1, Mid_I32_V)));
      uint16x8_t cy_V = vminq_u16(y_U16_V, Max_U16_V);
      uint16x8_t cu_V = vminq_u16(u_U16_V, Max_U16_V);
      uint16x8_t cv_V = vminq_u16(v_U16_V, Max_U16_V);
      //store
      vst1q_u16((Y + x), cy_V);
      vst1q_u16((U + x), cu_V);
      vst1q_u16((V + x), cv_V);
    }
    for(int32 x = Width8; x < Width; x++)
    {
      int32 r  = R[x];
      int32 g  = G[x];
      int32 b  = B[x];
      int32 ty = ((Y_R*r    + Y_G*g + Y_B*b    + Add)>>Shr);
      int32 tu = ((U_R*r    + U_G*g + (b<<Shl) + Add)>>Shr);
      int32 tv = (((r<<Shl) + V_G*g + V_B*b    + Add)>>Shr);
      int32 cy = xClipU(ty      , Max);
      int32 cu = xClipU(tu + Mid, Max);
      int32 cv = xClipU(tv + Mid, Max);
      Y[x] = (uint16)cy;
      U[x] = (uint16)cu;
      V[x] = (uint16)cv;
    }
    R += SrcStride; G += SrcStride; B += SrcStride;
    Y += DstStride; U += DstStride; V += DstStride;
  } //y
}
void xColorSpaceNEON::ConvertYCbCr2RGB_I32(uint16* restrict R, uint16* restrict G, uint16* restrict B, const uint16* Y, const uint16* U, const uint16* V, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
{
//const int32 R_Y = xColorSpaceCoeffYCbCr::c_YCbCr2RGB_I32[(uint32)ClrSpc][0][0]; //is always 1.0
//const int32 R_U = xColorSpaceCoeffYCbCr::c_YCbCr2RGB_I32[(uint32)ClrSpc][0][1]; //is always 0.0
  const int32 R_V = xColorSpaceCoeffYCbCr::c_YCbCr2RGB_I32[(uint32)ClrSpc][0][2];
//const int32 G_Y = xColorSpaceCoeffYCbCr::c_YCbCr2RGB_I32[(uint32)ClrSpc][1][0]; //is always 1.0
  const int32 G_U = xColorSpaceCoeffYCbCr::c_YCbCr2RGB_I32[(uint32)ClrSpc][1][1];
  const int32 G_V = xColorSpaceCoeffYCbCr::c_YCbCr2RGB_I32[(uint32)ClrSpc][1][2];
//const int32 B_Y = xColorSpaceCoeffYCbCr::c_YCbCr2RGB_I32[(uint32)ClrSpc][2][0]; //is always 1.0
  const int32 B_U = xColorSpaceCoeffYCbCr::c_YCbCr2RGB_I32[(uint32)ClrSpc][2][1];
//const int32 B_V = xColorSpaceCoeffYCbCr::c_YCbCr2RGB_I32[(uint32)ClrSpc][2][2]; //is always 0.0

  constexpr int32  Add = xColorSpaceCoeffYCbCr::c_Add;
  constexpr uint32 Shr = xColorSpaceCoeffYCbCr::c_Precision;
  const     int32  Mid = (int32)xBitDepth2MidValue(BitDepth);
  const     int32  Max = (int32)xBitDepth2MaxValue(BitDepth);

  const int32x4_t Add_I32_V  =  vdupq_n_s32(Add);
  const int32x4_t Mid_I32_V  =  vdupq_n_s32(Mid);
  //const uint16x8_t Max_U16_V =  vdupq_n_u16((uint16)Max);


  const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8<uint32>);
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width8; x += 8)
    {
      //load
      uint16x8_t y_U16_V = vld1q_u16((Y + x));
      uint16x8_t u_U16_V = vld1q_u16((U + x));
      uint16x8_t v_U16_V = vld1q_u16((V + x));
      
      //change data format (and remove chroma offset)
      int32x4_t y_I32_V0 =           vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(y_U16_V)));
      int32x4_t y_I32_V1 =           vreinterpretq_s32_u32(vmovl_high_u16(y_U16_V));
      int32x4_t u_I32_V0 = vsubq_s32(vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(u_U16_V))), Mid_I32_V);
      int32x4_t u_I32_V1 = vsubq_s32(vreinterpretq_s32_u32(vmovl_high_u16(u_U16_V)), Mid_I32_V);
      int32x4_t v_I32_V0 = vsubq_s32(vreinterpretq_s32_u32(vmovl_u16(vget_low_u16(v_U16_V))), Mid_I32_V);
      int32x4_t v_I32_V1 = vsubq_s32(vreinterpretq_s32_u32(vmovl_high_u16(v_U16_V)), Mid_I32_V);

      //convert YCbCr --> RGB
      //sy = (iy<<Shr) + Add;
      int32x4_t sy_V0 = vaddq_s32(vshlq_n_s32(y_I32_V0, Shr), Add_I32_V);
      int32x4_t sy_V1 = vaddq_s32(vshlq_n_s32(y_I32_V1, Shr), Add_I32_V);

      //r  = (sy +        + R_V*iv)>>Shr;
      int32x4_t r_V0 = vshrq_n_s32(vaddq_s32(sy_V0, vmulq_n_s32(v_I32_V0, R_V)), Shr);
      int32x4_t r_V1 = vshrq_n_s32(vaddq_s32(sy_V1, vmulq_n_s32(v_I32_V1, R_V)), Shr);

      //g  = (sy + G_U*iu + G_V*iv)>>Shr;
      int32x4_t g_V0 = vshrq_n_s32(vaddq_s32(sy_V0, vaddq_s32(vmulq_n_s32(u_I32_V0, G_U), vmulq_n_s32(v_I32_V0, G_V))), Shr);
      int32x4_t g_V1 = vshrq_n_s32(vaddq_s32(sy_V1, vaddq_s32(vmulq_n_s32(u_I32_V1, G_U), vmulq_n_s32(v_I32_V1, G_V))), Shr);

      //b  = (sy + B_U*iu         )>>Shr;
      int32x4_t b_V0 = vshrq_n_s32(vaddq_s32(sy_V0, vmulq_n_s32(u_I32_V0, B_U)), Shr);
      int32x4_t b_V1 = vshrq_n_s32(vaddq_s32(sy_V1, vmulq_n_s32(u_I32_V1, B_U)), Shr);

      uint16x8_t r_V = vcombine_u16(vqmovun_s32(r_V0), vqmovun_s32(r_V1));
      uint16x8_t g_V = vcombine_u16(vqmovun_s32(g_V0), vqmovun_s32(g_V1));
      uint16x8_t b_V = vcombine_u16(vqmovun_s32(b_V0), vqmovun_s32(b_V1));
      
      //store
      vst1q_u16((R + x), r_V);
      vst1q_u16((G + x), g_V);
      vst1q_u16((B + x), b_V);
    }
    for(int32 x = Width8; x < Width; x++)
    {
      int32 iy = (int32)(Y[x]);
      int32 iu = (int32)(U[x]) - Mid;
      int32 iv = (int32)(V[x]) - Mid;
      int32 sy = (iy<<Shr) + Add;
      int32 r  = (sy +         R_V*iv)>>Shr;
      int32 g  = (sy + G_U*iu + G_V*iv)>>Shr;
      int32 b  = (sy + B_U*iu         )>>Shr;
      int32 cr = xClipU(r, Max);
      int32 cg = xClipU(g, Max);
      int32 cb = xClipU(b, Max);
      R[x] = (uint16)cr;
      G[x] = (uint16)cg;
      B[x] = (uint16)cb;
    }
    Y += SrcStride; U += SrcStride; V += SrcStride;
    R += DstStride; G += DstStride; B += DstStride;
  } //y
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_NEON
