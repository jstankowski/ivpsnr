/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefCORE.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include "xMemory.h"

namespace PMBB_NAMESPACE {

//=============================================================================================================================================================================
//xRing - thread safe FIFO with ring buffer
//=============================================================================================================================================================================
template <class XXX> class xPtrRing
{
protected:
  XXX**  m_Ring     = nullptr  ; //ring data  
  int32  m_RingSize = NOT_VALID; //ring size
  int32  m_DataCnt  = NOT_VALID; //number of units in ring
  int32  m_WriteId  = NOT_VALID; //writting address 
  int32  m_ReadId   = NOT_VALID; //reading address

  //threading utils
  std::mutex              m_Mutex;
  std::condition_variable m_InsertConditionVariable;
  std::condition_variable m_RemoveConditionVariable;

public:
  xPtrRing (              ) {};
  xPtrRing (int32 RingSize) { create(RingSize, true); }
  ~xPtrRing(              ) { destroy(); }

  void   create    (int32 RingSize, bool FillZero);
  void   destroy   ();
  int32  getSize   () const { return m_RingSize; }

  void   insertWait(XXX* Data);
  void   insertWait(XXX** Data, int32 NumProvided);
  XXX*   removeWait();
  void   removeWait(XXX** Data, int32 NumExpected);
  bool   isEmpty   () const { return m_DataCnt == 0         ; }
  bool   isFull    () const { return m_DataCnt == m_RingSize; }
  int32  getLoad   () const { return m_DataCnt              ; }
};  

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template <class XXX> void xPtrRing<XXX>::create(int32 RingSize, bool FillZero)
{
  assert(RingSize>0);

  m_Ring     = (XXX**)xMemory::xAlignedMallocPageAuto(RingSize*sizeof(XXX*));
  m_RingSize = RingSize;
  m_DataCnt  = 0;
  m_WriteId  = 0;
  m_ReadId   = 0;

  if(FillZero) { memset(m_Ring, 0, RingSize * sizeof(XXX*)); }
}
template <class XXX> void xPtrRing<XXX>::destroy()
{
  if(m_Ring != nullptr) { xMemory::xAlignedFreeNull(m_Ring); }
}
template <class XXX> void xPtrRing<XXX>::insertWait(XXX* NodeData)
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_DataCnt>=m_RingSize) { m_InsertConditionVariable.wait(LockManager, [&]{ return m_DataCnt<m_RingSize;} ); }
  m_Ring[m_WriteId] = NodeData;
  m_WriteId         = (m_WriteId+1)%m_RingSize;
  m_DataCnt++;
  LockManager.unlock();
  m_RemoveConditionVariable.notify_all();
}
template <class XXX> void xPtrRing<XXX>::insertWait(XXX** Data, int32 NumProvided)
{
  int32 NumEnqueued = 0;
  while(NumEnqueued < NumProvided)
  {
    std::unique_lock<std::mutex> LockManager(m_Mutex);
    while(m_DataCnt >= m_RingSize) { m_InsertConditionVariable.wait(LockManager, [&] { return m_DataCnt < m_RingSize; }); }

    int32 NumSlotsAvailable = m_RingSize - m_DataCnt;
    int32 NumToEnqueue      = xMin(NumSlotsAvailable, NumProvided - NumEnqueued);

    if(m_DataCnt == 0)
    {
      memcpy(m_Ring, Data, NumProvided * sizeof(XXX*));
      m_DataCnt = NumProvided;
      m_WriteId = NumProvided;
      m_ReadId  = 0;      
    }
    else
    {
      for(int32 i = 0; i < NumToEnqueue; i++)
      {
        m_Ring[m_WriteId] = Data[NumEnqueued + i];
        m_WriteId         = (m_WriteId + 1) % m_RingSize;
        m_DataCnt++;
      }
    }
    NumEnqueued += NumToEnqueue;
    LockManager.unlock();
    m_RemoveConditionVariable.notify_all();
  }
}
template <class XXX> XXX* xPtrRing<XXX>::removeWait()
{
  XXX* NodeData = nullptr;
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_DataCnt<=0) { m_RemoveConditionVariable.wait(LockManager, [&]{ return m_DataCnt>0;} ); }
  NodeData = m_Ring[m_ReadId];
  m_ReadId = (m_ReadId+1)%m_RingSize;
  m_DataCnt--;
  LockManager.unlock();
  m_InsertConditionVariable.notify_all();
  return NodeData;
}
template <class XXX> void xPtrRing<XXX>::removeWait(XXX** Data, int32 NumExpected)
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_DataCnt < NumExpected) { m_RemoveConditionVariable.wait(LockManager, [&]{ return m_DataCnt >= NumExpected;} ); }
  if(m_ReadId == 0 && m_DataCnt == NumExpected)
  {
    memcpy(Data, m_Ring, NumExpected * sizeof(XXX*));
    m_DataCnt = 0;
    m_WriteId = 0;
    m_ReadId  = 0;
  }
  else
  {
    for(int32 i = 0; i < NumExpected; i++)
    {
      Data[i]  = m_Ring[m_ReadId];
      m_ReadId = (m_ReadId + 1) % m_RingSize;
      m_DataCnt--;
    }
  }
  LockManager.unlock();
  m_InsertConditionVariable.notify_all();
}

//=============================================================================================================================================================================

} //end of namespace PMBB
