/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define PMBB_xPlane_IMPLEMENTATION
#include "xPlane.h"
#include "xPixelOps.h"
#include "xMemory.h"
#include <typeinfo>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xPlane
//===============================================================================================================================================================================================================

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// xPlane - general functions
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <typename PelType> void xPlane<PelType>::create(int32V2 Size, int32 BitDepth, int32 Margin)
{
  if constexpr(std::is_integral_v<PelType>) { assert(BitDepth!=0); }
  xInit(Size, BitDepth, Margin, 1, sizeof(PelType));
  
  m_Buffer = (PelType*)xMemory::xAlignedMallocPageAuto(m_BuffCmpNumBytes);
  m_Origin = m_Buffer + Margin*m_Stride + Margin;
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
template <typename PelType> void xPlane<PelType>::fill(PelType Value)
{
  xPixelOps::Fill<PelType>(m_Buffer, Value, m_BuffCmpNumPels);
  m_IsMarginExtended = true;
}
template <typename PelType> bool xPlane<PelType>::check(const std::string& Name) const
{
  if constexpr(std::is_same_v<PelType, uint16>)
  {
    bool Correct = xPixelOps::CheckIfInRange(m_Origin, m_Stride, m_Width, m_Height, m_BitDepth);
    if(!Correct)
    {
      fmt::print("FILE BROKEN " + Name + "\n");
      std::string Msg = xPixelOps::FindOutOfRange(m_Origin, m_Stride, m_Width, m_Height, m_BitDepth, -1);
      fmt::print(Msg);
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
template <typename PelType> void xPlane<PelType>::extend()
{
  if constexpr(std::is_same_v<PelType, uint16>)
  {
    xPixelOps::ExtendMargin(m_Origin, m_Stride, m_Width, m_Height, m_Margin);
  }
  else
  {
    assert(0); 
  }
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
      fmt::print(Msg);
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
// instantiation for base types
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template class xPlane<uint8 >;
template class xPlane< int8 >;
template class xPlane<uint16>;
template class xPlane< int16>;
template class xPlane<uint32>;
template class xPlane< int32>;
template class xPlane<uint64>;
template class xPlane< int64>;
template class xPlane< flt32>;
template class xPlane< flt64>;

//===============================================================================================================================================================================================================
// xPlaneRental
//===============================================================================================================================================================================================================

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
  Tmp->create(m_Size, m_Margin, m_BitDepth);
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
// instantiation for base types
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template class xPlaneRental<uint8 >;
template class xPlaneRental< int8 >;
template class xPlaneRental<uint16>;
template class xPlaneRental< int16>;
template class xPlaneRental<uint32>;
template class xPlaneRental< int32>;
template class xPlaneRental<uint64>;
template class xPlaneRental< int64>;
template class xPlaneRental< flt32>;
template class xPlaneRental< flt64>;

//===============================================================================================================================================================================================================

} //end of namespace PMBB
