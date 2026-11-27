/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xThreadPool.h"
#if X_PMBB_THREAD_POOL_HAS_CORE_SELECTION
#include "xCoreAffinity.h"
#endif //X_PMBB_THREAD_POOL_HAS_CORE_SELECTION

using namespace std::chrono_literals;

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xThreadPool::xTaskBase::StarterFunction(xTaskBase* WorkerTask, int32 ThreadIdx)
{
  assert(WorkerTask->m_Status == xTaskBase::eStatus::Waiting);
  WorkerTask->m_Status = xTaskBase::eStatus::Processed;
  WorkerTask->WorkingFunction(ThreadIdx);
  WorkerTask->m_Status = xTaskBase::eStatus::Completed;
}

//===============================================================================================================================================================================================================

void xThreadPool::create(int32 NumThreads, int32 WaitingQueueSize)
{
  assert(NumThreads      >0);
  assert(WaitingQueueSize>0);

  m_NumThreads = NumThreads;
  m_WaitingTasks.create(WaitingQueueSize, true);

  std::atomic_thread_fence(std::memory_order_seq_cst);

  for(int32 i=0; i<m_NumThreads; i++)
  {
    std::packaged_task<uint32(xThreadPool*)> PackagedTask(xThreadStarter);
    m_Future.push_back(PackagedTask.get_future());
    std::thread Thread = std::thread(std::move(PackagedTask), this);
    m_ThreadId.push_back(Thread.get_id());
    m_Thread  .push_back(std::move(Thread));      
  }

  std::atomic_thread_fence(std::memory_order_seq_cst);
  
  m_Event.set();
}
#if X_PMBB_THREAD_POOL_HAS_CORE_SELECTION
void xThreadPool::create(const std::vector<int32>& CoreIdxs, const xCoreInfo* CoreInfos, int32 WaitingQueueSize)
{
  int32 NumThreads = (int32)CoreIdxs.size();

  assert(NumThreads      > 0);
  assert(WaitingQueueSize > 0);

  m_NumThreads = NumThreads;
  m_WaitingTasks.create(WaitingQueueSize, true);

  for(int32 i = 0; i < m_NumThreads; i++)
  {
    m_CoreInfos.push_back(CoreInfos + CoreIdxs[i]);
  }

  std::atomic_thread_fence(std::memory_order_seq_cst);

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
#endif //X_PMBB_THREAD_POOL_HAS_CORE_SELECTION
void xThreadPool::destroy()
{
  bool AnyActive = false;

  assert(isWaitingQueueEmpty());

  for(int32 i=0; i<m_NumThreads; i++)
  {
    xTaskBase* Terminator = new xTaskTerminator;
    m_WaitingTasks.insertWait(Terminator);
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

  for(auto& [Id, CompletedTaskQueue] : m_CompletedTasks)
  {
    int32 NumCompleted = (int32)CompletedTaskQueue.getLoad();
    for(int32 i=0; i<NumCompleted; i++)
    {
      xTaskBase* Task = CompletedTaskQueue.removeWait(); delete Task;
    }
  }

  m_WaitingTasks.destroy();
}
int8 xThreadPool::registerClient(int32 CompletedQueueSize)
{
  int8 ClientIdx = (int8)m_ClientIdxGen.borrowIdx();
  m_CompletedTasks.emplace(ClientIdx, CompletedQueueSize);
  return ClientIdx;
}
bool xThreadPool::unregisterClient(int8 ClientIdx)
{
  if(m_CompletedTasks.find(ClientIdx) == m_CompletedTasks.end()) { return false; }

  tCQ& CompletedTaskQueue = m_CompletedTasks.at(ClientIdx);
  int32 NumCompleted = (int32)CompletedTaskQueue.getLoad();
  for(int32 i=0; i<NumCompleted; i++)
  {
    xTaskBase* Task = CompletedTaskQueue.removeWait(); delete Task;
  }
  m_CompletedTasks.erase(ClientIdx);
  m_ClientIdxGen.givebackIdx(ClientIdx);
  return true;
}
uint32 xThreadPool::xThreadFunc() 
{
  m_Event.wait();
  std::thread::id ThreadId = std::this_thread::get_id();
  int32 ThreadIdx = (int32)(std::find(m_ThreadId.begin(), m_ThreadId.end(), ThreadId) - m_ThreadId.begin());

#if X_PMBB_THREAD_POOL_HAS_CORE_SELECTION
  if(!m_CoreInfos.empty()) { xCoreAffinity::pinCurrentThreadToCore(m_CoreInfos[ThreadIdx]); }
#endif //X_PMBB_THREAD_POOL_HAS_CORE_SELECTION

  while(1)
  {    
    xTaskBase* Task = m_WaitingTasks.removeWait();
    if(Task->getType() == xTaskBase::eType::Terminator) { delete Task; break; }
    xTaskBase::StarterFunction(Task, ThreadIdx);
    m_CompletedTasks.at(Task->getClientId()).insertWait(Task);
  }
  return EXIT_SUCCESS;
}

//===============================================================================================================================================================================================================
// xThreadPoolInterfaceBase
//===============================================================================================================================================================================================================

void xThreadPoolInterfaceBase::init(xThreadPool* ThreadPool, int32 CompletedQueueSize, int32 /*NumPreAllocatedFunctionTasks*/)
{
  m_ThreadPool = ThreadPool;
  m_ClientIdx  = m_ThreadPool->registerClient(CompletedQueueSize);
  m_NumChunks  = m_ThreadPool->getNumThreads();
}
void xThreadPoolInterfaceBase::uninit()
{
  if(m_ThreadPool == nullptr) { return; }
  m_ThreadPool->unregisterClient(m_ClientIdx);
  m_ThreadPool = nullptr;
  m_ClientIdx  = NOT_VALID;
}
void xThreadPoolInterfaceBase::submitTask(tTask* Task)
{
  //inactive xThreadPoolInterface will execute function taks in calling thread context
  //allows to simplity code and avoid duplicating threaded and non-theaded variants
  if(!isActive()) { Task->StarterFunction(Task, 0); return; }

  Task->setClientId(m_ClientIdx);
  Task->setPriority(m_Priority );
  m_ThreadPool->submitTask(Task);
}
xThreadPoolInterfaceBase::tTask* xThreadPoolInterfaceBase::receiveTask()
{
  //inactive xThreadPoolInterface will execute function taks in calling thread context
  //allows to simplity code and avoid duplicating threaded and non-theaded variants
  if(!isActive()) { return nullptr; }

  return m_ThreadPool->receiveTask(m_ClientIdx);
}

//===============================================================================================================================================================================================================

void xThreadPoolInterfaceFunction::init(xThreadPool* ThreadPool, int32 CompletedQueueSize, int32 NumPreAllocatedFunctionTasks)
{
  xThreadPoolInterfaceBase::init(ThreadPool, CompletedQueueSize, NumPreAllocatedFunctionTasks);
  //pre init tasks
  m_UnusedTasks.reserve(NumPreAllocatedFunctionTasks);
  for(int32 i = 0; i < NumPreAllocatedFunctionTasks; i++) { m_UnusedTasks.push_back(new tTaskF(m_ClientIdx, m_Priority, nullptr)); }
  m_StoredTasks.reserve(CompletedQueueSize);
}
void xThreadPoolInterfaceFunction::uninit()
{
  xThreadPoolInterfaceBase::uninit();
  //clean unused tasks
  while(!m_UnusedTasks.empty()) { tTaskF* Task = m_UnusedTasks.back(); m_UnusedTasks.pop_back(); delete Task; }
}
void xThreadPoolInterfaceFunction::addWaitingTask(tFunct Function)
{ 
  //inactive xThreadPoolInterface will execute function taks in calling thread context
  //allows to simplity code and avoid duplicating threaded and non-theaded variants
  if X_ATTR_UNLIKELY(!isActive()) { Function(0); return; }

  tTaskF* Task = nullptr;
  if X_ATTR_LIKELY (!m_UnusedTasks.empty())
  { 
    Task = m_UnusedTasks.back(); m_UnusedTasks.pop_back();
    Task->setFunction(m_ClientIdx, m_Priority, Function);
  }
  else
  { 
    Task = new tTaskF(m_ClientIdx, m_Priority, Function);
  }
  m_ThreadPool->submitTask(Task);
}
void xThreadPoolInterfaceFunction::waitUntilTasksFinished(int32 NumTasksToWaitFor)
{
  //inactive xThreadPoolInterface will execute function taks in calling thread context
  //allows to simplity code and avoid duplicating threaded and non-theaded variants
  if X_ATTR_LIKELY (isActive())
  {
    if(m_StoredTasks.size()) { assert(0); abort(); }
    m_StoredTasks.resize(NumTasksToWaitFor, nullptr);
    m_ThreadPool->receiveTasks(m_StoredTasks.data(), NumTasksToWaitFor, m_ClientIdx);
  }

  for(int32 TaskId=0; TaskId < NumTasksToWaitFor; TaskId++)
  {
    tTask* Task = m_StoredTasks[TaskId];
    if(Task->getType() == tTask::eType::Function)
    {
      Task->setStatus(tTask::eStatus::UNKNOWN);
      m_UnusedTasks.push_back((tTaskF*)Task);
    }
    else { delete Task; }
  }

  m_StoredTasks.clear();
}
void xThreadPoolInterfaceFunction::storeTask(tFunct Function)
{
  tTaskF* Task = nullptr;
  if(!m_UnusedTasks.empty())
  {
    Task = m_UnusedTasks.back(); m_UnusedTasks.pop_back();
    Task->setFunction(m_ClientIdx, m_Priority, Function);
  }
  else
  {
    Task = new tTaskF(m_ClientIdx, m_Priority, Function);
  }
  m_StoredTasks.push_back(Task);
}
int32 xThreadPoolInterfaceFunction::submitStoredTasks()
{
  //inactive xThreadPoolInterface will execute function taks in calling thread context
  //allows to simplity code and avoid duplicating threaded and non-theaded variants

  int32 NumStoredTasks = (int32)m_StoredTasks.size();

  if X_ATTR_LIKELY (isActive())
  {
    m_ThreadPool->submitTasks(m_StoredTasks.data(), (int32)m_StoredTasks.size());
    m_StoredTasks.clear();
  }
  else
  {
    for(int32 i = 0; i < NumStoredTasks; i++) { m_StoredTasks[i]->StarterFunction(m_StoredTasks[i], 0); }
  }  

  return NumStoredTasks;
}
void xThreadPoolInterfaceFunction::executeStoredTasks()
{
  if(m_StoredTasks.empty()) { return; }
  const int32 NumStoredTasks = submitStoredTasks();
  waitUntilTasksFinished(NumStoredTasks);
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
