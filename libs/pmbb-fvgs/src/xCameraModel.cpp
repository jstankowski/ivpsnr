/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define PMBB_xCameraModel_IMPLEMENTATION
#include "xCameraModel.h"

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================
// xRotationMat
//===============================================================================================================================================================================================================
template <class T> xQuaternion<T> xRotationMat<T>::getQuaternion() const
{
  tQuaternion Q;

  T x, y, z, w;
  T Trace = m_E[0][0] + m_E[1][1] + m_E[2][2]; // I removed + 1.0f; see discussion with Ethan

  if(Trace > 0) // I changed M_EPSILON to 0
  {
    T s = (T)0.5 / sqrt(Trace + (T)1.0);
    w = (T)0.25 / s;
    x = (m_E[2][1] - m_E[1][2]) * s;
    y = (m_E[0][2] - m_E[2][0]) * s;
    z = (m_E[1][0] - m_E[0][1]) * s;
  }
  else
  {
    if(m_E[0][0] > m_E[1][1] && m_E[0][0] > m_E[2][2])
    {
      T s = (T)2.0 * sqrt((T)1.0 + m_E[0][0] - m_E[1][1] - m_E[2][2]);
      w = (m_E[2][1] - m_E[1][2]) / s;
      x = (T)0.25 * s;
      y = (m_E[0][1] + m_E[1][0]) / s;
      z = (m_E[0][2] + m_E[2][0]) / s;
    }
    else if(m_E[1][1] > m_E[2][2])
    {
      T s = (T)2.0 * sqrt((T)1.0 + m_E[1][1] - m_E[0][0] - m_E[2][2]);
      w = (m_E[0][2] - m_E[2][0]) / s;
      x = (m_E[0][1] + m_E[1][0]) / s;
      y = (T)0.25 * s;
      z = (m_E[1][2] + m_E[2][1]) / s;
    }
    else
    {
      T s = (T)2.0 * sqrt((T)1.0 + m_E[2][2] - m_E[0][0] - m_E[1][1]);
      w = (m_E[1][0] - m_E[0][1]) / s;
      x = (m_E[0][2] + m_E[2][0]) / s;
      y = (m_E[1][2] + m_E[2][1]) / s;
      z = (T)0.25 * s;
    }
  }
  Q[0] = w;
  Q[1] = x;
  Q[2] = y;
  Q[3] = z;
  return Q;
}
template <class T> void xRotationMat<T>::setQuaternion(tQuaternion Q)
{
  m_E[0][0] = ((T)1.0 - (T)2.0 * (xPow2(Q[2]) + xPow2(Q[3])));
  m_E[1][0] = ((T)2.0 * (Q[1] * Q[2] + Q[3] * Q[0]));
  m_E[2][0] = ((T)2.0 * (Q[1] * Q[3] - Q[2] * Q[0]));
  m_E[0][1] = ((T)2.0 * (Q[1] * Q[2] - Q[3] * Q[0]));
  m_E[1][1] = ((T)1.0 - (T)2.0 * (xPow2(Q[1]) + xPow2(Q[3])));
  m_E[2][1] = ((T)2.0 * (Q[3] * Q[2] + Q[1] * Q[0]));
  m_E[0][2] = ((T)2.0 * (Q[1] * Q[3] + Q[2] * Q[0]));
  m_E[1][2] = ((T)2.0 * (Q[2] * Q[3] - Q[1] * Q[0]));
  m_E[2][2] = ((T)1.0 - (T)2.0 * (xPow2(Q[1]) + xPow2(Q[2])));
}

//===============================================================================================================================================================================================================
// xTranslationVec
//===============================================================================================================================================================================================================
template <class T> void xTranslationVec<T>::setRotation(const xTranslationVec<T>& Src, const tQuaternion& Q, bool Inv)
{
  tQuaternion invQuat  = Q.getInvertion();
  tQuaternion resQuant = !Inv ? ((Q * Src) * invQuat) : (((invQuat) * Src) * Q);
  m_E[0][0] = resQuant[1];
  m_E[1][0] = resQuant[2];
  m_E[2][0] = resQuant[3];
}
template <class T> xTranslationVec<T> xTranslationVec<T>::getCrossProduct(xTranslationVec<T>& Vec) const
{
  return { (at(1) * Vec.at(2)) - (at(2) * Vec.at(1)), (at(2) * Vec.at(0)) - (at(0) * Vec.at(2)), (at(0) * Vec.at(1)) - (at(1) * Vec.at(0)) };
}

//===============================================================================================================================================================================================================
// xDistortion
//===============================================================================================================================================================================================================
template <class T> std::string xDistortionParams<T>::print(const std::string& Separator) const
{
  std::vector<T> DistCoeffs(m_NumParams);
  exportOpenCV(DistCoeffs);

  std::string Result;
  Result.reserve(4096);
  for(int32 i = 0; i < m_NumParams; i++)
  { 
    Result += fmt::sprintf("%14.8f", DistCoeffs[i]);
    if(i < m_NumParams - 1) { Result += Separator; }
    else                    { Result += "\n"; }
  }
  return Result;
}
template <class T> void xDistortionParams<T>::importOpenCV(const std::vector<T>& DistCoeffs)
{
  zero();

  m_NumParams = (int32)DistCoeffs.size();

  if(DistCoeffs.size() >= 4)
  {
    m_RadialNum [0] = DistCoeffs[ 0]; //k1
    m_RadialNum [1] = DistCoeffs[ 1]; //k2
    m_Tangential[0] = DistCoeffs[ 2]; //p1
    m_Tangential[1] = DistCoeffs[ 3]; //p2
  }
  if(DistCoeffs.size() >= 5)
  {
    m_RadialNum [2] = DistCoeffs[ 4]; //k3
  }
  if(DistCoeffs.size() >= 8)
  {
    m_RadialDen [0] = DistCoeffs[ 5]; //k4
    m_RadialDen [1] = DistCoeffs[ 6]; //k5
    m_RadialDen [2] = DistCoeffs[ 7]; //k6
  }
  if(DistCoeffs.size() >= 12)
  {
    m_Prism     [0] = DistCoeffs[ 8]; //s1
    m_Prism     [1] = DistCoeffs[ 9]; //s2 
    m_Prism     [2] = DistCoeffs[10]; //s3
    m_Prism     [3] = DistCoeffs[11]; //s4
  }
  if(DistCoeffs.size() == 14)
  {
    m_Tilt      [0] = DistCoeffs[12]; //tx
    m_Tilt      [1] = DistCoeffs[13]; //ty
  }
}
template <class T> void xDistortionParams<T>::exportOpenCV(std::vector<T>& DistCoeffs) const
{
  if(DistCoeffs.size() == 0) { DistCoeffs.resize(m_NumParams); }

  std::fill(DistCoeffs.begin(), DistCoeffs.end(), (T)0);

  if(DistCoeffs.size() >= 4 && m_NumParams >= 4)
  {
    DistCoeffs[ 0] = m_RadialNum [0]; //k1
    DistCoeffs[ 1] = m_RadialNum [1]; //k2
    DistCoeffs[ 2] = m_Tangential[0]; //p1
    DistCoeffs[ 3] = m_Tangential[1]; //p2
  }
  if(DistCoeffs.size() >= 5 && m_NumParams >= 5)
  {
    DistCoeffs[ 4] = m_RadialNum [2]; //k3
  }
  if(DistCoeffs.size() >= 8 && m_NumParams >= 8)
  {
    DistCoeffs[ 5] = m_RadialDen [0]; //k4
    DistCoeffs[ 6] = m_RadialDen [1]; //k5
    DistCoeffs[ 7] = m_RadialDen [2]; //k6
  }
  if(DistCoeffs.size() >= 12 && m_NumParams >= 12)
  {
    DistCoeffs[ 8] = m_Prism     [0]; //s1
    DistCoeffs[ 9] = m_Prism     [1]; //s2 
    DistCoeffs[10] = m_Prism     [2]; //s3
    DistCoeffs[11] = m_Prism     [3]; //s4
  }
  if(DistCoeffs.size() == 14 && m_NumParams >= 14)
  {
    DistCoeffs[12] = m_Tilt      [0]; //tx
    DistCoeffs[13] = m_Tilt      [1]; //ty
  }
}
template <class T> T xDistortionParams<T>::calcRadialMultiplier(xVec2<T> xy) const
{
  xVec2<T> xyP2 = xy*xy;
  T        rP2  = xyP2.getSum();
  T        kr   = (1 + ((m_RadialNum[2]*rP2 + m_RadialNum[1])*rP2 + m_RadialNum[0])*rP2) / (1 + ((m_RadialDen[2]*rP2 + m_RadialDen[1])*rP2 + m_RadialDen[0])*rP2);
  return kr;
}
template <class T> xVec2<T> xDistortionParams<T>::calcTangentialAddend(xVec2<T> xy) const
{
  xVec2<T> xyP2 = xy*xy;
  T        rP2  = xyP2.getSum();
  T        xy2  = 2 * xy.getMul();
  xVec2<T> pxy  = { (m_Tangential[0] * xy2 + m_Tangential[1] * (rP2 + 2 * xyP2[xV::X])),
                    (m_Tangential[0] * (rP2 + 2 * xyP2[xV::Y]) + m_Tangential[1] * xy2) };
  return pxy;
}

//===============================================================================================================================================================================================================

template class xRotationMat     <flt32>;
template class xTranslationVec  <flt32>;
template class xIntrinsicMat    <flt32>;
template class xExtrinsicMat    <flt32>;
template class xDistortionParams<flt32>;
template class xAngleOfview     <flt32>;
template class xRotationMat     <flt64>;
template class xTranslationVec  <flt64>;
template class xIntrinsicMat    <flt64>;
template class xExtrinsicMat    <flt64>;
template class xDistortionParams<flt64>;
template class xAngleOfview     <flt64>;

//===============================================================================================================================================================================================================

} //end of namespace PMBB
