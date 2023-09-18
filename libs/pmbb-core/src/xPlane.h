/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefPMBB-CORE.h"
#include "xPicCommon.h"
#include <type_traits>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xPlane
//===============================================================================================================================================================================================================
template <typename PelType> class xPlane : public xPicCommon
{
public:
  typedef PelType T;

protected:
  PelType* m_Buffer = nullptr; //picture buffer
  PelType* m_Origin = nullptr; //pel origin, pel access -> m_PelOrg[y*m_PelStride + x]

public:
  //general functions
  xPlane () = default;
  xPlane (int32V2 Size, int32 BitDepth, int32 Margin = c_DefMargin) { create(Size, BitDepth, Margin); }
  xPlane (xPlane* Ref) { create(Ref); }
  ~xPlane() { destroy(); }
                                   
  void   create (int32V2 Size, int32 BitDepth, int32 Margin = c_DefMargin);
  void   create (const xPlane* Ref) { create(Ref->getSize(), Ref->getBitDepth(), Ref->getMargin()); }
  void   destroy();

  void   clear  ();
  void   copy   (const xPlane* Src);
  void   fill   (PelType Value);
  bool   check  (const std::string& Name);
  void   extend ();                                   
                                   
  //data type/range helpers       
  PelType getMaxPelValue() const { if constexpr (std::is_integral_v<PelType>) { return (PelType)xBitDepth2MaxValue(m_BitDepth);} else { return (PelType)1.0; } }
  PelType getMidPelValue() const { if constexpr (std::is_integral_v<PelType>) { return (PelType)xBitDepth2MidValue(m_BitDepth);} else { return (PelType)0.5; } }
  PelType getMinPelValue() const { return (PelType)0; }

  //access picture data
  inline int32          getStride(                ) const { return m_Stride; }
  inline int32          getPitch (                ) const { return 1       ; }  
  inline PelType*       getAddr  (                )       { return m_Origin; } 
  inline const PelType* getAddr  (                ) const { return m_Origin; }
  inline int32          getOffset(int32V2 Position) const { return Position.getY() * m_Stride + Position.getX(); }
  inline PelType*       getAddr  (int32V2 Position)       { return getAddr() + getOffset(Position); }
  inline const PelType* getAddr  (int32V2 Position) const { return getAddr() + getOffset(Position); }
  //slow pel access
  inline PelType&       accessPel(int32V2 Position)       { return *(getAddr() + getOffset(Position)); }
  inline const PelType& accessPel(int32V2 Position) const { return *(getAddr() + getOffset(Position)); }
  inline PelType&       accessPel(int32   Offset  )       { return *(getAddr() + Offset); }
  inline const PelType& accessPel(int32   Offset  ) const { return *(getAddr() + Offset); }

  //low level buffer modification / access - dangerous
  inline int32          getBuffNumPels() const { return m_BuffCmpNumPels ; }
  inline int32          getBuffSize   () const { return m_BuffCmpNumBytes; }
  inline PelType*       getBuffer     ()       { return m_Buffer; }
  inline const PelType* getBuffer     () const { return m_Buffer; }  
         bool           bindBuffer    (PelType*  Buffer);
         PelType*       unbindBuffer  (                );
         bool           swapBuffer    (PelType*& Buffer);
         bool           swapBuffer    (xPlane* TheOther);              

  //access picture data for specific block unit - TODO - to check
  //PelType*    getBlockPelAddr      (int32 BlockWidth,     int32 BlockHeight,     int32 BlockPosX, int32 BlockPosY) { return m_Origin + ( BlockPosY * m_Stride   * BlockHeight    ) + (BlockPosX  * BlockWidth    ); }
  //PelType*    getBlockPelAddrLog2  (int32 Log2BlockWidth, int32 Log2BlockHeight, int32 BlockPosX, int32 BlockPosY) { return m_Origin + ((BlockPosY * m_Stride) << Log2BlockHeight) + (BlockPosX << Log2BlockWidth); }
  //PelType*    getBlockPelAddr      (int32 BlockWidth,     int32 BlockHeight,     int16V2 BlockPelPos) { return m_Origin + ( BlockPelPos.getY() * m_Stride   * BlockHeight    ) + (BlockPelPos.getX()  * BlockWidth    ); }
  //PelType*    getBlockPelAddrLog2  (int32 Log2BlockWidth, int32 Log2BlockHeight, int16V2 BlockPelPos) { return m_Origin + ((BlockPelPos.getY() * m_Stride) << Log2BlockHeight) + (BlockPelPos.getX() << Log2BlockWidth); }

  //int32       getBlockPelOffsetLog2(int32V2 BlockPos, int32   Log2BlockSize) const { return ((BlockPos.getY() * m_Stride) << Log2BlockSize       ) + (BlockPos.getX() << Log2BlockSize       ); }
  //int32       getBlockPelOffsetLog2(int32V2 BlockPos, int32V2 Log2BlockSize) const { return ((BlockPos.getY() * m_Stride) << Log2BlockSize.getY()) + (BlockPos.getX() << Log2BlockSize.getX()); }
  //int32       getBlockPelOffset    (int32V2 BlockPos, int32   BlockSize    ) const { return (BlockPos.getY() * BlockSize        * m_Stride) + (BlockPos.getX() * BlockSize       ); }
  //int32       getBlockPelOffset    (int32V2 BlockPos, int32V2 BlockSize    ) const { return (BlockPos.getY() * BlockSize.getY() * m_Stride) + (BlockPos.getX() * BlockSize.getX()); }

  //PelType*       getBlockPelAddrLog2(int32V2 BlockPos, int32   Log2BlockSize)       { return getAddr() + getBlockPelOffsetLog2(BlockPos, Log2BlockSize); }
  //const PelType* getBlockPelAddrLog2(int32V2 BlockPos, int32   Log2BlockSize) const { return getAddr() + getBlockPelOffsetLog2(BlockPos, Log2BlockSize); }
  //PelType*       getBlockPelAddrLog2(int32V2 BlockPos, int32V2 Log2BlockSize)       { return getAddr() + getBlockPelOffsetLog2(BlockPos, Log2BlockSize); }
  //const PelType* getBlockPelAddrLog2(int32V2 BlockPos, int32V2 Log2BlockSize) const { return getAddr() + getBlockPelOffsetLog2(BlockPos, Log2BlockSize); }
  //PelType*       getBlockPelAddr    (int32V2 BlockPos, int32   BlockSize    )       { return getAddr() + getBlockPelOffset    (BlockPos, BlockSize    ); }
  //const PelType* getBlockPelAddr    (int32V2 BlockPos, int32   BlockSize    ) const { return getAddr() + getBlockPelOffset    (BlockPos, BlockSize    ); }
  //PelType*       getBlockPelAddr    (int32V2 BlockPos, int32V2 BlockSize    )       { return getAddr() + getBlockPelOffset    (BlockPos, BlockSize    ); }
  //const PelType* getBlockPelAddr    (int32V2 BlockPos, int32V2 BlockSize    ) const { return getAddr() + getBlockPelOffset    (BlockPos, BlockSize    ); }
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// xPlane - instantiation for base types
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#ifndef PMBB_xPlane_IMPLEMENTATION
extern template class xPlane<uint8 >;
extern template class xPlane< int8 >;
extern template class xPlane<uint16>;
extern template class xPlane< int16>;
extern template class xPlane<uint32>;
extern template class xPlane< int32>;
extern template class xPlane<uint64>;
extern template class xPlane< int64>;
extern template class xPlane< flt32>;
extern template class xPlane< flt64>;
#endif // !PMBB_xPlane_IMPLEMENTATION

//===============================================================================================================================================================================================================
// xPlaneRental
//===============================================================================================================================================================================================================
template <typename PelType> class xPlaneRental : public xRentalCommon
{
public:
  typedef PelType T;
  using tUnitPtrV = std::vector<xPlane<T>*>;

protected:
  //Unit creation parameters
  int32V2    m_Size;
  int32      m_Margin;
  int32      m_BitDepth;

public:
  void       create         (int32V2 Size, int32 Margin, int32 BitDepth, uintSize InitSize = 0, uintSize SizeLimit = std::numeric_limits<uintSize>::max());
  void       recreate       (int32V2 Size, int32 Margin, int32 BitDepth, uintSize InitSize = 0, uintSize SizeLimit = std::numeric_limits<uintSize>::max()) { destroy(); create(Size, Margin, BitDepth, InitSize, SizeLimit); }
  void       destroy        () { while(!m_Buffer.empty()) { xDestroyUnit(); } }
             
  xPlane<T>* borrow  (                ) { return(xPlane<T>*)xBorrow(); }
  void       giveback(xPlane<T>* Plane) { xGiveback((xPicCommon*)Plane); }

  bool       isCompatible   (xPlane<T>* Plane) { assert(Plane != nullptr); return Plane->isCompatible(m_Size, m_Margin, m_BitDepth); }
             
protected:   
  virtual void xCreateNewUnit () final;
  virtual void xDestroyUnit   () final;
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// xPlaneRental - instantiation for base types
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#ifndef PMBB_xPlane_IMPLEMENTATION
extern template class xPlaneRental<uint8 >;
extern template class xPlaneRental< int8 >;
extern template class xPlaneRental<uint16>;
extern template class xPlaneRental< int16>;
extern template class xPlaneRental<uint32>;
extern template class xPlaneRental< int32>;
extern template class xPlaneRental<uint64>;
extern template class xPlaneRental< int64>;
extern template class xPlaneRental< flt32>;
extern template class xPlaneRental< flt64>;
#endif // !PMBB_xPlane_IMPLEMENTATION

//===============================================================================================================================================================================================================

} //end of namespace PMBB
