/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefCORE.h"
#include "xPicCommon.h"
#include "xMemory.h"
#include "xErrMsg.h"
#include "xPixelOps.h"
#include "xMarginOps.h"
#include <type_traits>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xPlane
//===============================================================================================================================================================================================================
template <typename PelType> class xPlane : public xPicCommon
{
public:
  using T = PelType;

protected:
  T* m_Buffer = nullptr; //picture buffer
  T* m_Origin = nullptr; //pel origin, pel access -> m_PelOrg[y*m_PelStride + x]

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
  void   zero   (bool IncludeMargin = true);
  void   fill   (PelType Value, bool IncludeMargin = true);
  bool   check  (const std::string& Name) const;  
  void   conceal();
  void   extend (eMrgExt MarginExtendMode = eMrgExt::Nearest);

  bool   equal  (const xPlane* Ref, bool PrintFirstDiscrepancy) const;
                                   
  //data type/range helpers       
  T getMaxPelValue() const { if constexpr (std::is_integral_v<PelType>) { return (PelType)xBitDepth2MaxValue(m_BitDepth);} else { return (PelType)1.0; } }
  T getMidPelValue() const { if constexpr (std::is_integral_v<PelType>) { return (PelType)xBitDepth2MidValue(m_BitDepth);} else { return (PelType)0.5; } }
  T getMinPelValue() const { return (PelType)0; }

  //access picture data
  inline int32    getStride (             ) const { return m_Stride; }
  inline int32    getPitch  (             ) const { return 1       ; }  
  inline T*       getAddr   (             )       { return m_Origin; } 
  inline const T* getAddr   (             ) const { return m_Origin; }
  inline int32    getOffset (int32V2 PosXY) const { return PosXY.getY() * m_Stride + PosXY.getX(); }
  inline T*       getAddr   (int32V2 PosXY)       { return m_Origin + getOffset(PosXY); }
  inline const T* getAddr   (int32V2 PosXY) const { return m_Origin + getOffset(PosXY); }
  inline T*       getRowAddr(int32   y    )       { return m_Origin + y * m_Stride; }
  inline const T* getRowAddr(int32   y    ) const { return m_Origin + y * m_Stride; }
  //slow pel access
  inline T&       accessPel(int32V2 PosXY )       { return *(getAddr() + getOffset(PosXY)); }
  inline const T& accessPel(int32V2 PosXY ) const { return *(getAddr() + getOffset(PosXY)); }
  inline T&       accessPel(int32   Offset)       { return *(getAddr() + Offset); }
  inline const T& accessPel(int32   Offset) const { return *(getAddr() + Offset); }

  //low level buffer modification / access - dangerous
  inline int64    getBuffNumPels() const { return m_BuffCmpNumPels ; }
  inline int64    getBuffSize   () const { return m_BuffCmpNumBytes; }
  inline T*       getBuffer     ()       { return m_Buffer; }
  inline const T* getBuffer     () const { return m_Buffer; }
         bool     bindBuffer    (T*  Buffer      );
         T*       unbindBuffer  (                );
         bool     swapBuffer    (T*& Buffer      );
         bool     swapBuffer    (xPlane* TheOther);              

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
// xPlane - general functions
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <typename PelType> void xPlane<PelType>::create(int32V2 Size, int32 BitDepth, int32 Margin)
{
  if constexpr(std::is_integral_v<PelType>) { assert(BitDepth!=0); }
  xInit(Size, BitDepth, Margin, 1, sizeof(PelType));
  
  m_Buffer = (PelType*)xMemory::xAlignedMallocPageAuto(m_BuffCmpNumBytes);
  m_Origin = m_Buffer + Margin*m_Stride + Margin;
  if(m_Buffer == nullptr) { xErrMsg::printError(fmt::format("TERRIBLE ERROR --> memory allocation failed in xPlane<PelType>::create while using xMemory::xAlignedMallocPageAuto({})", m_BuffCmpNumBytes)); abort(); }
}
template <typename PelType> void xPlane<PelType>::destroy()
{
  if(m_Buffer != nullptr) { xMemory::xAlignedFree(m_Buffer); m_Buffer = nullptr; }
  m_Origin = nullptr;
  xUnInit();
}
template <typename PelType> void xPlane<PelType>::clear()
{
  std::memset((void*)m_Buffer, 0, m_BuffCmpNumBytes);
  m_POC              = NOT_VALID;  
  m_Timestamp        = NOT_VALID;  
  m_IsMarginExtended = false;
}
template <typename PelType> void xPlane<PelType>::copy(const xPlane* Src)
{
  assert(Src!=nullptr && isCompatible(Src));
  memcpy(m_Buffer, Src->m_Buffer, m_BuffCmpNumBytes);
  m_POC              = Src->m_POC             ;
  m_Timestamp        = Src->m_Timestamp       ;
  m_IsMarginExtended = Src->m_IsMarginExtended;
}
template <typename PelType> void xPlane<PelType>::zero(bool IncludeMargin)
{
  if(IncludeMargin) { std::memset((void*)m_Buffer, 0, m_BuffCmpNumBytes)                ; }
  else              { xPixelOps::Fill<PelType>(m_Origin, 0, m_Stride, m_Width, m_Height); }
  m_IsMarginExtended = IncludeMargin;
}
template <typename PelType> void xPlane<PelType>::fill(PelType Value, bool IncludeMargin)
{
  if(IncludeMargin) { xPixelOps::Fill<PelType>(m_Buffer, Value, m_BuffCmpNumPels)           ; }
  else              { xPixelOps::Fill<PelType>(m_Origin, Value, m_Stride, m_Width, m_Height); }
  m_IsMarginExtended = IncludeMargin;
}
template <typename PelType> bool xPlane<PelType>::check(const std::string& Name) const
{
  if constexpr(std::is_same_v<PelType, uint16>)
  {
    bool Correct = xPixelOps::CheckIfInRange(m_Origin, m_Stride, m_Width, m_Height, m_BitDepth);
    if(!Correct)
    {
      fmt::print("FILE BROKEN {}\n", Name);
      std::string Msg = xPixelOps::FindOutOfRange(m_Origin, m_Stride, m_Width, m_Height, m_BitDepth, -1);
      fmt::print("{}", Msg);
    }
    return Correct;
  }
  else
  {
    assert(0); return false;
  }
}
template <typename PelType> void xPlane<PelType>::conceal()
{
  if constexpr(std::is_same_v<PelType, uint16>)
  {
    xPixelOps::ClipToRange(m_Origin, m_Stride, m_Width, m_Height, m_BitDepth);
    m_IsMarginExtended = false;
  }
  else
  {
    assert(0);
  }
}
template <typename PelType> void xPlane<PelType>::extend(eMrgExt MarginExtendMode)
{
  xMarginOps::ExtendMargin(m_Origin, m_Stride, m_Width, m_Height, m_Margin, std::numeric_limits< PelType>::max(), MarginExtendMode);
}
template <typename PelType> bool xPlane<PelType>::equal(const xPlane* Ref, bool PrintFirstDiscrepancy) const
{
  assert(Ref != nullptr && isCompatible(Ref));
  if constexpr(std::is_same_v<PelType, uint16>)
  {
    bool Equal = xPixelOps::CompareEqual(Ref->getAddr(), m_Origin, Ref->getStride(), m_Stride, m_Width, m_Height);
    if(PrintFirstDiscrepancy && !Equal)
    {
      fmt::print("ERROR current plane values differs from reference\n");
      std::string Msg = xPixelOps::FindOutOfRange(m_Origin, m_Stride, m_Width, m_Height, m_BitDepth, 1);
      fmt::print("{}", Msg);
    }
    return Equal;
  }
  else
  {
    assert(0);
    return false;
  }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// xPlane - low level buffer modification / access - dangerous
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <typename PelType> bool xPlane<PelType>::bindBuffer(PelType* Buffer)
{
  assert(Buffer!=nullptr); if(m_Buffer) { return false; }
  m_Buffer = Buffer;
  m_Origin = m_Buffer + (m_Margin * m_Stride) + m_Margin;
  return true;
}
template <typename PelType> PelType* xPlane<PelType>::unbindBuffer()
{
  if(m_Buffer==nullptr) { return nullptr; }
  PelType* Tmp = m_Buffer;
  m_Buffer = nullptr;
  m_Origin = nullptr;
  return Tmp;
}
template <typename PelType> bool xPlane<PelType>::swapBuffer(PelType*& Buffer)
{
  assert(Buffer != nullptr); if(m_Buffer==nullptr) { return false; }
  std::swap(m_Buffer, Buffer);
  m_Origin = m_Buffer + (m_Margin * m_Stride) + m_Margin;
  return true;
}
template <typename PelType> bool xPlane<PelType>::swapBuffer(xPlane* TheOther)
{
  assert(TheOther != nullptr); if(TheOther==nullptr || !isCompatible(TheOther)) { return false; }
  std::swap(this->m_Buffer, TheOther->m_Buffer);
  this    ->m_Origin = this    ->m_Buffer + (this    ->m_Margin * this    ->m_Stride) + this    ->m_Margin;
  TheOther->m_Origin = TheOther->m_Buffer + (TheOther->m_Margin * TheOther->m_Stride) + TheOther->m_Margin;
  return true;
}

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
  int32      m_BitDepth;
  int32      m_Margin;  

public:
  void       create  (int32V2 Size, int32 BitDepth,int32 Margin, uintSize InitSize = 0, uintSize SizeLimit = std::numeric_limits<uintSize>::max());
  void       recreate(int32V2 Size, int32 BitDepth,int32 Margin, uintSize InitSize = 0, uintSize SizeLimit = std::numeric_limits<uintSize>::max()) { destroy(); create(Size, BitDepth, Margin, InitSize, SizeLimit); }
  void       destroy () { while(!m_Buffer.empty()) { xDestroyUnit(); } }
             
  xPlane<T>* borrow  (                ) { return(xPlane<T>*)xBorrow(); }
  void       giveback(xPlane<T>* Plane) { xGiveback((xPicCommon*)Plane); }

  bool       isCompatible   (xPlane<T>* Plane) { assert(Plane != nullptr); return Plane->isCompatible(m_Size, m_Margin, m_BitDepth); }
             
protected:   
  virtual void xCreateNewUnit () final;
  virtual void xDestroyUnit   () final;
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template <typename PelType> void xPlaneRental<PelType>::create(int32V2 Size, int32 Margin, int32 BitDepth, uintSize InitSize, uintSize SizeLimit)
{
  m_Mutex.lock();
  m_Size         = Size;
  m_Margin       = Margin;
  m_BitDepth     = BitDepth;
  m_CreatedUnits = 0;
  m_SizeLimit    = SizeLimit;

  for(uintSize i=0; i<InitSize; i++) { xCreateNewUnit(); }
  m_Mutex.unlock();
}
template <typename PelType> void xPlaneRental<PelType>::xCreateNewUnit()
{
  xPlane<PelType>* Tmp = new xPlane<PelType>;
  Tmp->create(m_Size, m_BitDepth, m_Margin);
  m_Buffer.push_back(Tmp);
  m_CreatedUnits++;
}
template <typename PelType> void xPlaneRental<PelType>::xDestroyUnit()
{
  if(!m_Buffer.empty())
  {
    xPlane<PelType>* Tmp = (xPlane<PelType>*)m_Buffer.back();
    m_Buffer.pop_back();
    Tmp->destroy();
    delete Tmp;
  }
}

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
