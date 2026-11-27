/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
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
#if X_PMBB_STALLED
  //(corresponding pixel) Shift Compensated Pictures (SCP)
  xPicP* m_PicSCPDownsampledTst[c_NumMultiScales] = { nullptr };
  xPicP* m_PicSCPDownsampledRef[c_NumMultiScales] = { nullptr };
#endif //X_PMBB_STALLED

public:
  virtual void create (int32V2 Size, int32 BitDepth, int32 Margin, bool EnableMS, bool EnableMask);
  virtual void destroy();

  flt64 calcPicIVSSIM  (const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP);
  flt64 calcPicIVMSSSIM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP);
#if X_PMBB_STALLED
  flt64 calcPicMSIVSSIM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP);
#endif //X_PMBB_STALLED

  //mask mode
  flt64 calcPicIVSSIMM  (const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP, const xPicP* Msk);
  flt64 calcPicIVMSSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP, const xPicP* Msk);

  //visualization
  void  visualizeIVSSIM(xPlane<uint16>* Vis, const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP, eCmp CmpId);

protected:
  flt64V4 xCalcPicSSIM(const xPicP* Tst, const xPicP* Ref,             bool UseWS, bool CalcL);
  flt64   xCalcCmpSSIM(const xPicP* Tst, const xPicP* Ref, eCmp CmpId, bool UseWS, bool CalcL);

  flt64V4 xCalcPicMSSSIM(const xPicP* Tst, const xPicP* Ref);

  flt64V4 xCalcPicSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk,             bool UseWS, bool CalcL);
  flt64   xCalcCmpSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, eCmp CmpId, bool UseWS, bool CalcL);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB