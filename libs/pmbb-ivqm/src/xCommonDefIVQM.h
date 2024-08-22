/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

//===============================================================================================================================================================================================================
// PMBB-core
//===============================================================================================================================================================================================================
#include "xCommonDefCORE.h"
#include "xThreadPool.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// Threading
//===============================================================================================================================================================================================================
using tThPI = xThreadPoolInterface;

class xMultiThreaded
{
protected:
  tThPI m_ThPI; //thread pool interface

public:
  void  initThreadPool  (xThreadPool* ThreadPool, int32 Height) { if(ThreadPool) { m_ThPI.init(ThreadPool, Height, Height); } }
  void  uninitThreadPool() { m_ThPI.uininit(); }
};

//===============================================================================================================================================================================================================
// Compile time settings
//===============================================================================================================================================================================================================
static constexpr bool xc_USE_RUNTIME_CMPWEIGHTS  = true ; // use component weights provided at runtime
static constexpr bool xc_CLIP_CURR_TST_RANGE     = false; // introduces consistency beetwen both IVPSNR methods, breaks compatibility

//===============================================================================================================================================================================================================

} //end of namespace PMBB
