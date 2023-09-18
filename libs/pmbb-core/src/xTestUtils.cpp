/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xTestUtils.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xTestUtils::fillGradient1X(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset)
{
  const int32 MaxValueMask = xBitDepth2BitMask(BitDepth);
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++) { Dst[x] = (uint16)((x + Offset) & MaxValueMask); }
    Dst += DstStride;
  }
}
void xTestUtils::fillGradient4X(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset)
{
  const int32 MaxValueMask = xBitDepth2BitMask(BitDepth);
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++) { Dst[x] = (uint16)(((x<<2) + Offset) & MaxValueMask); }
    Dst += DstStride;
  }
}
void xTestUtils::fillGradient1Y(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset)
{
  const int32 MaxValueMask = xBitDepth2BitMask(BitDepth);
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++) { Dst[x] = (uint16)((y + Offset) & MaxValueMask); }
    Dst += DstStride;
  }
}
void xTestUtils::fillGradient4Y(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset)
{
  const int32 MaxValueMask = xBitDepth2BitMask(BitDepth);
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++) { Dst[x] = (uint16)(((y<<2) + Offset) & MaxValueMask); }
    Dst += DstStride;
  }
}
void xTestUtils::fillGradientXY(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset)
{
  const int32 MaxValueMask = xBitDepth2BitMask(BitDepth);
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++) { Dst[x] = (uint16)((x+y+Offset) & MaxValueMask); }
    Dst += DstStride;
  }
}
void xTestUtils::fillRandom(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, uint32 Seed)
{
  const uint32 MaxValueMask = (uint32)xBitDepth2BitMask(BitDepth);
  uint32 State = Seed;
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++) 
    { 
      State = xXorShift32(State);
      Dst[x] = (uint16)(State & MaxValueMask);
    }
    Dst += DstStride;
  }
}
void xTestUtils::fillMidNoise(uint16* Dst, int32 DstStride, int32 Width, int32 Height, int32 BitDepth, int32 Offset, uint32 Seed)
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
      Dst[x] = (uint16)((MidValue + Noise + Offset) & MaxValueMask);
    }
    Dst += DstStride;
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
