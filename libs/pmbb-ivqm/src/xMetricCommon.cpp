/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xMetricCommon.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xMetricCommon::initRowBuffers(int32 Height)
{
  for(int32 CmpIdx = 0; CmpIdx < 4; CmpIdx++) { m_RowDistortions[CmpIdx].resize(Height); }
  for(int32 CmpIdx = 0; CmpIdx < 4; CmpIdx++) { m_RowErrors     [CmpIdx].resize(Height); }

  m_RowDistsV4 .resize(Height);
  m_RowErrorsV4.resize(Height);
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
