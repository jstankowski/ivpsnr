/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xColorSpaceAVX512.h"
#include "xColorSpaceCoeff.h"

#if X_SIMD_CAN_USE_AVX512

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xColorSpaceAVX512::ConvertRGB2YCbCr_I32(uint16* restrict Y, uint16* restrict U, uint16* restrict V, const uint16* R, const uint16* G, const uint16* B, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
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

  const __m512i Y_R_I32_V  = _mm512_set1_epi32(Y_R);
  const __m512i Y_G_I32_V  = _mm512_set1_epi32(Y_G);
  const __m512i Y_B_I32_V  = _mm512_set1_epi32(Y_B);
  const __m512i U_R_I32_V  = _mm512_set1_epi32(U_R);
  const __m512i U_G_I32_V  = _mm512_set1_epi32(U_G);
  const __m512i V_G_I32_V  = _mm512_set1_epi32(V_G);
  const __m512i V_B_I32_V  = _mm512_set1_epi32(V_B);
  const __m512i Add_I32_V  = _mm512_set1_epi32(Add);
  const __m512i Mid_I32_V  = _mm512_set1_epi32(Mid);
  const __m512i Max_U16_V  = _mm512_set1_epi16((uint16)Max);
  const __m512i PermCtlV   = _mm512_setr_epi64(0, 2, 4, 6, 1, 3, 5, 7);

  const int32  Width32     = (int32)((uint32)Width & c_MultipleMask32);
  const uint32 Remainder32 = (uint32)(Width)&c_RemainderMask32;
  const uint32 Mask32      = ((uint32)1 << Remainder32) - 1;

  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width32; x += 32)
    {
      //load
      __m512i r_U16_V = _mm512_loadu_si512((__m512i*)(R + x));
      __m512i g_U16_V = _mm512_loadu_si512((__m512i*)(G + x));
      __m512i b_U16_V = _mm512_loadu_si512((__m512i*)(B + x));

      //convert uint16 to int32
      __m512i r_I32_V0 = _mm512_cvtepu16_epi32(_mm512_castsi512_si256   (r_U16_V   ));
      __m512i r_I32_V1 = _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(r_U16_V, 1));
      __m512i g_I32_V0 = _mm512_cvtepu16_epi32(_mm512_castsi512_si256   (g_U16_V   ));
      __m512i g_I32_V1 = _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(g_U16_V, 1));
      __m512i b_I32_V0 = _mm512_cvtepu16_epi32(_mm512_castsi512_si256   (b_U16_V   ));
      __m512i b_I32_V1 = _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(b_U16_V, 1));

      //convert RGB --> YCbCr
      __m512i y_I32_V0 = _mm512_srai_epi32(_mm512_add_epi32(_mm512_add_epi32(_mm512_mullo_epi32(r_I32_V0, Y_R_I32_V), _mm512_mullo_epi32(g_I32_V0, Y_G_I32_V)), _mm512_add_epi32(_mm512_mullo_epi32(b_I32_V0, Y_B_I32_V), Add_I32_V)), Shr);
      __m512i y_I32_V1 = _mm512_srai_epi32(_mm512_add_epi32(_mm512_add_epi32(_mm512_mullo_epi32(r_I32_V1, Y_R_I32_V), _mm512_mullo_epi32(g_I32_V1, Y_G_I32_V)), _mm512_add_epi32(_mm512_mullo_epi32(b_I32_V1, Y_B_I32_V), Add_I32_V)), Shr);
      __m512i u_I32_V0 = _mm512_srai_epi32(_mm512_add_epi32(_mm512_add_epi32(_mm512_mullo_epi32(r_I32_V0, U_R_I32_V), _mm512_mullo_epi32(g_I32_V0, U_G_I32_V)), _mm512_add_epi32(_mm512_slli_epi32 (b_I32_V0, Shl      ), Add_I32_V)), Shr);
      __m512i u_I32_V1 = _mm512_srai_epi32(_mm512_add_epi32(_mm512_add_epi32(_mm512_mullo_epi32(r_I32_V1, U_R_I32_V), _mm512_mullo_epi32(g_I32_V1, U_G_I32_V)), _mm512_add_epi32(_mm512_slli_epi32 (b_I32_V1, Shl      ), Add_I32_V)), Shr);
      __m512i v_I32_V0 = _mm512_srai_epi32(_mm512_add_epi32(_mm512_add_epi32(_mm512_slli_epi32 (r_I32_V0, Shl      ), _mm512_mullo_epi32(g_I32_V0, V_G_I32_V)), _mm512_add_epi32(_mm512_mullo_epi32(b_I32_V0, V_B_I32_V), Add_I32_V)), Shr);
      __m512i v_I32_V1 = _mm512_srai_epi32(_mm512_add_epi32(_mm512_add_epi32(_mm512_slli_epi32 (r_I32_V1, Shl      ), _mm512_mullo_epi32(g_I32_V1, V_G_I32_V)), _mm512_add_epi32(_mm512_mullo_epi32(b_I32_V1, V_B_I32_V), Add_I32_V)), Shr);

      //change data format (and apply chroma offset) + clip to range 0-Max [_mm256_permute4x64_epi64 used to fix AVX per lane mess]
      __m512i y_U16_V  = _mm512_permutexvar_epi64(PermCtlV, _mm512_packus_epi32(y_I32_V0, y_I32_V1)                                                          );
      __m512i u_U16_V  = _mm512_permutexvar_epi64(PermCtlV, _mm512_packus_epi32(_mm512_add_epi32(u_I32_V0, Mid_I32_V), _mm512_add_epi32(u_I32_V1, Mid_I32_V)));
      __m512i v_U16_V  = _mm512_permutexvar_epi64(PermCtlV, _mm512_packus_epi32(_mm512_add_epi32(v_I32_V0, Mid_I32_V), _mm512_add_epi32(v_I32_V1, Mid_I32_V)));
      __m512i cy_U16_V = _mm512_min_epu16(y_U16_V, Max_U16_V);
      __m512i cu_U16_V = _mm512_min_epu16(u_U16_V, Max_U16_V);
      __m512i cv_U16_V = _mm512_min_epu16(v_U16_V, Max_U16_V);

      //store
      _mm512_storeu_si512((__m512i*)(Y + x), cy_U16_V);
      _mm512_storeu_si512((__m512i*)(U + x), cu_U16_V);
      _mm512_storeu_si512((__m512i*)(V + x), cv_U16_V);
    }
    if(Remainder32)
    {
      //load
      __m512i r_U16_V = _mm512_maskz_loadu_epi16(Mask32, (__m512i*)(R + Width32));
      __m512i g_U16_V = _mm512_maskz_loadu_epi16(Mask32, (__m512i*)(G + Width32));
      __m512i b_U16_V = _mm512_maskz_loadu_epi16(Mask32, (__m512i*)(B + Width32));

      //convert uint16 to int32
      __m512i r_I32_V0 = _mm512_cvtepu16_epi32(_mm512_castsi512_si256   (r_U16_V   ));
      __m512i r_I32_V1 = _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(r_U16_V, 1));
      __m512i g_I32_V0 = _mm512_cvtepu16_epi32(_mm512_castsi512_si256   (g_U16_V   ));
      __m512i g_I32_V1 = _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(g_U16_V, 1));
      __m512i b_I32_V0 = _mm512_cvtepu16_epi32(_mm512_castsi512_si256   (b_U16_V   ));
      __m512i b_I32_V1 = _mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(b_U16_V, 1));

      //convert RGB --> YCbCr
      __m512i y_I32_V0 = _mm512_srai_epi32(_mm512_add_epi32(_mm512_add_epi32(_mm512_mullo_epi32(r_I32_V0, Y_R_I32_V), _mm512_mullo_epi32(g_I32_V0, Y_G_I32_V)), _mm512_add_epi32(_mm512_mullo_epi32(b_I32_V0, Y_B_I32_V), Add_I32_V)), Shr);
      __m512i y_I32_V1 = _mm512_srai_epi32(_mm512_add_epi32(_mm512_add_epi32(_mm512_mullo_epi32(r_I32_V1, Y_R_I32_V), _mm512_mullo_epi32(g_I32_V1, Y_G_I32_V)), _mm512_add_epi32(_mm512_mullo_epi32(b_I32_V1, Y_B_I32_V), Add_I32_V)), Shr);
      __m512i u_I32_V0 = _mm512_srai_epi32(_mm512_add_epi32(_mm512_add_epi32(_mm512_mullo_epi32(r_I32_V0, U_R_I32_V), _mm512_mullo_epi32(g_I32_V0, U_G_I32_V)), _mm512_add_epi32(_mm512_slli_epi32 (b_I32_V0, Shl      ), Add_I32_V)), Shr);
      __m512i u_I32_V1 = _mm512_srai_epi32(_mm512_add_epi32(_mm512_add_epi32(_mm512_mullo_epi32(r_I32_V1, U_R_I32_V), _mm512_mullo_epi32(g_I32_V1, U_G_I32_V)), _mm512_add_epi32(_mm512_slli_epi32 (b_I32_V1, Shl      ), Add_I32_V)), Shr);
      __m512i v_I32_V0 = _mm512_srai_epi32(_mm512_add_epi32(_mm512_add_epi32(_mm512_slli_epi32 (r_I32_V0, Shl      ), _mm512_mullo_epi32(g_I32_V0, V_G_I32_V)), _mm512_add_epi32(_mm512_mullo_epi32(b_I32_V0, V_B_I32_V), Add_I32_V)), Shr);
      __m512i v_I32_V1 = _mm512_srai_epi32(_mm512_add_epi32(_mm512_add_epi32(_mm512_slli_epi32 (r_I32_V1, Shl      ), _mm512_mullo_epi32(g_I32_V1, V_G_I32_V)), _mm512_add_epi32(_mm512_mullo_epi32(b_I32_V1, V_B_I32_V), Add_I32_V)), Shr);

      //change data format (and apply chroma offset) + clip to range 0-Max [_mm256_permute4x64_epi64 used to fix AVX per lane mess]
      __m512i y_U16_V  = _mm512_permutexvar_epi64(PermCtlV, _mm512_packus_epi32(y_I32_V0, y_I32_V1)                                                          );
      __m512i u_U16_V  = _mm512_permutexvar_epi64(PermCtlV, _mm512_packus_epi32(_mm512_add_epi32(u_I32_V0, Mid_I32_V), _mm512_add_epi32(u_I32_V1, Mid_I32_V)));
      __m512i v_U16_V  = _mm512_permutexvar_epi64(PermCtlV, _mm512_packus_epi32(_mm512_add_epi32(v_I32_V0, Mid_I32_V), _mm512_add_epi32(v_I32_V1, Mid_I32_V)));
      __m512i cy_U16_V = _mm512_min_epu16(y_U16_V, Max_U16_V);
      __m512i cu_U16_V = _mm512_min_epu16(u_U16_V, Max_U16_V);
      __m512i cv_U16_V = _mm512_min_epu16(v_U16_V, Max_U16_V);

      //store
      _mm512_mask_storeu_epi16((__m512i*)(Y + Width32), Mask32, cy_U16_V);
      _mm512_mask_storeu_epi16((__m512i*)(U + Width32), Mask32, cu_U16_V);
      _mm512_mask_storeu_epi16((__m512i*)(V + Width32), Mask32, cv_U16_V);

    }
    R += SrcStride; G += SrcStride; B += SrcStride;
    Y += DstStride; U += DstStride; V += DstStride;
  } //y
}
void xColorSpaceAVX512::ConvertYCbCr2RGB_I32(uint16* restrict R, uint16* restrict G, uint16* restrict B, const uint16* Y, const uint16* U, const uint16* V, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
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

  const __m512i R_V_I32_V = _mm512_set1_epi32(R_V);
  const __m512i G_U_I32_V = _mm512_set1_epi32(G_U);
  const __m512i G_V_I32_V = _mm512_set1_epi32(G_V);
  const __m512i B_U_I32_V = _mm512_set1_epi32(B_U);
  const __m512i Add_I32_V = _mm512_set1_epi32(Add);
  const __m512i Mid_I32_V = _mm512_set1_epi32(Mid);
  const __m512i Max_U16_V = _mm512_set1_epi16((uint16)Max);
  const __m512i PermCtlV  = _mm512_setr_epi64(0, 2, 4, 6, 1, 3, 5, 7);

  const int32  Width32     = (int32)((uint32)Width & c_MultipleMask32);
  const uint32 Remainder32 = (uint32)(Width)&c_RemainderMask32;
  const uint32 Mask32      = ((uint32)1 << Remainder32) - 1;  

  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width32; x += 32)
    {
      //load
      __m512i y_U16_V = _mm512_loadu_si512((__m512i*)(Y + x));
      __m512i u_U16_V = _mm512_loadu_si512((__m512i*)(U + x));
      __m512i v_U16_V = _mm512_loadu_si512((__m512i*)(V + x));
      
      //convert uint16 to int32, remove chroma offset
      __m512i y_I32_V0 =                  _mm512_cvtepi16_epi32(_mm512_castsi512_si256   (y_U16_V   ));
      __m512i y_I32_V1 =                  _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(y_U16_V, 1));
      __m512i u_I32_V0 = _mm512_sub_epi32(_mm512_cvtepi16_epi32(_mm512_castsi512_si256   (u_U16_V   )), Mid_I32_V);
      __m512i u_I32_V1 = _mm512_sub_epi32(_mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(u_U16_V, 1)), Mid_I32_V);
      __m512i v_I32_V0 = _mm512_sub_epi32(_mm512_cvtepi16_epi32(_mm512_castsi512_si256   (v_U16_V   )), Mid_I32_V);
      __m512i v_I32_V1 = _mm512_sub_epi32(_mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(v_U16_V, 1)), Mid_I32_V);

      //convert YCbCr --> RGB
      __m512i sy_I32_V0 = _mm512_add_epi32(_mm512_slli_epi32(y_I32_V0, Shr), Add_I32_V);
      __m512i sy_I32_V1 = _mm512_add_epi32(_mm512_slli_epi32(y_I32_V1, Shr), Add_I32_V);

      __m512i r_I32_V0 = _mm512_srai_epi32(_mm512_add_epi32(sy_I32_V0,                                                           _mm512_mullo_epi32(v_I32_V0, R_V_I32_V) ), Shr);
      __m512i r_I32_V1 = _mm512_srai_epi32(_mm512_add_epi32(sy_I32_V1,                                                           _mm512_mullo_epi32(v_I32_V1, R_V_I32_V) ), Shr);
      __m512i g_I32_V0 = _mm512_srai_epi32(_mm512_add_epi32(sy_I32_V0, _mm512_add_epi32(_mm512_mullo_epi32(u_I32_V0, G_U_I32_V), _mm512_mullo_epi32(v_I32_V0, G_V_I32_V))), Shr);
      __m512i g_I32_V1 = _mm512_srai_epi32(_mm512_add_epi32(sy_I32_V1, _mm512_add_epi32(_mm512_mullo_epi32(u_I32_V1, G_U_I32_V), _mm512_mullo_epi32(v_I32_V1, G_V_I32_V))), Shr);
      __m512i b_I32_V0 = _mm512_srai_epi32(_mm512_add_epi32(sy_I32_V0,                  _mm512_mullo_epi32(u_I32_V0, B_U_I32_V)                                          ), Shr);
      __m512i b_I32_V1 = _mm512_srai_epi32(_mm512_add_epi32(sy_I32_V1,                  _mm512_mullo_epi32(u_I32_V1, B_U_I32_V)                                          ), Shr);

      //clip to range 0-Max, convert int32 to uint16 [_mm256_permute4x64_epi64 used to fix AVX per lane mess]
      __m512i r_U16_V  = _mm512_permutexvar_epi64(PermCtlV, _mm512_packus_epi32(r_I32_V0, r_I32_V1));
      __m512i g_U16_V  = _mm512_permutexvar_epi64(PermCtlV, _mm512_packus_epi32(g_I32_V0, g_I32_V1));
      __m512i b_U16_V  = _mm512_permutexvar_epi64(PermCtlV, _mm512_packus_epi32(b_I32_V0, b_I32_V1));
      __m512i cr_U16_V = _mm512_min_epu16(r_U16_V, Max_U16_V);
      __m512i cg_U16_V = _mm512_min_epu16(g_U16_V, Max_U16_V);
      __m512i cb_U16_V = _mm512_min_epu16(b_U16_V, Max_U16_V);

      //store
      _mm512_storeu_si512((__m512i*)(R + x), cr_U16_V);
      _mm512_storeu_si512((__m512i*)(G + x), cg_U16_V);
      _mm512_storeu_si512((__m512i*)(B + x), cb_U16_V);
    }
    if(Remainder32)
    {
      __m512i y_U16_V = _mm512_maskz_loadu_epi16(Mask32, (__m512i*)(Y + Width32));
      __m512i u_U16_V = _mm512_maskz_loadu_epi16(Mask32, (__m512i*)(U + Width32));
      __m512i v_U16_V = _mm512_maskz_loadu_epi16(Mask32, (__m512i*)(V + Width32));
      
      //convert uint16 to int32, remove chroma offset
      __m512i y_I32_V0 =                  _mm512_cvtepi16_epi32(_mm512_castsi512_si256   (y_U16_V   ));
      __m512i y_I32_V1 =                  _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(y_U16_V, 1));
      __m512i u_I32_V0 = _mm512_sub_epi32(_mm512_cvtepi16_epi32(_mm512_castsi512_si256   (u_U16_V   )), Mid_I32_V);
      __m512i u_I32_V1 = _mm512_sub_epi32(_mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(u_U16_V, 1)), Mid_I32_V);
      __m512i v_I32_V0 = _mm512_sub_epi32(_mm512_cvtepi16_epi32(_mm512_castsi512_si256   (v_U16_V   )), Mid_I32_V);
      __m512i v_I32_V1 = _mm512_sub_epi32(_mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(v_U16_V, 1)), Mid_I32_V);

      //convert YCbCr --> RGB
      __m512i sy_I32_V0 = _mm512_add_epi32(_mm512_slli_epi32(y_I32_V0, Shr), Add_I32_V);
      __m512i sy_I32_V1 = _mm512_add_epi32(_mm512_slli_epi32(y_I32_V1, Shr), Add_I32_V);

      __m512i r_I32_V0 = _mm512_srai_epi32(_mm512_add_epi32(sy_I32_V0,                                                           _mm512_mullo_epi32(v_I32_V0, R_V_I32_V) ), Shr);
      __m512i r_I32_V1 = _mm512_srai_epi32(_mm512_add_epi32(sy_I32_V1,                                                           _mm512_mullo_epi32(v_I32_V1, R_V_I32_V) ), Shr);
      __m512i g_I32_V0 = _mm512_srai_epi32(_mm512_add_epi32(sy_I32_V0, _mm512_add_epi32(_mm512_mullo_epi32(u_I32_V0, G_U_I32_V), _mm512_mullo_epi32(v_I32_V0, G_V_I32_V))), Shr);
      __m512i g_I32_V1 = _mm512_srai_epi32(_mm512_add_epi32(sy_I32_V1, _mm512_add_epi32(_mm512_mullo_epi32(u_I32_V1, G_U_I32_V), _mm512_mullo_epi32(v_I32_V1, G_V_I32_V))), Shr);
      __m512i b_I32_V0 = _mm512_srai_epi32(_mm512_add_epi32(sy_I32_V0,                  _mm512_mullo_epi32(u_I32_V0, B_U_I32_V)                                          ), Shr);
      __m512i b_I32_V1 = _mm512_srai_epi32(_mm512_add_epi32(sy_I32_V1,                  _mm512_mullo_epi32(u_I32_V1, B_U_I32_V)                                          ), Shr);

      //clip to range 0-Max, convert int32 to uint16 [_mm256_permute4x64_epi64 used to fix AVX per lane mess]
      __m512i r_U16_V  = _mm512_permutexvar_epi64(PermCtlV, _mm512_packus_epi32(r_I32_V0, r_I32_V1));
      __m512i g_U16_V  = _mm512_permutexvar_epi64(PermCtlV, _mm512_packus_epi32(g_I32_V0, g_I32_V1));
      __m512i b_U16_V  = _mm512_permutexvar_epi64(PermCtlV, _mm512_packus_epi32(b_I32_V0, b_I32_V1));
      __m512i cr_U16_V = _mm512_min_epu16(r_U16_V, Max_U16_V);
      __m512i cg_U16_V = _mm512_min_epu16(g_U16_V, Max_U16_V);
      __m512i cb_U16_V = _mm512_min_epu16(b_U16_V, Max_U16_V);

      //store
      _mm512_mask_storeu_epi16((__m512i*)(R + Width32), Mask32, cr_U16_V);
      _mm512_mask_storeu_epi16((__m512i*)(G + Width32), Mask32, cg_U16_V);
      _mm512_mask_storeu_epi16((__m512i*)(B + Width32), Mask32, cb_U16_V);

    }
    Y += SrcStride; U += SrcStride; V += SrcStride;
    R += DstStride; G += DstStride; B += DstStride;
  } //y
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_AVX512
