/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <functional>
#include <utility>
#include <array>
#include "xTestUtils.h"
#include "xMemory.h"
#include "xCommonDefCORE.h"
#include "xPic.h"
#include "xColorSpace.h"
#include "xColorSpaceCoeff.h"

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

static const std::vector<int32    > c_Dimms   = { 64, 63, 65, 128, 127, 129 };
static const std::vector<int32    > c_Margs   = { 0, 4, 32 };
static const std::vector<int32    > c_BitDs   = { 8, 14 };
static const std::vector<eClrSpcLC> c_ClrSpcs = { eClrSpcLC::BT601, eClrSpcLC::BT709, eClrSpcLC::SMPTE240M, eClrSpcLC::BT2020 };
static constexpr int32            c_DefBitDepth    = 14;
static constexpr int32            c_DefMaxValue    = (1<<c_DefBitDepth) - 1;
static constexpr int32            c_NumRandomTests = 8;

//===============================================================================================================================================================================================================

void testColorSpaceCoeff()
{
  static constexpr flt32 Tolerance = 0.00001f;

  auto RGB2YCbCr_BT601_F32 = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)eClrSpcLC::BT601];
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[0][0], (flt32) 0.29900, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[0][1], (flt32) 0.58700, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[0][2], (flt32) 0.11400, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[1][0], (flt32)-0.16874, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[1][1], (flt32)-0.33126, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[1][2], (flt32) 0.50000, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[2][0], (flt32) 0.50000, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[2][1], (flt32)-0.41869, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[2][2], (flt32)-0.08131, Tolerance));

  auto RGB2YCbCr_BT709_F32 = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)eClrSpcLC::BT709];
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[0][0], (flt32) 0.21260, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[0][1], (flt32) 0.71520, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[0][2], (flt32) 0.07220, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[1][0], (flt32)-0.11457, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[1][1], (flt32)-0.38543, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[1][2], (flt32) 0.50000, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[2][0], (flt32) 0.50000, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[2][1], (flt32)-0.45415, Tolerance));
  CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[2][2], (flt32)-0.04585, Tolerance));

  int32 Precision           = xColorSpaceCoeff<int32>::c_Precision;
  flt64 Mul                 = xColorSpaceCoeff<int32>::c_Mul;
  //flt64 Rnd               = xColorSpaceCoeff<int32>::c_Rnd;
  CHECK(xColorSpaceCoeff<int32>::c_Mul == (1 << xColorSpaceCoeff<int32>::c_Precision));
  CHECK(Mul == (1 << Precision));

  for(const auto c : c_ClrSpcs)
  {
    auto RGB2YCbCr_F32 = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)c];
    auto RGB2YCbCr_I32 = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)c];
    for(int32 i = 0; i < 3; i++)
    {
      for(int32 j = 0; j < 3; j++)
      {
        CHECK(RGB2YCbCr_I32[i][j] == (int32)round((1 << Precision) * RGB2YCbCr_F32[i][j]));
      }
    }
  }
}

//===============================================================================================================================================================================================================

void testColorSpace(
  std::function<void(uint16*, uint16*, uint16*, const uint16*, const uint16*, const uint16*, int32, int32, int32, int32, int32, eClrSpcLC)> ConvertRGB2YCbCr,
  std::function<void(uint16*, uint16*, uint16*, const uint16*, const uint16*, const uint16*, int32, int32, int32, int32, int32, eClrSpcLC)> ConvertYCbCr2RGB
)
{
  //uint32 State = xTestUtils::c_XorShiftSeed;

  for(const int32 y : c_Dimms)
  {
    for(const int32 x : c_Dimms)
    {
      int32V2 Size = { x, y };

      for(const int32 m : c_Margs)
      {
        const std::string Description = fmt::format("SizeXxY={}x{} Margin={}", x, y, m);

        //buffers create
        xPicP* Src = new xPicP(Size, c_DefBitDepth, m);
        xPicP* Imm = new xPicP(Size, c_DefBitDepth, m);
        xPicP* Dst = new xPicP(Size, c_DefBitDepth, m);

        Src->fill(0);
        Imm->fill(0);
        Dst->fill(0);

        //simple deterministic test
        for(const eClrSpcLC cs : c_ClrSpcs)
        {
          CAPTURE(Description + fmt::format(" SimpleDeterministic"));
          Src->fill(0);
          xTestUtils::fillGradient1X(Src->getAddr(eCmp::C0), Src->getStride(), Src->getWidth(), Src->getHeight(), c_DefBitDepth, 0  );
          xTestUtils::fillGradient1X(Src->getAddr(eCmp::C1), Src->getStride(), Src->getWidth(), Src->getHeight(), c_DefBitDepth, 100);
          xTestUtils::fillGradient1X(Src->getAddr(eCmp::C2), Src->getStride(), Src->getWidth(), Src->getHeight(), c_DefBitDepth, 200);
          Imm->fill(0);
          Dst->fill(0);
          ConvertRGB2YCbCr(Imm->getAddr(eCmp::C0), Imm->getAddr(eCmp::C1), Imm->getAddr(eCmp::C2),
                           Src->getAddr(eCmp::C0), Src->getAddr(eCmp::C1), Src->getAddr(eCmp::C2),
                           Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight(), c_DefBitDepth, cs);
          ConvertYCbCr2RGB(Dst->getAddr(eCmp::C0), Dst->getAddr(eCmp::C1), Dst->getAddr(eCmp::C2),
                           Imm->getAddr(eCmp::C0), Imm->getAddr(eCmp::C1), Imm->getAddr(eCmp::C2),
                           Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight(), c_DefBitDepth, cs);
          for(int32 c = 0; c < 3; c++)
          {
            CHECK(xTestUtils::isSimilarBuffer(Src->getBuffer((eCmp)c), Dst->getBuffer((eCmp)c), Dst->getBuffNumPels(), (uint16)1, true));
          }
        }

        //random test
        {

        }

        delete Src;
        delete Imm;
        delete Dst;
      }
    }
  }
}

std::tuple<flt64, flt64> perfColorSpace(
  std::function<void(uint16*, uint16*, uint16*, const uint16*, const uint16*, const uint16*, int32, int32, int32, int32, int32, eClrSpcLC)> ConvertRGB2YCbCr,
  std::function<void(uint16*, uint16*, uint16*, const uint16*, const uint16*, const uint16*, int32, int32, int32, int32, int32, eClrSpcLC)> ConvertYCbCr2RGB
)
{
#ifdef NDEBUG
  constexpr int32 NumIters = 8;
  constexpr int32 NumPels = 1024 * 1024 * 64;
#else
  constexpr int32 NumIters = 4;
  constexpr int32 NumPels  = 1024;
  INFO("WARNING: performance evaluation results are useless when build in debug mode");
#endif  
  constexpr int64 BuffSize = NumPels * sizeof(uint16);

  xPicP* Src = new xPicP({ NumPels , 1 }, c_DefBitDepth, 0);
  xPicP* Imm = new xPicP({ NumPels , 1 }, c_DefBitDepth, 0);
  xPicP* Dst = new xPicP({ NumPels , 1 }, c_DefBitDepth, 0);
  xPicP* Ref = new xPicP({ NumPels , 1 }, c_DefBitDepth, 0);

  uint32 State = xTestUtils::c_XorShiftSeed;
  State = xTestUtils::fillRandom(Src->getAddr(eCmp::C0), Src->getStride(), Src->getWidth(), Src->getHeight(), c_DefBitDepth, State);
  State = xTestUtils::fillRandom(Src->getAddr(eCmp::C1), Src->getStride(), Src->getWidth(), Src->getHeight(), c_DefBitDepth, State);
  State = xTestUtils::fillRandom(Src->getAddr(eCmp::C2), Src->getStride(), Src->getWidth(), Src->getHeight(), c_DefBitDepth, State);

  //reference
  xColorSpaceSTD::ConvertRGB2YCbCr_I32(Imm->getAddr(eCmp::C0), Imm->getAddr(eCmp::C1), Imm->getAddr(eCmp::C2),
                                       Src->getAddr(eCmp::C0), Src->getAddr(eCmp::C1), Src->getAddr(eCmp::C2),
                                       Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight(), c_DefBitDepth, eClrSpcLC::BT709);
  xColorSpaceSTD::ConvertYCbCr2RGB_I32(Ref->getAddr(eCmp::C0), Ref->getAddr(eCmp::C1), Ref->getAddr(eCmp::C2),
                                       Imm->getAddr(eCmp::C0), Imm->getAddr(eCmp::C1), Imm->getAddr(eCmp::C2),
                                       Ref->getStride(), Imm->getStride(), Ref->getWidth(), Ref->getHeight(), c_DefBitDepth, eClrSpcLC::BT709);

  //warmup  
  ConvertRGB2YCbCr(Imm->getAddr(eCmp::C0), Imm->getAddr(eCmp::C1), Imm->getAddr(eCmp::C2),
                   Src->getAddr(eCmp::C0), Src->getAddr(eCmp::C1), Src->getAddr(eCmp::C2),
                   Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight(), c_DefBitDepth, eClrSpcLC::BT709);
  ConvertYCbCr2RGB(Dst->getAddr(eCmp::C0), Dst->getAddr(eCmp::C1), Dst->getAddr(eCmp::C2),
                   Imm->getAddr(eCmp::C0), Imm->getAddr(eCmp::C1), Imm->getAddr(eCmp::C2),
                   Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight(), c_DefBitDepth, eClrSpcLC::BT709);
  for(int32 c = 0; c < 3; c++)
  {
    CHECK(xTestUtils::isSimilarBuffer(Src->getBuffer((eCmp)c), Dst->getBuffer((eCmp)c), Dst->getBuffNumPels(), (uint16)8, true));
  }

  //measure
  tDuration RY = (tDuration)0;
  tDuration YR = (tDuration)0;

  for(int32 j = 0; j < NumIters; j++)
  {
    tTimePoint T0 = tClock::now();
    ConvertRGB2YCbCr(Imm->getAddr(eCmp::C0), Imm->getAddr(eCmp::C1), Imm->getAddr(eCmp::C2),
                     Src->getAddr(eCmp::C0), Src->getAddr(eCmp::C1), Src->getAddr(eCmp::C2),
                     Imm->getStride(), Src->getStride(), Imm->getWidth(), Imm->getHeight(), c_DefBitDepth, eClrSpcLC::BT709);
    tTimePoint T1 = tClock::now();
    ConvertYCbCr2RGB(Dst->getAddr(eCmp::C0), Dst->getAddr(eCmp::C1), Dst->getAddr(eCmp::C2),
                     Imm->getAddr(eCmp::C0), Imm->getAddr(eCmp::C1), Imm->getAddr(eCmp::C2),
                     Dst->getStride(), Imm->getStride(), Dst->getWidth(), Dst->getHeight(), c_DefBitDepth, eClrSpcLC::BT709);
    tTimePoint T2 = tClock::now();
    for(int32 c = 0; c < 3; c++)
    {
      CHECK(xTestUtils::isSimilarBuffer(Src->getBuffer((eCmp)c), Dst->getBuffer((eCmp)c), Dst->getBuffNumPels(), (uint16)8, true));
    }
    RY += T1 - T0;
    YR += T2 - T1;
  }

  int64 NumBytes      = BuffSize * NumIters * 3;
  flt64 BytesPerSecRY = NumBytes / std::chrono::duration_cast<tDurationS>(RY).count();
  flt64 BytesPerSecYR = NumBytes / std::chrono::duration_cast<tDurationS>(YR).count();

  delete Src;
  delete Imm;
  delete Dst;
  delete Ref;

  return { BytesPerSecRY, BytesPerSecYR };
}

//===============================================================================================================================================================================================================

TEST_CASE("ColorSpaceCoeff")
{
  testColorSpaceCoeff();
}

TEST_CASE("xColorSpaceSTD-F32")
{
  testColorSpace(xColorSpaceSTD::ConvertRGB2YCbCr_F32, xColorSpaceSTD::ConvertYCbCr2RGB_F32);
}

TEST_CASE("xColorSpaceSTD-I32")
{
  testColorSpace(xColorSpaceSTD::ConvertRGB2YCbCr_I32, xColorSpaceSTD::ConvertYCbCr2RGB_I32);
}

#if X_SIMD_CAN_USE_SSE
TEST_CASE("xColorSpaceSSE-I32")
{
  testColorSpace(xColorSpaceSSE::ConvertRGB2YCbCr_I32, xColorSpaceSSE::ConvertYCbCr2RGB_I32);
}
#endif //X_SIMD_CAN_USE_SSE

#if X_SIMD_CAN_USE_AVX
TEST_CASE("xColorSpaceAVX-I32")
{
  testColorSpace(xColorSpaceAVX::ConvertRGB2YCbCr_I32, xColorSpaceAVX::ConvertYCbCr2RGB_I32);
}
#endif //X_SIMD_CAN_USE_AVX

#if X_SIMD_CAN_USE_AVX512
TEST_CASE("xColorSpaceAVX512-I32")
{
  testColorSpace(xColorSpaceAVX512::ConvertRGB2YCbCr_I32, xColorSpaceAVX512::ConvertYCbCr2RGB_I32);
}
#endif //X_SIMD_CAN_USE_AVX512

TEST_CASE("xColorSpaceSTD-F32-perf")
{
  auto [RY, YR] = perfColorSpace(xColorSpaceSTD::ConvertRGB2YCbCr_F32, xColorSpaceSTD::ConvertYCbCr2RGB_F32);
  fmt::print("TIME(xColorSpaceSTD::ConvertRGB2YCbCr_F32) = {:.2f} MiB/s\n", RY / (1024 * 1024));
  fmt::print("TIME(xColorSpaceSTD::ConvertYCbCr2RGB_F32) = {:.2f} MiB/s\n", YR / (1024 * 1024));
}

TEST_CASE("xColorSpaceSTD-I32-perf")
{
  auto [RY, YR] = perfColorSpace(xColorSpaceSTD::ConvertRGB2YCbCr_I32, xColorSpaceSTD::ConvertYCbCr2RGB_I32);
  fmt::print("TIME(xColorSpaceSTD::ConvertRGB2YCbCr_I32) = {:.2f} MiB/s\n", RY / (1024 * 1024));
  fmt::print("TIME(xColorSpaceSTD::ConvertYCbCr2RGB_I32) = {:.2f} MiB/s\n", YR / (1024 * 1024));
}

#if X_SIMD_CAN_USE_SSE
TEST_CASE("xColorSpaceSSE-I32-perf")
{
  auto [RY, YR] = perfColorSpace(xColorSpaceSSE::ConvertRGB2YCbCr_I32, xColorSpaceSSE::ConvertYCbCr2RGB_I32);
  fmt::print("TIME(xColorSpaceSSE::ConvertRGB2YCbCr_I32) = {:.2f} MiB/s\n", RY / (1024 * 1024));
  fmt::print("TIME(xColorSpaceSSE::ConvertYCbCr2RGB_I32) = {:.2f} MiB/s\n", YR / (1024 * 1024));
}
#endif //X_SIMD_CAN_USE_SSE

#if X_SIMD_CAN_USE_AVX
TEST_CASE("xColorSpaceAVX-I32-perf")
{
  auto [RY, YR] = perfColorSpace(xColorSpaceAVX::ConvertRGB2YCbCr_I32, xColorSpaceAVX::ConvertYCbCr2RGB_I32);
  fmt::print("TIME(xColorSpaceAVX::ConvertRGB2YCbCr_I32) = {:.2f} MiB/s\n", RY / (1024 * 1024));
  fmt::print("TIME(xColorSpaceAVX::ConvertYCbCr2RGB_I32) = {:.2f} MiB/s\n", YR / (1024 * 1024));
}
#endif //X_SIMD_CAN_USE_AVX

#if X_SIMD_CAN_USE_AVX512
TEST_CASE("xColorSpaceAVX512-I32-perf")
{
  auto [RY, YR] = perfColorSpace(xColorSpaceAVX512::ConvertRGB2YCbCr_I32, xColorSpaceAVX512::ConvertYCbCr2RGB_I32);
  fmt::print("TIME(xColorSpaceAVX512::ConvertRGB2YCbCr_I32) = {:.2f} MiB/s\n", RY / (1024 * 1024));
  fmt::print("TIME(xColorSpaceAVX512::ConvertYCbCr2RGB_I32) = {:.2f} MiB/s\n", YR / (1024 * 1024));
}
#endif //X_SIMD_CAN_USE_AVX512

//===============================================================================================================================================================================================================
