/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xMemory.h"

#ifdef X_PMBB_OPERATING_SYSTEM_WINDOWS
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #undef WIN32_LEAN_AND_MEAN

  //clanup max min crap
  #ifdef max
    #undef max
  #endif
  #ifdef min
    #undef min
  #endif
#endif //X_PMBB_OPERATING_SYSTEM_WINDOWS

#ifdef X_PMBB_OPERATING_SYSTEM_LINUX
  #if __has_include(<unistd.h>)
    #define X_PMBB_SYSTEM_UNISTD 1
    #include <unistd.h>
  #endif
  #include <dirent.h>
#endif //X_PMBB_OPERATING_SYSTEM_LINUX

#ifdef X_PMBB_OPERATING_SYSTEM_DARWIN
  #include <sys/types.h>
  #include <sys/sysctl.h>
#endif //X_PMBB_OPERATING_SYSTEM_DARWIN


//=============================================================================================================================================================================
// Helper functions - memory
//=============================================================================================================================================================================

namespace {

int32_t xDetectCacheLineSize()
{
  int32_t CacheLineSize = NOT_VALID;

#if defined(X_PMBB_OPERATING_SYSTEM_WINDOWS)

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
  CacheLineSize = LineSize;

#elif defined(X_PMBB_OPERATING_SYSTEM_LINUX)

  FILE* File = 0;
  File = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
  int LineSize = 0;
  if(File != nullptr)
  {
    int Result = fscanf(File, "%d", &LineSize);
    fclose(File);
    if(Result != 1) { return 0; }
  }
#  if defined(X_PMBB_ARCH_ARM64)
  else
  {
    uint64_t CTR_EL0 = 0;
    asm volatile ("mrs %0, ctr_el0;" : "=r"(CTR_EL0) :: "memory");
    uint64_t CTR_DminLine = (CTR_EL0 >> 16) & 0xF;
    LineSize = (int32_t)(4 << CTR_DminLine);
  }
#  endif //defined(X_PMBB_ARCH_ARM64)

  CacheLineSize = LineSize;

#elif defined(X_PMBB_OPERATING_SYSTEM_DARWIN)

  int64_t Ret = 0;
  size_t  Size = sizeof(Ret);

  if(sysctlbyname("hw.cachelinesize", &Ret, &Size, NULL, 0) == 0)
  {
    CacheLineSize = (int32_t)Ret;
  }

#endif //X_PMBB_OPERATING_SYSTEM

  return CacheLineSize;
}

int64_t xDetectMemoryBasePageSize()
{
  int64_t PageSize = NOT_VALID;

#if defined(X_PMBB_OPERATING_SYSTEM_WINDOWS) || defined(X_PMBB_OPERATING_SYSTEM_UNIX) || defined(X_PMBB_OPERATING_SYSTEM_MACOS)

  SYSTEM_INFO SysInfo;
  GetSystemInfo(&SysInfo);
  PageSize = SysInfo.dwPageSize;

#elif defined(X_PMBB_OPERATING_SYSTEM_LINUX)

  PageSize = sysconf(_SC_PAGE_SIZE);

#elif defined(X_PMBB_OPERATING_SYSTEM_DARWIN)

  int64_t Ret = 0;
  size_t  Size = sizeof(Ret);

  if(sysctlbyname("hw.pagesize", &Ret, &Size, NULL, 0) == 0)
  {
    PageSize = Ret;
  }

#endif //X_PMBB_OPERATING_SYSTEM_LINUX

  return PageSize;
}

int64_t xDetectMemoryHugePageSize()
{
  //https://wiki.debian.org/Hugepages
  //https://docs.kernel.org/arch/arm64/hugetlbpage.html
  
  int64_t PageSizeBase = xDetectMemoryBasePageSize();
  if(PageSizeBase == NOT_VALID) { return NOT_VALID;}

  int64_t PageSizeHuge = NOT_VALID;

#if defined(X_PMBB_OPERATING_SYSTEM_WINDOWS)

  size_t  LargePageMinimum = (int64_t)GetLargePageMinimum();
  PageSizeHuge = LargePageMinimum !=0 ? (int64_t)LargePageMinimum : NOT_VALID;
 
#elif defined(X_PMBB_OPERATING_SYSTEM_LINUX)

  //expected page sizes
  std::vector<int64_t> ExpectedPageSizes;
  #if defined(X_PMBB_ARCH_AMD64)
  ExpectedPageSizes = {2097152 /*2MB*/, 1073741824 /*1GB*/};
  #endif //X_PMBB_ARCH_AMD64

  #if defined(X_PMBB_ARCH_ARM64)
  switch(PageSizeBase)
  {
    case 4096 /* 4K*/: ExpectedPageSizes = {2097152   /*2  MB*/, 1073741824 /*1GB*/}; break;
    case 16384/*16K*/: ExpectedPageSizes = {33554432  /*32 MB*/                    }; break;
    case 65536/*64K*/: ExpectedPageSizes = {536870912 /*512MB*/                    }; break;
    default: break;
  }
  #endif //X_PMBB_ARCH_ARM64
  
  //declared page sizes
  std::vector<int64_t> KnownPageSizes;  
  //search for all available page sizes
  DIR* dirp = opendir("/sys/kernel/mm/hugepages");
  if(dirp != nullptr)
  {
    for(;; )
    {
      struct dirent* dirent = readdir(dirp);
      if(NULL == dirent) break;

      if('.' == dirent->d_name[0]) continue;
      char* p = strchr(dirent->d_name, '-');
      if(NULL == p) continue;

      uint64_t CurrPageSize = 1024ULL * strtoull(p + 1, NULL, 0);
      KnownPageSizes.push_back((int64_t)CurrPageSize);
      //MinPageSize = std::min(MinPageSize, CurrPageSize);
    }
    closedir(dirp);
  }

  if(KnownPageSizes.empty()) { return NOT_VALID;}
   
  if(ExpectedPageSizes.empty())
  {
    PageSizeHuge = KnownPageSizes[0];
  }
  else
  {
    for(int64_t EPS : ExpectedPageSizes)
    {
      if(std::find(KnownPageSizes.begin(), KnownPageSizes.end(), EPS) != KnownPageSizes.end())
      {
        PageSizeHuge = EPS;
        break;
      }
    }
  }

#endif //X_PMBB_OPERATING_SYSTEM

  return PageSizeHuge;
}

} //end of namespace


namespace PMBB_BASE {

//=============================================================================================================================================================================
// xMemory
//=============================================================================================================================================================================

#ifdef __cpp_lib_hardware_interference_size
#if defined (X_PMBB_ARCH_AMD64)
  static_assert(xMemory::xc_CacheLineSizeDef == std::hardware_constructive_interference_size);
  static_assert(xMemory::xc_CacheLineSizeDef == std::hardware_destructive_interference_size );
#elif defined (X_PMBB_ARCH_ARM64)
  static_assert(xMemory::xc_CacheLineSizeDef == std::hardware_constructive_interference_size);
  //NOTE: ARM64 defines std::hardware_destructive_interference_size as 256 (A64FX uses 256 byte cache lines), however, cores with cache line different than 64 bytes are exotic. https://github.com/gcc-mirror/gcc/commit/76b75018b3d053a890ebe155e47814de14b3c9fb
#endif
#endif


const uint64 xMemory::c_MemSizeCacheLine     = xCvtSize(xDetectCacheLineSize     ());
const uint64 xMemory::c_MemSizePageBase      = xCvtSize(xDetectMemoryBasePageSize());
const uint64 xMemory::c_MemSizePageHuge      = xCvtSize(xDetectMemoryHugePageSize());

const uint32 xMemory::c_Log2MemSizeCacheLine = (uint32)xLog2(c_MemSizeCacheLine);
const uint32 xMemory::c_Log2MemSizePageBase  = (uint32)xLog2(c_MemSizePageBase );
const uint32 xMemory::c_Log2MemSizePageHuge  = (uint32)xLog2(c_MemSizePageHuge );

const uint32 xMemory::c_SizeMaskCacheLine    = (1<<c_Log2MemSizeCacheLine) - 1;
const uint32 xMemory::c_SizeMaskPageBase     = (1<<c_Log2MemSizePageBase ) - 1;
const uint32 xMemory::c_SizeMaskPageHuge     = (1<<c_Log2MemSizePageHuge ) - 1;

const uint32 xMemory::c_AllocThresholdPageBase = c_MemSizePageBase ? xCalcAllocThreshold(c_SizeMaskPageBase) : xCalcAllocThreshold(xc_MemSizePageDef);
const uint32 xMemory::c_AllocThresholdPageHuge = c_MemSizePageHuge ? xCalcAllocThreshold(c_SizeMaskPageHuge) : std::numeric_limits<uint32>::max();

void* xMemory::xAlignedMallocCacheLine(uintSize Size)
{
  if(c_MemSizeCacheLine) { return xAlignedMalloc(xRoundUpToNearestMultiple(Size, (uintSize) c_Log2MemSizeCacheLine), c_MemSizeCacheLine ); }
  else                   { return xAlignedMalloc(xRoundUpToNearestMultiple(Size, (uintSize)xc_Log2CacheLineSizeDef), xc_CacheLineSizeDef); }
}
void* xMemory::xAlignedMallocPageBase(uintSize Size)
{
  if(c_MemSizePageBase) { return xAlignedMalloc(xRoundUpToNearestMultiple(Size, (uintSize)c_Log2MemSizePageBase), c_MemSizePageBase); }
  else                  { return xAlignedMalloc(xRoundUpToNearestMultiple(Size, (uintSize)xc_Log2MemSizePageDef), xc_MemSizePageDef); }
}
void* xMemory::xAlignedMallocPageHuge(uintSize Size)
{
  if(!c_MemSizePageHuge) { return xAlignedMallocPageBase(Size); }

  uintPtr NewSize = xRoundUpToNearestMultiple(Size, (uintSize)c_Log2MemSizePageHuge);
  void*   Memmory = xAlignedMalloc(NewSize, c_MemSizePageHuge);
#ifdef MADV_HUGEPAGE
  if(Memmory != nullptr) { madvise(Memmory, NewSize, MADV_HUGEPAGE); }
#endif
  return Memmory;
}
void* xMemory::xAlignedMallocPageAuto(uintSize Size)
{
  bool UsePageHuge = c_MemSizePageHuge && xWorthUseHuge(Size);
  return UsePageHuge ? xAlignedMallocPageHuge(Size) : xAlignedMallocPageBase(Size);
}
void* xMemory::xAlignedMallocAuto(uintSize Size)
{  
  bool UsePageHuge = c_MemSizePageHuge && xWorthUseHuge(Size);
  if(UsePageHuge) { return xAlignedMallocPageHuge(Size); }
  bool UsePageBase = c_MemSizePageBase && xWorthUseBase(Size);
  if(UsePageBase) { return xAlignedMallocPageBase(Size); }
  bool UseLineSize = c_MemSizeCacheLine && xWorthUseLine(Size);
  if(UseLineSize) { return xAlignedMallocCacheLine(Size); }
  return xAlignedMalloc(Size, 1);
}
void* xMemory::AlignedMalloc(uintSize Size, eMemAlignment Alignment)
{
  switch(Alignment)
  {
  case eMemAlignment::None     : return xAlignedMalloc      (Size, 1); break;
  case eMemAlignment::CacheLine: return xAlignedMallocCacheLine(Size); break;
  case eMemAlignment::PageBase : return xAlignedMallocPageBase (Size); break;
  case eMemAlignment::PageHuge : return xAlignedMallocPageHuge (Size); break;
  case eMemAlignment::PageAuto : return xAlignedMallocPageAuto (Size); break;
  case eMemAlignment::Auto     : return xAlignedMallocAuto     (Size); break;
  default                      : return xAlignedMallocAuto     (Size); break;
  }
}

//=============================================================================================================================================================================

} //end of namespace PMBB
