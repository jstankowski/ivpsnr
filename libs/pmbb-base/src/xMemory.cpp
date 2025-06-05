/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
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
  CacheLineSize = LineSize;

#endif //X_PMBB_OPERATING_SYSTEM_LINUX

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

#endif //X_PMBB_OPERATING_SYSTEM_LINUX

  return PageSize;
}

int64_t xDetectMemoryHugePageSize()
{
  int64_t PageSizeBase = xDetectMemoryBasePageSize();
  if(PageSizeBase == NOT_VALID) { return NOT_VALID;}

#if defined(X_PMBB_OPERATING_SYSTEM_WINDOWS)

  size_t  LargePageMinimum = (int64_t)GetLargePageMinimum();
  int64_t PageSizeHuge = LargePageMinimum !=0 ? (int64_t)LargePageMinimum : NOT_VALID;
  return PageSizeHuge;
 
#elif defined(X_PMBB_OPERATING_SYSTEM_LINUX)

  //expected page sizes
  std::vector<int64_t> ExpectedPageSizes;
  #if defined(X_PMBB_ARCH_AMD64)
  ExpectedPageSizes = {2097152 /*2MB*/, 1073741824 /*1GB*/};
  #endif //X_PMBB_ARCH_AMD64
  
  //declared page sizes
  std::vector<int64_t> KnownPageSizes;  
  //search for all available page sizes
  DIR* dirp = opendir("/sys/kernel/mm/hugepages");
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

  if(KnownPageSizes.empty()) { return NOT_VALID;}
   
  int64_t PageSizeHuge = NOT_VALID;
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

  return PageSizeHuge;
#endif //X_PMBB_OPERATING_SYSTEM_LINUX
}

} //end of namespace


namespace PMBB_BASE {

//=============================================================================================================================================================================
// xMemory
//=============================================================================================================================================================================

const int64  xMemory::c_DetectedCacheLine = xDetectCacheLineSize     ();
const int64  xMemory::c_DetectedPageBase  = xDetectMemoryBasePageSize();
const int64  xMemory::c_DetectedPageHuge  = xDetectMemoryHugePageSize();

const uint64 xMemory::c_MemSizeCacheLine = c_DetectedCacheLine != NOT_VALID ? c_DetectedCacheLine : 0;
const uint64 xMemory::c_MemSizePageBase  = c_DetectedPageBase  != NOT_VALID ? c_DetectedPageBase  : 0;
const uint64 xMemory::c_MemSizePageHuge  = c_DetectedPageHuge  != NOT_VALID ? c_DetectedPageHuge  : 0;

const uint32 xMemory::c_Log2MemSizeCacheLine = (uint32)xLog2(c_MemSizeCacheLine);
const uint32 xMemory::c_Log2MemSizePageBase  = (uint32)xLog2(c_MemSizePageBase );
const uint32 xMemory::c_Log2MemSizePageHuge  = (uint32)xLog2(c_MemSizePageHuge );

const uint32 xMemory::c_SizeMaskCacheLine = (1<<c_Log2MemSizeCacheLine) - 1;
const uint32 xMemory::c_SizeMaskPageBase  = (1<<c_Log2MemSizePageBase ) - 1;
const uint32 xMemory::c_SizeMaskPageHuge  = (1<<c_Log2MemSizePageHuge ) - 1;

const uint32 xMemory::c_AllocThresholdPageBase = c_DetectedPageBase  != NOT_VALID ? xCalcAllocThreshold(c_SizeMaskPageBase) : xCalcAllocThreshold(xc_MemSizePageDef);
const uint32 xMemory::c_AllocThresholdPageHuge = c_DetectedPageHuge  != NOT_VALID ? xCalcAllocThreshold(c_SizeMaskPageHuge) : std::numeric_limits<uint32>::max();

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
