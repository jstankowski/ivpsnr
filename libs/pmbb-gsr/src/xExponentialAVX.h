/*
    SPDX-FileCopyrightText: 2026 Blazej Szydelko <blazej.szydelko@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefGSR.h"
#include "xExponential.h"

#if X_SIMD_CAN_USE_AVX
#include <immintrin.h>
#include <limits>

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================

template<bool BoundChecking, bool UseFMA> class xExponentialAVX
{
public:
  static __m256 SchraudolphExp         (__m256 x); //Schraudolph exponential [N. Schraudolph, "A Fast, Compact Approximation of the Exponential Function", Neural Computation 11, 853-862(1999)]
  static __m256 SchraudolphExpCorrected(__m256 x); //Schraudolph exponential wit 2-nd order correction [based on https://specbranch.com/posts/fast-exp/]
  static __m256 PreciseExp             (__m256 x); //Cody-Waite Reduction with a Remez Minimax Polynomial evaluated via Estrin's Method
  template<int32 Degree> static __m256 DomainExpEstrin(__m256 x); //Degree-selectable polynomial fit to exp(x) for x in [-5.545, 0] (matches c_PowerThreshold) - see xDomainExpCoeffs32 in xMathFunctions.h
  template<int32 Degree> static __m256 DomainExpHorner(__m256 x); //Same fit as DomainExpEstrin, evaluated via plain Horner's method instead
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<bool BoundChecking, bool UseFMA> __m256 xExponentialAVX<BoundChecking, UseFMA>::SchraudolphExp(__m256 x)
{
  const __m256 A = _mm256_set1_ps((1 << 23) / 0.69314718f);
  const __m256 B = _mm256_set1_ps((1 << 23) * (127 - 0.043677448f));

  __m256 t;
  if constexpr(UseFMA) { t = _mm256_fmadd_ps            (A, x , B); }
  else                 { t = _mm256_add_ps(_mm256_mul_ps(A, x), B); }

  if constexpr(BoundChecking)
  {
    const __m256 C = _mm256_set1_ps((flt32)(1 << 23));
    const __m256 D = _mm256_set1_ps((flt32)(1 << 23) * 255.0f);

    const __m256 UnderflowMask = _mm256_cmp_ps(t, C, _CMP_LT_OQ);
    t = _mm256_blendv_ps(t, _mm256_setzero_ps(), UnderflowMask); // x < c -> 0
    t = _mm256_min_ps   (t, D);                                  // x > d -> d
  }

  const __m256i tInt = _mm256_cvttps_epi32(t);
  return _mm256_castsi256_ps(tInt);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<bool BoundChecking, bool UseFMA> __m256 xExponentialAVX<BoundChecking, UseFMA>::SchraudolphExpCorrected(__m256 x)
{
  constexpr int32 K32_TUNED_MINIMAX = 366393;
  constexpr int32 Add32 = (127 << 23) - K32_TUNED_MINIMAX;

  constexpr flt32 P0 = 1.469318866729736328125f;
  constexpr flt32 P1 = -0.671999752521514892578125f;
  constexpr flt32 P2 = 0.22670517861843109130859375f;

  // Piecewise linear approximation
  const __m256  x_base      = _mm256_mul_ps(x, _mm256_set1_ps(12102203.0f)); // multiply by log2(e) * 2^23
  const __m256i i_base      = _mm256_cvttps_epi32(x_base);                  // convert to Q9.23 fixed point, rounding down
  const __m256i i_result    = _mm256_add_epi32(i_base, _mm256_set1_epi32(Add32));
  const __m256  first_order = _mm256_castsi256_ps(i_result);

  // Install 127 as the exponent of i_result, placing correction_x in [1, 2)
  __m256i correction_xi = _mm256_and_si256(i_result, _mm256_set1_epi32(0x7fffff));
  correction_xi = _mm256_or_si256(correction_xi, _mm256_set1_epi32(127 << 23));
  const __m256 correction_x = _mm256_castsi256_ps(correction_xi);

  // 2nd order correction factor, evaluated with a Horner scheme ((x * P2 + P1) * x + P0)
  __m256 Correction;
  if constexpr(UseFMA) { Correction = _mm256_fmadd_ps(_mm256_fmadd_ps(correction_x, _mm256_set1_ps(P2), _mm256_set1_ps(P1)), correction_x, _mm256_set1_ps(P0));  }
  else                 { Correction = _mm256_add_ps(_mm256_mul_ps(_mm256_add_ps(_mm256_mul_ps(correction_x, _mm256_set1_ps(P2)), _mm256_set1_ps(P1)), correction_x), _mm256_set1_ps(P0));  }

  __m256 Result = _mm256_mul_ps(first_order, Correction);
  return Result;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<bool BoundChecking, bool UseFMA> __m256 xExponentialAVX<BoundChecking, UseFMA>::PreciseExp(__m256 x)
{
  const __m256 LOG2E = _mm256_set1_ps(1.4426950408889634073599246810018921374219f);
  const __m256 LN2H  = _mm256_set1_ps(0.693145751953125f);
  const __m256 LN2L  = _mm256_set1_ps(1.4286068203094172e-6f);
  const __m256 C5    = _mm256_set1_ps(0.008333359040319919586181640625f);
  const __m256 C4    = _mm256_set1_ps(0.0416664816439151763916015625f);
  const __m256 C3    = _mm256_set1_ps(0.16666667163372039794921875f);
  const __m256 C2    = _mm256_set1_ps(0.4999999701976776123046875f);
  const __m256 C1    = _mm256_set1_ps(1.0f);
  const __m256 C0    = _mm256_set1_ps(1.0f);

  //Step 1 = High-Precision Cody-Waite Range Reduction
  const __m256 k = _mm256_round_ps(_mm256_mul_ps(x, LOG2E), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
  //Cody-Waite range reduction: s = x - k*c1 - k*c2
  __m256 s;
  if constexpr(UseFMA) { s = _mm256_fnmadd_ps(k, LN2L, _mm256_fnmadd_ps(k, LN2H, x));  }
  else                 { s = _mm256_sub_ps(_mm256_sub_ps(x, _mm256_mul_ps(k, LN2H)), _mm256_mul_ps(k, LN2L)); }

  //Step 2 = Remez Minimax Polynomial (Estrin's Parallel Tree Evaluation)  
  const __m256 s2 = _mm256_mul_ps(s, s);

  __m256 poly;
  if constexpr(UseFMA)
  {
    const __m256 p01   = _mm256_fmadd_ps(s, C1, C0);
    const __m256 p23   = _mm256_fmadd_ps(s, C3, C2);
    const __m256 p45   = _mm256_fmadd_ps(s, C5, C4);
    const __m256 p2345 = _mm256_fmadd_ps(s2, p45, p23);
    poly = _mm256_fmadd_ps(s2, p2345, p01);
  }
  else
  {
    const __m256 p01   = _mm256_add_ps(_mm256_mul_ps(s, C1), C0);
    const __m256 p23   = _mm256_add_ps(_mm256_mul_ps(s, C3), C2);
    const __m256 p45   = _mm256_add_ps(_mm256_mul_ps(s, C5), C4);
    const __m256 p2345 = _mm256_add_ps(_mm256_mul_ps(s2, p45), p23);
    poly = _mm256_add_ps(_mm256_mul_ps(s2, p2345), p01);
  }

  // Step 3 - multiply poly by 2^k via bit manipulation
  const __m256i kInt       = _mm256_cvtps_epi32(k);         // convert (k is already integer-valued, so exact)
  const __m256i expOffset  = _mm256_slli_epi32(kInt, 23);   // shift into the float32 exponent bit position
  const __m256i polyBits   = _mm256_castps_si256(poly);     // reinterpret bits, no value change
  const __m256i resultBits = _mm256_add_epi32(polyBits, expOffset);
  __m256 Result = _mm256_castsi256_ps(resultBits);          // reinterpret back to float

  if constexpr(BoundChecking)
  {
    const __m256 OverflowMask  = _mm256_cmp_ps(x, _mm256_set1_ps(88.72283935546875f),    _CMP_GT_OQ);
    const __m256 UnderflowMask = _mm256_cmp_ps(x, _mm256_set1_ps(-103.972076416015625f), _CMP_LT_OQ);
    const __m256 NanMask       = _mm256_cmp_ps(x, x, _CMP_UNORD_Q); // x is NaN iff x!=x ("unordered")

    Result = _mm256_blendv_ps(Result, _mm256_set1_ps(std::numeric_limits<flt32>::infinity()), OverflowMask );
    Result = _mm256_blendv_ps(Result, _mm256_setzero_ps(),                                    UnderflowMask);
    Result = _mm256_blendv_ps(Result, x,                                                      NanMask      ); // pass NaN through
  }

  return Result;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<bool BoundChecking, bool UseFMA> template<int32 Degree> __m256 xExponentialAVX<BoundChecking, UseFMA>::DomainExpEstrin(__m256 x)
{
  constexpr auto C = xDomainExpCoeffs32<Degree>();
  const __m256 x2 = _mm256_mul_ps(x, x);

  if constexpr(Degree == 5)
  {
    __m256 p01, p23, p45;
    if constexpr(UseFMA)
    {
      p01 = _mm256_fmadd_ps(_mm256_set1_ps(C[1]), x, _mm256_set1_ps(C[0]));
      p23 = _mm256_fmadd_ps(_mm256_set1_ps(C[3]), x, _mm256_set1_ps(C[2]));
      p45 = _mm256_fmadd_ps(_mm256_set1_ps(C[5]), x, _mm256_set1_ps(C[4]));
    }
    else
    {
      p01 = _mm256_add_ps(_mm256_set1_ps(C[0]), _mm256_mul_ps(_mm256_set1_ps(C[1]), x));
      p23 = _mm256_add_ps(_mm256_set1_ps(C[2]), _mm256_mul_ps(_mm256_set1_ps(C[3]), x));
      p45 = _mm256_add_ps(_mm256_set1_ps(C[4]), _mm256_mul_ps(_mm256_set1_ps(C[5]), x));
    }
    const __m256 x4 = _mm256_mul_ps(x2, x2);
    __m256 p03; if constexpr(UseFMA) { p03 = _mm256_fmadd_ps(x2, p23, p01); } else { p03 = _mm256_add_ps(p01, _mm256_mul_ps(x2, p23)); }
    __m256 Result; if constexpr(UseFMA) { Result = _mm256_fmadd_ps(x4, p45, p03); } else { Result = _mm256_add_ps(p03, _mm256_mul_ps(x4, p45)); }
    return Result;
  }
  else if constexpr(Degree == 6)
  {
    __m256 p01, p23, p45;
    if constexpr(UseFMA)
    {
      p01 = _mm256_fmadd_ps(_mm256_set1_ps(C[1]), x, _mm256_set1_ps(C[0]));
      p23 = _mm256_fmadd_ps(_mm256_set1_ps(C[3]), x, _mm256_set1_ps(C[2]));
      p45 = _mm256_fmadd_ps(_mm256_set1_ps(C[5]), x, _mm256_set1_ps(C[4]));
    }
    else
    {
      p01 = _mm256_add_ps(_mm256_set1_ps(C[0]), _mm256_mul_ps(_mm256_set1_ps(C[1]), x));
      p23 = _mm256_add_ps(_mm256_set1_ps(C[2]), _mm256_mul_ps(_mm256_set1_ps(C[3]), x));
      p45 = _mm256_add_ps(_mm256_set1_ps(C[4]), _mm256_mul_ps(_mm256_set1_ps(C[5]), x));
    }
    __m256 p03, p46;
    if constexpr(UseFMA)
    {
      p03 = _mm256_fmadd_ps(x2, p23, p01);
      p46 = _mm256_fmadd_ps(x2, _mm256_set1_ps(C[6]), p45);
    }
    else
    {
      p03 = _mm256_add_ps(p01, _mm256_mul_ps(x2, p23));
      p46 = _mm256_add_ps(p45, _mm256_mul_ps(x2, _mm256_set1_ps(C[6])));
    }
    const __m256 x4 = _mm256_mul_ps(x2, x2);
    __m256 Result; if constexpr(UseFMA) { Result = _mm256_fmadd_ps(x4, p46, p03); } else { Result = _mm256_add_ps(p03, _mm256_mul_ps(x4, p46)); }
    return Result;
  }
  else if constexpr(Degree == 7)
  {
    __m256 p01, p23, p45, p67;
    if constexpr(UseFMA)
    {
      p01 = _mm256_fmadd_ps(_mm256_set1_ps(C[1]), x, _mm256_set1_ps(C[0]));
      p23 = _mm256_fmadd_ps(_mm256_set1_ps(C[3]), x, _mm256_set1_ps(C[2]));
      p45 = _mm256_fmadd_ps(_mm256_set1_ps(C[5]), x, _mm256_set1_ps(C[4]));
      p67 = _mm256_fmadd_ps(_mm256_set1_ps(C[7]), x, _mm256_set1_ps(C[6]));
    }
    else
    {
      p01 = _mm256_add_ps(_mm256_set1_ps(C[0]), _mm256_mul_ps(_mm256_set1_ps(C[1]), x));
      p23 = _mm256_add_ps(_mm256_set1_ps(C[2]), _mm256_mul_ps(_mm256_set1_ps(C[3]), x));
      p45 = _mm256_add_ps(_mm256_set1_ps(C[4]), _mm256_mul_ps(_mm256_set1_ps(C[5]), x));
      p67 = _mm256_add_ps(_mm256_set1_ps(C[6]), _mm256_mul_ps(_mm256_set1_ps(C[7]), x));
    }
    __m256 p03, p47;
    if constexpr(UseFMA)
    {
      p03 = _mm256_fmadd_ps(x2, p23, p01);
      p47 = _mm256_fmadd_ps(x2, p67, p45);
    }
    else
    {
      p03 = _mm256_add_ps(p01, _mm256_mul_ps(x2, p23));
      p47 = _mm256_add_ps(p45, _mm256_mul_ps(x2, p67));
    }
    const __m256 x4 = _mm256_mul_ps(x2, x2);
    __m256 Result; if constexpr(UseFMA) { Result = _mm256_fmadd_ps(x4, p47, p03); } else { Result = _mm256_add_ps(p03, _mm256_mul_ps(x4, p47)); }
    return Result;
  }
  else //Degree == 8
  {
    __m256 p01, p23, p45, p67;
    if constexpr(UseFMA)
    {
      p01 = _mm256_fmadd_ps(_mm256_set1_ps(C[1]), x, _mm256_set1_ps(C[0]));
      p23 = _mm256_fmadd_ps(_mm256_set1_ps(C[3]), x, _mm256_set1_ps(C[2]));
      p45 = _mm256_fmadd_ps(_mm256_set1_ps(C[5]), x, _mm256_set1_ps(C[4]));
      p67 = _mm256_fmadd_ps(_mm256_set1_ps(C[7]), x, _mm256_set1_ps(C[6]));
    }
    else
    {
      p01 = _mm256_add_ps(_mm256_set1_ps(C[0]), _mm256_mul_ps(_mm256_set1_ps(C[1]), x));
      p23 = _mm256_add_ps(_mm256_set1_ps(C[2]), _mm256_mul_ps(_mm256_set1_ps(C[3]), x));
      p45 = _mm256_add_ps(_mm256_set1_ps(C[4]), _mm256_mul_ps(_mm256_set1_ps(C[5]), x));
      p67 = _mm256_add_ps(_mm256_set1_ps(C[6]), _mm256_mul_ps(_mm256_set1_ps(C[7]), x));
    }
    __m256 p03, p47;
    if constexpr(UseFMA)
    {
      p03 = _mm256_fmadd_ps(x2, p23, p01);
      p47 = _mm256_fmadd_ps(x2, p67, p45);
    }
    else
    {
      p03 = _mm256_add_ps(p01, _mm256_mul_ps(x2, p23));
      p47 = _mm256_add_ps(p45, _mm256_mul_ps(x2, p67));
    }
    //C[8] is a lone leftover term - fold it in with one more Horner step reusing x4 instead of computing an independent x8=x4*x4
    const __m256 x4 = _mm256_mul_ps(x2, x2);
    __m256 F1; if constexpr(UseFMA) { F1 = _mm256_fmadd_ps(x4, _mm256_set1_ps(C[8]), p47); } else { F1 = _mm256_add_ps(p47, _mm256_mul_ps(x4, _mm256_set1_ps(C[8]))); }
    __m256 Result; if constexpr(UseFMA) { Result = _mm256_fmadd_ps(x4, F1, p03); } else { Result = _mm256_add_ps(p03, _mm256_mul_ps(x4, F1)); }
    return Result;
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<bool BoundChecking, bool UseFMA> template<int32 Degree> __m256 xExponentialAVX<BoundChecking, UseFMA>::DomainExpHorner(__m256 x)
{
  constexpr auto C = xDomainExpCoeffs32<Degree>();
  __m256 Result = _mm256_set1_ps(C[Degree]);
  for(int32 i = Degree - 1; i >= 0; i--)
  {
    const __m256 Ci = _mm256_set1_ps(C[i]);
    if constexpr(UseFMA) { Result = _mm256_fmadd_ps(Result, x, Ci)               ; }
    else                 { Result = _mm256_add_ps  (_mm256_mul_ps(Result, x), Ci); }
  }
  return Result;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB_NAMESPACE::GSR

#endif //X_SIMD_CAN_USE_AVX
