/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefPMBB-CORE.h"
#include "xQueue.h"
#include "xEvent.h"
#include <vector>
#include <map>
#include <future>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xThreadPool
{
public:
  enum class eTaskStatus
  {
    INVALID = NOT_VALID,
    UNKNOWN = 0,
    Waiting,
    Processed,
    Completed,
    Terminate,
  };

public:
  class xWorkerTask
  {
  public:
    static const int8 c_DefaultPriority = 0;

  protected:
    uintPtr     m_ClientId;
    int8        m_Priority;
    eTaskStatus m_Status;

  protected:
    virtual void WorkingFunction(int32 ThreadIdx) = 0;

  public:
             xWorkerTask(                               ) { m_ClientId = (uintPtr)nullptr;  m_Priority = c_DefaultPriority; m_Status = eTaskStatus::UNKNOWN; }
             xWorkerTask(uintPtr ClientId, int8 Priority) { m_ClientId = ClientId;          m_Priority = Priority;          m_Status = eTaskStatus::UNKNOWN; }
    virtual ~xWorkerTask(                               ) { }

    static  void StarterFunction(xWorkerTask* WorkerTask, int32 ThreadIdx);

    void         setClientId (uintPtr     ClientId ){ m_ClientId = ClientId; }
    uintPtr      getClientId (                     ){ return m_ClientId; }
    void         setPriority (int8        Priority ){ m_Priority = Priority; }
    int8         getPriority (                     ){ return m_Priority; }
    void         setStatus   (eTaskStatus Status   ){ m_Status = Status; }
    eTaskStatus  getStatus   (                     ){ return m_Status; }

  public:
    class Comparator
    {
    public:
      bool operator()(xWorkerTask* a, xWorkerTask* b) { return a->getPriority() < b->getPriority(); }
    };
  };

  class xWorkerTaskFunction : public xWorkerTask
  {
  protected:
    std::function<void(int32)> m_Function; //void Function(int32 ThreadIdx)

  public:
    xWorkerTaskFunction(uintPtr ClientId, int8 Priority, std::function<void(int32)> Function) : xWorkerTask(ClientId, Priority) { m_Function = Function; m_Status = eTaskStatus::Waiting; }

  protected:
    void WorkingFunction(int32 ThreadIdx) final { m_Function(ThreadIdx); }
  };

protected:
  class xWorkerTaskTerminator : public xWorkerTask
  {
  public:
    xWorkerTaskTerminator() { m_ClientId = (uintPtr)nullptr;  m_Priority = int8_max; m_Status = eTaskStatus::Terminate; }
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

  //input queque
  xPriorityQueue<xWorkerTask*> m_WaitingTasks;

  //output
  std::map<uintPtr, xQueue<xWorkerTask*>> m_CompletedTasks;


protected:  
  uint32        xThreadFunc();
  static uint32 xThreadStarter(xThreadPool* ThreadPool) { return ThreadPool->xThreadFunc(); }

public:
  xThreadPool() : m_Event(true, false) { m_NumThreads = 0; }

  void         create   (int32 NumThreads, int32 WaitingQueueSize);
  void         destroy  ();
               
  bool         registerClient  (uintPtr ClientId, int32 CompletedQueueSize);
  bool         unregisterClient(uintPtr ClientId);

  void         addWaitingTask       (xWorkerTask* Task) { m_WaitingTasks.EnqueueWait(Task); }
  xWorkerTask* receiveCompletedTask (uintPtr ClientId ) { xWorkerTask* Task; m_CompletedTasks.at(ClientId).DequeueWait(Task); return Task; }  
  int32        getWaitingQueueSize  (                 ) { return m_WaitingTasks.getSize(); }
  bool         isWaitingQueueEmpty  (                 ) { return m_WaitingTasks.isEmpty(); }
  int32        getCompletedQueueSize(uintPtr ClientId ) { return m_CompletedTasks.at(ClientId).getSize(); }
  bool         isCompletedQueueEmpty(uintPtr ClientId ) { return m_CompletedTasks.at(ClientId).isEmpty(); }
  int32        getNumThreads        (                 ) { return m_NumThreads; }
};

//===============================================================================================================================================================================================================

class xThreadPoolInterface
{
public:
  using xWorkerTask = xThreadPool::xWorkerTask;

protected:
  xThreadPool* m_ThreadPool;
  int8         m_Priority;
  int32        m_NumChunks;

public:
  xThreadPoolInterface() { m_ThreadPool = nullptr; m_Priority = xThreadPool::xWorkerTask::c_DefaultPriority; m_NumChunks = NOT_VALID; }

  void init   (xThreadPool* ThreadPool, int32 CompletedQueueSize);
  void uininit();
  bool isActive() { return m_ThreadPool != nullptr; }

  void         addWaitingTask        (xThreadPool::xWorkerTask* Task);
  void         addWaitingTask        (std::function<void(int32)> Function);
  xWorkerTask* receiveCompletedTask  () { return m_ThreadPool->receiveCompletedTask(getClientId()); }
  void         waitUntilTasksFinished(int32 NumTasksToWaitFor);
  void         executeTask           (std::function<void(int32)> Function);

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
