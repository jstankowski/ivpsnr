/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefIVQM.h"
#include "xPic.h"
#include "xPlane.h"
#include "xMetricCommon.h"
#include "xStructSim.h"
#include "xWeightedSpherically.h"
#include "xGlobClrDiff.h"
#include "xShftCompPic.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xSSIM : public xMetricCommon, public xStructSimConsts, public xWeightedSpherically
{
public:
  using fltTP  = flt64;
  using tFltrF = xStructSim<fltTP, true>::tFltrF;

protected:
  int32V2 m_Size        = { NOT_VALID, NOT_VALID };
  int32   m_BitDepth    = NOT_VALID;
  fltTP   m_C1          = std::numeric_limits<fltTP>::quiet_NaN();
  fltTP   m_C2          = std::numeric_limits<fltTP>::quiet_NaN();

  std::vector<flt64> m_RowSums[4];

protected: //MSSSIM 
  xPicP* m_SubPicTst[c_NumMultiScales] = { nullptr };
  xPicP* m_SubPicRef[c_NumMultiScales] = { nullptr };

public:
  virtual void create (int32V2 Size, int32 BitDepth, int32 Margin, bool EnableMS);
  virtual void destroy();

  flt64V4 calcPicSSIM  (const xPicP* Tst, const xPicP* Ref, bool CalcL = true);
  flt64V4 calcPicMSSSIM(const xPicP* Tst, const xPicP* Ref);

protected:  
  flt64   xCalcCmpSSIM(const xPicP* Tst, const xPicP* Ref, eCmp CmpId, bool CalcL);
  flt64   xCalcRowSSIM(const xPicP* Tst, const xPicP* Ref, eCmp CmpId, const int32 y, bool CalcL);

  static void xDownsamplePic(xPicP* Dst, const xPicP* Src);
};

//===============================================================================================================================================================================================================

class xIVSSIM : public xSSIM, public xGlobClrDiffPrms, public xCorrespPixelShiftPrms
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
  virtual void create (int32V2 Size, int32 BitDepth, int32 Margin, bool EnableMS);
  virtual void destroy();

  flt64 calcPicIVSSIM  (const xPicP* Tst, const xPicP* Ref);
  flt64 calcPicIVSSIM  (const xPicP* Tst, const xPicP* Ref, const xPicP* TstSCP, const xPicP* RefSCP);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB