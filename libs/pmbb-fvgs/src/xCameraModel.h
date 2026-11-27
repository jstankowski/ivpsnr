/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefFVGS.h"
#include "xMatrix.h"
#include "xQuaternion.h"

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================

template <class T> class xRotationMat final : public xMatrix<3,3,T>
{
public:
  using tQuaternion = xQuaternion<T>;
  using xMatrix<3,3,T>::xMatrix; //constructor inheritance

protected:  
  using xMatrix<3,3,T>::m_E;

public:
  tQuaternion getQuaternion(             ) const;
  void        setQuaternion(tQuaternion Q);
};

//===============================================================================================================================================================================================================

template <class T> class xTranslationVec final : public xMatrix<3,1,T>
{
public:
  using tQuaternion = xQuaternion<T>;
  using xMatrix<3,1,T>::xMatrix; //constructor inheritance
  using xMatrix<3,1,T>::at;

protected:
  using xMatrix<3,1,T>::m_E;

public:
  void             setRotation(const xTranslationVec& Src, const tQuaternion& Q, bool Inv);
  xTranslationVec  getRotation(                            const tQuaternion& Q, bool Inv) { xTranslationVec Tr; Tr.setRotation(*this, Q, Inv); return Tr; }
  xTranslationVec& modRotation(                            const tQuaternion& Q, bool Inv) { setRotation(*this, Q, Inv); return *this;}

  T                getLength   () const { return sqrt(xPow2(m_E[0][0]) + xPow2(m_E[1][0]) + xPow2(m_E[2][0])); }
  void             modNormalize() { T Length = getLength(); xMatrix<3,1,T>::modMultiplyByScalar(1 / Length); }

  xTranslationVec  getCrossProduct(xTranslationVec& Vec) const;

  //inline const  std::array<T, 1>& operator[] (int32 y) const = delete;
  //inline        std::array<T, 1>& operator[] (int32 y)       = delete;

  //special multiplication - have to be defined here
  friend xQuaternion<T> operator* (xQuaternion<T> const& Q, xTranslationVec<T> const& Tr)
  {
    xTranslationVec::tQuaternion retQuat;
    retQuat[1] =  Q[0] * Tr[0][0] + Q[2] * Tr[2][0] - Q[3] * Tr[1][0]; // Q.W * V.X + Q.Y * V.Z - Q.Z * V.Y  
    retQuat[2] =  Q[0] * Tr[1][0] + Q[3] * Tr[0][0] - Q[1] * Tr[2][0]; // Q.W * V.Y + Q.Z * V.X - Q.X * V.Z  
    retQuat[3] =  Q[0] * Tr[2][0] + Q[1] * Tr[1][0] - Q[2] * Tr[0][0]; // Q.W * V.Z + Q.X * V.Y - Q.Y * V.X  
    retQuat[0] = -Q[1] * Tr[0][0] - Q[2] * Tr[1][0] - Q[3] * Tr[2][0]; //-Q.X * V.X - Q.Y * V.Y - Q.Z * V.Z
    return retQuat;
  }
};

//===============================================================================================================================================================================================================

template <class T> class xIntrinsicMat final : public xMatrix<4, 4, T>
{
public:
  using tMatrix3x3 = xMatrix<3, 3, T>;

protected:
  using xMatrix<4,4,T>::m_E;

public:
  void rescaleByFactorXY(xVec2<T> ScalingFactorXY) 
  {
    m_E[0][0] *= ScalingFactorXY.getX(); //fx
    m_E[1][1] *= ScalingFactorXY.getY(); //fy
    m_E[0][2] *= ScalingFactorXY.getX(); //cx
    m_E[1][2] *= ScalingFactorXY.getY(); //cy
  }

public:
  xVec2<T> getFocalLengths  () const { return { m_E[0][0], m_E[1][1] }; }
  xVec2<T> getPrincipalPoint() const { return { m_E[0][2], m_E[1][2] }; }

  tMatrix3x3        getIntrinsics3x3 () const { tMatrix3x3 Matrix3x3; for(uint32 h = 0; h < 3; h++) { for(uint32 w = 0; w < 3; w++) { Matrix3x3[h][w] = m_E[h][w]; } }; return Matrix3x3;  }
};

//===============================================================================================================================================================================================================

template <class T> class xExtrinsicMat final : public xMatrix<4, 4, T>
{
public:
  using xRotation    = xRotationMat   <T>;
  using xTranslation = xTranslationVec<T>;
  using tMatrix3x4   = xMatrix<3, 4, T>;

protected:
  using xMatrix<4,4,T>::m_E;

public:
  void              setExtrinsics3x4 (tMatrix3x4  Ext)       { for(uint32 h = 0; h < 3; h++) { for(uint32 w = 0; w < 4; w++) { m_E[h][w] = Ext[h][w]; } }; }
  tMatrix3x4        getExtrinsics3x4 (               ) const { tMatrix3x4 Matrix3x4; for(uint32 h = 0; h < 3; h++) { for(uint32 w = 0; w < 4; w++) { Matrix3x4[h][w] = m_E[h][w]; } }; return Matrix3x4; }
  void              setRotation      (xRotation    RR)       { for(uint32 h = 0; h < 3; h++) { for(uint32 w = 0; w < 3; w++) { m_E[h][w] = RR[h][w]; } }; }
  xRotation         getRotation      (               ) const { xRotation Rotation; for(uint32 h = 0; h < 3; h++) { for(uint32 w = 0; w < 3; w++) { Rotation[h][w] = m_E[h][w]; } }; return Rotation;  }
  void              setTranslation   (xTranslation TT)       { for(uint32 h = 0; h < 3; h++) { m_E[h][3] = TT[h][0]; }; }
  xTranslation      getTranslation   (               ) const { xTranslation Translation; for(uint32 h = 0; h < 3; h++) { Translation[h][0] = m_E[h][3]; }; return Translation;  }
};

//===============================================================================================================================================================================================================

template <class T> class xDistortionParams
{
protected:
  int32            m_NumParams ; //number of active params
  std::array<T, 3> m_RadialNum ; //K = radial distortion (numerotor)
  std::array<T, 3> m_RadialDen ; //K = radial distortion (denominator)
  std::array<T, 2> m_Tangential; //P = tangential distortion
  std::array<T, 4> m_Prism     ; //S = thin prism distortion
  std::array<T, 2> m_Tilt      ; //T = tilted sensor model

public:
  xDistortionParams() { zero(); }
  void        zero () { memset(this, 0, sizeof(xDistortionParams<T>)); }
  std::string print(const std::string& Separator) const;

  void       importOpenCV(const std::vector<T>& DistCoeffs);
  void       exportOpenCV(      std::vector<T>& DistCoeffs) const;

  T        calcRadialMultiplier(xVec2<T> xy) const;
  xVec2<T> calcTangentialAddend(xVec2<T> xy) const;

public:
  int32                   getNumParams    () const { return m_NumParams; }
  std::array<T, 3>&       getRadialNum    ()       { return m_RadialNum; }
  const std::array<T, 3>& getRadialNum    () const { return m_RadialNum; }
  std::array<T, 3>*       getRadialNumPtr ()       { return &m_RadialNum; }
  std::array<T, 3>&       getRadialDen    ()       { return m_RadialDen; }
  const std::array<T, 3>& getRadialDen    () const { return m_RadialDen; }
  std::array<T, 3>*       getRadialDenPtr ()       { return &m_RadialDen; }
  std::array<T, 2>&       getTangential   ()       { return m_Tangential; }
  const std::array<T, 2>& getTangential   () const { return m_Tangential; }
  std::array<T, 2>*       getTangentialPtr()       { return &m_Tangential; }
  std::array<T, 4>&       getPrism        ()       { return m_Prism; }
  const std::array<T, 4>& getPrism        () const { return m_Prism; }
  std::array<T, 4>*       getPrismPtr     ()       { return &m_Prism; }
  std::array<T, 2>&       getTilt         ()       { return m_Tilt; }
  const std::array<T, 2>& getTilt         () const { return m_Tilt; }
  std::array<T, 2>*       getTiltPtr      ()       { return &m_Tilt; }
};

//===============================================================================================================================================================================================================

template <class T> class xAngleOfview
{
protected:
  std::array<T, 4> m_AOV;

public:
  void zero() { m_AOV.fill(0); }
  void set (const std::array <T, 4>& AOV) { m_AOV = AOV; }
  void set (const std::vector<T   >& AOV) { memcpy(m_AOV.data(), AOV.data(), 4*sizeof(T)); }

  T getDegLeft  () const { return m_AOV[0]; }
  T getDegRight () const { return m_AOV[1]; }
  T getDegTop   () const { return m_AOV[2]; }
  T getDegBottom() const { return m_AOV[3]; }
  T getDegWidth () const { return getDegRight() - getDegLeft  (); }
  T getDegHeight() const { return getDegTop  () - getDegBottom(); }

  T getRadLeft  () const { return getDegLeft  () * xc_DegToRad<T>; }
  T getRadRight () const { return getDegRight () * xc_DegToRad<T>; }
  T getRadTop   () const { return getDegTop   () * xc_DegToRad<T>; }
  T getRadBottom() const { return getDegBottom() * xc_DegToRad<T>; }
  T getRadWidth () const { return getDegWidth () * xc_DegToRad<T>; }
  T getRadHeight() const { return getDegHeight() * xc_DegToRad<T>; }
};

//===============================================================================================================================================================================================================

#ifndef PMBB_xCameraModel_IMPLEMENTATION
extern template class xRotationMat     <flt32>;
extern template class xTranslationVec  <flt32>;
extern template class xIntrinsicMat    <flt32>;
extern template class xExtrinsicMat    <flt32>;
extern template class xDistortionParams<flt32>;
extern template class xAngleOfview     <flt32>;
extern template class xRotationMat     <flt64>;
extern template class xTranslationVec  <flt64>;
extern template class xIntrinsicMat    <flt64>;
extern template class xExtrinsicMat    <flt64>;
extern template class xDistortionParams<flt64>;
extern template class xAngleOfview     <flt64>;
#endif // !PMBB_xCameraModel_IMPLEMENTATION

//===============================================================================================================================================================================================================

} //end of namespace PMBB

