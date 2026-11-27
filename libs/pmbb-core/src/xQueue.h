/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefCORE.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <queue>

//clanup max min crap
#ifdef X_PMBB_OPERATING_SYSTEM_WINDOWS
  #ifdef max
    #undef max
  #endif
  #ifdef min
    #undef min
  #endif
#endif //X_PMBB_OPERATING_SYSTEM_WINDOWS

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xQueue - thread safe std::queue (FIFO) for any type of data
//===============================================================================================================================================================================================================
template <class XXX> class xQueue
{
protected:
  std::queue<XXX> m_Queue;
  uint32          m_Capacity;

  //threading utils
  std::mutex              m_Mutex;
  std::condition_variable m_EnqueueConditionVariable;
  std::condition_variable m_DequeueConditionVariable;
  
public:
  xQueue(int32 Capacity = 1) { setCapacity(Capacity); }

  void     setCapacity(int32 Capacity)       { assert(Capacity > 0); std::lock_guard<std::mutex> LockManager(m_Mutex); m_Capacity = Capacity; m_EnqueueConditionVariable.notify_all(); }
  int32    getCapacity(              ) const { return m_Capacity; }

  bool     isEmpty() const { return m_Queue.empty(); }
  bool     isFull () const { return m_Queue.size () == m_Capacity; }
  uintSize getLoad() const { return m_Queue.size (); }

  void EnqueueResize(XXX  Data);
  bool DequeueTry   (XXX& Data);

  void EnqueueWait   (XXX  Data);
  void EnqueueWait   (const XXX* Data, int32 Num);
  void DequeueWait   (XXX& Data);
  void DequeueWait   (XXX* Data, int32 Num);

  template<class Rep, class Period> bool EnqueueWaitFor(XXX  Data, const std::chrono::duration<Rep, Period>& Duration);
  template<class Rep, class Period> bool DequeueWaitFor(XXX& Data, const std::chrono::duration<Rep, Period>& Duration);
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<class XXX> void xQueue<XXX>::EnqueueResize(XXX Data)
{
  m_Mutex.lock();
  if(isFull()) { m_Capacity++; }
  m_Queue.push(Data);
  m_Mutex.unlock();
  m_DequeueConditionVariable.notify_one();
}
template<class XXX> bool xQueue<XXX>::DequeueTry(XXX& Data)
{
  m_Mutex.lock(); 
  if(!m_Queue.empty())
  {
    Data = m_Queue.front();
    m_Queue.pop();
    m_Mutex.unlock();
    m_EnqueueConditionVariable.notify_one();
    return true;
  }
  else
  {
    m_Mutex.unlock();
    return false;
  }
}
template<class XXX> void xQueue<XXX>::EnqueueWait(XXX Data)
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_Queue.size()>= m_Capacity) { m_EnqueueConditionVariable.wait(LockManager, [&]{ return m_Queue.size()< m_Capacity; } ); }
  m_Queue.push(Data);
  LockManager.unlock();
  m_DequeueConditionVariable.notify_one();
  //release lock - std::unique_lock destructor... 
}
template<class XXX> void xQueue<XXX>::EnqueueWait(const XXX* Data, int32 NumProvided)
{
  int32 NumEnqueued = 0;
  while(NumEnqueued < NumProvided)
  {
    std::unique_lock<std::mutex> LockManager(m_Mutex);
    while(m_Queue.size() >= m_Capacity) { m_EnqueueConditionVariable.wait(LockManager, [&] { return m_Queue.size() < m_Capacity; }); }
    int32 NumSlotsAvailable = m_Capacity - (int32)(m_Queue.size());
    int32 NumToEnqueue      = xMin(NumSlotsAvailable, NumProvided - NumEnqueued);
    for(int32 i = 0; i < NumToEnqueue; i++)
    {
      m_Queue.push(Data[NumEnqueued + i]);
    }
    NumEnqueued += NumToEnqueue;
    LockManager.unlock();
    m_DequeueConditionVariable.notify_one();
  }
}
template<class XXX> void xQueue<XXX>::DequeueWait(XXX& Data)
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_Queue.empty()) { m_DequeueConditionVariable.wait(LockManager, [&]{ return !m_Queue.empty(); } ); }
  Data = m_Queue.front();
  m_Queue.pop();
  LockManager.unlock();
  m_EnqueueConditionVariable.notify_one();
}
template<class XXX> void xQueue<XXX>::DequeueWait(XXX* Data, int32 NumExpected)
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_Queue.size() < NumExpected) { m_DequeueConditionVariable.wait(LockManager, [&] { return m_Queue.size() >= NumExpected; }); }
  for(int32 i = 0; i < NumExpected; i++)
  {
    Data[i] = m_Queue.front(); m_Queue.pop();
  }
  LockManager.unlock();
  m_EnqueueConditionVariable.notify_one();
}
template<class XXX> template<class Rep, class Period> bool xQueue<XXX>::EnqueueWaitFor(XXX Data, const std::chrono::duration<Rep, Period>& Duration)
{
  if(Duration == std::chrono::duration<Rep, Period>::max()) { EnqueueWait(Data); return true; } //fix for some broken implementations (i.e. MSVC 14)

  std::unique_lock<std::mutex> LockManager(m_Mutex);
  std::cv_status Status = std::cv_status::no_timeout;
  if(m_Queue.size()>= m_Capacity) { Status = m_EnqueueConditionVariable.wait_for(LockManager, Duration); }
  if(Status==std::cv_status::no_timeout && m_Queue.size()< m_Capacity)
  {
    m_Queue.push(Data);
    LockManager.unlock();
    m_DequeueConditionVariable.notify_one();
    return true;
  }
  else { return false; }    
  //release lock - std::unique_lock destructor... 
}
template<class XXX> template<class Rep, class Period> bool xQueue<XXX>::DequeueWaitFor(XXX& Data, const std::chrono::duration<Rep, Period>& Duration)
{
  if(Duration == std::chrono::duration<Rep, Period>::max()) { DequeueWait(Data); return true; } //fir for some broken implementations (i.e. MSVC 14)

  std::unique_lock<std::mutex> LockManager(m_Mutex);
  std::cv_status Status = std::cv_status::no_timeout;
  if(m_Queue.empty()) { Status = m_DequeueConditionVariable.wait_for(LockManager, Duration); }
  if(Status==std::cv_status::no_timeout && !m_Queue.empty())
  {
    Data = m_Queue.front();
    m_Queue.pop();
    LockManager.unlock();
    m_EnqueueConditionVariable.notify_one();
    return true;
  }
  else { return false; }
  //release lock - std::unique_lock destructor... 
}

//===============================================================================================================================================================================================================
//xPriorityQueue - thread safe std::priority_queue for any type of data
//===============================================================================================================================================================================================================
template<class XXX> class xPriorityQueue
{
  using BaseType = typename std::remove_pointer<XXX>::type::Comparator;
protected:
  std::priority_queue<XXX, std::vector<XXX>, BaseType> m_Queue;
  uint32                   m_Capacity;

  //threading utils
  std::mutex              m_Mutex;
  std::condition_variable m_EnqueueConditionVariable;
  std::condition_variable m_DequeueConditionVariable;

public:
  xPriorityQueue(int32 QueueSize = 1) { setCapacity(QueueSize); }

  void     setCapacity(int32 Capacity)       { assert(Capacity > 0); std::lock_guard<std::mutex> LockManager(m_Mutex); m_Capacity = Capacity; m_EnqueueConditionVariable.notify_all(); }
  int32    getCapacity(              ) const { return m_Capacity; }

  bool     isEmpty  () { return m_Queue.empty(); }
  bool     isFull   () { return (m_Queue.size() == m_Capacity); }
  uintSize getLoad  () { return m_Queue.size(); }

  void EnqueueResize(XXX  Data);
  bool DequeueTry   (XXX& Data);

  void EnqueueWait  (XXX  Data);
  void EnqueueWaitN (XXX* Data, int32 Num);
  void DequeueWait  (XXX& Data);
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<class XXX> void xPriorityQueue<XXX>::EnqueueResize(XXX Data)
{
  std::lock_guard<std::mutex> LockManager(m_Mutex);
  if(isFull()) { m_Capacity++; }
  m_Queue.push(Data);
  m_DequeueConditionVariable.notify_one();
  //release lock - std::lock_guard destructor...
}
template<class XXX> bool xPriorityQueue<XXX>::DequeueTry(XXX& Data)
{
  std::lock_guard<std::mutex> LockManager(m_Mutex);
  if(!m_Queue.empty())
  {
    Data = m_Queue.top();
    m_Queue.pop();
    m_EnqueueConditionVariable.notify_one();
    return true;
  }
  else { return false; }
  //release lock - std::lock_guard destructor...
}
template<class XXX> void xPriorityQueue<XXX>::EnqueueWait(XXX Data)
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_Queue.size()>= m_Capacity) { m_EnqueueConditionVariable.wait(LockManager, [&]{ return m_Queue.size()< m_Capacity;} ); }
  m_Queue.push(Data);
  m_DequeueConditionVariable.notify_one();
  //release lock - std::unique_lock destructor... 
}
template<class XXX> void xPriorityQueue<XXX>::EnqueueWaitN(XXX* /*Data*/, int32 /*Num*/)
{
  assert(0); exit(EXIT_FAILURE);
}
template<class XXX> void xPriorityQueue<XXX>::DequeueWait(XXX& Data)
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_Queue.empty()) { m_DequeueConditionVariable.wait(LockManager, [&]{ return !m_Queue.empty(); } ); }
  Data = m_Queue.top();
  m_Queue.pop();
  m_EnqueueConditionVariable.notify_one();
  //release lock - std::unique_lock destructor... 
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
