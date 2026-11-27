/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xBfrCommon.h"
#include "xPic.h"
#include "xPixelOps.h"
#include "xMemory.h"
#include <typeinfo>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
template <typename PelType, int32 NumCmps> class xBfrPicP : public xBfrCommon
{
public:
  using tCAV = const std::array<PelType, NumCmps>&;

protected:
  PelType* m_Buffer[NumCmps] = {nullptr}; //picture buffer

public:
  //constructors $ destructors
  xBfrPicP() { };
  xBfrPicP(int32V2 Size, int32 BitDepth) { create(Size, BitDepth); }
  ~xBfrPicP() { destroy(); }

  //genral functions
  void   create (int32V2 Size, int32 BitDepth);
  void   create (const xBfrPicP*Ref) { create(Ref->getSize(), Ref->getBitDepth()); }
  void   destroy();

  void   clear  (                               ) { for(int32 c=0; c<NumCmps; c++) { std::memset((void*)m_Buffer[c], 0, getBuffNumByte()); } }
  void   copy   (const xBfrPicP* Src            ) { for(int32 c=0; c<NumCmps; c++) { copy(Src, (eCmp)c); } }
  void   copy   (const xBfrPicP* Src, eCmp CmpId) { assert(isCompatible(Src)); memcpy(m_Buffer[(int32)CmpId], Src->m_Buffer[(int32)CmpId], getBuffNumByte()); }
  void   fill   (PelType Value            ) { for(int32 c=0; c<NumCmps; c++) { fill(Value, (eCmp)c); } }
  void   fill   (PelType Value, eCmp CmpId) { xPixelOps::Fill<PelType>(m_Buffer[(int32)CmpId], Value, getBuffNumPels()); }

  //data type/range helpers       
  PelType getMaxPelValue() const { if constexpr (std::is_integral_v<PelType>) { return (PelType)xBitDepth2MaxValue(m_BitDepth);} else { return (PelType)1.0; } }
  PelType getMidPelValue() const { if constexpr (std::is_integral_v<PelType>) { return (PelType)xBitDepth2MidValue(m_BitDepth);} else { return (PelType)0.5; } }
  PelType getMinPelValue() const { return (PelType)0; }

  //parameters
  inline int32 getNumCmps() const { return NumCmps; }

  //access picture data
  inline int32          getStride(                         ) const { return m_Size.getX()         ; }
  inline int32          getPitch (                         ) const { return 1                     ; }  
  inline PelType*       getAddr  (               eCmp CmpId)       { return m_Buffer[(int32)CmpId]; }
  inline const PelType* getAddr  (               eCmp CmpId) const { return m_Buffer[(int32)CmpId]; }
  inline int32          getOffset(tPSV Position            ) const { return Position.getY() * getStride() + Position.getX(); }
  inline PelType*       getAddr  (tPSV Position, eCmp CmpId)       { return getAddr(CmpId) + getOffset(Position); }
  inline const PelType* getAddr  (tPSV Position, eCmp CmpId) const { return getAddr(CmpId) + getOffset(Position); }
  //slow pel access
  inline PelType&       accessPel(tPSV  Position, eCmp CmpId)       { return *(getAddr(CmpId) + getOffset(Position)); }
  inline const PelType& accessPel(tPSV  Position, eCmp CmpId) const { return *(getAddr(CmpId) + getOffset(Position)); }
  inline PelType&       accessPel(int32 Offset  , eCmp CmpId)       { return *(getAddr(CmpId) + Offset); }
  inline const PelType& accessPel(int32 Offset  , eCmp CmpId) const { return *(getAddr(CmpId) + Offset); }

  //low level buffer modification / access - dangerous
  inline int32          getBuffNumPels(          ) const { return (int32)m_Size.getX() * (int32)m_Size.getY(); }
  inline int32          getBuffNumByte(          ) const { return getBuffNumPels() * sizeof(PelType); }
  inline PelType*       getBuffer     (eCmp CmpId)       { return m_Buffer[(int32)CmpId]; }
  inline const PelType* getBuffer     (eCmp CmpId) const { return m_Buffer[(int32)CmpId]; }

  //block access & ops
  void fillBlock (tPSV BlockPos, tPSV BlockSize, PelType Value, eCmp CmpId)
  {
    assert(getSize()[0] >= BlockPos[0] + BlockSize[0] && getSize()[1] >= BlockPos[1] + BlockSize[1]);
    xPixelOps::Fill(getAddr(BlockPos, CmpId), Value, getStride(), BlockSize.getX(), BlockSize.getY());
  }  
  //Copy the part of bigger(or same) external buffer and fill whole local buffer
  void copyBlockFrom(tPSV BlockPos, const xBfrPicP* SrcBlock)
  { 
    assert(SrcBlock->getSize()[0] >= m_Size[0] + BlockPos[0] && SrcBlock->getSize()[1] >= m_Size[1] + BlockPos[1]);
    for(int32 c=0; c<NumCmps; c++) { xPixelOps::Copy(getAddr((eCmp)c), SrcBlock->getAddr(BlockPos, (eCmp)c), getStride(), SrcBlock->getStride(), getWidth(), getHeight()); }
  }
  //Copy whole local buffer to the part of bigger(or same) external buffer
  void copyBlockTo(tPSV BlockPos, xBfrPicP* DstBlock) const
  { 
    assert(DstBlock->getSize()[0] >= m_Size[0] + BlockPos[0] && DstBlock->getSize()[1] >= m_Size[1] + BlockPos[1]);
    for(int32 c=0; c<NumCmps; c++) { xPixelOps::Copy(DstBlock->getAddr(BlockPos, (eCmp)c), getAddr((eCmp)c), DstBlock->getStride(), getStride(), getWidth(), getHeight()); }
  }
  //Copy the part of external picture and fill whole local buffer
  void copyBlockFrom(tPSV BlockPos, const xPicP* SrcPic)
  {
    if constexpr(std::is_same_v<PelType, uint16>)
    {
      for(int32 c = 0; c < NumCmps; c++) { xPixelOps::Copy(getAddr((eCmp)c), SrcPic->getAddr(BlockPos, (eCmp)c), getStride(), SrcPic->getStride(), getWidth(), getHeight()); }
    }
  }
  //Copy whole local buffer to the part of bigger(or same) external buffer
  void copyBlockTo(tPSV BlockPos, xPicP* DstPic) const
  {
    if constexpr(std::is_same_v<PelType, uint16>)
    {
      for(int32 c = 0; c < NumCmps; c++) { xPixelOps::Copy(DstPic->getAddr(BlockPos, (eCmp)c), getAddr((eCmp)c), DstPic->getStride(), getStride(), getWidth(), getHeight()); }
    }
  }


  //visualization
  void drawUnit    (tPSV BlockPos, tPSV BlockSize, tCAV Fill, tCAV Edge);
  void drawSelected(tPSV BlockPos, tPSV BlockSize, const uint8* Sel, int32 SelStride, uint8 SelVal, tCAV Fill, tCAV Edge);
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// xBfrPicP - implementation
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <typename PelType, int32 NumCmps> void xBfrPicP<PelType, NumCmps>::create(int32V2 Size, int32 BitDepth)
{
  if constexpr(std::is_integral_v<PelType>) { assert(BitDepth!=0); }
  assert(Size.getX() <= (int32)std::numeric_limits<int16>::max());
  assert(Size.getY() <= (int32)std::numeric_limits<int16>::max());
  assert(BitDepth    <= (int32)std::numeric_limits<int8 >::max());

  m_Size     = (int16V2)Size;
  m_BitDepth = (int8   )BitDepth;

  const int32 NumnBytesPerCmp = getBuffNumByte();
  for(int32 c = 0; c < NumCmps; c++)
  { 
    m_Buffer[c] = (PelType*)xMemory::xAlignedMallocAuto(NumnBytesPerCmp);
  }
}
template <typename PelType, int32 NumCmps> void xBfrPicP<PelType, NumCmps>::destroy()
{
  for(int32 c = 0; c < NumCmps; c++) { if(m_Buffer[c] != nullptr) { xMemory::xAlignedFreeNull(m_Buffer[c]); } }
  m_Size     = { NOT_VALID, NOT_VALID };
  m_BitDepth = NOT_VALID;
}
template <typename PelType, int32 NumCmps> void xBfrPicP<PelType, NumCmps>::drawUnit(tPSV BlockPos, tPSV BlockSize, tCAV Fill, tCAV Edge)
{
  assert(getSize()[0] >= BlockPos[0] + BlockSize[0] && getSize()[1] >= BlockPos[1] + BlockSize[1]);
  for(int32 c = 0; c < NumCmps; c++)
  {
    PelType* restrict Dst       = getAddr  (BlockPos, (eCmp)c);
    const int32       DstStride = getStride();
    const int32       Width     = BlockSize.getX();
    const int32       Height    = BlockSize.getY();
    
    if(Width > 2 && Height > 2) { xPixelOps::Fill(Dst, Fill[c], DstStride, Width, Height); }
    
    const PelType E = Edge[c];

    for(int32 x = 0; x < Width; x++) { Dst[x] = E; }
    if(Height == 1) { continue; }
    Dst += DstStride;

    for(int32 y = 1; y < Height - 1; y++)
    {
      Dst[0        ] = E;
      Dst[Width - 1] = E;
      Dst += DstStride;
    }
    for(int32 x = 0; x < Width; x++) { Dst[x] = E; }
  }
}
template <typename PelType, int32 NumCmps> void xBfrPicP<PelType, NumCmps>::drawSelected(tPSV BlockPos, tPSV BlockSize, const uint8* Sel, int32 SelStride, uint8 SelVal, tCAV Fill, tCAV Edge)
{
  for (int32 c = 0; c < NumCmps; c++)
  {
    PelType* restrict Dst       = getAddr  (BlockPos, (eCmp)c);
    const int32       DstStride = getStride();
    const uint8*      Src       = Sel;
    const int32       Width     = BlockSize.getX();
    const int32       Height    = BlockSize.getY();

    for (int32 y = 0; y < Height; y++)
    {
      for (int32 x = 0; x < Width; x++) 
      {
        if (Src[x] == SelVal)
        {
          Dst[x] = Fill[c];
          if (x == 0 || y == 0 || x == Width - 1 || y == Height - 1) Dst[x] = Edge[c]; //TODO TOCHECK go by rows, find x0 and x1, fill
        }
      }
      Dst += DstStride;
      Src += SelStride;
    }
  }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// xBfrPicP - instantiation for base types
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#ifndef PMBB_xBfrPic_IMPLEMENTATION
extern template class xBfrPicP<uint8 , 3>;
extern template class xBfrPicP< int8 , 3>;
extern template class xBfrPicP<uint16, 3>;
extern template class xBfrPicP< int16, 3>;
extern template class xBfrPicP<uint32, 3>;
extern template class xBfrPicP< int32, 3>;
extern template class xBfrPicP<uint64, 3>;
extern template class xBfrPicP< int64, 3>;
extern template class xBfrPicP< flt32, 3>;
extern template class xBfrPicP< flt64, 3>;
#endif // !PMBB_xBfrPic_IMPLEMENTATION


//===============================================================================================================================================================================================================

} //end of namespace PMBB
