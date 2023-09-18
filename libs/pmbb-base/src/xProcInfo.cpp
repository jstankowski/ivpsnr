/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xProcInfo.h"
#include <cstring>


#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#else
#error "Unknown compiler"
#endif


#if (defined(WIN32) || defined(_WIN32) || defined(_WIN64))
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#elif __has_include(<unistd.h>)
#include <unistd.h>
#endif

//=============================================================================================================================================================================
// Helper functions - CPU
//=============================================================================================================================================================================


namespace {

static constexpr uint32_t c_RegEAX = 0;
static constexpr uint32_t c_RegEBX = 1;
static constexpr uint32_t c_RegECX = 2;
static constexpr uint32_t c_RegEDX = 3;
static constexpr uint32_t c_RegNUM = 4;

void xCPUID(uint32_t RegistersTable[4], uint32_t Leaf, uint32_t SubLeaf=0)
{
#if defined(__GNUC__) || defined(__clang__)
  __get_cpuid_count(Leaf, SubLeaf, RegistersTable + c_RegEAX, RegistersTable + c_RegEBX, RegistersTable + c_RegECX, RegistersTable + c_RegEDX);
#elif defined(_MSC_VER)
  __cpuidex((int*)RegistersTable, Leaf, SubLeaf);
#else
  #error "Unknown compiler"
#endif
}

uint64_t xXGETBV(uint32_t ExtendedControlRegisterIdx)
{
#if (defined (_MSC_FULL_VER) && _MSC_FULL_VER >= 160040000) || (defined (__INTEL_COMPILER) && __INTEL_COMPILER >= 1200)
  return uint64_t(_xgetbv(ExtendedControlRegisterIdx));
#elif defined(__GNUC__) ||  defined (__clang__)
  uint32_t a, d;
  __asm("xgetbv" : "=a"(a), "=d"(d) : "c"(ExtendedControlRegisterIdx) : );
  return uint64_t(a) | (uint64_t(d) << 32);
#else
  #error "Unknown compiler"
#endif
}

} //end of namespace

//=============================================================================================================================================================================
// Helper functions - memory
//=============================================================================================================================================================================

namespace {

int32_t xDetectCacheLineSize()
{
#if (defined(WIN32) || defined(_WIN32) || defined(_WIN64))
  DWORD bufferSize = 0;
  SYSTEM_LOGICAL_PROCESSOR_INFORMATION* buffer = 0;

  GetLogicalProcessorInformation(0, &bufferSize);
  buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION*)malloc(bufferSize);
  GetLogicalProcessorInformation(&buffer[0], &bufferSize);

  int32_t LineSize = 0;
  for(int32_t i = 0; i != bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i)
  {
    if(buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1)
    {
      LineSize = buffer[i].Cache.LineSize;
      break;
    }
  }
  free(buffer);
  return LineSize;
#elif defined(__linux__)
  FILE* File = 0;
  File = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
  int LineSize = 0;
  if(File != nullptr)
  {
    int Result = fscanf(File, "%d", &LineSize);
    fclose(File);
    if(Result != 1) { return 0; }
  }
  return LineSize;
#endif
}

int32_t xDetectMemoryPageSize()
{
  int32_t PageSize = NOT_VALID;
#if (defined(WIN32) || defined(_WIN32) || defined(_WIN64))
  SYSTEM_INFO SysInfo;
  GetSystemInfo(&SysInfo);
  PageSize = SysInfo.dwPageSize;
#elif __has_include(<unistd.h>)
  PageSize = sysconf(_SC_PAGE_SIZE);
#else
  #error "Unknown system"
#endif
  return PageSize;
}

} //end of namespace

namespace PMBB_BASE {

//=============================================================================================================================================================================
// xProcInfo::xExt
//=============================================================================================================================================================================
std::string xProcInfo::xExts::eExtToName(eExt Ext)
{
  std::string Result;

  switch(Ext)
  {
    case eExt::FPU                 : Result = "FPU                "; break;
    case eExt::CMPXCHG8B           : Result = "CMPXCHG8B          "; break;
    case eExt::MMX                 : Result = "MMX                "; break;
    case eExt::CMOV                : Result = "CMOV               "; break;
    case eExt::PSE                 : Result = "PSE                "; break;
    case eExt::TSC                 : Result = "TSC                "; break;
    case eExt::PAE                 : Result = "PAE                "; break;
    case eExt::SEP                 : Result = "SEP                "; break;
    case eExt::PSE36               : Result = "PSE36              "; break;
    case eExt::SSE1                : Result = "SSE1               "; break;
    case eExt::FXRS                : Result = "FXRS               "; break;
    case eExt::SSE2                : Result = "SSE2               "; break;
    case eExt::CLFLUSH             : Result = "CLFLUSH            "; break;
    case eExt::HT                  : Result = "HT                 "; break;
    case eExt::SSE3                : Result = "SSE3               "; break;
    case eExt::CMPXCHG16B          : Result = "CMPXCHG16B         "; break;
    case eExt::SSSE3               : Result = "SSSE3              "; break;
    case eExt::LAHF_SAHF           : Result = "LAHF_SAHF          "; break;
    case eExt::SSE4_1              : Result = "SSE4_1             "; break;
    case eExt::SSE4_2              : Result = "SSE4_2             "; break;
    case eExt::POPCNT              : Result = "POPCNT             "; break;
    case eExt::AES                 : Result = "AES                "; break;
    case eExt::CLMUL               : Result = "CLMUL              "; break;
    case eExt::AVX1                : Result = "AVX1               "; break;
    case eExt::FP16C               : Result = "FP16C              "; break;
    case eExt::RDRAND              : Result = "RDRAND             "; break;
    case eExt::AVX2                : Result = "AVX2               "; break;
    case eExt::LZCNT               : Result = "LZCNT              "; break;
    case eExt::MOVBE               : Result = "MOVBE              "; break;
    case eExt::ABM                 : Result = "ABM                "; break;
    case eExt::BMI1                : Result = "BMI1               "; break;
    case eExt::BMI2                : Result = "BMI2               "; break;
    case eExt::FMA3                : Result = "FMA3               "; break;
    case eExt::RTM                 : Result = "RTM                "; break;
    case eExt::HLE                 : Result = "HLE                "; break;
    case eExt::TSX                 : Result = "TSX                "; break;
    case eExt::INVPCID             : Result = "INVPCID            "; break;
    case eExt::ADX                 : Result = "ADX                "; break;
    case eExt::RDSEED              : Result = "RDSEED             "; break;
    case eExt::PREFETCHW           : Result = "PREFETCHW          "; break;
    case eExt::MPX                 : Result = "MPX                "; break;
    case eExt::SGX                 : Result = "SGX                "; break;
    case eExt::SHA                 : Result = "SHA                "; break;
    case eExt::AVX512F             : Result = "AVX512F            "; break;
    case eExt::AVX512VL            : Result = "AVX512VL           "; break;
    case eExt::AVX512BW            : Result = "AVX512BW           "; break;
    case eExt::AVX512DQ            : Result = "AVX512DQ           "; break;
    case eExt::AVX512CD            : Result = "AVX512CD           "; break;
    case eExt::AVX512ER            : Result = "AVX512ER           "; break;
    case eExt::AVX512PF            : Result = "AVX512PF           "; break;
    case eExt::UMIP                : Result = "UMIP               "; break;
    case eExt::AVX512VMBI          : Result = "AVX512VMBI         "; break;
    case eExt::AVX512IFMA          : Result = "AVX512IFMA         "; break;
    case eExt::AVX512_4VNNIW       : Result = "AVX512_4VNNIW      "; break;
    case eExt::AVX512_4FMAPS       : Result = "AVX512_4FMAPS      "; break;
    case eExt::CLWB                : Result = "CLWB               "; break;
    case eExt::RDPID               : Result = "RDPID              "; break;
    case eExt::AVX512_VNNI         : Result = "AVX512_VNNI        "; break;
    case eExt::AVX512_VBMI2        : Result = "AVX512_VBMI2       "; break;
    case eExt::AVX512_BITALG       : Result = "AVX512_BITALG      "; break;
    case eExt::AVX512_VPOPCNTDQ    : Result = "AVX512_VPOPCNTDQ   "; break;
    case eExt::VPCLMULQDQ          : Result = "VPCLMULQDQ         "; break;
    case eExt::VAES                : Result = "VAES               "; break;
    case eExt::GFNI                : Result = "GFNI               "; break;
    case eExt::AVX512_VP2INTERSECT : Result = "AVX512_VP2INTERSECT"; break;
    case eExt::AVX512_BF16         : Result = "AVX512_BF16        "; break;
    case eExt::AVX512_FP16         : Result = "AVX512_FP16        "; break;
    case eExt::AMX_BF16            : Result = "AMX_BF16           "; break;
    case eExt::AMX_TILE            : Result = "AMX_TILE           "; break;
    case eExt::AMX_INT8            : Result = "AMX_INT8           "; break;

    case eExt::MMX_3DNow           : Result = "MMX_3DNow          "; break;
    case eExt::MMX_3DNowExt        : Result = "MMX_3DNowExt       "; break;
    case eExt::SSE4_A              : Result = "SSE4_A             "; break;
    case eExt::SSE_XOP             : Result = "SSE_XOP            "; break;
    case eExt::FMA4                : Result = "FMA4               "; break;
    case eExt::TBM                 : Result = "TBM                "; break;

    default                        : Result = "unknown            "; break;
  }

  const auto End = Result.find_first_of(' ');
  return Result.substr(0, End);
}

//=============================================================================================================================================================================
// xProcInfo
//=============================================================================================================================================================================
void xProcInfo::detectSysInfo()
{
  xDetectExts();
  xDetectMem ();
}
void xProcInfo::printSysInfo()
{
  if(!m_ExtsChecked) { xDetectExts(); }
  printf("%s\n", xFormatProcExts(m_Exts).c_str());

  printf("Detected OS features:\n");
  printf("  AVX-instructions-allowed = %d\n\n", (int)m_OSAVX);

  if(!m_MemChecked) { xDetectMem(); }
  printf("%s\n\n", xFormatMemInfo(m_Mem).c_str());
}
std::string xProcInfo::xFormatProcExts(const xExts& Exts)
{
  std::string Message = "";
  //available
  Message += "Detected CPU features:\n";
  Message += "  Available-extensions-list = ";
  for(int32_t ExtIdx = 0; ExtIdx<(int32_t)eExt::NUM_OF_EXTS; ExtIdx++)
  {
    if(Exts.hasExt((eExt)ExtIdx)) { Message += xExts::eExtToName((eExt)ExtIdx) + " "; }
  }
  Message += "\n";
  return Message;
}
std::string xProcInfo::xFormatMemInfo(const xMem& Mem)
{
  std::string Message = "";
  Message += "Detected memory features:\n";
  Message += "  CacheLineSize  = " + std::to_string(Mem.getCacheLineSize ()) + "\n";
  Message += "  MemoryPageSize = " + std::to_string(Mem.getMemoryPageSize()) + "\n";
  return Message;
}
int32_t xProcInfo::determineMicroArchFeatureLevel()
{
  //x86-64-v4 : AVX512F, AVX512BW, AVX512CD, AVX512DQ, AVX512VL
  if(matchesAMD64v4()) { return 4; }
  //x86-64-v3 : AVX, AVX2, BMI1, BMI2, F16C, FMA, LZCNT, MOVBE, XSAVE
  if(matchesAMD64v3()) { return 3; }
  //x86-64-v2 : CMPXCHG16B, LAHF-SAHF, POPCNT, SSE3, SSE4.1, SSE4.2, SSSE3
  if(matchesAMD64v2()) { return 2; }
  //x86-64    : CMOV, CMPXCHG8B, FPU, FXSR, MMX, FXSR, SCE, SSE, SSE2
  if(matchesAMD64v1()) { return 1; }
  //nothing
  return 0;
}

void xProcInfo::xDetectExts()
{
  //http://www.sandpile.org/x86/cpuid.htm
  //StandardLevel = 0
  uint32_t CPUInfo[4]; //[0]=EAX, [1]=EBX, [2]=ECX, [3]=EDX  
  xCPUID(CPUInfo, 0);
  uint32_t HighestFunctionSupported = CPUInfo[0];
  memcpy(m_VendorID, &CPUInfo[1], 3*sizeof(int32_t));

  m_Exts  = xDetectProcExts(HighestFunctionSupported);
  m_OSAVX = xDetectOSAVX();
  m_ExtsChecked = true;
}
void xProcInfo::xDetectMem()
{
  int32_t CacheLineSize  = xDetectCacheLineSize();
  int32_t MemoryPageSize = xDetectMemoryPageSize();
  m_Mem.setCacheLineSize (CacheLineSize );
  m_Mem.setMemoryPageSize(MemoryPageSize);
  m_MemChecked = true;
}
xProcInfo::xExts xProcInfo::xDetectProcExts(uint32_t HighestFunctionSupported)
{
  xExts Exts;

  // http://www.sandpile.org/x86/cpuid.htm
  uint32_t CPUInfo[4] = { 0 }; //[0] =EAX, [1]=EBX, [2]=ECX, [3]=EDX  

  //StandardLevel = 1
  if(HighestFunctionSupported>=1)
  {
    xCPUID(CPUInfo, 1);
    //EDX
    Exts.setExt(eExt::FPU               , (CPUInfo[c_RegEDX] & (1<< 0)) != 0);
    //vme              
    //de               
    Exts.setExt(eExt::PSE               , (CPUInfo[c_RegEDX] & (1<< 3)) != 0);
    Exts.setExt(eExt::TSC               , (CPUInfo[c_RegEDX] & (1<< 4)) != 0);
    //msr              
    Exts.setExt(eExt::PAE               , (CPUInfo[c_RegEDX] & (1<< 6)) != 0);
    //mce              
    Exts.setExt(eExt::CMPXCHG8B         , (CPUInfo[c_RegEDX] & (1<< 8)) != 0);
    //apic             
    //NN               
    Exts.setExt(eExt::SEP               , (CPUInfo[c_RegEDX] & (1<<11)) != 0);
    //mtrr             
    //pge              
    //mca              
    Exts.setExt(eExt::CMOV              , (CPUInfo[c_RegEDX] & (1<<15)) != 0);
    //pat              
    Exts.setExt(eExt::PSE36             , (CPUInfo[c_RegEDX] & (1<<17)) != 0);
    //psn              
    Exts.setExt(eExt::CLFLUSH           , (CPUInfo[c_RegEDX] & (1<<19)) != 0);
    //NN               
    //ds               
    //acpi             
    Exts.setExt(eExt::MMX               , (CPUInfo[c_RegEDX] & (1<<23)) != 0);
    Exts.setExt(eExt::FXRS              , (CPUInfo[c_RegEDX] & (1<<24)) != 0);
    Exts.setExt(eExt::SSE1              , (CPUInfo[c_RegEDX] & (1<<25)) != 0);
    Exts.setExt(eExt::SSE2              , (CPUInfo[c_RegEDX] & (1<<26)) != 0);
    //ss               
    Exts.setExt(eExt::HT                , (CPUInfo[c_RegEDX] & (1<<28)) != 0);
    //tm               
    //ia64             
    //pbe              
                       
    //ECX              
    Exts.setExt(eExt::SSE3              , (CPUInfo[c_RegECX] & (1<< 0)) != 0);
    Exts.setExt(eExt::CLMUL             , (CPUInfo[c_RegECX] & (1<< 1)) != 0);
    //dtes64           
    //monitor          
    //cr8_legacy       
    Exts.setExt(eExt::LZCNT             , (CPUInfo[c_RegECX] & (1<< 5)) != 0);
    Exts.setExt(eExt::SSSE3             , (CPUInfo[c_RegECX] & (1<< 9)) != 0);
    Exts.setExt(eExt::FMA3              , (CPUInfo[c_RegECX] & (1<<12)) != 0);
    Exts.setExt(eExt::CMPXCHG16B        , (CPUInfo[c_RegECX] & (1<<13)) != 0);
    Exts.setExt(eExt::SSE4_1            , (CPUInfo[c_RegECX] & (1<<19)) != 0);
    Exts.setExt(eExt::SSE4_2            , (CPUInfo[c_RegECX] & (1<<20)) != 0);
    Exts.setExt(eExt::MOVBE             , (CPUInfo[c_RegECX] & (1<<22)) != 0);
    Exts.setExt(eExt::POPCNT            , (CPUInfo[c_RegECX] & (1<<23)) != 0);
    Exts.setExt(eExt::AES               , (CPUInfo[c_RegECX] & (1<<25)) != 0);
    Exts.setExt(eExt::AVX1              , (CPUInfo[c_RegECX] & (1<<28)) != 0);
    Exts.setExt(eExt::FP16C             , (CPUInfo[c_RegECX] & (1<<29)) != 0);
    Exts.setExt(eExt::RDRAND            , (CPUInfo[c_RegECX] & (1<<30)) != 0);
  }

  //StandardLevel , 7
  if(HighestFunctionSupported>=7)
  {
    xCPUID(CPUInfo, 7);    
    //EBX
    //fsgsbase
    Exts.setExt(eExt::SGX               , (CPUInfo[c_RegEBX] & (1<< 2)) != 0);
    Exts.setExt(eExt::BMI1              , (CPUInfo[c_RegEBX] & (1<< 3)) != 0);
    Exts.setExt(eExt::HLE               , (CPUInfo[c_RegEBX] & (1<< 4)) != 0);
    Exts.setExt(eExt::AVX2              , (CPUInfo[c_RegEBX] & (1<< 5)) != 0);
    //NN                
    //smep             
    Exts.setExt(eExt::BMI2              , (CPUInfo[c_RegEBX] & (1<< 8)) != 0);
    //erms             
    Exts.setExt(eExt::INVPCID           , (CPUInfo[c_RegEBX] & (1<<10)) != 0);
    Exts.setExt(eExt::RTM               , (CPUInfo[c_RegEBX] & (1<<11)) != 0);
    //pqm              
    //NN                
    Exts.setExt(eExt::MPX               , (CPUInfo[c_RegEBX] & (1<<14)) != 0);
    //pqe              
    Exts.setExt(eExt::AVX512F           , (CPUInfo[c_RegEBX] & (1<<16)) != 0);
    Exts.setExt(eExt::AVX512DQ          , (CPUInfo[c_RegEBX] & (1<<17)) != 0);
    Exts.setExt(eExt::RDSEED            , (CPUInfo[c_RegEBX] & (1<<18)) != 0);
    Exts.setExt(eExt::ADX               , (CPUInfo[c_RegEBX] & (1<<19)) != 0);
    //smap             
    Exts.setExt(eExt::AVX512IFMA        , (CPUInfo[c_RegEBX] & (1<<21)) != 0);
    //pcommit
    //clflushopt
    Exts.setExt(eExt::CLWB              , (CPUInfo[c_RegEBX] & (1<<24)) != 0);
    //intel_pt         
    Exts.setExt(eExt::AVX512PF          , (CPUInfo[c_RegEBX] & (1<<26)) != 0);
    Exts.setExt(eExt::AVX512ER          , (CPUInfo[c_RegEBX] & (1<<27)) != 0);
    Exts.setExt(eExt::AVX512CD          , (CPUInfo[c_RegEBX] & (1<<28)) != 0);
    Exts.setExt(eExt::SHA               , (CPUInfo[c_RegEBX] & (1<<29)) != 0);
    Exts.setExt(eExt::AVX512BW          , (CPUInfo[c_RegEBX] & (1<<30)) != 0);
    Exts.setExt(eExt::AVX512VL          , (CPUInfo[c_RegEBX] & (1<<31)) != 0);    
                       
    //ECX              
    Exts.setExt(eExt::PREFETCHW         , (CPUInfo[c_RegECX] & (1<< 0)) != 0);
    Exts.setExt(eExt::AVX512VMBI        , (CPUInfo[c_RegECX] & (1<< 1)) != 0);
    Exts.setExt(eExt::UMIP              , (CPUInfo[c_RegECX] & (1<< 2)) != 0);
    //pku              
    //ospke            
    //NN               
    Exts.setExt(eExt::AVX512_VBMI2      , (CPUInfo[c_RegECX] & (1<< 6)) != 0);
    //NN               
    Exts.setExt(eExt::GFNI              , (CPUInfo[c_RegECX] & (1<< 8)) != 0);
    Exts.setExt(eExt::VAES              , (CPUInfo[c_RegECX] & (1<< 9)) != 0);
    Exts.setExt(eExt::VPCLMULQDQ        , (CPUInfo[c_RegECX] & (1<<10)) != 0);
    Exts.setExt(eExt::AVX512_VNNI       , (CPUInfo[c_RegECX] & (1<<11)) != 0);
    Exts.setExt(eExt::AVX512_BITALG     , (CPUInfo[c_RegECX] & (1<<12)) != 0);
    //NN
    Exts.setExt(eExt::AVX512_VPOPCNTDQ  , (CPUInfo[c_RegECX] & (1<<14)) != 0);
    //NN
    //NN
    //mawau
    //mawau
    //mawau
    //mawau
    //mawau
    Exts.setExt(eExt::RDPID             , (CPUInfo[c_RegECX] & (1<<22)) != 0);
    //NN
    //NN
    //NN
    //NN
    //NN
    //NN
    //NN
    //sgx_ic
    //NN

    //EDX
    //NN
    //NN
    Exts.setExt(eExt::AVX512_4VNNIW     , (CPUInfo[c_RegEDX] & (1<< 2)) != 0);
    Exts.setExt(eExt::AVX512_4FMAPS     , (CPUInfo[c_RegEDX] & (1<< 3)) != 0);
    Exts.setExt(eExt::AVX512_BF16       , (CPUInfo[c_RegEDX] & (1<< 5)) != 0);
    Exts.setExt(eExt::AVX512_VP2INTERSECT , (CPUInfo[c_RegEDX] & (1<< 8)) != 0);
    Exts.setExt(eExt::AVX512_FP16       , (CPUInfo[c_RegEDX] & (1<<23)) != 0);
    Exts.setExt(eExt::AMX_TILE          , (CPUInfo[c_RegEDX] & (1<<24)) != 0);
    Exts.setExt(eExt::AMX_INT8          , (CPUInfo[c_RegEDX] & (1<<25)) != 0);    
  }

  //StandardLevel = 7
  if(HighestFunctionSupported>=7)
  {
    xCPUID(CPUInfo, 7, 1);
    Exts.setExt(eExt::AMX_BF16 , (CPUInfo[c_RegEAX] & (1<<22)) != 0);
  }
  

  //derrived
  Exts.setExt(eExt::LZCNT       , Exts.hasExt(eExt::BMI1));
  Exts.setExt(eExt::ABM         , Exts.hasExt(eExt::LZCNT) & Exts.hasExt(eExt::POPCNT));
  Exts.setExt(eExt::TSX         , Exts.hasExt(eExt::RTM) & Exts.hasExt(eExt::HLE));
  
  //ExtendedStandardLevel = 0x80000000
  xCPUID(CPUInfo, 0x80000000);
  unsigned int HighestExtendedFunctionSupported = CPUInfo[0];

  //ExtendedStandardLevel = 0x80000001
  if(HighestExtendedFunctionSupported>=0x80000001)
  {
    xCPUID(CPUInfo, 0x80000001);
    Exts.setExt(eExt::LAHF_SAHF    , (CPUInfo[c_RegECX] & (1<< 0)) != 0);
    Exts.setExt(eExt::SSE4_A       , (CPUInfo[c_RegECX] & (1<< 6)) != 0);
    Exts.setExt(eExt::SSE_XOP      , (CPUInfo[c_RegECX] & (1<<11)) != 0);
    Exts.setExt(eExt::FMA4         , (CPUInfo[c_RegECX] & (1<<16)) != 0);
    Exts.setExt(eExt::TBM          , (CPUInfo[c_RegECX] & (1<<21)) != 0);
    Exts.setExt(eExt::MMX_3DNow    , (CPUInfo[c_RegEDX] & (1<<31)) != 0);
    Exts.setExt(eExt::MMX_3DNowExt , (CPUInfo[c_RegEDX] & (1<<30)) != 0);
  }

  return Exts;
}

bool xProcInfo::xDetectOSAVX()
{ 
  return (xXGETBV(0) & 6) == 6; //AVX enabled in O.S.
}

//=============================================================================================================================================================================

} //end of namespace PMBB
