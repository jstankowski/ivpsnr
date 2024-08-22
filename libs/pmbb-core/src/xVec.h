/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefCORE.h"
#include <array>
#include <vector>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// VecX - named access
//===============================================================================================================================================================================================================
class xV { public: enum eVec { X = 0, Y = 1, Z = 2, A = 3 }; };

//===============================================================================================================================================================================================================
// Vec2
//===============================================================================================================================================================================================================
template<typename XXX> class xVec2
{
public:
  using tVec = xVec2<XXX>;

protected:
  alignas(2 * alignof(XXX)) XXX m_V[2];

public:
  inline           xVec2() {}
  inline constexpr xVec2(const XXX x, const XXX y) : m_V{ x, y } {}

  inline xVec2(const XXX* V               ) { std::memcpy(m_V, V       , sizeof(m_V)); }
  inline xVec2(const std::vector<XXX>&   V) { std::memcpy(m_V, V.data(), sizeof(m_V)); }
  inline xVec2(const std::array<XXX, 2>& A) { std::memcpy(m_V, A.data(), sizeof(m_V)); }

  template<typename YYY> explicit operator xVec2<YYY>() const { return xVec2<YYY>((YYY)m_V[0], (YYY)m_V[1]); }

  inline void  set1    (const XXX v                 ) { m_V[0] = v; m_V[1] = v; }
  inline void  set     (const XXX x, const XXX y    ) { m_V[0] = x; m_V[1] = y; }
  inline void  set     (const XXX* V                ) { std::memcpy(m_V, V       , sizeof(m_V)); }
  inline void  set     (const std::vector<XXX>&   V ) { std::memcpy(m_V, V.data(), sizeof(m_V)); }
  inline void  set     (const std::array<XXX, 2>& A ) { std::memcpy(m_V, A.data(), sizeof(m_V)); }
  inline void  setIdx  (const XXX v, const int32 Idx) { m_V[Idx] = v; }
  inline void  setX    (const XXX x                 ) { m_V[0] = x;}
  inline void  setY    (const XXX y                 ) { m_V[1] = y;}
  inline void  setZero (                            ) { std::memset(m_V, 0, sizeof(m_V));}

  inline void  get       (XXX* restrict V        ) const { std::memcpy(V         , m_V       , sizeof(m_V)); }
  inline void  get       (std::vector<XXX>& Vec  ) const { std::memcpy(Vec.data(), m_V.data(), sizeof(m_V)); }
  inline void  get       (std::array<XXX, 2>& Arr) const { std::memcpy(Arr.data(), m_V.data(), sizeof(m_V)); }
  inline XXX   getIdx    (const int32 Idx)         const { return m_V[Idx]; }
  inline XXX   getX      ()                        const { return m_V[0];}
  inline XXX   getY      ()                        const { return m_V[1];}  
  inline XXX   getAbsX   ()                        const { return xAbs(m_V[0]);}
  inline XXX   getAbsY   ()                        const { return xAbs(m_V[1]);}

  inline tVec  getVecAbs () const { return tVec(xAbs(m_V[0]), xAbs(m_V[1])); }
  inline tVec  getVecPow2() const { return tVec(xPow2(m_V[0]), xPow2(m_V[1])); }

  inline XXX   getMax    () const { return xMax(m_V[0], m_V[1]);}
  inline XXX   getMin    () const { return xMin(m_V[0], m_V[1]);}
  inline XXX   getMaxAbs () const { return xMax(xAbs(m_V[0]), xAbs(m_V[1]));}
  inline XXX   getMinAbs () const { return xMin(xAbs(m_V[0]), xAbs(m_V[1]));}
  inline XXX   getSum    () const { return (m_V[0] + m_V[1]);}  
  inline XXX   getSumAbs () const { return (xAbs(m_V[0]) + xAbs(m_V[1]));}
  inline XXX   getSumPow2() const { return (xPow2(m_V[0]) + xPow2(m_V[1]));}
  inline XXX   getMul    () const { return m_V[0] * m_V[1];}

  inline const  XXX* getElementsPtr() const { return m_V; }
  inline        XXX* getElementsPtr()       { return m_V; }

  template <typename YYY> inline YYY getSumCvtPow2() const { return (xPow2((YYY)m_V[0]) + xPow2((YYY)m_V[1]));}

  inline const tVec  operator -  () const { return xVec2(-m_V[0], -m_V[1]); }

  inline const tVec& operator += (const tVec& Vec2) {m_V[0]  += Vec2.m_V[0]; m_V[1]  += Vec2.m_V[1]; return *this; }  
  inline const tVec& operator -= (const tVec& Vec2) {m_V[0]  -= Vec2.m_V[0]; m_V[1]  -= Vec2.m_V[1]; return *this; }
  inline const tVec& operator *= (const tVec& Vec2) {m_V[0]  *= Vec2.m_V[0]; m_V[1]  *= Vec2.m_V[1]; return *this; }
  inline const tVec& operator /= (const tVec& Vec2) {m_V[0]  /= Vec2.m_V[0]; m_V[1]  /= Vec2.m_V[1]; return *this; }
  inline const tVec& operator %= (const tVec& Vec2) {m_V[0]  %= Vec2.m_V[0]; m_V[1]  %= Vec2.m_V[1]; return *this; }
  inline const tVec& operator>>= (const tVec& Vec2) {m_V[0] >>= Vec2.m_V[0]; m_V[1] >>= Vec2.m_V[1]; return *this; }
  inline const tVec& operator<<= (const tVec& Vec2) {m_V[0] <<= Vec2.m_V[0]; m_V[1] <<= Vec2.m_V[1]; return *this; }
               
  inline const tVec& operator += (const XXX   i   ) {m_V[0]  += i;           m_V[1]  += i;           return *this; }
  inline const tVec& operator -= (const XXX   i   ) {m_V[0]  -= i;           m_V[1]  -= i;           return *this; }
  inline const tVec& operator *= (const XXX   i   ) {m_V[0]  *= i;           m_V[1]  *= i;           return *this; }
  inline const tVec& operator /= (const XXX   i   ) {m_V[0]  /= i;           m_V[1]  /= i;           return *this; }
  inline const tVec& operator %= (const XXX   i   ) {m_V[0]  %= i;           m_V[1]  %= i;           return *this; }
  inline const tVec& operator>>= (const XXX   i   ) {m_V[0] >>= i;           m_V[1] >>= i;           return *this; }
  inline const tVec& operator<<= (const XXX   i   ) {m_V[0] <<= i;           m_V[1] <<= i;           return *this; }
  inline const tVec& operator &= (const XXX   i   ) {m_V[0]  &= i;           m_V[1]  &= i;           return *this; }
  inline const tVec& operator |= (const XXX   i   ) {m_V[0]  |= i;           m_V[1]  |= i;           return *this; }
  inline const tVec& operator ^= (const XXX   i   ) {m_V[0]  ^= i;           m_V[1]  ^= i;           return *this; }

  inline const tVec  operator -  (const tVec& Vec2) const { return xVec2(m_V[0]  - Vec2.m_V[0], m_V[1]  - Vec2.m_V[1]); }
  inline const tVec  operator +  (const tVec& Vec2) const { return xVec2(m_V[0]  + Vec2.m_V[0], m_V[1]  + Vec2.m_V[1]); }  
  inline const tVec  operator *  (const tVec& Vec2) const { return xVec2(m_V[0]  * Vec2.m_V[0], m_V[1]  * Vec2.m_V[1]); }
  inline const tVec  operator /  (const tVec& Vec2) const { return xVec2(m_V[0]  / Vec2.m_V[0], m_V[1]  / Vec2.m_V[1]); }
  inline const tVec  operator %  (const tVec& Vec2) const { return xVec2(m_V[0]  % Vec2.m_V[0], m_V[1]  % Vec2.m_V[1]); }
  inline const tVec  operator>>  (const tVec& Vec2) const { return xVec2(m_V[0] >> Vec2.m_V[0], m_V[1] >> Vec2.m_V[1]); }
  inline const tVec  operator<<  (const tVec& Vec2) const { return xVec2(m_V[0] << Vec2.m_V[0], m_V[1] << Vec2.m_V[1]); }

  inline const tVec  operator +  (const XXX   i   ) const { return xVec2(m_V[0]  + i,           m_V[1]  + i          ); }
  inline const tVec  operator -  (const XXX   i   ) const { return xVec2(m_V[0]  - i,           m_V[1]  - i          ); }
  inline const tVec  operator *  (const XXX   i   ) const { return xVec2(m_V[0]  * i,           m_V[1]  * i          ); }
  inline const tVec  operator /  (const XXX   i   ) const { return xVec2(m_V[0]  / i,           m_V[1]  / i          ); }
  inline const tVec  operator %  (const XXX   i   ) const { return xVec2(m_V[0]  % i,           m_V[1]  % i          ); }
  inline const tVec  operator<<  (const XXX   i   ) const { return xVec2(m_V[0] << i,           m_V[1] << i          ); }
  inline const tVec  operator>>  (const XXX   i   ) const { return xVec2(m_V[0] >> i,           m_V[1] >> i          ); }
  inline const tVec  operator &  (const XXX   i   ) const { return xVec2(m_V[0]  & i,           m_V[1]  & i          ); }
  inline const tVec  operator |  (const XXX   i   ) const { return xVec2(m_V[0]  | i,           m_V[1]  | i          ); }
  inline const tVec  operator ^  (const XXX   i   ) const { return xVec2(m_V[0]  ^ i,           m_V[1]  ^ i          ); }

  inline bool        operator==  (const tVec& Vec2) const { return (m_V[0]==Vec2.m_V[0] && m_V[1]==Vec2.m_V[1]);}
  inline bool        operator!=  (const tVec& Vec2) const { return (m_V[0]!=Vec2.m_V[0] || m_V[1]!=Vec2.m_V[1]);}
  inline bool        operator==  (const XXX i)      const {return (m_V[0]==i           && m_V[1]==i          );}

  inline       XXX&  operator[]  (int32 Idx)       {return m_V[Idx];}
  inline const XXX&  operator[]  (int32 Idx) const {return m_V[Idx];}

  inline bool  isZero   () const { return (m_V[0]==0 && m_V[1]==0); }
  inline bool  isNonZero() const { return (m_V[0]!=0 && m_V[1]!=0); } 

  inline bool  isInsideSize  (xVec2<XXX> Size) const { return (m_V[0] >= 0 && m_V[0] < Size[0] && m_V[1] >= 0 && m_V[1] < Size[1]); }
  inline bool  isInsideRange (XXX MinX, XXX MaxX, XXX MinY, XXX MaxY) const {return (m_V[0]>=MinX && m_V[0]<=MaxX && m_V[1]>=MinY && m_V[1]<=MaxY);}
  inline bool  isOutsideRange(XXX MinX, XXX MaxX, XXX MinY, XXX MaxY) const {return (m_V[0]< MinX || m_V[0]> MaxX || m_V[1]< MinY || m_V[1]> MaxY);}

  inline void  modClip (const tVec& Min, const tVec& Max) { m_V[0] = xClip (m_V[0], Min.m_V[0], Max.m_V[0]); m_V[1] = xClip (m_V[1], Min.m_V[1], Max.m_V[1]); }
  inline void  modClipU(                 const tVec& Max) { m_V[0] = xClipU(m_V[0],             Max.m_V[0]); m_V[1] = xClipU(m_V[1],             Max.m_V[1]); }
  inline void  modClipU(                         XXX Max) { m_V[0] = xClipU(m_V[0],             Max       ); m_V[1] = xClipU(m_V[1],             Max       ); }

  inline tVec  getClip (const tVec& Min, const tVec& Max) const { return { xClip (m_V[0], Min.m_V[0], Max.m_V[0]), xClip (m_V[1], Min.m_V[1], Max.m_V[1]) }; }
  inline tVec  getClipU(                 const tVec& Max) const { return { xClipU(m_V[0],             Max.m_V[0]), xClipU(m_V[1],             Max.m_V[1]) }; }
  inline tVec  getClipU(                         XXX Max) const { return { xClipU(m_V[0],             Max       ), xClipU(m_V[1],             Max       ) }; }
};

//===============================================================================================================================================================================================================
// Vec3 - if performance is important - do not use 
//===============================================================================================================================================================================================================
template<typename XXX> class xVec3
{
public:
  using tVec = xVec3<XXX>;

protected:
  XXX m_V[3];

public:
  xVec3() {}
  xVec3(XXX x, XXX y, XXX z)           { m_V[0] = x; m_V[1] = y; m_V[2] = z;}
  xVec3(const XXX* V)                  { std::memcpy(m_V, V, sizeof(m_V)); }
  xVec3(const std::vector<XXX>&   V)   { std::memcpy(m_V, V.data(), sizeof(m_V)); }
  xVec3(const std::array<XXX, 3>& A)   { std::memcpy(m_V, A.data(), sizeof(m_V)); }
  xVec3(const xVec2<XXX>& Vec2, XXX z) { m_V[0] = Vec2[0]; m_V[1] = Vec2[1]; m_V[2] = z; }
  xVec3(XXX x, const xVec2<XXX>& Vec2) { m_V[0] = x; m_V[1] = Vec2[0]; m_V[2] = Vec2[1]; }

  template<typename YYY> explicit operator xVec3<YYY>() const { return xVec3<YYY>((YYY)m_V[0], (YYY)m_V[1], (YYY)m_V[2]); }

  inline void  set1    (const XXX x, const XXX y, const XXX z) {m_V[0] = x; m_V[1] = y; m_V[2] = z; }
  inline void  set     (const XXX* V)                          { std::memcpy(m_V, V, sizeof(m_V)); }
  inline void  setIdx  (const XXX v, const int32 Idx)          {m_V[Idx] = v; }
  inline void  setX    (const XXX x)                           {m_V[0] = x;}
  inline void  setY    (const XXX y)                           {m_V[1] = y;}
  inline void  setZ    (const XXX z)                           {m_V[2] = z;}
  inline void  setZero ()                                      {std::memset(m_V, 0, sizeof(m_V));}

  inline XXX   getIdx    (const int32 Idx) const {return m_V[Idx]; }
  inline void  get       (XXX* V         ) const { std::memcpy(V, m_V, sizeof(m_V)); }
  inline XXX   getX      (               ) const {return m_V[0];}
  inline XXX   getY      (               ) const {return m_V[1];}
  inline XXX   getZ      (               ) const {return m_V[2];}
  inline XXX   getAbsX   (               ) const {return xAbs(m_V[0]);}
  inline XXX   getAbsY   (               ) const {return xAbs(m_V[1]);}
  inline XXX   getAbsZ   (               ) const {return xAbs(m_V[2]);}
  inline XXX   getMax    (               ) const {return xMax(xMax(m_V[0], m_V[1]), m_V[2]);}
  inline XXX   getMin    (               ) const {return xMin(xMin(m_V[0], m_V[1]), m_V[2]);}
  inline XXX   getMaxAbs (               ) const {return xMax(xMax(xAbs(m_V[0]), xAbs(m_V[1])), xAbs(m_V[2]));}
  inline XXX   getMinAbs (               ) const {return xMin(xMin(xAbs(m_V[0]), xAbs(m_V[1])), xAbs(m_V[2]));}
  inline XXX   getSum    (               ) const {return (m_V[0] + m_V[1] + m_V[2]);}
  inline XXX   getSumSqrs(               ) const {return (xPow2(m_V[0]) + xPow2(m_V[1]) + xPow2(m_V[2]));}
  inline XXX   getSumAbs (               ) const {return (xAbs(m_V[0]) + xAbs(m_V[1]) + xAbs(m_V[2]));}
  inline XXX   getSumPow2(               ) const {return (xPow2(m_V[0]) + xPow2(m_V[1]) + xPow2(m_V[2])); }
  inline XXX   getMul    (               ) const {return m_V[0] * m_V[1] * m_V[2];}

  inline xVec2<XXX> SubsetToVec2() const { return xVec2<XXX>(m_V[0], m_V[1]); }

  template <typename YYY> inline YYY getSumCvtPow2() const {return (xPow2((YYY)m_V[0]) + xPow2((YYY)m_V[1]) + xPow2((YYY)m_V[2]));}

  inline const  XXX* getElementsPtr() const { return m_V; }
  inline        XXX* getElementsPtr()       { return m_V; }

  inline const tVec& operator += (const tVec& Vec3)       {m_V[0]  += Vec3.m_V[0]; m_V[1]  += Vec3.m_V[1]; m_V[2]  += Vec3.m_V[2]; return *this;}  
  inline const tVec& operator -= (const tVec& Vec3)       {m_V[0]  -= Vec3.m_V[0]; m_V[1]  -= Vec3.m_V[1]; m_V[2]  -= Vec3.m_V[2]; return *this;}
  inline const tVec& operator *= (const tVec& Vec3)       {m_V[0]  *= Vec3.m_V[0]; m_V[1]  *= Vec3.m_V[1]; m_V[2]  *= Vec3.m_V[2]; return *this;}
  inline const tVec& operator /= (const tVec& Vec3)       {m_V[0]  /= Vec3.m_V[0]; m_V[1]  /= Vec3.m_V[1]; m_V[2]  /= Vec3.m_V[2]; return *this;}
  inline const tVec& operator %= (const tVec& Vec3)       {m_V[0]  %= Vec3.m_V[0]; m_V[1]  %= Vec3.m_V[1]; m_V[2]  %= Vec3.m_V[2]; return *this;}
  inline const tVec& operator>>= (const tVec& Vec3)       {m_V[0] >>= Vec3.m_V[0]; m_V[1] >>= Vec3.m_V[1]; m_V[2] >>= Vec3.m_V[2]; return *this;}
  inline const tVec& operator<<= (const tVec& Vec3)       {m_V[0] <<= Vec3.m_V[0]; m_V[1] <<= Vec3.m_V[1]; m_V[2] <<= Vec3.m_V[2]; return *this;}

  inline const tVec& operator += (const XXX i)                  {m_V[0]  += i;           m_V[1]  += i;           m_V[2]  += i;           return *this;}
  inline const tVec& operator -= (const XXX i)                  {m_V[0]  -= i;           m_V[1]  -= i;           m_V[2]  -= i;           return *this;}
  inline const tVec& operator *= (const XXX i)                  {m_V[0]  *= i;           m_V[1]  *= i;           m_V[2]  *= i;           return *this;}
  inline const tVec& operator /= (const XXX i)                  {m_V[0]  /= i;           m_V[1]  /= i;           m_V[2]  /= i;           return *this;}
  inline const tVec& operator %= (const XXX i)                  {m_V[0]  %= i;           m_V[1]  %= i;           m_V[2]  %= i;           return *this;}
  inline const tVec& operator>>= (const XXX i)                  {m_V[0] >>= i;           m_V[1] >>= i;           m_V[2] >>= i;           return *this;}  
  inline const tVec& operator<<= (const XXX i)                  {m_V[0] <<= i;           m_V[1] <<= i;           m_V[2] <<= i;           return *this;}  
  inline const tVec& operator &= (const XXX i)                  {m_V[0]  &= i;           m_V[1]  &= i;           m_V[2]  &= i;           return *this;}
  inline const tVec& operator |= (const XXX i)                  {m_V[0]  |= i;           m_V[1]  |= i;           m_V[2]  |= i;           return *this;}
  inline const tVec& operator ^= (const XXX i)                  {m_V[0]  ^= i;           m_V[1]  ^= i;           m_V[2]  ^= i;           return *this;}

  inline const tVec  operator +  (const tVec& Vec3) const {return xVec3(m_V[0]  + Vec3.m_V[0], m_V[1]  + Vec3.m_V[1], m_V[2]  + Vec3.m_V[2]);}  
  inline const tVec  operator -  (const tVec& Vec3) const {return xVec3(m_V[0]  - Vec3.m_V[0], m_V[1]  - Vec3.m_V[1], m_V[2]  - Vec3.m_V[2]);}
  inline const tVec  operator *  (const tVec& Vec3) const {return xVec3(m_V[0]  * Vec3.m_V[0], m_V[1]  * Vec3.m_V[1], m_V[2]  * Vec3.m_V[2]);}
  inline const tVec  operator /  (const tVec& Vec3) const {return xVec3(m_V[0]  / Vec3.m_V[0], m_V[1]  / Vec3.m_V[1], m_V[2]  / Vec3.m_V[2]);}
  inline const tVec  operator %  (const tVec& Vec3) const {return xVec3(m_V[0]  % Vec3.m_V[0], m_V[1]  % Vec3.m_V[1], m_V[2]  % Vec3.m_V[2]);}
  inline const tVec  operator>>  (const tVec& Vec3) const {return xVec3(m_V[0] >> Vec3.m_V[0], m_V[1] >> Vec3.m_V[1], m_V[2] >> Vec3.m_V[2]);}
  inline const tVec  operator<<  (const tVec& Vec3) const {return xVec3(m_V[0] << Vec3.m_V[0], m_V[1] << Vec3.m_V[1], m_V[2] << Vec3.m_V[2]);}

  inline const tVec  operator +  (const XXX i) const            {return xVec3(m_V[0]  + i,           m_V[1]  + i,           m_V[2]  + i          );}
  inline const tVec  operator -  (const XXX i) const            {return xVec3(m_V[0]  - i,           m_V[1]  - i,           m_V[2]  - i          );}
  inline const tVec  operator *  (const XXX i) const            {return xVec3(m_V[0]  * i,           m_V[1]  * i,           m_V[2]  * i          );}
  inline const tVec  operator /  (const XXX i) const            {return xVec3(m_V[0]  / i,           m_V[1]  / i,           m_V[2]  / i          );}
  inline const tVec  operator<<  (const XXX i) const            {return xVec3(m_V[0] << i,           m_V[1] << i,           m_V[2] << i          );}
  inline const tVec  operator>>  (const XXX i) const            {return xVec3(m_V[0] >> i,           m_V[1] >> i,           m_V[2] >> i          );}
  inline const tVec  operator &  (const XXX i) const            {return xVec3(m_V[0]  & i,           m_V[1]  & i,           m_V[2]  & i          );}
  inline const tVec  operator |  (const XXX i) const            {return xVec3(m_V[0]  | i,           m_V[1]  | i,           m_V[2]  | i          );}
  inline const tVec  operator ^  (const XXX i) const            {return xVec3(m_V[0]  ^ i,           m_V[1]  ^ i,           m_V[2]  ^ i          );}

  inline bool              operator==  (const tVec& Vec3) const {return (m_V[0]==Vec3.m_V[0] && m_V[1]==Vec3.m_V[1] && m_V[2]==Vec3.m_V[2]);}
  inline bool              operator!=  (const tVec& Vec3) const {return (m_V[0]!=Vec3.m_V[0] || m_V[1]!=Vec3.m_V[1] || m_V[2]!=Vec3.m_V[2]);}
  inline bool              operator==  (const XXX i) const            {return (m_V[0]==i           && m_V[1]==i           && m_V[2]==i          );}
  inline bool              operator!=  (const XXX i) const            {return (m_V[0]!=i           || m_V[1]!=i           || m_V[2]!=i          );}

  inline XXX&              operator[]  (int32 Idx)       {return m_V[Idx];}
  inline const XXX&        operator[]  (int32 Idx) const {return m_V[Idx];}

  inline bool  isZero()    const { return (m_V[0]==0 && m_V[1]==0 && m_V[2]==0); }
  inline bool  isNonZero() const { return (m_V[0]!=0 && m_V[1]!=0 && m_V[2]!=0); } 

  inline bool  isInsideSize  (tVec Size) const { return (m_V[0] >= 0 && m_V[0] < Size[0] && m_V[1] >= 0 && m_V[1] < Size[1] && m_V[2] >= 0 && m_V[2] < Size[2]); }
  inline bool  isInsideRange (XXX MinX, XXX MaxX, XXX MinY, XXX MaxY, XXX MinZ, XXX MaxZ) const {return (m_V[0]>=MinX && m_V[0]<=MaxX && m_V[1]>=MinY && m_V[1]<=MaxY && m_V[2]>=MinZ && m_V[2]<=MaxZ);}
  inline bool  isOutsideRange(XXX MinX, XXX MaxX, XXX MinY, XXX MaxY, XXX MinZ, XXX MaxZ) const {return (m_V[0]< MinX || m_V[0]> MaxX || m_V[1]< MinY || m_V[1]> MaxY || m_V[2]< MinZ || m_V[2]> MaxZ);}

  inline void  clip (tVec& Min, tVec& Max) { m_V[0] = xClip (m_V[0], Min.m_V[0], Max.m_V[0]); m_V[1] = xClip (m_V[1], Min.m_V[1], Max.m_V[1]); m_V[2] = xClip (m_V[2], Min.m_V[2], Max.m_V[2]); }
  inline void  clipU(                 tVec& Max) { m_V[0] = xClipU(m_V[0],             Max.m_V[0]); m_V[1] = xClipU(m_V[1],             Max.m_V[1]); m_V[2] = xClipU(m_V[2],             Max.m_V[2]); }
};

//===============================================================================================================================================================================================================
// Vec4
//===============================================================================================================================================================================================================
template<typename XXX> class xVec4
{
public:
  using tVec = xVec4<XXX>;

protected:
  alignas(4 * alignof(XXX)) XXX m_V[4];

public:
  inline           xVec4() {}
  inline constexpr xVec4(const XXX x, const XXX y, const XXX z, const XXX a) : m_V{ x, y, z, a } {}

  inline xVec4(const XXX* V               ) { std::memcpy(m_V, V       , sizeof(m_V)); }
  inline xVec4(const std::vector<XXX>&   V) { std::memcpy(m_V, V.data(), sizeof(m_V)); }
  inline xVec4(const std::array<XXX, 4>& A) { std::memcpy(m_V, A.data(), sizeof(m_V)); }

  //template<typename YYY> inline xVec4(xVec4<YYY> Vec) { m_V[0] = (XXX)Vec[0]; m_V[1] = (XXX)Vec[1]; m_V[2] = (XXX)Vec[2]; m_V[3] = (XXX)Vec[3]; }
  template<typename YYY> explicit operator xVec4<YYY>() const { return xVec4<YYY>((YYY)m_V[0], (YYY)m_V[1], (YYY)m_V[2], (YYY)m_V[3]); }

  inline void  set1    (const XXX v                                       ) { m_V[0] = v; m_V[1] = v; m_V[2] = v; m_V[3] = v; }
  inline void  set     (const XXX x, const XXX y, const XXX z, const XXX a) { m_V[0] = x; m_V[1] = y; m_V[2] = z; m_V[3] = a; }
  inline void  setIdx  (const XXX v, const int32 Idx                      ) { m_V[Idx] = v; }
  inline void  setX    (const XXX x                                       ) { m_V[0] = x; }
  inline void  setY    (const XXX y                                       ) { m_V[1] = y; }
  inline void  setZ    (const XXX z                                       ) { m_V[2] = z; }
  inline void  setA    (const XXX a                                       ) { m_V[3] = a; }
  inline void  setZero (                                                  ) { std::memset(m_V, 0, sizeof(m_V)); }

  inline void  get       (XXX* restrict V        ) const { std::memcpy(V         , m_V       , sizeof(m_V)); }
  inline void  get       (std::vector<XXX>& Vec  ) const { std::memcpy(Vec.data(), m_V.data(), sizeof(m_V)); }
  inline void  get       (std::array<XXX, 4>& Arr) const { std::memcpy(Arr.data(), m_V.data(), sizeof(m_V)); }
  inline XXX   getIdx    (const int32 Idx        ) const { return m_V[Idx]; }
  inline XXX   getX      (                       ) const { return m_V[0];}
  inline XXX   getY      (                       ) const { return m_V[1];}
  inline XXX   getZ      (                       ) const { return m_V[2];}
  inline XXX   getA      (                       ) const { return m_V[3];}
  inline XXX   getAbsX   (                       ) const { return xAbs(m_V[0]);}
  inline XXX   getAbsY   (                       ) const { return xAbs(m_V[1]);}
  inline XXX   getAbsZ   (                       ) const { return xAbs(m_V[2]);}
  inline XXX   getAbsA   (                       ) const { return xAbs(m_V[3]);}
  inline tVec  getReplace(XXX v, int32 Idx       ) const { tVec V; V[Idx]=v; return V; }

  inline tVec  getVecAbs () const { return tVec(xAbs(m_V[0]), xAbs(m_V[1]), xAbs(m_V[2]), xAbs(m_V[3])); }
  inline tVec  getVecPow2() const { return tVec(xPow2(m_V[0]), xPow2(m_V[1]), xPow2(m_V[2]), xPow2(m_V[3])); }
  
  //terrible slow (since std::pow is slow) but sometimes usefull, defined only for floating point types
  //template <typename = typename std::enable_if_t<std::is_floating_point_v<XXX>>>
  inline tVec  getVecPow1(XXX  P) const { return tVec(std::pow(m_V[0], P), std::pow(m_V[1], P), std::pow(m_V[2], P), std::pow(m_V[3], P)); }
  //template <typename = typename std::enable_if_t<std::is_floating_point_v<XXX>>>
  inline tVec  getVecPowV(tVec P) const { return tVec(std::pow(m_V[0], P[0]), std::pow(m_V[1], P[1]), std::pow(m_V[2], P[2]), std::pow(m_V[3], P[3])); }

  inline XXX   getMax    () const {return xMax(xMax(m_V[0], m_V[1]), xMax(m_V[2], m_V[3]));}
  inline XXX   getMin    () const {return xMin(xMin(m_V[0], m_V[1]), xMin(m_V[2], m_V[3]));}
  inline XXX   getMaxAbs () const {return xMax(xMax(xAbs(m_V[0]), xAbs(m_V[1])), xMax(xAbs(m_V[2]), xAbs(m_V[3])));}
  inline XXX   getMinAbs () const {return xMin(xMin(xAbs(m_V[0]), xAbs(m_V[1])), xMin(xAbs(m_V[2]), xAbs(m_V[3])));}
  inline XXX   getSum    () const {return (m_V[0] + m_V[1] + m_V[2] + m_V[3]);}
  inline XXX   getSumAbs () const {return (xAbs(m_V[0]) + xAbs(m_V[1]) + xAbs(m_V[2]) + xAbs(m_V[3]));}
  inline XXX   getSumPow2() const {return (xPow2(m_V[0]) + xPow2(m_V[1]) + xPow2(m_V[2]) + xPow2(m_V[3])); }
  inline XXX   getMul    () const {return m_V[0] * m_V[1] * m_V[2] * m_V[3];}

  template <typename YYY> inline YYY getSumCvtPow2() const { return (xPow2((YYY)m_V[0]) + xPow2((YYY)m_V[1]) + xPow2((YYY)m_V[2]) + xPow2((YYY)m_V[3])); }

  inline const  XXX* getElementsPtr() const { return m_V; }
  inline        XXX* getElementsPtr()       { return m_V; }

  inline const tVec  operator -  () const { return xVec4(-m_V[0], -m_V[1], -m_V[2], -m_V[3]); }

  inline const tVec& operator += (const tVec& Vec4) {m_V[0]  += Vec4.m_V[0]; m_V[1]  += Vec4.m_V[1]; m_V[2]  += Vec4.m_V[2]; m_V[3]  += Vec4.m_V[3]; return *this;}
  inline const tVec& operator -= (const tVec& Vec4) {m_V[0]  -= Vec4.m_V[0]; m_V[1]  -= Vec4.m_V[1]; m_V[2]  -= Vec4.m_V[2]; m_V[3]  -= Vec4.m_V[3]; return *this;}
  inline const tVec& operator *= (const tVec& Vec4) {m_V[0]  *= Vec4.m_V[0]; m_V[1]  *= Vec4.m_V[1]; m_V[2]  *= Vec4.m_V[2]; m_V[3]  *= Vec4.m_V[3]; return *this;}
  inline const tVec& operator /= (const tVec& Vec4) {m_V[0]  /= Vec4.m_V[0]; m_V[1]  /= Vec4.m_V[1]; m_V[2]  /= Vec4.m_V[2]; m_V[3]  /= Vec4.m_V[3]; return *this;}
  inline const tVec& operator %= (const tVec& Vec4) {m_V[0]  %= Vec4.m_V[0]; m_V[1]  %= Vec4.m_V[1]; m_V[2]  %= Vec4.m_V[2]; m_V[3]  %= Vec4.m_V[3]; return *this;}
  inline const tVec& operator>>= (const tVec& Vec4) {m_V[0] >>= Vec4.m_V[0]; m_V[1] >>= Vec4.m_V[1]; m_V[2] >>= Vec4.m_V[2]; m_V[3] >>= Vec4.m_V[3]; return *this;}
  inline const tVec& operator<<= (const tVec& Vec4) {m_V[0] <<= Vec4.m_V[0]; m_V[1] <<= Vec4.m_V[1]; m_V[2] <<= Vec4.m_V[2]; m_V[3] <<= Vec4.m_V[3]; return *this;}

  inline const tVec& operator += (const XXX   i   ) {m_V[0]  += i;           m_V[1]  += i;           m_V[2]  += i;           m_V[3]  += i;           return *this;}
  inline const tVec& operator -= (const XXX   i   ) {m_V[0]  -= i;           m_V[1]  -= i;           m_V[2]  -= i;           m_V[3]  -= i;           return *this;}
  inline const tVec& operator *= (const XXX   i   ) {m_V[0]  *= i;           m_V[1]  *= i;           m_V[2]  *= i;           m_V[3]  *= i;           return *this;}
  inline const tVec& operator /= (const XXX   i   ) {m_V[0]  /= i;           m_V[1]  /= i;           m_V[2]  /= i;           m_V[3]  /= i;           return *this;}
  inline const tVec& operator %= (const XXX   i   ) {m_V[0]  %= i;           m_V[1]  %= i;           m_V[2]  %= i;           m_V[3]  %= i;           return *this;}
  inline const tVec& operator>>= (const XXX   i   ) {m_V[0] >>= i;           m_V[1] >>= i;           m_V[2] >>= i;           m_V[3] >>= i;           return *this;}  
  inline const tVec& operator<<= (const XXX   i   ) {m_V[0] <<= i;           m_V[1] <<= i;           m_V[2] <<= i;           m_V[3] <<= i;           return *this;}  
  inline const tVec& operator &= (const XXX   i   ) {m_V[0]  &= i;           m_V[1]  &= i;           m_V[2]  &= i;           m_V[3]  &= i;           return *this;}
  inline const tVec& operator |= (const XXX   i   ) {m_V[0]  |= i;           m_V[1]  |= i;           m_V[2]  |= i;           m_V[3]  |= i;           return *this;}
  inline const tVec& operator ^= (const XXX   i   ) {m_V[0]  ^= i;           m_V[1]  ^= i;           m_V[2]  ^= i;           m_V[3]  ^= i;           return *this;}

  inline const tVec  operator +  (const tVec& Vec4) const { return xVec4(m_V[0]  + Vec4.m_V[0], m_V[1]  + Vec4.m_V[1], m_V[2]  + Vec4.m_V[2], m_V[3]  + Vec4.m_V[3]);}  
  inline const tVec  operator -  (const tVec& Vec4) const { return xVec4(m_V[0]  - Vec4.m_V[0], m_V[1]  - Vec4.m_V[1], m_V[2]  - Vec4.m_V[2], m_V[3]  - Vec4.m_V[3]);}
  inline const tVec  operator *  (const tVec& Vec4) const { return xVec4(m_V[0]  * Vec4.m_V[0], m_V[1]  * Vec4.m_V[1], m_V[2]  * Vec4.m_V[2], m_V[3]  * Vec4.m_V[3]);}
  inline const tVec  operator /  (const tVec& Vec4) const { return xVec4(m_V[0]  / Vec4.m_V[0], m_V[1]  / Vec4.m_V[1], m_V[2]  / Vec4.m_V[2], m_V[3]  / Vec4.m_V[3]);}
  inline const tVec  operator %  (const tVec& Vec4) const { return xVec4(m_V[0]  % Vec4.m_V[0], m_V[1]  % Vec4.m_V[1], m_V[2]  % Vec4.m_V[2], m_V[3]  % Vec4.m_V[3]);}
  inline const tVec  operator>>  (const tVec& Vec4) const { return xVec4(m_V[0] >> Vec4.m_V[0], m_V[1] >> Vec4.m_V[1], m_V[2] >> Vec4.m_V[2], m_V[3] >> Vec4.m_V[3]);}
  inline const tVec  operator<<  (const tVec& Vec4) const { return xVec4(m_V[0] << Vec4.m_V[0], m_V[1] << Vec4.m_V[1], m_V[2] << Vec4.m_V[2], m_V[3] << Vec4.m_V[3]);}

  inline const tVec  operator +  (const XXX   i   ) const { return xVec4(m_V[0]  + i,           m_V[1]  + i,           m_V[2]  + i,           m_V[3]  + i          );}
  inline const tVec  operator -  (const XXX   i   ) const { return xVec4(m_V[0]  - i,           m_V[1]  - i,           m_V[2]  - i,           m_V[3]  - i          );}
  inline const tVec  operator *  (const XXX   i   ) const { return xVec4(m_V[0]  * i,           m_V[1]  * i,           m_V[2]  * i,           m_V[3]  * i          );}
  inline const tVec  operator /  (const XXX   i   ) const { return xVec4(m_V[0]  / i,           m_V[1]  / i,           m_V[2]  / i,           m_V[3]  / i          );}
  inline const tVec  operator %  (const XXX   i   ) const { return xVec4(m_V[0]  % i,           m_V[1]  % i,           m_V[2]  % i,           m_V[3]  % i          );}
  inline const tVec  operator<<  (const XXX   i   ) const { return xVec4(m_V[0] << i,           m_V[1] << i,           m_V[2] << i,           m_V[3] << i          );}
  inline const tVec  operator>>  (const XXX   i   ) const { return xVec4(m_V[0] >> i,           m_V[1] >> i,           m_V[2] >> i,           m_V[3] >> i          );}
  inline const tVec  operator &  (const XXX   i   ) const { return xVec4(m_V[0]  & i,           m_V[1]  & i,           m_V[2]  & i,           m_V[3]  & i          );}
  inline const tVec  operator |  (const XXX   i   ) const { return xVec4(m_V[0]  | i,           m_V[1]  | i,           m_V[2]  | i,           m_V[3]  | i          );}
  inline const tVec  operator ^  (const XXX   i   ) const { return xVec4(m_V[0]  ^ i,           m_V[1]  ^ i,           m_V[2]  ^ i,           m_V[3]  ^ i          );}

  constexpr inline bool operator==  (const tVec& Vec4) const { return (m_V[0]==Vec4.m_V[0] && m_V[1]==Vec4.m_V[1] && m_V[2]==Vec4.m_V[2] && m_V[3]==Vec4.m_V[3]);}
  constexpr inline bool operator!=  (const tVec& Vec4) const { return (m_V[0]!=Vec4.m_V[0] || m_V[1]!=Vec4.m_V[1] || m_V[2]!=Vec4.m_V[2] || m_V[3]!=Vec4.m_V[3]);}
  constexpr inline bool operator==  (const XXX   i   ) const { return (m_V[0]==i           && m_V[1]==i           && m_V[2]==i           && m_V[3]==i          );}

  inline XXX&              operator[]  (int32 Idx)       {return m_V[Idx];}
  inline const XXX&        operator[]  (int32 Idx) const {return m_V[Idx];}

  inline bool  isZero   () const { return (m_V[0]==0 && m_V[1]==0 && m_V[2]==0 && m_V[3]==0); }
  inline bool  isNonZero() const { return (m_V[0]!=0 && m_V[1]!=0 && m_V[2]!=0 && m_V[3]!=0); } 

  inline bool  isInsideSize  (xVec4<XXX> Size) const { return (m_V[0] >= 0 && m_V[0] < Size[0] && m_V[1] >= 0 && m_V[1] < Size[1] && m_V[2] >= 0 && m_V[2] < Size[2] && m_V[3] >= 0 && m_V[3] < Size[3]); }
  inline bool  isInsideRange (XXX MinX, XXX MaxX, XXX MinY, XXX MaxY, XXX MinZ, XXX MaxZ, XXX MinA, XXX MaxA) const {return (m_V[0]>=MinX && m_V[0]<=MaxX && m_V[1]>=MinY && m_V[1]<=MaxY && m_V[2]>=MinZ && m_V[2]<=MaxZ && m_V[3]>=MinA && m_V[3]<=MaxA);}
  inline bool  isOutsideRange(XXX MinX, XXX MaxX, XXX MinY, XXX MaxY, XXX MinZ, XXX MaxZ, XXX MinA, XXX MaxA) const {return (m_V[0]< MinX || m_V[0]> MaxX || m_V[1]< MinY || m_V[1]> MaxY || m_V[2]< MinZ || m_V[2]> MaxZ || m_V[3]< MinA || m_V[3]> MaxA);}

  inline void  modClip (const tVec& Min, const tVec& Max) { m_V[0] = xClip (m_V[0], Min.m_V[0], Max.m_V[0]); m_V[1] = xClip (m_V[1], Min.m_V[1], Max.m_V[1]); m_V[2] = xClip (m_V[2], Min.m_V[2], Max.m_V[2]); m_V[3] = xClip (m_V[3], Min.m_V[3], Max.m_V[3]); }
  inline void  modClipU(                 const tVec& Max) { m_V[0] = xClipU(m_V[0],             Max.m_V[0]); m_V[1] = xClipU(m_V[1],             Max.m_V[1]); m_V[2] = xClipU(m_V[2],             Max.m_V[2]); m_V[3] = xClipU(m_V[3],             Max.m_V[3]); }

  inline tVec  getClip (const tVec& Min, const tVec& Max) const { return { xClip (m_V[0], Min.m_V[0], Max.m_V[0]), xClip (m_V[1], Min.m_V[1], Max.m_V[1]) , xClip (m_V[2], Min.m_V[2], Max.m_V[2]) , xClip (m_V[3], Min.m_V[3], Max.m_V[3]) }; }
  inline tVec  getClipU(                 const tVec& Max) const { return { xClipU(m_V[0],             Max.m_V[0]), xClipU(m_V[1],             Max.m_V[1]) , xClipU(m_V[2],             Max.m_V[2]) , xClipU(m_V[3],             Max.m_V[3]) }; }
};

//===============================================================================================================================================================================================================
// predefined vectors
//===============================================================================================================================================================================================================
typedef xVec2<  bool>   boolV2;
typedef xVec2<  int8>   int8V2;
typedef xVec2< uint8>  uint8V2;
typedef xVec2< int16>  int16V2;
typedef xVec2<uint16> uint16V2;
typedef xVec2< int32>  int32V2;
typedef xVec2<uint32> uint32V2;
typedef xVec2< int64>  int64V2;
typedef xVec2<uint64> uint64V2;
typedef xVec2< flt32>  flt32V2;
typedef xVec2< flt64>  flt64V2;

typedef xVec3<  bool>   boolV3;
typedef xVec3<  int8>   int8V3;
typedef xVec3< uint8>  uint8V3;
typedef xVec3< int16>  int16V3;
typedef xVec3<uint16> uint16V3;
typedef xVec3< int32>  int32V3;
typedef xVec3<uint32> uint32V3;
typedef xVec3< int64>  int64V3;
typedef xVec3<uint64> uint64V3;
typedef xVec3< flt32>  flt32V3;
typedef xVec3< flt64>  flt64V3;

typedef xVec4<  bool>   boolV4;
typedef xVec4<  int8>   int8V4;
typedef xVec4< uint8>  uint8V4;
typedef xVec4< int16>  int16V4;
typedef xVec4<uint16> uint16V4;
typedef xVec4< int32>  int32V4;
typedef xVec4<uint32> uint32V4;
typedef xVec4< int64>  int64V4;
typedef xVec4<uint64> uint64V4;
typedef xVec4< flt32>  flt32V4;
typedef xVec4< flt64>  flt64V4;

//===============================================================================================================================================================================================================
// vector from scalar
//===============================================================================================================================================================================================================
template<typename XXX> static inline xVec2<XXX> xMakeVec2(XXX Value) { return xVec2<XXX>{Value, Value              }; }
template<typename XXX> static inline xVec3<XXX> xMakeVec3(XXX Value) { return xVec3<XXX>{Value, Value, Value       }; }
template<typename XXX> static inline xVec4<XXX> xMakeVec4(XXX Value) { return xVec4<XXX>{Value, Value, Value, Value}; }

//===============================================================================================================================================================================================================
// special rounding routines
//===============================================================================================================================================================================================================
template <class XXX> static inline int32V2 xRoundFltToInt32(const xVec2<XXX>& FltV);
template <> inline int32V2 xRoundFltToInt32(const xVec2<flt32>& FltV) { return { xRoundFlt32ToInt32(FltV[0]), xRoundFlt32ToInt32(FltV[1])}; }
template <> inline int32V2 xRoundFltToInt32(const xVec2<flt64>& FltV) { return { xRoundFlt64ToInt32(FltV[0]), xRoundFlt64ToInt32(FltV[1])}; }

template <class XXX> static inline int32V3 xRoundFltToInt32(const xVec3<XXX>& FltV);
template <> inline int32V3 xRoundFltToInt32(const xVec3<flt32>& FltV) { return { xRoundFlt32ToInt32(FltV[0]), xRoundFlt32ToInt32(FltV[1]), xRoundFlt32ToInt32(FltV[2])}; }
template <> inline int32V3 xRoundFltToInt32(const xVec3<flt64>& FltV) { return { xRoundFlt64ToInt32(FltV[0]), xRoundFlt64ToInt32(FltV[1]), xRoundFlt64ToInt32(FltV[2])}; }

template <class XXX> static inline int32V4 xRoundFltToInt32(const xVec4<XXX>& FltV);
template <> inline int32V4 xRoundFltToInt32(const xVec4<flt32>& FltV) { return { xRoundFlt32ToInt32(FltV[0]), xRoundFlt32ToInt32(FltV[1]), xRoundFlt32ToInt32(FltV[2]), xRoundFlt32ToInt32(FltV[3])}; }
template <> inline int32V4 xRoundFltToInt32(const xVec4<flt64>& FltV) { return { xRoundFlt64ToInt32(FltV[0]), xRoundFlt64ToInt32(FltV[1]), xRoundFlt64ToInt32(FltV[2]), xRoundFlt64ToInt32(FltV[3])}; }

//===============================================================================================================================================================================================================

} //end of namespace PMBB
