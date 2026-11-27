/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define PMBB_xQuaternion_IMPLEMENTATION
#include "xQuaternion.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

template<typename T> xQuaternion<T> xQuaternion<T>::operator * (const xQuaternion<T>& Quat) const
{
  xQuaternion<T> Q;
  Q[0] = m_Q[0] * Quat[0] - m_Q[1] * Quat[1] - m_Q[2] * Quat[2] - m_Q[3] * Quat[3];
  Q[1] = m_Q[0] * Quat[1] + m_Q[1] * Quat[0] + m_Q[2] * Quat[3] - m_Q[3] * Quat[2];
  Q[2] = m_Q[0] * Quat[2] + m_Q[2] * Quat[0] + m_Q[3] * Quat[1] - m_Q[1] * Quat[3];
  Q[3] = m_Q[0] * Quat[3] + m_Q[3] * Quat[0] + m_Q[1] * Quat[2] - m_Q[2] * Quat[1];
  
  return Q;
}
template<typename T> xQuaternion<T>& xQuaternion<T>::operator *= (const xQuaternion<T>& Quat)
{
  std::array<T, 4> Tmp;
  Tmp[0] = m_Q[0] * Quat[0] - m_Q[1] * Quat[1] - m_Q[2] * Quat[2] - m_Q[3] * Quat[3];
  Tmp[1] = m_Q[0] * Quat[1] + m_Q[1] * Quat[0] + m_Q[2] * Quat[3] - m_Q[3] * Quat[2];
  Tmp[2] = m_Q[0] * Quat[2] + m_Q[2] * Quat[0] + m_Q[3] * Quat[1] - m_Q[1] * Quat[3];
  Tmp[3] = m_Q[0] * Quat[3] + m_Q[3] * Quat[0] + m_Q[1] * Quat[2] - m_Q[2] * Quat[1];  
  m_Q = Tmp;
  return *this;
}
template<typename T> void xQuaternion<T>::setFromAxisAngle(T x, T y, T z, T Angle)
{
  T Result = (T)sin(Angle / 2.0);
  m_Q[0] = (T)cos(Angle / 2.0);
  m_Q[1] = T(x * Result);
  m_Q[2] = T(y * Result);
  m_Q[3] = T(z * Result);
}
template<typename T> void xQuaternion<T>::setFromEulerAngles(T Yaw, T Pitch, T Roll)
{
  T YawDiv2   = Yaw   * (T)0.5;
  T PitchDiv2 = Pitch * (T)0.5;
  T RollDiv2  = Roll  * (T)0.5;
  T CosYaw    = cos(YawDiv2);
  T SinYaw    = sin(YawDiv2);
  T CosPitch  = cos(PitchDiv2);
  T SinPitch  = sin(PitchDiv2);
  T CosRoll   = cos(RollDiv2);
  T SinRoll   = sin(RollDiv2);

  m_Q[1] = CosRoll * SinPitch * CosYaw + SinRoll * CosPitch * SinYaw;
  m_Q[2] = CosRoll * CosPitch * SinYaw - SinRoll * SinPitch * CosYaw;
  m_Q[3] = SinRoll * CosPitch * CosYaw - CosRoll * SinPitch * SinYaw;
  m_Q[0] = CosRoll * CosPitch * CosYaw + SinRoll * SinPitch * SinYaw;
}
template<typename T> std::string xQuaternion<T>::print(const std::string& FstPrefix, const std::string& Separator) const
{
  std::string Result;
  Result += FstPrefix;
  for(uint32 e = 0; e < 4; e++) { Result += fmt::sprintf("%14.8f", m_Q[e]); Result += Separator; }
  Result += "\n";
  return Result;
}

//===============================================================================================================================================================================================================

template class xQuaternion<flt32>;
template class xQuaternion<flt64>;

//===============================================================================================================================================================================================================

} //end of namespace PMBB

