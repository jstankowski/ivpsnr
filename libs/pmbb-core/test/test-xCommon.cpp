/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "xCommonDefCORE.h"
#include "xHelpersFLT.h"
#include <cfenv>

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

uint32 xRefFastLog2(uint32 Value)
{
  for(uint32 i = 31; i >= 0; i--)
  {
    if(Value & (1 << i)) { return i; }
  }
  return std::numeric_limits<uint32>::max(); // Undefined
}

void testFastLog2()
{
  for(uint32 i = 1; i < 65536; i++)
  {
    uint32 Ref = xRefFastLog2(i);
    uint32 Tst = xFastLog2   (i);
    CHECK(Ref == Tst);
  }

  for(uint32 p = 0; p < 32; p++)
  {
    uint32 i = 1 << p;
    uint32 Ref = xRefFastLog2(i);
    uint32 Tst = xFastLog2   (i);
    CHECK(Ref == Tst);
  }
}

//===============================================================================================================================================================================================================

void testRounding(bool Verbose)
{
#if X_PMBB_ARCH_AMD64
  uint32 RoundingModeSSE = _MM_GET_ROUNDING_MODE();
  if(Verbose)
  {
    switch(RoundingModeSSE)
    {
      case _MM_ROUND_NEAREST    : fmt::print("RoundingMode = _MM_ROUND_NEAREST    \n"); break;
      case _MM_ROUND_DOWN       : fmt::print("RoundingMode = _MM_ROUND_DOWN       \n"); break;
      case _MM_ROUND_UP         : fmt::print("RoundingMode = _MM_ROUND_UP         \n"); break;
      case _MM_ROUND_TOWARD_ZERO: fmt::print("RoundingMode = _MM_ROUND_TOWARD_ZERO\n"); break;
    }
  }
  CHECK(RoundingModeSSE == _MM_ROUND_NEAREST);
#endif

  int32 RoundingModeSTD = std::fegetround();
  if(Verbose)
  {
    switch(RoundingModeSTD)
    {
      case FE_TONEAREST : fmt::print("RoundingMode = FE_TONEAREST \n"); break;
      case FE_UPWARD    : fmt::print("RoundingMode = FE_UPWARD    \n"); break;
      case FE_DOWNWARD  : fmt::print("RoundingMode = FE_DOWNWARD  \n"); break;
      case FE_TOWARDZERO: fmt::print("RoundingMode = FE_TOWARDZERO\n"); break;
    }
  }
  CHECK(RoundingModeSTD == FE_TONEAREST);

  for(int32 i = -65536; i < 65536; i++)
  {
    flt32 F32 = (flt32)i / (flt32)1000;
    flt64 F64 = (flt64)i / (flt64)1000;

    int64 RefNF32I32 = (int64)nearbyint(F32);
    int64 RefNF64I64 = (int64)nearbyint(F64);
    int64 RefRF32I32 = (int64)round    (F32);
    int64 RefRF64I64 = (int64)round    (F64);

    int64 TstNF32I32 = (int64)xNearbyF32ToI32(F32);
    int64 TstNF64I32 = (int64)xNearbyF64ToI32(F64);
    int64 TstNF64I64 = (int64)xNearbyF64ToI64(F64);
    int64 TstRF32I32 = (int64)xRoundF32ToI32 (F32);
    int64 TstRF64I32 = (int64)xRoundF64ToI32 (F64);

    CHECK(TstNF32I32 == RefNF32I32);
    CHECK(TstNF64I32 == RefNF64I64);
    CHECK(TstNF64I64 == RefNF64I64);
    CHECK(TstRF32I32 == RefRF32I32);
    CHECK(TstRF64I32 == RefRF64I64);

    if(i >= 0)
    {
      int64 RefNF32U32 = xClipU((int64)nearbyint(F32), (int64)std::numeric_limits<uint32>::max());
      int64 RefNF64U64 = xClipU((int64)nearbyint(F64), (int64)std::numeric_limits<uint32>::max());
      int64 RefRF32U32 = xClipU((int64)round    (F32), (int64)std::numeric_limits<uint32>::max());
      int64 RefRF64U64 = xClipU((int64)round    (F64), (int64)std::numeric_limits<uint32>::max());

      int64 TstNF32U32 = (int64)xNearbyF32ToU32(F32);
      int64 TstNF64U32 = (int64)xNearbyF64ToU32(F64);
      int64 TstNF64U64 = (int64)xNearbyF64ToU64(F64);
      int64 TstRF32U32 = (int64)xRoundF32ToU32 (F32);
      int64 TstRF64U32 = (int64)xRoundF64ToU32 (F64);

      CHECK(TstNF32U32 == RefNF32U32);
      CHECK(TstNF64U32 == RefNF64U64);
      CHECK(TstNF64U64 == RefNF64U64);
      CHECK(TstRF32U32 == RefRF32U32);
      CHECK(TstRF64U32 == RefRF64U64);
    }
  }
}

//===============================================================================================================================================================================================================

TEST_CASE("testFastLog2")
{
  testFastLog2();
}

TEST_CASE("testRounding")
{
  testRounding(false);
}

//===============================================================================================================================================================================================================
