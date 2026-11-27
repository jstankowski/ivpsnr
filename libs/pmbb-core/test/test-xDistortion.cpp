/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2025 Patrycja Kaźmierczak <patrycja.kazmierczak@student.put.poznan.pl>
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

static const std::vector<int32> c_Dimms = { 15, 16, 17, 31, 32, 33, 63, 64, 65, 128, 127, 129 };
static const std::vector<int32> c_Margs = { 0, 4, 32 };
static const std::vector<int32> c_BitDs = { 12, 14, 16 };

//===============================================================================================================================================================================================================

void testDistortionSD(std::function<int64(const uint16*, const uint16*, int32, int32, int32, int32)>FunSD, int32 MaxBitDepth)
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
          if(b > MaxBitDepth) { continue; }
          const std::string Description = fmt::format("SizeXxY={}x{} Margin={} BitDepth={}", x, y, m, b);
        
          //buffers create
          tPlane* PL = new tPlane(Size, b, m);
          tPlane* PC = new tPlane(Size, b, m);
          tPlane* PU = new tPlane(Size, b, m);

          int64  SD  = 0;
          //sets of near constant values
          const std::vector<int32> Cntrs = { 1, xBitDepth2MidValue(b) -1, xBitDepth2MidValue(b), xBitDepth2MidValue(b) + 1, xBitDepth2MaxValue(b) - 1 };
          for(const int32 c : Cntrs)
          {
            CAPTURE(Description + fmt::format(" Center={}", c));

            PL->fill(uint16(c-1));
            PC->fill(uint16(c  ));
            PU->fill(uint16(c+1));

            SD  = FunSD (PL->getAddr(), PL->getAddr(), PL->getStride(), PL->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SD == 0      );
            SD  = FunSD (PC->getAddr(), PC->getAddr(), PC->getStride(), PC->getStride(), PC->getWidth(), PC->getHeight()); CHECK(SD == 0      );
            SD  = FunSD (PU->getAddr(), PU->getAddr(), PU->getStride(), PU->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SD == 0      );
            SD  = FunSD (PC->getAddr(), PL->getAddr(), PC->getStride(), PL->getStride(), PC->getWidth(), PC->getHeight()); CHECK(SD == Area   );
            SD  = FunSD (PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SD == 2*Area );
            SD  = FunSD (PL->getAddr(), PC->getAddr(), PL->getStride(), PC->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SD == -Area  );
            SD  = FunSD (PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SD == -2*Area);
            SD  = FunSD (PU->getAddr(), PC->getAddr(), PU->getStride(), PC->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SD == Area   );
            SD  = FunSD (PC->getAddr(), PU->getAddr(), PC->getStride(), PU->getStride(), PC->getWidth(), PC->getHeight()); CHECK(SD == -Area  );
          }
        
          //extreme constant values
          const int64 MaxVal = xBitDepth2MaxValue(b);
          CAPTURE(Description + fmt::format(" extreme constant values Max={}", MaxVal));
          PL->fill(             0);
          PU->fill((uint16)MaxVal);
          if(b < 16 || Area < 8192)
          {
            SD  = FunSD (PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SD  == -MaxVal * Area);
            SD  = FunSD (PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SD  ==  MaxVal * Area);
          }

          //pseudo-random values
          CAPTURE(Description + " pseudo-random values");
          PL->fill(0);
          PU->fill(0);
          uint32 OrgState = State;
          State = xTestUtils::fillMidNoise(PL->getAddr(), PL->getStride(), PL->getWidth(), PL->getHeight(), PL->getBitDepth(), 0, OrgState);
          State = xTestUtils::fillMidNoise(PU->getAddr(), PU->getStride(), PU->getWidth(), PU->getHeight(), PU->getBitDepth(), 1, OrgState);
          SD  = FunSD (PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SD  == -Area);
          SD  = FunSD (PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SD  ==  Area);

          //gradient values
          CAPTURE(Description + " gradient values");
          xTestUtils::fillGradientXY(PL->getAddr(), PL->getStride(), PL->getWidth(), PL->getHeight(), PL->getBitDepth(), 0);
          xTestUtils::fillGradientXY(PU->getAddr(), PU->getStride(), PU->getWidth(), PU->getHeight(), PU->getBitDepth(), 1);
          SD  = FunSD (PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SD  == -Area);
          SD  = FunSD (PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SD  ==  Area);

          //buffers destroy
          delete PL;
          delete PC;
          delete PU;
        }
      }
    }
  }    
}

void testDistortionSAD(std::function<uint64(const uint16*, const uint16*, int32, int32, int32, int32)>FunSAD, int32 MaxBitDepth)
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
          if(b > MaxBitDepth) { continue; }
          const std::string Description = fmt::format("SizeXxY={}x{} Margin={} BitDepth={}", x, y, m, b);
        
          //buffers create
          tPlane* PL = new tPlane(Size, b, m);
          tPlane* PC = new tPlane(Size, b, m);
          tPlane* PU = new tPlane(Size, b, m);

          uint64 SAD = 0;

          //sets of near constant values
          const std::vector<int32> Cntrs = { 1, xBitDepth2MidValue(b) -1, xBitDepth2MidValue(b), xBitDepth2MidValue(b) + 1, xBitDepth2MaxValue(b) - 1 };
          for(const int32 c : Cntrs)
          {
            CAPTURE(Description + fmt::format(" Center={}", c));

            PL->fill(uint16(c-1));
            PC->fill(uint16(c  ));
            PU->fill(uint16(c+1));


            SAD = FunSAD(PL->getAddr(), PL->getAddr(), PL->getStride(), PL->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SAD == 0     );
            SAD = FunSAD(PC->getAddr(), PC->getAddr(), PC->getStride(), PC->getStride(), PC->getWidth(), PC->getHeight()); CHECK(SAD == 0     );
            SAD = FunSAD(PU->getAddr(), PU->getAddr(), PU->getStride(), PU->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SAD == 0     );
            SAD = FunSAD(PC->getAddr(), PL->getAddr(), PC->getStride(), PL->getStride(), PC->getWidth(), PC->getHeight()); CHECK(SAD == Area  );
            SAD = FunSAD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SAD == 2*Area);
            SAD = FunSAD(PL->getAddr(), PC->getAddr(), PL->getStride(), PC->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SAD == Area  );
            SAD = FunSAD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SAD == 2*Area);
            SAD = FunSAD(PU->getAddr(), PC->getAddr(), PU->getStride(), PC->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SAD == Area  );
            SAD = FunSAD(PC->getAddr(), PU->getAddr(), PC->getStride(), PU->getStride(), PC->getWidth(), PC->getHeight()); CHECK(SAD == Area  );
          }
        
          //extreme constant values
          const int64 MaxVal = xBitDepth2MaxValue(b);
          CAPTURE(Description + fmt::format(" extreme constant values Max={}", MaxVal));
          PL->fill(             0);
          PU->fill((uint16)MaxVal);
          if(b < 16 || Area < 8192)
          {
            SAD = FunSAD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SAD ==  MaxVal * Area);
            SAD = FunSAD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SAD ==  MaxVal * Area);
          }

          //pseudo-random values
          CAPTURE(Description + " pseudo-random values");
          PL->fill(0);
          PU->fill(0);
          uint32 OrgState = State;
          State = xTestUtils::fillMidNoise(PL->getAddr(), PL->getStride(), PL->getWidth(), PL->getHeight(), PL->getBitDepth(), 0, OrgState);
          State = xTestUtils::fillMidNoise(PU->getAddr(), PU->getStride(), PU->getWidth(), PU->getHeight(), PU->getBitDepth(), 1, OrgState);
          SAD = FunSAD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SAD ==  Area);
          SAD = FunSAD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SAD ==  Area);

          //gradient values
          CAPTURE(Description + " gradient values");
          xTestUtils::fillGradientXY(PL->getAddr(), PL->getStride(), PL->getWidth(), PL->getHeight(), PL->getBitDepth(), 0);
          xTestUtils::fillGradientXY(PU->getAddr(), PU->getStride(), PU->getWidth(), PU->getHeight(), PU->getBitDepth(), 1);
          SAD = FunSAD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SAD ==  Area);
          SAD = FunSAD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SAD ==  Area);

          //buffers destroy
          delete PL;
          delete PC;
          delete PU;
        }
      }
    }
  }    
}

void testDistortionSSD(std::function<uint64(const uint16*, const uint16*, int32, int32, int32, int32)>FunSSD, int32 MaxBitDepth)
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
          if(b > MaxBitDepth) { continue; }
          const std::string Description = fmt::format("SizeXxY={}x{} Margin={} BitDepth={}", x, y, m, b);
        
          //buffers create
          tPlane* PL = new tPlane(Size, b, m);
          tPlane* PC = new tPlane(Size, b, m);
          tPlane* PU = new tPlane(Size, b, m);

          uint64 SSD = 0;

          //sets of near constant values
          const std::vector<int32> Cntrs = { 1, xBitDepth2MidValue(b) -1, xBitDepth2MidValue(b), xBitDepth2MidValue(b) + 1, xBitDepth2MaxValue(b) - 1 };
          for(const int32 c : Cntrs)
          {
            CAPTURE(Description + fmt::format(" Center={}", c));

            PL->fill(uint16(c-1));
            PC->fill(uint16(c  ));
            PU->fill(uint16(c+1));

            SSD = FunSSD(PL->getAddr(), PL->getAddr(), PL->getStride(), PL->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SSD == 0     );
            SSD = FunSSD(PC->getAddr(), PC->getAddr(), PC->getStride(), PC->getStride(), PC->getWidth(), PC->getHeight()); CHECK(SSD == 0     );
            SSD = FunSSD(PU->getAddr(), PU->getAddr(), PU->getStride(), PU->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SSD == 0     );
            SSD = FunSSD(PC->getAddr(), PL->getAddr(), PC->getStride(), PL->getStride(), PC->getWidth(), PC->getHeight()); CHECK(SSD == Area  );
            SSD = FunSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SSD == 4*Area);
            SSD = FunSSD(PL->getAddr(), PC->getAddr(), PL->getStride(), PC->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SSD == Area  );
            SSD = FunSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SSD == 4*Area);
            SSD = FunSSD(PU->getAddr(), PC->getAddr(), PU->getStride(), PC->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SSD == Area  );
            SSD = FunSSD(PC->getAddr(), PU->getAddr(), PC->getStride(), PU->getStride(), PC->getWidth(), PC->getHeight()); CHECK(SSD == Area  );
          }
        
          //extreme constant values
          const int64 MaxVal = xBitDepth2MaxValue(b);
          CAPTURE(Description + fmt::format(" extreme constant values Max={}", MaxVal));
          PL->fill(             0);
          PU->fill((uint16)MaxVal);
          if(b < 16 || Area < 8192)
          {
            SSD = FunSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SSD == xPow2<int64>(MaxVal) * Area);
            SSD = FunSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SSD == xPow2<int64>(MaxVal) * Area);

          }

          //pseudo-random values
          CAPTURE(Description + " pseudo-random values");
          PL->fill(0);
          PU->fill(0);
          uint32 OrgState = State;
          State = xTestUtils::fillMidNoise(PL->getAddr(), PL->getStride(), PL->getWidth(), PL->getHeight(), PL->getBitDepth(), 0, OrgState);
          State = xTestUtils::fillMidNoise(PU->getAddr(), PU->getStride(), PU->getWidth(), PU->getHeight(), PU->getBitDepth(), 1, OrgState);
          SSD = FunSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SSD ==  Area);
          SSD = FunSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SSD ==  Area);

          //gradient values
          CAPTURE(Description + " gradient values");
          xTestUtils::fillGradientXY(PL->getAddr(), PL->getStride(), PL->getWidth(), PL->getHeight(), PL->getBitDepth(), 0);
          xTestUtils::fillGradientXY(PU->getAddr(), PU->getStride(), PU->getWidth(), PU->getHeight(), PU->getBitDepth(), 1);
          SSD = FunSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SSD ==  Area);
          SSD = FunSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SSD ==  Area);

          //buffers destroy
          delete PL;
          delete PC;
          delete PU;
        }
      }
    }
  }    
}

void testDistortionSSS(std::function<tSDSSD(const uint16*, const uint16*, int32, int32, int32, int32)>FunSSS, int32 MaxBitDepth)
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
          if(b > MaxBitDepth) { continue; }
          const std::string Description = fmt::format("SizeXxY={}x{} Margin={} BitDepth={}", x, y, m, b);
        
          //buffers create
          tPlane* PL = new tPlane(Size, b, m);
          tPlane* PC = new tPlane(Size, b, m);
          tPlane* PU = new tPlane(Size, b, m);

          int64  SD  = 0;
          uint64 SSD = 0;

          //sets of near constant values
          const std::vector<int32> Cntrs = { 1, xBitDepth2MidValue(b) -1, xBitDepth2MidValue(b), xBitDepth2MidValue(b) + 1, xBitDepth2MaxValue(b) - 1 };
          for(const int32 c : Cntrs)
          {
            CAPTURE(Description + fmt::format(" Center={}", c));

            PL->fill(uint16(c-1));
            PC->fill(uint16(c  ));
            PU->fill(uint16(c+1));

            std::tie(SD, SSD) = FunSSS(PL->getAddr(), PL->getAddr(), PL->getStride(), PL->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SD == 0      ); CHECK(SSD == 0     );
            std::tie(SD, SSD) = FunSSS(PC->getAddr(), PC->getAddr(), PC->getStride(), PC->getStride(), PC->getWidth(), PC->getHeight()); CHECK(SD == 0      ); CHECK(SSD == 0     );
            std::tie(SD, SSD) = FunSSS(PU->getAddr(), PU->getAddr(), PU->getStride(), PU->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SD == 0      ); CHECK(SSD == 0     );
            std::tie(SD, SSD) = FunSSS(PC->getAddr(), PL->getAddr(), PC->getStride(), PL->getStride(), PC->getWidth(), PC->getHeight()); CHECK(SD == Area   ); CHECK(SSD == Area  );
            std::tie(SD, SSD) = FunSSS(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SD == 2*Area ); CHECK(SSD == 4*Area);
            std::tie(SD, SSD) = FunSSS(PL->getAddr(), PC->getAddr(), PL->getStride(), PC->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SD == -Area  ); CHECK(SSD == Area  );
            std::tie(SD, SSD) = FunSSS(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SD == -2*Area); CHECK(SSD == 4*Area);
            std::tie(SD, SSD) = FunSSS(PU->getAddr(), PC->getAddr(), PU->getStride(), PC->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SD == Area   ); CHECK(SSD == Area  );
            std::tie(SD, SSD) = FunSSS(PC->getAddr(), PU->getAddr(), PC->getStride(), PU->getStride(), PC->getWidth(), PC->getHeight()); CHECK(SD == -Area  ); CHECK(SSD == Area  );
          }
        
          //extreme constant values
          const int64 MaxVal = xBitDepth2MaxValue(b);
          CAPTURE(Description + fmt::format(" extreme constant values Max={}", MaxVal));
          PL->fill(             0);
          PU->fill((uint16)MaxVal);
          if(b < 16 || Area < 8192)
          {
            std::tie(SD, SSD) = FunSSS(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SD == -MaxVal * Area); CHECK(SSD == xPow2<int64>(MaxVal) * Area);
            std::tie(SD, SSD) = FunSSS(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SD ==  MaxVal * Area); CHECK(SSD == xPow2<int64>(MaxVal) * Area);

          }

          //pseudo-random values
          CAPTURE(Description + " pseudo-random values");
          PL->fill(0);
          PU->fill(0);
          uint32 OrgState = State;
          State = xTestUtils::fillMidNoise(PL->getAddr(), PL->getStride(), PL->getWidth(), PL->getHeight(), PL->getBitDepth(), 0, OrgState);
          State = xTestUtils::fillMidNoise(PU->getAddr(), PU->getStride(), PU->getWidth(), PU->getHeight(), PU->getBitDepth(), 1, OrgState);
          std::tie(SD, SSD) = FunSSS(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SD  == -Area); CHECK(SSD == Area);
          std::tie(SD, SSD) = FunSSS(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SD  ==  Area); CHECK(SSD == Area);

          //gradient values
          CAPTURE(Description + " gradient values");
          xTestUtils::fillGradientXY(PL->getAddr(), PL->getStride(), PL->getWidth(), PL->getHeight(), PL->getBitDepth(), 0);
          xTestUtils::fillGradientXY(PU->getAddr(), PU->getStride(), PU->getWidth(), PU->getHeight(), PU->getBitDepth(), 1);
          std::tie(SD, SSD) = FunSSS(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight()); CHECK(SD  == -Area); CHECK(SSD == Area);
          std::tie(SD, SSD) = FunSSS(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight()); CHECK(SD  ==  Area); CHECK(SSD == Area);

          //buffers destroy
          delete PL;
          delete PC;
          delete PU;
        }
      }
    }
  }    
}

void testDistortion(
  std::function<int64 (const uint16*, const uint16*, int32, int32, int32, int32, int32)>FunSD ,
  std::function<uint64(const uint16*, const uint16*, int32, int32, int32, int32, int32)>FunSAD,
  std::function<uint64(const uint16*, const uint16*, int32, int32, int32, int32, int32)>FunSSD,
  std::function<tSDSSD(const uint16*, const uint16*, int32, int32, int32, int32, int32)>FunSSS)
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

          int64  SD  = 0;
          uint64 SAD = 0;
          uint64 SSD = 0;

          //sets of near constant values
          const std::vector<int32> Cntrs = { 1, xBitDepth2MidValue(b) -1, xBitDepth2MidValue(b), xBitDepth2MidValue(b) + 1, xBitDepth2MaxValue(b) - 1 };
          for(const int32 c : Cntrs)
          {
            CAPTURE(Description + fmt::format(" Center={}", c));

            PL->fill(uint16(c-1));
            PC->fill(uint16(c  ));
            PU->fill(uint16(c+1));

            SD  = FunSD (PL->getAddr(), PL->getAddr(), PL->getStride(), PL->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SD == 0      );
            SD  = FunSD (PC->getAddr(), PC->getAddr(), PC->getStride(), PC->getStride(), PC->getWidth(), PC->getHeight(), b); CHECK(SD == 0      );
            SD  = FunSD (PU->getAddr(), PU->getAddr(), PU->getStride(), PU->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SD == 0      );
            SD  = FunSD (PC->getAddr(), PL->getAddr(), PC->getStride(), PL->getStride(), PC->getWidth(), PC->getHeight(), b); CHECK(SD == Area   );
            SD  = FunSD (PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SD == 2*Area );
            SD  = FunSD (PL->getAddr(), PC->getAddr(), PL->getStride(), PC->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SD == -Area  );
            SD  = FunSD (PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SD == -2*Area);
            SD  = FunSD (PU->getAddr(), PC->getAddr(), PU->getStride(), PC->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SD == Area   );
            SD  = FunSD (PC->getAddr(), PU->getAddr(), PC->getStride(), PU->getStride(), PC->getWidth(), PC->getHeight(), b); CHECK(SD == -Area  );

            SAD = FunSAD(PL->getAddr(), PL->getAddr(), PL->getStride(), PL->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SAD == 0     );
            SAD = FunSAD(PC->getAddr(), PC->getAddr(), PC->getStride(), PC->getStride(), PC->getWidth(), PC->getHeight(), b); CHECK(SAD == 0     );
            SAD = FunSAD(PU->getAddr(), PU->getAddr(), PU->getStride(), PU->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SAD == 0     );
            SAD = FunSAD(PC->getAddr(), PL->getAddr(), PC->getStride(), PL->getStride(), PC->getWidth(), PC->getHeight(), b); CHECK(SAD == Area  );
            SAD = FunSAD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SAD == 2*Area);
            SAD = FunSAD(PL->getAddr(), PC->getAddr(), PL->getStride(), PC->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SAD == Area  );
            SAD = FunSAD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SAD == 2*Area);
            SAD = FunSAD(PU->getAddr(), PC->getAddr(), PU->getStride(), PC->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SAD == Area  );
            SAD = FunSAD(PC->getAddr(), PU->getAddr(), PC->getStride(), PU->getStride(), PC->getWidth(), PC->getHeight(), b); CHECK(SAD == Area  );

            SSD = FunSSD(PL->getAddr(), PL->getAddr(), PL->getStride(), PL->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SSD == 0     );
            SSD = FunSSD(PC->getAddr(), PC->getAddr(), PC->getStride(), PC->getStride(), PC->getWidth(), PC->getHeight(), b); CHECK(SSD == 0     );
            SSD = FunSSD(PU->getAddr(), PU->getAddr(), PU->getStride(), PU->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SSD == 0     );
            SSD = FunSSD(PC->getAddr(), PL->getAddr(), PC->getStride(), PL->getStride(), PC->getWidth(), PC->getHeight(), b); CHECK(SSD == Area  );
            SSD = FunSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SSD == 4*Area);
            SSD = FunSSD(PL->getAddr(), PC->getAddr(), PL->getStride(), PC->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SSD == Area  );
            SSD = FunSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SSD == 4*Area);
            SSD = FunSSD(PU->getAddr(), PC->getAddr(), PU->getStride(), PC->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SSD == Area  );
            SSD = FunSSD(PC->getAddr(), PU->getAddr(), PC->getStride(), PU->getStride(), PC->getWidth(), PC->getHeight(), b); CHECK(SSD == Area  );

            std::tie(SD, SSD) = FunSSS(PL->getAddr(), PL->getAddr(), PL->getStride(), PL->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SD == 0      ); CHECK(SSD == 0     );
            std::tie(SD, SSD) = FunSSS(PC->getAddr(), PC->getAddr(), PC->getStride(), PC->getStride(), PC->getWidth(), PC->getHeight(), b); CHECK(SD == 0      ); CHECK(SSD == 0     );
            std::tie(SD, SSD) = FunSSS(PU->getAddr(), PU->getAddr(), PU->getStride(), PU->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SD == 0      ); CHECK(SSD == 0     );
            std::tie(SD, SSD) = FunSSS(PC->getAddr(), PL->getAddr(), PC->getStride(), PL->getStride(), PC->getWidth(), PC->getHeight(), b); CHECK(SD == Area   ); CHECK(SSD == Area  );
            std::tie(SD, SSD) = FunSSS(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SD == 2*Area ); CHECK(SSD == 4*Area);
            std::tie(SD, SSD) = FunSSS(PL->getAddr(), PC->getAddr(), PL->getStride(), PC->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SD == -Area  ); CHECK(SSD == Area  );
            std::tie(SD, SSD) = FunSSS(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SD == -2*Area); CHECK(SSD == 4*Area);
            std::tie(SD, SSD) = FunSSS(PU->getAddr(), PC->getAddr(), PU->getStride(), PC->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SD == Area   ); CHECK(SSD == Area  );
            std::tie(SD, SSD) = FunSSS(PC->getAddr(), PU->getAddr(), PC->getStride(), PU->getStride(), PC->getWidth(), PC->getHeight(), b); CHECK(SD == -Area  ); CHECK(SSD == Area  );
          }
        
          //extreme constant values
          const int64 MaxVal = xBitDepth2MaxValue(b);
          CAPTURE(Description + fmt::format(" extreme constant values Max={}", MaxVal));
          PL->fill(             0);
          PU->fill((uint16)MaxVal);
          if(b < 16 || Area < 8192)
          {
            SD  = FunSD (PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SD  == -MaxVal * Area);
            SD  = FunSD (PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SD  ==  MaxVal * Area);
            SAD = FunSAD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SAD ==  MaxVal * Area);
            SAD = FunSAD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SAD ==  MaxVal * Area);
            SSD = FunSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SSD == xPow2<int64>(MaxVal) * Area);
            SSD = FunSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SSD == xPow2<int64>(MaxVal) * Area);
            std::tie(SD, SSD) = FunSSS(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SD == -MaxVal * Area); CHECK(SSD == xPow2<int64>(MaxVal) * Area);
            std::tie(SD, SSD) = FunSSS(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SD ==  MaxVal * Area); CHECK(SSD == xPow2<int64>(MaxVal) * Area);

          }

          //pseudo-random values
          CAPTURE(Description + " pseudo-random values");
          PL->fill(0);
          PU->fill(0);
          uint32 OrgState = State;
          State = xTestUtils::fillMidNoise(PL->getAddr(), PL->getStride(), PL->getWidth(), PL->getHeight(), PL->getBitDepth(), 0, OrgState);
          State = xTestUtils::fillMidNoise(PU->getAddr(), PU->getStride(), PU->getWidth(), PU->getHeight(), PU->getBitDepth(), 1, OrgState);
          SD  = FunSD (PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SD  == -Area);
          SD  = FunSD (PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SD  ==  Area);
          SAD = FunSAD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SAD ==  Area);
          SAD = FunSAD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SAD ==  Area);
          SSD = FunSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SSD ==  Area);
          SSD = FunSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SSD ==  Area);
          std::tie(SD, SSD) = FunSSS(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SD  == -Area); CHECK(SSD == Area);
          std::tie(SD, SSD) = FunSSS(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SD  ==  Area); CHECK(SSD == Area);

          //gradient values
          CAPTURE(Description + " gradient values");
          xTestUtils::fillGradientXY(PL->getAddr(), PL->getStride(), PL->getWidth(), PL->getHeight(), PL->getBitDepth(), 0);
          xTestUtils::fillGradientXY(PU->getAddr(), PU->getStride(), PU->getWidth(), PU->getHeight(), PU->getBitDepth(), 1);
          SD  = FunSD (PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SD  == -Area);
          SD  = FunSD (PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SD  ==  Area);
          SAD = FunSAD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SAD ==  Area);
          SAD = FunSAD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SAD ==  Area);
          SSD = FunSSD(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SSD ==  Area);
          SSD = FunSSD(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SSD ==  Area);
          std::tie(SD, SSD) = FunSSS(PL->getAddr(), PU->getAddr(), PL->getStride(), PU->getStride(), PL->getWidth(), PL->getHeight(), b); CHECK(SD  == -Area); CHECK(SSD == Area);
          std::tie(SD, SSD) = FunSSS(PU->getAddr(), PL->getAddr(), PU->getStride(), PL->getStride(), PU->getWidth(), PU->getHeight(), b); CHECK(SD  ==  Area); CHECK(SSD == Area);

          //buffers destroy
          delete PL;
          delete PC;
          delete PU;
        }
      }
    }
  }    
}


//===============================================================================================================================================================================================================

TEST_CASE("xDistortionSTD")
{
  testDistortionSD (&xDistortionSTD::CalcSD16 , 16);
  testDistortionSAD(&xDistortionSTD::CalcSAD16, 16);
  testDistortionSSD(&xDistortionSTD::CalcSSD16, 16);
  testDistortionSSS(&xDistortionSTD::CalcSSS16, 16);
}

#if X_SIMD_CAN_USE_SSE
TEST_CASE("xDistortionSSE")
{
  testDistortionSD (&xDistortionSSE::CalcSD14 , 14);
  testDistortionSD (&xDistortionSSE::CalcSD16 , 16);
  testDistortionSAD(&xDistortionSSE::CalcSAD16, 16);
  testDistortionSSD(&xDistortionSSE::CalcSSD14, 14);
  testDistortionSSS(&xDistortionSSE::CalcSSS14, 14);
}
#endif //X_SIMD_CAN_USE_SSE

#if X_SIMD_CAN_USE_AVX
TEST_CASE("xDistortionAVX")
{
  testDistortionSD (&xDistortionAVX::CalcSD14 , 14);
  testDistortionSD (&xDistortionAVX::CalcSD16 , 16);
  testDistortionSAD(&xDistortionAVX::CalcSAD16, 16);
  testDistortionSSD(&xDistortionAVX::CalcSSD14, 14);
  testDistortionSSS(&xDistortionAVX::CalcSSS14, 14);
}
#endif //X_SIMD_CAN_USE_AVX

#if X_SIMD_CAN_USE_AVX512
TEST_CASE("xDistortionAVX512")
{
  testDistortionSD (&xDistortionAVX512::CalcSD14 , 14);
  testDistortionSD (&xDistortionAVX512::CalcSD16 , 16);
  testDistortionSAD(&xDistortionAVX512::CalcSAD16, 16);
  testDistortionSSD(&xDistortionAVX512::CalcSSD14, 14);
  testDistortionSSS(&xDistortionAVX512::CalcSSS14, 14);
}
#endif //X_SIMD_CAN_USE_AVX512

#if X_SIMD_CAN_USE_NEON
TEST_CASE("xDistortionNEON")
{
  testDistortionSD (&xDistortionNEON::CalcSD14 , 14);
  testDistortionSD (&xDistortionNEON::CalcSD16 , 16);
  testDistortionSAD(&xDistortionNEON::CalcSAD16, 16);
  testDistortionSSD(&xDistortionNEON::CalcSSD14, 14);
  testDistortionSSD(&xDistortionNEON::CalcSSD16, 14);
  testDistortionSSS(&xDistortionNEON::CalcSSS14, 14);
}
#endif //X_SIMD_CAN_USE_NEON

//===============================================================================================================================================================================================================

