/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefIVQM.h"
#include "xPic.h"
#include "xPlane.h"
#include "xMetricCommon.h"
#include "xStructSim.h"
#include "xStructSimConsts.h"
#include "xWeightedSpherically.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xSSIM : public xMetricCommon, public xStructSimConsts
{
public:
  enum class eMode // struct mode
  {
    INVALID            = NOT_VALID,
    RegularGaussianFlt = 0,
    RegularGaussianInt = 1,
    RegularAveraged    = 2,
    BlockGaussianInt   = 3,
    BlockAveraged      = 4,
  };
  static eMode       xStrToMode(const std::string& Mode);
  static std::string xModeToStr(eMode              Mode);

  static constexpr eMode c_DefaultStructSimMode   = eMode::BlockAveraged;
  static constexpr int32 c_DefaultStructSimStride = 4;
  static constexpr int32 c_DefaultStructSimWindow = 8;

protected:
  int32V2 m_PicSize     = { NOT_VALID, NOT_VALID };
  int32   m_BitDepth    = NOT_VALID;
  eMode   m_Mode        = eMode::INVALID;
  bool    m_IsRegular   = false;
  bool    m_UseMargin   = false;
  int32   m_WndSize     = NOT_VALID; //window size
  int32   m_WndStride   = NOT_VALID; //window stride

  int32   m_LoopBegY    = NOT_VALID;
  int32   m_LoopEndY    = NOT_VALID;
  int32   m_NumUnitY    = NOT_VALID;
  int32   m_LoopBegX    = NOT_VALID;
  int32   m_LoopEndX    = NOT_VALID;
  int32   m_NumUnitX    = NOT_VALID;

  flt64   m_C1        = std::numeric_limits<flt64>::quiet_NaN();
  flt64   m_C2        = std::numeric_limits<flt64>::quiet_NaN();

  //function pointer
  flt64(*m_CalcPtr   ) (const uint16*, const uint16*, int32, int32, int32, flt64, flt64, bool) = nullptr;
  flt64(*m_CalcPtrMsk) (const uint16*, const uint16*, const uint16*, int32, int32, int32, int32, flt64, flt64, bool) = nullptr;

  std::vector<flt64> m_RowSums[4];

protected: //MSSSIM 
  int32  m_NumScales = NOT_VALID;
  xPicP* m_SubPicTst[c_NumMultiScales] = { nullptr };
  xPicP* m_SubPicRef[c_NumMultiScales] = { nullptr };

public:
  virtual void create (int32V2 PicSize, int32 BitDepth, int32 Margin, bool EnableMS);
  virtual void destroy();

  void setStructSimParams(eMode Mode, bool UseMargin, int32 BlockSize, int32 WndStride);

  flt64V4 calcPicSSIM  (const xPicP* Tst, const xPicP* Ref) { return xCalcPicSSIM(Tst, Ref, true); }
  flt64V4 calcPicMSSSIM(const xPicP* Tst, const xPicP* Ref);
  flt64V4 calcPicSSIMM (const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, int32 NumNonMasked = NOT_VALID);

  static bool  isRegularMode(eMode Mode) { return Mode == eMode::RegularGaussianFlt || Mode == eMode::RegularGaussianInt || Mode == eMode::RegularAveraged; }
  static int32 determineWindowSize(eMode Mode, int32 BlockSize) { return isRegularMode(Mode) ? 11 : BlockSize; }

protected:  
  void    xInitLoopRanges(int32 Width, int32 Height);

  flt64V4 xCalcPicSSIM(const xPicP* Tst, const xPicP* Ref,                            bool CalcL);
  flt64   xCalcCmpSSIM(const xPicP* Tst, const xPicP* Ref, eCmp CmpId,                bool CalcL);
  flt64   xCalcRowSSIM(const xPicP* Tst, const xPicP* Ref, eCmp CmpId, const int32 y, bool CalcL) const;

  flt64V4 xCalcPicSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk,             int32 NumNonMasked, bool CalcL);
  flt64   xCalcCmpSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, eCmp CmpId, int32 NumNonMasked, bool CalcL);
  flt64   xCalcRowSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, eCmp CmpId, const int32 y     , bool CalcL) const;

  static int32 xCalcNumBlocks(int32 Length, int32 BlockSize, int32 BlockStride);
  static int32 xNumOverlappingBlocksInSize(int32 Length, int32 Log2BlockSize) { return (Length >> Log2BlockSize) + ((Length - xLog2SizeToSize(Log2BlockSize - 1)) >> Log2BlockSize); }
  static int32 xCalcNumScales(int32V2 PicSize);
  static void  xDownsamplePic(xPicP* Dst, const xPicP* Src);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB