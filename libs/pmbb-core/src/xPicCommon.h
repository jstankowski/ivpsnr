/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefPMBB-CORE.h"
#include "xVec.h"
#include <mutex>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xPicCommon
//===============================================================================================================================================================================================================
class xPicCommon
{
public:
  static constexpr int32 c_MaxNumCmps = 4; //maximum number of components - do non change
  static constexpr int32 c_DefNumCmps = 3;
  static constexpr int32 c_DefMargin  = 4;

protected:
  int32   m_Width            = NOT_VALID;
  int32   m_Height           = NOT_VALID;
  int32   m_Margin           = NOT_VALID;
  int32   m_Stride           = NOT_VALID;
  int32   m_BitDepth         = NOT_VALID;
  int32   m_NumCmps          = NOT_VALID;

  int32   m_BytesPerSample   = NOT_VALID;
  int32   m_BuffCmpNumPels   = NOT_VALID; //size of picture buffer (including margin) in number of PELs
  int32   m_BuffCmpNumBytes  = NOT_VALID; //size of picture buffer (including margin) in bytes

  int64   m_POC              = NOT_VALID;
  int64   m_Timestamp        = NOT_VALID;

  bool    m_IsMarginExtended = false;

protected:
  void xInit  (int32V2 Size, int32 BitDepth, int32 Margin, int32 NumCmps, int32 PelNumBytes);
  void xUnInit();

public:
  //inter-buffer compatibility functions
  inline bool isSameSize      (int32V2 Size                              ) const { return (m_Width == Size.getX() && m_Height == Size.getY()); }
  inline bool isSameMargin    (int32 Margin                              ) const { return (m_Margin == Margin); }
  inline bool isSameSizeMargin(int32V2 Size, int32 Margin                ) const { return isSameSize(Size) && isSameMargin(Margin); }
  inline bool isSameBitDepth  (int32 BitDepth                            ) const { return m_BitDepth == BitDepth; }
  inline bool isSameNumCmps   (int32 NumCmps                             ) const { return m_NumCmps == NumCmps; }
  inline bool isCompatible    (int32V2 Size, int32 BitDepth, int32 Margin) const { return (isSameSizeMargin(Size, Margin) && isSameBitDepth(BitDepth)); }

  inline bool isSameSize      (const xPicCommon* Pic) const { return isSameSize  (Pic->getSize  ()); }
  inline bool isSameMargin    (const xPicCommon* Pic) const { return isSameMargin(Pic->getMargin()); }
  inline bool isSameSizeMargin(const xPicCommon* Pic) const { return isSameSize(Pic) && isSameMargin(Pic); }
  inline bool isSameBitDepth  (const xPicCommon* Pic) const { return isSameBitDepth(Pic->m_BitDepth); }
  inline bool isSameNumCmps   (const xPicCommon* Pic) const { return isSameNumCmps(Pic->m_NumCmps); }
  inline bool isCompatible    (const xPicCommon* Pic) const { return isSameSizeMargin(Pic) && isSameBitDepth(Pic) && isSameNumCmps(Pic); }

  //parameters
  inline int32V2 getSize    () const { return int32V2(m_Width, m_Height); }
  inline int32   getWidth   () const { return m_Width           ; }
  inline int32   getHeight  () const { return m_Height          ; }
  inline int32   getArea    () const { return m_Width * m_Height; }
  inline int32   getMargin  () const { return m_Margin          ; }
  inline int32   getBitDepth() const { return m_BitDepth        ; }
  inline int32   getNumCmps () const { return m_NumCmps         ; }

  //time
  inline void  setPOC      (int64 POC      )       { m_POC = POC; }
  inline int64 getPOC      (               ) const { return m_POC; }
  inline void  setTimestamp(int64 Timestamp)       { m_Timestamp = Timestamp; }
  inline int64 getTimestamp(               ) const { return m_Timestamp; }

  //data type/range helpers
  inline uint16  getMaxPelValue() const { return (uint16)xBitDepth2MaxValue(m_BitDepth); }
  inline uint16  getMidPelValue() const { return (uint16)xBitDepth2MidValue(m_BitDepth); }
  inline uint16  getMinPelValue() const { return (uint16)0;                              }
};

//===============================================================================================================================================================================================================
// xRentalCommon
//===============================================================================================================================================================================================================
class xRentalCommon
{
protected:
  std::vector<xPicCommon*> m_Buffer;
  std::mutex m_Mutex;
  uintSize   m_CreatedUnits;
  uintSize   m_SizeLimit;

public:
  void       setSizeLimit(uintSize SizeLimit) { m_SizeLimit = SizeLimit; while(m_Buffer.size() > SizeLimit) { xDestroyUnit(); } }

  uintSize   getLoad        () { return m_Buffer.size(); }
  uintSize   getCreatedUnits() { return m_CreatedUnits;  }

protected:
  xPicCommon* xBorrow  ();
  void        xGiveback(xPicCommon* PicCommon);

  virtual void xCreateNewUnit() = 0;
  virtual void xDestroyUnit  () = 0;
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
