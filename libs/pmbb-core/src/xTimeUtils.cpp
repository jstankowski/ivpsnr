/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xTimeUtils.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xTimeStamp::calibrateTimeStamp()
{
  tDuration TotalProcTime  = m_ProcEndTime  - m_ProcBegTime ;
  flt64     TotalProcSec   = (flt64)(std::chrono::duration_cast<tDurationS>(TotalProcTime).count());
  uint64    TotalProcTicks = m_ProcEndTicks - m_ProcBegTicks;  
  flt64     TicksPerSec    = (flt64)TotalProcTicks / (flt64)TotalProcSec;

  setupCalibration(TicksPerSec);
}
void xTimeStamp::setupCalibration(flt64 TicksPerSec)
{
  m_TicksPerSec      = TicksPerSec          ;
  m_TicksPerMiliSec  = TicksPerSec / 1000   ;
  m_TicksPerMicroSec = TicksPerSec / 1000000;
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