/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefPMBB-BASE.h"
#include <vector>

namespace PMBB_BASE {

//=============================================================================================================================================================================
// xProcInfo
//=============================================================================================================================================================================

class xProcInfo
{
public:

  enum class eExt
  {
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
    //Westmere               
    AES               , //EAS encryption accelerator 
    CLMUL             , //Carry-less Multiplication
    //Sandy Bridge           
    AVX1              ,
    //Ivy Bridge             
    FP16C             , //Half float convertion
    RDRAND            , //SP 800-90A - Cryptographically secure pseudorandom number generator
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
    //Skylake-X                
    AVX512F           , //Foundation  
    AVX512VL          , //Vector Length Extensions
    AVX512BW          , //Byte and Word Instructions
    AVX512DQ          , //Doubleword and Quadword Instructions
    AVX512CD          , //Conflict Detection Instructions
    //Knights Landing
    AVX512ER          , //Exponential and Reciprocal Instructions
    AVX512PF          , //Prefetch Instructions    
    //Cannonlake
    UMIP              , //User-Mode Instruction Prevention
    AVX512VMBI        , //Vector Byte Manipulation Instructions
    AVX512IFMA        , //Integer Fused Multiply Add (52-bit Integer Multiply-Add)
    //Knights Mill            
    AVX512_4VNNIW     , //4-register Neural Network Instructions / Vector Neural Network Instructions Word variable precision
    AVX512_4FMAPS     , //4-register Multiply Accumulation Single precision
    //Sunny Cove (Ice Lake)
    CLWB              , //Cache Line Write Back
    RDPID             , //Read Processor ID
    AVX512_VNNI       , //Vector Neural Network Instructions
    AVX512_VBMI2      , //Vector Byte Manipulation Instructions 2
    AVX512_BITALG     , //Support for VPOPCNT[B,W] and VPSHUF-BITQMB
    AVX512_VPOPCNTDQ  , //Vector POPCNT
    VPCLMULQDQ        , //Carry-Less Multiplication Quadword
    VAES              , //Vector AES
    GFNI              , //Galois Field New Instructions
    //Willow Cove (Tiger Lake)
    AVX512_VP2INTERSECT,
    //Willow Cove (Sapphire Rapids)
    AVX512_BF16       ,
    AVX512_FP16       ,
    AMX_BF16          ,
    AMX_TILE          ,
    AMX_INT8          ,
    //Golden Cove
    HYBRID            ,
    //???


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

    //TYPE
    NUM_OF_EXTS
  };

  class xExts
  {
  protected:
    std::vector<bool> m_Exts = std::vector<bool>((int32_t)(eExt::NUM_OF_EXTS), false);

  public:
    inline void setExt(eExt Ext, bool Val) { m_Exts[(int32_t)Ext] = Val; }
    inline bool hasExt(eExt Ext) const { return m_Exts[(int32_t)Ext]; }

    inline bool hasSSEx() const { return hasExt(eExt::SSE1) && hasExt(eExt::SSE2) && hasExt(eExt::SSE3) && hasExt(eExt::SSSE3) && hasExt(eExt::SSE4_1) && hasExt(eExt::SSE4_2); }
    inline bool hasAVX1() const { return hasSSEx() && hasExt(eExt::AVX1); }
    inline bool hasAVX2() const { return hasAVX1() && hasExt(eExt::AVX2); }
    inline bool hasFMA () const { return hasAVX2() && hasExt(eExt::FMA3); }

    inline bool matchesAMD64v1() const { return hasExt(eExt::CMOV) && hasExt(eExt::CMPXCHG8B) && hasExt(eExt::FPU) && hasExt(eExt::FXRS) && hasExt(eExt::MMX) && hasExt(eExt::SSE1) && hasExt(eExt::SSE2); }
    inline bool matchesAMD64v2() const { return hasExt(eExt::CMPXCHG16B) && hasExt(eExt::LAHF_SAHF) && hasExt(eExt::POPCNT) && hasExt(eExt::SSE3) && hasExt(eExt::SSSE3) && hasExt(eExt::SSE4_1) && hasExt(eExt::SSE4_2); }
    inline bool matchesAMD64v3() const { return hasExt(eExt::AVX1) && hasExt(eExt::AVX2) && hasExt(eExt::BMI1) && hasExt(eExt::BMI2) && hasExt(eExt::FP16C) && hasExt(eExt::FMA3) && hasExt(eExt::LZCNT) && hasExt(eExt::MOVBE); }
    inline bool matchesAMD64v4() const { return hasExt(eExt::AVX512F) && hasExt(eExt::AVX512BW) && hasExt(eExt::AVX512CD) && hasExt(eExt::AVX512DQ) && hasExt(eExt::AVX512VL); }

    static std::string eExtToName(eExt Ext);
  };

  class xMem
  {
  protected:
    int32_t m_CacheLineSize  = 0;
    int32_t m_MemoryPageSize = 0;

  public:
    void setCacheLineSize (int32_t CacheLineSize ) { m_CacheLineSize  = CacheLineSize ; }
    void setMemoryPageSize(int32_t MemoryPageSize) { m_MemoryPageSize = MemoryPageSize; }

    int32_t getCacheLineSize () const { return m_CacheLineSize ; }
    int32_t getMemoryPageSize() const { return m_MemoryPageSize; }
  };

public:
  enum class eMFL : int32
  {
    INVALID   = NOT_VALID,
    UNDEFINED = 0,
    AMD64v1   = 1,
    AMD64v2   = 2,
    AMD64v3   = 3,
    AMD64v4   = 4,
    LAST      = 4
  };

protected:
  bool  m_ExtsChecked = false;
  bool  m_MemChecked  = false;
  char  m_VendorID[13] = { 0 };
  xExts m_Exts; 
  xMem  m_Mem;
  bool  m_OSAVX = false; //OS level AVX support OSXSAVE

public:
  void        detectSysInfo();
  std::string formatSysInfo();
  eMFL        determineMicroArchFeatureLevel();

public:
  const xExts& getExts() const { return m_Exts; }

  inline bool hasSSEx() const { return m_Exts.hasSSEx()           ; }
  inline bool hasAVX1() const { return m_Exts.hasAVX1() && m_OSAVX; }
  inline bool hasAVX2() const { return m_Exts.hasAVX2() && m_OSAVX; }
  inline bool hasFMA () const { return m_Exts.hasFMA () && m_OSAVX; }

  inline bool matchesAMD64v1() const { return m_Exts.matchesAMD64v1()           ; }
  inline bool matchesAMD64v2() const { return m_Exts.matchesAMD64v2()           ; }
  inline bool matchesAMD64v3() const { return m_Exts.matchesAMD64v3() && m_OSAVX; }
  inline bool matchesAMD64v4() const { return m_Exts.matchesAMD64v4() && m_OSAVX; }

  static eMFL        xStrToMfl(const std::string_view Mfl);
  static std::string xMflToStr(eMFL Mfl);

protected:  
  static std::string xFormatProcExts(const xExts& Exts);
  static std::string xFormatMemInfo (const xMem&  Mem );
  
  void xDetectExts();
  void xDetectMem ();

  static xExts xDetectProcExts(uint32_t HighestFunctionSupported);
  static bool  xDetectOSAVX   ();
};

//=============================================================================================================================================================================

} //end of namespace PMBB
