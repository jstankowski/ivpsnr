/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefPMBB-CORE.h"
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xEvent - thread safe binary event
//===============================================================================================================================================================================================================
class xEvent
{
protected:
  bool                    m_State;
  bool                    m_ManualReset;
  std::mutex              m_Mutex;
  std::condition_variable m_ConditionVariable;

public:
  xEvent(bool ManualReset, bool InitialState) : m_State(InitialState), m_ManualReset(ManualReset) {}
  xEvent(const xEvent&) = delete;
  xEvent& operator=(const xEvent&) = delete;

  inline void set     ();
  inline void reset   ();
  inline void wait    ();
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xEvent::set()
{
  std::lock_guard<std::mutex> LockManager(m_Mutex);
  m_State = true;
  m_ConditionVariable.notify_all();
}  
void xEvent::reset()
{
  std::lock_guard<std::mutex> LockManager(m_Mutex);
  m_State = false;
}
void xEvent::wait()
{
  std::unique_lock<std::mutex> LockManager(m_Mutex);
  while(m_State == false){ m_ConditionVariable.wait(LockManager, [&]{ return m_State;}); }
  if(!m_ManualReset) { m_State = false; }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
