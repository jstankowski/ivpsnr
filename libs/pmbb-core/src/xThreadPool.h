/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefCORE.h"
#include "xQueue.h"
#include "xRing.h"
#include "xEvent.h"
#include "xMemory.h"
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <future>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xIdxGenerator
{
private:
  std::set<int32> m_UnusedIdxs;
  size_t          m_TotalIdxs = 0;

public:
  int32 borrowIdx()
  {
    int32 Idx = NOT_VALID;
    if(m_UnusedIdxs.empty()) { Idx = (int32)m_TotalIdxs++; }
    else                     { Idx = m_UnusedIdxs.extract(m_UnusedIdxs.begin()).value(); }
    return Idx;
  }

  void givebackIdx(int32 Idx) { m_UnusedIdxs.insert(Idx); }

  size_t size() const { return m_TotalIdxs - m_UnusedIdxs.size(); }
};

//===============================================================================================================================================================================================================

class xThreadPool
{
public:
  class xTaskBase
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
    
    static const int8 c_PriorityDef = 0;
    static const int8 c_PriorityMin = std::numeric_limits<int8>::min();
    static const int8 c_PriorityMax = std::numeric_limits<int8>::max();

  protected:
    int8    m_ClientId = NOT_VALID;
    int8    m_Priority = c_PriorityDef;
    eType   m_Type     = eType  ::UNKNOWN;
    eStatus m_Status   = eStatus::UNKNOWN;

    virtual void WorkingFunction(int32 ThreadIdx) = 0;

  public:
//             xPoolTask(uintPtr ClientId, int8 Priority) { m_ClientId = ClientId; m_Priority = Priority; m_Type = eType::UNKNOWN; m_Status = eStatus::UNKNOWN; }
    virtual ~xTaskBase() {};

    static  void StarterFunction(xTaskBase* PoolTask, int32 ThreadIdx);

    void    setClientId(int8    ClientId )       { m_ClientId = ClientId; }
    int8    getClientId(                 ) const { return m_ClientId;     }
    void    setPriority(int8    Priority )       { m_Priority = Priority; }
    int8    getPriority(                 ) const { return m_Priority;     }
    eType   getType    (                 ) const { return m_Type;         }
    void    setStatus  (eStatus Status   )       { m_Status = Status;     }
    eStatus getStatus  (                 ) const { return m_Status;       }

  public:
    class Comparator
    {
    public:
      bool operator()(xTaskBase* a, xTaskBase* b) { return a->getPriority() < b->getPriority(); }
    };
  };

  class xTaskFunction : public xTaskBase
  {
  public:
    using tFunct = std::function<void(int32)>; //void Function(int32 ThreadIdx)
  protected:
    tFunct m_Function; //void Function(int32 ThreadIdx)
  public:
    xTaskFunction(                                             ) { m_ClientId = 0       ; m_Priority = c_PriorityMin; m_Type = eType::Function; m_Status = eStatus::UNKNOWN; m_Function = nullptr ; }
    xTaskFunction(int8 ClientId, int8 Priority, tFunct Function) { m_ClientId = ClientId; m_Priority = Priority     ; m_Type = eType::Function; m_Status = eStatus::Waiting; m_Function = Function; }
    
    void setFunction ([[maybe_unused]] int8 ClientId, int8 Priority, tFunct Function) { assert(m_ClientId == ClientId); m_Priority = Priority; assert(m_Type == eType::Function); m_Status = eStatus::Waiting; m_Function = Function; }
  protected:
    void WorkingFunction(int32 ThreadIdx) final { m_Function(ThreadIdx); }
  };

protected:
  class xTaskTerminator : public xTaskBase
  {
  public:
    xTaskTerminator() { m_ClientId = NOT_VALID; m_Priority = c_PriorityMax; m_Type = eType::Terminator; m_Status = eStatus::Waiting; }
  protected:
    void WorkingFunction(int32 /*ThreadIdx*/) final {}
  };

public:
  using tWQ = xPtrRing<xTaskBase>;
  using tCQ = xPtrRing<xTaskBase>;

protected:
  //threads data
  int32                            m_NumThreads;
  xEvent                           m_Event;
  std::vector<std::future<uint32>> m_Future;
  std::vector<std::thread        > m_Thread;
  std::vector<std::thread::id    > m_ThreadId;

  //client id
  xIdxGenerator m_ClientIdxGen;

  //input & output queques
  tWQ                 m_WaitingTasks  ;
  std::map<int8, tCQ> m_CompletedTasks;
  
protected:  
  uint32        xThreadFunc();
  static uint32 xThreadStarter(xThreadPool* ThreadPool) { return ThreadPool->xThreadFunc(); }

public:
  xThreadPool() : m_Event(true, false) { m_NumThreads = 0; }
  xThreadPool            (const xThreadPool&) = delete; //delete copy constructor
  xThreadPool& operator= (const xThreadPool&) = delete; //delete assignement operator

  void       create   (int32 NumThreads, int32 WaitingQueueSize);
  void       destroy  ();
             
  int8       registerClient  (int32 CompletedQueueSize);
  bool       unregisterClient(int8  ClientId          );

  void       submitTask  (xTaskBase*  Task                           ) { m_WaitingTasks.insertWait(Task); }
  void       submitTasks (xTaskBase** Tasks, int32 Num               ) { m_WaitingTasks.insertWait(Tasks, Num); }
  xTaskBase* receiveTask (                              int8 ClientId) { return m_CompletedTasks.at(ClientId).removeWait(); }
  void       receiveTasks(xTaskBase** Tasks, int32 Num, int8 ClientId) { m_CompletedTasks.at(ClientId).removeWait(Tasks, Num); }

  int32      getWaitingQueueCapacity  (             ) { return m_WaitingTasks.getSize(); }
  int32      getWaitingQueueLoad      (             ) { return m_WaitingTasks.getLoad(); }
  bool       isWaitingQueueEmpty      (             ) { return m_WaitingTasks.isEmpty(); }
  bool       isWaitingQueueFull       (             ) { return m_WaitingTasks.isFull (); }

  int32      getCompletedQueueCapacity(int8 ClientId) { return m_CompletedTasks.at(ClientId).getSize(); }
  int32      getCompletedQueueLoad    (int8 ClientId) { return m_CompletedTasks.at(ClientId).getLoad(); }
  bool       isCompletedQueueEmpty    (int8 ClientId) { return m_CompletedTasks.at(ClientId).isEmpty(); }
  bool       isCompletedQueueFull     (int8 ClientId) { return m_CompletedTasks.at(ClientId).isFull (); }

  int32      getNumThreads            (             ) { return m_NumThreads; }
};

//===============================================================================================================================================================================================================

//template <class XXX> class xTaskStorage
//{
//  XXX*  m_Buff = nullptr  ;
//  int32 m_Size = NOT_VALID;
//  int32 m_Pos  = NOT_VALID;
//
//  void create(int32 Size)
//  {
//    m_Stack = (XXX*)xMemory::AlignedMallocPage(Size * sizeof(XXX));
//    m_Size  = Size;
//    m_Pos   = 0;
//  }
//  void destroy()
//  {
//    xMemory::xAlignedFreeNull(m_Stack);
//    m_Size = NOT_VALID;
//    m_Pos  = NOT_VALID;
//  }
//  void push(XXX* Unit)
//  {
//    assert(m_Pos + 1 < m_Size);
//    m_Stack[m_Pos++] = Unit;
//  }
//  void push(XXX** Units, int32 NumUnits)
//  {
//    assert(m_Pos + NumUnits < m_Size);
//    memcpy(m_Stack + m_Pos, Units, NumUnits * sizeof(XXX*));
//    m_Pos += NumUnits;
//  }
//  XXX* poptop()
//  {
//    return m_Buff[--m_Pos];
//  }
//};

//===============================================================================================================================================================================================================

class xThreadPoolInterfaceBase
{
public:
  using tTask = xThreadPool::xTaskBase;

protected:
  xThreadPool*  m_ThreadPool = nullptr;
  int8          m_ClientIdx  = NOT_VALID;
  int8          m_Priority   = std::numeric_limits<uint8>::min();
  int32         m_NumChunks  = NOT_VALID;

public:
  xThreadPoolInterfaceBase() { m_ClientIdx = NOT_VALID; m_ThreadPool = nullptr; m_Priority = tTask::c_PriorityDef; m_NumChunks = NOT_VALID; }
  xThreadPoolInterfaceBase            (const xThreadPoolInterfaceBase&) = delete; //delete copy constructor
  virtual ~xThreadPoolInterfaceBase() {};
  xThreadPoolInterfaceBase& operator= (const xThreadPoolInterfaceBase&) = delete; //delete assignement operator

  virtual void init    (xThreadPool* ThreadPool, int32 CompletedQueueSize, int32 NumPreAllocatedFunctionTasks);
  virtual void uninit  ();
  bool         isActive() { return m_ThreadPool != nullptr; }

  void   setPriority  (int8  Priority ){ m_Priority = Priority; }
  int8   getPriority  (               ){ return m_Priority; }
  void   setNumChunks (int32 NumChunks){ m_NumChunks = NumChunks; }
  int32  getNumChunks (               ){ return m_NumChunks; }

  void   submitTask (tTask* Task); // submit new waiting task
  tTask* receiveTask(           ); // receive completed task

  int32  getWaitingQueueLoad  () { return m_ThreadPool->getWaitingQueueLoad(); }
  bool   isWaitingQueueEmpty  () { return m_ThreadPool->isWaitingQueueEmpty(); }
  bool   isWaitingQueueFull   () { return m_ThreadPool->isWaitingQueueFull (); }
  int32  getCompletedQueueLoad() { return m_ThreadPool->getCompletedQueueLoad(m_ClientIdx); }
  bool   isCompletedQueueEmpty() { return m_ThreadPool->isCompletedQueueEmpty(m_ClientIdx); }
  bool   isCompletedQueueFull () { return m_ThreadPool->isCompletedQueueFull (m_ClientIdx); }
  int32  getNumThreads        () { return m_ThreadPool != nullptr ? m_ThreadPool->getNumThreads() : 0; }

};

//===============================================================================================================================================================================================================

class xThreadPoolInterfaceFunction : public xThreadPoolInterfaceBase
{
public:
  using tTaskF = xThreadPool::xTaskFunction;
  using tFunct = std::function<void(int32)>;

protected:
  std::vector<tTaskF*> m_UnusedTasks;
  std::vector<tTask* > m_StoredTasks;

public:
  xThreadPoolInterfaceFunction() { m_ClientIdx = NOT_VALID; m_ThreadPool = nullptr; m_Priority = tTask::c_PriorityDef; m_NumChunks = NOT_VALID; }
  xThreadPoolInterfaceFunction            (const xThreadPoolInterfaceFunction&) = delete; //delete copy constructor
  xThreadPoolInterfaceFunction& operator= (const xThreadPoolInterfaceFunction&) = delete; //delete assignement operator

  void init  (xThreadPool* ThreadPool, int32 CompletedQueueSize, int32 NumPreAllocatedFunctionTasks) final;
  void uninit() final;

  void   addWaitingTask        (tFunct Function);
  void   waitUntilTasksFinished(int32 NumTasksToWaitFor);

  //faster interface for batch submision - less locking overhead
  void   storeTask             (tFunct Function); // store new task in buffer
  int32  submitStoredTasks     (); // submit entire content of buffer
  void   executeStoredTasks    (); // submit entire content of buffer & wait until finished

  /*EXAMPLE 0 - task are available for worker pool one by one, just after each addWaitingTask completion
  *   for(int32 i=0; i<Num; i++) { THPI->addWaitingTask([some function]); }
  *   THPI->waitUntilTasksFinished(Num);
  * 
  * EXAMPLE 1 - task are available for worker pool in sigle batch, just after submitStoredTasks completion
  *   for(int32 i=0; i<Num; i++) { THPI->storeTask([some function]); }
  *   THPI->submitStoredTasks();
  *   THPI->waitUntilTasksFinished(Num);
  * 
  * EXAMPLE 2 - simplified EXAMPLE 1
  *   for(int32 i=0; i<Num; i++) { THPI->storeTask([some function]); }
  *   THPI->executeStoredTasks();
  */
};

//===============================================================================================================================================================================================================
// Convinient threading
//===============================================================================================================================================================================================================

using tThPI = xThreadPoolInterfaceFunction;

//===============================================================================================================================================================================================================

} //end of namespace PMBB
