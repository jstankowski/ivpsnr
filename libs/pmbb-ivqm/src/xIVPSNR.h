/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xWSPSNR.h"
#include "xGlobClrDiff.h"
#include "xCorrespPixelShift.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xIVPSNR : public xWSPSNR, public xGlobClrDiffPrms, public xCorrespPixelShiftPrms
{
//debuging ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
public:
  using tCPS    = xCorrespPixelShift;
  using tDCfGCS = std::function<void(const int32V4&)>; //GCS = GlobalColorDiff
  using tDCfQAP = std::function<void(flt64, flt64)>;   //QAP = QualAsymmetricPic
protected:
  tDCfGCS m_DebugCallbackGCS;
  tDCfQAP m_DebugCallbackQAP;
public:
  void  setDebugCallbackGCS(tDCfGCS DebugCallbackGCS) { m_DebugCallbackGCS = DebugCallbackGCS; }
  void  setDebugCallbackQAP(tDCfQAP DebugCallbackQAP) { m_DebugCallbackQAP = DebugCallbackQAP; }

//IVPSNR 
public:
  flt64 calcPicIVPSNR(const xPicP* Tst, const xPicP* Ref, const xPicI* TstI = nullptr, const xPicI* RefI = nullptr);

  flt64 calcPicIVPSNR(const xPicP* Tst, const xPicP* Ref, const int32V4& GlobalColorDiffRef2Tst);
  flt64 calcPicIVPSNR(const xPicI* Tst, const xPicI* Ref, const int32V4& GlobalColorDiffRef2Tst);

protected:  
  flt64 xCalcQualAsymmetricPic(const xPicP* Tst, const xPicP* Ref, const int32V4& GlobalColorDiff); //asymetric Q planar
  flt64 xCalcQualAsymmetricPic(const xPicI* Tst, const xPicI* Ref, const int32V4& GlobalColorDiff); //asymetric Q interleaved
};

//===============================================================================================================================================================================================================

class xIVPSNRM : public xIVPSNR
{
public:
  flt64 calcPicIVPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const xPicI* TstI, const xPicI* RefI);

  flt64 calcPicIVPSNRM(const xPicI* Tst, const xPicI* Ref, const xPicP* Msk, int32 NumNonMasked, const int32V4& GlobalColorDiffRef2Tst);

protected:

  //asymetric Q interleaved
  flt64 xCalcQualAsymmetricPicM(const xPicI* Tst, const xPicI* Ref, const xPicP* Msk, const int32V4& GlobalColorDiff, const int32 NumNonMasked);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
