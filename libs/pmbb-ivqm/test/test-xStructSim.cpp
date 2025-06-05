/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
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
#include "xTimeUtils.h"
#include "xMemory.h"
#include "xStructSim.h"
#include "xPlane.h"

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

static const std::vector<int32> c_Dimms = { 8, 11, 16, 32 };
static const std::vector<int32> c_BitDs = { 8, 14 };
static const std::vector<int32> c_Margs = { 0, 4, 32 };
static constexpr int32          c_NumRandomTests = 128;
static constexpr flt64 C1 = 0.000001;
static constexpr flt64 C2 = 0.000002;

//===============================================================================================================================================================================================================

void testCalcBlckAvg(std::function<flt64(const uint16* Tst, const uint16* Ref, int32 StrideT, int32 StrideR, int32 WndSize, flt64 C1, flt64 C2, bool CalcL)> CalcBlckAvg)
{
  uint32 State = xTestUtils::c_XorShiftSeed;

  for(const int32 d : c_Dimms)
  {
    int32V2 Size = { d, d };

    for(const int32 m : c_Margs)
    {
      for(const int32 b : c_BitDs)
      {
        const std::string Description = fmt::format("SizeXxY={}x{} Margin={} BitDepth={}", d, d, m, b);

        //buffers create
        xPlane<uint16>* Ref = new xPlane<uint16>(Size, b, m);
        xPlane<uint16>* Tst = new xPlane<uint16>(Size, b, m);

        //deterministic tests
        CAPTURE(Description + fmt::format(" Deterministic"));
        uint16 Max = (uint16)xBitDepth2MaxValue(b);
        uint16 Mid = (uint16)xBitDepth2MidValue(b);
        std::vector<uint16V2> VPs{ {0,Max}, {Max,0}, {Mid,0}, {Mid,0}, {Max,Mid}, {Max,Mid} };

        for(const uint16V2 TestVals : VPs)
        {
          Ref->fill(TestVals[0]);
          Tst->fill(TestVals[1]);

          flt64 A = xStructSimSTD::CalcBlckAvg(Tst->getAddr(), Ref->getAddr(), Tst->getStride(), Ref->getStride(), d, C1, C2, true);
          flt64 B =                CalcBlckAvg(Tst->getAddr(), Ref->getAddr(), Tst->getStride(), Ref->getStride(), d, C1, C2, true);
          CHECK(A == B);
        }

        //gradient test
        CAPTURE(Description + fmt::format(" Gradient"));
        Ref->fill(0);
        Tst->fill(0);
        xTestUtils::fillGradientXY(Ref->getAddr(), Ref->getStride(), Ref->getWidth(), Ref->getHeight(), Ref->getBitDepth(), 0);
        for(int32 Offset = 0; Offset <= 10; Offset++)
        {          
          xTestUtils::fillGradientXY(Tst->getAddr(), Tst->getStride(), Tst->getWidth(), Tst->getHeight(), Tst->getBitDepth(), Offset);
          flt64 A = xStructSimSTD::CalcBlckAvg(Tst->getAddr(), Ref->getAddr(), Tst->getStride(), Ref->getStride(), d, C1, C2, true);
          flt64 B =                CalcBlckAvg(Tst->getAddr(), Ref->getAddr(), Tst->getStride(), Ref->getStride(), d, C1, C2, true);
          CHECK(A == B);
        }

        //random tests
        CAPTURE(Description + fmt::format(" Random"));
        Ref->fill(0);
        Tst->fill(0);
        for(int32 n = 0; n < c_NumRandomTests; n++)
        {
          State = xTestUtils::fillMidNoise(Ref->getAddr(), Ref->getStride(), d, d, b, 0, State);
          State = xTestUtils::fillMidNoise(Tst->getAddr(), Tst->getStride(), d, d, b, 0, State);

          flt64 A = xStructSimSTD::CalcBlckAvg(Tst->getAddr(), Ref->getAddr(), Tst->getStride(), Ref->getStride(), d, C1, C2, true);
          flt64 B =                CalcBlckAvg(Tst->getAddr(), Ref->getAddr(), Tst->getStride(), Ref->getStride(), d, C1, C2, true);
          CHECK(A == B);
        }
      }
    }
  }
}

//===============================================================================================================================================================================================================

TEST_CASE("xStructSimSTD")
{
  testCalcBlckAvg(xStructSimSTD::CalcBlckAvg);
}

#if X_SIMD_CAN_USE_SSE
TEST_CASE("xStructSimSSE")
{
  testCalcBlckAvg(xStructSimSSE::CalcBlckAvg);
}
#endif //X_SIMD_CAN_USE_SSE

#if X_SIMD_CAN_USE_AVX
TEST_CASE("xStructSimAVX")
{
  testCalcBlckAvg(xStructSimAVX::CalcBlckAvg);
}
#endif //X_SIMD_CAN_USE_AVX

#if X_SIMD_CAN_USE_AVX512
TEST_CASE("xStructSimAVX512")
{
  testCalcBlckAvg(xStructSimAVX512::CalcBlckAvg);
}
#endif //X_SIMD_CAN_USE_AVX512

//===============================================================================================================================================================================================================