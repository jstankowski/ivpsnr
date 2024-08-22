/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xColorSpaceAVX.h"
#include "xColorSpaceCoeff.h"

#if X_SIMD_CAN_USE_AVX

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xColorSpaceAVX::ConvertRGB2YCbCr_I32(uint16* restrict Y, uint16* restrict U, uint16* restrict V, const uint16* R, const uint16* G, const uint16* B, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
{
  const int32 Y_R = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][0][0];
  const int32 Y_G = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][0][1];
  const int32 Y_B = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][0][2];
  const int32 U_R = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][1][0];
  const int32 U_G = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][1][1];
//const int32 U_B = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][1][2]; //is always 0.5
//const int32 V_R = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][2][0]; //is always 0.5
  const int32 V_G = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][2][1];
  const int32 V_B = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][2][2];

  constexpr int32  Add = xColorSpaceCoeff<int32>::c_Add;
  constexpr uint32 Shr = xColorSpaceCoeff<int32>::c_Precision;
  constexpr uint32 Shl = Shr - 1;
  const     int32  Mid = (int32)xBitDepth2MidValue(BitDepth);
  const     int32  Max = (int32)xBitDepth2MaxValue(BitDepth);

  const __m256i Y_R_I32_V  = _mm256_set1_epi32(Y_R);
  const __m256i Y_G_I32_V  = _mm256_set1_epi32(Y_G);
  const __m256i Y_B_I32_V  = _mm256_set1_epi32(Y_B);
  const __m256i U_R_I32_V  = _mm256_set1_epi32(U_R);
  const __m256i U_G_I32_V  = _mm256_set1_epi32(U_G);
  const __m256i V_G_I32_V  = _mm256_set1_epi32(V_G);
  const __m256i V_B_I32_V  = _mm256_set1_epi32(V_B);
  const __m256i Add_I32_V  = _mm256_set1_epi32(Add);
  const __m256i Mid_I32_V  = _mm256_set1_epi32(Mid);
  const __m256i Max_U16_V  = _mm256_set1_epi16((uint16)Max);

  const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16);
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width16; x += 16)
    {
      //load
      __m256i r_U16_V = _mm256_loadu_si256((__m256i*)(R + x));
      __m256i g_U16_V = _mm256_loadu_si256((__m256i*)(G + x));
      __m256i b_U16_V = _mm256_loadu_si256((__m256i*)(B + x));

      //convert uint16 to int32
      __m256i r_I32_V0 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128  (r_U16_V   ));
      __m256i r_I32_V1 = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(r_U16_V, 1));
      __m256i g_I32_V0 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128  (g_U16_V   ));
      __m256i g_I32_V1 = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(g_U16_V, 1));
      __m256i b_I32_V0 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128  (b_U16_V   ));
      __m256i b_I32_V1 = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(b_U16_V, 1));

      //convert RGB --> YCbCr
      __m256i y_I32_V0 = _mm256_srai_epi32(_mm256_add_epi32(_mm256_add_epi32(_mm256_mullo_epi32(r_I32_V0, Y_R_I32_V), _mm256_mullo_epi32(g_I32_V0, Y_G_I32_V)), _mm256_add_epi32(_mm256_mullo_epi32(b_I32_V0, Y_B_I32_V), Add_I32_V)), Shr);
      __m256i y_I32_V1 = _mm256_srai_epi32(_mm256_add_epi32(_mm256_add_epi32(_mm256_mullo_epi32(r_I32_V1, Y_R_I32_V), _mm256_mullo_epi32(g_I32_V1, Y_G_I32_V)), _mm256_add_epi32(_mm256_mullo_epi32(b_I32_V1, Y_B_I32_V), Add_I32_V)), Shr);
      __m256i u_I32_V0 = _mm256_srai_epi32(_mm256_add_epi32(_mm256_add_epi32(_mm256_mullo_epi32(r_I32_V0, U_R_I32_V), _mm256_mullo_epi32(g_I32_V0, U_G_I32_V)), _mm256_add_epi32(_mm256_slli_epi32 (b_I32_V0, Shl      ), Add_I32_V)), Shr);
      __m256i u_I32_V1 = _mm256_srai_epi32(_mm256_add_epi32(_mm256_add_epi32(_mm256_mullo_epi32(r_I32_V1, U_R_I32_V), _mm256_mullo_epi32(g_I32_V1, U_G_I32_V)), _mm256_add_epi32(_mm256_slli_epi32 (b_I32_V1, Shl      ), Add_I32_V)), Shr);
      __m256i v_I32_V0 = _mm256_srai_epi32(_mm256_add_epi32(_mm256_add_epi32(_mm256_slli_epi32 (r_I32_V0, Shl      ), _mm256_mullo_epi32(g_I32_V0, V_G_I32_V)), _mm256_add_epi32(_mm256_mullo_epi32(b_I32_V0, V_B_I32_V), Add_I32_V)), Shr);
      __m256i v_I32_V1 = _mm256_srai_epi32(_mm256_add_epi32(_mm256_add_epi32(_mm256_slli_epi32 (r_I32_V1, Shl      ), _mm256_mullo_epi32(g_I32_V1, V_G_I32_V)), _mm256_add_epi32(_mm256_mullo_epi32(b_I32_V1, V_B_I32_V), Add_I32_V)), Shr);

      //change data format (and apply chroma offset) + clip to range 0-Max [_mm256_permute4x64_epi64 used to fix AVX per lane mess]
      __m256i y_U16_V  = _mm256_permute4x64_epi64(_mm256_packus_epi32(y_I32_V0, y_I32_V1),                                                           0xD8);
      __m256i u_U16_V  = _mm256_permute4x64_epi64(_mm256_packus_epi32(_mm256_add_epi32(u_I32_V0, Mid_I32_V), _mm256_add_epi32(u_I32_V1, Mid_I32_V)), 0xD8);
      __m256i v_U16_V  = _mm256_permute4x64_epi64(_mm256_packus_epi32(_mm256_add_epi32(v_I32_V0, Mid_I32_V), _mm256_add_epi32(v_I32_V1, Mid_I32_V)), 0xD8);
      __m256i cy_U16_V = _mm256_min_epu16(y_U16_V, Max_U16_V);
      __m256i cu_U16_V = _mm256_min_epu16(u_U16_V, Max_U16_V);
      __m256i cv_U16_V = _mm256_min_epu16(v_U16_V, Max_U16_V);

      //store
      _mm256_storeu_si256((__m256i*)(Y + x), cy_U16_V);
      _mm256_storeu_si256((__m256i*)(U + x), cu_U16_V);
      _mm256_storeu_si256((__m256i*)(V + x), cv_U16_V);
    }
    for(int32 x = Width16; x < Width; x++)
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
void xColorSpaceAVX::ConvertYCbCr2RGB_I32(uint16* restrict R, uint16* restrict G, uint16* restrict B, const uint16* Y, const uint16* U, const uint16* V, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
{
//const int32 R_Y = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][0][0]; //is always 1.0
//const int32 R_U = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][0][1]; //is always 0.0
  const int32 R_V = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][0][2];
//const int32 G_Y = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][1][0]; //is always 1.0
  const int32 G_U = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][1][1];
  const int32 G_V = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][1][2];
//const int32 B_Y = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][2][0]; //is always 1.0
  const int32 B_U = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][2][1];
//const int32 B_V = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][2][2]; //is always 0.0

  constexpr int32  Add = xColorSpaceCoeff<int32>::c_Add;
  constexpr uint32 Shr = xColorSpaceCoeff<int32>::c_Precision;
  const     int32  Mid = (int32)xBitDepth2MidValue(BitDepth);
  const     int32  Max = (int32)xBitDepth2MaxValue(BitDepth);

  const __m256i R_V_I32_V = _mm256_set1_epi32(R_V);
  const __m256i G_U_I32_V = _mm256_set1_epi32(G_U);
  const __m256i G_V_I32_V = _mm256_set1_epi32(G_V);
  const __m256i B_U_I32_V = _mm256_set1_epi32(B_U);
  const __m256i Add_I32_V = _mm256_set1_epi32(Add);
  const __m256i Mid_I32_V = _mm256_set1_epi32(Mid);
  const __m256i Max_U16_V = _mm256_set1_epi16((uint16)Max);

  const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16);
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width16; x += 16)
    {
      //load
      __m256i y_U16_V = _mm256_loadu_si256((__m256i*)(Y + x));
      __m256i u_U16_V = _mm256_loadu_si256((__m256i*)(U + x));
      __m256i v_U16_V = _mm256_loadu_si256((__m256i*)(V + x));
      
      //convert uint16 to int32, remove chroma offset
      __m256i y_I32_V0 =                  _mm256_cvtepi16_epi32(_mm256_castsi256_si128  (y_U16_V   ));
      __m256i y_I32_V1 =                  _mm256_cvtepi16_epi32(_mm256_extracti128_si256(y_U16_V, 1));
      __m256i u_I32_V0 = _mm256_sub_epi32(_mm256_cvtepi16_epi32(_mm256_castsi256_si128  (u_U16_V   )), Mid_I32_V);
      __m256i u_I32_V1 = _mm256_sub_epi32(_mm256_cvtepi16_epi32(_mm256_extracti128_si256(u_U16_V, 1)), Mid_I32_V);
      __m256i v_I32_V0 = _mm256_sub_epi32(_mm256_cvtepi16_epi32(_mm256_castsi256_si128  (v_U16_V   )), Mid_I32_V);
      __m256i v_I32_V1 = _mm256_sub_epi32(_mm256_cvtepi16_epi32(_mm256_extracti128_si256(v_U16_V, 1)), Mid_I32_V);

      //convert YCbCr --> RGB
      __m256i sy_I32_V0 = _mm256_add_epi32(_mm256_slli_epi32(y_I32_V0, Shr), Add_I32_V);
      __m256i sy_I32_V1 = _mm256_add_epi32(_mm256_slli_epi32(y_I32_V1, Shr), Add_I32_V);

      __m256i r_I32_V0 = _mm256_srai_epi32(_mm256_add_epi32(sy_I32_V0,                                                           _mm256_mullo_epi32(v_I32_V0, R_V_I32_V) ), Shr);
      __m256i r_I32_V1 = _mm256_srai_epi32(_mm256_add_epi32(sy_I32_V1,                                                           _mm256_mullo_epi32(v_I32_V1, R_V_I32_V) ), Shr);
      __m256i g_I32_V0 = _mm256_srai_epi32(_mm256_add_epi32(sy_I32_V0, _mm256_add_epi32(_mm256_mullo_epi32(u_I32_V0, G_U_I32_V), _mm256_mullo_epi32(v_I32_V0, G_V_I32_V))), Shr);
      __m256i g_I32_V1 = _mm256_srai_epi32(_mm256_add_epi32(sy_I32_V1, _mm256_add_epi32(_mm256_mullo_epi32(u_I32_V1, G_U_I32_V), _mm256_mullo_epi32(v_I32_V1, G_V_I32_V))), Shr);
      __m256i b_I32_V0 = _mm256_srai_epi32(_mm256_add_epi32(sy_I32_V0,                  _mm256_mullo_epi32(u_I32_V0, B_U_I32_V)                                          ), Shr);
      __m256i b_I32_V1 = _mm256_srai_epi32(_mm256_add_epi32(sy_I32_V1,                  _mm256_mullo_epi32(u_I32_V1, B_U_I32_V)                                          ), Shr);

      //clip to range 0-Max, convert int32 to uint16 [_mm256_permute4x64_epi64 used to fix AVX per lane mess]
      __m256i r_U16_V  = _mm256_permute4x64_epi64(_mm256_packus_epi32(r_I32_V0, r_I32_V1), 0xD8);
      __m256i g_U16_V  = _mm256_permute4x64_epi64(_mm256_packus_epi32(g_I32_V0, g_I32_V1), 0xD8);
      __m256i b_U16_V  = _mm256_permute4x64_epi64(_mm256_packus_epi32(b_I32_V0, b_I32_V1), 0xD8);
      __m256i cr_U16_V = _mm256_min_epu16(r_U16_V, Max_U16_V);
      __m256i cg_U16_V = _mm256_min_epu16(g_U16_V, Max_U16_V);
      __m256i cb_U16_V = _mm256_min_epu16(b_U16_V, Max_U16_V);

      //store
      _mm256_storeu_si256((__m256i*)(R + x), cr_U16_V);
      _mm256_storeu_si256((__m256i*)(G + x), cg_U16_V);
      _mm256_storeu_si256((__m256i*)(B + x), cb_U16_V);
    }
    for(int32 x = Width16; x < Width; x++)
    {
      int32 iy = (int32)(Y[x]);
      int32 iu = (int32)(U[x]) - Mid;
      int32 iv = (int32)(V[x]) - Mid;
      int32 sy = (iy<<Shr) + Add;
      int32 r  = (sy +        + R_V*iv)>>Shr;
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

#endif //X_SIMD_CAN_USE_AVX
