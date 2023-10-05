/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xWSPSNR.h"
#include "xGlobalColorShift.h"
#include "xCorrespPixelShift.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xIVPSNR : public xWSPSNR, public xGlobalColorShift, public xCorrespPixelShift
{
//debuging ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
public:  
  using tDCfGCS = std::function<void(const int32V4&)>; //GCS = GlobalColorShift
  using tDCfQAP = std::function<void(flt64, flt64)>;   //QAP = QualAsymmetricPic
protected:
  tDCfGCS m_DebugCallbackGCS;
  tDCfQAP m_DebugCallbackQAP;
public:
  void  setDebugCallbackGCS(tDCfGCS DebugCallbackGCS) { m_DebugCallbackGCS = DebugCallbackGCS; }
  void  setDebugCallbackQAP(tDCfQAP DebugCallbackQAP) { m_DebugCallbackQAP = DebugCallbackQAP; }

//IVPSNR 
public:
  flt64 calcPicIVPSNR(const xPicP* Ref, const xPicP* Tst, const xPicI* RefI = nullptr, const xPicI* TstI = nullptr);

protected:  
  flt64 xCalcQualAsymmetricPic(const xPicP* Ref, const xPicP* Tst, const int32V4& GlobalColorShift); //asymetric Q planar
  flt64 xCalcQualAsymmetricPic(const xPicI* Ref, const xPicI* Tst, const int32V4& GlobalColorShift); //asymetric Q interleaved
};

//===============================================================================================================================================================================================================

class xIVPSNRM : public xIVPSNR
{
public:
  flt64 calcPicIVPSNRM (const xPicP* Ref, const xPicP* Tst, const xPicP* Mask, const xPicI* RefI, const xPicI* TstI);

protected:

  //asymetric Q interleaved
  flt64 xCalcQualAsymmetricPicM(const xPicI* Ref, const xPicI* Tst, const xPicP* Msk, const int32V4& GlobalColorShift, const int32 NumNonMasked);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
