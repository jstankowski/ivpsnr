/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2025 Patrycja Kaźmierczak <patrycja.kazmierczak@student.put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <functional>
#include <utility>
#include <array>
#include "xTestUtils.h"
#include "xTimeUtils.h"
#include "xMemory.h"
#include "xCommonDefCORE.h"
#include "xPic.h"
#include "xColorSpace.h"
#include "xColorSpaceCoeff.h"
#include "xHelpersFLT.h"

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

static const std::vector<int32    > c_Dimms   = { 64, 63, 65, 128, 127, 129 };
static const std::vector<int32    > c_Margs   = { 0, 4, 32 };
static const std::vector<int32    > c_BitDs   = { 8, 14 };
static const std::vector<eClrSpcLC> c_ClrSpcs = { eClrSpcLC::BT601, eClrSpcLC::BT709, eClrSpcLC::SMPTE240M, eClrSpcLC::BT2020 };
static constexpr int32              c_DefBitDepth    = 14;
static constexpr int32              c_DefMaxValue    = (1<<c_DefBitDepth) - 1;
static constexpr int32              c_NumRandomTests = 8;

//===============================================================================================================================================================================================================

void testColorSpaceCoeff()
{
  {
    static constexpr flt32 Tolerance = 0.0001f;

    auto RGB2YCbCr_BT601_F32 = xColorSpaceCoeffYCbCr::c_RGB2YCbCr_F32[(int32)eClrSpcLC::BT601];
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[0][0], (flt32) 0.29900, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[0][1], (flt32) 0.58700, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[0][2], (flt32) 0.11400, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[1][0], (flt32)-0.16874, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[1][1], (flt32)-0.33126, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[1][2], (flt32) 0.50000, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[2][0], (flt32) 0.50000, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[2][1], (flt32)-0.41869, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT601_F32[2][2], (flt32)-0.08131, Tolerance));

    auto RGB2YCbCr_BT709_F32 = xColorSpaceCoeffYCbCr::c_RGB2YCbCr_F32[(int32)eClrSpcLC::BT709];
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[0][0], (flt32) 0.21260, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[0][1], (flt32) 0.71520, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[0][2], (flt32) 0.07220, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[1][0], (flt32)-0.11457, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[1][1], (flt32)-0.38543, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[1][2], (flt32) 0.50000, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[2][0], (flt32) 0.50000, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[2][1], (flt32)-0.45415, Tolerance));
    CHECK(xIsApproximatelyEqual(RGB2YCbCr_BT709_F32[2][2], (flt32)-0.04585, Tolerance));
  }

  int32 Precision           = xColorSpaceCoeffYCbCr::c_Precision;
  flt64 Mul                 = xColorSpaceCoeffYCbCr::c_Mul;
  //flt64 Rnd               = xColorSpaceCoeff<int32>::c_Rnd;
  CHECK(xColorSpaceCoeffYCbCr::c_Mul == (1 << xColorSpaceCoeffYCbCr::c_Precision));
  CHECK(Mul == (1 << Precision));
  
  for(const auto c : c_ClrSpcs)
  {
    auto RGB2YCbCr_F32 = xColorSpaceCoeffYCbCr::c_RGB2YCbCr_F32[(int32)c];
    auto RGB2YCbCr_I32 = xColorSpaceCoeffYCbCr::c_RGB2YCbCr_I32[(int32)c];

    flt64 SumLm_F32 = (flt64)RGB2YCbCr_F32[0][0] + (flt64)RGB2YCbCr_F32[0][1] + (flt64)RGB2YCbCr_F32[0][2];
    flt64 SumCb_F32 = (flt64)RGB2YCbCr_F32[1][0] + (flt64)RGB2YCbCr_F32[1][1] + (flt64)RGB2YCbCr_F32[1][2];
    flt64 SumCr_F32 = (flt64)RGB2YCbCr_F32[2][0] + (flt64)RGB2YCbCr_F32[2][1] + (flt64)RGB2YCbCr_F32[2][2];

    static constexpr flt64 Tolerance = 0.0000001f;
    CHECK(xIsApproximatelyEqual(SumLm_F32, 1.0, Tolerance));
    CHECK(xIsApproximatelyEqual(SumCb_F32, 0.0, Tolerance));
    CHECK(xIsApproximatelyEqual(SumCr_F32, 0.0, Tolerance));

    int32 SumLm_I32 = RGB2YCbCr_I32[0][0] + RGB2YCbCr_I32[0][1] + RGB2YCbCr_I32[0][2];
    int32 SumCb_I32 = RGB2YCbCr_I32[1][0] + RGB2YCbCr_I32[1][1] + RGB2YCbCr_I32[1][2];
    int32 SumCr_I32 = RGB2YCbCr_I32[2][0] + RGB2YCbCr_I32[2][1] + RGB2YCbCr_I32[2][2];

    CHECK(SumLm_I32 == xColorSpaceCoeffYCbCr::c_Mul);
    CHECK(SumCb_I32 == 0                           );
    CHECK(SumCr_I32 == 0                           );
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

#if X_SIMD_CAN_USE_NEON
TEST_CASE("xColorSpaceNEON-I32")
{
  testColorSpace(xColorSpaceNEON::ConvertRGB2YCbCr_I32, xColorSpaceNEON::ConvertYCbCr2RGB_I32);
}
#endif //X_SIMD_CAN_USE_NEON

//===============================================================================================================================================================================================================
