/*
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xCommonDefHASH.h"
#include "xPic.h"
#include "xXXH.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xPicHash
{
public:
  static xXXH128::tHash CalcPicHash(const xPicP& Pic);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB