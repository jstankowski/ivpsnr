/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xProcInfo.h"
#include "xMemory.h"
#include "xString.h"
#include <cstring>

#if __has_include("xCoreAffinity.h")
#include "xCoreAffinity.h"
#define X_PMBB_PROC_INFO_HAS_CORE_SELECTION 1
#else
#define X_PMBB_PROC_INFO_HAS_CORE_SELECTION 0
#endif


#if defined(X_PMBB_COMPILER_MSVC)
#  include <intrin.h>
#endif

#if (defined(X_PMBB_COMPILER_GCC) || defined(X_PMBB_COMPILER_CLANG))
#  if defined(X_PMBB_ARCH_AMD64)
#    include <cpuid.h>
#  elif defined(X_PMBB_ARCH_ARM64) && defined(X_PMBB_OPERATING_SYSTEM_LINUX)
#    include <sys/auxv.h>
#    include <asm/hwcap.h>
#    include "xLinuxSysfs.h"
#  endif
#endif

#if __has_include(<unistd.h>)
#  include <unistd.h>
#endif

//#if defined(X_PMBB_OPERATING_SYSTEM_WINDOWS)
//  #define WIN32_LEAN_AND_MEAN
//  #include <windows.h>
//  #undef WIN32_LEAN_AND_MEAN
//#elif __has_include(<unistd.h>)
//  #define X_PMBB_SYSTEM_UNISTD 1
//  #include <unistd.h>
//#endif

#if defined (X_PMBB_OPERATING_SYSTEM_DARWIN)
#  include <sys/types.h>
#  include <sys/sysctl.h>
#endif //X_PMBB_OPERATING_SYSTEM_DARWIN

//=============================================================================================================================================================================
// Helper functions - CPU
//=============================================================================================================================================================================
namespace {

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if defined(X_PMBB_ARCH_AMD64)

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
#endif //X_PMBB_ARCH_AMD64

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if defined(X_PMBB_ARCH_ARM64)

uint64_t xFreqHz()
{
  uint64_t FreqHz; asm volatile ("mrs %0, cntfrq_el0; isb; " : "=r"(FreqHz) :: "memory"); return FreqHz;
}

#endif //X_PMBB_ARCH_ARM64

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if defined(X_PMBB_ARCH_ARM64) && defined(X_PMBB_OPERATING_SYSTEM_DARWIN)

bool xSysCtlHasFeature(const std::string SysCtlName)
{
  uint32_t Feature = 0;;
  size_t   Size    = sizeof(Feature);
  sysctlbyname(SysCtlName.c_str(), &Feature, &Size, NULL, 0);
  return Feature != 0;
}
int64_t xSysCtlGetValue(const std::string SysCtlName)
{
  uint64_t Value = 0;;
  size_t   Size    = sizeof(Value);
  int32_t Ret = sysctlbyname(SysCtlName.c_str(), &Value, &Size, NULL, 0);
  if(Ret == 0) { return Value    ; }
  else         { return NOT_VALID; }
}
std::string xSysCtlGetString(const std::string SysCtlName)
{
  char   Buffer[4096];
  size_t Size = sizeof(Buffer);
  int32_t Ret = sysctlbyname(SysCtlName.c_str(), &Buffer, &Size, NULL, 0);
  if(Ret == 0) { return std::string(Buffer); }
  else         { return ""                 ; }
}
#endif //defined(X_PMBB_ARCH_ARM64) && defined(X_PMBB_OPERATING_SYSTEM_DARWIN)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

} //end of anonymous namespace

//=============================================================================================================================================================================

namespace PMBB_BASE {

//=============================================================================================================================================================================
// xProcInfo::xFeats
//=============================================================================================================================================================================
bool xProcInfo::xFeats::hasFeats(const std::vector<eFeat>& Features) const
{
  for(const eFeat& Feature : Features)
  {
    if(!m_Exts[(int32_t)Feature]) { return false; }
  }
  return true;
}
std::string xProcInfo::xFeats::eFeatToName(eFeat Feat)
{
  std::string Result;

  switch(Feat)
  {
#if defined(X_PMBB_ARCH_AMD64) 
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
    case eFeat::FSGSBASE            : Result = "FSGSBASE           "; break;
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
    case eFeat::CLFLUSHOPT          : Result = "CLFLUSHOPT         "; break;
    case eFeat::AVX512_F            : Result = "AVX512F            "; break;
    case eFeat::AVX512_VL           : Result = "AVX512VL           "; break;
    case eFeat::AVX512_BW           : Result = "AVX512BW           "; break;
    case eFeat::AVX512_DQ           : Result = "AVX512DQ           "; break;
    case eFeat::AVX512_CD           : Result = "AVX512CD           "; break;
    case eFeat::AVX512_ER           : Result = "AVX512ER           "; break;
    case eFeat::AVX512_PF           : Result = "AVX512PF           "; break;
    case eFeat::UMIP                : Result = "UMIP               "; break;
    case eFeat::AVX512_VBMI         : Result = "AVX512VBMI         "; break;
    case eFeat::AVX512_IFMA         : Result = "AVX512IFMA         "; break;
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
    case eFeat::MONITORX            : Result = "MONITORX           "; break;
    case eFeat::CLZERO              : Result = "CLZERO             "; break;
    case eFeat::WBNOINVD            : Result = "WBNOINVD           "; break;

    case eFeat::ECR0_X87            : Result = "ECR0_X87           "; break;
    case eFeat::ECR0_SSE            : Result = "ECR0_SSE           "; break;
    case eFeat::ECR0_AVX            : Result = "ECR0_AVX           "; break;
    case eFeat::ECR0_Opmask         : Result = "ECR0_Opmask        "; break;
    case eFeat::ECR0_ZMM_Hi256      : Result = "ECR0_ZMM_Hi256     "; break;
    case eFeat::ECR0_Hi16_ZMM       : Result = "ECR0_Hi16_ZMM      "; break;
#endif //X_PMBB_ARCH_AMD64
    //----------------------------------------------------------------------
#if defined(X_PMBB_ARCH_ARM64)
    case eFeat::FP                  : Result = "FP                 "; break;
    case eFeat::ASIMD               : Result = "ASIMD              "; break;
    case eFeat::AES                 : Result = "AES                "; break;
    case eFeat::PMULL               : Result = "PMULL              "; break;
    case eFeat::SHA1                : Result = "SHA1               "; break;
    case eFeat::SHA2                : Result = "SHA2               "; break;
    case eFeat::CRC32               : Result = "CRC32              "; break;
    case eFeat::ATOMICS             : Result = "ATOMICS            "; break;
    case eFeat::FPHP                : Result = "FPHP               "; break;
    case eFeat::ASIMDHP             : Result = "ASIMDHP            "; break;
    case eFeat::CPUID               : Result = "CPUID              "; break;
    case eFeat::ASIMDRDM            : Result = "ASIMDRDM           "; break;
    case eFeat::JSCVT               : Result = "JSCVT              "; break;
    case eFeat::FCMA                : Result = "FCMA               "; break;
    case eFeat::LRCPC               : Result = "LRCPC              "; break;
    case eFeat::DCPOP               : Result = "DCPOP              "; break;
    case eFeat::SHA3                : Result = "SHA3               "; break;
    case eFeat::SM3                 : Result = "SM3                "; break;
    case eFeat::SM4                 : Result = "SM4                "; break;
    case eFeat::ASIMDDP             : Result = "ASIMDDP            "; break;
    case eFeat::SHA512              : Result = "SHA512             "; break;
    case eFeat::SVE                 : Result = "SVE                "; break;
    case eFeat::ASIMDFHM            : Result = "ASIMDFHM           "; break;
    case eFeat::DIT                 : Result = "DIT                "; break;
    case eFeat::USCAT               : Result = "USCAT              "; break;
    case eFeat::ILRCPC              : Result = "ILRCPC             "; break;
    case eFeat::FLAGM               : Result = "FLAGM              "; break;
    case eFeat::SSBS                : Result = "SSBS               "; break;
    case eFeat::SB                  : Result = "SB                 "; break;
    case eFeat::PACA                : Result = "PACA               "; break;
    case eFeat::PACG                : Result = "PACG               "; break;
#endif //X_PMBB_ARCH_ARM64
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
#if   defined(X_PMBB_ARCH_AMD64)
  Message += xFormatProcInfoAMD64();
#elif defined(X_PMBB_ARCH_ARM64)
  Message += xFormatProcInfoARM64();
#endif //X_PMBB_ARCH
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
  Message += "  CacheLineSize      = " + std::to_string(xMemory::getRealSizeCacheLine()) + " bytes\n";
  Message += "  MemoryBasePageSize = " + std::to_string(xMemory::getRealSizePageBase ()) + " bytes  (" + xString::formatBytes(xMemory::getRealSizePageBase()) + ")\n";
  Message += "  MemoryHugePageSize = " + std::to_string(xMemory::getRealSizePageHuge ()) + " bytes  (" + xString::formatBytes(xMemory::getRealSizePageHuge()) + ")\n";
  return Message;
}
xProcInfo::eMFL xProcInfo::determineMicroArchFeatureLevel() const
{
#if defined(X_PMBB_ARCH_AMD64) 
#if X_PMBB_EXPERIMENTAL
  if(matchesZenVer4()) { return eMFL::ZenVer4; }
#endif //X_PMBB_EXPERIMENTAL
  if(matchesAMD64v4()) { return eMFL::AMD64v4; } 
  if(matchesAMD64v3()) { return eMFL::AMD64v3; } 
  if(matchesAMD64v2()) { return eMFL::AMD64v2; } 
  if(matchesAMD64v1()) { return eMFL::AMD64v1; } 
#endif //X_PMBB_ARCH_AMD64

#if defined(X_PMBB_ARCH_ARM64)
  if(matchesARM64v8p2_DPC()) { return eMFL::ARM64v8p2_DPC; } 
  if(matchesARM64v8p2    ()) { return eMFL::ARM64v8p2    ; } 
  if(matchesARM64v8p0    ()) { return eMFL::ARM64v8p0    ; } 
#endif //X_PMBB_ARCH_ARM64

  return eMFL::UNDEFINED; //nothing
}
xProcInfo::tMFLV xProcInfo::determineMicroArchFeatureLevels() const
{
#if defined(X_PMBB_ARCH_AMD64) 
#if X_PMBB_EXPERIMENTAL
  if(matchesZenVer4()) { return { eMFL::ZenVer4, eMFL::AMD64v4, eMFL::AMD64v3, eMFL::AMD64v2, eMFL::AMD64v1, eMFL::AMD64SCLR }; }
#endif //X_PMBB_EXPERIMENTAL
  if(matchesAMD64v4()) { return { eMFL::AMD64v4, eMFL::AMD64v3, eMFL::AMD64v2, eMFL::AMD64v1, eMFL::AMD64SCLR }; }
  if(matchesAMD64v3()) { return {                eMFL::AMD64v3, eMFL::AMD64v2, eMFL::AMD64v1, eMFL::AMD64SCLR }; }
  if(matchesAMD64v2()) { return {                               eMFL::AMD64v2, eMFL::AMD64v1, eMFL::AMD64SCLR }; }
  if(matchesAMD64v1()) { return {                                              eMFL::AMD64v1, eMFL::AMD64SCLR }; }
#endif //X_PMBB_ARCH_AMD64

#if defined(X_PMBB_ARCH_ARM64)
  if(matchesARM64v8p2_DPC()) { return { eMFL::ARM64v8p2_DPC, eMFL::ARM64v8p2, eMFL::ARM64v8p0, eMFL::ARM64v8p0_AVEC, eMFL::ARM64v8p0_SCLR }; }
  if(matchesARM64v8p2    ()) { return {                      eMFL::ARM64v8p2, eMFL::ARM64v8p0, eMFL::ARM64v8p0_AVEC, eMFL::ARM64v8p0_SCLR }; }
  if(matchesARM64v8p0    ()) { return {                                       eMFL::ARM64v8p0, eMFL::ARM64v8p0_AVEC, eMFL::ARM64v8p0_SCLR }; }
#endif //X_PMBB_ARCH_ARM64

  return {};
}
xProcInfo::eMFL xProcInfo::xStrToMfl(const std::string_view Mfl)
{
  std::string MflU = xString::toUpper(Mfl);
  return (MflU == "UNDEFINED"                     ) ? eMFL::UNDEFINED      :
#if defined(X_PMBB_ARCH_AMD64) 
         (MflU == "ZENVER6"    || MflU == "ZNVER6"     ) ? eMFL::ZenVer6   :
         (MflU == "ZENVER5"    || MflU == "ZNVER5"     ) ? eMFL::ZenVer5   :
         (MflU == "ZENVER4"    || MflU == "ZNVER4"     ) ? eMFL::ZenVer4   :
         (MflU == "AMD64V4"    || MflU == "X86-64-V4"  ) ? eMFL::AMD64v4   :
         (MflU == "AMD64V3"    || MflU == "X86-64-V3"  ) ? eMFL::AMD64v3   :
         (MflU == "AMD64V2"    || MflU == "X86-64-V2"  ) ? eMFL::AMD64v2   :
         (MflU == "AMD64V1"    || MflU == "X86-64-V1"  ) ? eMFL::AMD64v1   :
         (MflU == "AMD64"      || MflU == "X86-64"     ) ? eMFL::AMD64v1   :   
         (MflU == "AMD64-SCLR" || MflU == "X86-64-SCLR") ? eMFL::AMD64SCLR :   
#elif defined(X_PMBB_ARCH_ARM64) 
         (MflU == "ARM64V8P0_SCLR" ) ? eMFL::ARM64v8p0_SCLR :
         (MflU == "ARM64V8P0_AVEC" ) ? eMFL::ARM64v8p0_AVEC :
         (MflU == "ARM64V8P0"      ) ? eMFL::ARM64v8p0      :
         (MflU == "ARM64V8P2"      ) ? eMFL::ARM64v8p2      :
         (MflU == "ARM64V8P2_DPC"  ) ? eMFL::ARM64v8p2_DPC  :
#endif //X_PMBB_ARCH_ARM64
        eMFL::INVALID;
}
std::string xProcInfo::xMflToStr(eMFL Mfl)
{
  return Mfl == eMFL::UNDEFINED      ? "UNDEFINED"      :
#if defined(X_PMBB_ARCH_AMD64) 
         Mfl == eMFL::AMD64SCLR      ? "x86-64-SCLR"    :
         Mfl == eMFL::AMD64v1        ? "x86-64"         :
         Mfl == eMFL::AMD64v2        ? "x86-64-v2"      :
         Mfl == eMFL::AMD64v3        ? "x86-64-v3"      :
         Mfl == eMFL::AMD64v4        ? "x86-64-v4"      :
         Mfl == eMFL::ZenVer4        ? "ZenVer4"        :
         Mfl == eMFL::ZenVer5        ? "ZenVer5"        :
         Mfl == eMFL::ZenVer6        ? "ZenVer6"        :
#elif defined(X_PMBB_ARCH_ARM64) 
         Mfl == eMFL::ARM64v8p0_SCLR ? "ARM64v8p0_SCLR" :
         Mfl == eMFL::ARM64v8p0_AVEC ? "ARM64v8p0_AVEC" :
         Mfl == eMFL::ARM64v8p0      ? "ARM64v8p0"      :
         Mfl == eMFL::ARM64v8p2      ? "ARM64v8p2"      :
         Mfl == eMFL::ARM64v8p2_DPC  ? "ARM64v8p2_DPC"  :
#endif //X_PMBB_ARCH_ARM64
                                       "INVALID";
}
std::string xProcInfo::xMflToDescription(eMFL Mfl)
{
  switch(Mfl)
  {
#if defined(X_PMBB_ARCH_AMD64)
    case eMFL::AMD64SCLR: return "x86-64 no using SIMD autovectorization"; break;
    case eMFL::AMD64v1  : return "x86-64    : CMOV, CMPXCHG8B, FPU, FXSR, MMX, FXSR, SCE, SSE, SSE2"; break;
    case eMFL::AMD64v2  : return "x86-64-v2 : x86-64 + CMPXCHG16B, LAHF-SAHF, POPCNT, SSE3, SSE4.1, SSE4.2, SSSE3"; break;
    case eMFL::AMD64v3  : return "x86-64-v3 : x86-64-v2 + AVX, AVX2, BMI1, BMI2, F16C, FMA, LZCNT, MOVBE, XSAVE"; break;
    case eMFL::AMD64v4  : return "x86-64-v4 : x86-64-v3 + AVX512F, AVX512BW, AVX512CD, AVX512DQ, AVX512VL"; break;
    case eMFL::ZenVer4  : return "Zen4      : BMI, BMI2, CLWB, F16C, FMA, FSGSBASE, AVX, AVX2, ADCX, RDSEED, MWAITX, SHA, CLZERO, AES, PCLMUL, CX16, MOVBE, MMX, SSE, SSE2, SSE3, SSE4A, SSSE3, SSE4.1, SSE4.2, ABM, XSAVEC, XSAVES, CLFLUSHOPT, POPCNT, RDPID, WBNOINVD, PKU, VPCLMULQDQ, VAES, AVX512F, AVX512DQ, AVX512IFMA, AVX512CD, AVX512BW, AVX512VL, AVX512BF16, AVX512VBMI, AVX512VBMI2, AVX512VNNI, AVX512BITALG, AVX512VPOPCNTDQ, GFNI"; break;
    case eMFL::ZenVer5  : return "Zen5      : Zen4 + AVX_VNNI, MOVDIRI, MOVDIR64B, AVX512_VP2INTERSECT, PREFETCHI"; break;
    case eMFL::ZenVer6  : return "Zen6      : Zen5 + AVX_VNNI_INT8, AVX_IFMA, AVX512_FP16, AVX_NE_CONVERT, AVX512_BMM"; break;
#elif defined(X_PMBB_ARCH_ARM64)
    case eMFL::ARM64v8p0_SCLR: return "ARMv8.0-A only scalar code (PMBB NEON implementation disabled) - for testing purposes only"; break;
    case eMFL::ARM64v8p0_AVEC: return "ARMv8.0-A only autovectorized code (PMBB NEON implementation disabled) - for testing purposes only"; break;
    case eMFL::ARM64v8p0     : return "ARMv8.0-A"; break;
    case eMFL::ARM64v8p2     : return "ARMv8.2-A"; break;
    case eMFL::ARM64v8p2_DPC : return "ARMv8.2-A + PMULL(crypto) + ASIMDDP"; break;
#endif 
    default: return ""; break;
  }
}
void xProcInfo::xDetectProcInfo()
{
#if defined(X_PMBB_ARCH_AMD64)
  xDetectProcInfoAMD64();
#elif defined(X_PMBB_ARCH_ARM64)
  xDetectProcInfoARM64();
#endif
  m_ProcInfoChecked = true;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// x86-64
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#if defined(X_PMBB_ARCH_AMD64)
std::string xProcInfo::xFormatProcInfoAMD64() const
{
  std::string Message = "";
  Message += fmt::format("  ManufacturerId = {}\n", m_ManufacturerID);
  Message += fmt::format("  BrandString    = {}\n", m_BrandString   );
  Message += fmt::format("  ProcVersion    = Family={} Model={} Stepping={}\n", m_Family, m_Model, m_Stepping);
  return Message;
}
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

  //Family, Model. Stepping
  if(HighestFunctionSupported >= 0x1)
  {
    xCPUID(CPUInfo, 1);
    uint32_t Data = CPUInfo[c_RegEAX];
    uint32_t FamilyExt  = (Data >> 20) & 0xFF;
    uint32_t ModelExt   = (Data >> 16) & 0xF;
    uint32_t FamilyBase = (Data >>  8) & 0xF;
    uint32_t ModelBase  = (Data >>  4) & 0xF;
    m_Family   = FamilyBase + (FamilyBase == 15 ? FamilyExt : 0);
    m_Model    = FamilyBase == 6 || FamilyBase == 15 ? (ModelExt << 4) + ModelBase : ModelBase;
    m_Stepping = (Data >>  0) & 0xF;
  }

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
    m_Feats.set(eFeat::FSGSBASE          , (CPUInfo[c_RegEBX] & (1<< 0)) != 0);
    //tsc_adjust
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
    m_Feats.set(eFeat::AVX512_F          , (CPUInfo[c_RegEBX] & (1<<16)) != 0);
    m_Feats.set(eFeat::AVX512_DQ         , (CPUInfo[c_RegEBX] & (1<<17)) != 0);
    m_Feats.set(eFeat::RDSEED            , (CPUInfo[c_RegEBX] & (1<<18)) != 0);
    m_Feats.set(eFeat::ADX               , (CPUInfo[c_RegEBX] & (1<<19)) != 0);
    //smap             
    m_Feats.set(eFeat::AVX512_IFMA       , (CPUInfo[c_RegEBX] & (1<<21)) != 0);
    //pcommit
    m_Feats.set(eFeat::CLFLUSHOPT        , (CPUInfo[c_RegEBX] & (1<<23)) != 0);
    m_Feats.set(eFeat::CLWB              , (CPUInfo[c_RegEBX] & (1<<24)) != 0);
    //intel_pt         
    m_Feats.set(eFeat::AVX512_PF         , (CPUInfo[c_RegEBX] & (1<<26)) != 0);
    m_Feats.set(eFeat::AVX512_ER         , (CPUInfo[c_RegEBX] & (1<<27)) != 0);
    m_Feats.set(eFeat::AVX512_CD         , (CPUInfo[c_RegEBX] & (1<<28)) != 0);
    m_Feats.set(eFeat::SHA               , (CPUInfo[c_RegEBX] & (1<<29)) != 0);
    m_Feats.set(eFeat::AVX512_BW         , (CPUInfo[c_RegEBX] & (1<<30)) != 0);
    m_Feats.set(eFeat::AVX512_VL         , (CPUInfo[c_RegEBX] & (1<<31)) != 0);    
                       
    //ECX              
    m_Feats.set(eFeat::PREFETCHW         , (CPUInfo[c_RegECX] & (1<< 0)) != 0);
    m_Feats.set(eFeat::AVX512_VBMI       , (CPUInfo[c_RegECX] & (1<< 1)) != 0);
    m_Feats.set(eFeat::UMIP              , (CPUInfo[c_RegECX] & (1<< 2)) != 0);
    m_Feats.set(eFeat::PKU               , (CPUInfo[c_RegECX] & (1<< 3)) != 0);
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
    m_Feats.set(eFeat::MOVDIRI           , (CPUInfo[c_RegECX] & (1<<27)) != 0);
    m_Feats.set(eFeat::MOVDIR64B         , (CPUInfo[c_RegECX] & (1<<28)) != 0);
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
    m_Feats.set(eFeat::AVX_VNNI      , (CPUInfo[c_RegEAX] & (1 << 4)) != 0);
    m_Feats.set(eFeat::AVX512_BF16   , (CPUInfo[c_RegEAX] & (1 << 5)) != 0);
    m_Feats.set(eFeat::AVX_IFMA      , (CPUInfo[c_RegEAX] & (1 <<23)) != 0);

    m_Feats.set(eFeat::AVX_VNNI_INT8 , (CPUInfo[c_RegEDX] & (1 << 4)) != 0);
    m_Feats.set(eFeat::AVX_NE_CONVERT, (CPUInfo[c_RegEDX] & (1 << 5)) != 0);
    m_Feats.set(eFeat::AVX_VNNI_INT16, (CPUInfo[c_RegEDX] & (1 <<10)) != 0);
    m_Feats.set(eFeat::PREFETCHI     , (CPUInfo[c_RegEDX] & (1 <<14)) != 0);
    m_Feats.set(eFeat::AVX10         , (CPUInfo[c_RegEDX] & (1 <<19)) != 0);
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
    m_Feats.set(eFeat::LAHF_SAHF   , (CPUInfo[c_RegECX] & (1<< 0)) != 0);
    m_Feats.set(eFeat::SSE4_A      , (CPUInfo[c_RegECX] & (1<< 6)) != 0);
    m_Feats.set(eFeat::SSE_XOP     , (CPUInfo[c_RegECX] & (1<<11)) != 0);
    m_Feats.set(eFeat::FMA4        , (CPUInfo[c_RegECX] & (1<<16)) != 0);
    m_Feats.set(eFeat::TBM         , (CPUInfo[c_RegECX] & (1<<21)) != 0);
    m_Feats.set(eFeat::MONITORX    , (CPUInfo[c_RegECX] & (1<<29)) != 0);
    m_Feats.set(eFeat::MMX_3DNow   , (CPUInfo[c_RegEDX] & (1<<31)) != 0);
    m_Feats.set(eFeat::MMX_3DNowExt, (CPUInfo[c_RegEDX] & (1<<30)) != 0);
  }

  //ExtendedStandardLevel = 0x80000008
  if(HighestExtendedFunctionSupported >= 0x80000008)
  {
    xCPUID(CPUInfo, 0x80000008);
    m_Feats.set(eFeat::CLZERO  , (CPUInfo[c_RegEBX] & (1<< 0)) != 0);
    m_Feats.set(eFeat::WBNOINVD, (CPUInfo[c_RegEBX] & (1<< 9)) != 0);
  }

  //ExtendedStandardLevel = 0x80000021
  if(HighestExtendedFunctionSupported >= 0x80000021)
  {
    m_Feats.set(eFeat::PREFETCHI , (CPUInfo[c_RegEAX] & (1<<21)) != 0);
    m_Feats.set(eFeat::AVX512_BMM, (CPUInfo[c_RegEAX] & (1<<23)) != 0);
  }

  //TODO read per-CPU CPUID leaf 1Ah to detect core/atom
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

#if defined(X_PMBB_ARCH_ARM64)
std::string xProcInfo::xFormatProcInfoARM64() const
{
  std::string Message = "";
#if defined(X_PMBB_OPERATING_SYSTEM_LINUX)  
  const int32 NumCores = sysconf(_SC_NPROCESSORS_CONF);
  for(int32 CoreIdx = 0; CoreIdx < NumCores; CoreIdx++)
  {
    Message += fmt::format("  Core={:02d}   {}\n", CoreIdx, m_CoreDescriptions[CoreIdx]);
  }
#elif defined(X_PMBB_OPERATING_SYSTEM_DARWIN)
  Message += fmt::format("  BrandString    = {}\n", m_BrandString);
  Message += fmt::format("  ProcVersion    = Family={} SubFamily={} Type={} SubType={}\n", m_Family, m_SubFamily, m_Type, m_SubType);
#endif //X_PMBB_OPERATING_SYSTEM
  return Message;
}
void xProcInfo::xDetectProcInfoARM64()
{
  xDetectProcHWCAPs  ();
  xDetectCntFrequency();
  
#if defined(X_PMBB_OPERATING_SYSTEM_LINUX)
  int32 NumCores = sysconf(_SC_NPROCESSORS_CONF);
  m_CoreDescriptions.resize(NumCores);
  for(int32 c=0; c<NumCores; c++) { xDetectVendorModel(c); }
#endif //defined(X_PMBB_OPERATING_SYSTEM_LINUX)

#if defined(X_PMBB_OPERATING_SYSTEM_DARWIN)  
  xDetectVendorModel();
#endif //defined(X_PMBB_OPERATING_SYSTEM_DARWIN)
}
void xProcInfo::xDetectProcHWCAPs()
{
#if defined(X_PMBB_OPERATING_SYSTEM_LINUX)
  //https://docs.kernel.org/arch/arm64/cpu-feature-registers.html
  //https://docs.kernel.org/arch/arm64/elf_hwcaps.html

  uint64 HwCaps1 = getauxval(AT_HWCAP);
  m_Feats.set(eFeat::FP      , (HwCaps1 & HWCAP_FP      ) != 0);
  m_Feats.set(eFeat::ASIMD   , (HwCaps1 & HWCAP_ASIMD   ) != 0);
  m_Feats.set(eFeat::AES     , (HwCaps1 & HWCAP_AES     ) != 0);
  m_Feats.set(eFeat::PMULL   , (HwCaps1 & HWCAP_PMULL   ) != 0);
  m_Feats.set(eFeat::SHA1    , (HwCaps1 & HWCAP_SHA1    ) != 0);
  m_Feats.set(eFeat::SHA2    , (HwCaps1 & HWCAP_SHA2    ) != 0);
  m_Feats.set(eFeat::CRC32   , (HwCaps1 & HWCAP_CRC32   ) != 0);
  m_Feats.set(eFeat::ATOMICS , (HwCaps1 & HWCAP_ATOMICS ) != 0);
  m_Feats.set(eFeat::FPHP    , (HwCaps1 & HWCAP_FPHP    ) != 0);
  m_Feats.set(eFeat::ASIMDHP , (HwCaps1 & HWCAP_ASIMDHP ) != 0);
  m_Feats.set(eFeat::CPUID   , (HwCaps1 & HWCAP_CPUID   ) != 0);
  m_Feats.set(eFeat::ASIMDRDM, (HwCaps1 & HWCAP_ASIMDRDM) != 0);
  m_Feats.set(eFeat::JSCVT   , (HwCaps1 & HWCAP_JSCVT   ) != 0);
  m_Feats.set(eFeat::FCMA    , (HwCaps1 & HWCAP_FCMA    ) != 0);
  m_Feats.set(eFeat::LRCPC   , (HwCaps1 & HWCAP_LRCPC   ) != 0);
  m_Feats.set(eFeat::DCPOP   , (HwCaps1 & HWCAP_DCPOP   ) != 0);
  m_Feats.set(eFeat::SHA3    , (HwCaps1 & HWCAP_SHA3    ) != 0);
  m_Feats.set(eFeat::SM3     , (HwCaps1 & HWCAP_SM3     ) != 0);
  m_Feats.set(eFeat::SM4     , (HwCaps1 & HWCAP_SM4     ) != 0);
  m_Feats.set(eFeat::ASIMDDP , (HwCaps1 & HWCAP_ASIMDDP ) != 0);
  m_Feats.set(eFeat::SHA512  , (HwCaps1 & HWCAP_SHA512  ) != 0);
  m_Feats.set(eFeat::SVE     , (HwCaps1 & HWCAP_SVE     ) != 0);
  m_Feats.set(eFeat::ASIMDFHM, (HwCaps1 & HWCAP_ASIMDFHM) != 0);
  m_Feats.set(eFeat::DIT     , (HwCaps1 & HWCAP_DIT     ) != 0);
  m_Feats.set(eFeat::USCAT   , (HwCaps1 & HWCAP_USCAT   ) != 0);
  m_Feats.set(eFeat::ILRCPC  , (HwCaps1 & HWCAP_ILRCPC  ) != 0);
  m_Feats.set(eFeat::FLAGM   , (HwCaps1 & HWCAP_FLAGM   ) != 0);
  m_Feats.set(eFeat::SSBS    , (HwCaps1 & HWCAP_SSBS    ) != 0);
  m_Feats.set(eFeat::SB      , (HwCaps1 & HWCAP_SB      ) != 0);
  m_Feats.set(eFeat::PACA    , (HwCaps1 & HWCAP_PACA    ) != 0);
  m_Feats.set(eFeat::PACG    , (HwCaps1 & HWCAP_PACG    ) != 0);

#elif defined (X_PMBB_OPERATING_SYSTEM_DARWIN)

  m_Feats.set(eFeat::FP      , xSysCtlHasFeature("hw.optional.floatingpoint"   ));
  m_Feats.set(eFeat::ASIMD   , xSysCtlHasFeature("hw.optional.AdvSIMD"         ));
  m_Feats.set(eFeat::AES     , xSysCtlHasFeature("hw.optional.arm.FEAT_AES"    ));
  m_Feats.set(eFeat::PMULL   , xSysCtlHasFeature("hw.optional.arm.FEAT_PMULL"  ));
  m_Feats.set(eFeat::SHA1    , xSysCtlHasFeature("hw.optional.arm.FEAT_SHA1"   ));
  m_Feats.set(eFeat::SHA2    , xSysCtlHasFeature("hw.optional.arm.FEAT_SHA256" ));
  m_Feats.set(eFeat::CRC32   , xSysCtlHasFeature("hw.optional.armv8_crc32"     ));
  m_Feats.set(eFeat::ATOMICS , xSysCtlHasFeature("hw.optional.arm.FEAT_LSE"    ));
  m_Feats.set(eFeat::FPHP    , xSysCtlHasFeature("hw.optional.arm.FEAT_FP16"   ));
  m_Feats.set(eFeat::ASIMDHP , xSysCtlHasFeature("hw.optional.arm.FEAT_FP16"   ));
//m_Feats.set(eFeat::CPUID   , xSysCtlHasFeature(""));
  m_Feats.set(eFeat::ASIMDRDM, xSysCtlHasFeature("hw.optional.arm.FEAT_RDM"    ));
  m_Feats.set(eFeat::JSCVT   , xSysCtlHasFeature("hw.optional.arm.FEAT_JSCVT"  ));
  m_Feats.set(eFeat::FCMA    , xSysCtlHasFeature("hw.optional.arm.FEAT_FCMA"   ));
  m_Feats.set(eFeat::LRCPC   , xSysCtlHasFeature("hw.optional.arm.FEAT_LRCPC"  ));
//m_Feats.set(eFeat::DCPOP   , xSysCtlHasFeature(""));
  m_Feats.set(eFeat::SHA3    , xSysCtlHasFeature("hw.optional.arm.FEAT_SHA3"   ));
//m_Feats.set(eFeat::SM3     , xSysCtlHasFeature(""));
//m_Feats.set(eFeat::SM4     , xSysCtlHasFeature(""));
  m_Feats.set(eFeat::ASIMDDP , xSysCtlHasFeature("hw.optional.arm.FEAT_DotProd"));
  m_Feats.set(eFeat::SHA512  , xSysCtlHasFeature("hw.optional.arm.FEAT_SHA512" ));
//m_Feats.set(eFeat::SVE     , xSysCtlHasFeature(""));
  m_Feats.set(eFeat::ASIMDFHM, xSysCtlHasFeature("hw.optional.arm.FEAT_FHM"    ));
//m_Feats.set(eFeat::DIT     , xSysCtlHasFeature(""));
//m_Feats.set(eFeat::USCAT   , xSysCtlHasFeature(""));
//m_Feats.set(eFeat::ILRCPC  , xSysCtlHasFeature(""));
  m_Feats.set(eFeat::FLAGM   , xSysCtlHasFeature("hw.optional.arm.FEAT_FlagM"  ));
  m_Feats.set(eFeat::SSBS    , xSysCtlHasFeature("hw.optional.arm.FEAT_SSBS"   ));
  m_Feats.set(eFeat::SB      , xSysCtlHasFeature("hw.optional.arm.FEAT_SB"     ));
//m_Feats.set(eFeat::PACA    , xSysCtlHasFeature(""));
//m_Feats.set(eFeat::PACG    , xSysCtlHasFeature(""));  

#endif //X_PMBB_OPERATING_SYSTEM_DARWIN
}
void xProcInfo::xDetectCntFrequency()
{
  m_TSC_Frequency = (flt64)xFreqHz();
}
#endif //X_PMBB_ARCH_ARM64

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if defined(X_PMBB_ARCH_ARM64) && defined(X_PMBB_OPERATING_SYSTEM_LINUX)
uint64 xProcInfo::xReadMIDR_EL1_SYS(int32 LogicalCoreIdx)
{
  if(LogicalCoreIdx == NOT_VALID) { LogicalCoreIdx = 0; } //use first
  std::string SysFsPath = fmt::format("/sys/devices/system/cpu/cpu{:d}/regs/identification/midr_el1", LogicalCoreIdx);
  uint64_t MIDR = xLinuxSysfs::xReadHex64FromSysFsFile(SysFsPath, 0);
  return MIDR;
}
uint64 xProcInfo::xReadMIDR_EL1_MSR(int32 LogicalCoreIdx)
{
  uint64_t MIDR = 0;
#if X_PMBB_PROC_INFO_HAS_CORE_SELECTION
  if(LogicalCoreIdx != NOT_VALID) { xCoreAffinity::pinCurrentThreadToCore(LogicalCoreIdx); }
  asm volatile ("mrs %0, midr_el1;" : "=r"(MIDR) :: "memory");
  if(LogicalCoreIdx != NOT_VALID) { xCoreAffinity::unpinCurrentThread(); }
#endif //X_PMBB_PROC_INFO_HAS_CORE_SELECTION
  return MIDR;
}
xProcInfo::tStr xProcInfo::xInterpretMIDR(uint64 MIDR)
{
  uint32_t Implementer  = (uint32_t)((MIDR & 0b11111111000000000000000000000000) >> 24); //MIDR[31:24]
  uint32_t Variant      = (uint32_t)((MIDR & 0b00000000111100000000000000000000) >> 20); //MIDR[23:20]
  uint32_t Architecture = (uint32_t)((MIDR & 0b00000000000011110000000000000000) >> 16); //MIDR[19:16]
  uint32_t PartNum      = (uint32_t)((MIDR & 0b00000000000000001111111111110000) >>  4); //MIDR[15: 4]
  uint32_t Revision     = (uint32_t)((MIDR & 0b00000000000000000000000000001111) >>  0); //MIDR[ 3: 0]

  std::string ImplementerS;
  switch(Implementer)
  {
    case 0x41: ImplementerS = "ARM         "; break;
    case 0x42: ImplementerS = "Broadcom    "; break;
    case 0x43: ImplementerS = "Cavium      "; break;
    case 0x44: ImplementerS = "DEC         "; break;
    case 0x46: ImplementerS = "Fujitsu     "; break;
    case 0x48: ImplementerS = "HiSilicon   "; break;
    case 0x49: ImplementerS = "Infineon    "; break;
    case 0x4D: ImplementerS = "Motorola    "; break;
    case 0x4E: ImplementerS = "NVIDIA      "; break;
    case 0x50: ImplementerS = "AppliedMicro"; break;  
    case 0x51: ImplementerS = "Qualcomm    "; break;
    case 0x56: ImplementerS = "Marvell     "; break;  
    case 0x61: ImplementerS = "Apple       "; break;
    case 0x69: ImplementerS = "Intel       "; break;
    case 0x6D: ImplementerS = "Microsoft   "; break;
    case 0xC0: ImplementerS = "Ampere      "; break;
    default  : ImplementerS = "UNRECOGNIZED"; break;
  }
  ImplementerS = ImplementerS.substr(0, ImplementerS.find_first_of(' '));

  std::string PartNumS = "UNRECOGNIZED";
  if(Implementer == 0x41) //Arm
  {
    switch(PartNum)
    {
      //pre
      case 0xD0F : PartNumS = "AEM_V8      "; break;
      case 0xD00 : PartNumS = "FOUNDATION  "; break;
      //2012
      case 0xD07 : PartNumS = "Cortex_A57  "; break;
      case 0xD03 : PartNumS = "Cortex_A53  "; break;
      //2014
      case 0xD08 : PartNumS = "Cortex_A72  "; break;
      //2015
      case 0xD04 : PartNumS = "Cortex_A35  "; break;
      //2016
      case 0xD09 : PartNumS = "Cortex_A73  "; break;
      //2017
      case 0xD0A : PartNumS = "Cortex_A75  "; break;
      case 0xD05 : PartNumS = "Cortex_A55  "; break;
      //2018
      case 0xD0B : PartNumS = "Cortex_A76  "; break;
      case 0xD0E : PartNumS = "Cortex_A76AE"; break;
      //2019
      case 0xD0C : PartNumS = "Neoverse_N1 "; break; //same uarch as A76
      case 0xD0D : PartNumS = "Cortex_A77  "; break;
      //2020
      case 0xD44 : PartNumS = "Cortex_X1   "; break;
      case 0xD40 : PartNumS = "Neoverse_V1 "; break; //same uarch as X1
      case 0xD41 : PartNumS = "Cortex_A78  "; break;
      case 0xD42 : PartNumS = "Cortex_A78AE"; break;
      //2021
      case 0xD4C : PartNumS = "Cortex_X1C  "; break;
      case 0xD4B : PartNumS = "Cortex_A78C "; break;
      case 0xD48 : PartNumS = "Cortex_X2   "; break;
      case 0xD47 : PartNumS = "Cortex_A710 "; break;
      case 0xD49 : PartNumS = "Neoverse_N2 "; break; //same uarch as A710
      case 0xD46 : PartNumS = "Cortex_A510 "; break;
      //2022
      case 0xD4E : PartNumS = "Cortex_X3   "; break;
      case 0xD4F : PartNumS = "Neoverse_V2 "; break; //same uarch as X3
      case 0xD4D : PartNumS = "Cortex_A715 "; break;
      //2023
      case 0xD82 : PartNumS = "Cortex_X4   "; break;
      case 0xD81 : PartNumS = "Cortex_A720 "; break;
      case 0xD80 : PartNumS = "Cortex_A520 "; break;
      //2024
      case 0xD85 : PartNumS = "Cortex_X925 "; break;
      case 0xD84 : PartNumS = "Neoverse_V3 "; break;
      case 0xD87 : PartNumS = "Cortex_A725 "; break;
      case 0xD8E : PartNumS = "Neoverse_N3 "; break;
      //2025
      case 0xD8A : PartNumS = "C1-Nano     "; break;
      case 0xD8B : PartNumS = "C1-Pro      "; break;
      case 0xD90 : PartNumS = "C1-Premium  "; break; //same uarch as C1-Ultra but reduced number of FP/NEON EUs
      case 0xD8C : PartNumS = "C1-Ultra    "; break;
    }
  }

  PartNumS = PartNumS.substr(0, PartNumS.find_first_of(' '));

  std::string CoreDescription = fmt::format("{} {} Variant={} Architecture={} Revision={}", ImplementerS, PartNumS, Variant, Architecture, Revision);
  return CoreDescription;
}
void xProcInfo::xDetectVendorModel(int32 LogicalCoreIdx)
{
  // constants based on https://github.com/torvalds/linux/blob/master/arch/arm64/include/asm/cputype.h
  // TODO MPIDR_EL1
  uint64_t MIDR = xReadMIDR_EL1_SYS(LogicalCoreIdx);

  //TODO use xReadMIDR_EL1_MSR only if kernel version > 4.11 // #include <sys/utsname.h>; struct utsname osInfo{}; uname(&osInfo);
   if(MIDR == 0) { MIDR = xReadMIDR_EL1_MSR(LogicalCoreIdx); }

  m_CoreDescriptions[LogicalCoreIdx] = xInterpretMIDR(MIDR);
}
#endif //defined(X_PMBB_ARCH_ARM64) && defined(X_PMBB_OPERATING_SYSTEM_LINUX)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if defined(X_PMBB_ARCH_ARM64) && defined (X_PMBB_OPERATING_SYSTEM_DARWIN)
void xProcInfo::xDetectVendorModel()
{
  m_BrandString = xSysCtlGetString("machdep.cpu.brand_string");
  m_Family      = xSysCtlGetValue ("hw.cpufamily"            );
  m_SubFamily   = xSysCtlGetValue ("hw.cpusubfamily"         );
  m_Type        = xSysCtlGetValue ("hw.cputype"              );
  m_SubType     = xSysCtlGetValue ("hw.cpusubtype"           );
}
#endif //defined(X_PMBB_ARCH_ARM64) && defined (X_PMBB_OPERATING_SYSTEM_DARWIN)

//=============================================================================================================================================================================

} //end of namespace PMBB
