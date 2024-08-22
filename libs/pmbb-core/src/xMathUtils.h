/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefCORE.h"
#include "xVec.h"
#include <vector>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xKBNS //Kahan Babuska Neumaier Sumation
{
protected:
  flt64 s = 0.0;
  flt64 c = 0.0;

public:
  inline void acc(flt64 v)
  {
    flt64 t = s + v;
    if(xAbs(s) >= xAbs(v)) { c += ((s - t) + v); }
    else                   { c += ((v - t) + s); }
    s = t;
  }

  inline void acc(const flt64* x, const uintSize n)
  {
    for(uintSize i = 0; i < n; i++)
    {
      flt64 t = s + x[i];
      if(xAbs(s) >= xAbs(x[i])) { c += ((s - t) + x[i]); }
      else                      { c += ((x[i] - t) + s); }
      s = t;
    }
  }  

  inline void  reset () { s = 0.0; c = 0.0; }
  inline flt64 result() const { return s + c; }

  static        flt64   Accumulate(const flt64* x, const uintSize n);
  static inline flt64   Accumulate(const std::vector<flt64>& Data) { return Accumulate(Data.data(), Data.size()); }
  static        flt64V4 Accumulate(const flt64V4* v, const uintSize n);
  static inline flt64V4 Accumulate(const std::vector<flt64V4>& Data) { return Accumulate(Data.data(), Data.size()); }
};

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
  void xAcc(const tF64V4* x, const uintSize n);

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

class xKBNS4
{
private:
#if X_SIMD_CAN_USE_AVX
  using tKBNS4 = xKBNS4_AVX;
  xKBNS4_AVX KBNS4;
#else //X_SIMD_CAN_USE_AVX
  using tKBNS4 = xKBNS4_STD;
  xKBNS4_STD KBNS4;
#endif //X_SIMD_CAN_USE_AVX

public:
  inline void acc(const flt64V4& v                  ) { KBNS4.acc(v   ); }
  inline void acc(const flt64V4* x, const uintSize n) { KBNS4.acc(x, n); }

  inline void    reset ()       { KBNS4.reset(); }
  inline flt64V4 result() const { return KBNS4.result(); }

  static inline flt64V4 Accumulate(const flt64V4* v, const uintSize n) { return tKBNS4::Accumulate(v, n); }
  static inline flt64V4 Accumulate(const std::vector<flt64V4>& Data  ) { return tKBNS4::Accumulate(Data); }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB