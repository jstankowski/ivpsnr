/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefCORE.h"
#include "xString.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// Enums
//===============================================================================================================================================================================================================
eCrF        xStr2CrF(const std::string& CrF);
std::string xCrF2Str(eCrF CrF);

eImgTp      xStr2ImgTp(const std::string& ImgTp);
std::string xImgTp2Str(eImgTp ImgTp);

eClrSpcLC   xStr2ClrSpcLC(const std::string& ClrSpcLC);
std::string xClrSpcLC2Str(eClrSpcLC ClrSpc);

eActn       xStr2Actn(const std::string& Actn);
std::string xActn2Str(eActn IPA);

//===============================================================================================================================================================================================================

class xMiscUtilsCORE
{
public:
  static std::string formatCompileTimeSetup();

};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
