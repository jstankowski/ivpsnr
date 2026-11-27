/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefCORE.h"
#include "xVec.h"
#include <vector>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// Kahan Babuska Neumaier Sumation - scalar input
//===============================================================================================================================================================================================================

template <class fltXX> class xKBNS1_T
{
protected:
  fltXX m_S = 0.0;
  fltXX m_C = 0.0;

public:
  void acc(fltXX v);
  void acc(const fltXX* x, const uintSize n);

  xKBNS1_T& operator += (fltXX v) { acc(v); return *this; }

  inline void  reset () { m_S = 0.0; m_C = 0.0; }
  inline fltXX result() const { return m_S + m_C; }

  static        fltXX   Accumulate(const fltXX* x, const uintSize n) { xKBNS1_T KBNS; KBNS.acc(x, n); return KBNS.result(); }
  static inline fltXX   Accumulate(const std::vector<fltXX>& Data) { return Accumulate(Data.data(), Data.size()); }
};

template <class fltXX> void xKBNS1_T<fltXX>::acc(fltXX v)
{
  fltXX t = m_S + v;
  if(std::abs(m_S) >= std::abs(v)) { m_C += ((m_S - t) + v  ); }
  else                             { m_C += ((v   - t) + m_S); }
  m_S = t;
}
template <class fltXX> void xKBNS1_T<fltXX>::acc(const fltXX* x, const uintSize n)
{
  for(uintSize i = 0; i < n; i++)
  {
    fltXX t = m_S + x[i];
    if(std::abs(m_S) >= std::abs(x[i])) { m_C += ((m_S  - t) + x[i]); }
    else                                { m_C += ((x[i] - t) + m_S ); }
    m_S = t;
  }
}

using xKBNS1_STD = xKBNS1_T<flt64>;

//===============================================================================================================================================================================================================

#if X_SIMD_CAN_USE_SSE
class xKBNS1_SSE
{
public:
  union tF64
  {
    __m128d R;
    flt64   V[2];
  };

protected:
  tF64 m_s = { _mm_setzero_pd() };
  tF64 m_c = { _mm_setzero_pd() };

  void xAcc(flt64 v);
  void xAcc(const flt64* x, const uintSize n) { for(uintSize i = 0; i < n; i++) { xAcc(x[i]); } }

public:  
  inline void acc(flt64 v) { xAcc(v); }
  inline void acc(const flt64* x, const uintSize n) { xAcc(x, n); }

  xKBNS1_SSE& operator += (flt64 v) { acc(v); return *this; }

  inline void  reset ()       { m_s.R = _mm_setzero_pd(); m_c.R = _mm_setzero_pd(); }
  inline flt64 result() const { tF64 Result = { _mm_add_sd(m_s.R, m_c.R) }; return Result.V[0]; }

  static        flt64   Accumulate(const flt64* x, const uintSize n) { xKBNS1_SSE KBNS; KBNS.acc(x, n); return KBNS.result(); }
  static inline flt64   Accumulate(const std::vector<flt64>& Data) { return Accumulate(Data.data(), Data.size()); }
};
#endif //X_SIMD_CAN_USE_SSE

//===============================================================================================================================================================================================================
// Kahan Babuska Neumaier Sumation - flt64V4 input
//===============================================================================================================================================================================================================

class xKBNS4_STD
{
protected:
  flt64V4 s = xMakeVec4<flt64>(0.0);
  flt64V4 c = xMakeVec4<flt64>(0.0);

public:
  void acc(const flt64V4& v);
  void acc(const flt64V4* x, const uintSize n);

  inline void    reset ()       { s = xMakeVec4<flt64>(0.0); c = xMakeVec4<flt64>(0.0);  }
  inline flt64V4 result() const { return s + c; }

  static inline flt64V4 Accumulate(const flt64V4* v, const uintSize n) { xKBNS4_STD A; A.acc(v, n); return A.result(); }
  static inline flt64V4 Accumulate(const std::vector<flt64V4>& Data) { return Accumulate(Data.data(), Data.size()); }
};

//===============================================================================================================================================================================================================

#if X_SIMD_CAN_USE_SSE
class xKBNS4_SSE
{
public:
  union tF64V4
  {
    __m128d R[2];
    flt64V4 V;
  };

protected:
  tF64V4 s = { _mm_setzero_pd(), _mm_setzero_pd() };
  tF64V4 c = { _mm_setzero_pd(), _mm_setzero_pd() };

  void xAcc(const tF64V4& v);
  void xAcc(const tF64V4* x, const uintSize n) { for(uintSize i = 0; i < n; i++) { xAcc(x[i]); } }

public:  
  inline void acc(const flt64V4& v) { xAcc(*((tF64V4*)(&v))); }
  inline void acc(const flt64V4* x, const uintSize n) { xAcc((tF64V4*)x, n); }

  inline void    reset ()       { s.R[0] = _mm_setzero_pd(); s.R[1] = _mm_setzero_pd(); c.R[0] = _mm_setzero_pd(); c.R[1] = _mm_setzero_pd(); }
  inline flt64V4 result() const { tF64V4 Result = { _mm_add_pd(s.R[0], c.R[0]), _mm_add_pd(s.R[1], c.R[1]) }; return Result.V; }

  static inline flt64V4 Accumulate(const flt64V4* v, const uintSize n) { xKBNS4_SSE A; A.acc(v, n); return A.result(); }
  static inline flt64V4 Accumulate(const std::vector<flt64V4>& Data) { return Accumulate(Data.data(), Data.size()); }
};
#endif //X_SIMD_CAN_USE_SSE

//===============================================================================================================================================================================================================

#if X_SIMD_CAN_USE_AVX
class xKBNS4_AVX
{
public:
  union tF64V4
  {
    __m256d R;
    flt64V4 V;
  };

protected:
  tF64V4 s = { _mm256_setzero_pd() };
  tF64V4 c = { _mm256_setzero_pd() };

  void xAcc(const tF64V4& v);
  void xAcc(const tF64V4* x, const uintSize n) { for(uintSize i = 0; i < n; i++) { xAcc(x[i]); } }

public:  
  inline void acc(const flt64V4& v) { xAcc(*((tF64V4*)(&v))); }
  inline void acc(const flt64V4* x, const uintSize n) { xAcc((tF64V4*)x, n); }

  inline void    reset ()       { s.R = _mm256_setzero_pd(); c.R = _mm256_setzero_pd();  }
  inline flt64V4 result() const { tF64V4 Result = { _mm256_add_pd(s.R, c.R) }; return Result.V; }

  static inline flt64V4 Accumulate(const flt64V4* v, const uintSize n) { xKBNS4_AVX A; A.acc(v, n); return A.result(); }
  static inline flt64V4 Accumulate(const std::vector<flt64V4>& Data) { return Accumulate(Data.data(), Data.size()); }
};
#endif //X_SIMD_CAN_USE_AVX

//===============================================================================================================================================================================================================

//class xKBNS4
//{
//private:
//#if X_SIMD_CAN_USE_AVX
//  using tKBNS4 = xKBNS4_AVX;
//#elif X_SIMD_CAN_USE_SSE
//  using tKBNS4 = xKBNS4_SSE;
//#else //X_SIMD_CAN_USE_SSE
//  using tKBNS4 = xKBNS4_STD;
//#endif //X_SIMD_CAN_USE_AVX
//
//  tKBNS4 KBNS4;
//
//public:
//  inline void acc(const flt64V4& v                  ) { KBNS4.acc(v   ); }
//  inline void acc(const flt64V4* x, const uintSize n) { KBNS4.acc(x, n); }
//
//  xKBNS4& operator += (const flt64V4& v) { acc(v); return *this; }
//
//  inline void    reset ()       { KBNS4.reset(); }
//  inline flt64V4 result() const { return KBNS4.result(); }
//
//  static inline flt64V4 Accumulate(const flt64V4* v, const uintSize n) { return tKBNS4::Accumulate(v, n); }
//  static inline flt64V4 Accumulate(const std::vector<flt64V4>& Data  ) { return tKBNS4::Accumulate(Data); }
//};

#if X_SIMD_CAN_USE_SSE
using xKBNS1 = xKBNS1_SSE;
#else 
using xKBNS1 = xKBNS1_STD;
#endif

#if X_SIMD_CAN_USE_AVX
  using xKBNS4 = xKBNS4_AVX;
#elif X_SIMD_CAN_USE_SSE
  using xKBNS4 = xKBNS4_SSE;
#else 
  using xKBNS4 = xKBNS4_STD;
#endif

class xKBNS
{
public:
  static        flt64   Accumulate(const flt64* x, const uintSize n) { return xKBNS1::Accumulate(x, n); }
  static inline flt64   Accumulate(const std::vector<flt64>& Data  ) { return xKBNS1::Accumulate(Data); }

  static inline flt64V4 Accumulate(const flt64V4* v, const uintSize n) { return xKBNS4::Accumulate(v, n); }
  static inline flt64V4 Accumulate(const std::vector<flt64V4>& Data  ) { return xKBNS4::Accumulate(Data); }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB