/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xDistortionSTD.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

int32 xDistortionSTD::CalcSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{
  int32 SD = 0;
  for(int32 i=0; i < Area; i++) { SD += (int32)Tst[i] - (int32)Ref[i]; }
  return SD;
}
int32 xDistortionSTD::CalcSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  int32 SD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SD += (int32)Tst[x] - (int32)Ref[x]; }
    Tst += TstStride;
    Ref += RefStride;
  }
  return SD;
}
uint32 xDistortionSTD::CalcSAD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{
  uint32 SAD = 0;
  for(int32 i=0; i < Area; i++) { SAD += (uint32)xAbs(((int32)Tst[i]) - ((int32)Ref[i])); }
  return SAD;
}
uint32 xDistortionSTD::CalcSAD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  uint32 SAD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SAD += (uint32)xAbs(((int32)Tst[x]) - ((int32)Ref[x])); }
    Tst += TstStride;
    Ref += RefStride;
  }
  return SAD;
}
uint64 xDistortionSTD::CalcSSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 Area)
{
  uint64 SSD = 0;
  for(int32 i=0; i < Area; i++) { SSD += (uint64)xPow2(((int32)Tst[i]) - ((int32)Ref[i])); }
  return SSD;
}
uint64 xDistortionSTD::CalcSSD(const uint16* restrict Tst, const uint16* restrict Ref, int32 TstStride, int32 RefStride, int32 Width, int32 Height)
{
  uint64 SSD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SSD += (uint64)xPow2(((int32)Tst[x]) - ((int32)Ref[x])); }
    Tst += TstStride;
    Ref += RefStride;
  }
  return SSD;
}
int64 xDistortionSTD::CalcWeightedSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Msk, int32 Area)
{
  int64 SD = 0;
  for(int32 i=0; i < Area; i++) { SD += ((int32)Tst[i] - (int32)Ref[i]) * (int32)Msk[i]; }
  return SD;
}
int64 xDistortionSTD::CalcWeightedSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Msk, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height)
{
  int64 SD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SD += ((int32)Tst[x] - (int32)Ref[x]) * (int32)Msk[x]; }
    Tst += TstStride;
    Ref += RefStride;
    Msk += MskStride;
  }
  return SD;
}
uint64 xDistortionSTD::CalcWeightedSSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Msk, int32 Area)
{
  uint64 SSD = 0;
  for(int32 i=0; i < Area; i++) { SSD += ((uint64)xPow2(((int32)Tst[i]) - ((int32)Ref[i]))) * (uint64)Msk[i]; }
  return SSD;
}
uint64 xDistortionSTD::CalcWeightedSSD(const uint16* restrict Tst, const uint16* restrict Ref, const uint16* restrict Msk, int32 TstStride, int32 RefStride, int32 MskStride, int32 Width, int32 Height)
{
  uint64 SSD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SSD += ((uint64)xPow2(((int32)Tst[x]) - ((int32)Ref[x]))) * (uint64)Msk[x]; }
    Tst += TstStride;
    Ref += RefStride;
    Msk += MskStride;
  }
  return SSD;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
