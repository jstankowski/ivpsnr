/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xThreadPool.h"

using namespace std::chrono_literals;

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xThreadPool::create(int32 NumThreads, int32 WaitingQueueSize)
{
  assert(NumThreads      >0);
  assert(WaitingQueueSize>0);

  m_NumThreads = NumThreads;
  m_WaitingTasks.setSize(WaitingQueueSize);

  for(int32 i=0; i<m_NumThreads; i++)
  {
    std::packaged_task<uint32(xThreadPool*)> PackagedTask(xThreadStarter);
    m_Future.push_back(PackagedTask.get_future());
    std::thread Thread = std::thread(std::move(PackagedTask), this);
    m_ThreadId.push_back(Thread.get_id());
    m_Thread  .push_back(std::move(Thread));      
  }
  
  m_Event.set();
}
void xThreadPool::destroy()
{
  bool AnyActive = false;

  assert(isWaitingQueueEmpty());

  for(int32 i=0; i<m_NumThreads; i++)
  {
    xPoolTask* Terminator = new xPoolTaskTerminator;
    m_WaitingTasks.EnqueueWait(Terminator);
  }

  for(int32 i=0; i<m_NumThreads; i++)
  {   
    std::future_status Status = m_Future[i].wait_for(500ms);
    if(Status == std::future_status::ready && m_Thread[i].joinable()) { m_Thread[i].join(); }
    else                                                              { AnyActive = true;   }
  }

  if(AnyActive)
  {
    for(int32 i=0; i<m_NumThreads; i++)
    {
      std::future_status Status = m_Future[i].wait_for(5s);
      if(Status == std::future_status::ready && m_Thread[i].joinable()) { m_Thread[i].join();    }
      else                                                              { m_Thread[i].~thread(); }
    }
  }

  for(std::pair<const uintPtr, xQueue<xPoolTask*>>& Pair : m_CompletedTasks)
  {
    xQueue<xPoolTask*>& CompletedTaskQueue = Pair.second;
    int32 NumCompleted = (int32)CompletedTaskQueue.getLoad();
    for(int32 i=0; i<NumCompleted; i++)
    {
      xPoolTask* Task;
      CompletedTaskQueue.DequeueWait(Task);
      delete Task;
    }
  }  
}
bool xThreadPool::registerClient(uintPtr ClientId, int32 CompletedQueueSize)
{
  if(m_CompletedTasks.find(ClientId) != m_CompletedTasks.end()) { return false; }

  m_CompletedTasks.emplace(ClientId, CompletedQueueSize);
  return true;
}
bool xThreadPool::unregisterClient(uintPtr ClientId)
{
  if(m_CompletedTasks.find(ClientId) == m_CompletedTasks.end()) { return false; }

  xQueue<xPoolTask*>& CompletedTaskQueue = m_CompletedTasks.at(ClientId);
  int32 NumCompleted = (int32)CompletedTaskQueue.getLoad();
  for(int32 i=0; i<NumCompleted; i++)
  {
    xPoolTask* Task;
    CompletedTaskQueue.DequeueWait(Task);
    delete Task;
  }

  m_CompletedTasks.erase(ClientId);
  return true;
}
uint32 xThreadPool::xThreadFunc() 
{
  m_Event.wait();
  std::thread::id ThreadId = std::this_thread::get_id();
  int32 ThreadIdx = (int32)(std::find(m_ThreadId.begin(), m_ThreadId.end(), ThreadId) - m_ThreadId.begin());
  while(1)
  {    
    xPoolTask* Task;
    m_WaitingTasks.DequeueWait(Task);
    if(Task->getType() == xPoolTask::eType::Terminator) { delete Task; break; }
    xPoolTask::StarterFunction(Task, ThreadIdx);
    m_CompletedTasks.at(Task->getClientId()).EnqueueWait(Task);
  }
  return EXIT_SUCCESS;
}

//===============================================================================================================================================================================================================

void xThreadPool::xPoolTask::StarterFunction(xPoolTask* WorkerTask, int32 ThreadIdx)
{
  assert(WorkerTask->m_Status == xPoolTask::eStatus::Waiting);
  WorkerTask->m_Status = xPoolTask::eStatus::Processed;
  WorkerTask->WorkingFunction(ThreadIdx);
  WorkerTask->m_Status = xPoolTask::eStatus::Completed;
}

//===============================================================================================================================================================================================================

void xThreadPoolInterface::init(xThreadPool* ThreadPool, int32 CompletedQueueSize, int32 NumPreAllocatedFunctionTasks)
{
  m_ThreadPool = ThreadPool;
  m_ThreadPool->registerClient(getClientId(), CompletedQueueSize);
  m_NumChunks  = m_ThreadPool->getNumThreads();
  //pre init tasks
  for(int32 i = 0; i < NumPreAllocatedFunctionTasks; i++) { m_UnusedTasks.push(new tTaskF(m_Id, m_Priority, nullptr)); }
}
void xThreadPoolInterface::uininit()
{
  if(m_ThreadPool == nullptr) { return; }
  m_ThreadPool->unregisterClient(getClientId());
  m_ThreadPool = nullptr;
  //clean unused tasks
  while(!m_UnusedTasks.empty()) { tTaskF* Task = m_UnusedTasks.top(); m_UnusedTasks.pop(); delete Task; }
}
void xThreadPoolInterface::addWaitingTask(tTask* Task)
{
  Task->setClientId(getClientId());
  Task->setPriority(m_Priority);
  m_ThreadPool->addWaitingTask(Task);
}
void xThreadPoolInterface::addWaitingTask(std::function<void(int32)> Function)
{ 
  //inactive xThreadPoolInterface will execute function taks in calling thread context
  //allows to simplity code and avoid duplicating threaded and non-theaded variants
  if(!isActive()) { Function(NOT_VALID); return; }

  tTaskF* Task = nullptr;
  if(m_UnusedTasks.empty()) { Task = m_UnusedTasks.top(); m_UnusedTasks.pop(); Task->setPrioFunction(m_Priority, Function); }
  else                      { Task = new tTaskF(m_Id, m_Priority, Function); }
  m_ThreadPool->addWaitingTask(Task);
}
void xThreadPoolInterface::waitUntilTasksFinished(int32 NumTasksToWaitFor)
{
  //inactive xThreadPoolInterface will execute function taks in calling thread context
  //allows to simplity code and avoid duplicating threaded and non-theaded variants
  if(!isActive()) { return; };

  for(int32 TaskId=0; TaskId < NumTasksToWaitFor; TaskId++)
  {
    tTask* Task = receiveCompletedTask();
    if(Task->getType() == tTask::eType::Function) { m_UnusedTasks.push((tTaskF*)Task); }
    else { delete Task; }
  }
}
void xThreadPoolInterface::executeTask(std::function<void(int32)> Function)
{
  addWaitingTask(Function);
  waitUntilTasksFinished(1);
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
