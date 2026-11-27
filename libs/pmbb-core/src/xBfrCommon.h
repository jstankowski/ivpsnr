/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefCORE.h"
#include "xVec.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xPicBfrCommon
//===============================================================================================================================================================================================================
class xBfrCommon
{
public:
  using tPSV = const int32V2&; //position or size vector (int32V2)

protected:
  int16V2 m_Size     = { NOT_VALID , NOT_VALID };
  int8    m_BitDepth = NOT_VALID;

public:
  xBfrCommon() = default;
  //avoid pictures beeing copied or assigned
  xBfrCommon(const xBfrCommon&) = delete;  //delete copy constructor
  xBfrCommon& operator= (const xBfrCommon&) = delete; //delete assignement operator

public:
  //inter-buffer compatibility functions
  inline bool isSameSize    (tPSV  Size                ) const { return (int32V2)m_Size == Size; }
  inline bool isSameBitDepth(int32 BitDepth            ) const { return m_BitDepth == (int8)BitDepth; }
  inline bool isCompatible  (tPSV  Size, int32 BitDepth) const { return (isSameSize(Size) && isSameBitDepth(BitDepth)); }

  inline bool isSameSize    (const xBfrCommon* Pic) const { return isSameSize    (Pic->getSize  ()); }
  inline bool isSameBitDepth(const xBfrCommon* Pic) const { return isSameBitDepth(Pic->m_BitDepth); }
  inline bool isCompatible  (const xBfrCommon* Pic) const { return isSameSize(Pic) && isSameBitDepth(Pic); }

  //parameters
  inline int32V2 getSize    () const { return (int32V2)m_Size; }
  inline int32   getWidth   () const { return m_Size.getX()  ; }
  inline int32   getHeight  () const { return m_Size.getY()  ; }
  inline int32   getArea    () const { return (int32)m_Size.getX() * (int32)m_Size.getY(); }
  inline int32   getBitDepth() const { return m_BitDepth     ; }

  //data type/range helpers
  inline uint16 getMaxPelValue() const { return (uint16)xBitDepth2MaxValue(m_BitDepth); }
  inline uint16 getMidPelValue() const { return (uint16)xBitDepth2MidValue(m_BitDepth); }
  inline uint16 getMinPelValue() const { return (uint16)0;                              }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB


