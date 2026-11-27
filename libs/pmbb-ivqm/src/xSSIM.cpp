/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xSSIM.h"
#include "xKBNS.h"
#include "xPixelOps.h"
#include "xString.h"
#include "xHelpersFLT.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xSSIM
//===============================================================================================================================================================================================================
xSSIM::eMode xSSIM::xStrToMode(const std::string& Mode)
{
  std::string ModeL = xString::toLower(Mode);
  return ModeL == "regulargaussianflt" ? eMode::RegularGaussianFlt :
         ModeL == "regulargaussianint" ? eMode::RegularGaussianInt :
         ModeL == "regularaveraged"    ? eMode::RegularAveraged    :
         ModeL == "blockgaussianint"   ? eMode::BlockGaussianInt   :
         ModeL == "blockaveraged"      ? eMode::BlockAveraged      :
                                         eMode::INVALID;
}
std::string xSSIM::xModeToStr(eMode Mode)
{
  return Mode == eMode::RegularGaussianFlt ? "RegularGaussianFlt" :
         Mode == eMode::RegularGaussianInt ? "RegularGaussianInt" :
         Mode == eMode::RegularAveraged    ? "RegularAveraged"    :
         Mode == eMode::BlockGaussianInt   ? "BlockGaussianInt"   :
         Mode == eMode::BlockAveraged      ? "BlockAveraged"      :
                                             "INVALID";
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xSSIM::create(int32V2 PicSize, int32 BitDepth, int32 Margin, bool EnableMS, bool EnableMask)
{
  m_PicSize    = PicSize;
  m_BitDepth   = BitDepth;
  m_StrSimMode = eMode::INVALID;
  m_IsRegular  = true;
  m_MrgExtMode = eMrgExt::INVALID;
  m_UseMargin  = false;
  m_WndSize    = NOT_VALID;
  m_WndStride  = NOT_VALID;

  flt64 MaxValue = (flt64)xBitDepth2MaxValue(BitDepth);
  m_C1 = xPow2(c_K1<flt64> * MaxValue);
  m_C2 = xPow2(c_K2<flt64> * MaxValue);

  for(int32 CmpIdx = 0; CmpIdx < 4; CmpIdx++)
  { 
    m_RowSumsSSIM[CmpIdx].resize(m_PicSize.getY(), 0.0);
    if(EnableMask) { m_RowSumsWght[CmpIdx].resize(m_PicSize.getY(), 0.0); }
  }

  if(EnableMS)
  {
    m_NumScales = xCalcNumScales(m_PicSize);
    int32V2 LastSize = m_PicSize;
    for(int32 i = 1; i < m_NumScales; i++)
    {
      int32V2 NewSize = LastSize >> 1;
      m_SubPicTst[i] = new xPicP(NewSize, BitDepth, Margin);
      m_SubPicRef[i] = new xPicP(NewSize, BitDepth, Margin);
      if(EnableMask)
      {
        m_SubPicMsk[i] = new xPicP(NewSize, BitDepth, Margin);
      }
      LastSize = NewSize;
    }
  }
}
void xSSIM::destroy()
{
  m_PicSize    = { NOT_VALID, NOT_VALID };
  m_StrSimMode = eMode::INVALID;
  m_C1         = std::numeric_limits<flt64>::quiet_NaN();
  m_C2         = std::numeric_limits<flt64>::quiet_NaN();

  for(int32 i = 1; i < c_NumMultiScales; i++)
  {
    if(m_SubPicTst[i]) { m_SubPicTst[i]->destroy(); delete m_SubPicTst[i]; m_SubPicTst[i] = nullptr; }
    if(m_SubPicRef[i]) { m_SubPicRef[i]->destroy(); delete m_SubPicRef[i]; m_SubPicRef[i] = nullptr; }
    if(m_SubPicMsk[i]) { m_SubPicMsk[i]->destroy(); delete m_SubPicMsk[i]; m_SubPicMsk[i] = nullptr; }
  }
}
void xSSIM::setStructSimParams(eMode Mode, eMrgExt MarginMode, int32 BlockSize, int32 WndStride)
{
  m_StrSimMode = Mode;
  m_IsRegular  = isRegularMode(m_StrSimMode);
  m_MrgExtMode = MarginMode;
  m_UseMargin  = m_IsRegular ? MarginMode != eMrgExt::None : false;
  m_WndSize    = determineWindowSize(m_StrSimMode, BlockSize);
  m_WndStride  = WndStride;

  switch(m_StrSimMode)
  {
  case eMode::RegularGaussianFlt: m_CalcPtr = xStructSim::CalcRglrFlt; m_CalcPtrMsk = nullptr                 ; break;
  case eMode::RegularGaussianInt: m_CalcPtr = xStructSim::CalcRglrInt; m_CalcPtrMsk = xStructSim::CalcRglrIntM; break;
  case eMode::RegularAveraged   : m_CalcPtr = xStructSim::CalcRglrAvg; m_CalcPtrMsk = xStructSim::CalcRglrAvgM; break;
  case eMode::BlockGaussianInt  : m_CalcPtr = xStructSim::CalcBlckInt; m_CalcPtrMsk = nullptr                 ; break;
  case eMode::BlockAveraged     : m_CalcPtr = xStructSim::CalcBlckAvg; m_CalcPtrMsk = xStructSim::CalcBlckAvgM; break;
  default: assert(0); break;
  }

  //Multi-Block Structural Similarity
  if(!m_IsRegular)
  {
    m_MultiBlockAvgBatchSize = xStructSimMultiBlk::getMultiBlockAvgBatchSize(m_WndSize, m_WndStride);
    if(m_MultiBlockAvgBatchSize > 0)
    {
      m_CalcPtrMultiBlkAvgBatch = xStructSimMultiBlk::getCalcPtrMultiBlkAvgBatch(m_WndSize, m_WndStride);
      m_CalcPtrMultiBlkAvgTail  = xStructSimMultiBlk::getCalcPtrMultiBlkAvgTail (m_WndSize, m_WndStride);
    }
  }
}
flt64V4 xSSIM::calcPicMSSSIM(const xPicP* Tst, const xPicP* Ref)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst) && Ref->isSameSize(m_PicSize) && Ref->isSameBitDepth(m_BitDepth));

  std::array<flt64V4, c_NumMultiScales> SubScores; SubScores.fill(xMakeVec4<flt64>(0));

  SubScores[0] = xCalcPicSSIM(Tst, Ref, m_NumScales==1);
  for(int32 i = 1; i < m_NumScales; i++)
  {
    if(i == 1) { xDownsamplePic(m_SubPicTst[i], Tst             ); xDownsamplePic(m_SubPicRef[i], Ref             ); }
    else       { xDownsamplePic(m_SubPicTst[i], m_SubPicTst[i-1]); xDownsamplePic(m_SubPicRef[i], m_SubPicRef[i-1]); }

    if(m_UseMargin) { m_SubPicTst[i]->extend(m_MrgExtMode); m_SubPicRef[i]->extend(m_MrgExtMode); }

    SubScores[i] = xCalcPicSSIM(m_SubPicTst[i], m_SubPicRef[i], i == m_NumScales - 1);
  }

  //hint: sometimes SubScore can be negative, so do the same as pytorch - use ReLU to avoid (-0.sth)^Scale
  std::array<flt64V4, c_NumMultiScales> m_CorrectedSubScores;
  for(int32 i = 0; i < c_NumMultiScales; i++) { m_CorrectedSubScores[i] = SubScores[i].getVecReLU(); }

  const std::array<flt64, c_NumMultiScales> MultiScaleExponentWeights = c_MultiScaleExponentWeights<flt64>[m_NumScales-1];
  flt64V4 CompoundScore = xMakeVec4<flt64>(1);
  for(int32 i = 0; i < m_NumScales; i++) { CompoundScore *= m_CorrectedSubScores[i].getVecPow1(MultiScaleExponentWeights[i]); }

  return CompoundScore;
}
flt64V4 xSSIM::calcPicSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk)
{
  assert(Ref != nullptr && Tst != nullptr && Msk != nullptr && Ref->isCompatible(Tst) && Ref->isSameSize(m_PicSize) && Ref->isSameBitDepth(m_BitDepth) && Msk->isSameSize(m_PicSize));

  flt64V4 SSIM = xCalcPicSSIMM(Tst, Ref, Msk, true);
  return SSIM;
}
flt64V4 xSSIM::calcPicMSSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk)
{
  assert(Ref != nullptr && Tst != nullptr && Msk != nullptr && Ref->isCompatible(Tst) && Ref->isSameSize(m_PicSize) && Ref->isSameBitDepth(m_BitDepth) && Msk->isSameSize(m_PicSize));

  std::array<flt64V4, c_NumMultiScales> SubScores; SubScores.fill(xMakeVec4<flt64>(0));

  SubScores[0] = xCalcPicSSIMM(Tst, Ref, Msk, m_NumScales==1);
  for(int32 i = 1; i < m_NumScales; i++)
  {
    if(i == 1) { xDownsampleMsk(m_SubPicMsk[i], Msk             ); xDownsamplePicWght(m_SubPicTst[i], Tst             , Msk             ); xDownsamplePicWght(m_SubPicRef[i], Ref             , Msk               ); }
    else       { xDownsampleMsk(m_SubPicMsk[i], m_SubPicMsk[i-1]); xDownsamplePicWght(m_SubPicTst[i], m_SubPicTst[i-1], m_SubPicMsk[i-1]); xDownsamplePicWght(m_SubPicRef[i], m_SubPicRef[i-1], m_SubPicMsk[i - 1]); }

    if(m_UseMargin) { m_SubPicTst[i]->extend(m_MrgExtMode); m_SubPicRef[i]->extend(m_MrgExtMode); m_SubPicMsk[i]->extend(m_MrgExtMode); }

    SubScores[i] = xCalcPicSSIMM(m_SubPicTst[i], m_SubPicRef[i], m_SubPicMsk[i], i == m_NumScales - 1);
  }

  //Hint: sometimes SubScore can be negative, so do the same as pytorch - use ReLU to avoid (-0.sth)^Scale
  std::array<flt64V4, c_NumMultiScales> m_CorrectedSubScores;
  for(int32 i = 0; i < c_NumMultiScales; i++) { m_CorrectedSubScores[i] = SubScores[i].getVecReLU(); }

  const std::array<flt64, c_NumMultiScales> MultiScaleExponentWeights = c_MultiScaleExponentWeights<flt64>[m_NumScales-1];
  flt64V4 CompoundScore = xMakeVec4<flt64>(1);
  for(int32 i = 0; i < m_NumScales; i++) { CompoundScore *= m_CorrectedSubScores[i].getVecPow1(MultiScaleExponentWeights[i]); }

  return CompoundScore;
}
void xSSIM::visualizeSSIM(xPlane<uint16>* Vis, const xPicP* Tst, const xPicP* Ref, eCmp CmpId)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst) && Ref->getHeight() <= m_PicSize.getY() && Ref->isSameBitDepth(m_BitDepth));
  assert(m_IsRegular == true && m_WndStride == 1);

  xVisPicSSIM(Vis, Tst, Ref, CmpId, false);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xSSIM::xInitLoopRanges(int32 Width, int32 Height)
{
  if(m_IsRegular)
  {
    if(m_UseMargin)
    {
      m_LoopBegY = 0;
      m_LoopEndY = Height;
      m_LoopBegX = 0;
      m_LoopEndX = Width;
    }
    else
    {
      m_LoopBegY = c_FilterRange;
      m_LoopEndY = Height - c_FilterRange;
      m_LoopBegX = c_FilterRange;
      m_LoopEndX = Width - c_FilterRange;
    }
    m_NumUnitY = (m_LoopEndY - m_LoopBegY + m_WndStride - 1) / m_WndStride;
    m_NumUnitX = (m_LoopEndX - m_LoopBegX + m_WndStride - 1) / m_WndStride;
  }
  else //block based
  {
    m_LoopBegY = 0;
    m_LoopEndY = Height - m_WndSize + 1;
    m_NumUnitY = xCalcNumBlocks(Height, m_WndSize, m_WndStride);
    m_LoopBegX = 0;
    m_LoopEndX = Width - m_WndSize + 1;
    m_NumUnitX = xCalcNumBlocks(Width, m_WndSize, m_WndStride);
    if(xc_USE_SSIM_MULTI_BLOCK && m_MultiBlockAvgBatchSize > 0)
    {
      const int32 BatchWidth = m_WndStride * m_MultiBlockAvgBatchSize;
      m_MultiBlockAvgBatchEndX = (m_LoopEndX / BatchWidth) * BatchWidth;
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

flt64V4 xSSIM::xCalcPicSSIM(const xPicP* Tst, const xPicP* Ref, bool CalcL)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst) && Ref->getHeight() <= m_PicSize.getY() && Ref->isSameBitDepth(m_BitDepth));
  xInitLoopRanges(Ref->getWidth(), Ref->getHeight());

  flt64V4 SSIM = xMakeVec4<flt64>(0.0);
  for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
  {
    SSIM[CmpIdx] = xCalcCmpSSIM(Tst, Ref, (eCmp)CmpIdx, CalcL);
  }

  return SSIM;
}
flt64 xSSIM::xCalcCmpSSIM(const xPicP* Tst, const xPicP* Ref, eCmp CmpId, bool CalcL)
{
  memset(m_RowSumsSSIM[(int32)CmpId].data(), 0, m_RowSumsSSIM[(int32)CmpId].size() * sizeof(flt64));

  if(xc_USE_SSIM_MULTI_BLOCK && m_MultiBlockAvgBatchSize > 0)
  {
    for(int32 y = m_LoopBegY; y < m_LoopEndY; y += m_WndStride)
    {
      m_ThPI->storeTask([this, &Tst, &Ref, CmpId, y, CalcL](int32) { m_RowSumsSSIM[(int32)CmpId][y] = xCalcRowSSIM_MB(Tst, Ref, CmpId, y, CalcL); });
    }
  }
  else
  {
    for(int32 y = m_LoopBegY; y < m_LoopEndY; y += m_WndStride)
    {
      m_ThPI->storeTask([this, &Tst, &Ref, CmpId, y, CalcL](int32) { m_RowSumsSSIM[(int32)CmpId][y] = xCalcRowSSIM(Tst, Ref, CmpId, y, CalcL); });
    }
  }
  m_ThPI->executeStoredTasks();

  flt64 PicSumSSIM = xKBNS::Accumulate(m_RowSumsSSIM[(int32)CmpId]);
  int64 NumActive  = (int64)m_NumUnitY * (int64)m_NumUnitX;
  flt64 SSIM       = PicSumSSIM / (flt64)NumActive;
  return SSIM;
}
flt64 xSSIM::xCalcRowSSIM(const xPicP* Tst, const xPicP* Ref, eCmp CmpId, const int32 y, bool CalcL) const
{
  const int32   TstStride = Tst->getStride();
  const int32   RefStride = Ref->getStride();
  const uint16* TstPtr    = Tst->getAddr(CmpId) + y * TstStride;
  const uint16* RefPtr    = Ref->getAddr(CmpId) + y * RefStride;
  
  xKBNS1 RowAccSSIM;

  for(int32 x = m_LoopBegX; x < m_LoopEndX; x += m_WndStride)
  {
    RowAccSSIM += m_CalcPtr(TstPtr + x, RefPtr + x, TstStride, RefStride, m_WndSize, m_C1, m_C2, CalcL);
  }

  flt64 RowSumSSIM = RowAccSSIM.result();
  return RowSumSSIM;
}
flt64 xSSIM::xCalcRowSSIM_MB(const xPicP* Tst, const xPicP* Ref, eCmp CmpId, const int32 y, bool CalcL) const
{
  const int32   TstStride = Tst->getStride();
  const int32   RefStride = Ref->getStride();
  const uint16* TstPtr    = Tst->getAddr(CmpId) + y * TstStride;
  const uint16* RefPtr    = Ref->getAddr(CmpId) + y * RefStride;

  xKBNS1 RowAccSSIM;

  std::array<flt64, xStructSimMultiBlk::c_MaxBatchSize> BatchSSIMs;

  const int32 BatchWidth = m_WndStride * m_MultiBlockAvgBatchSize;
  for(int32 x = m_LoopBegX; x < m_MultiBlockAvgBatchEndX; x += BatchWidth)
  {
    m_CalcPtrMultiBlkAvgBatch(BatchSSIMs.data(), TstPtr + x, RefPtr + x, TstStride, RefStride, m_C1, m_C2, CalcL);
    for(int32 i = 0; i < m_MultiBlockAvgBatchSize; i++)
    {
      RowAccSSIM += BatchSSIMs[i];
    }
  }
  for(int32 x = m_MultiBlockAvgBatchEndX; x < m_LoopEndX; x += m_WndStride)
  {
    RowAccSSIM += m_CalcPtrMultiBlkAvgTail(TstPtr + x, RefPtr + x, TstStride, RefStride, m_C1, m_C2, CalcL);
  }

  flt64 RowSumSSIM = RowAccSSIM.result();
  return RowSumSSIM;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

flt64V4 xSSIM::xCalcPicSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, bool CalcL)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst) && Ref->getHeight() <= m_PicSize.getY() && Ref->isSameBitDepth(m_BitDepth));
  xInitLoopRanges(Ref->getWidth(), Ref->getHeight());

  flt64V4 SSIM = xMakeVec4<flt64>(0.0);
  for(int32 CmpIdx = 0; CmpIdx < m_NumComponents; CmpIdx++)
  {
    SSIM[CmpIdx] = xCalcCmpSSIMM(Tst, Ref, Msk, (eCmp)CmpIdx, CalcL);
  }
  return SSIM;
}
flt64 xSSIM::xCalcCmpSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, eCmp CmpId, bool CalcL)
{
  memset(m_RowSumsSSIM[(int32)CmpId].data(), 0, m_RowSumsSSIM[(int32)CmpId].size() * sizeof(flt64));
  memset(m_RowSumsWght[(int32)CmpId].data(), 0, m_RowSumsWght[(int32)CmpId].size() * sizeof(flt64));

  for(int32 y = m_LoopBegY; y < m_LoopEndY; y += m_WndStride)
  {
    m_ThPI->storeTask([this, &Tst, &Ref, CmpId, y, CalcL](int32) { m_RowSumsSSIM[1][y] = xCalcRowSSIM(Tst, Ref, CmpId, y, CalcL); });
  }
  m_ThPI->executeStoredTasks();

  for(int32 y = m_LoopBegY; y < m_LoopEndY; y += m_WndStride)
  {
    m_ThPI->storeTask([this, &Tst, &Ref, &Msk, CmpId, y, CalcL](int32) { std::tie(m_RowSumsSSIM[(int32)CmpId][y], m_RowSumsWght[(int32)CmpId][y]) = xCalcRowSSIMM(Tst, Ref, Msk, CmpId, y, CalcL); });
  }
  m_ThPI->executeStoredTasks();

  flt64 PicSumSSIM = xKBNS::Accumulate(m_RowSumsSSIM[(int32)CmpId]);
  flt64 PicSumWght = xKBNS::Accumulate(m_RowSumsWght[(int32)CmpId]);
  flt64 SSIM       = PicSumSSIM / PicSumWght;
  return SSIM;
}
tSSIMM xSSIM::xCalcRowSSIMM(const xPicP* Tst, const xPicP* Ref, const xPicP* Msk, eCmp CmpId, const int32 y, bool CalcL) const
{
  const int32   TstStride = Tst->getStride();
  const int32   RefStride = Ref->getStride();
  const int32   MskStride = Msk->getStride();
  const uint16* TstPtr    = Tst->getAddr(CmpId   ) + y * TstStride;
  const uint16* RefPtr    = Ref->getAddr(CmpId   ) + y * RefStride;
  const uint16* MskPtr    = Msk->getAddr(eCmp::LM) + y * MskStride;
  
  xKBNS1 RowAccSSIM;
  xKBNS1 RowAccWght;

  for(int32 x = 0; x < m_LoopEndX; x += m_WndStride)
  {
    auto [SSIM, Weight] = m_CalcPtrMsk(TstPtr + x, RefPtr + x, MskPtr + x, TstStride, RefStride, MskStride, m_WndSize, m_C1, m_C2, CalcL);

    if(std::isfinite(SSIM) && Weight > 0.0)
    {
      RowAccSSIM += SSIM  ;
      RowAccWght += Weight;
    }
  }

  flt64 RowSumSSIM = RowAccSSIM.result();
  flt64 RowSumWght = RowAccWght.result();
  return { RowSumSSIM,RowSumWght };
}
void xSSIM::xVisPicSSIM(xPlane<uint16>* Vis, const xPicP* Tst, const xPicP* Ref, eCmp CmpId, bool UseMin)
{
  assert(Ref != nullptr && Tst != nullptr && Ref->isCompatible(Tst) && Ref->getHeight() <= m_PicSize.getY() && Ref->isSameBitDepth(m_BitDepth));
  assert(m_IsRegular == true && m_WndStride == 1);
  xInitLoopRanges(Ref->getWidth(), Ref->getHeight());

  const uint16*    TstPtr    = Tst->getAddr  (CmpId);
  const uint16*    RefPtr    = Ref->getAddr  (CmpId);
  uint16* restrict VisPtr    = Vis->getAddr  (     );
  const int32      TstStride = Tst->getStride();
  const int32      RefStride = Ref->getStride();
  const int32      VisStride = Vis->getStride();

  const int32      MaxVal = Vis->getMaxPelValue();

  for(int32 y = m_LoopBegY; y < m_LoopEndY; y += m_WndStride)
  {
    const uint16*    TstRowPtr = TstPtr + y*TstStride;
    const uint16*    RefRowPtr = RefPtr + y*RefStride;
    uint16* restrict VisRowPtr = VisPtr + y*VisStride;

    if(UseMin)
    { 
      for(int32 x = m_LoopBegX; x < m_LoopEndX; x += m_WndStride)
      {
        flt64  SSIM    = m_CalcPtr(TstRowPtr + x, RefRowPtr + x, TstStride, RefStride, m_WndSize, m_C1, m_C2, true);
        uint16 VisSSIM = (uint16)xRoundF64ToU32((xReLU(SSIM)) * MaxVal);
        VisRowPtr[x] = xMin(VisRowPtr[x], VisSSIM);
      }

    }
    else
    {
      for(int32 x = m_LoopBegX; x < m_LoopEndX; x += m_WndStride)
      {
        flt64  SSIM    = m_CalcPtr(TstRowPtr + x, RefRowPtr + x, TstStride, RefStride, m_WndSize, m_C1, m_C2, true);
        uint16 VisSSIM = (uint16)xRoundF64ToU32((xReLU(SSIM)) * MaxVal);
        VisRowPtr[x] = VisSSIM;
      }
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int32 xSSIM::xCalcNumBlocks(int32 Length, int32 BlockSize, int32 BlockStride)
{
  int32 NumActive = 0;
  assert(BlockSize >= BlockStride);

  if(xIsPowerOf2(BlockSize) && xIsPowerOf2(BlockStride))
  {
    const int32 Log2BlkSize     = xFastLog2((uint32)BlockSize  );
    const int32 Log2BlkStride   = xFastLog2((uint32)BlockStride);
    int32 NumBlocksIn1stRow     = Length >> Log2BlkSize;
    int32 RemainPixelsIn1stRow  = Length - (NumBlocksIn1stRow << Log2BlkSize);
    int32 Log2NumStridesInBlock = Log2BlkSize - Log2BlkStride;
    NumActive = ((NumBlocksIn1stRow - 1) << Log2NumStridesInBlock) + (RemainPixelsIn1stRow >> Log2BlkStride) + 1;
  }
  else
  {
    int32 End = Length - BlockSize + 1;
    int32 Cnt = 0;
    for(int32 i = 0; i < End; i += BlockStride) { Cnt++; }
    return Cnt;
  }

  return NumActive;
}
int32 xSSIM::xCalcNumScales(int32V2 PicSize)
{
  int32 NumScales = c_NumMultiScales;
  for(int32 i = 0; i < 5; i++)
  {
    const int32 ScaleLimit = c_ScaledSizeLimit << i;
    if(PicSize.getX() < ScaleLimit || PicSize.getY() < ScaleLimit)
    { 
      NumScales = i; break;
    }
  }
  return NumScales;
}
void xSSIM::xDownsamplePic(xPicP* Dst, const xPicP* Src)
{
  for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
  {
    xPixelOps::DownsampleHV(Dst->getAddr((eCmp)CmpIdx), Src->getAddr((eCmp)CmpIdx), Dst->getStride(), Src->getStride(), Dst->getWidth(), Dst->getHeight());
  }
}
void xSSIM::xDownsampleMsk(xPicP* DstMsk, const xPicP* SrcMsk)
{
  xPixelOps::DownsampleHV(DstMsk->getAddr(eCmp::C0), SrcMsk->getAddr(eCmp::C0), DstMsk->getStride(), SrcMsk->getStride(), DstMsk->getWidth(), DstMsk->getHeight());
}
void xSSIM::xDownsamplePicWght(xPicP* DstPic, const xPicP* SrcPic, const xPicP* SrcMsk)
{
  for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
  {
    xDownsampleWeightedHV(DstPic->getAddr((eCmp)CmpIdx), SrcPic->getAddr((eCmp)CmpIdx), SrcMsk->getAddr(eCmp::C0), DstPic->getStride(), SrcPic->getStride(), SrcMsk->getStride(), DstPic->getWidth(), DstPic->getHeight());
  }
}
void xSSIM::xDownsampleWeightedHV(uint16* restrict Dst, const uint16* Src, const uint16* Wgh, int32 DstStride, int32 SrcStride, int32 WghStride, int32 DstWidth, int32 DstHeight)
{
  const uint16* SrcL0 = Src;
  const uint16* SrcL1 = Src + SrcStride;
  const uint16* WghL0 = Wgh;
  const uint16* WghL1 = Wgh + WghStride;

  for(int32 y = 0; y < DstHeight; y++)
  {
    for(int32 x = 0; x < DstWidth; x++)
    {
      const int32 SrcX = x << 1;
      int32 WghA = WghL0[SrcX    ];
      int32 WghB = WghL0[SrcX + 1];
      int32 WghC = WghL1[SrcX    ];
      int32 WghD = WghL1[SrcX + 1];
      int32 SrcA = SrcL0[SrcX    ];
      int32 SrcB = SrcL0[SrcX + 1];
      int32 SrcC = SrcL1[SrcX    ];
      int32 SrcD = SrcL1[SrcX + 1];
      
      int32  SumWgh = WghA + WghB + WghC + WghD;
      int32  SumSrc = SrcA*WghA + SrcB*WghB + SrcC*WghC + SrcD*WghD;
      uint32 AvgSrc = xRoundF32ToU32((flt32)SumSrc / (flt32)SumWgh);

      Dst[x] = (uint16)AvgSrc;
    }
    Dst += DstStride;
    SrcL0 += (SrcStride << 1);
    SrcL1 += (SrcStride << 1);
    WghL0 += (WghStride << 1);
    WghL1 += (WghStride << 1);
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB