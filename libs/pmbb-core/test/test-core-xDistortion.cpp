/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <functional>

#include "../src/xCommonDefPMBB-CORE.h"
#include "../src/xDistortion.h"
#include "../src/xPlane.h"
#include "../src/xTestUtils.h"

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

using tPlane = xPlane<uint16>;

static const std::vector<int32> c_Dimms = { 128, 127, 129, 512, 511, 513, 2048, 2047, 2049 };
static const std::vector<int32> c_Margs = { 0, 4, 32 };
static const std::vector<int32> c_Cntrs = { 1, 256, 1024, 4096 };
static constexpr int32          c_Max   = 16383;

//===============================================================================================================================================================================================================

void testDistortion(
  std::function<int32 (const uint16*, const uint16*, int32                     )>AreaSD,
  std::function<int32 (const uint16*, const uint16*, int32, int32, int32, int32)>StrideSD,
  std::function<uint64(const uint16*, const uint16*, int32                     )>AreaSSD,
  std::function<uint64(const uint16*, const uint16*, int32, int32, int32, int32)>StrideSSD)
{
  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };
      int64   Area = x * y;

      for(const int32 m : c_Margs)
      {
        const std::string Description = fmt::sprintf("Size=%dx%d Margin=%d", x, y, m);
        
        //buffers create
        tPlane* PL = new tPlane(Size, 14, m);
        tPlane* PC = new tPlane(Size, 14, m);
        tPlane* PU = new tPlane(Size, 14, m);

        //sets of near constant values
        for(const int32 c : c_Cntrs)
        {
          CAPTURE(Description + fmt::sprintf(" Center=%d", c));

          PL->fill(uint16(c-1));
          PC->fill(uint16(c  ));
          PU->fill(uint16(c+1));

          if(m == 0)
          {
            CHECK(AreaSD(PL->getAddr(), PL->getAddr(), PL->getArea()) == 0);
            CHECK(AreaSD(PC->getAddr(), PC->getAddr(), PC->getArea()) == 0);
            CHECK(AreaSD(PU->getAddr(), PU->getAddr(), PU->getArea()) == 0);
            CHECK(AreaSD(PC->getAddr(), PL->getAddr(), PC->getArea()) ==  Area  );
            CHECK(AreaSD(PU->getAddr(), PL->getAddr(), PU->getArea()) ==  2*Area);
            CHECK(AreaSD(PL->getAddr(), PC->getAddr(), PL->getArea()) == -Area  );
            CHECK(AreaSD(PL->getAddr(), PU->getAddr(), PL->getArea()) == -2*Area);
            CHECK(AreaSD(PU->getAddr(), PC->getAddr(), PU->getArea()) ==  Area  );
            CHECK(AreaSD(PC->getAddr(), PU->getAddr(), PC->getArea()) == -Area  );

            CHECK(AreaSSD(PL->getAddr(), PL->getAddr(), PL->getArea()) == 0);
            CHECK(AreaSSD(PC->getAddr(), PC->getAddr(), PC->getArea()) == 0);
            CHECK(AreaSSD(PU->getAddr(), PU->getAddr(), PU->getArea()) == 0);
            CHECK(AreaSSD(PC->getAddr(), PL->getAddr(), PC->getArea()) == Area  );
            CHECK(AreaSSD(PU->getAddr(), PL->getAddr(), PU->getArea()) == 4*Area);
            CHECK(AreaSSD(PL->getAddr(), PC->getAddr(), PL->getArea()) == Area  );
            CHECK(AreaSSD(PL->getAddr(), PU->getAddr(), PL->getArea()) == 4*Area);
            CHECK(AreaSSD(PU->getAddr(), PC->getAddr(), PU->getArea()) == Area  );
            CHECK(AreaSSD(PC->getAddr(), PU->getAddr(), PC->getArea()) == Area  );
          }

          CHECK(StrideSD(PL->getAddr(), PL->getAddr(), PL->getStride(), PL->getStride(), PL->getWidth(), PL->getHeight()) == 0);
          CHECK(StrideSD(PC->getAddr(), PC->getAddr(), PC->getStride(), PC->getStride(), PC->getWidth(), PC->getHeight()) == 0);
          CHECK(StrideSD(PU->getAddr(), PU->getAddr(), PU->getStride(), PU->getStride(), PU->getWidth(), PU->getHeight()) == 0);
          CHECK(StrideSD(PC->getAddr(), PL->getAddr(), PC->getStride(), PL->getStride(), PC->getWidth(), PC->getHeight()) ==  Area  );
          CHECK(StrideSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()) ==  2*Area);
          CHECK(StrideSD(PL->getAddr(), PC->getAddr(), PL->getStride(), PC->getStride(), PL->getWidth(), PL->getHeight()) == -Area  );
          CHECK(StrideSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()) == -2*Area);
          CHECK(StrideSD(PU->getAddr(), PC->getAddr(), PU->getStride(), PC->getStride(), PU->getWidth(), PU->getHeight()) ==  Area  );
          CHECK(StrideSD(PC->getAddr(), PU->getAddr(), PC->getStride(), PU->getStride(), PC->getWidth(), PC->getHeight()) == -Area  );

          CHECK(StrideSSD(PL->getAddr(), PL->getAddr(), PL->getStride(), PL->getStride(), PL->getWidth(), PL->getHeight()) == 0);
          CHECK(StrideSSD(PC->getAddr(), PC->getAddr(), PC->getStride(), PC->getStride(), PC->getWidth(), PC->getHeight()) == 0);
          CHECK(StrideSSD(PU->getAddr(), PU->getAddr(), PU->getStride(), PU->getStride(), PU->getWidth(), PU->getHeight()) == 0);
          CHECK(StrideSSD(PC->getAddr(), PL->getAddr(), PC->getStride(), PL->getStride(), PC->getWidth(), PC->getHeight()) == Area  );
          CHECK(StrideSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()) == 4*Area);
          CHECK(StrideSSD(PL->getAddr(), PC->getAddr(), PL->getStride(), PC->getStride(), PL->getWidth(), PL->getHeight()) == Area  );
          CHECK(StrideSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()) == 4*Area);
          CHECK(StrideSSD(PU->getAddr(), PC->getAddr(), PU->getStride(), PC->getStride(), PU->getWidth(), PU->getHeight()) == Area  );
          CHECK(StrideSSD(PC->getAddr(), PU->getAddr(), PC->getStride(), PU->getStride(), PC->getWidth(), PC->getHeight()) == Area  );


        }
        
        //extreme constant values
        CAPTURE(Description + fmt::sprintf(" extreme constant values Max=%d", c_Max));
        PL->fill(    0);
        PU->fill(c_Max);
        if(m == 0)
        {
          if(Area < 131072)
          {
            CHECK(AreaSD(PL->getAddr(), PU->getAddr(), PL->getArea()) == -c_Max * Area);
            CHECK(AreaSD(PU->getAddr(), PL->getAddr(), PU->getArea()) ==  c_Max * Area);
          }
          CHECK(AreaSSD(PL->getAddr(), PU->getAddr(), PL->getArea()) == xPow2<int64>(c_Max) * Area);
          CHECK(AreaSSD(PU->getAddr(), PL->getAddr(), PU->getArea()) == xPow2<int64>(c_Max) * Area);
        }
        if(Area < 131072)
        {
          CHECK(StrideSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()) == -c_Max * Area);
          CHECK(StrideSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()) ==  c_Max * Area);
        }
        CHECK(StrideSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()) == xPow2<int64>(c_Max) * Area);
        CHECK(StrideSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()) == xPow2<int64>(c_Max) * Area);

        //pseudo-random values
        CAPTURE(Description + " pseudo-random values");
        PL->fill(0);
        PU->fill(0);
        xTestUtils::fillMidNoise(PL->getAddr(), PL->getStride(), PL->getWidth(), PL->getHeight(), PL->getBitDepth(), 0);
        xTestUtils::fillMidNoise(PU->getAddr(), PU->getStride(), PU->getWidth(), PU->getHeight(), PU->getBitDepth(), 1);
        if(m == 0)
        {
          CHECK(AreaSD (PL->getAddr(), PU->getAddr(), PL->getArea()) == -Area);
          CHECK(AreaSD (PU->getAddr(), PL->getAddr(), PU->getArea()) ==  Area);
          CHECK(AreaSSD(PL->getAddr(), PU->getAddr(), PL->getArea()) ==  Area);
          CHECK(AreaSSD(PU->getAddr(), PL->getAddr(), PU->getArea()) ==  Area);
        }
        CHECK(StrideSD (PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()) == -Area);
        CHECK(StrideSD (PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()) ==  Area);
        CHECK(StrideSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()) ==  Area);
        CHECK(StrideSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()) ==  Area);



        //gradient values
        CAPTURE(Description + " gradient values");
        xTestUtils::fillGradientXY(PL->getAddr(), PL->getStride(), PL->getWidth(), PL->getHeight(), PL->getBitDepth(), 0);
        xTestUtils::fillGradientXY(PU->getAddr(), PU->getStride(), PU->getWidth(), PU->getHeight(), PU->getBitDepth(), 1);
        if(m == 0)
        {
          CHECK(AreaSD (PL->getAddr(), PU->getAddr(), PL->getArea()) == -Area);
          CHECK(AreaSD (PU->getAddr(), PL->getAddr(), PU->getArea()) ==  Area);
          CHECK(AreaSSD(PL->getAddr(), PU->getAddr(), PL->getArea()) ==  Area);
          CHECK(AreaSSD(PU->getAddr(), PL->getAddr(), PU->getArea()) ==  Area);
        }
        CHECK(StrideSD (PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()) == -Area);
        CHECK(StrideSD (PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()) ==  Area);
        CHECK(StrideSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()) ==  Area);
        CHECK(StrideSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()) ==  Area);



        //buffers destroy
        delete PL;
        delete PC;
        delete PU;
      }
    }
  }    
}


//===============================================================================================================================================================================================================

TEST_CASE("xDistortionSTD")
{
  tTimePoint T = tClock::now();
  testDistortion
  (
    static_cast<int32 (*)(const uint16*, const uint16*, int32                     )>(&xDistortionSTD::CalcSD ),
    static_cast<int32 (*)(const uint16*, const uint16*, int32, int32, int32, int32)>(&xDistortionSTD::CalcSD ),
    static_cast<uint64(*)(const uint16*, const uint16*, int32                     )>(&xDistortionSTD::CalcSSD),
    static_cast<uint64(*)(const uint16*, const uint16*, int32, int32, int32, int32)>(&xDistortionSTD::CalcSSD)
  );
  fmt::printf("TIME(xDistortionSTD   ) = %fs\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}

#if X_SIMD_CAN_USE_SSE
TEST_CASE("xDistortionSSE")
{
  tTimePoint T = tClock::now();
  testDistortion
  (
    static_cast<int32 (*)(const uint16*, const uint16*, int32                     )>(&xDistortionSSE::CalcSD ),
    static_cast<int32 (*)(const uint16*, const uint16*, int32, int32, int32, int32)>(&xDistortionSSE::CalcSD ),
    static_cast<uint64(*)(const uint16*, const uint16*, int32                     )>(&xDistortionSSE::CalcSSD),
    static_cast<uint64(*)(const uint16*, const uint16*, int32, int32, int32, int32)>(&xDistortionSSE::CalcSSD)
  );
  fmt::printf("TIME(xDistortionSSE   ) = %fs\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}
#endif

#if X_SIMD_CAN_USE_AVX
TEST_CASE("xDistortionAVX")
{
  tTimePoint T = tClock::now();
  testDistortion
  (
    static_cast<int32 (*)(const uint16*, const uint16*, int32                     )>(&xDistortionAVX::CalcSD ),
    static_cast<int32 (*)(const uint16*, const uint16*, int32, int32, int32, int32)>(&xDistortionAVX::CalcSD ),
    static_cast<uint64(*)(const uint16*, const uint16*, int32                     )>(&xDistortionAVX::CalcSSD),
    static_cast<uint64(*)(const uint16*, const uint16*, int32, int32, int32, int32)>(&xDistortionAVX::CalcSSD)
  );
  fmt::printf("TIME(xDistortionAVX   ) = %fs\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}
#endif

#if X_SIMD_CAN_USE_AVX512
TEST_CASE("xDistortionAVX512")
{
  tTimePoint T = tClock::now();
  testDistortion
  (
    static_cast<int32 (*)(const uint16*, const uint16*, int32                     )>(&xDistortionAVX512::CalcSD ),
    static_cast<int32 (*)(const uint16*, const uint16*, int32, int32, int32, int32)>(&xDistortionAVX512::CalcSD ),
    static_cast<uint64(*)(const uint16*, const uint16*, int32                     )>(&xDistortionAVX512::CalcSSD),
    static_cast<uint64(*)(const uint16*, const uint16*, int32, int32, int32, int32)>(&xDistortionAVX512::CalcSSD)
  );
  fmt::printf("TIME(xDistortionAVX512) = %fs\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}
#endif
