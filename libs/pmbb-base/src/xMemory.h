/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefPMBB-BASE.h"

namespace PMBB_BASE {

//=============================================================================================================================================================================
// xProcInfo
//=============================================================================================================================================================================

class xMemory
{
public:
  enum class eMemAlignment
  {
    None = 0,
    CacheLine,
    PageBase,
    PageHuge,
    PageAuto,
    Auto
  };

  static constexpr eMemAlignment xc_DefAlignment = eMemAlignment::Auto;

  static constexpr uint32 xc_Log2CacheLineSizeDef = 6; //default cache line size = 64B
  static constexpr uint32 xc_CacheLineSizeDef     = (1 << xc_Log2CacheLineSizeDef);
  static constexpr uint32 xc_Log2MemSizePageDef   = 12; //default base page size = 4kB
  static constexpr uint32 xc_MemSizePageDef       = (1 << xc_Log2MemSizePageDef);

public:
  static const int64  c_DetectedCacheLine;
  static const int64  c_DetectedPageBase;
  static const int64  c_DetectedPageHuge;

  static const uint64 c_MemSizeCacheLine;
  static const uint64 c_MemSizePageBase ;
  static const uint64 c_MemSizePageHuge ;

  static const uint32 c_Log2MemSizeCacheLine;
  static const uint32 c_Log2MemSizePageBase ;
  static const uint32 c_Log2MemSizePageHuge ;

public:
  //Allocation with explicit alignment
#if defined(X_COMPILER_MSVC)
  static inline void* xAlignedMalloc(uintSize Size, uintSize Alignment) { return _aligned_malloc(Size, Alignment); }
  static inline void  xAlignedFree  (void* Memmory) { _aligned_free(Memmory); }
#else
  static inline void* xAlignedMalloc(uintSize Size, uintSize Alignment) { return aligned_alloc(Alignment, Size); }
  static inline void  xAlignedFree  (void* Memmory) { free(Memmory); }
#endif

  static void* xAlignedMallocCacheLine(uintSize Size);
  static void* xAlignedMallocPageBase (uintSize Size);
  static void* xAlignedMallocPageHuge (uintSize Size);
  static void* xAlignedMallocPageAuto (uintSize Size);
  static void* xAlignedMallocAuto     (uintSize Size);

  static void* AlignedMalloc         (uintSize Size, eMemAlignment Alignment = eMemAlignment::Auto);

protected:
  static uint64 xLog2(uint64 Val) { return (Val > 1) ? 1 + xLog2(Val >> 1) : 0; } //positive integer only
  static uint64 xRoundUpToNearestMultiple(uint64 Value, uint64 Log2Multiple) { return (((Value + ((1 << Log2Multiple) - 1)) >> Log2Multiple) << Log2Multiple); } //positive integer only
};

//=============================================================================================================================================================================

} //end of namespace PMBB
