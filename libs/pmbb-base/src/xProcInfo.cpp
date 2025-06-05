/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xProcInfo.h"
#include "xMemory.h"
#include "xString.h"
#include <cstring>


#if defined(X_PMBB_COMPILER_MSVC)
#  include <intrin.h>
#endif

#if (defined(X_PMBB_COMPILER_GCC) || defined(X_PMBB_COMPILER_CLANG))
#  if defined(X_PMBB_ARCH_AMD64)
#    include <cpuid.h>
#  endif
#endif

//#if defined(X_PMBB_OPERATING_SYSTEM_WINDOWS)
//  #define WIN32_LEAN_AND_MEAN
//  #include <windows.h>
//  #undef WIN32_LEAN_AND_MEAN
//#elif __has_include(<unistd.h>)
//  #define X_PMBB_SYSTEM_UNISTD 1
//  #include <unistd.h>
//#endif

//=============================================================================================================================================================================
// Helper functions - CPU
//=============================================================================================================================================================================

#if defined(X_PMBB_ARCH_AMD64)
namespace {

static constexpr uint32_t c_RegEAX = 0;
static constexpr uint32_t c_RegEBX = 1;
static constexpr uint32_t c_RegECX = 2;
static constexpr uint32_t c_RegEDX = 3;
static constexpr uint32_t c_RegNUM = 4;

void xCPUID(uint32_t RegistersTable[c_RegNUM], uint32_t Leaf, uint32_t SubLeaf=0)
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
  #error "Unknown or unsuported compiler"
#endif
}

} //end of anonymous namespace
#endif //X_PMBB_ARCH_AMD64

namespace PMBB_BASE {

//=============================================================================================================================================================================
// xProcInfo::xFeats
//=============================================================================================================================================================================
std::string xProcInfo::xFeats::eFeatToName(eFeat Feat)
{
  std::string Result;

  switch(Feat)
  {
    case eFeat::FPU                 : Result = "FPU                "; break;
    case eFeat::CMPXCHG8B           : Result = "CMPXCHG8B          "; break;
    case eFeat::MMX                 : Result = "MMX                "; break;
    case eFeat::CMOV                : Result = "CMOV               "; break;
    case eFeat::PSE                 : Result = "PSE                "; break;
    case eFeat::TSC                 : Result = "TSC                "; break;
    case eFeat::PAE                 : Result = "PAE                "; break;
    case eFeat::SEP                 : Result = "SEP                "; break;
    case eFeat::PSE36               : Result = "PSE36              "; break;
    case eFeat::SSE1                : Result = "SSE1               "; break;
    case eFeat::FXRS                : Result = "FXRS               "; break;
    case eFeat::SSE2                : Result = "SSE2               "; break;
    case eFeat::CLFLUSH             : Result = "CLFLUSH            "; break;
    case eFeat::HT                  : Result = "HT                 "; break;
    case eFeat::SSE3                : Result = "SSE3               "; break;
    case eFeat::CMPXCHG16B          : Result = "CMPXCHG16B         "; break;
    case eFeat::SSSE3               : Result = "SSSE3              "; break;
    case eFeat::LAHF_SAHF           : Result = "LAHF_SAHF          "; break;
    case eFeat::SSE4_1              : Result = "SSE4_1             "; break;
    case eFeat::SSE4_2              : Result = "SSE4_2             "; break;
    case eFeat::POPCNT              : Result = "POPCNT             "; break;
    case eFeat::XSAVE               : Result = "XSAVE              "; break;
    case eFeat::OSXSAVE             : Result = "OSXSAVE            "; break;
    case eFeat::AES                 : Result = "AES                "; break;
    case eFeat::CLMUL               : Result = "CLMUL              "; break;
    case eFeat::AVX1                : Result = "AVX1               "; break;
    case eFeat::FP16C               : Result = "FP16C              "; break;
    case eFeat::RDRAND              : Result = "RDRAND             "; break;
    case eFeat::AVX2                : Result = "AVX2               "; break;
    case eFeat::LZCNT               : Result = "LZCNT              "; break;
    case eFeat::MOVBE               : Result = "MOVBE              "; break;
    case eFeat::ABM                 : Result = "ABM                "; break;
    case eFeat::BMI1                : Result = "BMI1               "; break;
    case eFeat::BMI2                : Result = "BMI2               "; break;
    case eFeat::FMA3                : Result = "FMA3               "; break;
    case eFeat::RTM                 : Result = "RTM                "; break;
    case eFeat::HLE                 : Result = "HLE                "; break;
    case eFeat::TSX                 : Result = "TSX                "; break;
    case eFeat::INVPCID             : Result = "INVPCID            "; break;
    case eFeat::ADX                 : Result = "ADX                "; break;
    case eFeat::RDSEED              : Result = "RDSEED             "; break;
    case eFeat::PREFETCHW           : Result = "PREFETCHW          "; break;
    case eFeat::MPX                 : Result = "MPX                "; break;
    case eFeat::SGX                 : Result = "SGX                "; break;
    case eFeat::SHA                 : Result = "SHA                "; break;
    case eFeat::AVX512F             : Result = "AVX512F            "; break;
    case eFeat::AVX512VL            : Result = "AVX512VL           "; break;
    case eFeat::AVX512BW            : Result = "AVX512BW           "; break;
    case eFeat::AVX512DQ            : Result = "AVX512DQ           "; break;
    case eFeat::AVX512CD            : Result = "AVX512CD           "; break;
    case eFeat::AVX512ER            : Result = "AVX512ER           "; break;
    case eFeat::AVX512PF            : Result = "AVX512PF           "; break;
    case eFeat::UMIP                : Result = "UMIP               "; break;
    case eFeat::AVX512VMBI          : Result = "AVX512VMBI         "; break;
    case eFeat::AVX512IFMA          : Result = "AVX512IFMA         "; break;
    case eFeat::AVX512_4VNNIW       : Result = "AVX512_4VNNIW      "; break;
    case eFeat::AVX512_4FMAPS       : Result = "AVX512_4FMAPS      "; break;
    case eFeat::CLWB                : Result = "CLWB               "; break;
    case eFeat::RDPID               : Result = "RDPID              "; break;
    case eFeat::AVX512_VNNI         : Result = "AVX512_VNNI        "; break;
    case eFeat::AVX512_VBMI2        : Result = "AVX512_VBMI2       "; break;
    case eFeat::AVX512_BITALG       : Result = "AVX512_BITALG      "; break;
    case eFeat::AVX512_VPOPCNTDQ    : Result = "AVX512_VPOPCNTDQ   "; break;
    case eFeat::AVX512_VP2INTERSECT : Result = "AVX512_VP2INTERSECT"; break;
    case eFeat::VPCLMULQDQ          : Result = "VPCLMULQDQ         "; break;
    case eFeat::VAES                : Result = "VAES               "; break;
    case eFeat::GFNI                : Result = "GFNI               "; break;
    case eFeat::AVX_IFMA            : Result = "AVX_IFMA           "; break;
    case eFeat::AVX512_BF16         : Result = "AVX512_BF16        "; break;
    case eFeat::AVX512_FP16         : Result = "AVX512_FP16        "; break;
    case eFeat::AMX_BF16            : Result = "AMX_BF16           "; break;
    case eFeat::AMX_TILE            : Result = "AMX_TILE           "; break;
    case eFeat::AMX_INT8            : Result = "AMX_INT8           "; break;
    case eFeat::AVX_VNNI            : Result = "AVX_VNNI           "; break;
    case eFeat::HYBRID              : Result = "HYBRID             "; break;
    case eFeat::AVX_VNNI_INT8       : Result = "AVX_VNNI_INT8      "; break;
    case eFeat::AVX_NE_CONVERT      : Result = "AVX_NE_CONVERT     "; break;
    case eFeat::AVX_VNNI_INT16      : Result = "AVX_VNNI_INT16     "; break;
    case eFeat::AVX10               : Result = "AVX10              "; break;
    case eFeat::AVX10_128           : Result = "AVX10_128          "; break;
    case eFeat::AVX10_256           : Result = "AVX10_256          "; break;
    case eFeat::AVX10_512           : Result = "AVX10_512          "; break;

    case eFeat::MMX_3DNow           : Result = "MMX_3DNow          "; break;
    case eFeat::MMX_3DNowExt        : Result = "MMX_3DNowExt       "; break;
    case eFeat::SSE4_A              : Result = "SSE4_A             "; break;
    case eFeat::SSE_XOP             : Result = "SSE_XOP            "; break;
    case eFeat::FMA4                : Result = "FMA4               "; break;
    case eFeat::TBM                 : Result = "TBM                "; break;

    case eFeat::ECR0_X87            : Result = "ECR0_X87           "; break;
    case eFeat::ECR0_SSE            : Result = "ECR0_SSE           "; break;
    case eFeat::ECR0_AVX            : Result = "ECR0_AVX           "; break;
    case eFeat::ECR0_Opmask         : Result = "ECR0_Opmask        "; break;
    case eFeat::ECR0_ZMM_Hi256      : Result = "ECR0_ZMM_Hi256     "; break;
    case eFeat::ECR0_Hi16_ZMM       : Result = "ECR0_Hi16_ZMM      "; break;

    default                         : Result = "unknown            "; break;
  }

  const auto End = Result.find_first_of(' ');
  return Result.substr(0, End);
}

//=============================================================================================================================================================================
// xProcInfo
//=============================================================================================================================================================================
void xProcInfo::detectSysInfo()
{
  xDetectProcInfo();
}
std::string xProcInfo::formatSysInfo() const
{
  std::string Message; Message.reserve(4096);
  Message += xFormatProcInfo () + "\n";
  Message += xFormatProcFeats() + "\n";
  Message += xFormatMemInfo();
  return Message;
}
std::string xProcInfo::xFormatProcInfo() const
{
  std::string Message = "";
  Message += "Detected CPU info:\n";
  Message += fmt::format("  ManufacturerId = {}\n", m_ManufacturerID);
  Message += fmt::format("  BrandString    = {}\n", m_BrandString   );
  if(m_TSC_Frequency != std::numeric_limits<flt64>::quiet_NaN())
  {
    Message += fmt::format("  TSC_Frequency  = {}\n", m_TSC_Frequency);
  }
  return Message;
}
std::string xProcInfo::xFormatProcFeats() const
{
  std::string Message = "";
  //available
  Message += "Detected CPU/OS features:\n";
  Message += "  Available-features-list = ";
  for(int32_t ExtIdx = 0; ExtIdx<(int32_t)eFeat::NUM_OF_FEATURES; ExtIdx++)
  {
    if(m_Feats.has((eFeat)ExtIdx)) { Message += xFeats::eFeatToName((eFeat)ExtIdx) + " "; }
  }
  Message += "\n";
  return Message;
}
std::string xProcInfo::xFormatMemInfo()
{
  std::string Message = "";
  Message += "Detected memory features:\n";
  Message += "  CacheLineSize      = " + std::to_string(xMemory::c_DetectedCacheLine) + " bytes\n";
  Message += "  MemoryBasePageSize = " + std::to_string(xMemory::c_DetectedPageBase ) + " bytes  (" + xString::formatBytes(xMemory::c_DetectedPageBase) + ")\n";
  Message += "  MemoryHugePageSize = " + std::to_string(xMemory::c_DetectedPageHuge ) + " bytes  (" + xString::formatBytes(xMemory::c_DetectedPageHuge) + ")\n";
  return Message;
}
xProcInfo::eMFL xProcInfo::determineMicroArchFeatureLevel() const
{
#if defined(X_PMBB_ARCH_AMD64) 
  if(matchesAMD64v4()) { return eMFL::AMD64v4; } 
  if(matchesAMD64v3()) { return eMFL::AMD64v3; } 
  if(matchesAMD64v2()) { return eMFL::AMD64v2; } 
  if(matchesAMD64v1()) { return eMFL::AMD64v1; } 
#endif //X_PMBB_ARCH_AMD64

  return eMFL::UNDEFINED; //nothing
}
xProcInfo::tMFLV xProcInfo::determineMicroArchFeatureLevels() const
{
#if defined(X_PMBB_ARCH_AMD64) 
  if(matchesAMD64v4()) { return { eMFL::AMD64v4, eMFL::AMD64v3, eMFL::AMD64v2, eMFL::AMD64v1 }; }
  if(matchesAMD64v3()) { return {                eMFL::AMD64v3, eMFL::AMD64v2, eMFL::AMD64v1 }; }
  if(matchesAMD64v2()) { return {                               eMFL::AMD64v2, eMFL::AMD64v1 }; }
  if(matchesAMD64v1()) { return {                                              eMFL::AMD64v1 }; }
#endif //X_PMBB_ARCH_AMD64

  return {};
}
xProcInfo::eMFL xProcInfo::xStrToMfl(const std::string_view Mfl)
{
  std::string MflU = xString::toUpper(Mfl);
  return (MflU == "UNDEFINED"                     ) ? eMFL::UNDEFINED      :
#if defined(X_PMBB_ARCH_AMD64) 
         (MflU == "AMD64V4" || MflU == "X86-64-V4") ? eMFL::AMD64v4        :
         (MflU == "AMD64V3" || MflU == "X86-64-V3") ? eMFL::AMD64v3        :
         (MflU == "AMD64V2" || MflU == "X86-64-V2") ? eMFL::AMD64v2        :
         (MflU == "AMD64V1" || MflU == "X86-64-V1") ? eMFL::AMD64v1        :
         (MflU == "AMD64"   || MflU == "X86-64"   ) ? eMFL::AMD64v1        :   
#endif
                                                      eMFL::INVALID;
}
std::string xProcInfo::xMflToStr(eMFL Mfl)
{
  return Mfl == eMFL::UNDEFINED      ? "UNDEFINED"      :
#if defined(X_PMBB_ARCH_AMD64) 
         Mfl == eMFL::AMD64v1        ? "x86-64"         :
         Mfl == eMFL::AMD64v2        ? "x86-64-v2"      :
         Mfl == eMFL::AMD64v3        ? "x86-64-v3"      :
         Mfl == eMFL::AMD64v4        ? "x86-64-v4"      :
#endif
                                       "INVALID";
}
std::string xProcInfo::xMflToDescription(eMFL Mfl)
{
  switch(Mfl)
  {
#if defined(X_PMBB_ARCH_AMD64)
    case eMFL::AMD64v1: return "x86-64    : CMOV, CMPXCHG8B, FPU, FXSR, MMX, FXSR, SCE, SSE, SSE2"; break;
    case eMFL::AMD64v2: return "x86-64-v2 : x86-64 + CMPXCHG16B, LAHF-SAHF, POPCNT, SSE3, SSE4.1, SSE4.2, SSSE3"; break;
    case eMFL::AMD64v3: return "x86-64-v3 : x86-64-v2 + AVX, AVX2, BMI1, BMI2, F16C, FMA, LZCNT, MOVBE, XSAVE"; break;
    case eMFL::AMD64v4: return "x86-64-v4 : x86-64-v3 + AVX512F, AVX512BW, AVX512CD, AVX512DQ, AVX512VL"; break;
#endif 
    default: return ""; break;
  }
}
void xProcInfo::xDetectProcInfo()
{
#if defined(X_PMBB_ARCH_AMD64)
  xDetectProcInfoAMD64();
#endif
  m_ProcInfoChecked = true;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// x86-64
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#if defined(X_PMBB_ARCH_AMD64)
void xProcInfo::xDetectProcInfoAMD64()
{
  //http://www.sandpile.org/x86/cpuid.htm
  uint32_t CPUInfo[c_RegNUM]; //[0]=EAX, [1]=EBX, [2]=ECX, [3]=EDX  

  //Vendor id
  xCPUID(CPUInfo, 0);
  const uint32_t HighestFunctionSupported = CPUInfo[0];
  char ManufacturerID[13] = { 0 };
  memcpy(ManufacturerID   , &CPUInfo[c_RegEBX], sizeof(int32_t));
  memcpy(ManufacturerID +4, &CPUInfo[c_RegEDX], sizeof(int32_t));
  memcpy(ManufacturerID +8, &CPUInfo[c_RegECX], sizeof(int32_t));
  m_ManufacturerID = ManufacturerID;

  //Brand string
  xCPUID(CPUInfo, 0x80000000);
  const uint32_t HighestExtendedFunctionSupported = CPUInfo[0];
  if(HighestExtendedFunctionSupported >= 0x80000004)
  {
    char BrandString[49] = { 0 };
    xCPUID(CPUInfo, 0x80000002); memcpy(BrandString   , CPUInfo, 16);
    xCPUID(CPUInfo, 0x80000003); memcpy(BrandString+16, CPUInfo, 16);
    xCPUID(CPUInfo, 0x80000004); memcpy(BrandString+32, CPUInfo, 16);
    m_BrandString = BrandString;
  }

  //Get tsc frequency
  if(HighestFunctionSupported >= 0x16)
  {
    xCPUID(CPUInfo, 0x16);
    uint64 DenominatorTSC = CPUInfo[c_RegEAX];
    uint64 NumeratorTSC   = CPUInfo[c_RegEBX];
    uint64 CrystalFreqHz  = CPUInfo[c_RegECX];
    m_TSC_Frequency = (flt64)CrystalFreqHz * (flt64)NumeratorTSC / (flt64)DenominatorTSC;
  }

  xDetectCPUID();
  xDetectMSR0 ();
  xDetectOSAVX();
}
void xProcInfo::xDetectCPUID()
{
  // http://www.sandpile.org/x86/cpuid.htm
  uint32_t CPUInfo[c_RegNUM] = { 0 }; //[0]=EAX, [1]=EBX, [2]=ECX, [3]=EDX

  xCPUID(CPUInfo, 0);
  uint32_t HighestFunctionSupported = CPUInfo[0];

  //StandardLevel = 1
  if(HighestFunctionSupported>=1)
  {
    xCPUID(CPUInfo, 1);
    //EDX
    m_Feats.set(eFeat::FPU               , (CPUInfo[c_RegEDX] & (1<< 0)) != 0);
    //vme              
    //de               
    m_Feats.set(eFeat::PSE               , (CPUInfo[c_RegEDX] & (1<< 3)) != 0);
    m_Feats.set(eFeat::TSC               , (CPUInfo[c_RegEDX] & (1<< 4)) != 0);
    //msr              
    m_Feats.set(eFeat::PAE               , (CPUInfo[c_RegEDX] & (1<< 6)) != 0);
    //mce              
    m_Feats.set(eFeat::CMPXCHG8B         , (CPUInfo[c_RegEDX] & (1<< 8)) != 0);
    //apic             
    //NN               
    m_Feats.set(eFeat::SEP               , (CPUInfo[c_RegEDX] & (1<<11)) != 0);
    //mtrr             
    //pge              
    //mca              
    m_Feats.set(eFeat::CMOV              , (CPUInfo[c_RegEDX] & (1<<15)) != 0);
    //pat              
    m_Feats.set(eFeat::PSE36             , (CPUInfo[c_RegEDX] & (1<<17)) != 0);
    //psn              
    m_Feats.set(eFeat::CLFLUSH           , (CPUInfo[c_RegEDX] & (1<<19)) != 0);
    //NN               
    //ds               
    //acpi             
    m_Feats.set(eFeat::MMX               , (CPUInfo[c_RegEDX] & (1<<23)) != 0);
    m_Feats.set(eFeat::FXRS              , (CPUInfo[c_RegEDX] & (1<<24)) != 0);
    m_Feats.set(eFeat::SSE1              , (CPUInfo[c_RegEDX] & (1<<25)) != 0);
    m_Feats.set(eFeat::SSE2              , (CPUInfo[c_RegEDX] & (1<<26)) != 0);
    //ss               
    m_Feats.set(eFeat::HT                , (CPUInfo[c_RegEDX] & (1<<28)) != 0);
    //tm               
    //ia64             
    //pbe              
                       
    //ECX              
    m_Feats.set(eFeat::SSE3              , (CPUInfo[c_RegECX] & (1<< 0)) != 0);
    m_Feats.set(eFeat::CLMUL             , (CPUInfo[c_RegECX] & (1<< 1)) != 0);
    //dtes64           
    //monitor          
    //cr8_legacy       
    m_Feats.set(eFeat::LZCNT             , (CPUInfo[c_RegECX] & (1<< 5)) != 0);
    m_Feats.set(eFeat::SSSE3             , (CPUInfo[c_RegECX] & (1<< 9)) != 0);
    m_Feats.set(eFeat::FMA3              , (CPUInfo[c_RegECX] & (1<<12)) != 0);
    m_Feats.set(eFeat::CMPXCHG16B        , (CPUInfo[c_RegECX] & (1<<13)) != 0);
    m_Feats.set(eFeat::SSE4_1            , (CPUInfo[c_RegECX] & (1<<19)) != 0);
    m_Feats.set(eFeat::SSE4_2            , (CPUInfo[c_RegECX] & (1<<20)) != 0);
    m_Feats.set(eFeat::MOVBE             , (CPUInfo[c_RegECX] & (1<<22)) != 0);
    m_Feats.set(eFeat::POPCNT            , (CPUInfo[c_RegECX] & (1<<23)) != 0);
    //tsc-deadline
    m_Feats.set(eFeat::AES               , (CPUInfo[c_RegECX] & (1<<25)) != 0);
    m_Feats.set(eFeat::XSAVE             , (CPUInfo[c_RegECX] & (1<<26)) != 0);
    m_Feats.set(eFeat::OSXSAVE           , (CPUInfo[c_RegECX] & (1<<27)) != 0);
    m_Feats.set(eFeat::AVX1              , (CPUInfo[c_RegECX] & (1<<28)) != 0);
    m_Feats.set(eFeat::FP16C             , (CPUInfo[c_RegECX] & (1<<29)) != 0);
    m_Feats.set(eFeat::RDRAND            , (CPUInfo[c_RegECX] & (1<<30)) != 0);
  }

  //StandardLevel , 7
  if(HighestFunctionSupported>=7)
  {
    xCPUID(CPUInfo, 7);    
    //EBX
    //fsgsbase
    m_Feats.set(eFeat::SGX               , (CPUInfo[c_RegEBX] & (1<< 2)) != 0);
    m_Feats.set(eFeat::BMI1              , (CPUInfo[c_RegEBX] & (1<< 3)) != 0);
    m_Feats.set(eFeat::HLE               , (CPUInfo[c_RegEBX] & (1<< 4)) != 0);
    m_Feats.set(eFeat::AVX2              , (CPUInfo[c_RegEBX] & (1<< 5)) != 0);
    //NN                
    //smep             
    m_Feats.set(eFeat::BMI2              , (CPUInfo[c_RegEBX] & (1<< 8)) != 0);
    //erms             
    m_Feats.set(eFeat::INVPCID           , (CPUInfo[c_RegEBX] & (1<<10)) != 0);
    m_Feats.set(eFeat::RTM               , (CPUInfo[c_RegEBX] & (1<<11)) != 0);
    //pqm              
    //NN                
    m_Feats.set(eFeat::MPX               , (CPUInfo[c_RegEBX] & (1<<14)) != 0);
    //pqe              
    m_Feats.set(eFeat::AVX512F           , (CPUInfo[c_RegEBX] & (1<<16)) != 0);
    m_Feats.set(eFeat::AVX512DQ          , (CPUInfo[c_RegEBX] & (1<<17)) != 0);
    m_Feats.set(eFeat::RDSEED            , (CPUInfo[c_RegEBX] & (1<<18)) != 0);
    m_Feats.set(eFeat::ADX               , (CPUInfo[c_RegEBX] & (1<<19)) != 0);
    //smap             
    m_Feats.set(eFeat::AVX512IFMA        , (CPUInfo[c_RegEBX] & (1<<21)) != 0);
    //pcommit
    //clflushopt
    m_Feats.set(eFeat::CLWB              , (CPUInfo[c_RegEBX] & (1<<24)) != 0);
    //intel_pt         
    m_Feats.set(eFeat::AVX512PF          , (CPUInfo[c_RegEBX] & (1<<26)) != 0);
    m_Feats.set(eFeat::AVX512ER          , (CPUInfo[c_RegEBX] & (1<<27)) != 0);
    m_Feats.set(eFeat::AVX512CD          , (CPUInfo[c_RegEBX] & (1<<28)) != 0);
    m_Feats.set(eFeat::SHA               , (CPUInfo[c_RegEBX] & (1<<29)) != 0);
    m_Feats.set(eFeat::AVX512BW          , (CPUInfo[c_RegEBX] & (1<<30)) != 0);
    m_Feats.set(eFeat::AVX512VL          , (CPUInfo[c_RegEBX] & (1<<31)) != 0);    
                       
    //ECX              
    m_Feats.set(eFeat::PREFETCHW         , (CPUInfo[c_RegECX] & (1<< 0)) != 0);
    m_Feats.set(eFeat::AVX512VMBI        , (CPUInfo[c_RegECX] & (1<< 1)) != 0);
    m_Feats.set(eFeat::UMIP              , (CPUInfo[c_RegECX] & (1<< 2)) != 0);
    //pku              
    //ospke            
    //NN               
    m_Feats.set(eFeat::AVX512_VBMI2      , (CPUInfo[c_RegECX] & (1<< 6)) != 0);
    //NN               
    m_Feats.set(eFeat::GFNI              , (CPUInfo[c_RegECX] & (1<< 8)) != 0);
    m_Feats.set(eFeat::VAES              , (CPUInfo[c_RegECX] & (1<< 9)) != 0);
    m_Feats.set(eFeat::VPCLMULQDQ        , (CPUInfo[c_RegECX] & (1<<10)) != 0);
    m_Feats.set(eFeat::AVX512_VNNI       , (CPUInfo[c_RegECX] & (1<<11)) != 0);
    m_Feats.set(eFeat::AVX512_BITALG     , (CPUInfo[c_RegECX] & (1<<12)) != 0);
    //NN
    m_Feats.set(eFeat::AVX512_VPOPCNTDQ  , (CPUInfo[c_RegECX] & (1<<14)) != 0);
    //NN
    //NN
    //mawau
    //mawau
    //mawau
    //mawau
    //mawau
    m_Feats.set(eFeat::RDPID             , (CPUInfo[c_RegECX] & (1<<22)) != 0);
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
    m_Feats.set(eFeat::AVX512_4VNNIW       , (CPUInfo[c_RegEDX] & (1<< 2)) != 0);
    m_Feats.set(eFeat::AVX512_4FMAPS       , (CPUInfo[c_RegEDX] & (1<< 3)) != 0);
    m_Feats.set(eFeat::AVX512_VP2INTERSECT , (CPUInfo[c_RegEDX] & (1<< 8)) != 0);
    m_Feats.set(eFeat::HYBRID              , (CPUInfo[c_RegEDX] & (1<<15)) != 0);
    m_Feats.set(eFeat::AMX_BF16            , (CPUInfo[c_RegEDX] & (1<<22)) != 0);
    m_Feats.set(eFeat::AVX512_FP16         , (CPUInfo[c_RegEDX] & (1<<23)) != 0);
    m_Feats.set(eFeat::AMX_TILE            , (CPUInfo[c_RegEDX] & (1<<24)) != 0);
    m_Feats.set(eFeat::AMX_INT8            , (CPUInfo[c_RegEDX] & (1<<25)) != 0);
  }

  //StandardLevel = 7
  if(HighestFunctionSupported>=7)
  {
    xCPUID(CPUInfo, 7, 1);
    // EAX:0 = SHA512
    // EAX:1 = sm3 
    // EAX:2 = sm4
    // rao-int
    m_Feats.set(eFeat::AVX_VNNI      , (CPUInfo[c_RegEAX] & (1 <<  4)) != 0);
    m_Feats.set(eFeat::AVX512_BF16   , (CPUInfo[c_RegEAX] & (1 <<  5)) != 0);
    m_Feats.set(eFeat::AVX_IFMA      , (CPUInfo[c_RegEAX] & (1 << 23)) != 0);

    m_Feats.set(eFeat::AVX_VNNI_INT8 , (CPUInfo[c_RegEDX] & (1 <<  4)) != 0);
    m_Feats.set(eFeat::AVX_NE_CONVERT, (CPUInfo[c_RegEDX] & (1 <<  5)) != 0);
    m_Feats.set(eFeat::AVX_VNNI_INT16, (CPUInfo[c_RegEDX] & (1 << 10)) != 0);
    m_Feats.set(eFeat::AVX10         , (CPUInfo[c_RegEDX] & (1 << 19)) != 0);
  }

  //StandardLevel = 0x24
  if(HighestFunctionSupported >= 0x24)
  {
    xCPUID(CPUInfo, 0x24, 0);
    m_Feats.set(eFeat::AVX10_128, (CPUInfo[c_RegEBX] & (1 << 16)) != 0);
    m_Feats.set(eFeat::AVX10_256, (CPUInfo[c_RegEBX] & (1 << 17)) != 0);
    m_Feats.set(eFeat::AVX10_512, (CPUInfo[c_RegEBX] & (1 << 18)) != 0);
  }  

  //derrived
  m_Feats.set(eFeat::LZCNT       , m_Feats.has(eFeat::BMI1));
  m_Feats.set(eFeat::ABM         , m_Feats.has(eFeat::LZCNT) && m_Feats.has(eFeat::POPCNT));
  m_Feats.set(eFeat::TSX         , m_Feats.has(eFeat::RTM  ) && m_Feats.has(eFeat::HLE   ));
  
  //ExtendedStandardLevel = 0x80000000
  xCPUID(CPUInfo, 0x80000000);
  unsigned int HighestExtendedFunctionSupported = CPUInfo[0];

  //ExtendedStandardLevel = 0x80000001
  if(HighestExtendedFunctionSupported>=0x80000001)
  {
    xCPUID(CPUInfo, 0x80000001);
    m_Feats.set(eFeat::LAHF_SAHF    , (CPUInfo[c_RegECX] & (1<< 0)) != 0);
    m_Feats.set(eFeat::SSE4_A       , (CPUInfo[c_RegECX] & (1<< 6)) != 0);
    m_Feats.set(eFeat::SSE_XOP      , (CPUInfo[c_RegECX] & (1<<11)) != 0);
    m_Feats.set(eFeat::FMA4         , (CPUInfo[c_RegECX] & (1<<16)) != 0);
    m_Feats.set(eFeat::TBM          , (CPUInfo[c_RegECX] & (1<<21)) != 0);
    m_Feats.set(eFeat::MMX_3DNow    , (CPUInfo[c_RegEDX] & (1<<31)) != 0);
    m_Feats.set(eFeat::MMX_3DNowExt , (CPUInfo[c_RegEDX] & (1<<30)) != 0);
  }
}
void xProcInfo::xDetectMSR0()
{ 
  if(! (m_Feats.has(eFeat::XSAVE) && m_Feats.has(eFeat::OSXSAVE))) { return; }

  uint64 XRC0 = xXGETBV(0); // XCR0 - Extended Control Registers

  m_Feats.set(eFeat::ECR0_X87, (XRC0 & (1 << 0)) != 0);
  m_Feats.set(eFeat::ECR0_SSE, (XRC0 & (1 << 1)) != 0);
  m_Feats.set(eFeat::ECR0_AVX, (XRC0 & (1 << 2)) != 0);
  //ECR0_BNDREG,
  //ECR0_BNDREG,
  m_Feats.set(eFeat::ECR0_Opmask   , (XRC0 & (1 << 5)) != 0); 
  m_Feats.set(eFeat::ECR0_ZMM_Hi256, (XRC0 & (1 << 6)) != 0); 
  m_Feats.set(eFeat::ECR0_Hi16_ZMM , (XRC0 & (1 << 7)) != 0);
  //ECR0_PKRU
 }
void xProcInfo::xDetectOSAVX()
{
   m_OSAVX = m_Feats.has(eFeat::ECR0_AVX);
}
#endif //defined(X_PMBB_ARCH_AMD64)

//=============================================================================================================================================================================

} //end of namespace PMBB
