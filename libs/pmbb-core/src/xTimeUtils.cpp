/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xTimeUtils.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xTimeStamp::calibrateTimeStamp()
{
  tDuration TotalProcTime  = m_ProcEndTime  - m_ProcBegTime ;
  uint64    TotalProcTicks = m_ProcEndTicks - m_ProcBegTicks;

  m_TicksPerMicroSec = (flt64)TotalProcTicks / std::chrono::duration_cast<tDurationUS>(TotalProcTime).count();
  m_TicksPerMiliSec  = (flt64)TotalProcTicks / std::chrono::duration_cast<tDurationMS>(TotalProcTime).count();
  m_TicksPerSec      = (flt64)TotalProcTicks / std::chrono::duration_cast<tDurationS >(TotalProcTime).count();
}
std::string xTimeStamp::formatCalibration() const 
{ 
  return fmt::format("CalibratedTicksPerSec = {:.0f} ({:.3f}MHz)\n", m_TicksPerSec, m_TicksPerMicroSec);
}
flt64 xTimeStamp::calibrateTicksPerSec(tDurationMS Miliseconds)
{
  xTimeStamp TS;
  TS.sampleBeg();
  std::this_thread::sleep_for(Miliseconds);
  TS.sampleEnd();
  TS.calibrateTimeStamp();
  return TS.getTicksPerSec();
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB