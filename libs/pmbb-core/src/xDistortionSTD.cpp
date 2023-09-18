/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xDistortionSTD.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

int32 xDistortionSTD::CalcSD(const uint16* restrict Org, const uint16* restrict Dist, int32 Area)
{
  int32 SD = 0;
  for(int32 i=0; i < Area; i++) { SD += (int32)Org[i] - (int32)Dist[i]; }
  return SD;
}
int32 xDistortionSTD::CalcSD(const uint16* restrict Org, const uint16* restrict Dist, int32 OStride, int32 DStride, int32 Width, int32 Height)
{
  int32 SD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SD += (int32)Org[x] - (int32)Dist[x]; }
    Org  += OStride;
    Dist += DStride;
  }
  return SD;
}
uint64 xDistortionSTD::CalcSSD(const uint16* restrict Org, const uint16* restrict Dist, int32 Area)
{
  uint64 SSD = 0;
  for(int32 i=0; i < Area; i++) { SSD += (uint64)xPow2(((int32)Org[i]) - ((int32)Dist[i])); }
  return SSD;
}
uint64 xDistortionSTD::CalcSSD(const uint16* restrict Org, const uint16* restrict Dist, int32 OStride, int32 DStride, int32 Width, int32 Height)
{
  uint64 SSD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SSD += (uint64)xPow2(((int32)Org[x]) - ((int32)Dist[x])); }
    Org  += OStride;
    Dist += DStride;
  }
  return SSD;
}

int64 xDistortionSTD::CalcWeightedSD(const uint16* restrict Org, const uint16* restrict Dist, const uint16* restrict Mask, int32 Area)
{
  int64 SD = 0;
  for(int32 i=0; i < Area; i++) { SD += ((int32)Org[i] - (int32)Dist[i]) * (int32)Mask[i]; }
  return SD;
}
int64 xDistortionSTD::CalcWeightedSD(const uint16* restrict Org, const uint16* restrict Dist, const uint16* restrict Mask, int32 OStride, int32 DStride, int32 MStride, int32 Width, int32 Height)
{
  int64 SD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SD += ((int32)Org[x] - (int32)Dist[x]) * (int32)Mask[x]; }
    Org  += OStride;
    Dist += DStride;
    Mask += MStride;
  }
  return SD;
}
uint64 xDistortionSTD::CalcWeightedSSD(const uint16* restrict Org, const uint16* restrict Dist, const uint16* restrict Mask, int32 Area)
{
  uint64 SSD = 0;
  for(int32 i=0; i < Area; i++) { SSD += ((uint64)xPow2(((int32)Org[i]) - ((int32)Dist[i]))) * (uint64)Mask[i]; }
  return SSD;
}
uint64 xDistortionSTD::CalcWeightedSSD(const uint16* restrict Org, const uint16* restrict Dist, const uint16* restrict Mask, int32 OStride, int32 DStride, int32 MStride, int32 Width, int32 Height)
{
  uint64 SSD = 0;
  for(int32 y=0; y<Height; y++)
  {
    for(int32 x=0; x<Width; x++) { SSD += ((uint64)xPow2(((int32)Org[x]) - ((int32)Dist[x]))) * (uint64)Mask[x]; }
    Org  += OStride;
    Dist += DStride;
    Mask += MStride;
  }
  return SSD;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
