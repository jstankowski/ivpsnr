/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "../src/xCommonDefCORE.h"
#include "xThreadPool.h"
#include "xTimeUtils.h"
#include "xMemory.h"
#include <atomic>

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

static const int32 TestNumTasks   = 8192;
static const int32 TestQueueSize  = TestNumTasks + 16;
static const int32 TestNumThreads = 16;

//===============================================================================================================================================================================================================

struct xData
{
  int64  TimeProc;
  int32  ThreadIdx;
  int32  TaskIdx;
};

void testTask(int32 ThreadIdx, int32 TaskIdx, xData* Data, std::atomic_int32_t* Counter)
{
  int64 Beg = xTSC();
  std::this_thread::yield();
  (*Counter)++;
  std::this_thread::yield();
  int64 End = xTSC();
  Data[TaskIdx] = { End - Beg, ThreadIdx, TaskIdx};
}

void testPerTaskInterface()
{
  xThreadPool ThreadPool;
  ThreadPool.create(TestNumThreads, TestQueueSize);
  xThreadPoolInterfaceFunction ThPI;
  ThPI.init(&ThreadPool, TestQueueSize, TestQueueSize);

  xData* Data = (xData*)xMemory::AlignedMalloc(TestNumTasks * sizeof(xData));
  memset(Data, 0xFF, TestNumTasks * sizeof(xData));
  std::atomic_int32_t Cnt = 0;

  uint64 TP0 = xTSC();

  for(int32 i = 0; i < TestNumTasks; i++)
  {
    ThPI.addWaitingTask([i, &Data, &Cnt](int32 ThreadIdx) { testTask(ThreadIdx, i, Data, &Cnt); });
  }

  uint64 TP1 = xTSC();

  int32 WaitCount = 0;
  while(ThPI.getCompletedQueueLoad() < TestNumTasks)
  {
    WaitCount++;
    std::this_thread::sleep_for(tDurationMS(1));
  }

  uint64 TP2 = xTSC();

  ThPI.waitUntilTasksFinished(TestNumTasks);

  uint64 TP3 = xTSC();

  ThPI.uninit();
  ThreadPool.destroy();

  CHECK(Cnt == TestNumTasks);

  uint64 Total = 0;
  for(int32 i = 0; i < TestNumTasks; i++)
  {
    CHECK(Data[i].TaskIdx >= 0          );
    CHECK(Data[i].TaskIdx < TestNumTasks);
    CHECK(Data[i].ThreadIdx >= 0             );
    CHECK(Data[i].ThreadIdx <  TestNumThreads);
    Total += Data[i].TimeProc;
  }

  fmt::print("WC={:<2d} TS={:<10d}                                 TW={:<10d} ticks TT={:.2f} ticks/task\n", WaitCount, TP1 - TP0, TP3 - TP2, (flt64)Total / (flt64)TestNumTasks);
}

void testBulkInterface()
{
  xThreadPool ThreadPool;
  ThreadPool.create(TestNumThreads, TestQueueSize);
  xThreadPoolInterfaceFunction ThPI;
  ThPI.init(&ThreadPool, TestQueueSize, TestQueueSize);

  xData* Data = (xData*)xMemory::AlignedMalloc(TestNumTasks * sizeof(xData));
  memset(Data, 0xFF, TestNumTasks * sizeof(xData));
  std::atomic_int32_t Cnt = 0;

  uint64 TP0 = xTSC();

  for(int32 i = 0; i < TestNumTasks; i++)
  {
    ThPI.storeTask([i, &Data, &Cnt](int32 ThreadIdx) { testTask(ThreadIdx, i, Data, &Cnt); });
  }

  uint64 TP1 = xTSC();

  ThPI.submitStoredTasks();

  uint64 TP2 = xTSC();

  int32 WaitCount = 0;
  while(ThPI.getCompletedQueueLoad() < TestNumTasks)
  {
    WaitCount++;
    std::this_thread::sleep_for(tDurationMS(1));
  }

  uint64 TP3 = xTSC();

  ThPI.waitUntilTasksFinished(TestNumTasks);

  uint64 TP4 = xTSC();

  ThPI.uninit();
  ThreadPool.destroy();

  CHECK(Cnt == TestNumTasks);

  uint64 Total = 0;
  for(int32 i = 0; i < TestNumTasks; i++)
  {
    CHECK(Data[i].TaskIdx >= 0          );
    CHECK(Data[i].TaskIdx < TestNumTasks);
    CHECK(Data[i].ThreadIdx >= 0             );
    CHECK(Data[i].ThreadIdx <  TestNumThreads);
    Total += Data[i].TimeProc;
  }

  fmt::print("WC={:<2d} TS={:<10d} (ST={:<10d} + SS={:<10d}) TW={:<10d} ticks TT={:.2f} ticks/task\n", WaitCount, TP2 - TP0, TP1 - TP0, TP2 - TP1, TP4 - TP3, (flt64)Total/(flt64)TestNumTasks);
}

//===============================================================================================================================================================================================================

TEST_CASE("A")
{
  testPerTaskInterface();
}

TEST_CASE("B")
{
  testBulkInterface();
}

//===============================================================================================================================================================================================================
