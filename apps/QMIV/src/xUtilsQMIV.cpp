/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xUtilsQMIV.h"
#include "xString.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

eClrSpcApp xStr2ClrSpcApp(const std::string& ClrSpc)
{
  std::string ClrSpcU = xString::toUpper(ClrSpc);
  return ClrSpc=="RGB"             ? eClrSpcApp::RGB             :
         ClrSpc=="BGR"             ? eClrSpcApp::BGR             :
         ClrSpc=="GBR"             ? eClrSpcApp::GBR             :
         ClrSpc=="YCbCr"           ? eClrSpcApp::YCbCr           :
         ClrSpc=="YCbCr_BT601"     ? eClrSpcApp::YCbCr_BT601     :
         ClrSpc=="YCbCr_SMPTE170M" ? eClrSpcApp::YCbCr_SMPTE170M :
         ClrSpc=="YCbCr_BT709"     ? eClrSpcApp::YCbCr_BT709     :
         ClrSpc=="YCbCr_SMPTE240M" ? eClrSpcApp::YCbCr_SMPTE240M :
         ClrSpc=="YCbCr_BT2020"    ? eClrSpcApp::YCbCr_BT2020    :
                                     eClrSpcApp::INVALID;
}
std::string xClrSpcApp2Str(eClrSpcApp ClrSpc)
{
  return ClrSpc==eClrSpcApp::RGB             ? "RGB"             :
         ClrSpc==eClrSpcApp::BGR             ? "BGR"             :
         ClrSpc==eClrSpcApp::GBR             ? "GBR"             :
         ClrSpc==eClrSpcApp::YCbCr           ? "YCbCr"           :
         ClrSpc==eClrSpcApp::YCbCr_BT601     ? "YCbCr_BT601"     :
         ClrSpc==eClrSpcApp::YCbCr_SMPTE170M ? "YCbCr_SMPTE170M" :
         ClrSpc==eClrSpcApp::YCbCr_BT709     ? "YCbCr_BT709"     :
         ClrSpc==eClrSpcApp::YCbCr_SMPTE240M ? "YCbCr_SMPTE240M" :
         ClrSpc==eClrSpcApp::YCbCr_BT2020    ? "YCbCr_BT2020"    :
                                               "INVALID"         ;
}

eClrSpcLC xClrSpcAppToClrSpc(eClrSpcApp ClrSpcApp)
{
  return ClrSpcApp==eClrSpcApp::YCbCr_BT601     ? eClrSpcLC::BT601     :
         ClrSpcApp==eClrSpcApp::YCbCr_SMPTE170M ? eClrSpcLC::SMPTE170M :
         ClrSpcApp==eClrSpcApp::YCbCr_BT709     ? eClrSpcLC::BT709     :
         ClrSpcApp==eClrSpcApp::YCbCr_SMPTE240M ? eClrSpcLC::SMPTE240M :
         ClrSpcApp==eClrSpcApp::YCbCr_BT2020    ? eClrSpcLC::BT2020    :
                                                  eClrSpcLC::INVALID   ;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB