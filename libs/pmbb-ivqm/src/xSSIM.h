/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
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
  eMode   m_StrSimMode  = eMode::INVALID;
  bool    m_IsRegular   = false;
  eMrgExt m_MrgExtMode  = eMrgExt::INVALID;
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
  flt64 (*m_CalcPtr   ) (const uint16*, const uint16*,                int32, int32,        int32, flt64, flt64, bool) = nullptr; 
  tSSIMM(*m_CalcPtrMsk) (const uint16*, const uint16*, const uint16*, int32, int32, int32, int32, flt64, flt64, bool) = nullptr;

  //multi-block infrastructure
  int32 m_MultiBlockAvgBatchSize = NOT_VALID;
  int32 m_MultiBlockAvgBatchEndX = NOT_VALID;
  xStructSimMultiBlk::tCalcPtrMultiBlkAvgBatch* m_CalcPtrMultiBlkAvgBatch = nullptr;
  xStructSimMultiBlk::tCalcPtrMultiBlkAvgTail * m_CalcPtrMultiBlkAvgTail  = nullptr;

  //per-row partial results
  std::vector<flt64> m_RowSumsSSIM[4]; //TODO - probably single buffer is enough
  std::vector<flt64> m_RowSumsWght[4];

protected: //MSSSIM 
  int32  m_NumScales = NOT_VALID;
  xPicP* m_SubPicTst[c_NumMultiScales] = { nullptr };
  xPicP* m_SubPicRef[c_NumMultiScales] = { nullptr };
  xPicP* m_SubPicMsk[c_NumMultiScales] = { nullptr };

public:
  virtual void create (int32V2 PicSize, int32 BitDepth, int32 Margin, bool EnableMS, bool EnableMask);
  virtual void destroy();

  void setStructSimParams(eMode Mode, eMrgExt MarginMode, int32 BlockSize, int32 WndStride);

  flt64V4 calcPicSSIM   (const xPicP* Tst, const xPicP* Ref) { return xCalcPicSSIM(Tst, Ref, true); }
  flt64V4 calcPicMSSSIM (const xPicP* Tst, const xPicP* Ref);
  flt64V4 calcPicSSIMM  (const xPicP* Tst, const xPicP* Ref, const xPicP* Msk);
  flt64V4 calcPicMSSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk);

  void    visualizeSSIM(xPlane<uint16>* Vis, const xPicP* Tst, const xPicP* Ref, eCmp CmpId);

  static bool  isRegularMode(eMode Mode) { return Mode == eMode::RegularGaussianFlt || Mode == eMode::RegularGaussianInt || Mode == eMode::RegularAveraged; }
  static bool  isBlockMode  (eMode Mode) { return Mode == eMode::BlockAveraged      || Mode == eMode::BlockGaussianInt                                    ; }
  static int32 determineWindowSize(eMode Mode, int32 BlockSize) { return isRegularMode(Mode) ? 11 : BlockSize; }

protected:  
  void    xInitLoopRanges(int32 Width, int32 Height);

  flt64V4 xCalcPicSSIM(const xPicP* Tst, const xPicP* Ref,                            bool CalcL);
  flt64   xCalcCmpSSIM(const xPicP* Tst, const xPicP* Ref, eCmp CmpId,                bool CalcL);
  flt64   xCalcRowSSIM(const xPicP* Tst, const xPicP* Ref, eCmp CmpId, const int32 y, bool CalcL) const;
  flt64   xCalcRowSSIM_MB(const xPicP* Tst, const xPicP* Ref, eCmp CmpId, const int32 y, bool CalcL) const;

  flt64V4 xCalcPicSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk                           , bool CalcL);
  flt64   xCalcCmpSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, eCmp CmpId               , bool CalcL);
  tSSIMM  xCalcRowSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, eCmp CmpId, const int32 y, bool CalcL) const;

  void    xVisPicSSIM(xPlane<uint16>* Vis, const xPicP* Tst, const xPicP* Ref, eCmp CmpId, bool UseMin);

public:
  static int32 xCalcNumBlocks(int32 Length, int32 BlockSize, int32 BlockStride);
  static int32 xNumOverlappingBlocksInSize(int32 Length, int32 Log2BlockSize) { return (Length >> Log2BlockSize) + ((Length - xLog2SizeToSize(Log2BlockSize - 1)) >> Log2BlockSize); }
  static int32 xCalcNumScales(int32V2 PicSize);
  static void  xDownsamplePic(xPicP* Dst, const xPicP* Src);
  static void  xDownsampleMsk(xPicP* DstMsk, const xPicP* SrcMsk);
  static void  xDownsamplePicWght(xPicP* DstPic, const xPicP* SrcPic, const xPicP* SrcMsk);
  static void  xDownsampleWeightedHV(uint16* restrict Dst, const uint16* Src, const uint16* Wgh, int32 DstStride, int32 SrcStride, int32 WghStride, int32 DstWidth, int32 DstHeight);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB