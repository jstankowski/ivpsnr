/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "../src/xCommonDefCORE.h"
#include "../src/xVec.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xTestUtils
{
public:
  //pseudo-rng
  static inline uint32 xXorShift32(uint32 x) // Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs"
  {    
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x <<  5;
    return x;
  }
  static inline uint64 xXorShift64(uint64 x)
  {
    x ^= x << 13;
    x ^= x >>  7;
    x ^= x << 17;
    return x;
  }
  static constexpr uint32 c_XorShiftSeed = 666; //very god seed

  class xXorShiftGen32;
  class xXorShiftGen64;

  //utils
  static void   fillGradient1X(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset);
  static void   fillGradient4X(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset);
  static void   fillGradient1Y(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset);
  static void   fillGradient4Y(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset);
  static void   fillGradientXY(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset);
  template<typename XXX> static uint32 fillRandom    (XXX* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth,               uint32 Seed = c_XorShiftSeed);
  template<typename XXX> static uint32 fillMidNoise  (XXX* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset, uint32 Seed = c_XorShiftSeed);
  template<typename XXX> static uint32 fillRandom01  (XXX* Dst, int32 DstStride, int32 Width, int32 Height,                               uint32 Seed = c_XorShiftSeed);

  template<typename XXX> static bool isEqualValue(const XXX* Src, int32 SrcStride, int32 Width, int32 Height, XXX Value, bool Verbose = false);

  template<typename XXX> static bool isSameBuffer(const XXX* Ref,                  const XXX* Cmp,                  uint64 Area,                bool Verbose = false);
  template<typename XXX> static bool isSameBuffer(const XXX* Ref, int32 RefStride, const XXX* Cmp, int32 CmpStride, int32 Width, int32 Height, bool Verbose = false);

  template<typename XXX> static bool isSimilarBuffer(const XXX* Ref,                  const XXX* Cmp,                  uint64 Area,                XXX Threshold, bool Verbose = false);
  template<typename XXX> static bool isSimilarBuffer(const XXX* Ref, int32 RefStride, const XXX* Cmp, int32 CmpStride, int32 Width, int32 Height, XXX Threshold, bool Verbose = false);

  template<typename XXX> static int64 calcSum(const XXX* Src, int32 SrcStride, int32 Width, int32 Height);
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename XXX> uint32 xTestUtils::fillRandom(XXX* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, uint32 Seed)
{
  const uint32 MaxValueMask = (uint32)xBitDepth2BitMask(BitDepth);
  uint32 State = Seed;
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++) 
    { 
      State = xXorShift32(State);
      Dst[x] = (XXX)(State & MaxValueMask);
    }
    Dst += DstStride;
  }
  return State;
}
template<typename XXX> uint32 xTestUtils::fillMidNoise(XXX* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset, uint32 Seed)
{
  const int32 MidValue     = (uint32)xBitDepth2MidValue(BitDepth);
  const int32 MaxValueMask = (uint32)xBitDepth2BitMask(BitDepth);
  uint32 State = Seed;
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      State = xXorShift32(State);
      int32 Noise = ((((int32)State) & MaxValueMask) - MidValue) >> (BitDepth - 2);
      Dst[x] = (XXX)((MidValue + Noise + Offset) & MaxValueMask);
    }
    Dst += DstStride;
  }
  return State;
}
template<typename XXX> uint32 xTestUtils::fillRandom01(XXX* Dst, int32 DstStride, int32 Width, int32 Height, uint32 Seed)
{
  uint32 State = Seed;
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      State = xXorShift32(State);
      Dst[x] = (XXX)(State & 0x01);
    }
    Dst += DstStride;
  }
  return State;

}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template<typename XXX> bool xTestUtils::isEqualValue(const XXX* Src, int32 SrcStride, int32 Width, int32 Height, XXX Value, bool Verbose)
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
template<typename XXX> bool xTestUtils::isSameBuffer(const XXX* Ref, const XXX* Cmp, uint64 Area, bool Verbose)
{
  for(uint64 i = 0; i < Area; i++)
  {
    if(Ref[i] != Cmp[i])
    {
      if(Verbose) { fmt::print("xTestUtils::isSameBuffer --> discrepancy found at i={} Ref={} Cmp={}\n", i, Ref[i], Cmp[i]); std::fflush(stdout); } return false;
    }
  }
  return true;
}
template<typename XXX> bool xTestUtils::isSameBuffer(const XXX* Ref, int32 RefStride, const XXX* Cmp, int32 CmpStride, int32 Width, int32 Height, bool Verbose)
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
template<typename XXX> bool xTestUtils::isSimilarBuffer(const XXX* Ref, const XXX* Cmp, uint64 Area, XXX Threshold, bool Verbose)
{
  using IntermType = std::conditional_t<std::is_integral_v<XXX>, std::conditional_t<sizeof(XXX) <= 4, int32, int64>, XXX>;

  for(uint64 i = 0; i < Area; i++)
  {
    const IntermType AbsDiff = xAbs((IntermType)(Ref[i]) - (IntermType)(Cmp[i]));
    if(AbsDiff > Threshold)
    {
      if(Verbose) { fmt::print("xTestUtils::isSameBuffer --> discrepancy found at i={} Ref={} Cmp={}\n", i, Ref[i], Cmp[i]); std::fflush(stdout); } return false;
    }
  }
  return true;
}
template<typename XXX> bool xTestUtils::isSimilarBuffer(const XXX* Ref, int32 RefStride, const XXX* Cmp, int32 CmpStride, int32 Width, int32 Height, XXX Threshold, bool Verbose)
{
  using IntermType = std::conditional_t<std::is_integral_v<XXX>, std::conditional_t<sizeof(XXX) <= 4, int32, int64>, XXX>;

  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      const IntermType AbsDiff = xAbs((IntermType)(Ref[x]) - (IntermType)(Cmp[x]));
      if(AbsDiff > Threshold)
      {
        if(Verbose) { fmt::print("xTestUtils::isSameBuffer --> discrepancy found at y={} x={} Ref={} Cmp={}\n", y, x, Ref[x], Cmp[x]); std::fflush(stdout); } return false;
      }
    }
    Ref += RefStride;
    Cmp += CmpStride;
  }
  return true;
}
template<typename XXX> int64 xTestUtils::calcSum(const XXX* Src, int32 SrcStride, int32 Width, int32 Height)
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
// Xor Shift generator - compatible with C++ Random number engine interface
//===============================================================================================================================================================================================================
class xTestUtils::xXorShiftGen32
{
protected:
  uint32 m_State;

public:
  typedef uint32 result_type;

  xXorShiftGen32(uint32 Seed = xTestUtils::c_XorShiftSeed) { m_State = Seed; }

  uint32 operator()() { uint32 State = xTestUtils::xXorShift32(m_State); m_State = State; return State; }

  static constexpr uint32 min() { return std::numeric_limits<uint32>::min(); }
  static constexpr uint32 max() { return std::numeric_limits<uint32>::max(); }
};

class xTestUtils::xXorShiftGen64
{
protected:
  uint64 m_State;

public:
  typedef uint64 result_type;

  xXorShiftGen64(uint64 Seed = xTestUtils::c_XorShiftSeed) { m_State = Seed; }

  uint64 operator()() { uint64 State = xTestUtils::xXorShift64(m_State); m_State = State; return State; }

  static constexpr uint64 min() { return std::numeric_limits<uint64>::min(); }
  static constexpr uint64 max() { return std::numeric_limits<uint64>::max(); }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
