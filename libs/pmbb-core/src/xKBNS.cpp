/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xKBNS.h"
#include "xHelpersSIMD.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

#if X_SIMD_CAN_USE_SSE
void xKBNS1_SSE::xAcc(flt64 v)
{
  __m128d V_F64_S    = _mm_set_sd   (v);
  __m128d T_F64_S    = _mm_add_sd   (m_s.R, V_F64_S);
  __m128d AbsS_F64_S = _mm_abs_sd   (m_s.R);
  __m128d AbsV_F64_S = _mm_abs_sd   (V_F64_S);
  __m128d Mask_F64_S = _mm_cmpge_sd (AbsS_F64_S, AbsV_F64_S); //xAbs(s[i]) >= xAbs(v[i])
  __m128d CT_F64_S   = _mm_add_pd   (_mm_sub_pd(m_s.R  , T_F64_S), V_F64_S);
  __m128d CF_F64_S   = _mm_add_pd   (_mm_sub_pd(V_F64_S, T_F64_S), m_s.R  );
  __m128d C_F64_S    = _mm_blendv_pd(CF_F64_S, CT_F64_S, Mask_F64_S);
  m_c.R = _mm_add_pd(m_c.R, C_F64_S);
  m_s.R = T_F64_S;
}
#endif //X_SIMD_CAN_USE_SSE

//===============================================================================================================================================================================================================

void xKBNS4_STD::acc(const flt64V4& v)
{
  flt64V4 t = s + v;
  for(int32 j = 0; j < 4; j++)
  {
    if(std::abs(s[j]) >= std::abs(v[j])) { c[j] += ((s[j] - t[j]) + v[j]); }
    else                                 { c[j] += ((v[j] - t[j]) + s[j]); }
  }
  s = t;
}
void xKBNS4_STD::acc(const flt64V4* x, const uintSize n)
{
  for(uintSize i = 0; i < n; i++)
  {
    flt64V4 v = x[i];
    flt64V4 t = s + v;
    for(int32 j = 0; j < 4; j++)
    {
      if(std::abs(s[j]) >= std::abs(v[j])) { c[j] += ((s[j] - t[j]) + v[j]); }
      else                                 { c[j] += ((v[j] - t[j]) + s[j]); }
    }
    s = t;
  }
}

//===============================================================================================================================================================================================================

#if X_SIMD_CAN_USE_SSE
void xKBNS4_SSE::xAcc(const tF64V4& v)
{
  __m128d T_F64_V0    = _mm_add_pd   (s.R[0], v.R[0]);
  __m128d T_F64_V1    = _mm_add_pd   (s.R[1], v.R[1]);
  __m128d AbsS_F64_V0 = _mm_abs_pd   (s.R[0]);
  __m128d AbsS_F64_V1 = _mm_abs_pd   (s.R[1]);
  __m128d AbsV_F64_V0 = _mm_abs_pd   (v.R[0]);
  __m128d AbsV_F64_V1 = _mm_abs_pd   (v.R[1]);
  __m128d Mask_F64_V0 = _mm_cmpge_pd (AbsS_F64_V0, AbsV_F64_V0); //xAbs(s[i]) >= xAbs(v[i])
  __m128d Mask_F64_V1 = _mm_cmpge_pd (AbsS_F64_V1, AbsV_F64_V1); //xAbs(s[i]) >= xAbs(v[i])
  __m128d CT_F64_V0   = _mm_add_pd   (_mm_sub_pd(s.R[0], T_F64_V0), v.R[0]);
  __m128d CT_F64_V1   = _mm_add_pd   (_mm_sub_pd(s.R[1], T_F64_V1), v.R[1]);
  __m128d CF_F64_V0   = _mm_add_pd   (_mm_sub_pd(v.R[0], T_F64_V0), s.R[0]);
  __m128d CF_F64_V1   = _mm_add_pd   (_mm_sub_pd(v.R[1], T_F64_V1), s.R[1]);
  __m128d C_F64_V0    = _mm_blendv_pd(CF_F64_V0, CT_F64_V0, Mask_F64_V0);
  __m128d C_F64_V1    = _mm_blendv_pd(CF_F64_V1, CT_F64_V1, Mask_F64_V1);
  c.R[0] = _mm_add_pd(c.R[0], C_F64_V0);
  c.R[1] = _mm_add_pd(c.R[1], C_F64_V1);
  s.R[0] = T_F64_V0;
  s.R[1] = T_F64_V1;
}
#endif //X_SIMD_CAN_USE_SSE

//===============================================================================================================================================================================================================

#if X_SIMD_CAN_USE_AVX
void xKBNS4_AVX::xAcc(const tF64V4& v)
{
  __m256d T_F64_V    = _mm256_add_pd(s.R, v.R);
  __m256d AbsS_F64_V = _mm256_abs_pd(s.R);
  __m256d AbsV_F64_V = _mm256_abs_pd(v.R);
  __m256d Mask_F64_V = _mm256_cmp_pd(AbsS_F64_V, AbsV_F64_V, _CMP_GE_OQ); //xAbs(s[i]) >= xAbs(v[i])
  __m256d CT_F64_V   = _mm256_add_pd(_mm256_sub_pd(s.R, T_F64_V), v.R);
  __m256d CF_F64_V   = _mm256_add_pd(_mm256_sub_pd(v.R, T_F64_V), s.R);
  __m256d C_F64_V    = _mm256_blendv_pd(CF_F64_V, CT_F64_V, Mask_F64_V);
  c.R = _mm256_add_pd(c.R, C_F64_V);
  s.R = T_F64_V;
}
#endif //X_SIMD_CAN_USE_AVX

//===============================================================================================================================================================================================================

} //end of namespace PMBB