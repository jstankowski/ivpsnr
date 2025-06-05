/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefIVQM.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

enum class eClrSpcApp : int32
{
  INVALID         = -1,
  //RGBs
  RGB             = 2,
  BGR             = 3,
  GBR             = 4,  
  //generic YCbCr
  YCbCr           = 16,
  //standardized YCbCr
  YCbCr_BT601     = 17,
  YCbCr_SMPTE170M = 18,
  YCbCr_BT709     = 19,
  YCbCr_SMPTE240M = 20,
  YCbCr_BT2020    = 21,
};

eClrSpcApp  xStr2ClrSpcApp    (const std::string& ClrSpc   );
std::string xClrSpcApp2Str    (eClrSpcApp         ClrSpc   );
eClrSpcLC   xClrSpcAppToClrSpc(eClrSpcApp         ClrSpcApp);

static inline bool isRGB         (eClrSpcApp ClrSpc) { return ClrSpc == eClrSpcApp::RGB || ClrSpc == eClrSpcApp::BGR || ClrSpc == eClrSpcApp::GBR; }
static inline bool isYCbCr       (eClrSpcApp ClrSpc) { return (int32)ClrSpc >= (int32)eClrSpcApp::YCbCr; }
static inline bool isDefinedYCbCr(eClrSpcApp ClrSpc) { return (int32)ClrSpc >  (int32)eClrSpcApp::YCbCr; }

//===============================================================================================================================================================================================================

} //end of namespace PMBB