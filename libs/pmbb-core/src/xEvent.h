/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefCORE.h"
#if X_PMBB_CPP20
#  include <atomic>
#else
#  include <thread>
#  include <mutex>
#  include <condition_variable>
#endif

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
// xEvent - thread safe binary event
//===============================================================================================================================================================================================================

#if X_PMBB_CPP20

class xEvent
{
protected:
  std::atomic_flag m_State;
  const bool       m_ManualReset;

public:
  xEvent(bool ManualReset, bool InitialState) : m_ManualReset(ManualReset) { if(InitialState) { m_State.test_and_set(); } else { m_State.clear(); } }
  xEvent           (const xEvent&) = delete;
  xEvent& operator=(const xEvent&) = delete;

  inline void set  () { m_State.test_and_set(); m_State.notify_all(); }
  inline void reset() { m_State.clear(); }
  inline void wait () { m_State.wait(false); if(!m_ManualReset) { m_State.test_and_set(); } }
};

class xEvent2
{
protected:
  std::atomic_flag m_State;
  const bool       m_AutoReset;

public:
  xEvent2(bool InitialState = false, bool AutoReset = false) : m_AutoReset(AutoReset) { if(InitialState) { m_State.test_and_set(); } else { m_State.clear(); } }
  xEvent2           (const xEvent2&) = delete;
  xEvent2& operator=(const xEvent2&) = delete;

  inline void set  () { m_State.test_and_set(); m_State.notify_all(); }
  inline void reset() { m_State.clear(); }
  inline void wait () { m_State.wait(false); if(m_AutoReset) { m_State.test_and_set(); } }
};

#else //X_PMBB_CPP20

class xEvent
{
protected:
  bool                    m_State;
  bool                    m_ManualReset;
  std::mutex              m_Mutex;
  std::condition_variable m_ConditionVariable;

public:
  xEvent(bool ManualReset, bool InitialState) : m_State(InitialState), m_ManualReset(ManualReset) {}
  xEvent           (const xEvent&) = delete;
  xEvent& operator=(const xEvent&) = delete;

  inline bool get()
  {
    std::lock_guard<std::mutex> LockManager(m_Mutex);
    return m_State;
  }
  inline void set()
  {
    std::lock_guard<std::mutex> LockManager(m_Mutex);
    m_State = true;
    m_ConditionVariable.notify_all();
  }
  inline void reset()
  {
    std::lock_guard<std::mutex> LockManager(m_Mutex);
    m_State = false;
  }
  inline void wait()
  {
    std::unique_lock<std::mutex> LockManager(m_Mutex);
    while(m_State == false) { m_ConditionVariable.wait(LockManager, [&] { return m_State; }); }
    if(!m_ManualReset) { m_State = false; }
  }
  template<class Rep, class Period> bool wait_for(const std::chrono::duration<Rep, Period>& Duration)
  {
    std::unique_lock<std::mutex> LockManager(m_Mutex);
    bool State = m_ConditionVariable.wait_for(LockManager, Duration, [this] { return m_State; });
    if(!m_ManualReset && State) { m_State = false; }
    return State;
  }
};

class xEvent2
{
protected:
  bool                    m_State;
  const bool              m_AutoReset;
  std::mutex              m_Mutex;
  std::condition_variable m_ConditionVariable;

public:
  xEvent2(bool InitialState = false, bool AutoReset = false) : m_State(InitialState), m_AutoReset(AutoReset) { }
  xEvent2           (const xEvent2&) = delete;
  xEvent2& operator=(const xEvent2&) = delete;

  inline bool get()
  {
    std::lock_guard<std::mutex> LockManager(m_Mutex);
    return m_State;
  }
  inline void set()
  {
    std::lock_guard<std::mutex> LockManager(m_Mutex);
    m_State = true;
    m_ConditionVariable.notify_all();
  }
  inline void reset()
  {
    std::lock_guard<std::mutex> LockManager(m_Mutex);
    m_State = false;
  }
  inline void wait()
  {
    std::unique_lock<std::mutex> LockManager(m_Mutex);
    while(m_State == false) { m_ConditionVariable.wait(LockManager, [&] { return m_State; }); }
    if(m_AutoReset) { m_State = false; }
  }
  template<class Rep, class Period> bool wait_for(const std::chrono::duration<Rep, Period>& Duration)
  {
    std::unique_lock<std::mutex> LockManager(m_Mutex);
    bool State = m_ConditionVariable.wait_for(LockManager, Duration, [this] { return m_State; });
    if(m_AutoReset && State) { m_State = false; }
    return State;
  }  
};

#endif //X_PMBB_CPP20

//===============================================================================================================================================================================================================

} //end of namespace PMBB
