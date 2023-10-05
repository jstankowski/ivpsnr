/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <numeric>

#include "../src/xCommonDefPMBB-BASE.h"
#include "../src/xMathUtils.h"

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

static const int32 Log2NumPasses = 28;

//===============================================================================================================================================================================================================

void testAccumulate(flt64 Value, uintSize NumValues)
{
  std::vector<flt64> Buffer(NumValues, Value);
  flt64 SumKBMS = xMathUtils::KahanBabuskaNeumaierSumation(Buffer.data(), (int32)Buffer.size());
  CHECK(SumKBMS == NumValues * Value);
  //flt64 SumAcc  = std::accumulate(Buffer.begin(), Buffer.end(), (flt64)0);
  //WARN (SumAcc == NumValues * Value);
}

//===============================================================================================================================================================================================================

TEST_CASE("xMathUtils::KahanBabuskaNeumaierSumation" * doctest::no_breaks(true))
{ 
  tTimePoint T = tClock::now();
  SUBCASE("V=0.0")
  {
    for(int32 i = 0; i < Log2NumPasses; i++)
    {
      int32 NumValues = 1<<i;
      CAPTURE(fmt::sprintf(" NumValues=%d", NumValues));      
      testAccumulate(0.0, NumValues);
    }
  }

  SUBCASE("V=0.01")
  {
    for(int32 i = 0; i < Log2NumPasses; i++)
    {
      int32 NumValues = 1 << i;
      CAPTURE(fmt::sprintf(" NumValues=%d", NumValues));
      testAccumulate(0.01, NumValues);
    }
  }

  SUBCASE("V=0.1")
  {
    for(int32 i = 0; i < Log2NumPasses; i++)
    {
      int32 NumValues = 1 << i;
      CAPTURE(fmt::sprintf(" NumValues=%d", NumValues));
      testAccumulate(0.1, NumValues);
    }
  }

  SUBCASE("V=0.9")
  {
    for(int32 i = 0; i < Log2NumPasses; i++)
    {
      int32 NumValues = 1 << i;
      CAPTURE(fmt::sprintf(" NumValues=%d", NumValues));
      testAccumulate(0.9, NumValues);
    }
  }

  SUBCASE("V=0.99")
  {
    for(int32 i = 0; i < Log2NumPasses; i++)
    {
      int32 NumValues = 1 << i;
      CAPTURE(fmt::sprintf(" NumValues=%d", NumValues));
      testAccumulate(0.99, NumValues);
    }
  }

  SUBCASE("V=1.0")
  {
    for(int32 i = 0; i < Log2NumPasses; i++)
    {
      int32 NumValues = 1 << i;
      CAPTURE(fmt::sprintf(" NumValues=%d", NumValues));
      testAccumulate(1.0, NumValues);
    }
  }


  fmt::printf("TIME(KahanBabuskaNeumaierSumation) = %fs\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}

//===============================================================================================================================================================================================================
