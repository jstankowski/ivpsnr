/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefBASE.h"

namespace PMBB_BASE {

//===============================================================================================================================================================================================================

class xCoreInfo
{
public:
  static constexpr int32 NotValid = NOT_VALID;
  using tStr = std::string;

protected:
  int32 m_Logical = NotValid; //win - group relative
  int32 m_Core    = NotValid; //win - group relative
  int32 m_Tier    = NotValid; //win - group relative, Performance tier
  int32 m_LLC     = NotValid; //win - group relative
  int32 m_NUMA    = NotValid; //win - group relative
#if defined(X_PMBB_OPERATING_SYSTEM_WINDOWS)
  int32 m_SetId   = NotValid; 
  int32 m_Group   = NotValid; 
#endif //X_PMBB_OPERATING_SYSTEM_WINDOWS
#if defined(X_PMBB_OPERATING_SYSTEM_LINUX)
  //[ROOT][PACKAGE][DIEGRP][DIE][TILE][MODULE][CORE][THREAD]
  int32 m_Package = NotValid; //pratform specific
  int32 m_Die     = NotValid; //pratform specific
  int32 m_Cluster = NotValid; //pratform specific
  int32 m_PerfNom = NotValid; //requires ACPI CPPC data
  int32 m_PerfRef = NotValid; //requires ACPI CPPC data
  int32 m_PerfMin = NotValid; //requires ACPI CPPC data
  int32 m_PerfMax = NotValid; //requires ACPI CPPC data
  tStr  m_Name              ; //available on Intel "hybrid" cpus only
#endif //X_PMBB_OPERATING_SYSTEM_LINUX

public:
    int32 getLogical(             ) const { return m_Logical   ; }
    void  setLogical(int32 Logical)       { m_Logical = Logical; }
    int32 getCore   (             ) const { return m_Core      ; }
    void  setCore   (int32 Core   )       { m_Core = Core      ; }
    int32 getTier   (             ) const { return m_Tier      ; }
    void  setTier   (int32 Tier   )       { m_Tier = Tier      ; }
    int32 getLLC    (             ) const { return m_LLC       ; }
    void  setLLC    (int32 LLC    )       { m_LLC = LLC        ; }
    int32 getNUMA   (             ) const { return m_NUMA      ; }
    void  setNUMA   (int32 NUMA   )       { m_NUMA = NUMA      ; }
#if defined(X_PMBB_OPERATING_SYSTEM_WINDOWS)
    int32 getSetId  (             ) const { return m_SetId     ; }
    void  setSetId  (int32 SetId  )       { m_SetId = SetId    ; }
    int32 getGroup  (             ) const { return m_Group     ; }
    void  setGroup  (int32 Group  )       { m_Group = Group    ; }
#endif
#if defined(X_PMBB_OPERATING_SYSTEM_LINUX)
    int32 getPackage(             ) const { return m_Package   ; }
    void  setPackage(int32 Package)       { m_Package = Package; }
    int32 getDie    (             ) const { return m_Die       ; }
    void  setDie    (int32 Die    )       { m_Die = Die        ; }
    int32 getCluster(             ) const { return m_Cluster   ; }
    void  setCluster(int32 Cluster)       { m_Cluster = Cluster; }
    int32 getPerfNom(             ) const { return m_PerfNom   ; }
    void  setPerfNom(int32 PerfNom)       { m_PerfNom = PerfNom; }
    int32 getPerfRef(             ) const { return m_PerfRef   ; }
    void  setPerfRef(int32 PerfRef)       { m_PerfRef = PerfRef; }
    int32 getPerfMin(             ) const { return m_PerfMin   ; }
    void  setPerfMin(int32 PerfMin)       { m_PerfMin = PerfMin; }
    int32 getPerfMax(             ) const { return m_PerfMax   ; }
    void  setPerfMax(int32 PerfMax)       { m_PerfMax = PerfMax; }
    tStr  getName   (             ) const { return m_Name      ; }
    void  setName   (tStr Name    )       { m_Name = Name      ; }
#endif

  std::string format() const;
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
