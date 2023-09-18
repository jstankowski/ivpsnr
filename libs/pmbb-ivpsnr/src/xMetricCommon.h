/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefIVPSNR.h"
#include "xThreadPool.h"
#include "xMathUtils.h"
#include <numeric>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xMetricCommon
{
public:
  static constexpr bool  c_UseKBNS       = xc_USE_KBNS;
  static constexpr int32 c_NumComponents = 3;
  using tThPI = xThreadPoolInterface;

protected:
  int32 m_NumComponents = c_NumComponents;
  tThPI m_ThreadPoolIf;

public:
  void  initThreadPool  (xThreadPool* ThreadPool, int32 Height) { if(ThreadPool) { m_ThreadPoolIf.init(ThreadPool, Height); } }
  void  uninitThreadPool(                                     ) { m_ThreadPoolIf.uininit(); }

  static inline flt64 Accumulate(std::vector<flt64>& Data)
  {
    if constexpr(c_UseKBNS) { return xMathUtils::KahanBabuskaNeumaierSumation(Data.data(), (int32)Data.size()); }
    else                    { return std::accumulate(Data.begin(), Data.end(), (flt64)0); }
  }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB