/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xMathUtils.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

flt64 xKBNS::Accumulate(const flt64* x, const uintSize n)
{
  if(n == 0) { return 0.0; }

  flt64 s = x[0];
  flt64 c = 0;
  for(uintSize i = 1; i < n; i++)
  {
    flt64 t = s + x[i];
    if(xAbs(s) >= xAbs(x[i])) { c += ((s - t) + x[i]); }
    else                      { c += ((x[i] - t) + s); }
    s = t;
  }
  return s + c;
}

flt64V4 xKBNS::Accumulate(const flt64V4* v, const uintSize n)
{
  xKBNS Acc[4];

  for(uintSize i = 0; i < n; i++)
  {
    const flt64V4& x = v[i];
    for(int32 j = 0; j < 4; j++) { Acc[j].acc(x[j]); }
  }

  flt64V4 Sum = { Acc[0].result(), Acc[1].result(), Acc[2].result(), Acc[3].result() };

  return Sum;
}

//===============================================================================================================================================================================================================

void xKBNS4_STD::acc(const flt64V4& v)
{
  flt64V4 t = s + v;
  for(int32 j = 0; j < 4; j++)
  {
    if(xAbs(s[j]) >= xAbs(v[j])) { c[j] += ((s[j] - t[j]) + v[j]); }
    else                         { c[j] += ((v[j] - t[j]) + s[j]); }
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
      if(xAbs(s[j]) >= xAbs(v[j])) { c[j] += ((s[j] - t[j]) + v[j]); }
      else                         { c[j] += ((v[j] - t[j]) + s[j]); }
    }
    s = t;
  }
}

//===============================================================================================================================================================================================================

#if X_SIMD_CAN_USE_AVX
static inline __m256d _mm256_abs_pd(__m256d v)
{
  const __m256d SignMask = _mm256_set1_pd(-0.); // -0. = 1 << 63
  return _mm256_andnot_pd(SignMask, v); // !sign_mask & x
}
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
void xKBNS4_AVX::xAcc(const tF64V4* x, const uintSize n)
{
  for(uintSize i = 0; i < n; i++) { xAcc(x[i]); }
}
#endif //X_SIMD_CAN_USE_AVX

//===============================================================================================================================================================================================================

} //end of namespace PMBB