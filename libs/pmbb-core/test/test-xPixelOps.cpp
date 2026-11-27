/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2025 Patrycja Kaźmierczak <patrycja.kazmierczak@student.put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <functional>
#include <random>

#include "xCommonDefCORE.h"
#include "xPixelOps.h"
#include "xPic.h"
#include "xPlane.h"
#include "xTestUtils.h"
#include "xTimeUtils.h"

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

static const std::vector<int32> c_Dimms = { 24, 48, 63, 64, 65, 80, 128, 127, 129, 255, 256, 257 };
static const std::vector<int32> c_Margs = { 0, 4, 32 };
static const std::vector<int32> c_BitDs = { 8, 10, 12, 14 };
static constexpr int32          c_DefBitDepth    = 14;
static constexpr int32          c_DefMaxValue    = (1<<c_DefBitDepth) - 1;
static constexpr int32          c_NumRandomTests = 8;

//===============================================================================================================================================================================================================

using fCvtU8toU16 = std::function<void(uint16*, const uint8*, int32, int32, int32, int32)>;
using fCvtU16toU8 = std::function<void(uint8*, const uint16*, int32, int32, int32, int32)>;

using pCvtU8toU16 = void(*)(uint16*, const uint8*, int32, int32, int32, int32);
using pCvtU16toU8 = void(*)(uint8*, const uint16*, int32, int32, int32, int32);

using fUpsample   = std::function<void(uint16*, const uint16*, int32, int32, int32, int32)>;
using fDownsample = std::function<void(uint16*, const uint16*, int32, int32, int32, int32)>;

using fCvtUpsampleU8toU16   = std::function<void(uint16*, const uint8*, int32, int32, int32, int32)>;
using fCvtDownsampleU16toU8 = std::function<void(uint8*, const uint16*, int32, int32, int32, int32)>;

using fAOS4fromSOA3 = std::function<void(uint16*, const uint16*, const uint16*, const uint16*, const uint16, int32, int32, int32, int32)>;
using fSOA3fromAOS4 = std::function<void(uint16*, uint16*, uint16*, const uint16*, int32, int32, int32, int32)>                          ;

using fCheckIfInRange = std::function<bool (const uint16*, int32, int32, int32, int32)>;
using fCountNonZero   = std::function<int32(const uint16*, int32, int32, int32)>;
using fCompareEqual   = std::function<bool (const uint16*, const uint16*, int32, int32, int32, int32)>;

//===============================================================================================================================================================================================================

void testCopy()
{
  uint32 State = xTestUtils::c_XorShiftSeed;

  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };

      for(const int32 m : c_Margs)
      {
        const std::string Description = fmt::format("SizeXxY={}x{} Margin={}", x, y, m);
        
        //buffers create
        xPlane<uint16>* Src = new xPlane<uint16>(Size, 14, m);
        xPlane<uint16>* Dst = new xPlane<uint16>(Size, 14, m);

        for(int32 n = 0; n < c_NumRandomTests; n++)
        {          
          Src->fill(0);
          CAPTURE(Description + fmt::format(" State={}", State));
          State = xTestUtils::fillRandom(Src->getAddr(), Src->getStride(), Src->getWidth(), Src->getHeight(), 14, State);
          
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

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void testCvt(fCvtU8toU16 CvtU8toU16, fCvtU16toU8 CvtU16toU8)
{
  uint32 State = xTestUtils::c_XorShiftSeed;

  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };

      for(const int32 m : c_Margs)
      {
        const std::string Description = fmt::format("SizeXxY={}x{} Margin={}", x, y, m);

        //buffers create
        xPlane<uint16>* Src = new xPlane<uint16>(Size, 8, m);
        xPlane<uint8 >* Imm = new xPlane<uint8 >(Size, 8, m);
        xPlane<uint16>* Dst = new xPlane<uint16>(Size, 8, m);

        Src->fill(0);
        Imm->fill(0);
        Dst->fill(0);

        for(int32 n = 0; n < c_NumRandomTests; n++)
        {
          CAPTURE(Description + fmt::format(" State={}", State));
          State = xTestUtils::fillRandom(Src->getAddr(), Src->getStride(), Src->getWidth(), Src->getHeight(), 8, State);
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

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void testResample(fUpsample Upsample, fDownsample Downsample, const int32V2& SizeMultiplier)
{
  uint32 State = xTestUtils::c_XorShiftSeed;

  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };

      for(const int32 m : c_Margs)
      {
        const std::string Description = fmt::format("SizeXxY={}x{} Margin={}", x, y, m);
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
          CAPTURE(Description + fmt::format(" State={}", State));
          State = xTestUtils::fillRandom(Src->getAddr(), Src->getStride(), Src->getWidth(), Src->getHeight(), 14, State);
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

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void testCvtResample(fCvtU16toU8 CvtU16toU8, fCvtUpsampleU8toU16 CvtUpsampleU8toU16, fCvtDownsampleU16toU8 CvtDownsampleU16toU8, const int32V2& SizeMultiplier)
{
  xTestUtils::xXorShiftGen32 TestGen; std::uniform_int_distribution<uint32> RandomDistribution(0);

  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };

      for(const int32 m : c_Margs)
      {
        const std::string Description = fmt::format("SizeXxY={}x{} Margin={}", x, y, m);
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
          uint32 Seed = RandomDistribution(TestGen);
          CAPTURE(Description + fmt::format(" Seed={}", Seed));
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

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void testRearrange(fAOS4fromSOA3 AOS4fromSOA3, fSOA3fromAOS4 SOA3fromAOS4)
{
  xTestUtils::xXorShiftGen32 TestGen; std::uniform_int_distribution<uint32> RandomDistribution(0);

  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };

      for(const int32 m : c_Margs)
      {
        const std::string Description = fmt::format("SizeXxY={}x{} Margin={}", x, y, m);

        //buffers create
        xPicP* SrcP = new xPicP(Size, 14, m);
        xPicI* ImmI = new xPicI(Size, 14, m);
        xPicP* DstP = new xPicP(Size, 14, m);

        SrcP->fill(0);
        ImmI->fill(0);
        DstP->fill(0);

        //simple deterministic test
        {
          CAPTURE(Description + fmt::format(" SimpleDeterministic"));
          SrcP->fill(0);
          xTestUtils::fillGradient1X(SrcP->getAddr(eCmp::C0), SrcP->getStride(), SrcP->getWidth(), SrcP->getHeight(), 14,   0);
          xTestUtils::fillGradient1X(SrcP->getAddr(eCmp::C1), SrcP->getStride(), SrcP->getWidth(), SrcP->getHeight(), 14, 100);
          xTestUtils::fillGradient1X(SrcP->getAddr(eCmp::C2), SrcP->getStride(), SrcP->getWidth(), SrcP->getHeight(), 14, 200);
          ImmI->fill(0);
          DstP->fill(0);
          AOS4fromSOA3((uint16*)(ImmI->getAddr()), SrcP->getAddr(eCmp::C0), SrcP->getAddr(eCmp::C1), SrcP->getAddr(eCmp::C2), 16384, ImmI->getStride() * 4, SrcP->getStride(), ImmI->getWidth(), ImmI->getHeight());
          SOA3fromAOS4(DstP->getAddr(eCmp::C0), DstP->getAddr(eCmp::C1), DstP->getAddr(eCmp::C2), (uint16*)ImmI->getAddr(), DstP->getStride(), ImmI->getStride() * 4, DstP->getWidth(), DstP->getHeight());
          for(int32 c = 0; c < 3; c++)
          {
            CHECK(xTestUtils::isSameBuffer(SrcP->getBuffer((eCmp)c), DstP->getBuffer((eCmp)c), DstP->getBuffNumPels(), true));
          }
        }

        //random test
        SrcP->fill(0);
        for(int32 n = 0; n < c_NumRandomTests; n++)
        {
          CAPTURE(Description + fmt::format(" RandomTestCnt={}", n));
          for(int32 c = 0; c < 3; c++) { xTestUtils::fillRandom(SrcP->getAddr((eCmp)c), SrcP->getStride(), SrcP->getWidth(), SrcP->getHeight(), 8, RandomDistribution(TestGen)); }
          ImmI->fill(0);
          DstP->fill(0);
          AOS4fromSOA3((uint16*)(ImmI->getAddr()), SrcP->getAddr(eCmp::C0), SrcP->getAddr(eCmp::C1), SrcP->getAddr(eCmp::C2), 0, ImmI->getStride()*4, SrcP->getStride(), ImmI->getWidth(), ImmI->getHeight());
          SOA3fromAOS4(DstP->getAddr(eCmp::C0), DstP->getAddr(eCmp::C1), DstP->getAddr(eCmp::C2), (uint16*)ImmI->getAddr(), DstP->getStride(), ImmI->getStride()*4, DstP->getWidth(), DstP->getHeight());
          for(int32 c = 0; c < 3; c++)
          {
            CHECK(xTestUtils::isSameBuffer(SrcP->getBuffer((eCmp)c), DstP->getBuffer((eCmp)c), DstP->getBuffNumPels(), true));
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

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void testCheckIfInRange(fCheckIfInRange CheckIfInRange)
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
          const std::string Description = fmt::format("SizeXxY={}x{} Margin={} BitDepth={}", x, y, m, b);
          CAPTURE(Description);

          const int32 MaxValue = xBitDepth2MaxValue(b);

          //buffers create
          xPlane<uint16>* P = new xPlane<uint16>(Size, b, m);

          P->fill(0);
          CHECK(CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == true);

          P->accessPel({ 0,0 }) = uint16(MaxValue + 1);
          CHECK(CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == false);
          P->accessPel({ 0,0 }) = 0;

          P->accessPel({ 5, 9 }) = uint16(MaxValue + 1);
          CHECK(CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == false);
          P->accessPel({ 5, 9 }) = 0;

          P->accessPel({ x - 1, 0 }) = uint16(MaxValue + 1);
          CHECK(CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == false);
          P->accessPel({ x - 1, 0 }) = 0;

          P->accessPel({ 0, y - 1 }) = uint16(MaxValue + 1);
          CHECK(CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == false);
          P->accessPel({ 0, y - 1 }) = 0;

          P->accessPel({ x - 1, y - 1 }) = uint16(MaxValue + 1);
          CHECK(CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == false);
          P->accessPel({ x - 1, y - 1 }) = 0;

          P->fill(uint16(MaxValue));
          CHECK(CheckIfInRange(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), b) == true);

          //buffers destroy
          delete P;
        }
      }
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void testCountNonZero(fCountNonZero CountNonZero)
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
        const std::string Description = fmt::format("SizeXxY={}x{} Margin={}", x, y, m);
        CAPTURE(Description);

        //buffers create
        xPlane<uint16>* P = new xPlane<uint16>(Size, 14, m);

        //almost zero
        P->fill(0);
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 0);
        P->accessPel({0, 0}) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 1);
        P->accessPel({ 5, 9 }) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 2);
        P->accessPel({ 1, 1 }) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 3);
        P->accessPel({ x - 1, 0 }) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 4);
        P->accessPel({ 0, y - 1 }) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 5);
        P->accessPel({ x-1, y-1 }) = 1;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 6);
        if(x >= 35 && y >= 20)
        {
          P->accessPel({ 34, 19 }) = 1;
          CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == 7);
        }


        //almost all
        P->fill(c_DefMaxValue);
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 0);
        P->accessPel({ 0, 0 }) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 1);
        P->accessPel({ 5, 9 }) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 2);
        P->accessPel({ 1, 1 }) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 3);
        P->accessPel({ x - 1, 0 }) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 4);
        P->accessPel({ 0, y - 1 }) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 5);
        P->accessPel({ x - 1, y - 1 }) = 0;
        CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 6);
        if(x >= 35 && y >= 20)
        {
          P->accessPel({ 34, 19 }) = 0;
          CHECK(CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight()) == Area - 7);
        }


        //random
        P->fill(0);
        State = xTestUtils::fillRandom01(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight(), State);
        uint32 RefNonZero = xPixelOpsSTD::CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight());
        uint32 TstNonZero = CountNonZero(P->getAddr(), P->getStride(), P->getWidth(), P->getHeight());
        CHECK(RefNonZero == TstNonZero);

        //buffers destroy
        delete P;
      }
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void testCompareEqual(fCompareEqual CompareEqual)
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
          const std::string Description = fmt::format("SizeXxY={}x{} Margin={} BitDepth={}", x, y, m, b);
          CAPTURE(Description);

          const int32 MaxValue = xBitDepth2MaxValue(b);

          //buffers create
          xPlane<uint16>* R = new xPlane<uint16>(Size, b, m);
          xPlane<uint16>* T = new xPlane<uint16>(Size, b, m);

          R->fill(0);
          T->fill(0);
          CHECK(CompareEqual(T->getAddr(), R->getAddr(), T->getStride(), R->getStride(), R->getWidth(), R->getHeight()) == true);

          T->accessPel({ 0,0 }) = uint16(MaxValue + 1);
          CHECK(CompareEqual(T->getAddr(), R->getAddr(), T->getStride(), R->getStride(), R->getWidth(), R->getHeight()) == false);
          T->accessPel({ 0,0 }) = 0;

          T->accessPel({ 5, 9 }) = uint16(1);
          CHECK(CompareEqual(T->getAddr(), R->getAddr(), T->getStride(), R->getStride(), R->getWidth(), R->getHeight()) == false);
          T->accessPel({ 5, 9 }) = 0;

          T->accessPel({ x - 1, 0 }) = uint16(1);
          CHECK(CompareEqual(T->getAddr(), R->getAddr(), T->getStride(), R->getStride(), R->getWidth(), R->getHeight()) == false);
          T->accessPel({ x - 1, 0 }) = 0;

          T->accessPel({ 0, y - 1 }) = uint16(1);
          CHECK(CompareEqual(T->getAddr(), R->getAddr(), T->getStride(), R->getStride(), R->getWidth(), R->getHeight()) == false);
          T->accessPel({ 0, y - 1 }) = 0;

          T->accessPel({ x - 1, y - 1 }) = uint16(1);
          CHECK(CompareEqual(T->getAddr(), R->getAddr(), T->getStride(), R->getStride(), R->getWidth(), R->getHeight()) == false);
          T->accessPel({ x - 1, y - 1 }) = 0;

          R->fill(uint16(MaxValue));
          T->fill(uint16(MaxValue));
          CHECK(CompareEqual(T->getAddr(), R->getAddr(), T->getStride(), R->getStride(), R->getWidth(), R->getHeight()) == true);

          //buffers destroy
          delete R;
          delete T;
        }
      }
    }
  }
}

//===============================================================================================================================================================================================================

TEST_CASE("xPixelOps::Copy")
{
  testCopy();
}

TEST_CASE("xPixelOpsSTD")
{
  testCvt           (static_cast<pCvtU8toU16>(&xPixelOpsSTD::Cvt), static_cast<pCvtU16toU8>(&xPixelOpsSTD::Cvt));
  testResample      (&xPixelOpsSTD::UpsampleHV  ,&xPixelOpsSTD::DownsampleHV, { 2,2 });
  testCvtResample   (static_cast<pCvtU16toU8>(&xPixelOpsSTD::Cvt), &xPixelOpsSTD::CvtUpsampleHV, &xPixelOpsSTD::CvtDownsampleHV, { 2,2 });
  testResample      (&xPixelOpsSTD::UpsampleH, &xPixelOpsSTD::DownsampleH, { 2,1 } );
  testCvtResample   (static_cast<pCvtU16toU8>(&xPixelOpsSTD::Cvt), &xPixelOpsSTD::CvtUpsampleH, &xPixelOpsSTD::CvtDownsampleH, { 2,1 });
  testRearrange     (&xPixelOpsSTD::AOS4fromSOA3, &xPixelOpsSTD::SOA3fromAOS4);
  testCheckIfInRange(&xPixelOpsSTD::CheckIfInRange                           );
  testCountNonZero  (&xPixelOpsSTD::CountNonZero                             );
  testCompareEqual  (&xPixelOpsSTD::CompareEqual                             );
}

#if X_SIMD_CAN_USE_SSE
TEST_CASE("xPixelOpsSSE")
{
  testCvt           (static_cast<pCvtU8toU16>(&xPixelOpsSSE::Cvt), static_cast<pCvtU16toU8>(&xPixelOpsSSE::Cvt));
  testResample      (&xPixelOpsSSE::UpsampleHV  ,&xPixelOpsSSE::DownsampleHV, { 2,2 });
  testCvtResample   (static_cast<pCvtU16toU8>(&xPixelOpsSSE::Cvt), &xPixelOpsSSE::CvtUpsampleHV, &xPixelOpsSSE::CvtDownsampleHV, { 2,2 });
  testResample      (&xPixelOpsSSE::UpsampleH, &xPixelOpsSSE::DownsampleH, { 2,1 } );
  testCvtResample   (static_cast<pCvtU16toU8>(&xPixelOpsSSE::Cvt), &xPixelOpsSSE::CvtUpsampleH, &xPixelOpsSSE::CvtDownsampleH, { 2,1 });
  testRearrange     (&xPixelOpsSSE::AOS4fromSOA3, &xPixelOpsSSE::SOA3fromAOS4);
  testCheckIfInRange(&xPixelOpsSSE::CheckIfInRange                           );
  testCountNonZero  (&xPixelOpsSSE::CountNonZero                             );
  testCompareEqual  (&xPixelOpsSSE::CompareEqual                             );
}
#endif //X_SIMD_CAN_USE_SSE

#if X_SIMD_CAN_USE_AVX
TEST_CASE("xPixelOpsAVX")
{
  testCvt           (static_cast<pCvtU8toU16>(&xPixelOpsAVX::Cvt), static_cast<pCvtU16toU8>(&xPixelOpsAVX::Cvt));
  testResample      (&xPixelOpsAVX::UpsampleHV  ,&xPixelOpsAVX::DownsampleHV, { 2,2 });
  testCvtResample   (static_cast<pCvtU16toU8>(&xPixelOpsAVX::Cvt), &xPixelOpsAVX::CvtUpsampleHV, &xPixelOpsAVX::CvtDownsampleHV, { 2,2 });
  testResample      (&xPixelOpsAVX::UpsampleH, &xPixelOpsAVX::DownsampleH, { 2,1 } );
  testCvtResample   (static_cast<pCvtU16toU8>(&xPixelOpsAVX::Cvt), &xPixelOpsAVX::CvtUpsampleH, &xPixelOpsAVX::CvtDownsampleH, { 2,1 });
  testRearrange     (&xPixelOpsAVX::AOS4fromSOA3, &xPixelOpsAVX::SOA3fromAOS4);
  testCheckIfInRange(&xPixelOpsAVX::CheckIfInRange                           );
  testCountNonZero  (&xPixelOpsAVX::CountNonZero                             );
  testCompareEqual  (&xPixelOpsAVX::CompareEqual                             );
}
#endif //X_SIMD_CAN_USE_AVX

#if X_SIMD_CAN_USE_AVX512
TEST_CASE("xPixelOpsAVX512")
{
  testCvt           (static_cast<pCvtU8toU16>(&xPixelOpsAVX512::Cvt), static_cast<pCvtU16toU8>(&xPixelOpsAVX512::Cvt));
  testResample      (&xPixelOpsAVX512::UpsampleHV  ,&xPixelOpsAVX512::DownsampleHV, { 2,2 });
  //testCvtResample   (static_cast<pCvtU16toU8>(&xPixelOpsAVX512::Cvt), &xPixelOpsAVX512::CvtUpsampleHV, &xPixelOpsAVX512::CvtDownsampleHV, { 2,2 });
  //testResample      (&xPixelOpsAVX512::UpsampleH, &xPixelOpsAVX512::DownsampleH, { 2,1 } );
  //testCvtResample   (static_cast<pCvtU16toU8>(&xPixelOpsAVX512::Cvt), &xPixelOpsAVX512::CvtUpsampleH, &xPixelOpsAVX512::CvtDownsampleH, { 2,1 });
  testRearrange     (&xPixelOpsAVX512::AOS4fromSOA3, &xPixelOpsAVX512::SOA3fromAOS4);
  testCheckIfInRange(&xPixelOpsAVX512::CheckIfInRange                              );
  testCountNonZero  (&xPixelOpsAVX512::CountNonZero                                );
  testCompareEqual  (&xPixelOpsAVX512::CompareEqual                                );
}
#endif //X_SIMD_CAN_USE_AVX512

#if X_SIMD_CAN_USE_NEON
TEST_CASE("xPixelOpsNEON")
{
  testCvt           (static_cast<pCvtU8toU16>(&xPixelOpsNEON::Cvt), static_cast<pCvtU16toU8>(&xPixelOpsNEON::Cvt));
  testResample      (&xPixelOpsNEON::UpsampleHV  ,&xPixelOpsNEON::DownsampleHV, { 2,2 });
  testCvtResample   (static_cast<pCvtU16toU8>(&xPixelOpsNEON::Cvt), &xPixelOpsNEON::CvtUpsampleHV, &xPixelOpsNEON::CvtDownsampleHV, { 2,2 });
  testResample      (&xPixelOpsNEON::UpsampleH, &xPixelOpsNEON::DownsampleH, { 2,1 } );
  testCvtResample   (static_cast<pCvtU16toU8>(&xPixelOpsNEON::Cvt), &xPixelOpsNEON::CvtUpsampleH, &xPixelOpsNEON::CvtDownsampleH, { 2,1 });
  testRearrange     (&xPixelOpsNEON::AOS4fromSOA3, &xPixelOpsNEON::SOA3fromAOS4);
  testCheckIfInRange(&xPixelOpsNEON::CheckIfInRange                            );
  testCountNonZero  (&xPixelOpsNEON::CountNonZero                              );
  testCompareEqual  (&xPixelOpsNEON::CompareEqual                              );
}
#endif //X_SIMD_CAN_USE_NEON

//===============================================================================================================================================================================================================