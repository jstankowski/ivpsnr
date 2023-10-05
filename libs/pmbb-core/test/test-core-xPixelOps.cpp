/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <functional>
#include <random>

#include "../src/xCommonDefPMBB-CORE.h"
#include "../src/xPixelOps.h"
#include "../src/xPic.h"
#include "../src/xPlane.h"
#include "../src/xTestUtils.h"

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

static const std::vector<int32> c_Dimms = { 128, 127, 129, 512, 511, 513 };
static const std::vector<int32> c_Margs = { 0, 4, 32 };
static const std::vector<int32> c_BitDs = { 8, 10 ,12, 14 };
static constexpr int32          c_DefBitDepth    = 14;
static constexpr int32          c_DefMaxValue    = (1<<c_DefBitDepth) - 1;
static constexpr int32          c_NumRandomTests = 8;

//===============================================================================================================================================================================================================

void testCopy()
{
  std::random_device RandomDevice;  //Will be used to obtain a seed for the random number engine
  std::mt19937       RandomGenerator(RandomDevice()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint32> RandomDistribution(0);

  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };

      for(const int32 m : c_Margs)
      {
        const std::string Description = fmt::sprintf("SizeXxY=%dx%d Margin=%d", x, y, m);
        
        //buffers create
        xPlane<uint16>* Src = new xPlane<uint16>(Size, 14, m);
        xPlane<uint16>* Dst = new xPlane<uint16>(Size, 14, m);

        for(int32 n = 0; n < c_NumRandomTests; n++)
        {          
          Src->fill(0);
          uint32 Seed = RandomDistribution(RandomGenerator);
          xTestUtils::fillRandom(Src->getAddr(), Src->getStride(), Src->getWidth(), Src->getHeight(), 14, Seed);
          CAPTURE(Description + fmt::sprintf(" Seed=%n", Seed));

          if(m == 0)
          {
            Dst->fill(0);
            xPixelOps::Copy(Dst->getAddr(), Src->getAddr(), Dst->getArea());
            CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));
          }

          Dst->fill(0);
          xPixelOps::Copy(Dst->getAddr(), Src->getAddr(), Dst->getStride(), Src->getStride(), Dst->getWidth(), Dst->getHeight());
          CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));

          Dst->fill(0);
          xPixelOps::CopyPart(Dst->getAddr(), Src->getAddr(), Dst->getStride(), Src->getStride(), { 0,0 }, { 0,0 }, { Dst->getWidth(), Dst->getHeight() });
          CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));
        }

        //buffers destroy
        delete Src;
        delete Dst;
      }
    }
  }
}

void testCvt(
  std::function<void(uint16*, const uint8* , int32, int32, int32, int32)>CvtU8toU16,
  std::function<void(uint8* , const uint16*, int32, int32, int32, int32)>CvtU16toU8
  )
{
  std::random_device RandomDevice;  //Will be used to obtain a seed for the random number engine
  std::mt19937       RandomGenerator(RandomDevice()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint32> RandomDistribution(0);

  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };

      for(const int32 m : c_Margs)
      {
        const std::string Description = fmt::sprintf("SizeXxY=%dx%d Margin=%d", x, y, m);

        //buffers create
        xPlane<uint16>* Src = new xPlane<uint16>(Size, 8, m);
        xPlane<uint8 >* Imm = new xPlane<uint8 >(Size, 8, m);
        xPlane<uint16>* Dst = new xPlane<uint16>(Size, 8, m);

        Src->fill(0);
        Imm->fill(0);
        Dst->fill(0);

        for(int32 n = 0; n < c_NumRandomTests; n++)
        {
          uint32 Seed = RandomDistribution(RandomGenerator);
          CAPTURE(Description + fmt::sprintf(" Seed=%d", Seed));
          xTestUtils::fillRandom(Src->getAddr(), Src->getStride(), Src->getWidth(), Src->getHeight(), 8, Seed);
          CvtU16toU8(Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
          CvtU8toU16(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
          CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));
        }

        //clip cases
        CAPTURE(Description + " Fill=256");
        Src->fill(256);
        CvtU16toU8(Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
        CvtU8toU16(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
        CHECK(xTestUtils::isEqualValue(Dst->getAddr(), Dst->getStride(), Dst->getWidth(), Dst->getHeight(), (uint16)255));

        CAPTURE(Description + " Fill=c_Max14bit");
        Src->fill(c_DefMaxValue);
        CvtU16toU8(Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
        CvtU8toU16(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
        CHECK(xTestUtils::isEqualValue(Dst->getAddr(), Dst->getStride(), Dst->getWidth(), Dst->getHeight(), (uint16)255));

        //buffers destroy
        delete Src;
        delete Imm;
        delete Dst;
      }
    }
  }
}

void testResample(
  std::function<void(uint16*, const uint16*, int32, int32, int32, int32)>Upsample,
  std::function<void(uint16*, const uint16*, int32, int32, int32, int32)>Downsample,
  const int32V2& SizeMultiplier
)
{
  std::random_device RandomDevice;  //Will be used to obtain a seed for the random number engine
  std::mt19937       RandomGenerator(RandomDevice()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint32> RandomDistribution(0);

  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };

      for(const int32 m : c_Margs)
      {
        const std::string Description = fmt::sprintf("SizeXxY=%dx%d Margin=%d", x, y, m);
        const int32V2     ImmSize     = Size * SizeMultiplier;
        const int64       AreaMult    = SizeMultiplier.getMul();
      
        //buffers create        
        xPlane<uint16>* Src = new xPlane<uint16>(Size   , 14, m);
        xPlane<uint16>* Imm = new xPlane<uint16>(ImmSize, 14, m);
        xPlane<uint16>* Dst = new xPlane<uint16>(Size   , 14, m);

        Src->fill(0);
        Imm->fill(0);
        Dst->fill(0);

        for(int32 n = 0; n < c_NumRandomTests; n++)
        {
          uint32 Seed = RandomDistribution(RandomGenerator);
          CAPTURE(Description + fmt::sprintf(" Seed=%d", Seed));
          xTestUtils::fillRandom(Src->getAddr(), Src->getStride(), Src->getWidth(), Src->getHeight(), 14, Seed);
          int64 SumSrc = xTestUtils::calcSum(Src->getAddr(), Src->getStride(), Src->getWidth(), Src->getHeight());
          Upsample  (Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
          int64 SumImm = xTestUtils::calcSum(Imm->getAddr(), Imm->getStride(), Imm->getWidth(), Imm->getHeight());
          CHECK(AreaMult * SumSrc == SumImm);
          Downsample(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
          CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));
        }

        //buffers destroy
        delete Src;
        delete Imm;
        delete Dst;
      }
    }
  }
}
void testCvtResample(
  std::function<void(uint8*, const uint16*, int32, int32, int32, int32)>CvtU16toU8,
  std::function<void(uint16*, const uint8*, int32, int32, int32, int32)>CvtUpsampleU8toU16,
  std::function<void(uint8*, const uint16*, int32, int32, int32, int32)>CvtDownsampleU16toU8,
  const int32V2& SizeMultiplier
)
{
  std::random_device RandomDevice;  //Will be used to obtain a seed for the random number engine
  std::mt19937       RandomGenerator(RandomDevice()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint32> RandomDistribution(0);

  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };

      for(const int32 m : c_Margs)
      {
        const std::string Description = fmt::sprintf("SizeXxY=%dx%d Margin=%d", x, y, m);
        const int32V2     ImmSize     = Size * SizeMultiplier;
        const int64       AreaMult    = SizeMultiplier.getMul();

        //buffers create
        xPlane<uint16>* Pre = new xPlane<uint16>(Size   , 8, m);
        xPlane<uint8 >* Src = new xPlane<uint8 >(Size   , 8, m);
        xPlane<uint16>* Imm = new xPlane<uint16>(ImmSize, 8, m);
        xPlane<uint8 >* Dst = new xPlane<uint8 >(Size   , 8, m);

        Pre->fill(0);
        Src->fill(0);
        Imm->fill(0);
        Dst->fill(0);

        for(int32 n = 0; n < c_NumRandomTests; n++)
        {
          uint32 Seed = RandomDistribution(RandomGenerator);
          CAPTURE(Description + fmt::sprintf(" Seed=%d", Seed));
          xTestUtils::fillRandom(Pre->getAddr(), Pre->getStride(), Pre->getWidth(), Pre->getHeight(), 8, Seed);
          CvtU16toU8          (Src->getAddr(), Pre->getAddr(), Src->getStride(), Pre->getStride(), Src->getWidth(), Src->getHeight());
          int64 SumSrc = xTestUtils::calcSum(Src->getAddr(), Src->getStride(), Src->getWidth(), Src->getHeight());
          CvtUpsampleU8toU16  (Imm->getAddr(), Src->getAddr(), Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight());
          int64 SumImm = xTestUtils::calcSum(Imm->getAddr(), Imm->getStride(), Imm->getWidth(), Imm->getHeight());
          CHECK(AreaMult * SumSrc == SumImm);
          CvtDownsampleU16toU8(Dst->getAddr(), Imm->getAddr(), Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight());
          CHECK(xTestUtils::isSameBuffer(Src->getBuffer(), Dst->getBuffer(), Dst->getBuffNumPels(), true));
        }

        //buffers destroy
        delete Pre;
        delete Src;
        delete Imm;
        delete Dst;
      }
    }
  }
}

void testRearrange(
  std::function<void(uint16*, const uint16*, const uint16*, const uint16*, const uint16, int32, int32, int32, int32)> AOS4fromSOA3,
  std::function<void(uint16*, uint16*, uint16*, const uint16*, int32, int32, int32, int32)> SOA3fromAOS4
)
{
  std::random_device RandomDevice;  //Will be used to obtain a seed for the random number engine
  std::mt19937       RandomGenerator(RandomDevice()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint32> RandomDistribution(0);

  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };

      for(const int32 m : c_Margs)
      {
        const std::string Description = fmt::sprintf("SizeXxY=%dx%d Margin=%d", x, y, m);

        //buffers create
        xPicP* SrcP = new xPicP(Size, 14, m);
        xPicI* ImmI = new xPicI(Size, 14, m);
        xPicP* DstP = new xPicP(Size, 14, m);

        SrcP->fill(0);
        ImmI->fill(0);
        DstP->fill(0);

        for(int32 n = 0; n < c_NumRandomTests; n++)
        {
          CAPTURE(Description + fmt::sprintf(" RandomTestCnt=%d", n));
          for(int32 c = 0; c < 3; c++)
          {
            xTestUtils::fillRandom(SrcP->getAddr((eCmp)c), SrcP->getStride(), SrcP->getWidth(), SrcP->getHeight(), 8, RandomDistribution(RandomGenerator));
          }
          AOS4fromSOA3((uint16*)(ImmI->getAddr()), SrcP->getAddr(eCmp::C0), SrcP->getAddr(eCmp::C1), SrcP->getAddr(eCmp::C2), 0, ImmI->getStride()*4, SrcP->getStride(), ImmI->getWidth(), ImmI->getHeight());
          SOA3fromAOS4(DstP->getAddr(eCmp::C0), DstP->getAddr(eCmp::C1), DstP->getAddr(eCmp::C2), (uint16*)ImmI->getAddr(), DstP->getStride(), ImmI->getStride()*4, DstP->getWidth(), DstP->getHeight());
          for(int32 c = 0; c < 3; c++)
          {
            CHECK(xTestUtils::isSameBuffer(SrcP->getBuffer((eCmp)c), DstP->getBuffer((eCmp)c), DstP->getBuffNumPels()));
          }
        }

        //buffers destroy
        delete SrcP;
        delete ImmI;
        delete DstP;
      }
    }
  }
}

void testCheckValues(std::function<bool(const uint16*, int32, int32, int32, int32)> CheckValues)
{
  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };

      for(const int32 m : c_Margs)
      {
        for(const int32 b : c_BitDs)
        {
          const std::string Description = fmt::sprintf("SizeXxY=%dx%d Margin=%d BitDepth=%d", x, y, m, b);
          CAPTURE(Description);

          const int32 MaxValue = xBitDepth2MaxValue(b);

          //buffers create
          xPlane<uint16>* P = new xPlane<uint16>(Size, b, m);

          P->fill(0);
          CHECK(CheckValues(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == true);

          P->accessPel({ 0,0 }) = uint16(MaxValue + 1);
          CHECK(CheckValues(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == false);
          P->accessPel({ 0,0 }) = 0;

          P->accessPel({ 5, 9 }) = uint16(MaxValue + 1);
          CHECK(CheckValues(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == false);
          P->accessPel({ 5, 9 }) = 0;

          P->accessPel({ x - 1, 0 }) = uint16(MaxValue + 1);
          CHECK(CheckValues(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == false);
          P->accessPel({ x - 1, 0 }) = 0;

          P->accessPel({ 0, y - 1 }) = uint16(MaxValue + 1);
          CHECK(CheckValues(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == false);
          P->accessPel({ 0, y - 1 }) = 0;

          P->accessPel({ x - 1, y - 1 }) = uint16(MaxValue + 1);
          CHECK(CheckValues(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == false);
          P->accessPel({ x - 1, y - 1 }) = 0;

          P->fill(uint16(MaxValue));
          CHECK(CheckValues(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == true);
        }
      }
    }
  }
}

void testCountNonZero(std::function<int32(const uint16*, int32, int32, int32)> CountNonZero)
{
  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };
      int64   Area = x * y;

      for(const int32 m : c_Margs)
      {
        const std::string Description = fmt::sprintf("SizeXxY=%dx%d Margin=%d", x, y, m);
        CAPTURE(Description);

        //buffers create
        xPlane<uint16>* P = new xPlane<uint16>(Size, 14, m);

        P->fill(0);
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 0);
        P->accessPel({0, 0}) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 1);
        P->accessPel({ 5, 9 }) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 2);
        P->accessPel({ 1, 1 }) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 3);
        P->accessPel({ 34, 19 }) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 4);
        P->accessPel({ x - 1, 0 }) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 5);
        P->accessPel({ 0, y - 1 }) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 6);
        P->accessPel({ x-1, y-1 }) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 7);

        P->fill(c_DefMaxValue);
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 0);
        P->accessPel({ 0, 0 }) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 1);
        P->accessPel({ 5, 9 }) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 2);
        P->accessPel({ 1, 1 }) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 3);
        P->accessPel({ 34, 19 }) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 4);
        P->accessPel({ x - 1, 0 }) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 5);
        P->accessPel({ 0, y - 1 }) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 6);
        P->accessPel({ x - 1, y - 1 }) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 7);
      }
    }
  }
}

//===============================================================================================================================================================================================================

TEST_CASE("xPixelOps::Copy")
{
  tTimePoint T = tClock::now();
  testCopy();
  fmt::printf("TIME(xPixelOps::Copy) = %fs\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}

TEST_CASE("xPixelOpsSTD")
{
  tTimePoint T = tClock::now();
  testCvt
  (
    static_cast<void(*)(uint16*, const uint8* , int32, int32, int32, int32)>(&xPixelOpsSTD::Cvt),
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsSTD::Cvt)
  );
  testResample
  (
    static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsSTD::UpsampleHV  ),
    static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsSTD::DownsampleHV),
    { 2,2 }
  );
  testCvtResample
  (
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsSTD::Cvt            ),
    static_cast<void(*)(uint16*, const uint8* , int32, int32, int32, int32)>(&xPixelOpsSTD::CvtUpsampleHV  ),
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsSTD::CvtDownsampleHV),
    { 2,2 }
  );
  testResample
  (
    static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsSTD::UpsampleH  ),
    static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsSTD::DownsampleH),
    { 2,1 }
  );
  testCvtResample
  (
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsSTD::Cvt           ),
    static_cast<void(*)(uint16*, const uint8* , int32, int32, int32, int32)>(&xPixelOpsSTD::CvtUpsampleH  ),
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsSTD::CvtDownsampleH),
    { 2,1 }
  );
  testRearrange
  (
    &xPixelOpsSTD::AOS4fromSOA3,
    &xPixelOpsSTD::SOA3fromAOS4
  );
  testCheckValues
  (
    &xPixelOpsSTD::CheckValues
  );
  testCountNonZero
  (
    &xPixelOpsSTD::CountNonZero
  );
  fmt::printf("TIME(xPixelOpsSTD) = %fs\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}

#if X_SIMD_CAN_USE_SSE
TEST_CASE("xPixelOpsSSE")
{
  tTimePoint T = tClock::now();
  testCvt
  (
    static_cast<void(*)(uint16*, const uint8* , int32, int32, int32, int32)>(&xPixelOpsSSE::Cvt),
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsSSE::Cvt)
  );
  testResample
  (
    static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsSSE::UpsampleHV  ),
    static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsSSE::DownsampleHV),
    { 2,2 }
  );
  testCvtResample
  (
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsSSE::Cvt            ),
    static_cast<void(*)(uint16*, const uint8* , int32, int32, int32, int32)>(&xPixelOpsSSE::CvtUpsampleHV  ),
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsSSE::CvtDownsampleHV),
    { 2,2 }
  );
  testResample
  (
    static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsSSE::UpsampleH  ),
    static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsSSE::DownsampleH),
    { 2,1 }
  );
  testCvtResample
  (
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsSSE::Cvt           ),
    static_cast<void(*)(uint16*, const uint8* , int32, int32, int32, int32)>(&xPixelOpsSSE::CvtUpsampleH  ),
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsSSE::CvtDownsampleH),
    { 2,1 }
  );
  testRearrange
  (
    &xPixelOpsSSE::AOS4fromSOA3,
    &xPixelOpsSSE::SOA3fromAOS4
  );
  testCheckValues
  (
    &xPixelOpsSSE::CheckValues
  );
  testCountNonZero
  (
    &xPixelOpsSSE::CountNonZero
  );
  fmt::printf("TIME(xPixelOpsSSE) = %fs\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}
#endif

#if X_SIMD_CAN_USE_AVX
TEST_CASE("xPixelOpsAVX")
{
  tTimePoint T = tClock::now();
  testCvt
  (
    static_cast<void(*)(uint16*, const uint8* , int32, int32, int32, int32)>(&xPixelOpsAVX::Cvt),
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX::Cvt)
  );
  testResample
  (
    static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX::UpsampleHV  ),
    static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX::DownsampleHV),
    { 2,2 }
  );
  testCvtResample
  (
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX::Cvt            ),
    static_cast<void(*)(uint16*, const uint8* , int32, int32, int32, int32)>(&xPixelOpsAVX::CvtUpsampleHV  ),
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX::CvtDownsampleHV),
    { 2,2 }
  );
  testResample
  (
    static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX::UpsampleH  ),
    static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX::DownsampleH),
    { 2,1 }
  );
  testCvtResample
  (
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX::Cvt           ),
    static_cast<void(*)(uint16*, const uint8* , int32, int32, int32, int32)>(&xPixelOpsAVX::CvtUpsampleH  ),
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX::CvtDownsampleH),
    { 2,1 }
  );

  testRearrange
  (
    &xPixelOpsAVX::AOS4fromSOA3,
    &xPixelOpsAVX::SOA3fromAOS4
  );
  testCheckValues
  (
    &xPixelOpsAVX::CheckValues
  );
  testCountNonZero
  (
    &xPixelOpsAVX::CountNonZero
  );
  fmt::printf("TIME(xPixelOpsAVX) = %fs\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}
#endif

#if X_SIMD_CAN_USE_AVX512
TEST_CASE("xPixelOpsAVX512")
{
  tTimePoint T = tClock::now();
  testCvt
  (
    static_cast<void(*)(uint16*, const uint8* , int32, int32, int32, int32)>(&xPixelOpsAVX512::Cvt),
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX512::Cvt)
  );
  testResample
  (
    static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX512::UpsampleHV  ),
    static_cast<void(*)(uint16*, const uint16*, int32, int32, int32, int32)>(&xPixelOpsSTD   ::DownsampleHV),
    { 2,2 }
  );
  testCvtResample
  (
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsAVX512::Cvt            ),
    static_cast<void(*)(uint16*, const uint8* , int32, int32, int32, int32)>(&xPixelOpsAVX512::CvtUpsampleHV  ),
    static_cast<void(*)(uint8* , const uint16*, int32, int32, int32, int32)>(&xPixelOpsSTD   ::CvtDownsampleHV),
    { 2,2 }
  );
  testRearrange
  (
    &xPixelOpsAVX512::AOS4fromSOA3,
    &xPixelOpsSTD   ::SOA3fromAOS4
  );
  testCheckValues
  (
    &xPixelOpsAVX512::CheckValues
  );
  testCountNonZero
  (
    &xPixelOpsAVX512::CountNonZero
  );
  fmt::printf("TIME(xPixelOpsAVX512) = %fs\n", std::chrono::duration_cast<tDurationS>(tClock::now() - T).count());
}
#endif
