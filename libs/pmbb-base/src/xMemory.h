/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefBASE.h"
#include "xMemoryAlign.h"

namespace PMBB_BASE {

//=============================================================================================================================================================================
// xMemmory
//=============================================================================================================================================================================

//TODO: consider Meyers' Singleton (Lazy Initialization) for initialization safety

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

protected:
  static const uint64 c_MemSizeCacheLine;
  static const uint64 c_MemSizePageBase ;
  static const uint64 c_MemSizePageHuge ;

  static const uint32 c_Log2MemSizeCacheLine;
  static const uint32 c_Log2MemSizePageBase ;
  static const uint32 c_Log2MemSizePageHuge ;

  static const uint32 c_SizeMaskCacheLine;
  static const uint32 c_SizeMaskPageBase ;
  static const uint32 c_SizeMaskPageHuge ;

  static const uint32 c_AllocThresholdPageBase;
  static const uint32 c_AllocThresholdPageHuge;

public:
  //Allocation with explicit alignment
#if defined(X_PMBB_COMPILER_MSVC)
  static inline void* xAlignedMalloc(uintSize Size, uintSize Alignment) { return _aligned_malloc(Size, Alignment); }
  static inline void  xAlignedFree  (void* Memmory) { _aligned_free(Memmory); }
#elif defined (__ANDROID__)
  static inline void* xAlignedMalloc(uintSize Size, uintSize Alignment) { void* Ptr = nullptr; posix_memalign(&Ptr, Alignment, Size); return Ptr; }
  static inline void  xAlignedFree(void* Memmory) { free(Memmory); }
#else
  static inline void* xAlignedMalloc(uintSize Size, uintSize Alignment) { return aligned_alloc(Alignment, Size); }
  static inline void  xAlignedFree  (void* Memmory) { free(Memmory); }
#endif
  //static inline void  xAlignedFreeNull(void*& Memmory) { xAlignedFree(Memmory); Memmory = nullptr; }
  template <class XXX> static inline void xAlignedFreeNull(XXX*& Memmory) { xAlignedFree(Memmory); Memmory = nullptr; }

  static void* xAlignedMallocCacheLine(uintSize Size);
  static void* xAlignedMallocPageBase (uintSize Size);
  static void* xAlignedMallocPageHuge (uintSize Size);
  static void* xAlignedMallocPageAuto (uintSize Size);
  static void* xAlignedMallocAuto     (uintSize Size);

  static void* AlignedMalloc         (uintSize Size, eMemAlignment Alignment = eMemAlignment::Auto);

  static inline uint64 getRealSizeCacheLine() { return c_MemSizeCacheLine; } //may return 0 if value is not known
  static inline uint64 getRealSizePageBase () { return c_MemSizePageBase ; } //may return 0 if value is not known
  static inline uint64 getRealSizePageHuge () { return c_MemSizePageHuge ; } //may return 0 if value is not known
  
  static inline uint64 getBestEffortSizePageBase    () { return c_MemSizePageBase     ? c_MemSizePageBase     : xc_MemSizePageDef    ; }
  static inline uint64 getBestEffortLog2SizePageBase() { return c_Log2MemSizePageBase ? c_Log2MemSizePageBase : xc_Log2MemSizePageDef; }

  static inline uint64 RoundUpToPageSizeBase(uint64 Value) { return xRoundUpToNearestMultiple(Value, getBestEffortLog2SizePageBase()); }

protected:
  static uint64 xCvtSize(int64 Size) { return Size != NOT_VALID ? Size : 0; }
  static uint64 xLog2   (uint64 Val) { return (Val > 1) ? 1 + xLog2(Val >> 1) : 0; } //positive integer only
  static uint64 xRoundUpToNearestMultiple(uint64 Value, uint64 Log2Multiple) { return (((Value + ((1 << Log2Multiple) - 1)) >> Log2Multiple) << Log2Multiple); } //positive integer only
  
  static uint32 xCalcAllocThreshold(uint32 PageSize) { return (PageSize >> 1) + (PageSize >> 2) + (PageSize >> 3); }

  static inline bool xWorthUseHuge(uintSize Size) { return (Size > (c_MemSizePageHuge  << 1)) || ((Size & c_SizeMaskPageHuge ) > c_AllocThresholdPageHuge); }
  static inline bool xWorthUseBase(uintSize Size) { return (Size > (c_MemSizePageBase  << 1)) || ((Size & c_SizeMaskPageBase ) > c_AllocThresholdPageBase); }
  static inline bool xWorthUseLine(uintSize Size) { return (Size > (c_MemSizeCacheLine << 2)) || ((Size & c_SizeMaskCacheLine) == 0); }
};

//=============================================================================================================================================================================

} //end of namespace PMBB
