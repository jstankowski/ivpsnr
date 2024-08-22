/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: If you use this software, please cite the following paper: A. Dziembowski, D. Mieloch, J. Stankowski and A. Grzelka, "IV-PSNR—The Objective Quality Metric for Immersive Video Applications," in IEEE Transactions on Circuits and Systems for Video Technology, vol. 32, no. 11, pp. 7575-7591, Nov. 2022, doi: 10.1109/TCSVT.2022.3179575.
    SPDX-License-Identifier: BSD-3-Clause
*/

//===============================================================================================================================================================================================================

#include "xAppQMIV.h"

#include "xFile.h"
#include "xSeq.h"
#include "xIVPSNR.h"
#include "xSSIM.h"
#include "xCfgINI.h"
#include "xFmtScn.h"
#include "xMemory.h"
#include "xMiscUtilsCORE.h"
#include "xMathUtils.h"
#include <math.h>
#include <fstream>
#include <time.h>
#include <limits>
#include <numeric>
#include <cassert>
#include <thread>
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
  fmt::print("{}\n", xAppQMIV::c_BannerString);
  tTimePoint AppBeg = tClock::now();
  xAppQMIV AppQMIV;

  //===================================================================================================================
  //configuring
  //===================================================================================================================

  //parsing configuration
  AppQMIV.registerCmdParams();
  bool CfgLoadResult = AppQMIV.loadConfiguration(argc, const_cast<const char**>(argv));
  if(!CfgLoadResult) { xCfgINI::printError(AppQMIV.getErrorLog() + "\n\n", xAppQMIV::c_HelpString); return EXIT_FAILURE; }
  bool CfgReadResult = AppQMIV.readConfiguration();
  if(!CfgReadResult) { xCfgINI::printError(AppQMIV.getErrorLog() + "\n\n", xAppQMIV::c_HelpString); return EXIT_FAILURE; }
  const int32 VerboseLevel = AppQMIV.getVerboseLevel();

  if(VerboseLevel >= 2)
  { 
    fmt::print("WorkingDir = " + std::filesystem::current_path().string() + "\n\n");    
    fmt::print("Commandline args:\n"); xCfgINI::printCommandlineArgs(argc, const_cast<const char**>(argv));
  }

  //print compile time setup
  if (VerboseLevel >= 1)
  {
    fmt::print(xMiscUtilsCORE::formatCompileTimeSetup());
    fmt::print("USE_RUNTIME_CMPWEIGHTS = {}\n", xc_USE_RUNTIME_CMPWEIGHTS);
    fmt::print("\n");
  }

  //print config
  if(VerboseLevel >= 1) { fmt::print("{}\n", AppQMIV.formatConfiguration()); }

  //validate file names against input parameters
  eRes ValidFilesRes = AppQMIV.validateInputFiles();
  if(ValidFilesRes == eRes::Warning) { xCfgINI::printError(std::string("PARAMETERS WARNING: Invalid parameters\n") + AppQMIV.getErrorLog()); }
  if(ValidFilesRes == eRes::Error  ) { xCfgINI::printError(std::string("PARAMETERS WARNING: Invalid parameters\n") + AppQMIV.getErrorLog()); return EXIT_FAILURE; }

  //print configuration warnings
  std::string ConfigWarnings = AppQMIV.formatWarnings();
  if(!ConfigWarnings.empty()) { fmt::print(ConfigWarnings); }

  //hardware concurency
  AppQMIV.setupMultithreading();
  if(VerboseLevel >= 1) { fmt::print(AppQMIV.formatMultithreading() + "\n"); }

  //spacer
  fmt::print("\n\n\n");


  //===================================================================================================================
  // preparation
  //===================================================================================================================
  if(VerboseLevel >= 2) { fmt::print("Initializing:\n"); }

  eRes SeqRes = AppQMIV.setupSeqAndBuffs();
  if(SeqRes == eRes::Error) { return EXIT_FAILURE; }


  AppQMIV.createProcessors();

  //===================================================================================================================
  //running
  //===================================================================================================================
  tTimePoint PrcBeg = tClock::now();
  eRes ClcRes = AppQMIV.processAllFrames();
  if(ClcRes == eRes::Error) { return EXIT_FAILURE; }
  tTimePoint PrcEnd = tClock::now();

  //===================================================================================================================
  //finalizing
  //===================================================================================================================
  if(VerboseLevel >= 1) { fmt::print("\n"); fmt::print(AppQMIV.calibrateTimeStamp()); }
  fmt::print("\n\n");
  AppQMIV.combineFrameStats  ();
  AppQMIV.ceaseSeqAndBuffs   ();
  AppQMIV.ceaseMultithreading();

  //output file
  if(!AppQMIV.m_ResultFile.empty())
  {
    std::ofstream ResultStream(AppQMIV.m_ResultFile, std::ios::app);
    ResultStream << AppQMIV.formatResultsFile();
    ResultStream.close();
  }

  //printout results
  fmt::print(AppQMIV.formatResultsStdOut());
  fmt::print("\n");
  tTimePoint AppEnd = tClock::now();
  fmt::print("TotalProcessingTime  = {:.3f} s\n", std::chrono::duration_cast<tDurationS>(PrcEnd - PrcBeg).count());
  fmt::print("TotalApplicationTime = {:.3f} s\n", std::chrono::duration_cast<tDurationS>(AppEnd - AppBeg).count());
  fmt::print("END-OF-LOG\n");
  fflush(stdout);

  return EXIT_SUCCESS;
}

//===============================================================================================================================================================================================================
