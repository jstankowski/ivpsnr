/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "../src/xCommonDefPMBB-CORE.h"
#include "../src/xVec.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xTestUtils
{
protected:
  static constexpr uint32 c_XorShiftSeed = 666; //very god seed

  static inline uint32 xXorShift32(uint32 x) // Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs"
  {    
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
  }

public:
  static void fillGradient1X(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset);
  static void fillGradient4X(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset);
  static void fillGradient1Y(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset);
  static void fillGradient4Y(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset);
  static void fillGradientXY(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset);
  static void fillRandom    (uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth,               uint32 Seed = c_XorShiftSeed);
  static void fillMidNoise  (uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset, uint32 Seed = c_XorShiftSeed);

  template<typename XXX> static bool isEqualValue(XXX* Src, int32 SrcStride, int32 Width, int32 Height, XXX Value, bool Verbose = false);

  template<typename XXX> static bool isSameBuffer(XXX* Ref,                  XXX* Cmp,                  int32 Area,               bool Verbose = false);
  template<typename XXX> static bool isSameBuffer(XXX* Ref, int32 RefStride, XXX* Cmp, int32 CmpStride, int32 Width, int32 Height, bool Verbose = false);

  template<typename XXX> static int64 calcSum(XXX* Src, int32 SrcStride, int32 Width, int32 Height);
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename XXX> bool xTestUtils::isEqualValue(XXX* Src, int32 SrcStride, int32 Width, int32 Height, XXX Value, bool Verbose)
{
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      if(Src[x] != Value)
      {
        if(Verbose) { fmt::print("xTestUtils::isEqualValue --> discrepancy found at y={} x={} Src={} expected Val={}\n", y, x, Src[x], Value); std::fflush(stdout); } return false;
      }
    }
    Src += SrcStride;
  }
  return true;
}
template<typename XXX> bool xTestUtils::isSameBuffer(XXX* Ref, XXX* Cmp, int32 Area, bool Verbose)
{
  for(int32 i = 0; i < Area; i++)
  {
    if(Ref[i] != Cmp[i])
    {
      if(Verbose) { fmt::print("xTestUtils::isSameBuffer --> discrepancy found at i={} Ref={} Cmp={}\n", i, Ref[i], Cmp[i]); std::fflush(stdout); } return false;
    }
  }
  return true;
}
template<typename XXX> bool xTestUtils::isSameBuffer(XXX* Ref, int32 RefStride, XXX* Cmp, int32 CmpStride, int32 Width, int32 Height, bool Verbose)
{
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      if(Ref[x] != Cmp[x])
      {
        if(Verbose) { fmt::print("xTestUtils::isSameBuffer --> discrepancy found at y={} x={} Ref={} Cmp={}\n", y, x, Ref[x], Cmp[x]); std::fflush(stdout); } return false;
      }
    }
    Ref += RefStride;
    Cmp += CmpStride;
  }
  return true;
}
template<typename XXX> int64 xTestUtils::calcSum(XXX* Src, int32 SrcStride, int32 Width, int32 Height)
{
  int64 Sum = 0;
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++) { Sum += Src[x]; }
    Src += SrcStride;
  }
  return Sum;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
