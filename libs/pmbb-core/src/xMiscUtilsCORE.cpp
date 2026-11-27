/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xMiscUtilsCORE.h"
#include "xTimeUtils.h"
#include <type_traits>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

#if X_PMBB_CPP20
#  define IS_CONSTEVAL std::is_constant_evaluated()
#else //X_PMBB_CPP20
#  if defined(__clang__) || defined(__GNUC__)
#    define IS_CONSTEVAL __builtin_is_constant_evaluated()
#  elif defined(_MSC_VER)
#    define IS_CONSTEVAL __builtin_is_constant_evaluated()
#  else
#    define IS_CONSTEVAL false // Fallback if unsupported
#  endif
#endif //X_PMBB_CPP20

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
  static constexpr uint32_t xHash(std::string_view Str) //based on CRC32C
  {
    //if(IS_CONSTEVAL)
    //{
    //  std::string_view TmpStr = xStrip(Str);
    //
    //}
    //else
    {
      uint32_t CRC = 0xffffffff;
      for(char C : Str)
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
         CrF_U=="CF400" || CrF_U=="400" ? eCrF::CF400 :
                                          eCrF::INVALID;
}
std::string xCrF2Str(eCrF CrF)
{
  switch(CrF)
  {
  case eCrF::CF444  : return "CF444"  ;
  case eCrF::CF422  : return "CF422"  ;
  case eCrF::CF420  : return "CF420"  ;
  case eCrF::CF400  : return "CF400"  ;
  case eCrF::UNKNOWN: return "UNKNOWN";
  default           : return "INVALID";
  }
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
  switch(ImgTp)
  {
  case eImgTp::YCbCr  : return "YCbCr"  ;
//case eImgTp::YCbCrA : return "YCbCrA" ;
//case eImgTp::YCbCrD : return "YCbCrD" ;
  case eImgTp::RGB    : return "RGB"    ;
  case eImgTp::BGR    : return "BGR"    ;
  case eImgTp::GBR    : return "GBR"    ;
//case eImgTp::Bayer  : return "Bayer"  ;
//case eImgTp::UNKNOWN: return "UNKNOWN";
  default             : return "INVALID";
  }
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
  switch(ClrSpc)
  {
  case eClrSpcLC::BT601    : return "BT601 / SMPTE170M / JPEG";
  case eClrSpcLC::BT709    : return "BT709"                   ;
  case eClrSpcLC::SMPTE240M: return "SMPTE240M"               ;
  case eClrSpcLC::BT2020   : return "BT2020"                  ;
  case eClrSpcLC::JPEG2000 : return "JPEG2000"                ;
  case eClrSpcLC::YCoCg    : return "YCoCg"                   ;
  case eClrSpcLC::YCoCgR   : return "YCoCgR"                  ;
  default                  : return "INVALID"                 ;
  }
}

eMrgExt xStr2MrgExt(const std::string& MrgExt)
{
  const uint32 MrgExtH = xSwitchUtils::xHash(MrgExt);
  switch(MrgExtH)
  {
  case xSwitchUtils::xHash("None    "): return eMrgExt::None    ; break;
  case xSwitchUtils::xHash("Nearest "): return eMrgExt::Nearest ; break;
  case xSwitchUtils::xHash("Reflect "): return eMrgExt::Reflect ; break;
  case xSwitchUtils::xHash("Mirror  "): return eMrgExt::Mirror  ; break;
  case xSwitchUtils::xHash("Constant"): return eMrgExt::Constant; break;
  case xSwitchUtils::xHash("Zero    "): return eMrgExt::Zero    ; break;
  default                             : return eMrgExt::INVALID ; break;
  }
}
std::string xMrgExt2Str(eMrgExt MrgExt)
{
  switch(MrgExt)
  {
  case eMrgExt::INVALID  : return "INVALID"  ; break;
  case eMrgExt::None     : return "None"     ; break;
  case eMrgExt::Nearest  : return "Edge"     ; break;
  case eMrgExt::Reflect  : return "Symmetric"; break;
  case eMrgExt::Mirror   : return "Reflect"  ; break;
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
  switch(IPA)
  {
  case eActn::SKIP: return "SKIP"   ; 
  case eActn::WARN: return "WARN"   ; 
  case eActn::STOP: return "STOP"   ; 
  case eActn::CNCL: return "CNCL"   ; 
  default         : return "INVALID";
  }
}

eFileFmt xStr2FileFmt(const std::string& FileFmt)
{
  std::string FileFmtU = xString::toUpper(FileFmt);
  return FileFmt=="RAW" ? eFileFmt::RAW    :
         FileFmt=="PNG" ? eFileFmt::PNG    :
         FileFmt=="BMP" ? eFileFmt::BMP    :
                          eFileFmt::INVALID;
}
std::string xFileFmt2Str(eFileFmt FileFmt)
{
  switch(FileFmt)
  {
  case eFileFmt::RAW: return "RAW"    ;
  case eFileFmt::PNG: return "PNG"    ;
  case eFileFmt::BMP: return "BMP"    ;
  default           : return "INVALID";
  }
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
#if defined(X_PMBB_ARCH_ARM64)
    Str += fmt::format("SIMD_CAN_USE_NEON      = {:d}\n", X_SIMD_CAN_USE_NEON      );
    Str += fmt::format("PMBB_CAN_USE_CRC32     = {:d}\n", X_PMBB_CAN_USE_ARM_CRC32 );
    Str += fmt::format("PMBB_CAN_USE_CRYPTO    = {:d}\n", X_PMBB_CAN_USE_ARM_CRYPTO);
#endif
  }
  Str += fmt::format("TSC_IMPLEMENTATION     = {}\n", X_IMPLEMENTATION_TSC);
  return Str;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
