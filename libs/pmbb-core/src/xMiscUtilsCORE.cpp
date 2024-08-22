/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xMiscUtilsCORE.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// Enums
//===============================================================================================================================================================================================================
eCrF xStr2CrF(const std::string& CrF)
{
  std::string CrF_U = xString::toUpper(CrF);
  return CrF_U=="CF444" || CrF_U=="444" ? eCrF::CF444 :
         CrF_U=="CF422" || CrF_U=="422" ? eCrF::CF422 :
         CrF_U=="CF420" || CrF_U=="420" ? eCrF::CF420 :
         CrF_U=="CF400" || CrF_U=="400" ? eCrF::CF420 :
                                          eCrF::INVALID;
}
std::string xCrF2Str(eCrF CrF)
{
  return CrF==eCrF::CF444   ? "CF444" :
         CrF==eCrF::CF422   ? "CF422" :
         CrF==eCrF::CF420   ? "CF420" :
         CrF==eCrF::CF420   ? "CF400" :
         CrF==eCrF::UNKNOWN ? "UNKNOWN":
                              "INVALID";
}
eImgTp xStr2ImgTp(const std::string& ImgTp)
{
  std::string ImgTpU = xString::toUpper(ImgTp);
  return ImgTpU=="YCbCr"  ? eImgTp::YCbCr  :
         ImgTpU=="YCbCrA" ? eImgTp::YCbCrA :
         ImgTpU=="YCbCrD" ? eImgTp::YCbCrD :
         ImgTpU=="RGB"    ? eImgTp::RGB    :
         ImgTpU=="BGR"    ? eImgTp::BGR    :
         ImgTpU=="Bayer"  ? eImgTp::Bayer  :
                            eImgTp::INVALID;
}
std::string xImgTp2Str(eImgTp ImgTp)
{
  return ImgTp==eImgTp::YCbCr   ? "YCbCr"  :
         ImgTp==eImgTp::YCbCrA  ? "YCbCrA" :
         ImgTp==eImgTp::YCbCrD  ? "YCbCrD" :
         ImgTp==eImgTp::RGB     ? "RGB"    :
         ImgTp==eImgTp::BGR     ? "BGR"    :
         ImgTp==eImgTp::Bayer   ? "Bayer"  :
         ImgTp==eImgTp::UNKNOWN ? "UNKNOWN":
                                  "INVALID";
}
eClrSpcLC xStr2ClrSpcLC(const std::string& ClrSpc)
{
  std::string ClrSpcU = xString::toUpper(ClrSpc);
  return ClrSpc=="BT601"     ? eClrSpcLC::BT601     :
         ClrSpc=="SMPTE170M" ? eClrSpcLC::SMPTE170M :
         ClrSpc=="BT709"     ? eClrSpcLC::BT709     :
         ClrSpc=="SMPTE240M" ? eClrSpcLC::SMPTE240M :
         ClrSpc=="BT2020"    ? eClrSpcLC::BT2020    :
         ClrSpc=="JPEG2000"  ? eClrSpcLC::JPEG2000  :
         ClrSpc=="YCoCg"     ? eClrSpcLC::YCoCg     :
         ClrSpc=="YCoCgR"    ? eClrSpcLC::YCoCgR    :
                               eClrSpcLC::INVALID;
}
std::string xClrSpcLC2Str(eClrSpcLC ClrSpc)
{
  return ClrSpc==eClrSpcLC::BT601     ? "BT601"     :
         ClrSpc==eClrSpcLC::SMPTE170M ? "SMPTE170M" :
         ClrSpc==eClrSpcLC::BT709     ? "BT709"     :
         ClrSpc==eClrSpcLC::SMPTE240M ? "SMPTE240M" :
         ClrSpc==eClrSpcLC::BT2020    ? "BT2020"    :
         ClrSpc==eClrSpcLC::JPEG2000  ? "JPEG2000"  :
         ClrSpc==eClrSpcLC::YCoCg     ? "YCoCg"     :
         ClrSpc==eClrSpcLC::YCoCgR    ? "YCoCgR"    :
                                        "INVALID"   ;
}
eActn xStr2Actn(const std::string& Actn)
{
  std::string IPA_U = xString::toUpper(Actn);
  return IPA_U == "SKIP" ? eActn::SKIP :
         IPA_U == "WARN" ? eActn::WARN :
         IPA_U == "STOP" ? eActn::STOP :
         IPA_U == "CNCL" ? eActn::CNCL :
                           eActn::INVALID;
}
std::string xActn2Str(eActn IPA)
{
  return IPA == eActn::SKIP ? "SKIP" :
         IPA == eActn::WARN ? "WARN" :
         IPA == eActn::STOP ? "STOP" :
         IPA == eActn::CNCL ? "CNCL" :
                              "INVALID";
}

//===============================================================================================================================================================================================================

std::string xMiscUtilsCORE::formatCompileTimeSetup()
{
  std::string Str;
  Str += "Compile-time configuration:\n";
  Str += fmt::format("USE_SIMD               = {:d}\n", USE_SIMD);
  if(USE_SIMD)
  {
    Str += fmt::format("SIMD_CAN_USE_SSE       = {:d}\n", X_SIMD_CAN_USE_SSE);
    Str += fmt::format("SIMD_CAN_USE_AVX       = {:d}\n", X_SIMD_CAN_USE_AVX);
    Str += fmt::format("SIMD_CAN_USE_AVX512    = {:d}\n", X_SIMD_CAN_USE_AVX512);
  }
  Str += fmt::format("TSC_IMPLEMENTATION     = {}\n", X_TSC_IMPLEMENTATION);
  return Str;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
