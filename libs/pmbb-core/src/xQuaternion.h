/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefCORE.h"
#include "xString.h"
#include "xVec.h"
#include "xMatrix.h"
#include <array>
#include <vector>

/*
Quaternion
Components     W X Y Z
Memory address 0 1 2 3
*/

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

//template<typename MatrixType>
template<typename T> class xQuaternion
{
protected:
#if X_SIMD_CAN_USE_AVX
  alignas(alignof(__m256)) std::array<T, 4> m_Q;
#elif X_SIMD_CAN_USE_SSE
  alignas(alignof(__m128)) std::array<T, 4> m_Q;
#else
  alignas(4*alignof(T)) std::array<T, 4> m_Q;
#endif

public:
  xQuaternion (                         ) {              } //default constructor
  xQuaternion (const T Value            ) { fill(Value); } //parameterized constructor
  xQuaternion (const xQuaternion<T>& Src) { copy(Src);   } //copy constructor
  ~xQuaternion(                         ) {              } //nothing to destruct

  //copy assignment operator
  xQuaternion<T>& operator= (const xQuaternion<T>& Src) { copy(Src); return *this; }

  //convertion operator
  template<typename OtherType> explicit operator xQuaternion<OtherType>() const { xQuaternion<OtherType> Tmp; Tmp.convert(*this); return Tmp; }

  //utils
  inline void copy (const xQuaternion<T>& Src) { memcpy(m_Q.data(), Src.m_Q.data(), 4 * sizeof(T)); }
  inline void load (const T* Src, uint32 SrcStride) { for(uint32 i = 0; i < 4; i++) { m_Q[i] = *Src; Src += SrcStride; } }
  inline void store(T* Dst, uint32 DstStride) const { for(uint32 i = 0; i < 4; i++) { *Dst = m_Q[i]; Dst += DstStride; } }
  inline void zero (             ) { memset(m_Q.data(), 0, 4 * sizeof(T)); }
  inline void fill (const T Value) { xMemsetX<T>(m_Q.data(), Value, 4); }

  //type convertion
  template <typename OtherType> void convert(const xQuaternion<OtherType>& Src) { for(uint32 i = 0; i < 4; i++) { m_Q[i] = (T)(Src[i]); } }

  //quaternion operations - identity
  inline void setIdentity      (                 ) { m_Q[0] = (T)1; m_Q[1] = (T)0; m_Q[2] = (T)0; m_Q[3] = (T)0; }
  inline bool isIdentity       (                 ) const { return (xIsApproximatelyOne(m_Q[0]) && xIsApproximatelyZero(m_Q[1]) && xIsApproximatelyZero(m_Q[2]) && xIsApproximatelyZero(m_Q[3])); }
  inline bool isCloseToIdentity(T ToleranceOffset) const { return (xIsApproximatelyOne(m_Q[0], ToleranceOffset) && xIsApproximatelyZero(m_Q[1], ToleranceOffset) && xIsApproximatelyZero(m_Q[2], ToleranceOffset) && xIsApproximatelyZero(m_Q[3], ToleranceOffset)); }

  //quaternion operations - invert
  void            setInvertion(const xQuaternion<T>& Src)       { m_Q[0] = Src[0]; m_Q[1] = -Src[1]; m_Q[2] = -Src[2]; m_Q[3] = -Src[3]; }
  xQuaternion<T>  getInvertion(                         ) const { xQuaternion<T> Q; Q.setInvertion(*this); return Q; }
  xQuaternion<T>& modInvert   (                         )       { setInvertion(*this); return *this; }

  //quaternion operations - add, sub, mul
  inline xQuaternion<T>& operator  += (const xQuaternion<T>& Quat) { for(uint32 h = 0; h < 4; h++) { m_Q[h] += Quat[h]; } return *this; }
  inline xQuaternion<T>& operator  -= (const xQuaternion<T>& Quat) { for(uint32 h = 0; h < 4; h++) { m_Q[h] -= Quat[h]; } return *this; }
         xQuaternion<T>& operator  *= (const xQuaternion<T>& Quat);
  inline xQuaternion<T>  operator  +  (const xQuaternion<T>& Quat) const { xQuaternion<T> Q; for(uint32 h = 0; h < 4; h++) { Q[h] = m_Q[h] + Quat[h]; } return Q; }
  inline xQuaternion<T>  operator  -  (const xQuaternion<T>& Quat) const { xQuaternion<T> Q; for(uint32 h = 0; h < 4; h++) { Q[h] = m_Q[h] - Quat[h]; } return Q; }
         xQuaternion<T>  operator  *  (const xQuaternion<T>& Quat) const;

  //element access
  inline const  T& at(uint32 e) const { return m_Q.at(e); }
  inline        T& at(uint32 e)       { return m_Q.at(e); }
  inline const  T& operator[] (int32 e) const { return m_Q[e]; }
  inline        T& operator[] (int32 e)       { return m_Q[e]; }
  inline const  T* getPtr() const { return m_Q.data(); }
  inline        T* getPtr()       { return m_Q.data(); }

  //quaternion properties - operation on angles
  inline void setFromAxisAngle  (T x, T y, T z, T Angle);
  inline void setFromAxisAngle  (xVec3<T>& Vec, T Angle) { setFromAxisAngle(Vec[0], Vec[1], Vec[2], Angle); }
  inline void setFromEulerAngles(T Yaw, T Pitch, T Roll);

  //print
  void          display(const std::string& Prefix) const { fmt::print("{}", print(Prefix, "  ")); }
  std::string   print  (const std::string& Prefix1st, const std::string& Separator) const;
};

//===============================================================================================================================================================================================================

#ifndef PMBB_xQuaternion_IMPLEMENTATION
extern template class xQuaternion<flt32>;
extern template class xQuaternion<flt64>;
#endif // !AVlib_xQuaternion_IMPLEMENTATION

//===============================================================================================================================================================================================================

} //end of namespace PMBB

