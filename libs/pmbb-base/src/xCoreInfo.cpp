/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xCoreInfo.h"
#include "xMemory.h"

namespace PMBB_BASE {

//===============================================================================================================================================================================================================

std::string xCoreInfo::format() const
{
  std::string Result; Result.reserve(xMemory::getBestEffortSizePageBase());
  Result += fmt::format("Log={:<2d} Core={:<2d} Tier={:<3d} LLC={:<2d} NUMA={} ", m_Logical, m_Core, m_Tier, m_LLC, m_NUMA);
#if defined(X_PMBB_OPERATING_SYSTEM_WINDOWS)
  Result += fmt::format("W_SetID={:<3d} W_Group={} ", m_SetId, m_Group);
#endif
#if defined(X_PMBB_OPERATING_SYSTEM_LINUX)
  Result += fmt::format("L_Package={:d} L_Die={:d} L_Cluster={:d} ", m_Package, m_Die, m_Cluster);
  if(m_PerfNom != NotValid) { Result += fmt::format("L_PerfNom={:d} ", m_PerfNom); }
  if(m_PerfRef != NotValid) { Result += fmt::format("L_PerfRef={:d} ", m_PerfRef); }
  if(m_PerfMin != NotValid) { Result += fmt::format("L_PerfMin={:d} ", m_PerfMin); }
  if(m_PerfMax != NotValid) { Result += fmt::format("L_PerfMax={:d} ", m_PerfMax); }
  if(!m_Name.empty()      ) { Result += fmt::format("L_Name={} "     , m_Name   ); }
#endif
  return Result;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
