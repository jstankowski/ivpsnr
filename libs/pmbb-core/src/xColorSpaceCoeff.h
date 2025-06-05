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
  using tMatCoeffsF32 = std::array<std::array<flt32, 3>, 3>;
  using tMatCoeffsI32 = std::array<std::array<int32, 3>, 3>;

public:
  static std::string formatColorSpaceCoeff(tMatCoeffsF32 Coeff, const std::vector<std::string>& SrcCmp, const std::vector<std::string>& DstCmp);
  static std::string formatColorSpaceCoeff(tMatCoeffsI32 Coeff, const std::vector<std::string>& SrcCmp, const std::vector<std::string>& DstCmp);
};

//===============================================================================================================================================================================================================
// xColorSpaceCoeffYCbCr
//===============================================================================================================================================================================================================

//INFO - fake template bacause of:
// https://stackoverflow.com/questions/29551223/static-constexpr-function-called-in-a-constant-expression-is-an-error
// https://stackoverflow.com/questions/62147479/static-constexpr-member-variable-initialization 

class xColorSpaceCoeffYCbCr : public xColorSpaceCoeffCommon
{
public:
  static constexpr int32 c_NumClrSpcs = 4;
  static constexpr int32 c_Precision  = 16;
  static constexpr int32 c_Mul        = 1 << c_Precision;
  static constexpr int32 c_Add        = (1 << c_Precision) >> 1;

public:
  static constexpr std::array<tMatCoeffsF32, c_NumClrSpcs> c_RGB2YCbCr_F32 =
  {{
    {{ {(flt32)0.299 , (flt32)0.587 , (flt32)0.114 }, { (flt32)-0.1687, (flt32)-0.3313, (flt32)0.5}, { (flt32)0.5, (flt32)-0.4187, (flt32)-0.0813} }},  //BT.601 / SMPTE170M / JPEG
    {{ {(flt32)0.2126, (flt32)0.7152, (flt32)0.0722}, { (flt32)-0.1146, (flt32)-0.3854, (flt32)0.5}, { (flt32)0.5, (flt32)-0.4542, (flt32)-0.0458} }},  //BT709
    {{ {(flt32)0.212 , (flt32)0.701 , (flt32)0.087 }, { (flt32)-0.1161, (flt32)-0.3839, (flt32)0.5}, { (flt32)0.5, (flt32)-0.4448, (flt32)-0.0552} }},  //SMPTE240M
    {{ {(flt32)0.2627, (flt32)0.678 , (flt32)0.0593}, { (flt32)-0.1396, (flt32)-0.3604, (flt32)0.5}, { (flt32)0.5, (flt32)-0.4598, (flt32)-0.0402} }},  //BT2020
  }};

  static constexpr std::array<tMatCoeffsF32, c_NumClrSpcs> c_YCbCr2RGB_F32 =
  {{
    {{ { (flt32)1.0, (flt32)0.0, (flt32)1.402 }, { (flt32)1.0, (flt32)-0.3441, (flt32)-0.7141}, { (flt32)1.0, (flt32)1.772 , (flt32)0.0} }},  //BT.601 / SMPTE170M / JPEG
    {{ { (flt32)1.0, (flt32)0.0, (flt32)1.5748}, { (flt32)1.0, (flt32)-0.1873, (flt32)-0.4681}, { (flt32)1.0, (flt32)1.8556, (flt32)0.0} }},  //BT709
    {{ { (flt32)1.0, (flt32)0.0, (flt32)1.576 }, { (flt32)1.0, (flt32)-0.2266, (flt32)-0.4766}, { (flt32)1.0, (flt32)1.826 , (flt32)0.0} }},  //SMPTE240M
    {{ { (flt32)1.0, (flt32)0.0, (flt32)1.4746}, { (flt32)1.0, (flt32)-0.1646, (flt32)-0.5714}, { (flt32)1.0, (flt32)1.8814, (flt32)0.0} }},  //BT2020
  }};

  static constexpr std::array<tMatCoeffsI32, c_NumClrSpcs> c_RGB2YCbCr_I32 =
  {{
    {{ {19595, 38470, 7471}, {-11058, -21710, 32768}, { 32768, -27439, -5329} }},  //BT.601 / SMPTE170M / JPEG
    {{ {13933, 46871, 4732}, { -7509, -25259, 32768}, { 32768, -29763, -3005} }},  //BT709
    {{ {13894, 45941, 5701}, { -7609, -25159, 32768}, { 32768, -29150, -3618} }},  //SMPTE240M
    {{ {17216, 44434, 3886}, { -9151, -23617, 32768}, { 32768, -30133, -2635} }},  //BT2020
  }};

  static constexpr std::array<tMatCoeffsI32, c_NumClrSpcs> c_YCbCr2RGB_I32 =
  {{
    {{ {65536, 0,  91881}, {65536, -22553, -46802 }, {65536, 116130, 0} }},  //BT.601 / SMPTE170M / JPEG
    {{ {65536, 0, 103206}, {65536, -12276, -30679 }, {65536, 121609, 0} }},  //BT709
    {{ {65536, 0, 103285}, {65536, -14852, -31236 }, {65536, 119669, 0} }},  //SMPTE240M
    {{ {65536, 0,  96639}, {65536, -10784, -37444 }, {65536, 123299, 0} }},  //BT2020
  }};

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