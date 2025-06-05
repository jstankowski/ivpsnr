/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xProcInfo.h"
#include "xString.h"
#include "fmt/printf.h"
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>

//===============================================================================================================================================================================================================
// xDispatcher
//===============================================================================================================================================================================================================
class xDispatcher
{
public:
  using eMFL     = PMBB_BASE::xProcInfo::eMFL;
  using tMFLV    = PMBB_BASE::xProcInfo::tMFLV;
  using tMainPtr = int(*)(int, char* [], char* []);
  using tMPM     = std::map<eMFL, tMainPtr>;

  static std::string MflToMainName(eMFL Mfl)
  {
    if(Mfl == eMFL::UNDEFINED) { return "MotImplemented"; }
    return fmt::format("main_{}", PMBB_BASE::xProcInfo::xMflToStr(Mfl));
  }

protected:
  tMFLV m_HardwareLevels;
  tMFLV m_SoftwareLevels;
  tMPM  m_SoftwareEntryPoints;

public:
  void init(const PMBB_BASE::xProcInfo& ProcInfo)
  {
    m_HardwareLevels = ProcInfo.determineMicroArchFeatureLevels();
    xDetermineSoftwareAvailable();
  }

  std::string formatDetected() const
  {
    std::string Result; Result.reserve(4096);
    Result += "DetectedHardwareMFL = ";
    for(eMFL l : m_HardwareLevels) { Result += fmt::format("{} ", PMBB_BASE::xProcInfo::xMflToStr(l)); }
    Result += "\n";    
    Result += "DetectedSoftwareMFL = ";
    for(auto l : m_SoftwareLevels) { Result += fmt::format("{} ", PMBB_BASE::xProcInfo::xMflToStr(l)); }
    return Result;
  }
  
  std::string formatSoftwareInfo() const
  {
    std::string Result; Result.reserve(4096);
    Result += "SoftwareAvailable:\n";
    for(auto Itr = m_SoftwareLevels.crbegin(); Itr != m_SoftwareLevels.crend(); Itr++)
    {
      Result += fmt::format("  main_{:<15} @ {:p}    [{}]\n", PMBB_BASE::xProcInfo::xMflToStr(*Itr), (void*)(m_SoftwareEntryPoints.at(*Itr)), PMBB_BASE::xProcInfo::xMflToDescription(*Itr));
    }
    return Result;
  }

  const tMFLV& getHardwareLevels() const { return m_HardwareLevels; }
  const tMFLV& getSoftwareLevels() const { return m_SoftwareLevels; }

  tMainPtr getSoftwareEntryPoint(eMFL MFL) const { return m_SoftwareEntryPoints.at(MFL); }

protected:
  void xDetermineSoftwareAvailable();
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// xDispatcher - x86-64 (AMD64)
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#if defined(X_PMBB_ARCH_AMD64)

#if defined(BUILD_WITH_AMD64v1)
int main_AMD64v1(int argc, char* argv[], char* envp[]);
static constexpr bool c_BUILD_WITH_AMD64v1 = true;
#else
int main_AMD64v1(int /*argc*/, char* /*argv*/[], char* /*envp*/[]) { return EXIT_FAILURE; }
static constexpr bool c_BUILD_WITH_AMD64v1 = false;
#endif

#if defined(BUILD_WITH_AMD64v2)
int main_AMD64v2(int argc, char* argv[], char* envp[]);
static constexpr bool c_BUILD_WITH_AMD64v2 = true;
#else
int main_AMD64v2(int /*argc*/, char* /*argv*/[], char* /*envp*/[]) { return EXIT_FAILURE; }
static constexpr bool c_BUILD_WITH_AMD64v2 = false;
#endif

#if defined(BUILD_WITH_AMD64v3)
int main_AMD64v3(int argc, char* argv[], char* envp[]);
static constexpr bool c_BUILD_WITH_AMD64v3 = true;
#else
int main_AMD64v3(int /*argc*/, char* /*argv*/[], char* /*envp*/[]) { return EXIT_FAILURE; }
static constexpr bool c_BUILD_WITH_AMD64v3 = false;
#endif

#if defined(BUILD_WITH_AMD64v4)
int main_AMD64v4(int argc, char* argv[], char* envp[]);
static constexpr bool c_BUILD_WITH_AMD64v4 = true;
#else
int main_AMD64v4(int /*argc*/, char* /*argv*/[], char* /*envp*/[]) { return EXIT_FAILURE; }
static constexpr bool c_BUILD_WITH_AMD64v4 = false;
#endif

void xDispatcher::xDetermineSoftwareAvailable()
{
  m_SoftwareLevels.clear(); m_SoftwareEntryPoints.clear();
  if constexpr(c_BUILD_WITH_AMD64v4) { m_SoftwareLevels.push_back(eMFL::AMD64v4); m_SoftwareEntryPoints.emplace(eMFL::AMD64v4, main_AMD64v4); }
  if constexpr(c_BUILD_WITH_AMD64v3) { m_SoftwareLevels.push_back(eMFL::AMD64v3); m_SoftwareEntryPoints.emplace(eMFL::AMD64v3, main_AMD64v3); }
  if constexpr(c_BUILD_WITH_AMD64v2) { m_SoftwareLevels.push_back(eMFL::AMD64v2); m_SoftwareEntryPoints.emplace(eMFL::AMD64v2, main_AMD64v2); }
  if constexpr(c_BUILD_WITH_AMD64v1) { m_SoftwareLevels.push_back(eMFL::AMD64v1); m_SoftwareEntryPoints.emplace(eMFL::AMD64v1, main_AMD64v1); }
}

#endif //X_PMBB_ARCH_AMD64

//===============================================================================================================================================================================================================
