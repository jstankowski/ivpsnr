/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefCORE.h"
#include "xThreadPool.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// Threading
//===============================================================================================================================================================================================================
class xMultiThreaded
{
public:
  static constexpr int32 c_NumRowsInRng = 8;

protected:
  tThPI* m_ThPI     = nullptr; //thread pool interface
  bool   m_OwnsThPI = false  ;

public:
  void createThrdPoolIntf (xThreadPool* ThreadPool, int32 MaxNumTasks)
  { 
    m_ThPI     = new tThPI;
    m_OwnsThPI = true;
    if(ThreadPool) { m_ThPI->init(ThreadPool, MaxNumTasks, MaxNumTasks); }
  }
  void destroyThrdPoolIntf()
  {
    if(!m_OwnsThPI) { return; }
    m_ThPI->uninit();
    delete m_ThPI; m_ThPI = nullptr;
  }
  bool bindThrdPoolIntf(tThPI* ThPI)
  {
    if(m_ThPI != nullptr) { return false; }
    m_ThPI     = ThPI ;
    m_OwnsThPI = false;
    return true;
  }
  tThPI* unbindThrdPoolIntf()
  {
    if(m_OwnsThPI || m_ThPI == nullptr) { return nullptr; }
    tThPI* Tmp = m_ThPI; 
    m_ThPI = nullptr;
    return Tmp;
  }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
