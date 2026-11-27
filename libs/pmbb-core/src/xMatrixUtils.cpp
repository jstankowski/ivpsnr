/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define PMBB_xMatrixUtils_IMPLEMENTATION
#include "xMatrixUtils.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xMatrixUtilsNxM
//===============================================================================================================================================================================================================
template<uint32 R, uint32 C> void xMatrixUtilsNxM<R, C>::xConvert_STD(flt32* restrict Dst, const flt64* restrict Src)
{
  for(int32 i = 0; i < c_Area; i++)
  {
    Dst[i] = (flt32)(Src[i]);
  }
}
template<uint32 R, uint32 C> void xMatrixUtilsNxM<R, C>::xConvert_STD(flt64* restrict Dst, const flt32* restrict Src)
{
  for(int32 i = 0; i < c_Area; i++)
  {
    Dst[i] = (flt64)(Src[i]);
  }
}

#if X_CAN_USE_SSE
template<uint32 R, uint32 C> void xMatrixUtilsNxM<R, C>::xConvert_SSE(flt32* restrict Dst, const flt64* restrict Src)
{
  constexpr int32 Area4 = (int32)((uint32)c_Area & c_MultipleMask4<uint32>);

  for(int32 i = 0; i < Area4; i += 4)
  {
    __m128d SrcF64_V0 = _mm_load_pd(Src + i + 0);
    __m128d SrcF64_V1 = _mm_load_pd(Src + i + 2);
    __m128  DstF32_V  = _mm_movelh_ps(_mm_cvtpd_ps(SrcF64_V0), _mm_cvtpd_ps(SrcF64_V1));
    _mm_store_ps(Dst + i, DstF32_V);
  }
  for(int32 i = Area4; i < c_Area; i++)
  {
    Dst[i] = (flt32)(Src[i]);
  }
}
template<uint32 R, uint32 C> void xMatrixUtilsNxM<R, C>::xConvert_SSE(flt64* restrict Dst, const flt32* restrict Src)
{
  constexpr int32 Area4 = (int32)((uint32)c_Area & c_MultipleMask4<uint32>);

  for(int32 i = 0; i < Area4; i += 8)
  {
    __m128  SrcF32_V  = _mm_load_ps(Src + i);
    __m128d DstF64_V0 = _mm_cvtps_pd(SrcF32_V);
    __m128d DstF64_V1 = _mm_cvtps_pd(_mm_movehl_ps(SrcF32_V, SrcF32_V));
    _mm_store_pd(Dst + i + 0, DstF64_V0);
    _mm_store_pd(Dst + i + 2, DstF64_V1);
  }
  for(int32 i = Area4; i < c_Area; i++)
  {
    Dst[i] = (flt64)(Src[i]);
  }
}
#endif //X_CAN_USE_SSE

#if X_CAN_USE_AVX
template<uint32 R, uint32 C> void xMatrixUtilsNxM<R, C>::xConvert_AVX(flt32* restrict Dst, const flt64* restrict Src)
{
  constexpr int32 Area8 = (int32)((uint32)c_Area & c_MultipleMask8<uint32>);

  for(int32 i = 0; i < Area8; i += 8)
  {
    __m256d SrcF64_V0 = _mm256_load_pd(Src + i + 0);
    __m256d SrcF64_V1 = _mm256_load_pd(Src + i + 4);
    __m256  DstF32_V  = _mm256_set_m128(_mm256_cvtpd_ps(SrcF64_V1), _mm256_cvtpd_ps(SrcF64_V0));
    _mm256_store_ps(Dst + i, DstF32_V);
  }
  for(int32 i = Area8; i < c_Area; i++)
  {
    Dst[i] = (flt32)(Src[i]);
  }
}
template<uint32 R, uint32 C> void xMatrixUtilsNxM<R, C>::xConvert_AVX(flt64* restrict Dst, const flt32* restrict Src)
{
  constexpr int32 Area8 = (int32)((uint32)c_Area & c_MultipleMask8<uint32>);

  for(int32 i = 0; i < Area8; i += 8)
  {
    __m256  SrcF32_V  = _mm256_load_ps(Src + i);
    __m256d DstF64_V0 = _mm256_cvtps_pd(_mm256_castps256_ps128(SrcF32_V   ));
    __m256d DstF64_V1 = _mm256_cvtps_pd(_mm256_extractf128_ps (SrcF32_V, 1));
    _mm256_store_pd(Dst + i + 0, DstF64_V0);
    _mm256_store_pd(Dst + i + 4, DstF64_V1);
  }
  for(int32 i = Area8; i < c_Area; i++)
  {
    Dst[i] = (flt64)(Src[i]);
  }
}
#endif //X_CAN_USE_AVX

//===============================================================================================================================================================================================================
// xMatrixUtils4x4
//===============================================================================================================================================================================================================
void xMatrixUtils4x4::xMatrixMultiply4x4_STD(xM4x4F* restrict Dst, const xM4x4F* restrict A, const xM4x4F* restrict B)
{
  xM4x4F Tmp;
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      Tmp.M[i][j] = (A->M[i][0]*B->M[0][j] + A->M[i][1]*B->M[1][j]) + (A->M[i][2]*B->M[2][j] + A->M[i][3]*B->M[3][j]);
    }
  }
  *Dst = Tmp;
}
void xMatrixUtils4x4::xMatrixConvert4x4_STD(xM4x4F* restrict Dst, const xM4x4D* restrict Src)
{
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      Dst->M[j][i] = (float)(Src->M[j][i]);
    }
  }
}
void xMatrixUtils4x4::xMatrixConvert4x4_STD(xM4x4D* restrict Dst, const xM4x4F* restrict Src)
{
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      Dst->M[j][i] = (double)(Src->M[j][i]);
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if X_CAN_USE_SSE
void xMatrixUtils4x4::xMatrixMultiply4x4_SSE(xM4x4F* restrict Dst, const xM4x4F* restrict A, const xM4x4F* restrict B)
{
  xM4x4F Tmp;
  Tmp.R[0] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(A->R[0], A->R[0], 0x00), B->R[0]),  //A0 * B00, A0 * B01, A0 * B02, A0 * B03 
                          _mm_mul_ps(_mm_shuffle_ps(A->R[0], A->R[0], 0x55), B->R[1])), //A1 * B04, A1 * B05, A1 * B06, A1 * B07
                        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(A->R[0], A->R[0], 0xaa), B->R[2]),  //A2 * B08, A2 * B09, A2 * B10, A2 * B11
                          _mm_mul_ps(_mm_shuffle_ps(A->R[0], A->R[0], 0xff), B->R[3])));//A3 * B12, A3 * B13, A3 * B14, A3 * B15
  Tmp.R[1] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(A->R[1], A->R[1], 0x00), B->R[0]),   
                          _mm_mul_ps(_mm_shuffle_ps(A->R[1], A->R[1], 0x55), B->R[1])), 
                        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(A->R[1], A->R[1], 0xaa), B->R[2]),  
                          _mm_mul_ps(_mm_shuffle_ps(A->R[1], A->R[1], 0xff), B->R[3])));
  Tmp.R[2] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(A->R[2], A->R[2], 0x00), B->R[0]),   
                          _mm_mul_ps(_mm_shuffle_ps(A->R[2], A->R[2], 0x55), B->R[1])), 
                        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(A->R[2], A->R[2], 0xaa), B->R[2]),  
                          _mm_mul_ps(_mm_shuffle_ps(A->R[2], A->R[2], 0xff), B->R[3])));
  Tmp.R[3] = _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(A->R[3], A->R[3], 0x00), B->R[0]),   
                          _mm_mul_ps(_mm_shuffle_ps(A->R[3], A->R[3], 0x55), B->R[1])), 
                        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(A->R[3], A->R[3], 0xaa), B->R[2]),  
                          _mm_mul_ps(_mm_shuffle_ps(A->R[3], A->R[3], 0xff), B->R[3])));
  *Dst = Tmp;
}
void xMatrixUtils4x4::xMatrixConvert4x4_SSE(xM4x4F* restrict Dst, const xM4x4D* restrict Src)
{
  Dst->R[0] = _mm_movelh_ps(_mm_cvtpd_ps(Src->R[0]), _mm_cvtpd_ps(Src->R[1]));
  Dst->R[1] = _mm_movelh_ps(_mm_cvtpd_ps(Src->R[2]), _mm_cvtpd_ps(Src->R[3]));
  Dst->R[2] = _mm_movelh_ps(_mm_cvtpd_ps(Src->R[4]), _mm_cvtpd_ps(Src->R[5]));
  Dst->R[3] = _mm_movelh_ps(_mm_cvtpd_ps(Src->R[6]), _mm_cvtpd_ps(Src->R[7]));
}
void xMatrixUtils4x4::xMatrixConvert4x4_SSE(xM4x4D* restrict Dst, const xM4x4F* restrict Src)
{
  Dst->R[0] = _mm_cvtps_pd(Src->R[0]                          );
  Dst->R[1] = _mm_cvtps_pd(_mm_movehl_ps(Src->R[0], Src->R[0]));
  Dst->R[2] = _mm_cvtps_pd(Src->R[1]                          );
  Dst->R[3] = _mm_cvtps_pd(_mm_movehl_ps(Src->R[1], Src->R[1]));
  Dst->R[4] = _mm_cvtps_pd(Src->R[2]                          );
  Dst->R[5] = _mm_cvtps_pd(_mm_movehl_ps(Src->R[2], Src->R[2]));
  Dst->R[6] = _mm_cvtps_pd(Src->R[3]                          );
  Dst->R[7] = _mm_cvtps_pd(_mm_movehl_ps(Src->R[3], Src->R[3]));
}
#endif //X_CAN_USE_SSE

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if X_CAN_USE_AVX
void xMatrixUtils4x4::xMatrixMultiply4x4_AVX(xM4x4F* restrict Dst, const xM4x4F* restrict A, const xM4x4F* restrict B)
{
  xM4x4F Tmp;
  Tmp.DR[0] = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(_mm256_shuffle_ps(A->DR[0], A->DR[0], 0x00), _mm256_broadcast_ps(&(B->R[0]))),
    _mm256_mul_ps(_mm256_shuffle_ps(A->DR[0], A->DR[0], 0x55), _mm256_broadcast_ps(&(B->R[1])))),
    _mm256_add_ps(_mm256_mul_ps(_mm256_shuffle_ps(A->DR[0], A->DR[0], 0xaa), _mm256_broadcast_ps(&(B->R[2]))),
      _mm256_mul_ps(_mm256_shuffle_ps(A->DR[0], A->DR[0], 0xff), _mm256_broadcast_ps(&(B->R[3])))));

  Tmp.DR[1] = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(_mm256_shuffle_ps(A->DR[1], A->DR[1], 0x00), _mm256_broadcast_ps(&(B->R[0]))),
    _mm256_mul_ps(_mm256_shuffle_ps(A->DR[1], A->DR[1], 0x55), _mm256_broadcast_ps(&(B->R[1])))),
    _mm256_add_ps(_mm256_mul_ps(_mm256_shuffle_ps(A->DR[1], A->DR[1], 0xaa), _mm256_broadcast_ps(&(B->R[2]))),
      _mm256_mul_ps(_mm256_shuffle_ps(A->DR[1], A->DR[1], 0xff), _mm256_broadcast_ps(&(B->R[3])))));
  *Dst = Tmp;
}
void xMatrixUtils4x4::xMatrixConvert4x4_AVX(xM4x4F* restrict Dst, const xM4x4D* restrict Src)
{
  Dst->DR[0] = _mm256_set_m128(_mm256_cvtpd_ps(Src->DR[1]), _mm256_cvtpd_ps(Src->DR[0]));
  Dst->DR[1] = _mm256_set_m128(_mm256_cvtpd_ps(Src->DR[3]), _mm256_cvtpd_ps(Src->DR[2]));
  //Dst->R[0] = _mm256_cvtpd_ps(Src->DR[0]);
  //Dst->R[1] = _mm256_cvtpd_ps(Src->DR[1]);
  //Dst->R[2] = _mm256_cvtpd_ps(Src->DR[2]);
  //Dst->R[3] = _mm256_cvtpd_ps(Src->DR[3]);
}
void xMatrixUtils4x4::xMatrixConvert4x4_AVX(xM4x4D* restrict Dst, const xM4x4F* restrict Src)
{
  Dst->DR[0] = _mm256_cvtps_pd(Src->R[0]);
  Dst->DR[1] = _mm256_cvtps_pd(Src->R[1]);
  Dst->DR[2] = _mm256_cvtps_pd(Src->R[2]);
  Dst->DR[3] = _mm256_cvtps_pd(Src->R[3]);
}
#endif //X_CAN_USE_AVX

//===============================================================================================================================================================================================================

} //end of namespace PMBB
