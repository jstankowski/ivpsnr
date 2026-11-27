/*
    SPDX-FileCopyrightText: 2026 Blazej Szydelko <blazej.szydelko@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefGSR.h"
#include "xExponential.h"

#if X_SIMD_CAN_USE_AVX512
#include <immintrin.h>
#include <limits>

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================

template<bool BoundChecking, bool UseFMA> class xExponentialAVX512
{
public:
  static __m512 SchraudolphExp         (__m512 x); //Schraudolph exponential [N. Schraudolph, "A Fast, Compact Approximation of the Exponential Function", Neural Computation 11, 853�862(1999)]
  static __m512 SchraudolphExpCorrected(__m512 x); //Schraudolph exponential wit 2-nd order correction [based on https://specbranch.com/posts/fast-exp/]
  static __m512 PreciseExp             (__m512 x); //Cody-Waite Reduction with a Remez Minimax Polynomial evaluated via Estrin's Method
  template<int32 Degree> static __m512 DomainExpEstrin(__m512 x); //Degree-selectable polynomial fit to exp(x) for x in [-5.545, 0] (matches c_PowerThreshold) - see xDomainExpCoeffs32 in xMathFunctions.h
  template<int32 Degree> static __m512 DomainExpHorner(__m512 x); //Same fit as DomainExpEstrin, evaluated via plain Horner's method instead
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<bool BoundChecking, bool UseFMA> __m512 xExponentialAVX512<BoundChecking, UseFMA>::SchraudolphExp(__m512 x)
{
  const __m512 A = _mm512_set1_ps((1 << 23) / 0.69314718f);
  const __m512 B = _mm512_set1_ps((1 << 23) * (127 - 0.043677448f));

  __m512 t;
  if constexpr(UseFMA) { t = _mm512_fmadd_ps            (A, x , B); }
  else                 { t = _mm512_add_ps(_mm512_mul_ps(A, x), B); }

  if constexpr(BoundChecking)
  {
    const __m512 C = _mm512_set1_ps((flt32)(1 << 23));
    const __m512 D = _mm512_set1_ps((flt32)(1 << 23) * 255.0f);

    const __mmask16 UnderflowMask = _mm512_cmp_ps_mask(t, C, _CMP_LT_OQ);
    t = _mm512_mask_blend_ps(UnderflowMask, t, _mm512_setzero_ps()); // x < c -> 0
    t = _mm512_min_ps(t, D);                                         // x > d -> d
  }

  const __m512i tInt = _mm512_cvttps_epi32(t);
  return _mm512_castsi512_ps(tInt);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<bool BoundChecking, bool UseFMA> __m512 xExponentialAVX512<BoundChecking, UseFMA>::SchraudolphExpCorrected(__m512 x)
{
  constexpr int32 K32_TUNED_MINIMAX = 366393;
  constexpr int32 Add32 = (127 << 23) - K32_TUNED_MINIMAX;

  constexpr flt32 P0 = 1.469318866729736328125f;
  constexpr flt32 P1 = -0.671999752521514892578125f;
  constexpr flt32 P2 = 0.22670517861843109130859375f;

  // Piecewise linear approximation
  const __m512  x_base      = _mm512_mul_ps(x, _mm512_set1_ps(12102203.0f)); // multiply by log2(e) * 2^23
  const __m512i i_base      = _mm512_cvttps_epi32(x_base);                  // convert to Q9.23 fixed point, rounding down
  const __m512i i_result    = _mm512_add_epi32(i_base, _mm512_set1_epi32(Add32));
  const __m512  first_order = _mm512_castsi512_ps(i_result);

  // Install 127 as the exponent of i_result, placing correction_x in [1, 2)
  __m512i correction_xi = _mm512_and_si512(i_result, _mm512_set1_epi32(0x7fffff));
  correction_xi = _mm512_or_si512(correction_xi, _mm512_set1_epi32(127 << 23));
  const __m512 correction_x = _mm512_castsi512_ps(correction_xi);

  // 2nd order correction factor, evaluated with a Horner scheme ((x * P2 + P1) * x + P0)
  __m512 Correction;
  if constexpr(UseFMA) { Correction = _mm512_fmadd_ps(_mm512_fmadd_ps(correction_x, _mm512_set1_ps(P2), _mm512_set1_ps(P1)), correction_x, _mm512_set1_ps(P0));  }
  else                 { Correction = _mm512_add_ps(_mm512_mul_ps(_mm512_add_ps(_mm512_mul_ps(correction_x, _mm512_set1_ps(P2)), _mm512_set1_ps(P1)), correction_x), _mm512_set1_ps(P0)); }

  const __m512 Result = _mm512_mul_ps(first_order, Correction);
  return Result;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<bool BoundChecking, bool UseFMA> __m512 xExponentialAVX512<BoundChecking, UseFMA>::PreciseExp(__m512 x)
{
  const __m512 LOG2E = _mm512_set1_ps(1.4426950408889634073599246810018921374219f);
  const __m512 LN2H  = _mm512_set1_ps(0.693145751953125f);
  const __m512 LN2L  = _mm512_set1_ps(1.4286068203094172e-6f);
  const __m512 C5    = _mm512_set1_ps(0.008333359040319919586181640625f);
  const __m512 C4    = _mm512_set1_ps(0.0416664816439151763916015625f);
  const __m512 C3    = _mm512_set1_ps(0.16666667163372039794921875f);
  const __m512 C2    = _mm512_set1_ps(0.4999999701976776123046875f);
  const __m512 C1    = _mm512_set1_ps(1.0f);
  const __m512 C0    = _mm512_set1_ps(1.0f);

  //Step 1 = High-Precision Cody-Waite Range Reduction
  const __m512 k = _mm512_roundscale_ps(_mm512_mul_ps(x, LOG2E), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
  //Cody-Waite range reduction: s = x - k*c1 - k*c2
  __m512 s;
  if constexpr(UseFMA) { s = _mm512_fnmadd_ps(k, LN2L, _mm512_fnmadd_ps(k, LN2H, x)); }
  else                 { s = _mm512_sub_ps(_mm512_sub_ps(x, _mm512_mul_ps(k, LN2H)), _mm512_mul_ps(k, LN2L)); }

  //Step 2 = Remez Minimax Polynomial (Estrin's Parallel Tree Evaluation)  
  const __m512 s2 = _mm512_mul_ps(s, s);

  __m512 poly;
  if constexpr(UseFMA)
  {
    const __m512 p01   = _mm512_fmadd_ps(s, C1, C0);
    const __m512 p23   = _mm512_fmadd_ps(s, C3, C2);
    const __m512 p45   = _mm512_fmadd_ps(s, C5, C4);
    const __m512 p2345 = _mm512_fmadd_ps(s2, p45, p23);
    poly = _mm512_fmadd_ps(s2, p2345, p01);
  }
  else
  {
    const __m512 p01   = _mm512_add_ps(_mm512_mul_ps(s, C1), C0);
    const __m512 p23   = _mm512_add_ps(_mm512_mul_ps(s, C3), C2);
    const __m512 p45   = _mm512_add_ps(_mm512_mul_ps(s, C5), C4);
    const __m512 p2345 = _mm512_add_ps(_mm512_mul_ps(s2, p45), p23);
    poly = _mm512_add_ps(_mm512_mul_ps(s2, p2345), p01);
  }

  // Step 3 - multiply poly by 2^k via bit manipulation
  const __m512i kInt       = _mm512_cvtps_epi32(k);         // convert (k is already integer-valued, so exact)
  const __m512i expOffset  = _mm512_slli_epi32(kInt, 23);   // shift into the float32 exponent bit position
  const __m512i polyBits   = _mm512_castps_si512(poly);     // reinterpret bits, no value change
  const __m512i resultBits = _mm512_add_epi32(polyBits, expOffset);
  __m512 Result = _mm512_castsi512_ps(resultBits);          // reinterpret back to float

  if constexpr(BoundChecking)
  {
    const __mmask16 OverflowMask  = _mm512_cmp_ps_mask(x, _mm512_set1_ps(88.72283935546875f),    _CMP_GT_OQ);
    const __mmask16 UnderflowMask = _mm512_cmp_ps_mask(x, _mm512_set1_ps(-103.972076416015625f), _CMP_LT_OQ);
    const __mmask16 NanMask       = _mm512_cmp_ps_mask(x, x, _CMP_UNORD_Q); // x is NaN iff x!=x ("unordered")

    Result = _mm512_mask_blend_ps(OverflowMask , Result, _mm512_set1_ps(std::numeric_limits<flt32>::infinity()));
    Result = _mm512_mask_blend_ps(UnderflowMask, Result, _mm512_setzero_ps());
    Result = _mm512_mask_blend_ps(NanMask      , Result, x); // pass NaN through
  }

  return Result;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<bool BoundChecking, bool UseFMA> template<int32 Degree> __m512 xExponentialAVX512<BoundChecking, UseFMA>::DomainExpEstrin(__m512 x)
{
  constexpr auto C = xDomainExpCoeffs32<Degree>();
  const __m512 x2 = _mm512_mul_ps(x, x);

  if constexpr(Degree == 5)
  {
    __m512 p01, p23, p45;
    if constexpr(UseFMA)
    {
      p01 = _mm512_fmadd_ps(_mm512_set1_ps(C[1]), x, _mm512_set1_ps(C[0]));
      p23 = _mm512_fmadd_ps(_mm512_set1_ps(C[3]), x, _mm512_set1_ps(C[2]));
      p45 = _mm512_fmadd_ps(_mm512_set1_ps(C[5]), x, _mm512_set1_ps(C[4]));
    }
    else
    {
      p01 = _mm512_add_ps(_mm512_set1_ps(C[0]), _mm512_mul_ps(_mm512_set1_ps(C[1]), x));
      p23 = _mm512_add_ps(_mm512_set1_ps(C[2]), _mm512_mul_ps(_mm512_set1_ps(C[3]), x));
      p45 = _mm512_add_ps(_mm512_set1_ps(C[4]), _mm512_mul_ps(_mm512_set1_ps(C[5]), x));
    }
    const __m512 x4 = _mm512_mul_ps(x2, x2);
    __m512 p03; if constexpr(UseFMA) { p03 = _mm512_fmadd_ps(x2, p23, p01); } else { p03 = _mm512_add_ps(p01, _mm512_mul_ps(x2, p23)); }
    __m512 Result; if constexpr(UseFMA) { Result = _mm512_fmadd_ps(x4, p45, p03); } else { Result = _mm512_add_ps(p03, _mm512_mul_ps(x4, p45)); }
    return Result;
  }
  else if constexpr(Degree == 6)
  {
    __m512 p01, p23, p45;
    if constexpr(UseFMA)
    {
      p01 = _mm512_fmadd_ps(_mm512_set1_ps(C[1]), x, _mm512_set1_ps(C[0]));
      p23 = _mm512_fmadd_ps(_mm512_set1_ps(C[3]), x, _mm512_set1_ps(C[2]));
      p45 = _mm512_fmadd_ps(_mm512_set1_ps(C[5]), x, _mm512_set1_ps(C[4]));
    }
    else
    {
      p01 = _mm512_add_ps(_mm512_set1_ps(C[0]), _mm512_mul_ps(_mm512_set1_ps(C[1]), x));
      p23 = _mm512_add_ps(_mm512_set1_ps(C[2]), _mm512_mul_ps(_mm512_set1_ps(C[3]), x));
      p45 = _mm512_add_ps(_mm512_set1_ps(C[4]), _mm512_mul_ps(_mm512_set1_ps(C[5]), x));
    }
    __m512 p03, p46;
    if constexpr(UseFMA)
    {
      p03 = _mm512_fmadd_ps(x2, p23, p01);
      p46 = _mm512_fmadd_ps(x2, _mm512_set1_ps(C[6]), p45);
    }
    else
    {
      p03 = _mm512_add_ps(p01, _mm512_mul_ps(x2, p23));
      p46 = _mm512_add_ps(p45, _mm512_mul_ps(x2, _mm512_set1_ps(C[6])));
    }
    const __m512 x4 = _mm512_mul_ps(x2, x2);
    __m512 Result; if constexpr(UseFMA) { Result = _mm512_fmadd_ps(x4, p46, p03); } else { Result = _mm512_add_ps(p03, _mm512_mul_ps(x4, p46)); }
    return Result;
  }
  else if constexpr(Degree == 7)
  {
    __m512 p01, p23, p45, p67;
    if constexpr(UseFMA)
    {
      p01 = _mm512_fmadd_ps(_mm512_set1_ps(C[1]), x, _mm512_set1_ps(C[0]));
      p23 = _mm512_fmadd_ps(_mm512_set1_ps(C[3]), x, _mm512_set1_ps(C[2]));
      p45 = _mm512_fmadd_ps(_mm512_set1_ps(C[5]), x, _mm512_set1_ps(C[4]));
      p67 = _mm512_fmadd_ps(_mm512_set1_ps(C[7]), x, _mm512_set1_ps(C[6]));
    }
    else
    {
      p01 = _mm512_add_ps(_mm512_set1_ps(C[0]), _mm512_mul_ps(_mm512_set1_ps(C[1]), x));
      p23 = _mm512_add_ps(_mm512_set1_ps(C[2]), _mm512_mul_ps(_mm512_set1_ps(C[3]), x));
      p45 = _mm512_add_ps(_mm512_set1_ps(C[4]), _mm512_mul_ps(_mm512_set1_ps(C[5]), x));
      p67 = _mm512_add_ps(_mm512_set1_ps(C[6]), _mm512_mul_ps(_mm512_set1_ps(C[7]), x));
    }
    __m512 p03, p47;
    if constexpr(UseFMA)
    {
      p03 = _mm512_fmadd_ps(x2, p23, p01);
      p47 = _mm512_fmadd_ps(x2, p67, p45);
    }
    else
    {
      p03 = _mm512_add_ps(p01, _mm512_mul_ps(x2, p23));
      p47 = _mm512_add_ps(p45, _mm512_mul_ps(x2, p67));
    }
    const __m512 x4 = _mm512_mul_ps(x2, x2);
    __m512 Result; if constexpr(UseFMA) { Result = _mm512_fmadd_ps(x4, p47, p03); } else { Result = _mm512_add_ps(p03, _mm512_mul_ps(x4, p47)); }
    return Result;
  }
  else //Degree == 8
  {
    __m512 p01, p23, p45, p67;
    if constexpr(UseFMA)
    {
      p01 = _mm512_fmadd_ps(_mm512_set1_ps(C[1]), x, _mm512_set1_ps(C[0]));
      p23 = _mm512_fmadd_ps(_mm512_set1_ps(C[3]), x, _mm512_set1_ps(C[2]));
      p45 = _mm512_fmadd_ps(_mm512_set1_ps(C[5]), x, _mm512_set1_ps(C[4]));
      p67 = _mm512_fmadd_ps(_mm512_set1_ps(C[7]), x, _mm512_set1_ps(C[6]));
    }
    else
    {
      p01 = _mm512_add_ps(_mm512_set1_ps(C[0]), _mm512_mul_ps(_mm512_set1_ps(C[1]), x));
      p23 = _mm512_add_ps(_mm512_set1_ps(C[2]), _mm512_mul_ps(_mm512_set1_ps(C[3]), x));
      p45 = _mm512_add_ps(_mm512_set1_ps(C[4]), _mm512_mul_ps(_mm512_set1_ps(C[5]), x));
      p67 = _mm512_add_ps(_mm512_set1_ps(C[6]), _mm512_mul_ps(_mm512_set1_ps(C[7]), x));
    }
    __m512 p03, p47;
    if constexpr(UseFMA)
    {
      p03 = _mm512_fmadd_ps(x2, p23, p01);
      p47 = _mm512_fmadd_ps(x2, p67, p45);
    }
    else
    {
      p03 = _mm512_add_ps(p01, _mm512_mul_ps(x2, p23));
      p47 = _mm512_add_ps(p45, _mm512_mul_ps(x2, p67));
    }
    //C[8] is a lone leftover term - fold it in with one more Horner step reusing x4 instead of computing an independent x8=x4*x4
    const __m512 x4 = _mm512_mul_ps(x2, x2);
    __m512 F1; if constexpr(UseFMA) { F1 = _mm512_fmadd_ps(x4, _mm512_set1_ps(C[8]), p47); } else { F1 = _mm512_add_ps(p47, _mm512_mul_ps(x4, _mm512_set1_ps(C[8]))); }
    __m512 Result; if constexpr(UseFMA) { Result = _mm512_fmadd_ps(x4, F1, p03); } else { Result = _mm512_add_ps(p03, _mm512_mul_ps(x4, F1)); }
    return Result;
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<bool BoundChecking, bool UseFMA> template<int32 Degree> __m512 xExponentialAVX512<BoundChecking, UseFMA>::DomainExpHorner(__m512 x)
{
  constexpr auto C = xDomainExpCoeffs32<Degree>();
  __m512 Result = _mm512_set1_ps(C[Degree]);
  for(int32 i = Degree - 1; i >= 0; i--)
  {
    const __m512 Ci = _mm512_set1_ps(C[i]);
    if constexpr(UseFMA) { Result = _mm512_fmadd_ps(Result, x, Ci)               ; }
    else                 { Result = _mm512_add_ps  (_mm512_mul_ps(Result, x), Ci); }
  }
  return Result;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB_NAMESPACE::GSR

#endif //X_SIMD_CAN_USE_AVX512
