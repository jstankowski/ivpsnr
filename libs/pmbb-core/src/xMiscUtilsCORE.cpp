/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xMiscUtilsCORE.h"
#include "xTimeUtils.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xSwitchUtils
{
public:
  static constexpr std::string_view xStrip(std::string_view Str)
  {
    int32 Beg = 0;
    int32 End = (int32)Str.size()-1;
    for(int32 i = 0; i < (int32)Str.size(); i++)
    {
      if(Str[i] != ' ' && Str[i] != '\t') { Beg = i; break; }
    }
    for(int32 i = (int32)Str.size() - 1; i >= 0; i--)
    {
      if(Str[i] != ' ' && Str[i] != '\t') { End = i; break; }
    }
    int32 Len = End - Beg + 1;
    return Str.substr(Beg, Len);
  }
  static constexpr uint32_t xHash(std::string_view str) //based on CRC32C
  {
    uint32_t CRC = 0xffffffff;
    for(char C : str)
    {
      if(C != ' ' && C != '\t')
      {
        char c = (C >= 'A' && C <= 'Z') ? C + ('a' - 'A') : C;
        CRC ^= c;
        for(int32 j = 7; j >= 0; j--)
        {
          CRC = (CRC >> 1) ^ ((CRC & 1) ? 0x82F63B78 : 0);
        }
      }
    }
    return CRC ^ 0xffffffff;
  }
};

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
       //ImgTpU=="YCbCrA" ? eImgTp::YCbCrA :
       //ImgTpU=="YCbCrD" ? eImgTp::YCbCrD :
         ImgTpU=="RGB"    ? eImgTp::RGB    :
         ImgTpU=="BGR"    ? eImgTp::BGR    :
         ImgTpU=="GBR"    ? eImgTp::GBR    :
       //ImgTpU=="Bayer"  ? eImgTp::Bayer  :
                            eImgTp::INVALID;
}
std::string xImgTp2Str(eImgTp ImgTp)
{
  return ImgTp==eImgTp::YCbCr   ? "YCbCr"  :
       //ImgTp==eImgTp::YCbCrA  ? "YCbCrA" :
       //ImgTp==eImgTp::YCbCrD  ? "YCbCrD" :
         ImgTp==eImgTp::RGB     ? "RGB"    :
         ImgTp==eImgTp::BGR     ? "BGR"    :
         ImgTp==eImgTp::GBR     ? "GBR"    :
       //ImgTp==eImgTp::Bayer   ? "Bayer"  :
       //ImgTp==eImgTp::UNKNOWN ? "UNKNOWN":
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

eMrgExt xStr2MrgExt(const std::string& MrgExt)
{
  const uint32 MrgExtH = xSwitchUtils::xHash(MrgExt);
  switch(MrgExtH)
  {
  case xSwitchUtils::xHash("None     "): return eMrgExt::None     ; break;
  case xSwitchUtils::xHash("Edge     "): return eMrgExt::Edge     ; break;
  case xSwitchUtils::xHash("Symmetric"): return eMrgExt::Symmetric; break;
  case xSwitchUtils::xHash("Reflect  "): return eMrgExt::Reflect  ; break;
  case xSwitchUtils::xHash("Constant "): return eMrgExt::Constant ; break;
  case xSwitchUtils::xHash("Zero     "): return eMrgExt::Zero     ; break;
  default                              : return eMrgExt::INVALID  ; break;
  }
}
std::string xMrgExt2Str(eMrgExt MrgExt)
{
  switch(MrgExt)
  {
  case eMrgExt::INVALID  : return "INVALID"  ; break;
  case eMrgExt::None     : return "None"     ; break;
  case eMrgExt::Edge     : return "Edge"     ; break;
  case eMrgExt::Symmetric: return "Symmetric"; break;
  case eMrgExt::Reflect  : return "Reflect"  ; break;
  case eMrgExt::Constant : return "Constant" ; break;
  case eMrgExt::Zero     : return "Zero"     ; break;
  default                : return "UNDEFINED"; break;
  }
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

eFileFmt xStr2FileFmt(const std::string& FileFmt)
{
  std::string FileFmtU = xString::toUpper(FileFmt);
  return FileFmt=="RAW" ? eFileFmt::RAW    :
         FileFmt=="PNG" ? eFileFmt::PNG    :
                          eFileFmt::INVALID;
}
std::string xFileFmt2Str(eFileFmt FileFmt)
{
  return FileFmt==eFileFmt::RAW ? "RAW"    :
         FileFmt==eFileFmt::PNG ? "PNG"    :
                                  "INVALID";
}

//===============================================================================================================================================================================================================

std::string xMiscUtilsCORE::formatCompileTimeSetup()
{
  std::string Str;
  Str += "Compile-time configuration:\n";
  Str += fmt::format("USE_SIMD               = {:d}\n", PMBB_USE_SIMD);
  if(PMBB_USE_SIMD)
  {
#if defined(X_PMBB_ARCH_AMD64)
    Str += fmt::format("SIMD_CAN_USE_SSE       = {:d}\n", X_SIMD_CAN_USE_SSE   );
    Str += fmt::format("SIMD_CAN_USE_AVX       = {:d}\n", X_SIMD_CAN_USE_AVX   );
    Str += fmt::format("SIMD_CAN_USE_AVX512    = {:d}\n", X_SIMD_CAN_USE_AVX512);
#endif
  }
  Str += fmt::format("TSC_IMPLEMENTATION     = {}\n", X_TSC_IMPLEMENTATION);
  return Str;
}
std::string xMiscUtilsCORE::formatBuildInfo()
{
  std::string Str;
  Str += "Build and target configuration:\n";
  Str += fmt::format("TARGET_OS_NAME   = {}\n", X_PMBB_OPERATING_SYSTEM_NAME);
  Str += fmt::format("TARGET_ARCH_NAME = {}\n", X_PMBB_ARCH_NAME            );
  Str += fmt::format("COMPILER_NAME    = {}\n", X_PMBB_COMPILER_NAME        );
  Str += fmt::format("COMPILER_VERSION = {}\n", X_PMBB_COMPILER_VER         );
  Str += fmt::format("CPP_VERSION      = {}\n", X_PMBB_CPUSPLUS_VER         );
  Str += fmt::format("BUILD_TIME       = {} {}\n", __DATE__, __TIME__       );
  return Str;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
