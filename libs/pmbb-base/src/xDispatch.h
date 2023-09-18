/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xProcInfo.h"
#include "fmt/printf.h"

//===============================================================================================================================================================================================================

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

//===============================================================================================================================================================================================================

int main(int argc, char* argv[], char* envp[])
{
  fmt::printf("PMBB x86-64 Microarchitecture Feature Level runtime dispatch module\n");
  fmt::printf("------------------------------------------------------------------------------\n");
  PMBB_BASE::xProcInfo ProcInfo;
  ProcInfo.detectSysInfo();
  //ProcInfo.printSysInfo ();

  int32_t Level = ProcInfo.determineMicroArchFeatureLevel();
  fmt::printf("Detected x86-64 Microarchitecture Feature Level = ");
  switch(Level)
  {
    case 1: fmt::printf("x86-64"   ); break;
    case 2: fmt::printf("x86-64-v2"); break;
    case 3: fmt::printf("x86-64-v3"); break;
    case 4: fmt::printf("x86-64-v4"); break;
  }
  fmt::printf("\n");
  if(Level < 1) { fmt::print("Errrrrrrrr!!!\n"); return EXIT_FAILURE; }

  fmt::printf("Software build with levels = ");
  if constexpr (c_BUILD_WITH_AMD64v1) { fmt::printf("x86-64 "   ); }
  if constexpr (c_BUILD_WITH_AMD64v2) { fmt::printf("x86-64-v2 "); }
  if constexpr (c_BUILD_WITH_AMD64v3) { fmt::printf("x86-64-v3 "); }
  if constexpr (c_BUILD_WITH_AMD64v4) { fmt::printf("x86-64-v4 "); }
  fmt::printf("\n");  

  if constexpr (c_BUILD_WITH_AMD64v4) { if(Level == 4) { fmt::print("Dispatching x86-64-v4 ----> main_AMD64v4\n\n\n\n"); return main_AMD64v4(argc, argv, envp); } }
  if constexpr (c_BUILD_WITH_AMD64v3) { if(Level == 3) { fmt::print("Dispatching x86-64-v3 ----> main_AMD64v3\n\n\n\n"); return main_AMD64v3(argc, argv, envp); } }
  if constexpr (c_BUILD_WITH_AMD64v2) { if(Level == 2) { fmt::print("Dispatching x86-64-v2 ----> main_AMD64v2\n\n\n\n"); return main_AMD64v2(argc, argv, envp); } }
  if constexpr (c_BUILD_WITH_AMD64v1) { if(Level == 1) { fmt::print("Dispatching x86-64    ----> main_AMD64v1\n\n\n\n"); return main_AMD64v1(argc, argv, envp); } }

  return EXIT_FAILURE;
}

//===============================================================================================================================================================================================================
