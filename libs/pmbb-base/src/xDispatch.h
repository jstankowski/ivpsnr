/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "xCfgINI.h"
#include "xErrMsg.h"
#include "xDispatchUtils.h"

//===============================================================================================================================================================================================================
// x86-64
//===============================================================================================================================================================================================================

#if defined(X_PMBB_ARCH_AMD64)

static const std::string_view HeadString = "PMBB runtime dispatch module for x86-64 Microarchitecture Feature Levels";

static const std::string_view HelpString =
R"PMBBRAWSTRING(
=============================================================================
PMBB runtime dispatch module for x86-64 Microarchitecture Feature Levels

Usage:

 Cmd                | Description
 --DispatchForce      Force dispatcher to selected microarchitecture (optional, default=UNDEFINED)
 --DispatchVerbose    Verbose level for runtime dispatch module      (optional, default=0)
=============================================================================
)PMBBRAWSTRING";

#endif //X_PMBB_ARCH_AMD64

//===============================================================================================================================================================================================================
// Common
//===============================================================================================================================================================================================================

int main(int argc, char* argv[], char* envp[])
{
  using eMFL = PMBB_BASE::xProcInfo::eMFL;

  // header
  fmt::print("{}\n-----------------------------------------------------------------------------\n", HeadString); std::cout.flush();

  // parsing configuration
  PMBB_BASE::xCfgINI::xParser* CfgParser = new PMBB_BASE::xCfgINI::xParser;
  CfgParser->addCmdParm("", "DispatchForce"  , "", "DispatchForce"  );
  CfgParser->addCmdParm("", "DispatchVerbose", "", "DispatchVerbose");
  CfgParser->setUnknownCmdParams(true);
  CfgParser->setEmptyCmdParams  (true);
  bool CommandlineResult = CfgParser->loadFromCmdln(argc, const_cast<const char**>(argv));
  if(!CommandlineResult) { PMBB_BASE::xErrMsg::printError(std::string("! invalid commandline\n") + CfgParser->getParsingLog() + "\n\n", HelpString); return EXIT_FAILURE; }

  std::string LevelOverrideS = CfgParser->getParam1stArg("DispatchForce", PMBB_BASE::xProcInfo::xMflToStr(eMFL::UNDEFINED));
  eMFL        LevelOverride  = PMBB_BASE::xProcInfo::xStrToMfl(LevelOverrideS);
  if(LevelOverride != eMFL::UNDEFINED) { fmt::print("UserOverrided = {}\n", PMBB_BASE::xProcInfo::xMflToStr(LevelOverride)); }
  bool        Verbose        = CfgParser->getParam1stArg("DispatchVerbose", false);

  // examining system
  PMBB_BASE::xProcInfo ProcInfo;
  ProcInfo.detectSysInfo();
  if(Verbose) { fmt::print("{}\n", ProcInfo.formatSysInfo()); }

  //configure dispatch
  xDispatcher Dispatcher;
  Dispatcher.init(ProcInfo);

  eMFL LevelDetected = ProcInfo.determineMicroArchFeatureLevel();
  fmt::print("DetectedHardware = {}\n", PMBB_BASE::xProcInfo::xMflToStr(LevelDetected));
  if(LevelDetected == eMFL::UNDEFINED) { fmt::print("Errrrrrrrror!!!\n"); return EXIT_FAILURE; }
   
  // examine software
  std::vector<eMFL> LevelSoftware = Dispatcher.getSoftwareLevels();
  fmt::print("DetectedSoftware = ");
  for(eMFL l : LevelSoftware) { fmt::print("{} ", PMBB_BASE::xProcInfo::xMflToStr(l)); }
  fmt::print("\n"); 
  if(Verbose) { fmt::print("{}\n\n", Dispatcher.formatSoftwareInfo()); }
  std::cout.flush();

  // select highest available and compatible level
  eMFL LevelSelected = eMFL::UNDEFINED;
  for(eMFL l : LevelSoftware) { if((int32_t)LevelDetected <= (int32_t)l) { LevelSelected = LevelDetected; break; } }
  if(LevelSelected == eMFL::UNDEFINED) { fmt::print("Errrrrrrrror!!! (LevelSelected == eMFL::UNDEFINED)\n"); return EXIT_FAILURE; }

  // optional override
  if(LevelOverride != eMFL::UNDEFINED && std::find(LevelSoftware.begin(), LevelSoftware.end(), LevelOverride) != std::end(LevelSoftware)) { LevelSelected = LevelOverride; }

  // dispatch
  xDispatcher::tMainPtr MainPtr = Dispatcher.getSoftwareEntryPoint(LevelSelected);
  fmt::print("Dispatching {} ----> {}", PMBB_BASE::xProcInfo::xMflToStr(LevelSelected), xDispatcher::MflToMainName(LevelSelected));
  if(Verbose) { fmt::print(" @ {:p}", (void*)MainPtr); }
  fmt::print("\n-----------------------------------------------------------------------------\n\n"); std::cout.flush();
  int RetVal = MainPtr(argc, argv, envp);
  return RetVal;
}

//===============================================================================================================================================================================================================

