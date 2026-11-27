/*
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

//===============================================================================================================================================================================================================

#include "xAppQMIVGSR.h"
#include "xFile.h"
#include "xErrMsg.h"
#include "xFmtScn.h"
#include "xMemory.h"
#include "xMiscUtilsBASE.h"
#include "xMiscUtilsCORE.h"
#include "xTimeUtils.h"
#include <filesystem>
#include "fmt/chrono.h"

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================
// Main
//===============================================================================================================================================================================================================
#ifndef APP_MAIN
#define APP_MAIN main
#endif

int32 APP_MAIN(int argc, char *argv[], char* /*envp*/[])
{
  fmt::print("{}\n", xAppQMIVGSR::c_BannerString);
  tTimePoint AppBeg = tClock::now();
  xAppQMIVGSR App;

  //===================================================================================================================
  //configuring
  //===================================================================================================================

  //parsing configuration
  App.registerCmdParams();
  bool CfgLoadResult = App.loadConfiguration(argc, const_cast<const char**>(argv));
  if(!CfgLoadResult) { xErrMsg::printError(App.getErrorLog() + "\n\n", App.formatHelp()); return EXIT_FAILURE; }
  bool CfgReadResult = App.readConfiguration();
  if(!CfgReadResult) { xErrMsg::printError(App.getErrorLog() + "\n\n", App.formatHelp()); return EXIT_FAILURE; }
  const int32 VerboseLevel = App.getVerboseLevel();

  if(VerboseLevel >= 2)
  {
    fmt::print("WorkingDir = {}\n\n", std::filesystem::current_path().string());
    fmt::print("Commandline args:\n"); xCfgINI::printCommandlineArgs(argc, const_cast<const char**>(argv));
  }

  //print compile time setup
  if(VerboseLevel >= 1)
  {
    fmt::print("{}", xMiscUtilsBASE::formatCompileTimeSetup());
    fmt::print("{}", xMiscUtilsCORE::formatCompileTimeSetup());
    fmt::print("\n");
  }

  if(VerboseLevel >= 2)
  {
    fmt::print("{}\n", xMiscUtilsBASE::formatBuildInfo());
  }

  //print config
  if(VerboseLevel >= 1) { fmt::print("{}\n", App.formatConfiguration()); }

  //validate file names against input parameters
  eAppRes ValidFilesRes = App.validateInputFiles();
  if(ValidFilesRes == eAppRes::Warning) { xErrMsg::printError(std::string("PARAMETERS WARNING: Invalid parameters\n") + App.getErrorLog()); }
  if(ValidFilesRes == eAppRes::Error  ) { xErrMsg::printError(std::string("PARAMETERS WARNING: Invalid parameters\n") + App.getErrorLog()); return EXIT_FAILURE; }

  //print configuration warnings
  std::string ConfigWarnings = App.formatWarnings();
  if(!ConfigWarnings.empty()) { fmt::print("{}", ConfigWarnings); }

  //hardware concurency
  App.setupMultithreading();
  if(VerboseLevel >= 1) { fmt::print("{}\n", App.formatMultithreading()); }

  //spacer
  fmt::print("\n\n");


  //===================================================================================================================
  // preparation
  //===================================================================================================================
  if(VerboseLevel >= 2) { fmt::print("Initializing:\n"); }

  eAppRes SeqRes = App.setupSeqs (); if(SeqRes == eAppRes::Error) { return EXIT_FAILURE; }
  eAppRes BufRes = App.setupBuffs(); if(BufRes == eAppRes::Error) { return EXIT_FAILURE; }

  App.createRendererProcessor();


  //===================================================================================================================
  //running
  //===================================================================================================================
  tTimePoint PrcBeg = tClock::now();
  eAppRes ClcRes = App.processAllFrames();
  if(ClcRes == eAppRes::Error) { return EXIT_FAILURE; }
  tTimePoint PrcEnd = tClock::now();

  //===================================================================================================================
  //finalizing
  //===================================================================================================================
  if(VerboseLevel >= 1) { fmt::print("\n"); fmt::print("{}", App.calibrateTimeStamp()); }
  fmt::print("\n\n");

  //printout results
  fmt::print("{}", App.formatResultsStdOut());
  fmt::print("\n");

  App.ceaseSeqs               ();
  App.ceaseBuffs              ();
  App.destroyRendererProcessor();
  App.ceaseMultithreading     ();

  tTimePoint AppEnd = tClock::now();
  fmt::print("TotalProcessingTime  = {:.3f} s\n", std::chrono::duration_cast<tDurationS>(PrcEnd - PrcBeg).count());
  fmt::print("TotalApplicationTime = {:.3f} s\n", std::chrono::duration_cast<tDurationS>(AppEnd - AppBeg).count());
  fmt::print("END-OF-LOG\n");
  fflush(stdout);

  return EXIT_SUCCESS;
}

//===============================================================================================================================================================================================================
