/*
    SPDX-FileCopyrightText: 2025 Patrycja Kaźmierczak <patrycja.kazmierczak@student.put.poznan.pl>
    SPDX-FileCopyrightText: 2025-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xCorrespPixelShiftNEON.h"

#if X_SIMD_CAN_USE_NEON

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xCorrespPixelShiftNEON
//===============================================================================================================================================================================================================
void xCorrespPixelShiftNEON::GenShftCompRowI16(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 Width  = Tst->getWidth();
  const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8<uint32>);

  const int32 TstOffset  = y * Tst->getStride();
  const int32 RefStride  = Ref->getStride();
  const int32 MaxValue   = DstRef->getMaxPelValue();
  const int32 WindowSize = 2 * SearchRange + 1;

  const uint16*    TstPtrLm = Tst   ->getAddr(eCmp::LM) + TstOffset;
  const uint16*    TstPtrCb = Tst   ->getAddr(eCmp::CB) + TstOffset;
  const uint16*    TstPtrCr = Tst   ->getAddr(eCmp::CR) + TstOffset;
  uint16* restrict DstPtrLm = DstRef->getAddr(eCmp::LM) + TstOffset;
  uint16* restrict DstPtrCb = DstRef->getAddr(eCmp::CB) + TstOffset;
  uint16* restrict DstPtrCr = DstRef->getAddr(eCmp::CR) + TstOffset;
  const uint16*    RefPtrLm = Ref   ->getAddr(eCmp::LM);
  const uint16*    RefPtrCb = Ref   ->getAddr(eCmp::CB);
  const uint16*    RefPtrCr = Ref   ->getAddr(eCmp::CR);

  const int16x8_t ShiftLm  = vdupq_n_s16((int16)GlobalColorShift[0]), ShiftCb  = vdupq_n_s16((int16)GlobalColorShift[1]), ShiftCr  = vdupq_n_s16((int16)GlobalColorShift[2]);
  const int16x8_t WeightLm = vdupq_n_s16((int16)CmpWeights[0])      , WeightCb = vdupq_n_s16((int16)CmpWeights[1])      , WeightCr = vdupq_n_s16((int16)CmpWeights[2]);
  const int32x4_t One      = vdupq_n_s32(1);
  static const int32 c_LaneIdx[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
  const int32x4_t LaneIdxLo = vld1q_s32(c_LaneIdx), LaneIdxHi = vld1q_s32(c_LaneIdx + 4);

  for(int32 x = 0; x < Width8; x += 8)
  {
    const int16x8_t TstLm = vaddq_s16(vreinterpretq_s16_u16(vld1q_u16(TstPtrLm + x)), ShiftLm);
    const int16x8_t TstCb = vaddq_s16(vreinterpretq_s16_u16(vld1q_u16(TstPtrCb + x)), ShiftCb);
    const int16x8_t TstCr = vaddq_s16(vreinterpretq_s16_u16(vld1q_u16(TstPtrCr + x)), ShiftCr);
    uint32x4_t BestErrorLo  = vdupq_n_u32(0xFFFFFFFF), BestErrorHi  = BestErrorLo;
    int32x4_t  BestOffsetLo = vdupq_n_s32(NOT_VALID) , BestOffsetHi = BestOffsetLo;

    for(int32 dy = -SearchRange; dy <= SearchRange; dy++)
    {
      const int32 RowOffset = (y + dy) * RefStride + x - SearchRange;
      int32x4_t CandOffset = vdupq_n_s32(RowOffset);
      for(int32 dx = 0; dx < WindowSize; dx++)
      {
        const int32     Offset  = RowOffset + dx;
        const int16x8_t DiffLm  = vsubq_s16(TstLm, vreinterpretq_s16_u16(vld1q_u16(RefPtrLm + Offset)));
        const int16x8_t DiffCb  = vsubq_s16(TstCb, vreinterpretq_s16_u16(vld1q_u16(RefPtrCb + Offset)));
        const int16x8_t DiffCr  = vsubq_s16(TstCr, vreinterpretq_s16_u16(vld1q_u16(RefPtrCr + Offset)));
        const int16x8_t WDiffLm = vmulq_s16(DiffLm, WeightLm);
        const int16x8_t WDiffCb = vmulq_s16(DiffCb, WeightCb);
        const int16x8_t WDiffCr = vmulq_s16(DiffCr, WeightCr);
        int32x4_t ErrorLo = vmull_s16(vget_low_s16(DiffLm), vget_low_s16(WDiffLm));
        ErrorLo           = vmlal_s16(ErrorLo, vget_low_s16(DiffCb), vget_low_s16(WDiffCb));
        ErrorLo           = vmlal_s16(ErrorLo, vget_low_s16(DiffCr), vget_low_s16(WDiffCr));
        int32x4_t ErrorHi = vmull_high_s16(DiffLm, WDiffLm);
        ErrorHi           = vmlal_high_s16(ErrorHi, DiffCb, WDiffCb);
        ErrorHi           = vmlal_high_s16(ErrorHi, DiffCr, WDiffCr);
        const uint32x4_t UErrorLo = vreinterpretq_u32_s32(ErrorLo);
        const uint32x4_t UErrorHi = vreinterpretq_u32_s32(ErrorHi);
        const uint32x4_t BetterLo = vcltq_u32(UErrorLo, BestErrorLo);
        const uint32x4_t BetterHi = vcltq_u32(UErrorHi, BestErrorHi);
        BestErrorLo  = vminq_u32(BestErrorLo, UErrorLo);
        BestErrorHi  = vminq_u32(BestErrorHi, UErrorHi);
        BestOffsetLo = vbslq_s32(BetterLo, CandOffset, BestOffsetLo);
        BestOffsetHi = vbslq_s32(BetterHi, CandOffset, BestOffsetHi);
        CandOffset   = vaddq_s32(CandOffset, One);
      } //dx
    } //dy

    int32 BestRefOffsets[8];
    vst1q_s32(BestRefOffsets    , vaddq_s32(BestOffsetLo, LaneIdxLo));
    vst1q_s32(BestRefOffsets + 4, vaddq_s32(BestOffsetHi, LaneIdxHi));
    for(int32 i = 0; i < 8; i++)
    {
      const int32 BestRefOffset = BestRefOffsets[i];
      DstPtrLm[x + i] = (uint16)xClipU((int32)RefPtrLm[BestRefOffset] - GlobalColorShift[0], MaxValue);
      DstPtrCb[x + i] = (uint16)xClipU((int32)RefPtrCb[BestRefOffset] - GlobalColorShift[1], MaxValue);
      DstPtrCr[x + i] = (uint16)xClipU((int32)RefPtrCr[BestRefOffset] - GlobalColorShift[2], MaxValue);
    }
  } //x

  if(Width8 < Width) { xGenShftCompRowTail(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights, Width8); }
}
void xCorrespPixelShiftNEON::GenShftCompRowU32(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 Width  = Tst->getWidth();
  const int32 Width8 = (int32)((uint32)Width & c_MultipleMask8<uint32>);

  const int32 TstOffset  = y * Tst->getStride();
  const int32 RefStride  = Ref->getStride();
  const int32 MaxValue   = DstRef->getMaxPelValue();
  const int32 WindowSize = 2 * SearchRange + 1;
  assert(DstRef->getBitDepth() <= 14);
  assert(GlobalColorShift[0] >= -MaxValue && GlobalColorShift[0] <= MaxValue && GlobalColorShift[1] >= -MaxValue && GlobalColorShift[1] <= MaxValue && GlobalColorShift[2] >= -MaxValue && GlobalColorShift[2] <= MaxValue);

  const uint16*    TstPtrLm = Tst   ->getAddr(eCmp::LM) + TstOffset;
  const uint16*    TstPtrCb = Tst   ->getAddr(eCmp::CB) + TstOffset;
  const uint16*    TstPtrCr = Tst   ->getAddr(eCmp::CR) + TstOffset;
  uint16* restrict DstPtrLm = DstRef->getAddr(eCmp::LM) + TstOffset;
  uint16* restrict DstPtrCb = DstRef->getAddr(eCmp::CB) + TstOffset;
  uint16* restrict DstPtrCr = DstRef->getAddr(eCmp::CR) + TstOffset;
  const uint16*    RefPtrLm = Ref   ->getAddr(eCmp::LM);
  const uint16*    RefPtrCb = Ref   ->getAddr(eCmp::CB);
  const uint16*    RefPtrCr = Ref   ->getAddr(eCmp::CR);

  const int16x8_t ShiftLm  = vdupq_n_s16((int16)GlobalColorShift[0]), ShiftCb  = vdupq_n_s16((int16)GlobalColorShift[1]), ShiftCr  = vdupq_n_s16((int16)GlobalColorShift[2]);
  const uint32    WeightLm = (uint32)CmpWeights[0]                   , WeightCb = (uint32)CmpWeights[1]                   , WeightCr = (uint32)CmpWeights[2];
  const int32x4_t One      = vdupq_n_s32(1);
  static const int32 c_LaneIdx[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
  const int32x4_t LaneIdxLo = vld1q_s32(c_LaneIdx), LaneIdxHi = vld1q_s32(c_LaneIdx + 4);

  for(int32 x = 0; x < Width8; x += 8)
  {
    const int16x8_t TstLm = vaddq_s16(vreinterpretq_s16_u16(vld1q_u16(TstPtrLm + x)), ShiftLm);
    const int16x8_t TstCb = vaddq_s16(vreinterpretq_s16_u16(vld1q_u16(TstPtrCb + x)), ShiftCb);
    const int16x8_t TstCr = vaddq_s16(vreinterpretq_s16_u16(vld1q_u16(TstPtrCr + x)), ShiftCr);
    uint32x4_t BestErrorLo  = vdupq_n_u32(0xFFFFFFFF), BestErrorHi  = BestErrorLo;
    int32x4_t  BestOffsetLo = vdupq_n_s32(NOT_VALID) , BestOffsetHi = BestOffsetLo;

    for(int32 dy = -SearchRange; dy <= SearchRange; dy++)
    {
      const int32 RowOffset = (y + dy) * RefStride + x - SearchRange;
      int32x4_t CandOffset = vdupq_n_s32(RowOffset);
      for(int32 dx = 0; dx < WindowSize; dx++)
      {
        const int32     Offset = RowOffset + dx;
        const int16x8_t DiffLm = vsubq_s16(TstLm, vreinterpretq_s16_u16(vld1q_u16(RefPtrLm + Offset)));
        const int16x8_t DiffCb = vsubq_s16(TstCb, vreinterpretq_s16_u16(vld1q_u16(RefPtrCb + Offset)));
        const int16x8_t DiffCr = vsubq_s16(TstCr, vreinterpretq_s16_u16(vld1q_u16(RefPtrCr + Offset)));
        uint32x4_t ErrorLo = vmulq_n_u32(         vreinterpretq_u32_s32(vmull_s16(vget_low_s16(DiffLm), vget_low_s16(DiffLm))), WeightLm);
        ErrorLo            = vmlaq_n_u32(ErrorLo, vreinterpretq_u32_s32(vmull_s16(vget_low_s16(DiffCb), vget_low_s16(DiffCb))), WeightCb);
        ErrorLo            = vmlaq_n_u32(ErrorLo, vreinterpretq_u32_s32(vmull_s16(vget_low_s16(DiffCr), vget_low_s16(DiffCr))), WeightCr);
        uint32x4_t ErrorHi = vmulq_n_u32(         vreinterpretq_u32_s32(vmull_high_s16(DiffLm, DiffLm)), WeightLm);
        ErrorHi            = vmlaq_n_u32(ErrorHi, vreinterpretq_u32_s32(vmull_high_s16(DiffCb, DiffCb)), WeightCb);
        ErrorHi            = vmlaq_n_u32(ErrorHi, vreinterpretq_u32_s32(vmull_high_s16(DiffCr, DiffCr)), WeightCr);
        const uint32x4_t BetterLo = vcltq_u32(ErrorLo, BestErrorLo);
        const uint32x4_t BetterHi = vcltq_u32(ErrorHi, BestErrorHi);
        BestErrorLo  = vminq_u32(BestErrorLo, ErrorLo);
        BestErrorHi  = vminq_u32(BestErrorHi, ErrorHi);
        BestOffsetLo = vbslq_s32(BetterLo, CandOffset, BestOffsetLo);
        BestOffsetHi = vbslq_s32(BetterHi, CandOffset, BestOffsetHi);
        CandOffset   = vaddq_s32(CandOffset, One);
      } //dx
    } //dy

    int32 BestRefOffsets[8];
    vst1q_s32(BestRefOffsets    , vaddq_s32(BestOffsetLo, LaneIdxLo));
    vst1q_s32(BestRefOffsets + 4, vaddq_s32(BestOffsetHi, LaneIdxHi));
    for(int32 i = 0; i < 8; i++)
    {
      const int32 BestRefOffset = BestRefOffsets[i];
      DstPtrLm[x + i] = (uint16)xClipU((int32)RefPtrLm[BestRefOffset] - GlobalColorShift[0], MaxValue);
      DstPtrCb[x + i] = (uint16)xClipU((int32)RefPtrCb[BestRefOffset] - GlobalColorShift[1], MaxValue);
      DstPtrCr[x + i] = (uint16)xClipU((int32)RefPtrCr[BestRefOffset] - GlobalColorShift[2], MaxValue);
    }
  } //x

  if(Width8 < Width) { xGenShftCompRowTail(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights, Width8); }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_NEON