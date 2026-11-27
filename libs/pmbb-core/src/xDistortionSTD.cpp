/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xDistortionSTD.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

int64 xDistortionSTD::CalcSD16(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  int64 SD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SD += (int32)Tst[x] - (int32)Ref[x]; }
    Tst += TstStride;
    Ref += RefStride;
  }
  return SD;
}
uint64 xDistortionSTD::CalcSAD16(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  uint64 SAD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SAD += (uint32)xAbs(((int32)Tst[x]) - ((int32)Ref[x])); }
    Tst += TstStride;
    Ref += RefStride;
  }
  return SAD;
}
uint64 xDistortionSTD::CalcSSD16(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  uint64 SSD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SSD += (uint64)xPow2(((int64)Tst[x]) - ((int64)Ref[x])); }
    Tst += TstStride;
    Ref += RefStride;
  }
  return SSD;
}
tSDSSD xDistortionSTD::CalcSSS16(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  int64 SD  = 0;
  int64 SSD = 0;
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      int32 Diff = (int32)Tst[x] - (int32)Ref[x];
      SD  += Diff;
      SSD += xPow2<int64>(Diff);
    }
    Tst += TstStride;
    Ref += RefStride;
  }
  return{ SD, SSD };
}
int64 xDistortionSTD::CalcMaskSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Msk, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height)
{
  int64 SD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { if(Msk[x]){ SD += (int32)Tst[x] - (int32)Ref[x]; }}
    Tst += TstStride;
    Ref += RefStride;
    Msk += MskStride;
  }
  return SD;
}
uint64 xDistortionSTD::CalcMaskSSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Msk, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height)
{
  uint64 SSD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { if(Msk[x]){ SSD += (uint64)xPow2<int64>((int32)Tst[x] - (int32)Ref[x]); } }
    Tst += TstStride;
    Ref += RefStride;
    Msk += MskStride;
  }
  return SSD;
}
tSDSSD xDistortionSTD::CalcMaskSSS(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Msk, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height)
{
  int64 SD  = 0;
  int64 SSD = 0;
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      if(Msk[x])
      {
        int32 Diff = (int32)Tst[x] - (int32)Ref[x];
        SD  += Diff;
        SSD += xPow2<int64>(Diff);
      }
    }
    Tst += TstStride;
    Ref += RefStride;
    Msk += MskStride;
  }
  return{ SD, SSD };
}
int64 xDistortionSTD::CalcWghtSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Wgh, int32 TstStride, int32 RefStride, int32 WghStride, int32 Width, int32 Height)
{
  int64 SD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SD += ((int32)Tst[x] - (int32)Ref[x]) * (int32)Wgh[x]; }
    Tst += TstStride;
    Ref += RefStride;
    Wgh += WghStride;
  }
  return SD;
}
uint64 xDistortionSTD::CalcWghtSSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Wgh, int32 TstStride, int32 RefStride, int32 WghStride, int32 Width, int32 Height)
{
  uint64 SSD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SSD += ((uint64)xPow2<int64>((int32)Tst[x] - (int32)Ref[x])) * (uint64)Wgh[x]; }
    Tst += TstStride;
    Ref += RefStride;
    Wgh += WghStride;
  }
  return SSD;
}
tSDSSD xDistortionSTD::CalcWghtSSS(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Wgh, int32 TstStride, int32 RefStride, int32 WghStride, int32 Width, int32 Height)
{
  int64 SD  = 0;
  int64 SSD = 0;
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      int32 Diff = ((int32)Tst[x] - (int32)Ref[x]);
      SD  += Diff               * (int64)Wgh[x];
      SSD += xPow2<int64>(Diff) * (int64)Wgh[x];
    }
    Tst += TstStride;
    Ref += RefStride;
    Wgh += WghStride;
  }
  return{ SD, SSD };
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
