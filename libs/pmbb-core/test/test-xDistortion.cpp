/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <functional>
#include "../src/xCommonDefCORE.h"
#include "../src/xDistortion.h"
#include "../src/xPlane.h"
#include "../src/xTestUtils.h"
#include "xTimeUtils.h"

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

using tPlane = xPlane<uint16>;

static const std::vector<int32> c_Dimms = { 63, 64, 65, 128, 127, 129, 255, 256, 257 };
static const std::vector<int32> c_Margs = { 0, 4, 32 };
static const std::vector<int32> c_BitDs = { 12, 14, 16 };

static constexpr int32 c_PerfUnitSize = 2048;
static constexpr int32 c_PerfNumIters = 256;

//===============================================================================================================================================================================================================

void testDistortion(
  std::function<int64 (const uint16*, const uint16*, int32, int32, int32, int32, int32)>FunSD ,
  std::function<uint64(const uint16*, const uint16*, int32, int32, int32, int32, int32)>FunSAD,
  std::function<uint64(const uint16*, const uint16*, int32, int32, int32, int32, int32)>FunSSD)
{
  uint32 State = xTestUtils::c_XorShiftSeed;

  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };
      int64   Area = x * y;

      for(const int32 m : c_Margs)
      {
        for(const int32 b : c_BitDs)
        {
          const std::string Description = fmt::format("SizeXxY={}x{} Margin={} BitDepth={}", x, y, m, b);
        
          //buffers create
          tPlane* PL = new tPlane(Size, b, m);
          tPlane* PC = new tPlane(Size, b, m);
          tPlane* PU = new tPlane(Size, b, m);

          //sets of near constant values
          const std::vector<int32> Cntrs = { 1, xBitDepth2MidValue(b) -1, xBitDepth2MidValue(b), xBitDepth2MidValue(b) + 1, xBitDepth2MaxValue(b) - 1 };
          for(const int32 c : Cntrs)
          {
            CAPTURE(Description + fmt::format(" Center={}", c));

            PL->fill(uint16(c-1));
            PC->fill(uint16(c  ));
            PU->fill(uint16(c+1));

            CHECK(FunSD(PL->getAddr(), PL->getAddr(), PL->getStride(), PL->getStride(), PL->getWidth(), PL->getHeight(), b) == 0);
            CHECK(FunSD(PC->getAddr(), PC->getAddr(), PC->getStride(), PC->getStride(), PC->getWidth(), PC->getHeight(), b) == 0);
            CHECK(FunSD(PU->getAddr(), PU->getAddr(), PU->getStride(), PU->getStride(), PU->getWidth(), PU->getHeight(), b) == 0);
            CHECK(FunSD(PC->getAddr(), PL->getAddr(), PC->getStride(), PL->getStride(), PC->getWidth(), PC->getHeight(), b) ==  Area  );
            CHECK(FunSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b) ==  2*Area);
            CHECK(FunSD(PL->getAddr(), PC->getAddr(), PL->getStride(), PC->getStride(), PL->getWidth(), PL->getHeight(), b) == -Area  );
            CHECK(FunSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b) == -2*Area);
            CHECK(FunSD(PU->getAddr(), PC->getAddr(), PU->getStride(), PC->getStride(), PU->getWidth(), PU->getHeight(), b) ==  Area  );
            CHECK(FunSD(PC->getAddr(), PU->getAddr(), PC->getStride(), PU->getStride(), PC->getWidth(), PC->getHeight(), b) == -Area  );

            CHECK(FunSAD(PL->getAddr(), PL->getAddr(), PL->getStride(), PL->getStride(), PL->getWidth(), PL->getHeight(), b) == 0);
            CHECK(FunSAD(PC->getAddr(), PC->getAddr(), PC->getStride(), PC->getStride(), PC->getWidth(), PC->getHeight(), b) == 0);
            CHECK(FunSAD(PU->getAddr(), PU->getAddr(), PU->getStride(), PU->getStride(), PU->getWidth(), PU->getHeight(), b) == 0);
            CHECK(FunSAD(PC->getAddr(), PL->getAddr(), PC->getStride(), PL->getStride(), PC->getWidth(), PC->getHeight(), b) == Area  );
            CHECK(FunSAD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b) == 2*Area);
            CHECK(FunSAD(PL->getAddr(), PC->getAddr(), PL->getStride(), PC->getStride(), PL->getWidth(), PL->getHeight(), b) == Area  );
            CHECK(FunSAD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b) == 2*Area);
            CHECK(FunSAD(PU->getAddr(), PC->getAddr(), PU->getStride(), PC->getStride(), PU->getWidth(), PU->getHeight(), b) == Area  );
            CHECK(FunSAD(PC->getAddr(), PU->getAddr(), PC->getStride(), PU->getStride(), PC->getWidth(), PC->getHeight(), b) == Area  );

            CHECK(FunSSD(PL->getAddr(), PL->getAddr(), PL->getStride(), PL->getStride(), PL->getWidth(), PL->getHeight(), b) == 0);
            CHECK(FunSSD(PC->getAddr(), PC->getAddr(), PC->getStride(), PC->getStride(), PC->getWidth(), PC->getHeight(), b) == 0);
            CHECK(FunSSD(PU->getAddr(), PU->getAddr(), PU->getStride(), PU->getStride(), PU->getWidth(), PU->getHeight(), b) == 0);
            CHECK(FunSSD(PC->getAddr(), PL->getAddr(), PC->getStride(), PL->getStride(), PC->getWidth(), PC->getHeight(), b) == Area  );
            CHECK(FunSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b) == 4*Area);
            CHECK(FunSSD(PL->getAddr(), PC->getAddr(), PL->getStride(), PC->getStride(), PL->getWidth(), PL->getHeight(), b) == Area  );
            CHECK(FunSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b) == 4*Area);
            CHECK(FunSSD(PU->getAddr(), PC->getAddr(), PU->getStride(), PC->getStride(), PU->getWidth(), PU->getHeight(), b) == Area  );
            CHECK(FunSSD(PC->getAddr(), PU->getAddr(), PC->getStride(), PU->getStride(), PC->getWidth(), PC->getHeight(), b) == Area  );
          }
        
          //extreme constant values
          const int64 MaxVal = xBitDepth2MaxValue(b);
          CAPTURE(Description + fmt::format(" extreme constant values Max={}", MaxVal));
          PL->fill(             0);
          PU->fill((uint16)MaxVal);
          if(b < 16 || Area < 8192)
          {
            CHECK(FunSD (PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b) == -MaxVal * Area);
            CHECK(FunSD (PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b) ==  MaxVal * Area);
            CHECK(FunSAD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b) ==  MaxVal * Area);
            CHECK(FunSAD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b) ==  MaxVal * Area);
            CHECK(FunSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b) == xPow2<int64>(MaxVal) * Area);
            CHECK(FunSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b) == xPow2<int64>(MaxVal) * Area);
          }

          //pseudo-random values
          CAPTURE(Description + " pseudo-random values");
          PL->fill(0);
          PU->fill(0);
          uint32 OrgState = State;
          State = xTestUtils::fillMidNoise(PL->getAddr(), PL->getStride(), PL->getWidth(), PL->getHeight(), PL->getBitDepth(), 0, OrgState);
          State = xTestUtils::fillMidNoise(PU->getAddr(), PU->getStride(), PU->getWidth(), PU->getHeight(), PU->getBitDepth(), 1, OrgState);
          CHECK(FunSD (PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b) == -Area);
          CHECK(FunSD (PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b) ==  Area);
          CHECK(FunSAD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b) ==  Area);
          CHECK(FunSAD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b) ==  Area);
          CHECK(FunSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b) ==  Area);
          CHECK(FunSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b) ==  Area);

          //gradient values
          CAPTURE(Description + " gradient values");
          xTestUtils::fillGradientXY(PL->getAddr(), PL->getStride(), PL->getWidth(), PL->getHeight(), PL->getBitDepth(), 0);
          xTestUtils::fillGradientXY(PU->getAddr(), PU->getStride(), PU->getWidth(), PU->getHeight(), PU->getBitDepth(), 1);
          CHECK(FunSD (PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b) == -Area);
          CHECK(FunSD (PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b) ==  Area);
          CHECK(FunSAD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b) ==  Area);
          CHECK(FunSAD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b) ==  Area);
          CHECK(FunSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b) ==  Area);
          CHECK(FunSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b) ==  Area);

          //buffers destroy
          delete PL;
          delete PC;
          delete PU;
        }
      }
    }
  }    
}

flt64 perfDistortion(std::function<int64(const uint16*, const uint16*, int32, int32, int32, int32, int32)>DistFun, int32 BitDepth)
{
  const int32V2 Size = { c_PerfUnitSize, c_PerfUnitSize };
  const int64   Area = c_PerfUnitSize * c_PerfUnitSize;

  tPlane* PL = new tPlane(Size, BitDepth, 0);
  tPlane* PU = new tPlane(Size, BitDepth, 0);

  uint32 State = xTestUtils::c_XorShiftSeed;

  uint32 OrgState = State;
  State = xTestUtils::fillMidNoise(PL->getAddr(), PL->getStride(), PL->getWidth(), PL->getHeight(), BitDepth, 0, OrgState);
  State = xTestUtils::fillMidNoise(PU->getAddr(), PU->getStride(), PU->getWidth(), PU->getHeight(), BitDepth, 1, OrgState);

  tDuration T = (tDuration)0;

  //warmup
  {
    int64 Result = DistFun(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getArea(), 1, BitDepth);
    CHECK(Result == Area);
  }

  //measure
  for(int32 j = 0; j < c_PerfNumIters; j++)
  {
    tTimePoint T0 = tClock::now();
    int64 Result = DistFun(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getArea(), 1, BitDepth);
    tTimePoint T1 = tClock::now();
    CHECK(Result == Area);
    T += T1 - T0;
  }

  //cleanup
  delete PL; PL = nullptr;
  delete PU; PU = nullptr;

  //calculate
  int64 NumBytes    = (int64)c_PerfUnitSize * (int64)c_PerfUnitSize * (int64)c_PerfNumIters * sizeof(int16);
  flt64 BytesPerSec = NumBytes / std::chrono::duration_cast<tDurationS>(T).count();

  return BytesPerSec;
}

//===============================================================================================================================================================================================================

TEST_CASE("xDistortionSTD")
{
  testDistortion
  (
    static_cast< int64(*)(const uint16*, const uint16*, int32, int32, int32, int32, int32)>(&xDistortionSTD::CalcSD ),
    static_cast<uint64(*)(const uint16*, const uint16*, int32, int32, int32, int32, int32)>(&xDistortionSTD::CalcSAD),
    static_cast<uint64(*)(const uint16*, const uint16*, int32, int32, int32, int32, int32)>(&xDistortionSTD::CalcSSD)
  );
}

#if X_SIMD_CAN_USE_SSE
TEST_CASE("xDistortionSSE")
{
  testDistortion
  (
    static_cast< int64(*)(const uint16*, const uint16*, int32, int32, int32, int32, int32)>(&xDistortionSSE::CalcSD ),
    static_cast<uint64(*)(const uint16*, const uint16*, int32, int32, int32, int32, int32)>(&xDistortionSSE::CalcSAD),
    static_cast<uint64(*)(const uint16*, const uint16*, int32, int32, int32, int32, int32)>(&xDistortionSSE::CalcSSD)
  );
}
#endif //X_SIMD_CAN_USE_SSE

#if X_SIMD_CAN_USE_AVX
TEST_CASE("xDistortionAVX")
{
  testDistortion
  (
    static_cast< int64(*)(const uint16*, const uint16*, int32, int32, int32, int32, int32)>(&xDistortionAVX::CalcSD ),
    static_cast<uint64(*)(const uint16*, const uint16*, int32, int32, int32, int32, int32)>(&xDistortionAVX::CalcSAD),
    static_cast<uint64(*)(const uint16*, const uint16*, int32, int32, int32, int32, int32)>(&xDistortionAVX::CalcSSD)
  );
}
#endif //X_SIMD_CAN_USE_AVX

#if X_SIMD_CAN_USE_AVX512
TEST_CASE("xDistortionAVX512")
{
  testDistortion
  (
    static_cast< int64(*)(const uint16*, const uint16*, int32, int32, int32, int32, int32)>(&xDistortionAVX512::CalcSD ),
    static_cast<uint64(*)(const uint16*, const uint16*, int32, int32, int32, int32, int32)>(&xDistortionAVX512::CalcSAD),
    static_cast<uint64(*)(const uint16*, const uint16*, int32, int32, int32, int32, int32)>(&xDistortionAVX512::CalcSSD)
  );
}
#endif //X_SIMD_CAN_USE_AVX512

//===============================================================================================================================================================================================================

