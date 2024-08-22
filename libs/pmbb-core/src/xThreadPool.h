/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefCORE.h"
#include "xQueue.h"
#include "xEvent.h"
#include <vector>
#include <map>
#include <stack>
#include <future>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xThreadPool
{
public:
  class xPoolTask
  {
  public:
    enum class eStatus : int8
    {
      INVALID = NOT_VALID,
      UNKNOWN = 0,
      Waiting,
      Processed,
      Completed,
    };

    enum class eType : int8
    {
      UNKNOWN = 0,
      Function,
      Terminator,
      Custom
    };
    
    static const int8 c_DefaultPriority = 0;

  protected:
    uintPtr m_ClientId = (uintPtr)nullptr;
    int8    m_Priority = c_DefaultPriority;
    eType   m_Type     = eType  ::UNKNOWN;
    eStatus m_Status   = eStatus::UNKNOWN;

  protected:
    virtual void WorkingFunction(int32 ThreadIdx) = 0;

  public:
//             xPoolTask(uintPtr ClientId, int8 Priority) { m_ClientId = ClientId; m_Priority = Priority; m_Type = eType::UNKNOWN; m_Status = eStatus::UNKNOWN; }
    virtual ~xPoolTask() {};

    static  void StarterFunction(xPoolTask* PoolTask, int32 ThreadIdx);

    void    setClientId(uintPtr ClientId )       { m_ClientId = ClientId; }
    uintPtr getClientId(                 ) const { return m_ClientId;     }
    void    setPriority(int8    Priority )       { m_Priority = Priority; }
    int8    getPriority(                 ) const { return m_Priority;     }
    eType   getType    (                 ) const { return m_Type;         }
    void    setStatus  (eStatus Status   )       { m_Status = Status;     }
    eStatus getStatus  (                 ) const { return m_Status;       }

  public:
    class Comparator
    {
    public:
      bool operator()(xPoolTask* a, xPoolTask* b) { return a->getPriority() < b->getPriority(); }
    };
  };

  class xPoolTaskFunction : public xPoolTask
  {
  public:
    using tFunct = std::function<void(int32)>; //void Function(int32 ThreadIdx)
  protected:
    tFunct m_Function; //void Function(int32 ThreadIdx)
  public:
    xPoolTaskFunction(                                                ) { m_ClientId = 0       ; m_Priority = int8_min; m_Type = eType::Function; m_Status = eStatus::UNKNOWN; m_Function = nullptr ; }
    xPoolTaskFunction(uintPtr ClientId, int8 Priority, tFunct Function) { m_ClientId = ClientId; m_Priority = Priority; m_Type = eType::Function; m_Status = eStatus::Waiting; m_Function = Function; }
    
    void setPrioFunction(int8 Priority, tFunct& Function) { m_Priority = Priority; m_Function = Function; }
  protected:
    void WorkingFunction(int32 ThreadIdx) final { m_Function(ThreadIdx); }
  };

protected:
  class xPoolTaskTerminator : public xPoolTask
  {
  public:
    xPoolTaskTerminator() { m_ClientId = (uintPtr)nullptr; m_Priority = int8_max; m_Type = eType::Terminator; m_Status = eStatus::Waiting; }
  protected:
    void WorkingFunction(int32 /*ThreadIdx*/) final {}
  };

protected:
  //threads data
  int32                            m_NumThreads;
  xEvent                           m_Event;
  std::vector<std::future<uint32>> m_Future;
  std::vector<std::thread>         m_Thread;
  std::vector<std::thread::id>     m_ThreadId;

  //input & output queques
  xPriorityQueue<xPoolTask*>            m_WaitingTasks;
  std::map<uintPtr, xQueue<xPoolTask*>> m_CompletedTasks;
  
protected:  
  uint32        xThreadFunc();
  static uint32 xThreadStarter(xThreadPool* ThreadPool) { return ThreadPool->xThreadFunc(); }

public:
  xThreadPool() : m_Event(true, false) { m_NumThreads = 0; }
  xThreadPool            (const xThreadPool&) = delete; //delete copy constructor
  xThreadPool& operator= (const xThreadPool&) = delete; //delete assignement operator

  void       create   (int32 NumThreads, int32 WaitingQueueSize);
  void       destroy  ();
             
  bool       registerClient  (uintPtr ClientId, int32 CompletedQueueSize);
  bool       unregisterClient(uintPtr ClientId);

  void       addWaitingTask       (xPoolTask* Task  ) { m_WaitingTasks.EnqueueWait(Task); }
  xPoolTask* receiveCompletedTask (uintPtr ClientId ) { xPoolTask* Task; m_CompletedTasks.at(ClientId).DequeueWait(Task); return Task; }
  int32      getWaitingQueueSize  (                 ) { return m_WaitingTasks.getSize(); }
  bool       isWaitingQueueEmpty  (                 ) { return m_WaitingTasks.isEmpty(); }
  int32      getCompletedQueueSize(uintPtr ClientId ) { return m_CompletedTasks.at(ClientId).getSize(); }
  bool       isCompletedQueueEmpty(uintPtr ClientId ) { return m_CompletedTasks.at(ClientId).isEmpty(); }
  int32      getNumThreads        (                 ) { return m_NumThreads; }
};

//===============================================================================================================================================================================================================

class xThreadPoolInterface
{
public:
  using tTask  = xThreadPool::xPoolTask;
  using tTaskF = xThreadPool::xPoolTaskFunction;

protected:
  const uintPtr m_Id         = 0;
  xThreadPool*  m_ThreadPool = nullptr;
  int8          m_Priority   = std::numeric_limits<uint8>::min();
  int32         m_NumChunks  = NOT_VALID;

  std::stack<xThreadPool::xPoolTaskFunction*> m_UnusedTasks;

public:
  xThreadPoolInterface() : m_Id((uintPtr)this) { m_ThreadPool = nullptr; m_Priority = tTask::c_DefaultPriority; m_NumChunks = NOT_VALID; }
  xThreadPoolInterface            (const xThreadPoolInterface&) = delete; //delete copy constructor
  xThreadPoolInterface& operator= (const xThreadPoolInterface&) = delete; //delete assignement operator

  void init   (xThreadPool* ThreadPool, int32 CompletedQueueSize, int32 NumPreAllocatedFunctionTasks);
  void uininit();
  bool isActive() { return m_ThreadPool != nullptr; }

  void   addWaitingTask        (tTask* Task);
  void   addWaitingTask        (std::function<void(int32)> Function);
  tTask* receiveCompletedTask  () { return m_ThreadPool->receiveCompletedTask(getClientId()); }
  void   waitUntilTasksFinished(int32 NumTasksToWaitFor);
  void   executeTask           (std::function<void(int32)> Function);

  int32  getWaitingQueueSize  () { return m_ThreadPool->getWaitingQueueSize(); }
  bool   isWaitingQueueEmpty  () { return m_ThreadPool->isWaitingQueueEmpty(); }
  int32  getCompletedQueueSize() { return m_ThreadPool->getCompletedQueueSize(getClientId()); }
  bool   isCompletedQueueEmpty() { return m_ThreadPool->isCompletedQueueEmpty(getClientId()); }
  int32  getNumThreads        () { return m_ThreadPool != nullptr ? m_ThreadPool->getNumThreads() : 0; }

  void   setPriority  (int8  Priority ){ m_Priority = Priority; }
  int8   getPriority  (               ){ return m_Priority; }
  void   setNumChunks (int32 NumChunks){ m_NumChunks = NumChunks; }
  int32  getNumChunks (               ){ return m_NumChunks; }

protected:
  uintPtr getClientId() { return (uintPtr)this; }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
