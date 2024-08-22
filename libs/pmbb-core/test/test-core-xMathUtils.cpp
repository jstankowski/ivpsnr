/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <numeric>
#include <random>

#include "../src/xCommonDefCORE.h"
#include "../src/xMathUtils.h"

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

static const int32 Log2NumPasses = 24;

//===============================================================================================================================================================================================================

void testAccumulate(flt64 Value, uintSize NumValues)
{
  std::vector<flt64> Buffer(NumValues, Value);
  
  {
    flt64 SumKBMS = xKBNS::Accumulate(Buffer);
    CHECK(SumKBMS == NumValues * Value);
  }

  {
    flt64 SumKBMS = xKBNS::Accumulate(Buffer.data(), (int32)Buffer.size());
    CHECK(SumKBMS == NumValues * Value);
  }

  xKBNS KBNS;
  for(uintSize i = 0; i < NumValues; i++) { KBNS.acc(Buffer[i]); }
  CHECK(KBNS.result() == NumValues * Value);
  KBNS.reset();
  for(uintSize i = 0; i < NumValues; i++) { KBNS.acc(Buffer[i]); }
  CHECK(KBNS.result() == NumValues * Value);
  KBNS.reset();
  KBNS.acc(Buffer.data(), Buffer.size());
  CHECK(KBNS.result() == NumValues * Value);

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
      CAPTURE(fmt::format(" NumValues={}", NumValues));
      testAccumulate(0.0, NumValues);
    }
  }

  SUBCASE("V=0.01")
  {
    for(int32 i = 0; i < Log2NumPasses; i++)
    {
      int32 NumValues = 1 << i;
      CAPTURE(fmt::format(" NumValues={}", NumValues));
      testAccumulate(0.01, NumValues);
    }
  }

  SUBCASE("V=0.1")
  {
    for(int32 i = 0; i < Log2NumPasses; i++)
    {
      int32 NumValues = 1 << i;
      CAPTURE(fmt::format(" NumValues={}", NumValues));
      testAccumulate(0.1, NumValues);
    }
  }

  SUBCASE("V=0.9")
  {
    for(int32 i = 0; i < Log2NumPasses; i++)
    {
      int32 NumValues = 1 << i;
      CAPTURE(fmt::format(" NumValues={}", NumValues));
      testAccumulate(0.9, NumValues);
    }
  }

  SUBCASE("V=0.99")
  {
    for(int32 i = 0; i < Log2NumPasses; i++)
    {
      int32 NumValues = 1 << i;
      CAPTURE(fmt::format(" NumValues={}", NumValues));
      testAccumulate(0.99, NumValues);
    }
  }

  SUBCASE("V=1.0")
  {
    for(int32 i = 0; i < Log2NumPasses; i++)
    {
      int32 NumValues = 1 << i;
      CAPTURE(fmt::format(" NumValues={}", NumValues));
      testAccumulate(1.0, NumValues);
    }
  }

  fmt::print("TIME(xKBNS) = {}s\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}

TEST_CASE("xKBNS4_STD")
{
  tTimePoint T = tClock::now();

  std::random_device RandomDevice;  //Will be used to obtain a seed for the random number engine
  std::mt19937       RandomGenerator(RandomDevice()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<flt64> RandomDistribution(-1, 1);

  const int32 MaxNumValues = 1 << (Log2NumPasses - 1);
  std::vector<flt64V4> Buffer(MaxNumValues);  
  for(int32 j = 0; j < 4; j++) { for(int32 i = 0; i < MaxNumValues; i++) { Buffer[j][i] = RandomDistribution(RandomGenerator); } }

  for(int32 i = 0; i < Log2NumPasses; i++)
  {
    int32 NumValues = 1 << i;
    flt64V4 Ref = xKBNS     ::Accumulate(Buffer.data(), NumValues);
    flt64V4 Tst = xKBNS4_STD::Accumulate(Buffer.data(), NumValues);
    for(int32 j = 0; j < 4; j++) { CHECK(Ref[j] == Tst[j]); }
  }

  fmt::print("TIME(xKBNS4_STD) = {}s\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}

#if X_SIMD_CAN_USE_AVX
TEST_CASE("xKBNS4_AVX")
{
  tTimePoint T = tClock::now();

  std::random_device RandomDevice;  //Will be used to obtain a seed for the random number engine
  std::mt19937       RandomGenerator(RandomDevice()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_real_distribution<flt64> RandomDistribution(-1, 1);

  const int32 MaxNumValues = 1 << (Log2NumPasses - 1);
  std::vector<flt64V4> Buffer(MaxNumValues);  
  for(int32 j = 0; j < 4; j++) { for(int32 i = 0; i < MaxNumValues; i++) { Buffer[j][i] = RandomDistribution(RandomGenerator); } }

  for(int32 i = 0; i < Log2NumPasses; i++)
  {
    int32 NumValues = 1 << i;
    flt64V4 Ref = xKBNS     ::Accumulate(Buffer.data(), NumValues);
    flt64V4 Tst = xKBNS4_AVX::Accumulate(Buffer.data(), NumValues);
    for(int32 j = 0; j < 4; j++) { CHECK(Ref[j] == Tst[j]); }
  }

  fmt::print("TIME(xKBNS4_AVX) = {}s\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}
#endif //X_SIMD_CAN_USE_AVX

//===============================================================================================================================================================================================================
