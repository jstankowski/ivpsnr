/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefCORE.h"
#include "xPicCommon.h"
#include "xVec.h"
#include <mutex>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xPicP - planar
//===============================================================================================================================================================================================================
class xPicP : public xPicCommon
{
protected:
  uint16* m_Buffer[c_MaxNumCmps] = { nullptr, nullptr, nullptr, nullptr }; //picture buffer
  uint16* m_Origin[c_MaxNumCmps] = { nullptr, nullptr, nullptr, nullptr }; //pel origin, pel access -> m_PelOrg[y*m_PelStride + x]

public:
  //constructors $ destructors
  xPicP () { };
  xPicP (int32V2 Size, int32 BitDepth, int32 Margin = c_DefMargin) { create(Size, BitDepth, Margin); }
  ~xPicP() { destroy(); }

  //genral functions
  void   create (int32V2 Size, int32 BitDepth, int32 Margin = c_DefMargin);
  void   create (const xPicP *Ref) { create(Ref->getSize(), Ref->getBitDepth(), Ref->getMargin()); }
  void   destroy();

  void   clear  (                            );
  void   copy   (const xPicP* Src            );
  void   copy   (const xPicP* Src, eCmp CmpId) { assert(isCompatible(Src)); xMemcpyX(m_Buffer[(int32)CmpId], Src->m_Buffer[(int32)CmpId], m_BuffCmpNumPels); }
  void   fill   (uint16 Value                );
  void   fill   (uint16 Value    , eCmp CmpId);
  bool   check  (const std::string& Name     )  const;
  void   conceal(                            );
  void   extend (                            );

  bool   equalPic (const xPicP* Src)  const;
  bool   equalCmp (const xPicP* Src, eCmp CmpId)  const;
  boolV4 equalCmps(const xPicP* Src)  const;

  //access picture data
  inline int32         getStride(                            ) const { return m_Stride              ; }
  inline int32         getPitch (                            ) const { return 1                     ; }  
  inline uint16*       getAddr  (                  eCmp CmpId)       { return m_Origin[(int32)CmpId]; }
  inline const uint16* getAddr  (                  eCmp CmpId) const { return m_Origin[(int32)CmpId]; }
  inline int32         getOffset(int32V2 Position            ) const { return Position.getY() * m_Stride + Position.getX(); }
  inline uint16*       getAddr  (int32V2 Position, eCmp CmpId)       { return getAddr(CmpId) + getOffset(Position); }
  inline const uint16* getAddr  (int32V2 Position, eCmp CmpId) const { return getAddr(CmpId) + getOffset(Position); }
  //slow pel access
  inline uint16&       accessPel(int32V2 Position, eCmp CmpId)       { return *(getAddr(CmpId) + getOffset(Position)); }
  inline const uint16& accessPel(int32V2 Position, eCmp CmpId) const { return *(getAddr(CmpId) + getOffset(Position)); }
  inline uint16&       accessPel(int32   Offset  , eCmp CmpId)       { return *(getAddr(CmpId) + Offset); }
  inline const uint16& accessPel(int32   Offset  , eCmp CmpId) const { return *(getAddr(CmpId) + Offset); }

  //low level buffer modification / access - dangerous
  inline int32         getBuffNumPels(          ) const { return m_BuffCmpNumPels ; }
  inline int32V2       getBuffSize   (          ) const { return int32V2(2*m_Margin+m_Width, 2 * m_Margin + m_Height); }
  inline int32         getBuffArea   (          ) const { return m_BuffCmpNumBytes; }
  inline uint16*       getBuffer     (eCmp CmpId)       { return m_Buffer[(int32)CmpId]; }
  inline const uint16* getBuffer     (eCmp CmpId) const { return m_Buffer[(int32)CmpId]; }
         bool          bindBuffer    (uint16*  Buffer, eCmp CmpId);
         uint16*       unbindBuffer  (                 eCmp CmpId);
         bool          swapBuffer    (uint16*& Buffer, eCmp CmpId);
         bool          swapBuffer    (xPicP* TheOther, eCmp CmpId);
         bool          swapBuffers   (xPicP* TheOther);
         bool          swapComponents(eCmp CmpIdA, eCmp CmpIdB);
};

//===============================================================================================================================================================================================================
// xPicI - interleaved
//===============================================================================================================================================================================================================
class xPicI : public xPicCommon
{
protected:
  uint16* m_Buffer = nullptr;
  uint16* m_Origin = nullptr;

public:
  //general functions
  xPicI () { };
  xPicI (int32V2 Size, int32 BitDepth, int32 Margin = c_DefMargin) { create(Size, BitDepth, Margin); }
  ~xPicI() { destroy(); }

  void   create (int32V2 Size, int32 BitDepth, int32 Margin = c_DefMargin);
  void   create (const xPicI* Ref) { create(Ref->getSize(), Ref->getBitDepth(), Ref->getMargin()); }
  void   destroy();

  void   clear();
  void   copy (const xPicI* Src);
  void   fill (uint16 Value    );

  //convertion
  void rearrangeFromPlanar(const xPicP* Planar);
  void rearrangeToPlanar  (      xPicP* Planar);

  //interleaved access
  //inline int32   getStride() const { return m_Stride<<2; }
  //inline int32   getPitch () const { return 4; }

  //vector access
  inline int32           getStride(                ) const { return m_Stride; }
  inline int32           getPitch (                ) const { return 1; }
  inline int32           getOffset(int32V2 Position) const { return (Position.getY() * m_Stride + Position.getX()); }
  inline uint16V4*       getAddr  (                )       { return (uint16V4*)m_Origin; }
  inline const uint16V4* getAddr  (                ) const { return (uint16V4*)m_Origin; }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
