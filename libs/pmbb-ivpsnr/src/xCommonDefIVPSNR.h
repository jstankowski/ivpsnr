/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

//=============================================================================================================================================================================
// PMBB-core
//=============================================================================================================================================================================
#include "xCommonDefPMBB-CORE.h"

namespace PMBB_NAMESPACE {

//=============================================================================================================================================================================
// Compile time settings
//=============================================================================================================================================================================
static constexpr bool xc_USE_KBNS               = true; // use Kahan-Babuška-Neumaier floating point sumation algorithm
static constexpr bool xc_USE_RUNTIME_CMPWEIGHTS = true; // use component weights provided at runtime

//=============================================================================================================================================================================

} //end of namespace PMBB
