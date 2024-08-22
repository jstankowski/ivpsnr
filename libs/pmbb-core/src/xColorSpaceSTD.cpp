/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xColorSpaceSTD.h"
#include "xColorSpaceCoeff.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xColorSpaceSTD::ConvertRGB2YCbCr_F32(uint16* restrict Y, uint16* restrict U, uint16* restrict V, const uint16* R, const uint16* G, const uint16* B, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
{
  const flt32 Y_R = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][0][0];
  const flt32 Y_G = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][0][1];
  const flt32 Y_B = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][0][2];
  const flt32 U_R = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][1][0];
  const flt32 U_G = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][1][1];
  const flt32 U_B = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][1][2];
  const flt32 V_R = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][2][0];
  const flt32 V_G = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][2][1];
  const flt32 V_B = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][2][2];

  const int32 Mid = (int32)xBitDepth2MidValue(BitDepth);
  const int32 Max = (int32)xBitDepth2MaxValue(BitDepth);

  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      flt32 r  = (flt32)(R[x]);
      flt32 g  = (flt32)(G[x]);
      flt32 b  = (flt32)(B[x]);
      int32 ty = (int32)round(Y_R*r + Y_G*g + Y_B*b);
      int32 tu = (int32)round(U_R*r + U_G*g + U_B*b);
      int32 tv = (int32)round(V_R*r + V_G*g + V_B*b);
      int32 cy = xClipU(ty      , Max);
      int32 cu = xClipU(tu + Mid, Max);
      int32 cv = xClipU(tv + Mid, Max);
      Y[x] = (uint16)cy;
      U[x] = (uint16)cu;
      V[x] = (uint16)cv;
    }
    R += SrcStride; G += SrcStride; B += SrcStride;
    Y += DstStride; U += DstStride; V += DstStride;
  }
}
void xColorSpaceSTD::ConvertYCbCr2RGB_F32(uint16* restrict R, uint16* restrict G, uint16* restrict B, const uint16* Y, const uint16* U, const uint16* V, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
{
//const flt32 R_Y = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][0][0]; //is always 1.0
//const flt32 R_U = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][0][1]; //is always 0.0
  const flt32 R_V = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][0][2];
//const flt32 G_Y = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][1][0]; //is always 1.0
  const flt32 G_U = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][1][1];
  const flt32 G_V = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][1][2];
//const flt32 B_Y = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][2][0]; //is always 1.0
  const flt32 B_U = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][2][1];
//const flt32 B_V = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][2][2]; //is always 0.0

  const int32  Mid = (int32)xBitDepth2MidValue(BitDepth);
  const int32  Max = (int32)xBitDepth2MaxValue(BitDepth);

  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      flt32 iy = (flt32)((int32)(Y[x])      );
      flt32 iu = (flt32)((int32)(U[x]) - Mid);
      flt32 iv = (flt32)((int32)(V[x]) - Mid);
      int32 r  = (int32)round(iy +        + R_V*iv);
      int32 g  = (int32)round(iy + G_U*iu + G_V*iv);
      int32 b  = (int32)round(iy + B_U*iu         );
      int32 cr = xClipU(r, Max);
      int32 cg = xClipU(g, Max);
      int32 cb = xClipU(b, Max);
      R[x] = (uint16)cr;
      G[x] = (uint16)cg;
      B[x] = (uint16)cb;
    }
    Y += SrcStride; U += SrcStride; V += SrcStride;
    R += DstStride; G += DstStride; B += DstStride;
  }
}
void xColorSpaceSTD::ConvertRGB2YCbCr_F32_LR(uint16* restrict Y, uint16* restrict U, uint16* restrict V, const uint16* R, const uint16* G, const uint16* B, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc, bool LimitedRangeYCbCr, bool LimitedRangeRGB)
{
  using tRng = xColorSpaceLimitedRange::tRng;

  const flt32 Y_R = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][0][0];
  const flt32 Y_G = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][0][1];
  const flt32 Y_B = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][0][2];
  const flt32 U_R = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][1][0];
  const flt32 U_G = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][1][1];
  const flt32 U_B = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][1][2];
  const flt32 V_R = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][2][0];
  const flt32 V_G = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][2][1];
  const flt32 V_B = xColorSpaceCoeff<flt32>::c_RGB2YCbCr[(int32)ClrSpc][2][2];

  const int32 SignalMid    = (int32)xBitDepth2MidValue(BitDepth);
  const tRng  SignalRangeX = LimitedRangeRGB ? xColorSpaceLimitedRange::getSignalRangeL(BitDepth) : tRng{0, xBitDepth2MaxValue(BitDepth) };
  const int32 SignalMinX   = SignalRangeX[0];
  const int32 SignalMaxX   = SignalRangeX[1];
  const int32 SignalRngX   = SignalMaxX - SignalMinX + 1;
  const flt32 MultiplierX  = (flt32)xBitDepth2MaxValue(BitDepth) / (flt32)SignalRngX;
  const tRng  SignalRangeL = LimitedRangeYCbCr ? xColorSpaceLimitedRange::getSignalRangeL(BitDepth) : tRng{ 0, xBitDepth2MaxValue(BitDepth) };
  const int32 SignalMinL   = SignalRangeL[0];
  const int32 SignalMaxL   = SignalRangeL[1];
  const int32 SignalRngL   = SignalMaxL - SignalMinL + 1;
  const flt32 MultiplierL  = (flt32)SignalRngL / (flt32)xBitDepth2MaxValue(BitDepth);
  const tRng  SignalRangeC = LimitedRangeYCbCr ? xColorSpaceLimitedRange::getSignalRangeC(BitDepth) : tRng{ 0, xBitDepth2MaxValue(BitDepth) };
  const int32 SignalMinC   = SignalRangeC[0];
  const int32 SignalMaxC   = SignalRangeC[1];
  const int32 SignalRngC   = SignalMaxC - SignalMinC + 1;
  const flt32 MultiplierC  = (flt32)SignalRngC / (flt32)xBitDepth2MaxValue(BitDepth);
  const tRng  RoomRange    = LimitedRangeYCbCr ? xColorSpaceLimitedRange::getRoomRange(BitDepth) : tRng{ 0, xBitDepth2MaxValue(BitDepth) };
  const int32 RoomMin      = RoomRange[0];
  const int32 RoomMax      = RoomRange[1];

  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      flt32 r  = (flt32)((int32)(R[x]) - SignalMinX) * MultiplierX;
      flt32 g  = (flt32)((int32)(G[x]) - SignalMinX) * MultiplierX;
      flt32 b  = (flt32)((int32)(B[x]) - SignalMinX) * MultiplierX;
      int32 ty = SignalMinL + (int32)round((Y_R*r + Y_G*g + Y_B*b) * MultiplierL);
      int32 tu = SignalMinC + (int32)round((U_R*r + U_G*g + U_B*b) * MultiplierC);
      int32 tv = SignalMinC + (int32)round((V_R*r + V_G*g + V_B*b) * MultiplierC);
      int32 cy = xClip(ty            , RoomMin, RoomMax);
      int32 cu = xClip(tu + SignalMid, RoomMin, RoomMax);
      int32 cv = xClip(tv + SignalMid, RoomMin, RoomMax);
      Y[x] = (uint16)cy;
      U[x] = (uint16)cu;
      V[x] = (uint16)cv;
    }
    R += SrcStride; G += SrcStride; B += SrcStride;
    Y += DstStride; U += DstStride; V += DstStride;
  }
}
void xColorSpaceSTD::ConvertYCbCr2RGB_F32_LR(uint16* restrict R, uint16* restrict G, uint16* restrict B, const uint16* Y, const uint16* U, const uint16* V, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc, bool LimitedRangeYCbCr, bool LimitedRangeRGB)
{
  using tRng = xColorSpaceLimitedRange::tRng;

  //const flt32 R_Y = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][0][0]; //is always 1.0
//const flt32 R_U = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][0][1]; //is always 0.0
  const flt32 R_V = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][0][2];
//const flt32 G_Y = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][1][0]; //is always 1.0
  const flt32 G_U = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][1][1];
  const flt32 G_V = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][1][2];
//const flt32 B_Y = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][2][0]; //is always 1.0
  const flt32 B_U = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][2][1];
//const flt32 B_V = xColorSpaceCoeff<flt32>::c_YCbCr2RGB[(uint32)ClrSpc][2][2]; //is always 0.0

  const int32 SignalMid    = (int32)xBitDepth2MidValue(BitDepth);
  const tRng  SignalRangeL = LimitedRangeYCbCr ? xColorSpaceLimitedRange::getSignalRangeL(BitDepth) : tRng{ 0, xBitDepth2MaxValue(BitDepth) };
  const int32 SignalMinL   = SignalRangeL[0];
  const int32 SignalMaxL   = SignalRangeL[1];
  const int32 SignalRngL   = SignalMaxL - SignalMinL + 1;
  const flt32 MultiplierL  = (flt32)xBitDepth2MaxValue(BitDepth) / (flt32)SignalRngL;
  const tRng  SignalRangeC = LimitedRangeYCbCr ? xColorSpaceLimitedRange::getSignalRangeC(BitDepth) : tRng{ 0, xBitDepth2MaxValue(BitDepth) };
  const int32 SignalMinC   = SignalRangeC[0];
  const int32 SignalMaxC   = SignalRangeC[1];
  const int32 SignalRngC   = SignalMaxC - SignalMinC + 1;
  const flt32 MultiplierC  = (flt32)xBitDepth2MaxValue(BitDepth) / (flt32)SignalRngC;
  const tRng  SignalRangeX = LimitedRangeRGB ? xColorSpaceLimitedRange::getSignalRangeL(BitDepth) : tRng{0, xBitDepth2MaxValue(BitDepth) };
  const int32 SignalMinX   = SignalRangeX[0];
  const int32 SignalMaxX   = SignalRangeX[1];
  const int32 SignalRngX   = SignalMaxX - SignalMinX + 1;
  const flt32 MultiplierX  = (flt32)SignalRngX / (flt32)xBitDepth2MaxValue(BitDepth);
  const tRng  RoomRange    = LimitedRangeRGB ? xColorSpaceLimitedRange::getRoomRange(BitDepth) : tRng{ 0, xBitDepth2MaxValue(BitDepth) };
  const int32 RoomMin      = RoomRange[0];
  const int32 RoomMax      = RoomRange[1];    

  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      flt32 iy = (flt32)((int32)(Y[x])             - SignalMinL) * MultiplierL;
      flt32 iu = (flt32)((int32)(U[x]) - SignalMid - SignalMinC) * MultiplierC;
      flt32 iv = (flt32)((int32)(V[x]) - SignalMid - SignalMinC) * MultiplierC;
      int32 r  = SignalMinX + (int32)round((iy +        + R_V*iv) * MultiplierX);
      int32 g  = SignalMinX + (int32)round((iy + G_U*iu + G_V*iv) * MultiplierX);
      int32 b  = SignalMinX + (int32)round((iy + B_U*iu         ) * MultiplierX);
      int32 cr = xClip(r, RoomMin, RoomMax);
      int32 cg = xClip(g, RoomMin, RoomMax);
      int32 cb = xClip(b, RoomMin, RoomMax);
      R[x] = (uint16)cr;
      G[x] = (uint16)cg;
      B[x] = (uint16)cb;
    }
    Y += SrcStride; U += SrcStride; V += SrcStride;
    R += DstStride; G += DstStride; B += DstStride;
  }
}
void xColorSpaceSTD::ConvertRGB2YCbCr_I32(uint16* restrict Y, uint16* restrict U, uint16* restrict V, const uint16* R, const uint16* G, const uint16* B, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
{
  const int32 Y_R = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][0][0];
  const int32 Y_G = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][0][1];
  const int32 Y_B = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][0][2];
  const int32 U_R = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][1][0];
  const int32 U_G = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][1][1];
//const int32 U_B = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][1][2]; //is always 0.5
//const int32 V_R = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][2][0]; //is always 0.5
  const int32 V_G = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][2][1];
  const int32 V_B = xColorSpaceCoeff<int32>::c_RGB2YCbCr[(int32)ClrSpc][2][2];

  const int32  Add = xColorSpaceCoeff<int32>::c_Add;
  const uint32 Shr = xColorSpaceCoeff<int32>::c_Precision;
  const uint32 Shl = Shr - 1;
  const int32  Mid = (int32)xBitDepth2MidValue(BitDepth);
  const int32  Max = (int32)xBitDepth2MaxValue(BitDepth);

  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      int32 r  = R[x];
      int32 g  = G[x];
      int32 b  = B[x];
      int32 ty = ((Y_R*r    + Y_G*g + Y_B*b    + Add)>>Shr);
      int32 tu = ((U_R*r    + U_G*g + (b<<Shl) + Add)>>Shr);
      int32 tv = (((r<<Shl) + V_G*g + V_B*b    + Add)>>Shr);
      int32 cy = xClipU(ty      , Max);
      int32 cu = xClipU(tu + Mid, Max);
      int32 cv = xClipU(tv + Mid, Max);
      Y[x] = (uint16)cy;
      U[x] = (uint16)cu;
      V[x] = (uint16)cv;
    }
    R += SrcStride; G += SrcStride; B += SrcStride;
    Y += DstStride; U += DstStride; V += DstStride;
  }
}
void xColorSpaceSTD::ConvertYCbCr2RGB_I32(uint16* restrict R, uint16* restrict G, uint16* restrict B, const uint16* Y, const uint16* U, const uint16* V, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth, eClrSpcLC ClrSpc)
{
//const int32 R_Y = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][0][0]; //is always 1.0
//const int32 R_U = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][0][1]; //is always 0.0
  const int32 R_V = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][0][2];
//const int32 G_Y = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][1][0]; //is always 1.0
  const int32 G_U = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][1][1];
  const int32 G_V = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][1][2];
//const int32 B_Y = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][2][0]; //is always 1.0
  const int32 B_U = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][2][1];
//const int32 B_V = xColorSpaceCoeff<int32>::c_YCbCr2RGB[(uint32)ClrSpc][2][2]; //is always 0.0

  const int32  Add = xColorSpaceCoeff<int32>::c_Add;
  const uint32 Shr = xColorSpaceCoeff<int32>::c_Precision;
  const int32  Mid = (int32)xBitDepth2MidValue(BitDepth);
  const int32  Max = (int32)xBitDepth2MaxValue(BitDepth);

  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      int32 iy = (int32)(Y[x]);
      int32 iu = (int32)(U[x]) - Mid;
      int32 iv = (int32)(V[x]) - Mid;
      int32 sy = (iy<<Shr) + Add;
      int32 r  = (sy +        + R_V*iv)>>Shr;
      int32 g  = (sy + G_U*iu + G_V*iv)>>Shr;
      int32 b  = (sy + B_U*iu         )>>Shr;
      int32 cr = xClipU(r, Max);
      int32 cg = xClipU(g, Max);
      int32 cb = xClipU(b, Max);
      R[x] = (uint16)cr;
      G[x] = (uint16)cg;
      B[x] = (uint16)cb;
    }
    Y += SrcStride; U += SrcStride; V += SrcStride;
    R += DstStride; G += DstStride; B += DstStride;
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
