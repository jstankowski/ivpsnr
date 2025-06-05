/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefIVQM.h"
#include <array>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xStructSimConsts //Structural Similarity Constants
{
public:
  //regular SSIM
  static constexpr int32 c_FilterSize  = 11;
  static constexpr int32 c_FilterRange = c_FilterSize >> 1;
  static constexpr int32 c_FilterArea  = c_FilterSize * c_FilterSize;
  static constexpr flt64 c_InvFltrArea = (flt64)1.0 / (flt64)c_FilterArea;

  //block SSIM
  static constexpr int32 c_Log2Block8Size  = 3;
  static constexpr int32 c_Block8Size      = 1<<c_Log2Block8Size;

  static constexpr int32 c_Log2Block16Size = 4;
  static constexpr int32 c_Block16Size     = 1<<c_Log2Block16Size;

  static constexpr int32 c_Log2Block32Size = 5;
  static constexpr int32 c_Block32Size     = 1 << c_Log2Block32Size;


  template<class XXX> static constexpr XXX c_Sigma = XXX(1.50);
  template<class XXX> static constexpr XXX c_K1    = XXX(0.01);
  template<class XXX> static constexpr XXX c_K2    = XXX(0.03);

  static constexpr int32                   c_NumMultiScales  = 5;
  static constexpr int32                   c_ScaledSizeLimit = 32; //sometimes 22

  template<class XXX> static constexpr std::array<std::array<XXX, c_NumMultiScales>, c_NumMultiScales> c_MultiScaleExponentWeights =
  { {
    { XXX(1.0000), XXX(0.0000), XXX(0.0000), XXX(0.0000), XXX(0.0000) },
    { XXX(0.1356), XXX(0.8644), XXX(0.0000), XXX(0.0000), XXX(0.0000) },
    { XXX(0.0711), XXX(0.4530), XXX(0.4760), XXX(0.0000), XXX(0.0000) },
    { XXX(0.0517), XXX(0.3295), XXX(0.3462), XXX(0.2726), XXX(0.0000) },
    { XXX(0.0448), XXX(0.2856), XXX(0.3001), XXX(0.2363), XXX(0.1333) },
  } };

  //regular gaussian filter with float coefficients
  using tFltrRglrFlt = std::array< std::array <flt32, c_FilterSize>, c_FilterSize>;

  static constexpr tFltrRglrFlt c_FilterRglrGaussFlt =
  { {
    { 0.0000010576f, 0.0000078144f, 0.0000370225f, 0.0001124644f, 0.0002190507f, 0.0002735612f, 0.0002190507f, 0.0001124644f, 0.0000370225f, 0.0000078144f, 0.0000010576f, },
    { 0.0000078144f, 0.0000577411f, 0.0002735612f, 0.0008310054f, 0.0016185776f, 0.0020213588f, 0.0016185776f, 0.0008310054f, 0.0002735612f, 0.0000577411f, 0.0000078144f, },
    { 0.0000370225f, 0.0002735612f, 0.0012960556f, 0.0039370693f, 0.0076683638f, 0.0095766275f, 0.0076683638f, 0.0039370693f, 0.0012960556f, 0.0002735612f, 0.0000370225f, },
    { 0.0001124644f, 0.0008310054f, 0.0039370693f, 0.0119597604f, 0.0232944325f, 0.0290912256f, 0.0232944325f, 0.0119597604f, 0.0039370693f, 0.0008310054f, 0.0001124644f, },
    { 0.0002190507f, 0.0016185776f, 0.0076683638f, 0.0232944325f, 0.0453713591f, 0.0566619705f, 0.0453713591f, 0.0232944325f, 0.0076683638f, 0.0016185776f, 0.0002190507f, },
    { 0.0002735612f, 0.0020213588f, 0.0095766275f, 0.0290912256f, 0.0566619705f, 0.0707622378f, 0.0566619705f, 0.0290912256f, 0.0095766275f, 0.0020213588f, 0.0002735612f, },
    { 0.0002190507f, 0.0016185776f, 0.0076683638f, 0.0232944325f, 0.0453713591f, 0.0566619705f, 0.0453713591f, 0.0232944325f, 0.0076683638f, 0.0016185776f, 0.0002190507f, },
    { 0.0001124644f, 0.0008310054f, 0.0039370693f, 0.0119597604f, 0.0232944325f, 0.0290912256f, 0.0232944325f, 0.0119597604f, 0.0039370693f, 0.0008310054f, 0.0001124644f, },
    { 0.0000370225f, 0.0002735612f, 0.0012960556f, 0.0039370693f, 0.0076683638f, 0.0095766275f, 0.0076683638f, 0.0039370693f, 0.0012960556f, 0.0002735612f, 0.0000370225f, },
    { 0.0000078144f, 0.0000577411f, 0.0002735612f, 0.0008310054f, 0.0016185776f, 0.0020213588f, 0.0016185776f, 0.0008310054f, 0.0002735612f, 0.0000577411f, 0.0000078144f, },
    { 0.0000010576f, 0.0000078144f, 0.0000370225f, 0.0001124644f, 0.0002190507f, 0.0002735612f, 0.0002190507f, 0.0001124644f, 0.0000370225f, 0.0000078144f, 0.0000010576f, },
  } };

  //it`s stupid approach but still usefull for debuging purposes
  //static constexpr tFltrRglrFlt c_FilterGrlrAvgFlt =
  //{ {
  //  { c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, },
  //  { c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, },
  //  { c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, },
  //  { c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, },
  //  { c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, },
  //  { c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, },
  //  { c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, },
  //  { c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, },
  //  { c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, },
  //  { c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, },
  //  { c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, c_InvFltrArea, },
  //} };

  //regular gaussian filter with integer coefficients (includes variant with padding to multiple of SIMD register width)

  using tFltrRglrInt    = std::array< std::array <int16, c_FilterSize>, c_FilterSize>;
  using tFltrRglrIntPad = std::array< std::array <int16, 16          >, c_FilterSize>;

  static constexpr int32 c_FltrIntShift  = 18;
  static constexpr int32 c_FltrIntMul    = 1<< c_FltrIntShift;
  static constexpr flt64 c_InvFltrIntMul = (flt64)1.0 / (flt64)c_FltrIntMul;

  static constexpr tFltrRglrInt c_FilterRglrGaussInt =
  { {
    {     0,     2,    10,    29,    57,    72,    57,    29,    10,     2,     0, },
    {     2,    15,    72,   218,   424,   530,   424,   218,    72,    15,     2, },
    {    10,    72,   340,  1032,  2010,  2510,  2010,  1032,   340,    72,    10, },
    {    29,   218,  1032,  3135,  6107,  7626,  6107,  3135,  1032,   218,    29, },
    {    57,   424,  2010,  6107, 11894, 14854, 11894,  6107,  2010,   424,    57, },
    {    72,   530,  2510,  7626, 14855, 18550, 14855,  7626,  2510,   530,    72, },
    {    57,   424,  2010,  6107, 11894, 14854, 11894,  6107,  2010,   424,    57, },
    {    29,   218,  1032,  3135,  6107,  7626,  6107,  3135,  1032,   218,    29, },
    {    10,    72,   340,  1032,  2010,  2510,  2010,  1032,   340,    72,    10, },
    {     2,    15,    72,   218,   424,   530,   424,   218,    72,    15,     2, },
    {     0,     2,    10,    29,    57,    72,    57,    29,    10,     2,     0, },
  } };

  static constexpr tFltrRglrIntPad c_FilterRglrGaussIntPad =
  { {
    {     0,     2,    10,    29,    57,    72,    57,    29,    10,     2,     0,     0,     0,     0,     0,     0, },
    {     2,    15,    72,   218,   424,   530,   424,   218,    72,    15,     2,     0,     0,     0,     0,     0, },
    {    10,    72,   340,  1032,  2010,  2510,  2010,  1032,   340,    72,    10,     0,     0,     0,     0,     0, },
    {    29,   218,  1032,  3135,  6107,  7626,  6107,  3135,  1032,   218,    29,     0,     0,     0,     0,     0, },
    {    57,   424,  2010,  6107, 11894, 14854, 11894,  6107,  2010,   424,    57,     0,     0,     0,     0,     0, },
    {    72,   530,  2510,  7626, 14855, 18550, 14855,  7626,  2510,   530,    72,     0,     0,     0,     0,     0, },
    {    57,   424,  2010,  6107, 11894, 14854, 11894,  6107,  2010,   424,    57,     0,     0,     0,     0,     0, },
    {    29,   218,  1032,  3135,  6107,  7626,  6107,  3135,  1032,   218,    29,     0,     0,     0,     0,     0, },
    {    10,    72,   340,  1032,  2010,  2510,  2010,  1032,   340,    72,    10,     0,     0,     0,     0,     0, },
    {     2,    15,    72,   218,   424,   530,   424,   218,    72,    15,     2,     0,     0,     0,     0,     0, },
    {     0,     2,    10,    29,    57,    72,    57,    29,    10,     2,     0,     0,     0,     0,     0,     0, },
  } };

  //experimental gaussian filter for block mode
  using tFltrBlckI8  = std::array< std::array <int16, c_Block8Size >, c_Block8Size >;
  using tFltrBlckI16 = std::array< std::array <int16, c_Block16Size>, c_Block16Size>;

  static constexpr tFltrBlckI8 c_FilterBlckGaussInt8 =
  { {
    {    81,   308,   749,  1168,  1168,   749,   308,    81, },
    {   308,  1168,  2842,  4432,  4432,  2842,  1168,   308, },
    {   749,  2842,  6913, 10781, 10781,  6913,  2842,   749, },
    {  1168,  4432, 10781, 16814, 16814, 10781,  4432,  1168, },
    {  1168,  4432, 10781, 16814, 16814, 10781,  4432,  1168, },
    {   749,  2842,  6913, 10781, 10781,  6913,  2842,   749, },
    {   308,  1168,  2842,  4432,  4432,  2842,  1168,   308, },
    {    81,   308,   749,  1168,  1168,   749,   308,    81, },
  } };

  static constexpr tFltrBlckI16 c_FilterBlckGaussInt16 =
  { {
    {     0,     1,     4,    10,    19,    32,    45,    54,    54,    45,    32,    19,    10,     4,     1,     0, },
    {     1,     5,    13,    32,    64,   108,   152,   181,   181,   152,   108,    64,    32,    13,     5,     1, },
    {     4,    13,    38,    91,   181,   305,   432,   514,   514,   432,   305,   181,    91,    38,    13,     4, },
    {    10,    32,    91,   216,   432,   727,  1029,  1224,  1224,  1029,   727,   432,   216,    91,    32,    10, },
    {    19,    64,   181,   432,   865,  1456,  2061,  2451,  2451,  2061,  1456,   865,   432,   181,    64,    19, },
    {    32,   108,   305,   727,  1456,  2451,  3469,  4126,  4126,  3469,  2451,  1456,   727,   305,   108,    32, },
    {    45,   152,   432,  1029,  2061,  3469,  4909,  5839,  5839,  4909,  3469,  2061,  1029,   432,   152,    45, },
    {    54,   181,   514,  1224,  2451,  4126,  5839,  6947,  6947,  5839,  4126,  2451,  1224,   514,   181,    54, },
    {    54,   181,   514,  1224,  2451,  4126,  5839,  6947,  6947,  5839,  4126,  2451,  1224,   514,   181,    54, },
    {    45,   152,   432,  1029,  2061,  3469,  4909,  5839,  5839,  4909,  3469,  2061,  1029,   432,   152,    45, },
    {    32,   108,   305,   727,  1456,  2451,  3469,  4126,  4126,  3469,  2451,  1456,   727,   305,   108,    32, },
    {    19,    64,   181,   432,   865,  1456,  2061,  2451,  2451,  2061,  1456,   865,   432,   181,    64,    19, },
    {    10,    32,    91,   216,   432,   727,  1029,  1224,  1224,  1029,   727,   432,   216,    91,    32,    10, },
    {     4,    13,    38,    91,   181,   305,   432,   514,   514,   432,   305,   181,    91,    38,    13,     4, },
    {     1,     5,    13,    32,    64,   108,   152,   181,   181,   152,   108,    64,    32,    13,     5,     1, },
    {     0,     1,     4,    10,    19,    32,    45,    54,    54,    45,    32,    19,    10,     4,     1,     0, },
  } };
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB