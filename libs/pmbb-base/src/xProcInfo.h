/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefBASE.h"
#include <vector>
#include <array>

namespace PMBB_BASE {

//=============================================================================================================================================================================
// xProcInfo
//=============================================================================================================================================================================

class xProcInfo
{
public:
  using tStr  = std::string;
  using tStrV = std::vector<std::string>;

  enum class eFeat
  {
    //-----------------------------------------------------------------
#if defined(X_PMBB_ARCH_AMD64) 
    //Intel
    //x87
    FPU               , //Floating Point Unit (integrated since 80486)
    //486 - 1989
    CMPXCHG8B         ,
    //P55C - 1993                 
    MMX               ,
    CMOV              , //Conditional move
    PSE               , //Page Size Extension (PSE)
    TSC               , //Time Stamp Counter (TSC) 
    //P6 - 1995
    PAE               , //Physical Address Extension (PAE)
    //Klamath - 1997
    SEP               , //SYSENTER and SYSEXIT instructions
    //Deschutes              
    PSE36             , //36-bit Page Size Extension
    //Katmai - 1999
    SSE1              ,
    FXRS              , //FXSAVE, FXRESTOR instructions
    //Willamette - 2000
    SSE2              ,
    CLFLUSH           ,
    //Northwood              
    HT                , //Hyperthreading
    //Prescott - 2004
    SSE3              ,
    CMPXCHG16B        ,
    //Merom - 2006
    SSSE3             , //SupplementalSSE3
    //Clovertown - 2006
    LAHF_SAHF         , //Intel VT
    //Penryn - 2007
    SSE4_1            ,
    //Nehalem - 2008
    SSE4_2            ,
    POPCNT            ,
    XSAVE             ,
    OSXSAVE           ,
    //Westmere               
    AES               , //EAS encryption accelerator 
    CLMUL             , //Carry-less Multiplication
    //Sandy Bridge           
    AVX1              ,
    //Ivy Bridge             
    FP16C             , //Half float convertion
    RDRAND            , //SP 800-90A - Cryptographically secure pseudorandom number generator
    FSGSBASE          , //Allows applications to directly write to the FS and GS segment registers
    //Haswell                
    AVX2              ,
    LZCNT             , //Leading zero count instruction
    MOVBE             , //Load and Store of Big Endian forms
    ABM               , //Advanced Bit Manipulation (LZCNT + POPCNT)
    BMI1              , //Bit Manipulation Instructions 1
    BMI2              , //Bit Manipulation Instructions 2
    FMA3              , //Fused multiple-add
    RTM               , //Restricted Transactional Memory
    HLE               , //Hardware Lock Elision
    TSX               , //TSX=RTM+HLE Transactional Synchronization Extensions
    INVPCID           , //Invalidate processor context ID 
    //Broadwell
    ADX               , //Multi-Precision Add-Carry Instruction Extensions (ADOX, ADCX, MULX)
    RDSEED            , //SP 800-90B & C - Non-deterministic random bit generator
    PREFETCHW         , 
    //Skylake
    MPX               , //Memory Protection Extensions  
    SGX               , //Software Guard Extensions
    SHA               , //Intel SHA Extensions
    CLFLUSHOPT        ,
    //Skylake-X                
    AVX512_F          , //Foundation  
    AVX512_VL         , //Vector Length Extensions
    AVX512_BW         , //Byte and Word Instructions
    AVX512_DQ         , //Doubleword and Quadword Instructions
    AVX512_CD         , //Conflict Detection Instructions
    PKU               , //Memory Protection Keys for User-mode pages
    //Knights Landing
    AVX512_ER         , //Exponential and Reciprocal Instructions
    AVX512_PF         , //Prefetch Instructions    
    //Cannonlake
    UMIP              , //User-Mode Instruction Prevention
    AVX512_VBMI       , //Vector Byte Manipulation Instructions
    AVX512_IFMA       , //Integer Fused Multiply Add (52-bit Integer Multiply-Add)
    //Knights Mill            
    AVX512_4VNNIW     , //4-register Neural Network Instructions / Vector Neural Network Instructions Word variable precision
    AVX512_4FMAPS     , //4-register Multiply Accumulation Single precision
    //Sunny Cove (Ice Lake)
    CLWB               , //Cache Line Write Back
    RDPID              , //Read Processor ID
    AVX512_VNNI        , //Vector Neural Network Instructions
    AVX512_VBMI2       , //Vector Byte Manipulation Instructions 2
    AVX512_BITALG      , //Support for VPOPCNT[B,W] and VPSHUF-BITQMB
    AVX512_VPOPCNTDQ   , //Vector POPCNT
    AVX512_VP2INTERSECT,
    VPCLMULQDQ         , //Carry-Less Multiplication Quadword
    VAES               , //Vector AES
    GFNI               , //Galois Field New Instructions
    //Willow Cove (Tiger Lake)    
    AVX_IFMA           ,
    MOVDIRI            ,
    MOVDIR64B          ,
    //Willow Cove (Sapphire Rapids) 
    AVX512_BF16        ,
    AVX512_FP16        ,
    AMX_BF16           ,
    AMX_TILE           ,
    AMX_INT8           ,
    //Golden Cove (Alder Lake)
    AVX_VNNI           ,
    HYBRID             ,
    //Crestmont (Sierra Forest)
    AVX_VNNI_INT8      ,
    PREFETCHI          ,
    //Lion Cove/Skymont (Arrow Lake/Lunar Lake)
    AVX_NE_CONVERT     ,
    AVX_VNNI_INT16     ,
    //???
    AVX10              ,
    AVX10_128          ,
    AVX10_256          ,
    AVX10_512          ,
   
    //AMD
    //Chompers
    MMX_3DNow         ,
    //Thunderbird          
    MMX_3DNowExt      ,
    //Barcelona            
    SSE4_A            ,
    //Bulldozer            
    SSE_XOP           ,
    FMA4              ,
    //Piledriver           
    TBM               ,
    //Excavator
    MONITORX          ,
    //Zen1
    CLZERO            ,
    //Zen2
    WBNOINVD          , //Write Back and Do Not Invalidate Cache
    //Zen6
    AVX512_BMM        , 

    //ECR0
    ECR0_X87          , //x87 FPU/MMX support (must be 1) 
    ECR0_SSE          , //XSAVE support for MXCSR and XMM registers 
    ECR0_AVX          , //AVX enabled and XSAVE support for upper halves of YMM registers 
    ECR0_Opmask       , //AVX-512 enabled and XSAVE support for opmask registers k0-k7 
    ECR0_ZMM_Hi256    , //AVX-512 enabled and XSAVE support for upper halves of lower ZMM registers
    ECR0_Hi16_ZMM     , //AVX-512 enabled and XSAVE support for upper ZMM registers 
#endif //X_PMBB_ARCH_AMD64
    //-----------------------------------------------------------------
#if defined(X_PMBB_ARCH_ARM64)
    //ARM64 (vel Aarch64) - HWCAPS
    FP          ,
    ASIMD       , //NEON
    AES         , 
    PMULL       , //Polynomial multiplication
    SHA1        ,
    SHA2        ,
    CRC32       ,
    ATOMICS     ,
    FPHP        ,
    ASIMDHP     ,
    CPUID       ,
    ASIMDRDM    ,
    JSCVT       ,
    FCMA        ,
    LRCPC       ,
    DCPOP       ,
    SHA3        ,
    SM3         ,
    SM4         ,
    ASIMDDP     , //dot product
    SHA512      ,
    SVE         ,
    ASIMDFHM    ,
    DIT         ,
    USCAT       ,
    ILRCPC      ,
    FLAGM       ,
    SSBS        ,
    SB          ,
    PACA        ,
    PACG        ,
    //(vel Aarch64) - HWCAPS2
    //DCPODP      ,
    //SVE2        ,
    //SVEAES      ,
    //SVEPMULL    ,
    //SVEBITPERM  ,
    //SVESHA3     ,
    //SVESM4      ,
    //FLAGM2      ,
    //FRINT       ,
    //SVEI8MM     ,
    //SVEF32MM    ,
    //SVEF64MM    ,
    //SVEBF16     ,
    //I8MM        ,
    //BF16        ,
    //DGH         ,
    //RNG         ,
    //BTI         ,
    //MTE         ,
    //ECV         ,
    //AFP         ,
    //RPRES       ,
    //MTE3        ,
    //SME         ,
    //SME_I16I64  ,
    //SME_F64F64  ,
    //SME_I8I32   ,
    //SME_F16F32  ,
    //SME_B16F32  ,
    //SME_F32F32  ,
    //SME_FA64    ,
    //WFXT        ,
    //EBF16       ,
    //SVE_EBF16   ,
    //CSSC        ,
    //RPRFM       ,
    //SVE2P1      ,
    //SME2        ,
    //SME2P1      ,
    //SME_I16I32  ,
    //SME_BI32I32 ,
    //SME_B16B16  ,
    //SME_F16F16  ,
    //MOPS        ,
    //HBC         ,
    //SVE_B16B16  ,
    //LRCPC3      ,
    //LSE128      ,
    //FPMR        ,
    //LUT         ,
    //FAMINMAX    ,
    //F8CVT       ,
    //F8FMA       ,
    //F8DP4       ,
    //F8DP2       ,
    //F8E4M3      ,
    //F8E5M2      ,
    //SME_LUTV2   ,
    //SME_F8F16   ,
    //SME_F8F32   ,
    //SME_SF8FMA  ,
    //SME_SF8DP4  ,
    //SME_SF8DP2  ,
    //POE         ,
    //(vel Aarc h64) - HWCAPS3
    //MTE_FAR       ,
    //MTE_STORE_ONLY,
#endif //X_PMBB_ARCH_ARM64
    //-----------------------------------------------------------------
    NUM_OF_FEATURES
  };

  class xFeats
  {
  public:
#if defined(X_PMBB_ARCH_AMD64) 
    static constexpr std::array c_FeaturesAMD64v1 = { eFeat::CMOV, eFeat::CMPXCHG8B, eFeat::FPU, eFeat::FXRS, eFeat::MMX, eFeat::SSE1, eFeat::SSE2 };
    static constexpr std::array c_FeatDiffAMD64v2 = { eFeat::CMPXCHG16B, eFeat::LAHF_SAHF, eFeat::POPCNT, eFeat::SSE3, eFeat::SSSE3, eFeat::SSE4_1, eFeat::SSE4_2 };
    static constexpr std::array c_FeatDiffAMD64v3 = { eFeat::AVX1, eFeat::AVX2, eFeat::BMI1, eFeat::BMI2, eFeat::FP16C, eFeat::FMA3, eFeat::LZCNT, eFeat::MOVBE };
    static constexpr std::array c_FeatDiffAMD64v4 = { eFeat::AVX512_F, eFeat::AVX512_BW, eFeat::AVX512_CD, eFeat::AVX512_DQ, eFeat::AVX512_VL };
    
    static constexpr std::array c_FeaturesZenVer4 = { eFeat::BMI1, eFeat::BMI2, eFeat::CLWB, eFeat::FP16C, eFeat::FMA3, eFeat::FSGSBASE, eFeat::AVX1, eFeat::AVX2, eFeat::ADX, eFeat::RDSEED, eFeat::MONITORX, eFeat::SHA, eFeat::CLZERO, eFeat::AES, eFeat::CLMUL, eFeat::CMPXCHG16B, eFeat::MOVBE, eFeat::MMX, eFeat::SSE1, eFeat::SSE2, eFeat::SSE3, eFeat::SSE4_A, eFeat::SSSE3, eFeat::SSE4_1, eFeat::SSE4_2, eFeat::ABM, /*eFeat::XSAVEC, eFeat::XSAVES,*/ eFeat::CLFLUSHOPT, eFeat::POPCNT, eFeat::RDPID, eFeat::WBNOINVD, /*eFeat::PKU,*/ eFeat::VPCLMULQDQ, eFeat::VAES, eFeat::AVX512_F, eFeat::AVX512_DQ, eFeat::AVX512_IFMA, eFeat::AVX512_CD, eFeat::AVX512_BW, eFeat::AVX512_VL, eFeat::AVX512_BF16, eFeat::AVX512_VBMI, eFeat::AVX512_VBMI2, eFeat::AVX512_VNNI, eFeat::AVX512_BITALG, eFeat::AVX512_VPOPCNTDQ, eFeat::GFNI};
    static constexpr std::array c_FeatDiffZenVer5 = { eFeat::AVX_VNNI, eFeat::MOVDIRI, eFeat::MOVDIR64B, eFeat::AVX512_VP2INTERSECT, eFeat::PREFETCHI };
    static constexpr std::array c_FeatDiffZenVer6 = { eFeat::AVX_VNNI_INT8, eFeat::AVX_IFMA, eFeat::AVX512_FP16, eFeat::AVX_NE_CONVERT, eFeat::AVX512_BMM };
#endif //X_PMBB_ARCH_AMD64

#if defined(X_PMBB_ARCH_ARM64) 
    static constexpr std::array c_FeaturesARM64v8p0     = { eFeat::FP, eFeat::ASIMD };
    static constexpr std::array c_FeatDiffARM64v8p2     = { eFeat::CRC32, eFeat::ATOMICS, eFeat::FPHP, eFeat::ASIMDHP };
    static constexpr std::array c_FeatDiffARM64v8p2_DPC = { eFeat::ASIMDDP, eFeat::PMULL };
#endif //X_PMBB_ARCH_ARM64

  protected:
    std::vector<bool> m_Exts = std::vector<bool>((int32_t)(eFeat::NUM_OF_FEATURES), false);

  public:
    inline void set(eFeat Feature, bool Val)       { m_Exts[(int32_t)Feature] = m_Exts[(int32_t)Feature] || Val; }
    inline bool has(eFeat Feature          ) const { return m_Exts[(int32_t)Feature]; }
    
    bool hasFeats(const std::vector<eFeat>& Features) const;

#if defined(X_PMBB_ARCH_AMD64) 
    inline bool hasSSEx() const { return has(eFeat::SSE1) && has(eFeat::SSE2) && has(eFeat::SSE3) && has(eFeat::SSSE3) && has(eFeat::SSE4_1) && has(eFeat::SSE4_2); }
    inline bool hasAVX1() const { return hasSSEx() && has(eFeat::AVX1); }
    inline bool hasAVX2() const { return hasAVX1() && has(eFeat::AVX2); }
    inline bool hasFMA () const { return hasAVX2() && has(eFeat::FMA3); }

    inline bool matchesAMD64v1() const { return hasFeats({ c_FeaturesAMD64v1.cbegin(), c_FeaturesAMD64v1.cend() }); }
    inline bool matchesAMD64v2() const { return matchesAMD64v1() && hasFeats({ c_FeatDiffAMD64v2.cbegin(), c_FeatDiffAMD64v2.cend() }); }
    inline bool matchesAMD64v3() const { return matchesAMD64v2() && hasFeats({ c_FeatDiffAMD64v3.cbegin(), c_FeatDiffAMD64v3.cend() }); }
    inline bool matchesAMD64v4() const { return matchesAMD64v3() && hasFeats({ c_FeatDiffAMD64v4.cbegin(), c_FeatDiffAMD64v4.cend() }); }
    inline bool matchesZenVer4() const { return hasFeats({ c_FeaturesZenVer4.cbegin(), c_FeaturesZenVer4.cend() }); }
    inline bool matchesZenVer5() const { return matchesZenVer4() && hasFeats({ c_FeatDiffZenVer5.cbegin(), c_FeatDiffZenVer5.cend() }); }
#endif //X_PMBB_ARCH_AMD64

#if defined(X_PMBB_ARCH_ARM64)
    inline bool matchesARM64v8p0    () const { return true; } //theoretically has(eFeat::FP) && has(eFeat::ASIMD)
    inline bool matchesARM64v8p2    () const { return matchesARM64v8p0() && hasFeats({ c_FeatDiffARM64v8p2    .cbegin(), c_FeatDiffARM64v8p2    .cend() }); }
    inline bool matchesARM64v8p2_DPC() const { return matchesARM64v8p2() && hasFeats({ c_FeatDiffARM64v8p2_DPC.cbegin(), c_FeatDiffARM64v8p2_DPC.cend() }); }
#endif //X_PMBB_ARCH_ARM64

    static std::string eFeatToName(eFeat Ext);
  };

public:
  enum class eMFL : int32
  {
    INVALID   = NOT_VALID,
    UNDEFINED = 0,
#if defined(X_PMBB_ARCH_AMD64) 
    AMD64SCLR, //x86-64 no using SIMD autovectorization
    AMD64v1  , //x86-64    : CMOV, CMPXCHG8B, FPU, FXSR, MMX, FXSR, SCE, SSE, SSE2
    AMD64v2  , //x86-64-v2 : x86-64 + CMPXCHG16B, LAHF-SAHF, POPCNT, SSE3, SSE4.1, SSE4.2, SSSE3
    AMD64v3  , //x86-64-v3 : x86-64-v2 + AVX, AVX2, BMI1, BMI2, F16C, FMA, LZCNT, MOVBE, XSAVE
    AMD64v4  , //x86-64-v4 : x86-64-v3 + AVX512F, AVX512BW, AVX512CD, AVX512DQ, AVX512VL
    ZenVer4  , //Zen4      : BMI, BMI2, CLWB, F16C, FMA, FSGSBASE, AVX, AVX2, ADCX, RDSEED, MWAITX, SHA, CLZERO, AES, PCLMUL, CX16, MOVBE, MMX, SSE, SSE2, SSE3, SSE4A, SSSE3, SSE4.1, SSE4.2, ABM, XSAVEC, XSAVES, CLFLUSHOPT, POPCNT, RDPID, WBNOINVD, PKU, VPCLMULQDQ, VAES, AVX512F, AVX512DQ, AVX512IFMA, AVX512CD, AVX512BW, AVX512VL, AVX512BF16, AVX512VBMI, AVX512VBMI2, AVX512VNNI, AVX512BITALG, AVX512VPOPCNTDQ, GFNI
    ZenVer5  , //Zen5      : Zen4 + AVX_VNNI, MOVDIRI, MOVDIR64B, AVX512_VP2INTERSECT, PREFETCHI
    ZenVer6  , //Zen6      : Zen5 + AVX_VNNI_INT8, AVX_IFMA, AVX512_FP16, AVX_NE_CONVERT, AVX512_BMM
#elif defined(X_PMBB_ARCH_ARM64) 
    ARM64v8p0_SCLR, //ARM64v8.0 no using SIMD autovectorization
    ARM64v8p0_AVEC, //ARM64v8.0 no NEON intrinsics - only autovectorization
    ARM64v8p0     , //ARM64v8.0 baseline
    ARM64v8p2     , //ARM64v8.2 = ARM64v8.0 + CRC32 + ATOMICS + FP16ASIMDDP
    ARM64v8p2_DPC , //ARM64v8.2 + ASIMDDP + PMULL
  //ARM64v9_0     
  //ARM64v9_4     
    NUM           ,
#endif //X_PMBB_ARCH_ARM64
  };

  using tMFLV = std::vector<eMFL>;

protected:
  bool   m_ProcInfoChecked = false;
#if defined(X_PMBB_ARCH_AMD64) 
  tStr   m_ManufacturerID;
  tStr   m_BrandString   ;
  int32  m_Family        ;
  int32  m_Model         ;
  int32  m_Stepping      ;
#endif //X_PMBB_ARCH_AMD64

#if defined(X_PMBB_ARCH_ARM64) && defined(X_PMBB_OPERATING_SYSTEM_LINUX)
  tStrV  m_CoreDescriptions;
#endif //defined(X_PMBB_ARCH_ARM64) && defined(X_PMBB_OPERATING_SYSTEM_LINUX)

#if defined(X_PMBB_ARCH_ARM64) && defined(X_PMBB_OPERATING_SYSTEM_DARWIN)
  tStr   m_BrandString;
  int64  m_Family   ;
  int64  m_SubFamily;  
  int64  m_Type     ;
  int64  m_SubType  ;
#endif //defined(X_PMBB_ARCH_ARM64) && defined (X_PMBB_OPERATING_SYSTEM_DARWIN)


  xFeats m_Feats         ;
  bool   m_OSAVX         = false; //OS level AVX support OSXSAVE
  flt64  m_TSC_Frequency = std::numeric_limits<flt64>::quiet_NaN(); //Hz

public:
  void  detectSysInfo();
  tStr  formatSysInfo() const;
  eMFL  determineMicroArchFeatureLevel () const;
  tMFLV determineMicroArchFeatureLevels() const;

public:
  const xFeats& getExts() const { return m_Feats; }

#if defined(X_PMBB_ARCH_AMD64) 
  inline bool hasSSEx() const { return m_Feats.hasSSEx()           ; }
  inline bool hasAVX1() const { return m_Feats.hasAVX1() && m_OSAVX; }
  inline bool hasAVX2() const { return m_Feats.hasAVX2() && m_OSAVX; }
  inline bool hasFMA () const { return m_Feats.hasFMA () && m_OSAVX; }

  inline bool matchesAMD64v1() const { return m_Feats.matchesAMD64v1()           ; }
  inline bool matchesAMD64v2() const { return m_Feats.matchesAMD64v2()           ; }
  inline bool matchesAMD64v3() const { return m_Feats.matchesAMD64v3() && m_OSAVX; }
  inline bool matchesAMD64v4() const { return m_Feats.matchesAMD64v4() && m_OSAVX; }
  inline bool matchesZenVer4() const { return m_Feats.matchesZenVer4() && m_OSAVX; }
#endif //X_PMBB_ARCH_AMD64

#if defined(X_PMBB_ARCH_ARM64)
  inline bool matchesARM64v8p0    () const { return m_Feats.matchesARM64v8p0    (); }
  inline bool matchesARM64v8p2    () const { return m_Feats.matchesARM64v8p2    (); }
  inline bool matchesARM64v8p2_DPC() const { return m_Feats.matchesARM64v8p2_DPC(); }
#endif //X_PMBB_ARCH_ARM64

  static eMFL xStrToMfl(const std::string_view Mfl);
  static tStr xMflToStr(eMFL Mfl);
  static tStr xMflToDescription(eMFL Mfl);

protected:  
  tStr xFormatProcInfo () const;
  tStr xFormatProcFeats() const;
  static tStr xFormatMemInfo();
  
  void xDetectProcInfo();

#if defined(X_PMBB_ARCH_AMD64)
  tStr xFormatProcInfoAMD64() const;
  void xDetectProcInfoAMD64();
  void xDetectCPUID        ();
  void xDetectMSR0         ();
  void xDetectOSAVX        ();
  
#endif //X_PMBB_ARCH_AMD64

#if defined(X_PMBB_ARCH_ARM64)
  tStr xFormatProcInfoARM64() const;
  void xDetectProcInfoARM64();
  void xDetectProcHWCAPs   ();
  void xDetectCntFrequency ();  
#endif //X_PMBB_ARCH_ARM64

#if defined(X_PMBB_ARCH_ARM64) && defined(X_PMBB_OPERATING_SYSTEM_LINUX)
  uint64 xReadMIDR_EL1_SYS   (int32 LogicalCoreIdx);
  uint64 xReadMIDR_EL1_MSR   (int32 LogicalCoreIdx);
  tStr   xInterpretMIDR      (uint64 MIDR);
  void   xDetectVendorModel  (int32 LogicalCoreIdx);
#endif //defined(X_PMBB_ARCH_ARM64) && defined(X_PMBB_OPERATING_SYSTEM_LINUX)

#if defined(X_PMBB_ARCH_ARM64) && defined (X_PMBB_OPERATING_SYSTEM_DARWIN)
  void   xDetectVendorModel();
#endif //defined(X_PMBB_ARCH_ARM64) && defined (X_PMBB_OPERATING_SYSTEM_DARWIN)
};

//=============================================================================================================================================================================

} //end of namespace PMBB
