/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefCORE.h"
#include "xGaussianCloud.h"
#include "xXXH.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

static inline xXXH128::tHash CalcCloudHash(const FVGS::xGaussianCloudI* Cloud)
{
  return xXXH128::calc((uint8*)(Cloud->getGaussianObjects()), Cloud->getNumValidSplats() * sizeof(FVGS::xGaussianObject));
}
static inline int32 xMirrorFrameIdx(int32 PoseIdx, int32 NumCloudFrames)
{
  if(NumCloudFrames <= 1) { return 0; }
  const int32 Period = 2 * (NumCloudFrames - 1);
  const int32 Mod    = PoseIdx % Period;
  return (Mod < NumCloudFrames) ? Mod : (Period - Mod);
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
