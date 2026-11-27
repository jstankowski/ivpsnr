/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xWSPSNR.h"
#include "xGlobClrDiff.h"
#include "xCorrespPixelShiftPrms.h"
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


public: //IVPSNR 
  flt64V2 calcPicIVPSNR (const xPicP* Tst, const xPicP* Ref);
  flt64V2 calcPicIVPSNR (const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP);
  flt64V2 calcPicIVPSNRM(const xPicP* Tst, const xPicP* Ref                                          , const xPicP* Msk, int32 NumNonMasked = NOT_VALID);
  flt64V2 calcPicIVPSNRM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP, const xPicP* Msk, int32 NumNonMasked);


protected: //asymetric Q  
  flt64V2 xCalcQualAsymmetricPic(const xPicP* Tst, const xPicP* RefSCP);
  flt64   xCalcDistAsymmetricCmp(const xPicP* Tst, const xPicP* RefSCP, eCmp CmpId);
  flt64V2 xCalcQualAsymmetricPic(const xPicP* Tst, const xPicP* Ref, const int32V4& GlobalColorDiff); 
  void    xCalcDistAsymmetricRng(const xPicP* Tst, const xPicP* Ref, const int32V4& GlobalColorDiff, int32 BegY, int32 EndY); 


  flt64V2 xCalcQualAsymmetricPicM(const xPicP* Tst, const xPicP* RefSCP, const xPicP* Msk, int32 NumNonMasked);
  flt64   xCalcDistAsymmetricCmpM(const xPicP* Tst, const xPicP* RefSCP, const xPicP* Msk, eCmp CmpId);
  flt64V2 xCalcQualAsymmetricPicM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32V4& GlobalColorDiff, const int32 NumNonMasked);
  void    xCalcDistAsymmetricRngM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, const int32V4& GlobalColorDiff, int32 BegY, int32 EndY);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
