/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefIVQM.h"
#include "xVec.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xMetricCommon : public xMultiThreaded
{
public:
  static constexpr int32 c_NumComponents = 3;

protected:
  int32 m_NumComponents = c_NumComponents;

  std::vector<uint64> m_RowDistortions[4];
  std::vector<flt64 > m_RowErrors     [4];

  std::vector<uint64V4> m_RowDistsV4;
  std::vector<flt64V4 > m_RowErrorsV4;

public:
  void  initRowBuffers  (int32 Height);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB