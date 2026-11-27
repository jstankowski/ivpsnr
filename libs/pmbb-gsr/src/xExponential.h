/*
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefGSR.h"
#include <array>
#if X_PMBB_CPP20
#include <bit>
#endif

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================
//Relative-error-weighted (weight=1/exp(x)) minimax-style fit for exp(x), x in [-5.545, 0] (matches c_PowerThreshold in xCommonDefGSR.h), power basis C0 + C1*x + ... + Cn*x^n
//fit minimizes (P(x)-exp(x))/exp(x), not (P(x)-exp(x)) directly - unweighted fits blow up in relative error near x=-5.545 where exp(x) is tiny
//===============================================================================================================================================================================================================

template<int32 Degree> constexpr std::array<flt32, Degree + 1> xDomainExpCoeffs32();
template<int32 Degree> constexpr std::array<flt64, Degree + 1> xDomainExpCoeffs64();

template<> constexpr std::array<flt32, 5> xDomainExpCoeffs32<4>() { return { 0.92024536211420982f, 0.7284760398668092f, 0.22698893495207512f, 0.032282809129840856f, 0.0017432013746833854f }; }
template<> constexpr std::array<flt32, 6> xDomainExpCoeffs32<5>() { return { 0.98139290827657022f, 0.89883505435765254f, 0.35984048672795871f, 0.076390544405578867f, 0.0083912336852017492f, 0.00037511906779923881f }; }
template<> constexpr std::array<flt32, 7> xDomainExpCoeffs32<6>() { return { 0.99629516899001858f, 0.96972414851099198f, 0.44340158872151908f, 0.11896505162281343f, 0.019188140712574374f, 0.0017195733726211898f, 6.5648090767922535e-05f }; }
template<> constexpr std::array<flt32, 8> xDomainExpCoeffs32<7>() { return { 0.99935518249561073f, 0.99250921916845392f, 0.48155178222719947f, 0.14660640236299954f, 0.029591249963619708f, 0.0038513920751619812f, 0.00029167880645062868f, 9.7213536278641661e-06f }; }
template<> constexpr std::array<flt32, 9> xDomainExpCoeffs32<8>() { return { 0.99990030524655182f, 0.99842726664325887f, 0.49501531147045003f, 0.15979958392034366f, 0.036454772258947062f, 0.0058985908785547242f, 0.00064375821540855723f, 4.223114748537972e-05f, 1.2492532725288573e-06f }; }

template<> constexpr std::array<flt64, 5> xDomainExpCoeffs64<4>() { return { 0.92024536211420982, 0.7284760398668092, 0.22698893495207512, 0.032282809129840856, 0.0017432013746833854 }; }
template<> constexpr std::array<flt64, 6> xDomainExpCoeffs64<5>() { return { 0.98139290827657022, 0.89883505435765254, 0.35984048672795871, 0.076390544405578867, 0.0083912336852017492, 0.00037511906779923881 }; }
template<> constexpr std::array<flt64, 7> xDomainExpCoeffs64<6>() { return { 0.99629516899001858, 0.96972414851099198, 0.44340158872151908, 0.11896505162281343, 0.019188140712574374, 0.0017195733726211898, 6.5648090767922535e-05 }; }
template<> constexpr std::array<flt64, 8> xDomainExpCoeffs64<7>() { return { 0.99935518249561073, 0.99250921916845392, 0.48155178222719947, 0.14660640236299954, 0.029591249963619708, 0.0038513920751619812, 0.00029167880645062868, 9.7213536278641661e-06 }; }
template<> constexpr std::array<flt64, 9> xDomainExpCoeffs64<8>() { return { 0.99990030524655182, 0.99842726664325887, 0.49501531147045003, 0.15979958392034366, 0.036454772258947062, 0.0058985908785547242, 0.00064375821540855723, 4.223114748537972e-05, 1.2492532725288573e-06 }; }

//===============================================================================================================================================================================================================

template<bool BoundChecking, bool UseFMA> class xExponential
{
protected:
#if X_PMBB_CPP20
  flt32 xInt32AsFlt32() {}
  static inline flt32 xBitCastI32asF32(int32 x) { std::bit_cast<flt32>(x); }
  static inline flt64 xBitCastI64asF64(int64 x) { std::bit_cast<flt64>(x); }

  static inline flt32  xBitCastU32asF32(uint32 x) { std::bit_cast<flt32 >(x); }
  static inline uint32 xBitCastF32asU32(flt32  x) { std::bit_cast<uint32>(x); }
  static inline flt64  xBitCastU64asF64(uint64 x) { std::bit_cast<flt64 >(x); }
  static inline uint64 xBitCastF64asU64(flt64  x) { std::bit_cast<uint64>(x); }
#else
  static inline flt32  xBitCastI32asF32(int32  x) { union { flt32 f; int32  i; } u; u.i = x; return u.f; }  
  static inline flt64  xBitCastI64asF64(int64  x) { union { flt64 f; int64  i; } u; u.i = x; return u.f; }

  static inline flt32  xBitCastU32asF32(uint32 x) { union { flt32 f; uint32 i; } u; u.i = x; return u.f; }
  static inline uint32 xBitCastF32asU32(flt32  x) { union { flt32 f; uint32 i; } u; u.f = x; return u.i; }
  static inline flt64  xBitCastU64asF64(uint64 x) { union { flt64 f; uint64 i; } u; u.i = x; return u.f; }
  static inline uint64 xBitCastF64asU64(flt64  x) { union { flt64 f; uint64 i; } u; u.f = x; return u.i; }
#endif

public:
  //Schraudolph exponential [N. Schraudolph, "A Fast, Compact Approximation of the Exponential Function", Neural Computation 11, 853-862(1999)]
  static inline flt32 SchraudolphExp(flt32 x);
  static inline flt64 SchraudolphExp(flt64 x);

  //Schraudolph exponential wit 2-nd order correction [based on https://specbranch.com/posts/fast-exp/]
  static flt32 SchraudolphExpCorrected(flt32 x);
  static flt64 SchraudolphExpCorrected(flt64 x);

  //Cody-Waite Reduction with a Remez Minimax Polynomial evaluated via Estrin's Method
  static flt32 PreciseExp(flt32 x);
  static flt64 PreciseExp(flt64 x);

  //Degree-selectable polynomial fit to exp(x) for x in [-5.545, 0] (matches c_PowerThreshold)
  template<int32 Degree> static flt32 DomainExpEstrin(flt32 x);
  template<int32 Degree> static flt64 DomainExpEstrin(flt64 x);

  template<int32 Degree> static flt32 DomainExpHorner(flt32 x);
  template<int32 Degree> static flt64 DomainExpHorner(flt64 x);
};


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<bool BoundChecking, bool UseFMA> flt32 xExponential<BoundChecking, UseFMA>::SchraudolphExp(flt32 x)
{
  constexpr flt32 a = (1 << 23) / 0.69314718f; //log2(e) * 2^23
  constexpr flt32 b = (1 << 23) * (127 - 0.043677448f);

  if constexpr(UseFMA) { x = std::fmaf(a, x, b); }
  else                 { x = a * x + b         ; }

  if constexpr(BoundChecking)
  {
    constexpr flt32 c = (1 << 23);
    constexpr flt32 d = (1 << 23) * 255;
    if(x < c) { x = 0.0; }
    if(x > d) { x = d; }
  }

  x = xBitCastI32asF32(static_cast<uint32_t>(x));
  return x;
}
template<bool BoundChecking, bool UseFMA> flt64 xExponential<BoundChecking, UseFMA>::SchraudolphExp(flt64 x)
{
  constexpr flt64 a = (1LL << 52) / 0.6931471805599453;
  constexpr flt64 b = (1LL << 52) * (1023 - 0.04367744890362246);

  if constexpr(UseFMA) { x = std::fma(a, x, b); }
  else                 { x = a * x + b        ; }

  if constexpr(BoundChecking)
  {
    constexpr flt64 c = (1LL << 52);
    constexpr flt64 d = (1LL << 52) * 2047;
    if(x < c) { x = 0.0; }
    if(x > d) { x = d  ; }
  }

  x = xBitCastI64asF64(static_cast<uint64_t>(x));
  return x;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<bool BoundChecking, bool UseFMA> flt32 xExponential<BoundChecking, UseFMA>::SchraudolphExpCorrected(flt32 x)
{
  constexpr int32 K32_TUNED_MINIMAX = 366393; // Max relative error: 2.98%
  //constexpr int32 K32_TUNED_L1      = 545948; // Mean relative error: 1.48%
  //constexpr int32 K32_TUNED_L2      = 486412; // Root mean squared relative error: 1.77%
  constexpr int32 K32 = K32_TUNED_MINIMAX;
  constexpr int32 Add32 = (127 << 23) - K32;

  constexpr flt32 P0 = (flt32)1.469318866729736328125     ;
  constexpr flt32 P1 = (flt32)-0.671999752521514892578125 ;
  constexpr flt32 P2 = (flt32)0.22670517861843109130859375;

  // Piecewise linear approximation
  flt32 x_base   = x * 12102203;               // Multiply by log2(e) * 2^23
  int32 i_base   = static_cast<int32>(x_base); // Convert to Q9.23 fixed point, rounding down
  int32 i_result = i_base + Add32;
  flt32 first_order = xBitCastI32asF32(i_result);

  // Install 127 as the exponent of i_result, placing correction_x in [1, 2)
  int32 correction_xi = (i_result & 0x7fffff) | (127 << 23);
  flt32 correction_x = xBitCastI32asF32(correction_xi);

  // 2nd order correction factor, evaluated with a Horner scheme ((x * P2 + P1) * x + P0)
  flt32 correction;
  if constexpr(UseFMA) { correction = std::fmaf(std::fmaf(correction_x, P2, P1), correction_x, P0); }
  else                 { correction = (((correction_x * P2) + P1) * correction_x) + P0            ; }

  flt32 Result = first_order * correction;
  return Result;
}
template<bool BoundChecking, bool UseFMA> flt64 xExponential<BoundChecking, UseFMA>::SchraudolphExpCorrected(flt64 x)
{
  constexpr int64 K64_TUNED_MINIMAX = 196705367621632; // K scaled for double: 366393 * 2^29 = 196705367621632
  constexpr int64 K64 = K64_TUNED_MINIMAX;
  constexpr int64 Add64 = (1023LL << 52) - K64;

  constexpr flt64 P0 = (flt64)1.469318866729736328125;
  constexpr flt64 P1 = (flt64)-0.671999752521514892578125;
  constexpr flt64 P2 = (flt64)0.22670517861843109130859375;

  // Piecewise linear approximation
  flt64 x_base      = x * 6497320848556798.0;     // Multiply x by log2(e) * 2^52
  int64 i_base      = static_cast<int64>(x_base); // Convert to Q9.23 fixed point, rounding down
  int64 i_result    = i_base + Add64;
  flt64 first_order = xBitCastI64asF64(i_result);

  // Extract the mantissa of the first-order approximation
  int64 mantissa_bits = (i_result & 0x000FFFFFFFFFFFFFLL) | 0x3FF0000000000000LL;
  flt64 correction_x  = xBitCastI64asF64(mantissa_bits);

  // 2nd order correction factor, evaluated with a Horner scheme ((x * P2 + P1) * x + P0)
  flt64 correction;
  if constexpr(UseFMA) { correction = std::fma(std::fma(correction_x, P2, P1), correction_x, P0); }
  else                 { correction = (((correction_x * P2) + P1) * correction_x) + P0          ; }

  flt64 Result = first_order * correction;
  return Result;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<bool BoundChecking, bool UseFMA> flt32 xExponential<BoundChecking, UseFMA>::PreciseExp(flt32 x)
{
  constexpr flt32 LOG2E = (flt32)1.4426950408889634073599246810018921374219;
  // Cody-Waite high/low split constants
  constexpr flt32 LN2H  = (flt32)0.693145751953125    ; // ln(2) high part
  constexpr flt32 LN2L  = (flt32)1.4286068203094172e-6; // ln(2) low part
  // Polynomial coefficients - Remez Minimax Polynomial
  constexpr flt32 C5    = (flt32)0.008333359040319919586181640625;
  constexpr flt32 C4    = (flt32)0.0416664816439151763916015625;
  constexpr flt32 C3    = (flt32)0.16666667163372039794921875;
  constexpr flt32 C2    = (flt32)0.4999999701976776123046875;
  constexpr flt32 C1    = (flt32)1.0;
  constexpr flt32 C0    = (flt32)1.0;

  if constexpr(BoundChecking)
  {
    //Step 0 = Boundary & Special Value Handling (C99 / IEEE-754 Compliance)
    if(x > (flt32)88.72283935546875   ) { return std::numeric_limits<flt32>::infinity(); } // Overflow
    if(x < (flt32)-103.972076416015625) { return (flt32)0.0                            ; } // Underflow
    if(std::isnan(x)                  ) { return x                                     ; } // Pass through NaNs
  }

  //Step 1 = High-Precision Cody-Waite Range Reduction
  flt32 k = (flt32)xNearbyF32ToI32(x * LOG2E); //Round x * log2(e) to nearest integer (IEEE tie-even) - do not use nearbyintf it is slow
  //Cody-Waite range reduction: s = x - k*c1 - k*c2
  flt32 s;
  if constexpr(UseFMA) { s = fmaf(-k, LN2L, fmaf(-k, LN2H, x)); }
  else                 { s = (- k * LN2L) + ((-k * LN2H) + x); }

  //Step 2 = Remez Minimax Polynomial (Estrin's Parallel Tree Evaluation)  
  flt32 s2 = s * s; // Compute s^2 to allow parallel branch computation (Estrin's method)

  flt32 poly;
  if constexpr(UseFMA)
  {
    flt32 p01   = std::fmaf(s, C1, C0); // (s * 1.0 + 1.0)
    flt32 p23   = std::fmaf(s, C3, C2); // (s * c3 + c2)
    flt32 p45   = std::fmaf(s, C5, C4); // (s * c5 + c4)
    flt32 p2345 = std::fmaf(s2, p45, p23);   // (s2 * p45 + p23)
    poly  = std::fmaf(s2, p2345, p01); // (s2 * p2345 + p01)
  }
  else
  {
    flt32 p01   = (s * C1) + C0;
    flt32 p23   = (s * C3) + C2;
    flt32 p45   = (s * C5) + C4;
    flt32 p2345 = (s2 * p45) + p23;
    poly  = (s2 * p2345) + p01;
  }

  //Step 3 = Multiply poly by 2^k - do not use "scalbnf" it is slow
  uint32_t poly_bits = xBitCastF32asU32(poly);
  // Add k directly to float exponent field
  int32_t exp_offset = ((int32)k) << 23;
  poly_bits += exp_offset;
  flt32 Result = xBitCastU32asF32(poly_bits);
  return Result;
}
template<bool BoundChecking, bool UseFMA> flt64 xExponential<BoundChecking, UseFMA>::PreciseExp(flt64 x)
{
  constexpr flt64 LOG2E = 1.4426950408889634073599246810018921374219;    
  // Cody-Waite high/low split constants 
  constexpr flt64 LN2H  = 0.69314718055994528622776601683795; 
  constexpr flt64 LN2L  = 2.319046813846299558417771141022886e-17;
  // Polynomial coefficients extended 
  constexpr flt64 C11 = 2.505210838544171877505244e-8;
  constexpr flt64 C10 = 2.7557319223985890652557319e-7;
  constexpr flt64 C9  = 2.7557319223985890652557319e-6;
  constexpr flt64 C8  = 2.4801587301587301587301587e-5;
  constexpr flt64 C7  = 1.9841269841269841269841270e-4;
  constexpr flt64 C6  = 1.3888888888888888888888889e-3;
  constexpr flt64 C5  = 8.3333333333333333333333333e-3;
  constexpr flt64 C4  = 4.1666666666666666666666667e-2;
  constexpr flt64 C3  = 1.6666666666666666666666667e-1;
  constexpr flt64 C2  = 0.5;
  constexpr flt64 C1  = 1.0;
  constexpr flt64 C0  = 1.0;

  if constexpr(BoundChecking)
  {
    // Step 0 = Boundary & Special Value Handling (IEEE-754 double limits)
    if(x > 709.782712893383996) { return std::numeric_limits<flt64>::infinity(); } // Overflow
    if(x < -745.1332191019411 ) { return 0.0; }                                    // Underflow
    if(std::isnan(x)          ) { return x; }                                      // Pass through NaNs
  }

  // Step 1 = High-Precision Cody-Waite Range Reduction
  int64_t k = xNearbyF64ToI64(x * LOG2E); 

  // Cody-Waite range reduction: s = x - k*LN2H - k*LN2L
  flt64 s;
  if constexpr(UseFMA) { s = std::fma(-k, LN2L, std::fma(-k, LN2H, x)); }
  else                 { s = (-k * LN2L) + ((-k * LN2H) + x)          ; }

  // Step 2 = Remez Minimax Polynomial (Estrin's Parallel Tree Evaluation)
  flt64 s2 = s * s;
  flt64 s4 = s2 * s2;

  flt64 poly;
  if constexpr(UseFMA)
  {
    flt64 p01   = std::fma(s, C1, C0);
    flt64 p23   = std::fma(s, C3, C2);
    flt64 p45   = std::fma(s, C5, C4);
    flt64 p67   = std::fma(s, C7, C6);
    flt64 p89   = std::fma(s, C9, C8);
    flt64 p1011 = std::fma(s, C11, C10);
    flt64 p03   = std::fma(s2, p23, p01);
    flt64 p47   = std::fma(s2, p67, p45);
    flt64 p811  = std::fma(s2, p1011, p89);
    flt64 p411  = std::fma(s4, p811, p47);
    poly        = std::fma(s4, p411, p03);
  }
  else
  {
    flt64 p01   = (s * C1) + C0;
    flt64 p23   = (s * C3) + C2;
    flt64 p45   = (s * C5) + C4;
    flt64 p67   = (s * C7) + C6;
    flt64 p89   = (s * C9) + C8;
    flt64 p1011 = (s * C11) + C10;
    flt64 p03   = (s2 * p23) + p01;
    flt64 p47   = (s2 * p67) + p45;
    flt64 p811  = (s2 * p1011) + p89;
    flt64 p411  = (s4 * p811) + p47;
    poly        = (s4 * p411) + p03;
  }

  // Step 3 = Multiply poly by 2^k
  uint64_t poly_bits = xBitCastF64asU64(poly);    
  // Add k directly to float exponent field (Bit 52 to 62 in double)
  int64_t exp_offset = k << 52;
  poly_bits += exp_offset;    
  flt64 Result = xBitCastU64asF64(poly_bits);

  return Result;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//Estrin's method, hand-unrolled per degree with named intermediates (no array/runtime loop) - mirrors PreciseExp's evaluation style above;
//a generic array+loop formulation was tried first but left Terms un-promoted to registers under MSVC, making it slower than PreciseExp despite doing less work
template<bool BoundChecking, bool UseFMA> template<int32 Degree> flt32 xExponential<BoundChecking, UseFMA>::DomainExpEstrin(flt32 x)
{
  constexpr auto C = xDomainExpCoeffs32<Degree>();
  const flt32 x2 = x * x;

  if constexpr(Degree == 5)
  {
    flt32 p01, p23, p45;
    if constexpr(UseFMA)
    {
      p01 = std::fmaf(C[1], x, C[0]);
      p23 = std::fmaf(C[3], x, C[2]);
      p45 = std::fmaf(C[5], x, C[4]);
    }
    else
    {
      p01 = C[0] + C[1] * x;
      p23 = C[2] + C[3] * x;
      p45 = C[4] + C[5] * x;
    }
    const flt32 x4 = x2 * x2;
    flt32 p03; if constexpr(UseFMA) { p03 = std::fmaf(x2, p23, p01); } else { p03 = p01 + x2 * p23; }
    flt32 Result; if constexpr(UseFMA) { Result = std::fmaf(x4, p45, p03); } else { Result = p03 + x4 * p45; }
    return Result;
  }
  else if constexpr(Degree == 6)
  {
    flt32 p01, p23, p45;
    if constexpr(UseFMA)
    {
      p01 = std::fmaf(C[1], x, C[0]);
      p23 = std::fmaf(C[3], x, C[2]);
      p45 = std::fmaf(C[5], x, C[4]);
    }
    else
    {
      p01 = C[0] + C[1] * x;
      p23 = C[2] + C[3] * x;
      p45 = C[4] + C[5] * x;
    }
    flt32 p03, p46;
    if constexpr(UseFMA)
    {
      p03 = std::fmaf(x2, p23, p01);
      p46 = std::fmaf(x2, C[6], p45);
    }
    else
    {
      p03 = p01 + x2 * p23;
      p46 = p45 + x2 * C[6];
    }
    const flt32 x4 = x2 * x2;
    flt32 Result; if constexpr(UseFMA) { Result = std::fmaf(x4, p46, p03); } else { Result = p03 + x4 * p46; }
    return Result;
  }
  else if constexpr(Degree == 7)
  {
    flt32 p01, p23, p45, p67;
    if constexpr(UseFMA)
    {
      p01 = std::fmaf(C[1], x, C[0]);
      p23 = std::fmaf(C[3], x, C[2]);
      p45 = std::fmaf(C[5], x, C[4]);
      p67 = std::fmaf(C[7], x, C[6]);
    }
    else
    {
      p01 = C[0] + C[1] * x;
      p23 = C[2] + C[3] * x;
      p45 = C[4] + C[5] * x;
      p67 = C[6] + C[7] * x;
    }
    flt32 p03, p47;
    if constexpr(UseFMA)
    {
      p03 = std::fmaf(x2, p23, p01);
      p47 = std::fmaf(x2, p67, p45);
    }
    else
    {
      p03 = p01 + x2 * p23;
      p47 = p45 + x2 * p67;
    }
    const flt32 x4 = x2 * x2;
    flt32 Result; if constexpr(UseFMA) { Result = std::fmaf(x4, p47, p03); } else { Result = p03 + x4 * p47; }
    return Result;
  }
  else //Degree == 8
  {
    flt32 p01, p23, p45, p67;
    if constexpr(UseFMA)
    {
      p01 = std::fmaf(C[1], x, C[0]);
      p23 = std::fmaf(C[3], x, C[2]);
      p45 = std::fmaf(C[5], x, C[4]);
      p67 = std::fmaf(C[7], x, C[6]);
    }
    else
    {
      p01 = C[0] + C[1] * x;
      p23 = C[2] + C[3] * x;
      p45 = C[4] + C[5] * x;
      p67 = C[6] + C[7] * x;
    }
    flt32 p03, p47;
    if constexpr(UseFMA)
    {
      p03 = std::fmaf(x2, p23, p01);
      p47 = std::fmaf(x2, p67, p45);
    }
    else
    {
      p03 = p01 + x2 * p23;
      p47 = p45 + x2 * p67;
    }
    //C[8] is a lone leftover term - fold it in with one more Horner step reusing x4 instead of computing an independent x8=x4*x4
    const flt32 x4 = x2 * x2;
    flt32 F1; if constexpr(UseFMA) { F1 = std::fmaf(x4, C[8], p47); } else { F1 = p47 + x4 * C[8]; }
    flt32 Result; if constexpr(UseFMA) { Result = std::fmaf(x4, F1, p03); } else { Result = p03 + x4 * F1; }
    return Result;
  }
}
template<bool BoundChecking, bool UseFMA> template<int32 Degree> flt64 xExponential<BoundChecking, UseFMA>::DomainExpEstrin(flt64 x)
{
  constexpr auto C = xDomainExpCoeffs64<Degree>();
  const flt64 x2 = x * x;

  if constexpr(Degree == 5)
  {
    flt64 p01, p23, p45;
    if constexpr(UseFMA)
    {
      p01 = std::fma(C[1], x, C[0]);
      p23 = std::fma(C[3], x, C[2]);
      p45 = std::fma(C[5], x, C[4]);
    }
    else
    {
      p01 = C[0] + C[1] * x;
      p23 = C[2] + C[3] * x;
      p45 = C[4] + C[5] * x;
    }
    const flt64 x4 = x2 * x2;
    flt64 p03; if constexpr(UseFMA) { p03 = std::fma(x2, p23, p01); } else { p03 = p01 + x2 * p23; }
    flt64 Result; if constexpr(UseFMA) { Result = std::fma(x4, p45, p03); } else { Result = p03 + x4 * p45; }
    return Result;
  }
  else if constexpr(Degree == 6)
  {
    flt64 p01, p23, p45;
    if constexpr(UseFMA)
    {
      p01 = std::fma(C[1], x, C[0]);
      p23 = std::fma(C[3], x, C[2]);
      p45 = std::fma(C[5], x, C[4]);
    }
    else
    {
      p01 = C[0] + C[1] * x;
      p23 = C[2] + C[3] * x;
      p45 = C[4] + C[5] * x;
    }
    flt64 p03, p46;
    if constexpr(UseFMA)
    {
      p03 = std::fma(x2, p23, p01);
      p46 = std::fma(x2, C[6], p45);
    }
    else
    {
      p03 = p01 + x2 * p23;
      p46 = p45 + x2 * C[6];
    }
    const flt64 x4 = x2 * x2;
    flt64 Result; if constexpr(UseFMA) { Result = std::fma(x4, p46, p03); } else { Result = p03 + x4 * p46; }
    return Result;
  }
  else if constexpr(Degree == 7)
  {
    flt64 p01, p23, p45, p67;
    if constexpr(UseFMA)
    {
      p01 = std::fma(C[1], x, C[0]);
      p23 = std::fma(C[3], x, C[2]);
      p45 = std::fma(C[5], x, C[4]);
      p67 = std::fma(C[7], x, C[6]);
    }
    else
    {
      p01 = C[0] + C[1] * x;
      p23 = C[2] + C[3] * x;
      p45 = C[4] + C[5] * x;
      p67 = C[6] + C[7] * x;
    }
    flt64 p03, p47;
    if constexpr(UseFMA)
    {
      p03 = std::fma(x2, p23, p01);
      p47 = std::fma(x2, p67, p45);
    }
    else
    {
      p03 = p01 + x2 * p23;
      p47 = p45 + x2 * p67;
    }
    const flt64 x4 = x2 * x2;
    flt64 Result; if constexpr(UseFMA) { Result = std::fma(x4, p47, p03); } else { Result = p03 + x4 * p47; }
    return Result;
  }
  else //Degree == 8
  {
    flt64 p01, p23, p45, p67;
    if constexpr(UseFMA)
    {
      p01 = std::fma(C[1], x, C[0]);
      p23 = std::fma(C[3], x, C[2]);
      p45 = std::fma(C[5], x, C[4]);
      p67 = std::fma(C[7], x, C[6]);
    }
    else
    {
      p01 = C[0] + C[1] * x;
      p23 = C[2] + C[3] * x;
      p45 = C[4] + C[5] * x;
      p67 = C[6] + C[7] * x;
    }
    flt64 p03, p47;
    if constexpr(UseFMA)
    {
      p03 = std::fma(x2, p23, p01);
      p47 = std::fma(x2, p67, p45);
    }
    else
    {
      p03 = p01 + x2 * p23;
      p47 = p45 + x2 * p67;
    }
    //C[8] is a lone leftover term - fold it in with one more Horner step reusing x4 instead of computing an independent x8=x4*x4
    const flt64 x4 = x2 * x2;
    flt64 F1; if constexpr(UseFMA) { F1 = std::fma(x4, C[8], p47); } else { F1 = p47 + x4 * C[8]; }
    flt64 Result; if constexpr(UseFMA) { Result = std::fma(x4, F1, p03); } else { Result = p03 + x4 * F1; }
    return Result;
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<bool BoundChecking, bool UseFMA> template<int32 Degree> flt32 xExponential<BoundChecking, UseFMA>::DomainExpHorner(flt32 x)
{
  constexpr auto C = xDomainExpCoeffs32<Degree>();
  flt32 Result = C[Degree];
  for(int32 i = Degree - 1; i >= 0; i--)
  {
    if constexpr(UseFMA) { Result = std::fmaf(Result, x, C[i]); }
    else                 { Result = Result * x + C[i]         ; }
  }
  return Result;
}
template<bool BoundChecking, bool UseFMA> template<int32 Degree> flt64 xExponential<BoundChecking, UseFMA>::DomainExpHorner(flt64 x)
{
  constexpr auto C = xDomainExpCoeffs64<Degree>();
  flt64 Result = C[Degree];
  for(int32 i = Degree - 1; i >= 0; i--)
  {
    if constexpr(UseFMA) { Result = std::fma(Result, x, C[i]); }
    else                 { Result = Result * x + C[i]        ; }
  }
  return Result;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB_NAMESPACE::GSR