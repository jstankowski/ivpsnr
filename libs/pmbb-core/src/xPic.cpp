/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xPic.h"
#include "xPixelOps.h"
#include "xMemory.h"
#include <cassert>
#include <cstring>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xPicP 
//===============================================================================================================================================================================================================
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// xPicP - general functions
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void xPicP::create(int32V2 Size, int32 BitDepth, int32 Margin)
{
  xInit(Size, BitDepth, Margin, c_DefNumCmps, sizeof(uint16));

  for(int32 c = 0; c < m_NumCmps; c++)
  {
    m_Buffer[c] = (uint16*)xMemory::xAlignedMallocPageAuto(m_BuffCmpNumBytes);
    m_Origin[c] = m_Buffer[c] + (m_Margin * m_Stride) + m_Margin;
  }  
}
void xPicP::destroy()
{
  for(int32 c = 0; c < m_NumCmps; c++)
  {
    if(m_Buffer[c] != nullptr) { xMemory::xAlignedFree(m_Buffer[c]); m_Buffer[c] = nullptr; }
    m_Origin[c] = nullptr;
  }
  xUnInit();
}
void xPicP::clear()
{
  for(int32 c=0; c < m_NumCmps; c++) { memset(m_Buffer[c], 0, m_BuffCmpNumBytes); }
  m_POC              = NOT_VALID;
  m_Timestamp        = NOT_VALID;
  m_IsMarginExtended = false;
}
void xPicP::copy(const xPicP* Src)
{
  assert(Src!=nullptr && isCompatible(Src));
  for(int32 c=0; c < m_NumCmps; c++) { memcpy(m_Buffer[c], Src->m_Buffer[c], m_BuffCmpNumBytes); }
  m_IsMarginExtended = Src->m_IsMarginExtended;
}
void xPicP::fill(uint16 Value)
{
  for(int32 c = 0; c < m_NumCmps; c++) { fill(Value, (eCmp)c); }
  m_IsMarginExtended = true;
}
void xPicP::fill(uint16 Value, eCmp CmpId)
{ 
  xPixelOps::Fill(m_Buffer[(int32)CmpId], Value, m_BuffCmpNumPels);
}
bool xPicP::check(const std::string& Name)
{
  boolV4 Correct = xMakeVec4(true);
  for(int32 CmpIdx = 0; CmpIdx < m_NumCmps; CmpIdx++)
  { 
    Correct[CmpIdx] = xPixelOps::CheckValues(m_Origin[CmpIdx], m_Stride, m_Width, m_Height, m_BitDepth);
  }

  for(int32 CmpIdx = 0; CmpIdx < m_NumCmps; CmpIdx++)
  {
    if(!Correct[CmpIdx])
    {
      fmt::printf("FILE BROKEN " + Name + " (CMP=%d)\n", CmpIdx);
      xPixelOps::FindBroken(m_Origin[CmpIdx], m_Stride, m_Width, m_Height, m_BitDepth);
      return false;
    }
  }

  return true;
}
void xPicP::conceal()
{
  for(int32 CmpIdx = 0; CmpIdx < m_NumCmps; CmpIdx++)
  {
    xPixelOps::ConcealBroken(m_Origin[CmpIdx], m_Stride, m_Width, m_Height, m_BitDepth);
  }
}
void xPicP::extend()
{
  for(int32 CmpIdx = 0; CmpIdx < m_NumCmps; CmpIdx++) { xPixelOps::ExtendMargin(m_Origin[CmpIdx], m_Stride, m_Width, m_Height, m_Margin); }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//low level buffer modification / access - dangerous
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool xPicP::bindBuffer(uint16* Buffer, eCmp CmpId)
{
  assert(Buffer!=nullptr); if(m_Buffer[(int32)CmpId]) { return false; }
  m_Buffer[(int32)CmpId] = Buffer;
  m_Origin[(int32)CmpId] = m_Buffer[(int32)CmpId] + (m_Margin * m_Stride) + m_Margin;
  return true;
}
uint16* xPicP::unbindBuffer(eCmp CmpId)
{
  if(m_Buffer[(int32)CmpId]==nullptr) { return nullptr; }
  uint16* Tmp = m_Buffer[(int32)CmpId];
  m_Buffer[(int32)CmpId] = nullptr;
  m_Origin[(int32)CmpId] = nullptr;
  return Tmp;
}
bool xPicP::swapBuffer(uint16*& Buffer, eCmp CmpId)
{
  assert(Buffer!=nullptr); if(m_Buffer[(int32)CmpId]==nullptr) { return false; }
  std::swap(m_Buffer[(int32)CmpId], Buffer);
  m_Origin[(int32)CmpId] = m_Buffer[(int32)CmpId] + (m_Margin * m_Stride) + m_Margin;
  return true;
}
bool xPicP::swapBuffer(xPicP* TheOther, eCmp CmpId)
{
  assert(TheOther != nullptr); if(TheOther==nullptr || !isCompatible(TheOther)) { return false; }
  std::swap(this->m_Buffer[(int32)CmpId], TheOther->m_Buffer[(int32)CmpId]);
  this    ->m_Origin[(int32)CmpId] = this    ->m_Buffer[(int32)CmpId] + (this    ->m_Margin * this    ->m_Stride) + this    ->m_Margin;
  TheOther->m_Origin[(int32)CmpId] = TheOther->m_Buffer[(int32)CmpId] + (TheOther->m_Margin * TheOther->m_Stride) + TheOther->m_Margin;
  return true;
}
bool xPicP::swapBuffers(xPicP* TheOther)
{
  for(int32 c = 0; c < m_NumCmps; c++)
  { 
    bool Result = swapBuffer(TheOther, (eCmp)c);
    if(!Result) { return false; }
  }
  return true;
}


//===============================================================================================================================================================================================================
// xPicI
//===============================================================================================================================================================================================================
void xPicI::create(int32V2 Size, int32 BitDepth, int32 Margin)
{
  xInit(Size, BitDepth, Margin, c_DefNumCmps, sizeof(uint16));

  m_Buffer = (uint16*)xMemory::xAlignedMallocPageAuto(m_BuffCmpNumBytes * c_MaxNumCmps);
  m_Origin = m_Buffer + (m_Margin * (m_Stride << 2)) + (m_Margin << 2);
}
void xPicI::destroy()
{
  xMemory::xAlignedFree(m_Buffer); m_Buffer = nullptr;
  m_Origin = nullptr;

  xUnInit();
}
void xPicI::clear()
{
  memset(m_Buffer, 0, m_BuffCmpNumBytes * c_MaxNumCmps);
  m_POC              = NOT_VALID;
  m_Timestamp        = NOT_VALID;
  m_IsMarginExtended = false;
}
void xPicI::copy(const xPicI* Src)
{
  assert(Src != nullptr && isCompatible(Src));
  memcpy(m_Buffer, Src->m_Buffer, m_BuffCmpNumBytes * c_MaxNumCmps);
  m_IsMarginExtended = Src->m_IsMarginExtended;
}
void xPicI::fill(uint16 Value)
{
  xPixelOps::Fill<uint16>(m_Buffer, Value, m_BuffCmpNumPels * c_MaxNumCmps);
  m_IsMarginExtended = true;
}
void xPicI::rearrangeFromPlanar(const xPicP* Planar)
{
  assert(isCompatible(Planar));
  const int32 ExtWidth  = m_Width  + (m_Margin << 1);
  const int32 ExtHeight = m_Height + (m_Margin << 1);
  xPixelOps::AOS4fromSOA3(m_Buffer, Planar->getBuffer(eCmp::C0), Planar->getBuffer(eCmp::C1), Planar->getBuffer(eCmp::C2), 0, m_Stride * c_MaxNumCmps, Planar->getStride(), ExtWidth, ExtHeight);
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
