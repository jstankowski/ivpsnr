/*
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefGSR.h"
#include "xProjectedCloud.h"

//portable implementation
#include "xGSR_ProjectorSTD.h"

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================
// xProjector - projects a range of Gaussians from world space to screen space, computes the 2D EWA covariance and evaluates spherical harmonic color
//===============================================================================================================================================================================================================
class xProjector
{
public:
  template<class tGauss> static inline void ProjectRng(xProjectedCloud* Projected, uint8* ValidMask, const tGauss* Cloud, const tCamPar& CamParams, int32V2 PicSize, uint32 BegIdx, uint32 EndIdx)
  {
    xProjectorSTD::ProjectRng(Projected, ValidMask, Cloud, CamParams, PicSize, BegIdx, EndIdx);
  }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB_NAMESPACE

