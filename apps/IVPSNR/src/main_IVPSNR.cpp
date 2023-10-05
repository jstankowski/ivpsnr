/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: If you use this software, please cite the following paper: A. Dziembowski, D. Mieloch, J. Stankowski and A. Grzelka, "IV-PSNR—The Objective Quality Metric for Immersive Video Applications," in IEEE Transactions on Circuits and Systems for Video Technology, vol. 32, no. 11, pp. 7575-7591, Nov. 2022, doi: 10.1109/TCSVT.2022.3179575.
    SPDX-License-Identifier: BSD-3-Clause
*/

//===============================================================================================================================================================================================================

#include "xFile.h"
#include "xSeq.h"
#include "xIVPSNR.h"
#include "xCfgINI.h"
#include "xFmtScn.h"
#include "xMemory.h"
#include "xMiscUtilsPMBB-CORE.h"
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

static const std::string_view BannerString =
R"PMBBRAWSTRING(
=============================================================================

IV-PSNR software v5.0

Copyright (c) 2020-2023, Jakub Stankowski & Adrian Dziembowski, All rights reserved.

Developed at Poznan University of Technology, Poznan, Poland
Authors: Jakub Stankowski, Adrian Dziembowski

If you use this software, please cite the following paper:
A. Dziembowski, D. Mieloch, J. Stankowski and A. Grzelka, "IV-PSNR - The Objective Quality Metric for Immersive Video Applications," in IEEE Transactions on Circuits and Systems for Video Technology, vol. 32, no. 11, pp. 7575-7591, Nov. 2022, doi: 10.1109/TCSVT.2022.3179575.
https://doi.org/10.1109/TCSVT.2022.3179575

=============================================================================

)PMBBRAWSTRING";

static const std::string_view HelpString =
R"PMBBRAWSTRING(
=============================================================================
IV-PSNR software v5.0

Usage:

 Cmd | ParamName        | Description
 -i0   InputFile0         YUV File path - input sequence 0
 -i1   InputFile1         YUV File path - input sequence 1
 -w    PictureWidth       Width of SeqIn
 -h    PictureHeight      Height of SeqIn
 -bd   BitDepth           Bit depth     (optional, default 8, up to 14) 
 -cf   ChromaFormat       Chroma format (optional, default 420) [420, 422, 444]
 -s0   StartFrame0        Start frame   (optional, default 0) 
 -s1   StartFrame1        Start frame   (optional, default 0) 
 -l    NumberOfFrames     Number of frames to be processed 
                          (optional, default -1=all)

 -r    ResultFile         Output file path for printing result(s) (optional)

 -im   InputFileM         YUV file path - mask (optional)
 -bdm  BitDepthM          Bit depth for mask     (optional, default=BitDepth, up to 16)
 -cfm  ChromaFormatM      Chroma format for mask (optional, default=ChromaFormat) [400, 420, 422, 444]

 -erp  Equirectangular    Equirectangular SeqIn (flag, default disabled)
 -lor  LonRangeDeg        Range for ERP SeqIn in degrees - Longitudinal
                          (optional, default 360)
 -lar  LatRangeDeg        Range for ERP SeqIn in degrees - Lateral
                          (optional, default 180)

 -sr   SearchRange        IV-PSNR search range around center point
                          (optional, default 2=5x5)
 -cws  ComponentWeights   IV-PSNR component weights
                          ("Lm:Cb:Cr:0" - per component integer weight, default "4:1:1:0")
                          quotes are required
 -unc  UnnoticeableCoef   IV-PSNR unnoticable color difference threshold coeff
                          ("Lm:Cb:Cr:0" - per component coeff, default "0.01:0.01:0.01:0")
                          quotes are required
 -ws8  Peak8bitWSPSNR     Use 1020 as peak value for 10-bps videos in WSPSNR metric
                          (provides compatibility with original WSPSNR implementation,
                          optional, default=0)

 -t    NumberOfThreads    Number of worker threads
                          (optional, default -1=all, suggested 4-8)
 -ilp  InterleavedPic     Use additional image buffer with interleaved layout for IV-PSNR 
                          (improves performance at a cost of increased memory usage
                          optional, default=1)
 -ipa  InvalidPelActn     Select action taken if invalid pixel value is detected 
                          (optional, default STOP) [SKIP - disable pixel value checking,
                          WARN - print warning and ignore, STOP - stop execution,
                          CNCL - try to conceal by clipping to bit depth range]
 -nma  NameMismatchActn   Select action taken if parameters derived from filename are different
                          than provided as input parameters. Checks resolution, bit depth
                          and chroma format. (optional, default WARN) [SKIP - disable checking,
                          WARN - print warning and ignore, STOP - stop execution]
 -v    VerboseLevel       Verbose level (optional, default=1)

 -c    "config.cfg"       External config file - in INI format (optional)

VerboseLevel:
  0 = final PSNR, WSPSNR, IVPSNR values only
  1 = 0 + configuration + detected frame numbers
  2 = 1 + argc/argv + frame level PSNR, WSPSNR, IVPSNR
  3 = 2 + computing time (LOAD, PSNR, WSPSNR, IVPSNR)
          (uses high_resolution_clock, could slightly slow down computations)
  4 = 3 + IVPSNR specific debug data (GlobalColorShift, R2T+T2R)

Example - commandline parameters:
  IVPSNR -i0 "A.yuv" -i1 "B.yuv" -w 2048 -h 1088 -bd 10 -cf 420 -v 3 -r "r.txt"

Example - config file:
  InputFile0      = "A.yuv"
  InputFile1      = "B.yuv"
  PictureWidth    = 2048
  PictureHeight   = 1088
  BitDepth        = 10
  ChromaFormat    = 420
  VerboseLevel    = 3
  ResultFile      = "IVPSNR.txt"


=============================================================================
)PMBBRAWSTRING";

//===============================================================================================================================================================================================================

enum class eActn : int32
{
  INVALID = NOT_VALID,
  SKIP = 0, //SKIP - disable value checking,
  WARN = 1, //WARN - print warning and ignore
  STOP = 2, //STOP - stop execution
  CNCL = 3, //CNCL - try to conceal
};

static inline eActn xStrToActn(const std::string_view Actn)
{
  std::string IPA_U = xString::toUpper(Actn);
  return IPA_U == "SKIP" ? eActn::SKIP :
         IPA_U == "WARN" ? eActn::WARN :
         IPA_U == "STOP" ? eActn::STOP :
         IPA_U == "CNCL" ? eActn::CNCL :
                           eActn::INVALID;
}
static inline std::string xActnToString(eActn IPA)
{
  return IPA == eActn::SKIP ? "SKIP" :
         IPA == eActn::WARN ? "WARN" :
         IPA == eActn::STOP ? "STOP" :
         IPA == eActn::CNCL ? "CNCL" :
                              "INVALID";
}

//===============================================================================================================================================================================================================
// Main
//===============================================================================================================================================================================================================
#ifndef IVPSNR_MAIN
#define IVPSNR_MAIN main
#endif

int32 IVPSNR_MAIN(int argc, char *argv[], char* /*envp*/[])
{
  fmt::printf("%s\n", BannerString);

  //==============================================================================
  // parsing configuration
  xCfgINI::xParser CfgParser;
  CfgParser.addCmdParm("i0" , "InputFile0"      , "", "InputFile0"          );
  CfgParser.addCmdParm("i1" , "InputFile1"      , "", "InputFile1"          );  
  CfgParser.addCmdParm("w"  , "PictureWidth"    , "", "PictureWidth"        );
  CfgParser.addCmdParm("h"  , "PictureHeight"   , "", "PictureHeight"       );
  CfgParser.addCmdParm("bd" , "BitDepth"        , "", "BitDepth"            );
  CfgParser.addCmdParm("cf" , "ChromaFormat"    , "", "ChromaFormat"        );
  CfgParser.addCmdParm("s0" , "StartFrame0"     , "", "StartFrame0"         );
  CfgParser.addCmdParm("s1" , "StartFrame1"     , "", "StartFrame1"         );
  CfgParser.addCmdParm("l"  , "NumberOfFrames"  , "", "NumberOfFrames"      );
  CfgParser.addCmdParm("o0" , "OutputFile0"     , "", "OutputFile0"         );
  CfgParser.addCmdParm("o1" , "OutputFile1"     , "", "OutputFile1"         );  
  CfgParser.addCmdParm("r"  , "ResultFile"      , "", "ResultFile"          );
  CfgParser.addCmdParm("im" , "InputFileM"      , "", "InputFileM"          );
  CfgParser.addCmdParm("bdm", "BitDepthM"       , "", "BitDepthM"           );
  CfgParser.addCmdParm("cfm", "ChromaFormatM"   , "", "ChromaFormatM"       );
  CfgParser.addCmdFlag("erp", "Equirectangular" , "", "Equirectangular", "1");
  CfgParser.addCmdParm("lor", "LonRangeDeg"     , "", "LonRangeDeg"         );
  CfgParser.addCmdParm("lar", "LatRangeDeg"     , "", "LatRangeDeg"         );
  CfgParser.addCmdParm("sr" , "SearchRange"     , "", "SearchRange"         );
  CfgParser.addCmdParm("cws", "ComponentWeights", "", "ComponentWeights"    );
  CfgParser.addCmdParm("unc", "UnnoticeableCoef", "", "UnnoticeableCoef"    ); 
  CfgParser.addCmdParm("ws8", "Legacy8bitWSPSNR", "", "Legacy8bitWSPSNR"    );  
  CfgParser.addCmdParm("t"  , "NumberOfThreads" , "", "NumberOfThreads"     );
  CfgParser.addCmdParm("ilp", "InterleavedPic"  , "", "InterleavedPic"      );
  CfgParser.addCmdParm("ipa", "InvalidPelActn"  , "", "InvalidPelActn"      );
  CfgParser.addCmdParm("nma", "NameMismatchActn", "", "NameMismatchActn"    );
  CfgParser.addCmdParm("v"  , "VerboseLevel"    , "", "VerboseLevel"        );  

  bool CommandlineResult = CfgParser.loadFromCmdln(argc, argv);
  if(!CommandlineResult) { xCfgINI::printError(std::string("! invalid commandline\n") + CfgParser.getParsingLog() + "\n\n", HelpString); return EXIT_FAILURE; }

  //readed from commandline/config 
  constexpr int32 NumInputsMax = 3;
  constexpr int32 NumOutputMax = 2;

  std::string InputFile [NumInputsMax];
  std::string OutputFile[NumInputsMax];
  int32       StartFrame[2];
              InputFile[0]       = CfgParser.getParam1stArg("InputFile0"      , std::string(""));
              InputFile[1]       = CfgParser.getParam1stArg("InputFile1"      , std::string(""));  
  int32       PictureWidth       = CfgParser.getParam1stArg("PictureWidth"    , NOT_VALID      );
  int32       PictureHeight      = CfgParser.getParam1stArg("PictureHeight"   , NOT_VALID      );
  int32       BitDepth           = CfgParser.getParam1stArg("BitDepth"        , 8              );  
  int32       ChromaFormat       = CfgParser.getParam1stArg("ChromaFormat"    , 420            );
              StartFrame[0]      = CfgParser.getParam1stArg("StartFrame0"     , 0              );
              StartFrame[1]      = CfgParser.getParam1stArg("StartFrame1"     , 0              );
  int32       NumberOfFrames     = CfgParser.getParam1stArg("NumberOfFrames"  , -1             );  
              OutputFile[0]      = CfgParser.getParam1stArg("OutputFile0"     , std::string(""));
              OutputFile[1]      = CfgParser.getParam1stArg("OutputFile1"     , std::string(""));
  std::string ResultFile         = CfgParser.getParam1stArg("ResultFile"      , std::string(""));
              InputFile[2]       = CfgParser.getParam1stArg("InputFileM"      , std::string(""));
  int32       BitDepthM          = CfgParser.getParam1stArg("BitDepthM"       , BitDepth       );
  int32       ChromaFormatM      = CfgParser.getParam1stArg("ChromaFormatM"   , ChromaFormat   );              
  bool        IsEquirectangular  = CfgParser.getParam1stArg("Equirectangular" , false          );
  int32       LonRangeDeg        = CfgParser.getParam1stArg("LonRangeDeg"     , 360            );
  int32       LatRangeDeg        = CfgParser.getParam1stArg("LatRangeDeg"     , 180            );
  int32       SearchRange        = CfgParser.getParam1stArg("SearchRange"     , xIVPSNR::c_DefaultSearchRange);
  std::string ComponentWeightsS  = CfgParser.getParam1stArg("ComponentWeights", xFmtScn::formatIntWeights(xIVPSNR::c_DefaultCmpWeights));
  int32V4     ComponentWeights   = xFmtScn::scanIntWeights(ComponentWeightsS);
  std::string UnnoticeableCoefS  = CfgParser.getParam1stArg("UnnoticeableCoef", xFmtScn::formatFltWeights(xIVPSNR::c_DefaultUnntcbCoef));
  flt32V4     UnnoticeableCoef   = xFmtScn::scanFltWeights(UnnoticeableCoefS);
  bool        Legacy8bitWSPSNR   = CfgParser.getParam1stArg("Legacy8bitWSPSNR", false          );
  int32       NumberOfThreads    = CfgParser.getParam1stArg("NumberOfThreads" , NOT_VALID      );
  bool        InterleavedPic     = CfgParser.getParam1stArg("InterleavedPic"  , true           );
  std::string InvalidPelActnS    = CfgParser.getParam1stArg("InvalidPelActn"  , "STOP"         );
  eActn       InvalidPelActn     = xStrToActn(InvalidPelActnS);
  std::string NameMismatchActnS  = CfgParser.getParam1stArg("NameMismatchActn", "WARN"         );
  eActn       NameMismatchActn   = xStrToActn(NameMismatchActnS);
  bool        Calc__PSNR         = CfgParser.getParam1stArg("Calc__PSNR"      , true           );
  bool        CalcWSPSNR         = CfgParser.getParam1stArg("CalcWSPSNR"      , true           );
  bool        CalcIVPSNR         = CfgParser.getParam1stArg("CalcIVPSNR"      , true           );
  int32       VerboseLevel       = CfgParser.getParam1stArg("VerboseLevel"    , 1              );

  if(VerboseLevel >= 2)
  { 
    fmt::printf("WorkingDir = " + std::filesystem::current_path().string() + "\n");    
    fmt::printf("Commandline args:\n"); xCfgINI::printCommandlineArgs(argc, argv);
  }

  //derrived
  const bool    UseMask       = !InputFile[2].empty();
  const int32   NumInputsCur  = !UseMask ? 2 : 3;
  const bool    GenBestPic    = !OutputFile[0].empty() && !OutputFile[1].empty();
  const std::string Suffix    = !UseMask ? "" : "-M";


  const int32V2 PictureSize   = { PictureWidth, PictureHeight };
  const int32   MarginIVPSNR  = xRoundUpToNearestMultiple(SearchRange, 2);
  const int32   PictureMargin = MarginIVPSNR;
  const int32   WindowSize    = 2 * SearchRange + 1;


  //print compile time setup
  if (VerboseLevel >= 1)
  {
    fmt::printf(xMiscUtilsCORE::formatCompileTimeSetup());
    fmt::printf("USE_KBNS               = %d\n", xc_USE_KBNS              );
    fmt::printf("USE_RUNTIME_CMPWEIGHTS = %d\n", xc_USE_RUNTIME_CMPWEIGHTS);
    fmt::printf("\n");
  }

  //print config
  if(VerboseLevel >= 1)
  {
    fmt::printf("Run-time configuration:\n");
    fmt::printf("InputFile0       = %s\n"  , InputFile[0]     );
    fmt::printf("InputFile1       = %s\n"  , InputFile[1]     );    
    fmt::printf("PictureWidth     = %d\n"  , PictureWidth     );
    fmt::printf("PictureHeight    = %d\n"  , PictureHeight    );
    fmt::printf("BitDepth         = %d\n"  , BitDepth         );
    fmt::printf("ChromaFormat     = %d\n"  , ChromaFormat     );
    fmt::printf("StartFrame0      = %d\n"  , StartFrame[0]    );
    fmt::printf("StartFrame1      = %d\n"  , StartFrame[1]    );
    fmt::printf("NumberOfFrames   = %d%s\n", NumberOfFrames, NumberOfFrames==NOT_VALID ? "  (all)" : "");
    fmt::printf("OutputFile0      = %s\n"  , OutputFile[0]    );
    fmt::printf("OutputFile1      = %s\n"  , OutputFile[1]    );
    fmt::printf("ResultFile       = %s\n"  , ResultFile.empty() ? "(unused)" : ResultFile);
    fmt::printf("InputFileM       = %s\n"  , InputFile[2].empty() ? "(unused)" : InputFile[2]);
    fmt::printf("BitDepthM        = %d\n"  , BitDepthM        );
    fmt::printf("ChromaFormatM    = %d\n"  , ChromaFormatM    );
    fmt::printf("Equirectangular  = %d\n"  , IsEquirectangular);
    fmt::printf("LonRangeDeg      = %d%s\n", LonRangeDeg, IsEquirectangular ? "" : "  (irrelevant)");
    fmt::printf("LatRangeDeg      = %d%s\n", LatRangeDeg, IsEquirectangular ? "" : "  (irrelevant)");
    fmt::printf("SearchRange      = %d%s\n", SearchRange, SearchRange == xIVPSNR::c_DefaultSearchRange ? "  (default)" : "  (custom)");
    fmt::printf("ComponentWeights = %s%s\n", xFmtScn::formatIntWeights(ComponentWeights), ComponentWeights == xIVPSNR::c_DefaultCmpWeights ? "  (default)" : "  (custom)");
    fmt::printf("UnnoticeableCoef = %s%s\n", xFmtScn::formatFltWeights(UnnoticeableCoef), UnnoticeableCoef == xIVPSNR::c_DefaultUnntcbCoef ? "  (default)" : "  (custom)");
    fmt::printf("Legacy8bitWSPSNR = %d\n"  , Legacy8bitWSPSNR );
    fmt::printf("NumberOfThreads  = %d%s\n", NumberOfThreads, NumberOfThreads == NOT_VALID ? "  (all)" : "");
    fmt::printf("InterleavedPic   = %d\n"  , InterleavedPic   );
    fmt::printf("InvalidPelActn   = %s\n"  , xActnToString(InvalidPelActn  ));
    fmt::printf("NameMismatchActn = %s\n"  , xActnToString(NameMismatchActn));
    fmt::printf("Calc__PSNR       = %d\n"  , Calc__PSNR       );
    fmt::printf("CalcWSPSNR       = %d\n"  , CalcWSPSNR       );
    fmt::printf("CalcIVPSNR       = %d\n"  , CalcIVPSNR       );
    fmt::printf("VerboseLevel     = %d\n"  , VerboseLevel     );    
    fmt::printf("\n");
    fmt::printf("Run-time derrived parameters:\n");
    fmt::printf("WindowSize       = %dx%d\n", WindowSize, WindowSize);
    fmt::printf("PictureMargin    = %d\n"   , PictureMargin);
    fmt::printf("UseMask          = %d\n"   , UseMask);
    fmt::printf("\n");
  }

  //check hardware concurrency
  int32 HardwareConcurency  = std::thread::hardware_concurrency();
  int32 NumberOfThreadsUsed = NumberOfThreads < 0 ? HardwareConcurency : std::min(NumberOfThreads, HardwareConcurency);
  if (VerboseLevel >= 1)
  {
    fmt::printf("Multithreading:\n");
    fmt::printf("HardwareConcurency  = %d\n", HardwareConcurency );
    fmt::printf("NumberOfThreadsUsed = %d\n", NumberOfThreadsUsed);
    fmt::printf("\n");
  }

  //check config
  std::string CfgMsg;
  if (InputFile[0].empty()              ) { CfgMsg += "!  InputFile0 is empty                 \n"; }
  if (InputFile[1].empty()              ) { CfgMsg += "!  InputFile1 is empty                 \n"; }
  if (PictureWidth <= 0                 ) { CfgMsg += "!  Invalid PictureWidth value          \n"; }
  if (PictureHeight <= 0                ) { CfgMsg += "!  Invalid PictureHeight value         \n"; }
  if (BitDepth < 8 || BitDepth > 14     ) { CfgMsg += "!  Invalid or unsuported BitDepth value\n"; }
  if (StartFrame[0]<0 || StartFrame[1]<0) { CfgMsg += "!  StartFrame value cannot be negative \n"; }
  if (!CfgMsg.empty()) { xCfgINI::printError(std::string("! Invalid parameters\n") + CfgMsg, HelpString); return EXIT_FAILURE; }

  //validate file params
  if(NameMismatchActn == eActn::WARN && NameMismatchActn == eActn::STOP)
  {
    std::string SeqMsg;
    for(int32 i = 0; i < 2; i++)
    {
      const auto [Valid, Message] = xFileNameScn::validateFileParams(InputFile[0], PictureSize, BitDepth, ChromaFormat);
      if(!Valid) { SeqMsg += Message; }
    }
    if(!SeqMsg.empty())
    {
      xCfgINI::printError(std::string("PARAMETERS WARNING: Invalid parameters\n") + SeqMsg, HelpString);
      if(NameMismatchActn == eActn::STOP) { return EXIT_FAILURE; }
    }
  }

  //check weights
  if constexpr (!xc_USE_RUNTIME_CMPWEIGHTS)
  {
    fmt::printf("CONFIGURATION WARNING: Software was build with USE_RUNTIME_CMPWEIGHTS option disabled. Config parameter ComponentWeights will be ignored. Default component weights will be used. The default weights are %s.\n\n", xFmtScn::formatIntWeights(xIVPSNR::c_DefaultCmpWeights));
  }

  //check conformance
  if(SearchRange != xIVPSNR::c_DefaultSearchRange)
  {
    fmt::printf("CONFORMANCE WARNING: Software was executed with SearchRange different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for imersive video. The default range is DefaultSearchRange=%d.\n\n", xIVPSNR::c_DefaultSearchRange);
  }
  if(xc_USE_RUNTIME_CMPWEIGHTS && ComponentWeights != xIVPSNR::c_DefaultCmpWeights)
  {
    fmt::printf("CONFORMANCE WARNING: Software was executed with ComponentWeights different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for imersive video. The default weights are DefaultCmpWeights=%s.\n\n", xFmtScn::formatIntWeights(xIVPSNR::c_DefaultCmpWeights));
  }
  if(UnnoticeableCoef != xIVPSNR::c_DefaultUnntcbCoef)
  {
    fmt::printf("CONFORMANCE WARNING: Software was executed with UnnoticeableCoef different than default one. This leads to result different than expected for MPEG Common Test Conditions defined for imersive video. The default coeffs are DefaultUnnoticeableCoef=%s.\n\n", xFmtScn::formatFltWeights(xIVPSNR::c_DefaultUnntcbCoef));
  }

  //check performance
  if(SearchRange > xIVPSNR::c_DefaultSearchRange)
  {
    fmt::printf("PERFORMANCE WARNING: Software was executed with SearchRange wider than default one. This leads to higher computational complexity and longer calculation time. The default range is DefaultSearchRange=%d.\n\n", xIVPSNR::c_DefaultSearchRange);
  }
  if(xc_USE_RUNTIME_CMPWEIGHTS && ComponentWeights == xIVPSNR::c_DefaultCmpWeights && !X_CORRESPPIXELSHIFT_CAN_USE_SSE)
  {
    fmt::printf("PERFORMANCE WARNING: Software was build with USE_RUNTIME_CMPWEIGHTS option enabled and default weights was selected. To speed up computation of IV-PSNR with default weights - dissable USE_RUNTIME_CMPWEIGHTS option.\n\n");
  }

  fmt::printf("\n\n\n");

  //==============================================================================
  //preparation
  if(VerboseLevel >= 2) { fmt::printf("Initializing:\n"); }

  //file size
  int64 SizeOfInputFile[NumInputsMax] = { 0 };
  for(int32 i = 0; i < 2; i++)
  {
    if(!xFile::exists(InputFile[i])) { xCfgINI::printError(fmt::sprintf("ERROR --> InputFile does not exist (%s)", InputFile[i])); return EXIT_FAILURE; }
    SizeOfInputFile[i] = xFile::size(InputFile[i]);
    if(VerboseLevel >= 1) { fmt::printf("SizeOfInputFile%d = %d\n", i, SizeOfInputFile[i]); }
  }

  if(UseMask)
  {
    if(!xFile::exists(InputFile[2])) { xCfgINI::printError(fmt::sprintf("ERROR --> InputFile does not exist (%s)", InputFile[2])); return EXIT_FAILURE; }
    SizeOfInputFile[2] = xFile::size(InputFile[2]);
    if(VerboseLevel >= 1) { fmt::printf("SizeOfInputFileM = %d\n", SizeOfInputFile[2]); }
  }

  //num of frames
  int32 NumOfFrames[NumInputsMax] = { 0 };
  for(int32 i = 0; i < 2; i++)
  {
    NumOfFrames[i] = xSeq::calcNumFramesInFile(PictureSize, BitDepth, ChromaFormat, SizeOfInputFile[i]);
    if(VerboseLevel >= 1) { fmt::printf("DetectedFrames%d  = %d\n", i, NumOfFrames[i]); }
    if(StartFrame[i] >= NumOfFrames[i]) { xCfgINI::printError(fmt::sprintf("ERROR --> StartFrame%d >= DetectedFrames%d for (%s)", i, i, InputFile[i])); return EXIT_FAILURE; }
  }

  if(UseMask)
  {
    NumOfFrames[2] = xSeq::calcNumFramesInFile(PictureSize, BitDepthM, ChromaFormatM, SizeOfInputFile[2]);
    if(VerboseLevel >= 1) { fmt::printf("DetectedFramesM  = %d\n", NumOfFrames[2]); }
    for(int32 i = 0; i < 2; i++) { if(StartFrame[i] != 0) { xCfgINI::printError(fmt::sprintf("ERROR --> StartFrame%d != 0 in not supported in masked mode", i)); return EXIT_FAILURE; } }
  }

  int32 MinSeqNumFrames = xMin(NumOfFrames[0], NumOfFrames[1]);
  int32 MinSeqRemFrames = xMin(NumOfFrames[0] - StartFrame[0], NumOfFrames[1] - StartFrame[1]);
  int32 NumFrames       = xMin(NumberOfFrames > 0 ? NumberOfFrames : MinSeqNumFrames, MinSeqRemFrames);
  int32 FirstFrame[3] = { 0 };
  for(int32 i = 0; i < 2; i++) { FirstFrame[i] = xMin(StartFrame[i], NumOfFrames[i] - 1); }
  if(VerboseLevel >= 1) { fmt::printf("FramesToProcess  = %d\n", NumFrames); }
  fmt::printf("\n");

  if(!InputFile[2].empty() && (NumFrames > NumOfFrames[2])) { xCfgINI::printError(fmt::sprintf("ERROR --> FramesToProcess > NumOfFramesM")); return EXIT_FAILURE; }
  
  const int32 BDs[NumInputsMax] = { BitDepth    , BitDepth    , BitDepthM };
  const int32 CFs[NumInputsMax] = { ChromaFormat, ChromaFormat, ChromaFormatM };
  std::vector<xSeq> SeqIn(NumInputsCur);
  for(int32 i = 0; i < NumInputsCur; i++) { SeqIn[i].create(PictureSize, BDs[i], CFs[i]); }
  std::vector<xPicP> PicInP(NumInputsCur);
  for(int32 i = 0; i < NumInputsCur; i++) { PicInP[i].create(PictureSize, BDs[i], PictureMargin); }
  std::vector<xPicI> PicInI(2);
  if (InterleavedPic && CalcIVPSNR) { for (int32 i = 0; i < 2; i++) { PicInI[i].create(PictureSize, BitDepth, PictureMargin); } }

  std::vector<xSeq > SeqOut (NumOutputMax);
  std::vector<xPicP> PicOutP(NumOutputMax);
  if(GenBestPic) { for(int32 i = 0; i < NumOutputMax; i++) { SeqOut [i].create(PictureSize, BDs[i], CFs[i]); PicOutP[i].create(PictureSize, BDs[i], PictureMargin); } }

  for(int32 i = 0; i < NumInputsCur; i++)
  {
    bool OpenSucces = (bool)(SeqIn[i].openFile(InputFile[i], xSeq::eMode::Read));
    if(!OpenSucces) { xCfgINI::printError(fmt::sprintf("ERROR --> InputFile opening failure (%s)", InputFile[i])); return EXIT_FAILURE; }
    if(FirstFrame[i] != 0) { SeqIn[i].seekFrame(FirstFrame[i]); }
  }

  if(GenBestPic) {
    for(int32 i = 0; i < 2; i++)
    {
      bool OpenSucces = (bool)(SeqOut[i].openFile(OutputFile[i], xSeq::eMode::Write));
      if(!OpenSucces) { xCfgINI::printError(fmt::sprintf("ERROR --> OutputFile opening failure (%s)", OutputFile[i])); return EXIT_FAILURE; }
    }
  }

  xThreadPool* ThreadPool = nullptr;
  tThPI        ThreadPoolIf;
  if(NumberOfThreadsUsed > 0)
  { 
    ThreadPool = new xThreadPool;
    ThreadPool->create(NumberOfThreadsUsed, PictureHeight+1);
    ThreadPoolIf.init(ThreadPool, 2);
  }  

  xIVPSNRM ProcessorPSNR;
  ProcessorPSNR.setLegacyWS8bit(Legacy8bitWSPSNR);
  ProcessorPSNR.setSearchRange (SearchRange     );
  ProcessorPSNR.setCmpWeights  (ComponentWeights);
  ProcessorPSNR.setUnntcbCoef  (UnnoticeableCoef);
  if(NumberOfThreadsUsed > 0) { ProcessorPSNR.initThreadPool(ThreadPool, PictureHeight); }
  ProcessorPSNR.init(PictureHeight);
  if(IsEquirectangular) { ProcessorPSNR.initWS(true, PictureWidth, PictureHeight, BitDepth, LonRangeDeg, LatRangeDeg); }

  //IVPSNR debug data
  int32V4 LastGCS = xMakeVec4(0);
  flt64   LastR2T = 0;
  flt64   LastT2R = 0;
  int32   NumNonMasked = 0;
  if(VerboseLevel >= 4 && CalcIVPSNR)
  {
    ProcessorPSNR.setDebugCallbackGCS([&LastGCS          ](const int32V4& GCS  ) { LastGCS = GCS;                });
    ProcessorPSNR.setDebugCallbackQAP([&LastR2T, &LastT2R](flt64 R2T, flt64 T2R) { LastR2T = R2T; LastT2R = T2R; });
    ProcessorPSNR.setDebugCallbackMSK([&NumNonMasked     ](int32 NNM           ) { NumNonMasked = NNM;           });
  }

  //==============================================================================
  //running
  if(VerboseLevel >= 2) { fmt::printf("Running:\n"); }
  tTimePoint ProcessingBeg = tClock::now();

  tDuration Duration__Load = tDuration(0);
  tDuration Duration__Prep = tDuration(0);
  tDuration Duration__PSNR = tDuration(0);
  tDuration DurationWSPSNR = tDuration(0);
  tDuration DurationIVPSNR = tDuration(0);

  std::vector<flt64> Frame__PSNR[4];
  std::vector<flt64> FrameWSPSNR[4];
  for(int32 CmpIdx = 0; CmpIdx < 4; CmpIdx++)
  {
    Frame__PSNR[CmpIdx].resize(NumFrames);
    FrameWSPSNR[CmpIdx].resize(NumFrames);
  }
  std::vector<flt64> FrameIVPSNR(NumFrames);
  std::vector<flt64> FrameIVSSIM(NumFrames);
  std::vector<flt64> FrameR2T(VerboseLevel >= 4 ? NumFrames : 0);
  std::vector<flt64> FrameT2R(VerboseLevel >= 4 ? NumFrames : 0);

  bool AllExact = true;
  bool AnyFake  = false;

  for(int32 f = 0; f < NumFrames; f++)
  {
    tTimePoint T0 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    std::vector<bool> ReadOK(NumInputsCur, true);
    if(ThreadPoolIf.isActive())
    {
      for(int32 i = 0; i < NumInputsCur; i++) { ThreadPoolIf.addWaitingTask([&SeqIn, &PicInP, &ReadOK, i](int32 /*ThreadIdx*/) { ReadOK[i] = (bool)SeqIn[i].readFrame(&(PicInP[i])); }); }
      ThreadPoolIf.waitUntilTasksFinished(NumInputsCur);
    }
    else
    {
      for(int32 i = 0; i < NumInputsCur; i++) { ReadOK[i] = (bool)(SeqIn[i].readFrame(&(PicInP[i]))); }
    }
    for(int32 i = 0; i < NumInputsCur; i++) { if(!ReadOK[i]) { xCfgINI::printError(fmt::sprintf("ERROR --> InputFile read error (%s)", InputFile[i])); return EXIT_FAILURE; } }

    tTimePoint T1 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    std::vector<bool> CheckOK(NumInputsCur, true);
    if(ThreadPoolIf.isActive())
    {
      for(int32 i = 0; i < NumInputsCur; i++)
      {
        ThreadPoolIf.addWaitingTask(
          [&PicInP, &PicInI, &CheckOK, &InputFile, InterleavedPic, InvalidPelActn, i](int32 /*ThreadIdx*/)
          {
            if(InvalidPelActn!=eActn::SKIP) { CheckOK[i] = PicInP[i].check(InputFile[i]); }
            if(InvalidPelActn==eActn::CNCL && !CheckOK[i]) { PicInP[i].conceal(); }
            PicInP[i].extend();
            if(InterleavedPic && i<2) { PicInI[i].rearrangeFromPlanar(&PicInP[i]); }
          }
        );
      }
      ThreadPoolIf.waitUntilTasksFinished(NumInputsCur);
    }
    else
    {
      for(int32 i = 0; i < NumInputsCur; i++)
      {
        if(InvalidPelActn!=eActn::SKIP) CheckOK[i] = PicInP[i].check(InputFile[i]);
        if(InvalidPelActn==eActn::CNCL && !CheckOK[i]) { PicInP[i].conceal(); }
        PicInP[i].extend();
        if(InterleavedPic && i < 2) { PicInI[i].rearrangeFromPlanar(&PicInP[i]); }
      }
    }

    if(InvalidPelActn==eActn::STOP)
    {
      for(int32 i = 0; i < NumInputsCur; i++)
      {
        if(!CheckOK[i]) { xCfgINI::printError(fmt::sprintf("ERROR --> InputFile contains invalid values (%s)", InputFile[i])); return EXIT_FAILURE; }
      }
    }    

    tTimePoint T2 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    if(Calc__PSNR)
    {
      flt64V4 PSNR  = xMakeVec4(0.0  );
      boolV4  Exact = xMakeVec4(false);
      if(UseMask) { std::tie(PSNR, Exact) = ProcessorPSNR.calcPicPSNRM(&PicInP[0], &PicInP[1], &PicInP[2]); }
      else        { std::tie(PSNR, Exact) = ProcessorPSNR.calcPicPSNR (&PicInP[0], &PicInP[1]              ); }

      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
      {
        if(Exact[CmpIdx]) { AnyFake  = true ; }
        else              { AllExact = false; }
        Frame__PSNR[CmpIdx][f] = PSNR[CmpIdx];        
      }

      if(VerboseLevel >= 2)
      {
        fmt::printf("Frame %08d   PSNR%s %8.4f %8.4f %8.4f", f, Suffix, PSNR[0], PSNR[1], PSNR[2]);
        if(Exact[0]) { fmt::printf(" ExactY"); }
        if(Exact[1]) { fmt::printf(" ExactU"); }
        if(Exact[2]) { fmt::printf(" ExactV"); }
        if(VerboseLevel >= 4 && UseMask) { fmt::printf("   NNM %d", NumNonMasked); }
        fmt::printf("\n");
      }
    }

    tTimePoint T3 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    if(CalcWSPSNR)
    {
      flt64V4 WSPSNR = xMakeVec4(0.0  );
      boolV4  Exact  = xMakeVec4(false);

      if(UseMask) { std::tie(WSPSNR, Exact) = ProcessorPSNR.calcPicWSPSNRM(&PicInP[0], &PicInP[1], &PicInP[2]); }
      else        { std::tie(WSPSNR, Exact) = ProcessorPSNR.calcPicWSPSNR (&PicInP[0], &PicInP[1]              ); }

      for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
      {
        FrameWSPSNR[CmpIdx][f] = WSPSNR[CmpIdx];
      }

      if(VerboseLevel >= 2)
      {
        fmt::printf("Frame %08d WSPSNR%s %8.4f %8.4f %8.4f", f, Suffix, WSPSNR[0], WSPSNR[1], WSPSNR[2]);
        if(Exact[0]) { fmt::printf(" ExactY"); }
        if(Exact[1]) { fmt::printf(" ExactU"); }
        if(Exact[2]) { fmt::printf(" ExactV"); }
        if(VerboseLevel >= 4 && UseMask) { fmt::printf("   NNM %d", NumNonMasked); }
        fmt::printf("\n");
      }
    }

    tTimePoint T4 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    if(CalcIVPSNR)
    {
      flt64 IVPSNR = 0.0;
      if(UseMask)
      {
        IVPSNR = ProcessorPSNR.calcPicIVPSNRM(&PicInP[0], &PicInP[1], &PicInP[2], &PicInI[0], &PicInI[1]);
      }
      else
      {
        if  (InterleavedPic) { IVPSNR = ProcessorPSNR.calcPicIVPSNR(&PicInP[0], &PicInP[1], &PicInI[0], &PicInI[1]); }
        else                 { IVPSNR = ProcessorPSNR.calcPicIVPSNR(&PicInP[0], &PicInP[1]                        ); }
      }
      FrameIVPSNR[f] = IVPSNR;

      if(VerboseLevel >= 2)
      {
        fmt::printf("Frame %08d IVPSNR%s %8.4f", f, Suffix, IVPSNR);
        if(VerboseLevel >= 4)
        { 
          FrameR2T[f] = LastR2T;
          FrameT2R[f] = LastT2R;
          fmt::printf("   GCS %d %d %d    R2T %7.4f  T2R %7.4f", LastGCS[0], LastGCS[1], LastGCS[2], LastR2T, LastT2R);
          if(UseMask) { fmt::printf("   NNM %d", NumNonMasked); }
        }
        fmt::printf("\n");
      }
    }

    tTimePoint T5 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    Duration__Load += (T1 - T0);
    Duration__Prep += (T2 - T1);
    Duration__PSNR += (T3 - T2);
    DurationWSPSNR += (T4 - T3);
    DurationIVPSNR += (T5 - T4);
  }

  //==============================================================================
  //finalizing

  //summary
  flt64V4 Sum__PSNR = xMakeVec4(0.0);
  flt64V4 SumWSPSNR = xMakeVec4(0.0);
  for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
  {
    Sum__PSNR[CmpIdx] = xPSNR::Accumulate(Frame__PSNR[CmpIdx]);
    SumWSPSNR[CmpIdx] = xPSNR::Accumulate(FrameWSPSNR[CmpIdx]);
  }
  flt64 SumIVPSNR = xPSNR::Accumulate(FrameIVPSNR);

  flt64V4 Avg__PSNR = Sum__PSNR / NumFrames;
  flt64V4 AvgWSPSNR = SumWSPSNR / NumFrames;
  flt64   AvgIVPSNR = SumIVPSNR / NumFrames;

  tTimePoint  ProcessingEnd  = tClock::now();

  //cleanup
  for(int32 i = 0; i < 2; i++) { SeqIn [i].closeFile(); }
  for(int32 i = 0; i < 2; i++) { SeqIn [i].destroy(); }
  for(int32 i = 0; i < 2; i++) { PicInP[i].destroy(); }
  if (InterleavedPic) { for(int32 i = 0; i < 2; i++) { PicInI[i].destroy(); } }
  if(ThreadPool) { ThreadPool->destroy(); }

  //output file
  if(!ResultFile.empty())
  {
    std::ofstream ResultStream(ResultFile, std::ios::app);
    std::time_t   TimeStamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    ResultStream                  << fmt::sprintf("FILE0  \"%s\"\n", InputFile[0]);
    ResultStream                  << fmt::sprintf("FILE1  \"%s\"\n", InputFile[1]);
    ResultStream                  << fmt::format ("TIME   {:%Y-%m-%d  %H:%M:%S}\n", fmt::localtime(TimeStamp));
    if(Calc__PSNR) { ResultStream << fmt::sprintf("PSNR%s   %8.4f dB  %8.4f dB  %8.4f dB\n", Suffix, Avg__PSNR[0], Avg__PSNR[1], Avg__PSNR[2]); }
    if(CalcWSPSNR) { ResultStream << fmt::sprintf("WSPSNR%s %8.4f dB  %8.4f dB  %8.4f dB\n", Suffix, AvgWSPSNR[0], AvgWSPSNR[1], AvgWSPSNR[2]); }
    if(CalcIVPSNR) { ResultStream << fmt::sprintf("IVPSNR%s %8.4f dB                    \n", Suffix, AvgIVPSNR                               ); }
    ResultStream.close();
  }

  //==============================================================================
  //printout results
  fmt::printf("\n\n");
  if(Calc__PSNR) { fmt::printf("Average          PSNR%s %8.4f dB  %8.4f dB  %8.4f dB\n", Suffix, Avg__PSNR[0], Avg__PSNR[1], Avg__PSNR[2]); }
  if(CalcWSPSNR) { fmt::printf("Average        WSPSNR%s %8.4f dB  %8.4f dB  %8.4f dB\n", Suffix, AvgWSPSNR[0], AvgWSPSNR[1], AvgWSPSNR[2]); }
  if(CalcIVPSNR) { fmt::printf("Average        IVPSNR%s %8.4f dB                    \n", Suffix, AvgIVPSNR                               ); }
  fmt::printf("\n");
  if(AnyFake ) { fmt::printf("FakePSNR\n"); }
  if(AllExact) { fmt::printf("ExactSeqRds\n"); }
  fmt::printf("\n");
  if(VerboseLevel >= 4 && CalcIVPSNR)
  {
    flt64 SumR2T = xPSNR::Accumulate(FrameR2T);
    flt64 SumT2R = xPSNR::Accumulate(FrameT2R);
    flt64 AvgR2T = SumR2T / NumFrames;
    flt64 AvgT2R = SumT2R / NumFrames;
    fmt::printf("Average        AC-R2T%s %8.4f dB                    \n", Suffix, AvgR2T);
    fmt::printf("Average        AC-T2R%s %8.4f dB                    \n", Suffix, AvgT2R);
    fmt::printf("\n");
  }
  if(VerboseLevel >= 3)
  {
    if(true      ) { fmt::printf("AvgTime          LOAD %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(Duration__Load).count() / NumFrames); }
    if(true      ) { fmt::printf("AvgTime       PREPROC %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(Duration__Prep).count() / NumFrames); }
    if(Calc__PSNR) { fmt::printf("AvgTime          PSNR %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(Duration__PSNR).count() / NumFrames); }
    if(CalcWSPSNR) { fmt::printf("AvgTime        WSPSNR %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationWSPSNR).count() / NumFrames); }
    if(CalcIVPSNR) { fmt::printf("AvgTime        IVPSNR %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationIVPSNR).count() / NumFrames); }
  }
  fmt::printf("\n");
  fmt::printf("TotalTime %.2f s\n", std::chrono::duration_cast<tDurationS>(ProcessingEnd - ProcessingBeg).count());
  fmt::printf("NumFrames %d\n", NumFrames);
  fmt::printf("END-OF-LOG\n");
  fflush(stdout);

  return EXIT_SUCCESS;
}

//===============================================================================================================================================================================================================
