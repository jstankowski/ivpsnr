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
#include "xTestUtilsIVQM.h"
#include "xPic.h"
#include "xGlobClrDiff.h"

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

static const std::vector<int32> c_Dimms = { 128, 256 };
static const std::vector<int32> c_BitDs = { 8, 10, 12, 14 };
constexpr int32 c_Margin = 0;

//===============================================================================================================================================================================================================

void testGlobClrDiff()
{
  uint32 State = xTestUtils::c_XorShiftSeed;

  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };

      for(const int32 b : c_BitDs)
      {
        xPicP* Ref = new xPicP(Size, b, c_Margin);
        xPicP* Mod = new xPicP(Size, b, c_Margin);

        //simple deterministic tests
        int32V4 GCD;
        Ref->fill(0);
        //zero
        Mod->fill(0);
        GCD = xGlobClrDiff::CalcGlobalColorDiff(Mod, Ref, { 1.0f, 1.0f, 1.0f, 1.0f }, nullptr);
        CHECK(GCD == int32V4(0, 0, 0, 0));
        GCD = xGlobClrDiff::CalcGlobalColorDiff(Ref, Mod, { 1.0f, 1.0f, 1.0f, 1.0f }, nullptr);
        CHECK(GCD == int32V4(0, 0, 0, 0));
        //one
        Mod->fill(1);
        GCD = xGlobClrDiff::CalcGlobalColorDiff(Mod, Ref, { 1.0f, 1.0f, 1.0f, 1.0f }, nullptr);
        CHECK(GCD == int32V4(1, 1, 1, 0));
        GCD = xGlobClrDiff::CalcGlobalColorDiff(Ref, Mod, { 1.0f, 1.0f, 1.0f, 1.0f }, nullptr);
        CHECK(GCD == int32V4(-1, -1, -1, 0));
        //max
        int32 Max = xBitDepth2MaxValue(b);
        Mod->fill((uint16)Max);
        GCD = xGlobClrDiff::CalcGlobalColorDiff(Mod, Ref, { 1.0f, 1.0f, 1.0f, 1.0f }, nullptr);
        CHECK(GCD == int32V4(Max, Max, Max, 0));
        GCD = xGlobClrDiff::CalcGlobalColorDiff(Ref, Mod, { 1.0f, 1.0f, 1.0f, 1.0f }, nullptr);
        CHECK(GCD == int32V4(-Max, -Max, -Max, 0));

        //random test        
        xPerlinNoise::fillPerlinNoiseI(Ref->getAddr(eCmp::C0), Ref->getStride(), Ref->getWidth(), Ref->getHeight(), Ref->getBitDepth(), 8, State); State = xTestUtils::xXorShift32(State);
        xPerlinNoise::fillPerlinNoiseI(Ref->getAddr(eCmp::C1), Ref->getStride(), Ref->getWidth(), Ref->getHeight(), Ref->getBitDepth(), 8, State); State = xTestUtils::xXorShift32(State);
        xPerlinNoise::fillPerlinNoiseI(Ref->getAddr(eCmp::C2), Ref->getStride(), Ref->getWidth(), Ref->getHeight(), Ref->getBitDepth(), 8, State); State = xTestUtils::xXorShift32(State);

        //zero
        Mod->copy(Ref);
        GCD = xGlobClrDiff::CalcGlobalColorDiff(Mod, Ref, { 1.0f, 1.0f, 1.0f, 1.0f }, nullptr);
        CHECK(GCD == int32V4(0, 0, 0, 0));
        GCD = xGlobClrDiff::CalcGlobalColorDiff(Ref, Mod, { 1.0f, 1.0f, 1.0f, 1.0f }, nullptr);
        CHECK(GCD == int32V4(0, 0, 0, 0));

        //const
        xTestUtilsIVQM::addConst(Mod->getAddr(eCmp::C0), Ref->getAddr(eCmp::C0), Mod->getStride(), Ref->getStride(), Ref->getWidth(), Ref->getHeight(), 4);
        xTestUtilsIVQM::addConst(Mod->getAddr(eCmp::C1), Ref->getAddr(eCmp::C1), Mod->getStride(), Ref->getStride(), Ref->getWidth(), Ref->getHeight(), 3);
        xTestUtilsIVQM::addConst(Mod->getAddr(eCmp::C2), Ref->getAddr(eCmp::C2), Mod->getStride(), Ref->getStride(), Ref->getWidth(), Ref->getHeight(), 2);
        GCD = xGlobClrDiff::CalcGlobalColorDiff(Mod, Ref, { 1.0f, 1.0f, 1.0f, 1.0f }, nullptr);
        CHECK(GCD == int32V4(4, 3, 2, 0));
        GCD = xGlobClrDiff::CalcGlobalColorDiff(Ref, Mod, { 1.0f, 1.0f, 1.0f, 1.0f }, nullptr);
        CHECK(GCD == int32V4(-4, -3, -2, 0));

        Ref->destroy(); Ref = nullptr;
        Mod->destroy(); Mod = nullptr;
      }
    }
  }
}

//===============================================================================================================================================================================================================

TEST_CASE("GlobClrDiff")
{
  testGlobClrDiff();
}
