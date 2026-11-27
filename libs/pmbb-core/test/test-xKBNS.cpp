/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <numeric>
#include <random>
#include <functional>

#include "xCommonDefCORE.h"
#include "xTimeUtils.h"
#include "xKBNS.h"
#include "xTestUtils.h"

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

static const int32 Log2NumPasses = 24;

//===============================================================================================================================================================================================================

template <class tKBNS> flt64 testAccumulate(flt64 Value)
{
  const int32 MaxNumValues = 1 << (Log2NumPasses - 1);
  std::vector<flt64> Buffer(MaxNumValues, Value);

  tTimePoint T = tClock::now();

  for(int32 l = 0; l < Log2NumPasses; l+=2)
  {
    int32 NumValues = 1 << l;
    CAPTURE(fmt::format(" NumValues={}", NumValues));    
  
    flt64 SumKBMS = tKBNS::Accumulate(Buffer.data(), NumValues);
    CHECK(SumKBMS == NumValues * Value);

    tKBNS KBNS;
    for(uintSize i = 0; i < NumValues; i++) { KBNS.acc(Buffer[i]); }
    CHECK(KBNS.result() == NumValues * Value);
    KBNS.reset();
    for(uintSize i = 0; i < NumValues; i++) { KBNS.acc(Buffer[i]); }
    CHECK(KBNS.result() == NumValues * Value);
    KBNS.reset();
    KBNS.acc(Buffer.data(), NumValues);
    CHECK(KBNS.result() == NumValues * Value);
  }

  return std::chrono::duration_cast<tDurationS>(tClock::now() - T).count();
}

flt64 testKBNS4(std::function<flt64V4(const flt64V4*, const uintSize)> Accumulate4)
{
  tDuration D = (tDuration)0;

  xTestUtils::xXorShiftGen32 TestGen; std::uniform_real_distribution<flt64> RandomDistribution(-1, 1);

  const int32 MaxNumValues = 1 << (Log2NumPasses - 1);
  std::vector<flt64V4> Buffer(MaxNumValues);
  for(int32 j = 0; j < 4; j++) { for(int32 i = 0; i < MaxNumValues; i++) { Buffer[j][i] = RandomDistribution(TestGen); } }

  for(int32 i = 8; i < Log2NumPasses; i+=2)
  {
    int32 NumValues = 1 << i;
    flt64V4 Ref = xKBNS4_STD::Accumulate(Buffer.data(), NumValues);
    tTimePoint T = tClock::now();
    flt64V4 Tst = Accumulate4(Buffer.data(), NumValues);
    D += tClock::now() - T;
    for(int32 j = 0; j < 4; j++) { CHECK(Ref[j] == Tst[j]); }
  }

  return std::chrono::duration_cast<tDurationS>(D).count();
}

template <class FLT> std::tuple<flt64, flt64> calcError(uint64 Num, FLT Val)
{
  static constexpr uint64 Log2MemLimit = 26;
  uint64 Log2Num = xFastLog2(Num);
  uint64 RecNum = (uint64)1 << Log2Num;
  CHECK(RecNum == Num);

  //avoid excessive memory usage
  uint64 NumIters = 1;
  uint64 NumUnits = Num;
  if(Log2Num > Log2MemLimit)
  {
    NumIters = NumUnits >> Log2MemLimit;
    NumUnits = (uint64)1 << Log2MemLimit;
  }

  //allocate storage
  std::vector<FLT> Data(NumUnits);
  std::fill(Data.begin(), Data.end(), Val);

  flt64         Ref  = (flt64)Num * (flt64)Val;
  FLT           SumN = 0;
  xKBNS1_T<FLT> SumK;
  for(uint64 i = 0; i < NumIters; i++)
  {
    for(uint64 u = 0; u < NumUnits; u++)
    {
      SumN += Data[u];
      SumK += Data[u];
    }
  }
  
  flt64 DeltaN = Ref - (flt64)SumN;
  flt64 DeltaK = Ref - (flt64)SumK.result();
  return { std::abs(DeltaN), std::abs(DeltaK) };
}

//===============================================================================================================================================================================================================

TEST_CASE("xMathUtils::xKBNS1_STD")
{
  SUBCASE("V=0.00") { flt64 Time = testAccumulate<xKBNS1_STD>(0.00); fmt::print("TIME(xKBNS1_STD 0.00) = {}s\n", Time); }
  SUBCASE("V=0.01") { flt64 Time = testAccumulate<xKBNS1_STD>(0.01); fmt::print("TIME(xKBNS1_STD 0.01) = {}s\n", Time); }
  SUBCASE("V=0.10") { flt64 Time = testAccumulate<xKBNS1_STD>(0.10); fmt::print("TIME(xKBNS1_STD 0.10) = {}s\n", Time); }
  SUBCASE("V=0.90") { flt64 Time = testAccumulate<xKBNS1_STD>(0.90); fmt::print("TIME(xKBNS1_STD 0.90) = {}s\n", Time); }
  SUBCASE("V=0.99") { flt64 Time = testAccumulate<xKBNS1_STD>(0.99); fmt::print("TIME(xKBNS1_STD 0.99) = {}s\n", Time); }
  SUBCASE("V=1.00") { flt64 Time = testAccumulate<xKBNS1_STD>(1.00); fmt::print("TIME(xKBNS1_STD 1.00) = {}s\n", Time); }
}

#if X_SIMD_CAN_USE_SSE
TEST_CASE("xMathUtils::xKBNS1_SSE")
{
  SUBCASE("V=0.00") { flt64 Time = testAccumulate<xKBNS1_SSE>(0.00); fmt::print("TIME(xKBNS1_SSE 0.00) = {}s\n", Time); }
  SUBCASE("V=0.01") { flt64 Time = testAccumulate<xKBNS1_SSE>(0.01); fmt::print("TIME(xKBNS1_SSE 0.01) = {}s\n", Time); }
  SUBCASE("V=0.10") { flt64 Time = testAccumulate<xKBNS1_SSE>(0.10); fmt::print("TIME(xKBNS1_SSE 0.10) = {}s\n", Time); }
  SUBCASE("V=0.90") { flt64 Time = testAccumulate<xKBNS1_SSE>(0.90); fmt::print("TIME(xKBNS1_SSE 0.90) = {}s\n", Time); }
  SUBCASE("V=0.99") { flt64 Time = testAccumulate<xKBNS1_SSE>(0.99); fmt::print("TIME(xKBNS1_SSE 0.99) = {}s\n", Time); }
  SUBCASE("V=1.00") { flt64 Time = testAccumulate<xKBNS1_SSE>(1.00); fmt::print("TIME(xKBNS1_SSE 1.00) = {}s\n", Time); }
}
#endif //X_SIMD_CAN_USE_SSE

TEST_CASE("xKBNS4_STD")
{
  flt64 Time = testKBNS4(static_cast<flt64V4(*)(const flt64V4*, const uintSize)>(&xKBNS4_STD::Accumulate));
  fmt::print("TIME(xKBNS4_STD) = {}s\n", Time);
}

#if X_SIMD_CAN_USE_SSE
TEST_CASE("xKBNS4_SSE")
{
  flt64 Time = testKBNS4(static_cast<flt64V4(*)(const flt64V4*, const uintSize)>(&xKBNS4_SSE::Accumulate));
  fmt::print("TIME(xKBNS4_SSE) = {}s\n", Time);
}
#endif //X_SIMD_CAN_USE_SSE

#if X_SIMD_CAN_USE_AVX
TEST_CASE("xKBNS4_AVX")
{
  flt64 Time = testKBNS4(static_cast<flt64V4(*)(const flt64V4*, const uintSize)>(&xKBNS4_AVX::Accumulate));
  fmt::print("TIME(xKBNS4_AVX) = {}s\n", Time);
}
#endif //X_SIMD_CAN_USE_AVX

#if 0
TEST_CASE("ErrorEstimation")
{
  static constexpr uint64 Log2Max = 32;
  fmt::print("FLT32\n");
  for(int64 i = 3; i <= Log2Max; i++)
  {
    auto [ErrN, ErrK] = calcError<flt32>((uint64)1 << i, (flt32)0.95);
    fmt::print("2^{:02d}, {}, {}\n", i, ErrN, ErrK);
  }
  fmt::print("FLT64\n");
  for(int64 i = 3; i <= Log2Max; i++)
  {
    auto [ErrN, ErrK] = calcError<flt64>((uint64)1 << i, (flt64)0.95);
    fmt::print("2^{:02d}, {}, {}\n", i, ErrN, ErrK);
  }
}
#endif

//===============================================================================================================================================================================================================
