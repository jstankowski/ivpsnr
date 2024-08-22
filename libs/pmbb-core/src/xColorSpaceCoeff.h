/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "xCommonDefCORE.h"
#include <array>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xColorSpaceCoeffCommon
{
public:
  using tBscCoeffsF = std::array<flt64, 3>;
  using tMatCoeffsF = std::array<std::array<flt64, 3>, 3>;

};

//===============================================================================================================================================================================================================
// xColorSpaceCoeffYCbCr
//===============================================================================================================================================================================================================

//INFO - fake template bacause of:
// https://stackoverflow.com/questions/29551223/static-constexpr-function-called-in-a-constant-expression-is-an-error
// https://stackoverflow.com/questions/62147479/static-constexpr-member-variable-initialization 

template <typename T = flt64> class xColorSpaceCoeffYCbCr
{
public:
  using tBscCoeffs = std::array<flt64, 3>;
  using tMatCoeffs = std::array<std::array<flt64, 3>, 3>;
  static constexpr int32 c_NumClrSpcs = 4;

  static constexpr std::array<tBscCoeffs, c_NumClrSpcs> c_BaseCoeffs =
  { {
    { 0.299 , 0.587 , 0.114 }, //BT.601 / SMPTE170M
    { 0.2126, 0.7152, 0.0722}, //BT.709
    { 0.2120, 0.7010, 0.0870}, //SMPTE240M
    { 0.2627, 0.6780, 0.0593}, //BT.2020    
  } };

protected:
  constexpr static inline T xcPow2(T x) { return x * x; }

  static constexpr inline tMatCoeffs calcMatrixRGB2YCbCr(const tBscCoeffs& BaseCoeffs)
  {
    const flt64 coeffR = BaseCoeffs[0];
    const flt64 coeffG = BaseCoeffs[1];
    const flt64 coeffB = BaseCoeffs[2];

    const flt64 scaleB = 0.5 / (coeffB - 1.0);
    const flt64 scaleR = 0.5 / (coeffR - 1.0);

    tMatCoeffs RGB2YCbCr =
    { {
      { coeffR         , coeffG         , coeffB          },
      { scaleB * coeffR, scaleB * coeffG, 0.5             },
      { 0.5            , scaleR * coeffG, scaleR * coeffB },
    } };

    // Y_R, Y_G, Y_B
    // U_R, U_G, U_B
    // V_R, V_B, V_G

    return RGB2YCbCr;
  };

  static constexpr inline tMatCoeffs calcMatrixYCbCr2RGB(const tBscCoeffs& BaseCoeffs)
  {
    const flt64 coeffR = BaseCoeffs[0];
    const flt64 coeffG = BaseCoeffs[1];
    const flt64 coeffB = BaseCoeffs[2];

    tMatCoeffs YCbCr2RGB =
    { {
      { 1.0, 0.0                                     , 2 - 2 * coeffR                           },
      { 1.0, 2.0 * (xcPow2(coeffB) - coeffB) / coeffG, 2.0 * (xcPow2(coeffR) - coeffR) / coeffG },
      { 1.0, 2 - 2 * coeffB                          , 0.0                                      },
    } };

    // R_Y, R_U, R_V
    // G_Y, G_U, G_V
    // B_Y, B_U, B_V

    return YCbCr2RGB;
  };

public:
  static constexpr std::array<tMatCoeffs, c_NumClrSpcs> c_MatricesRGB2YCbCr =
  {
    calcMatrixRGB2YCbCr(c_BaseCoeffs[0]), //BT.601 / SMPTE170M
    calcMatrixRGB2YCbCr(c_BaseCoeffs[1]), //BT.709
    calcMatrixRGB2YCbCr(c_BaseCoeffs[2]), //SMPTE240M
    calcMatrixRGB2YCbCr(c_BaseCoeffs[3]), //BT.2020    
  };
  static constexpr std::array<tMatCoeffs, c_NumClrSpcs> c_MatricesYCbCr2RGB =
  {
    calcMatrixYCbCr2RGB(c_BaseCoeffs[0]), //BT.601 / SMPTE170M
    calcMatrixYCbCr2RGB(c_BaseCoeffs[1]), //BT.709
    calcMatrixYCbCr2RGB(c_BaseCoeffs[2]), //SMPTE240M
    calcMatrixYCbCr2RGB(c_BaseCoeffs[3]), //BT.2020    
  };
};

//===============================================================================================================================================================================================================
// xColorSpaceCoeffExotic
//===============================================================================================================================================================================================================
class xColorSpaceCoeffExotic
{
public:
  using tMatCoeffs = std::array<std::array<flt64, 3>, 3>;

  //JPEG2000
  static constexpr tMatCoeffs c_MatrixRGB2YCbCr_JP2K =
  {{
    { 0.25,  0.50, 0.25}, //Y  = (R + 2G + B)/4
    { 0.00, -1.00, 1.00}, //Cb = B - G
    { 1.00, -1.00, 0.00}, //Cr = R - G
  }};
  static constexpr tMatCoeffs c_MatrixYCbCr2RGB_JP2K =
  {{
    { 1.00, -0.25,  0.75},//R = Cr + G = Cr + Y - (Cb + Cr)/4 = Y - 1/4*Cb + 3/4*Cr 
    { 1.00, -0.25, -0.25},//G = Y - (Cb + Cr)/4
    { 1.00,  0.75, -0.25},//B = Cb + G = Cb + Y - (Cb + Cr)/4 = Y + 3/4*Cb - 1/4*Cr 
  }};

  //YCoCg (luma, chrominance orange, chrominance green)
  static constexpr tMatCoeffs c_MatrixRGB2YCoCg =
  {{
    {  0.25, 0.50,  0.25}, //Y  = 0.25*R + 0.5*G + 0.25*B
    {  0.50, 0.00, -0.50}, //Co = 0.5*R - 0.5*B
    { -0.25, 0.50, -0.25}, //Cg = 
  }};
  static constexpr tMatCoeffs c_MatrixYCoCg2RGB =
  {{
    { 1,  1, -1}, 
    { 1,  0,  1}, 
    { 1, -1, -1}, 
  }};

  //YCoCg-R
  static constexpr tMatCoeffs c_MatrixRGB2YCoCgR =
  {{
    {  0.25, 0.50,  0.25}, 
    {  1.00, 0.00, -1.00}, 
    { -0.50, 1.00, -0.50}, 
  }};
  static constexpr tMatCoeffs c_MatrixYCoCgR2RGB =
  {{
    { 1,  0.50, -0.50},
    { 1,  0   ,  0.50},
    { 1, -0.50, -0.50},
  }};
};

//===============================================================================================================================================================================================================
// xColorSpaceCoeff
//===============================================================================================================================================================================================================
template <typename T = int32> class xColorSpaceCoeff
{
public:
  using tMatCoeffsF64 = std::array<std::array<flt64, 3>, 3>;
  using tMatCoeffsT   = std::array<std::array<T    , 3>, 3>;

  static constexpr int32 c_NumClrSpcs = xColorSpaceCoeffYCbCr<flt64>::c_NumClrSpcs;

  static constexpr uint32 c_Precision = std::is_integral_v<T> ? std::min((uint32)sizeof(T) << 2, (uint32)16) : 0;
  static constexpr flt64  c_Mul       = std::is_integral_v<T> ? (T)(((uint64)1) << c_Precision) : 1;
  static constexpr T      c_Add       = std::is_integral_v<T> ? (T)((1 << c_Precision) >> 1) : 0;
  static constexpr flt64  c_Rnd       = std::is_integral_v<T> ? 0.5 : 0;

public:
  static constexpr inline T xRnd(const flt64 v)
  {
    if constexpr(std::is_integral_v<T>) { return v >= 0.0 ? static_cast<T>(v + c_Rnd) : -static_cast<T>(-v + c_Rnd); }
    else                                { return (T)v;                                                               }
  }
  static constexpr inline T xCvt(const flt64 v)
  {
    return std::is_integral_v<T> ? xRnd(c_Mul * v) : (T)v;
  }

  static constexpr inline tMatCoeffsT convertMatCoeffs(const tMatCoeffsF64& Src)
  {
    tMatCoeffsT Dst =
    {{
      { xCvt(Src[0][0]), xCvt(Src[0][1]), xCvt(Src[0][2]) },
      { xCvt(Src[1][0]), xCvt(Src[1][1]), xCvt(Src[1][2]) },
      { xCvt(Src[2][0]), xCvt(Src[2][1]), xCvt(Src[2][2]) },
    }};

    return Dst;
  }

public:
  static constexpr std::array<tMatCoeffsT, c_NumClrSpcs> c_RGB2YCbCr =
  {
    convertMatCoeffs(xColorSpaceCoeffYCbCr<flt64>::c_MatricesRGB2YCbCr[0]), //BT.601 / SMPTE170M
    convertMatCoeffs(xColorSpaceCoeffYCbCr<flt64>::c_MatricesRGB2YCbCr[1]), //BT.709
    convertMatCoeffs(xColorSpaceCoeffYCbCr<flt64>::c_MatricesRGB2YCbCr[2]), //SMPTE240M
    convertMatCoeffs(xColorSpaceCoeffYCbCr<flt64>::c_MatricesRGB2YCbCr[3]), //BT.2020    
  };

  static constexpr std::array<tMatCoeffsT, c_NumClrSpcs> c_YCbCr2RGB =
  {
    convertMatCoeffs(xColorSpaceCoeffYCbCr<flt64>::c_MatricesYCbCr2RGB[0]), //BT.601 / SMPTE170M
    convertMatCoeffs(xColorSpaceCoeffYCbCr<flt64>::c_MatricesYCbCr2RGB[1]), //BT.709
    convertMatCoeffs(xColorSpaceCoeffYCbCr<flt64>::c_MatricesYCbCr2RGB[2]), //SMPTE240M
    convertMatCoeffs(xColorSpaceCoeffYCbCr<flt64>::c_MatricesYCbCr2RGB[3]), //BT.2020    
  };
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// xColorSpaceCoeff - instantiation for base types
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#ifndef PMBB_xColorSpaceCoeff_IMPLEMENTATION
extern template class xColorSpaceCoeff<int32>;
extern template class xColorSpaceCoeff<flt32>;
#endif // !PMBB_xColorSpaceCoeff_IMPLEMENTATION


//===============================================================================================================================================================================================================
// xColorSpaceLimitedRange
//===============================================================================================================================================================================================================
class xColorSpaceLimitedRange
{
public:
  using tRng = std::array<int32, 2>;

public:
  static constexpr tRng c_SignalRangeL8 = { 16,  235};
  static constexpr tRng c_SignalRangeC8 = { 16,  240};

public:
  static tRng getSignalRangeL(int32 BitDepth)
  {
    int32 Mul = 1 << (BitDepth - 8);
    tRng  Rng = { c_SignalRangeL8[0] * Mul, c_SignalRangeL8[1] * Mul };
    return Rng;
  }
  static tRng getSignalRangeC(int32 BitDepth)
  {
    int32 Mul = 1 << (BitDepth - 8);
    tRng  Rng = { c_SignalRangeC8[0] * Mul, c_SignalRangeC8[1] * Mul };
    return Rng;
  }
  static tRng getRoomRange(int32 BitDepth)
  {
    int32 Mul = 1 << (BitDepth - 8);
    tRng  Rng = { Mul, xBitDepth2MaxValue(BitDepth) - Mul };
    return Rng;
  }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB_NAMESPACE