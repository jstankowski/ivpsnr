/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefIVQM.h"
#include "xSSIM.h"
#include "xGlobClrDiff.h"
#include "xShftCompPic.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xIVSSIM : public xSSIM, public xGlobClrDiffPrms, public xCorrespPixelShiftPrms, public xWeightedSpherically
{
public:  
  using tDCfGCS = std::function<void(const int32V4&)>; //GCS = GlobalColorShift
  using tDCfQAP = std::function<void(flt64, flt64)>;   //QAP = QualAsymmetricPic
protected:
  tDCfGCS m_DebugCallbackGCS;
  tDCfQAP m_DebugCallbackQAP;
public:
  void  setDebugCallbackGCS(tDCfGCS DebugCallbackGCS) { m_DebugCallbackGCS = DebugCallbackGCS; }
  void  setDebugCallbackQAP(tDCfQAP DebugCallbackQAP) { m_DebugCallbackQAP = DebugCallbackQAP; }

//IVSSIM
protected:
  //(corresponding pixel) Shift Compensated Pictures (SCP)
  xPicP* m_TstSCP = nullptr; 
  xPicP* m_RefSCP = nullptr;

public:
  virtual void create (int32V2 Size, int32 BitDepth, int32 Margin, bool EnableMS, bool PreAllocateSCP = false);
  virtual void destroy();

  flt64 calcPicIVSSIM  (const xPicP* Tst, const xPicP* Ref);
  flt64 calcPicIVSSIM  (const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP);
  flt64 calcPicIVMSSSIM(const xPicP* Tst, const xPicP* Ref);
  flt64 calcPicIVMSSSIM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP);

  flt64 calcPicIVSSIMM (const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP, const xPicP* Msk, int32 NumNonMasked = NOT_VALID);


protected:
  flt64V4 xCalcPicSSIM(const xPicP* Tst, const xPicP* Ref,             bool UseWS, bool CalcL);
  flt64   xCalcCmpSSIM(const xPicP* Tst, const xPicP* Ref, eCmp CmpId, bool UseWS, bool CalcL);

  flt64V4 xCalcPicMSSSIM(const xPicP* Tst, const xPicP* Ref);

  flt64V4 xCalcPicSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk,             int32 NumNonMasked, bool UseWS, bool CalcL);
  flt64   xCalcCmpSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, eCmp CmpId, int32 NumNonMasked, bool UseWS, bool CalcL);

};

//===============================================================================================================================================================================================================

} //end of namespace PMBB