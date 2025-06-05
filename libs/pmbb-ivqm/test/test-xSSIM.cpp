/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <functional>
#include <utility>
#include <array>
#include "xTestUtils.h"
#include "xMemory.h"
#include "xSSIM.h"

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

class xSSIM_Test : public xSSIM
{
public:
  static int32 calcNumBlocks(int32 Length, int32 BlockSize, int32 BlockStride) { return xCalcNumBlocks(Length, BlockSize, BlockStride); }
};

void testCalcNumBlocks()
{
  for(int32 WindowSize = 7; WindowSize < 65; WindowSize++)
  {
    for(int32 WindowStride = 2; WindowStride <= WindowSize; WindowStride++)
    {
      for(int32 Length = 100; Length <= 4096; Length+=27)
      {
        int32 NumActive = xSSIM_Test::calcNumBlocks(Length, WindowSize, WindowStride);

        int32 Beg = 0;
        int32 End = Length - WindowSize + 1;
        int32 Cnt = 0;
        int32 i = 0;
        for(i = Beg; i < End; i += WindowStride)
        {
          Cnt++;          
        }
        CHECK(!(i- WindowStride < 0 || i - WindowStride > Length - WindowSize));
        CHECK(Cnt == NumActive);
      }
    }
  }
}

void testCalcNumPoints()
{
  constexpr int32 FilterSize  = xStructSimConsts::c_FilterSize ;
  constexpr int32 FilterRange = xStructSimConsts::c_FilterRange;

  for(int32 WindowStride = 2; WindowStride <= FilterSize; WindowStride++)
  {
    for(int32 Length = 100; Length <= 4096; Length++)
    {
      int32 LoopBeg = FilterRange;
      int32 LoopEnd = Length - FilterRange;
      int32 NumUnit = (LoopEnd - LoopBeg + WindowStride - 1) / WindowStride;

      int32 Cnt = 0;
      int32 i   = 0;
      for(i = LoopBeg; i < LoopEnd; i += WindowStride)
      {
        Cnt++;
      }
      CHECK(!(i - WindowStride < 0 || i - WindowStride > Length - FilterRange));
      CHECK(Cnt == NumUnit);
    }   
  }
}

//===============================================================================================================================================================================================================

TEST_CASE("xCalcNumBlocks")
{
  testCalcNumBlocks();
}

TEST_CASE("testCalcNumPoints")
{
  testCalcNumPoints();
}
