/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xCorrespPixelShiftAVX512.h"
#include "xHelpersSIMD.h"

#if X_SIMD_CAN_USE_AVX512

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xCorrespPixelShiftAVX512::GenShftCompRowI16(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 Width   = Tst->getWidth();
  const int32 Width32 = (int32)((uint32)Width & c_MultipleMask32<uint32>);

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

  const __m512i  ShiftLm  = _mm512_set1_epi16((short)GlobalColorShift[0]), ShiftCb  = _mm512_set1_epi16((short)GlobalColorShift[1]), ShiftCr  = _mm512_set1_epi16((short)GlobalColorShift[2]);
  const __m512i  WeightLm = _mm512_set1_epi16((short)CmpWeights[0])      , WeightCb = _mm512_set1_epi16((short)CmpWeights[1])      , WeightCr = _mm512_set1_epi16((short)CmpWeights[2]);
  const __mmask32 OddWords = 0xAAAAAAAAu, EvenWords = 0x55555555u;
  const __m512i  One      = _mm512_set1_epi32(1);
  const __m512i  LaneIdxE = _mm512_setr_epi32(0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30);
  const __m512i  LaneIdxO = _mm512_add_epi32(LaneIdxE, One);

  for(int32 x = 0; x < Width32; x += 32)
  {
    const __m512i TstLm = _mm512_add_epi16(_mm512_loadu_si512((const void*)(TstPtrLm + x)), ShiftLm);
    const __m512i TstCb = _mm512_add_epi16(_mm512_loadu_si512((const void*)(TstPtrCb + x)), ShiftCb);
    const __m512i TstCr = _mm512_add_epi16(_mm512_loadu_si512((const void*)(TstPtrCr + x)), ShiftCr);
    __m512i BestErrorE  = _mm512_set1_epi32(-1), BestErrorO  = BestErrorE;
    __m512i BestOffsetE = _mm512_set1_epi32(NOT_VALID), BestOffsetO = BestOffsetE;

    for(int32 dy = -SearchRange; dy <= SearchRange; dy++)
    {
      const int32 RowOffset = (y + dy) * RefStride + x - SearchRange;
      __m512i CandOffset = _mm512_set1_epi32(RowOffset);
      for(int32 dx = 0; dx < WindowSize; dx++)
      {
        const int32   Offset  = RowOffset + dx;
        const __m512i DiffLm  = _mm512_sub_epi16(TstLm, _mm512_loadu_si512((const void*)(RefPtrLm + Offset)));
        const __m512i DiffCb  = _mm512_sub_epi16(TstCb, _mm512_loadu_si512((const void*)(RefPtrCb + Offset)));
        const __m512i DiffCr  = _mm512_sub_epi16(TstCr, _mm512_loadu_si512((const void*)(RefPtrCr + Offset)));
        const __m512i WDiffLm = _mm512_mullo_epi16(DiffLm, WeightLm);
        const __m512i WDiffCb = _mm512_mullo_epi16(DiffCb, WeightCb);
        const __m512i WDiffCr = _mm512_mullo_epi16(DiffCr, WeightCr);
        const __m512i PairDE  = _mm512_mask_blend_epi16(OddWords, DiffLm, _mm512_slli_epi32(DiffCb, 16));
        const __m512i PairDO  = _mm512_mask_blend_epi16(OddWords, _mm512_srli_epi32(DiffLm, 16), DiffCb);
        const __m512i PairWE  = _mm512_mask_blend_epi16(OddWords, WDiffLm, _mm512_slli_epi32(WDiffCb, 16));
        const __m512i PairWO  = _mm512_mask_blend_epi16(OddWords, _mm512_srli_epi32(WDiffLm, 16), WDiffCb);
        const __m512i ErrorE  = _mm512_add_epi32(_mm512_madd_epi16(PairDE, PairWE), _mm512_madd_epi16(DiffCr, _mm512_maskz_mov_epi16(EvenWords, WDiffCr)));
        const __m512i ErrorO  = _mm512_add_epi32(_mm512_madd_epi16(PairDO, PairWO), _mm512_madd_epi16(DiffCr, _mm512_maskz_mov_epi16(OddWords , WDiffCr)));
        const __mmask16 BetterE = _mm512_cmplt_epu32_mask(ErrorE, BestErrorE);
        const __mmask16 BetterO = _mm512_cmplt_epu32_mask(ErrorO, BestErrorO);
        BestErrorE  = _mm512_min_epu32(BestErrorE, ErrorE);
        BestErrorO  = _mm512_min_epu32(BestErrorO, ErrorO);
        BestOffsetE = _mm512_mask_mov_epi32(BestOffsetE, BetterE, CandOffset);
        BestOffsetO = _mm512_mask_mov_epi32(BestOffsetO, BetterO, CandOffset);
        CandOffset  = _mm512_add_epi32(CandOffset, One);
      } //dx
    } //dy

    alignas(alignof(__m512i)) int32 BestRefOffsetsE[16], BestRefOffsetsO[16];
    _mm512_store_si512((__m512i*)BestRefOffsetsE, _mm512_add_epi32(BestOffsetE, LaneIdxE));
    _mm512_store_si512((__m512i*)BestRefOffsetsO, _mm512_add_epi32(BestOffsetO, LaneIdxO));
    for(int32 i = 0; i < 32; i++)
    {
      const int32 BestRefOffset = (i & 1) ? BestRefOffsetsO[i >> 1] : BestRefOffsetsE[i >> 1];
      DstPtrLm[x + i] = (uint16)xClipU((int32)RefPtrLm[BestRefOffset] - GlobalColorShift[0], MaxValue);
      DstPtrCb[x + i] = (uint16)xClipU((int32)RefPtrCb[BestRefOffset] - GlobalColorShift[1], MaxValue);
      DstPtrCr[x + i] = (uint16)xClipU((int32)RefPtrCr[BestRefOffset] - GlobalColorShift[2], MaxValue);
    }
  } //x

  if(Width32 < Width) { xGenShftCompRowTail(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights, Width32); }
}
void xCorrespPixelShiftAVX512::GenShftCompRowU32(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 Width   = Tst->getWidth();
  const int32 Width16 = (int32)((uint32)Width & c_MultipleMask16<uint32>);

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

  const __m512i ShiftLm  = _mm512_set1_epi32(GlobalColorShift[0]), ShiftCb  = _mm512_set1_epi32(GlobalColorShift[1]), ShiftCr  = _mm512_set1_epi32(GlobalColorShift[2]);
  const __m512i WeightLm = _mm512_set1_epi32(CmpWeights[0])      , WeightCb = _mm512_set1_epi32(CmpWeights[1])      , WeightCr = _mm512_set1_epi32(CmpWeights[2]);
  const __m512i One      = _mm512_set1_epi32(1);
  const __m512i LaneIdx  = _mm512_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

  for(int32 x = 0; x < Width16; x += 16)
  {
    const __m512i TstLm = _mm512_add_epi32(_mm512_cvtepu16_epi32(_mm256_loadu_si256((const __m256i*)(TstPtrLm + x))), ShiftLm);
    const __m512i TstCb = _mm512_add_epi32(_mm512_cvtepu16_epi32(_mm256_loadu_si256((const __m256i*)(TstPtrCb + x))), ShiftCb);
    const __m512i TstCr = _mm512_add_epi32(_mm512_cvtepu16_epi32(_mm256_loadu_si256((const __m256i*)(TstPtrCr + x))), ShiftCr);
    __m512i BestError  = _mm512_set1_epi32(-1); //0xFFFFFFFF
    __m512i BestOffset = _mm512_set1_epi32(NOT_VALID);

    for(int32 dy = -SearchRange; dy <= SearchRange; dy++)
    {
      const int32 RowOffset = (y + dy) * RefStride + x - SearchRange;
      __m512i CandOffset = _mm512_set1_epi32(RowOffset);
      for(int32 dx = 0; dx < WindowSize; dx++)
      {
        const int32   Offset = RowOffset + dx;
        const __m512i DiffLm = _mm512_sub_epi32(TstLm, _mm512_cvtepu16_epi32(_mm256_loadu_si256((const __m256i*)(RefPtrLm + Offset))));
        const __m512i DiffCb = _mm512_sub_epi32(TstCb, _mm512_cvtepu16_epi32(_mm256_loadu_si256((const __m256i*)(RefPtrCb + Offset))));
        const __m512i DiffCr = _mm512_sub_epi32(TstCr, _mm512_cvtepu16_epi32(_mm256_loadu_si256((const __m256i*)(RefPtrCr + Offset))));
        const __m512i Error  = _mm512_add_epi32(_mm512_add_epi32(_mm512_mullo_epi32(_mm512_mullo_epi32(DiffLm, DiffLm), WeightLm),
                                                                 _mm512_mullo_epi32(_mm512_mullo_epi32(DiffCb, DiffCb), WeightCb)),
                                                                 _mm512_mullo_epi32(_mm512_mullo_epi32(DiffCr, DiffCr), WeightCr));
        const __mmask16 Better = _mm512_cmplt_epu32_mask(Error, BestError);
        BestError  = _mm512_min_epu32(BestError, Error);
        BestOffset = _mm512_mask_mov_epi32(BestOffset, Better, CandOffset);
        CandOffset = _mm512_add_epi32(CandOffset, One);
      } //dx
    } //dy

    alignas(alignof(__m512i)) int32 BestRefOffsets[16];
    _mm512_store_si512((__m512i*)BestRefOffsets, _mm512_add_epi32(BestOffset, LaneIdx));
    for(int32 i = 0; i < 16; i++)
    {
      const int32 BestRefOffset = BestRefOffsets[i];
      DstPtrLm[x + i] = (uint16)xClipU((int32)RefPtrLm[BestRefOffset] - GlobalColorShift[0], MaxValue);
      DstPtrCb[x + i] = (uint16)xClipU((int32)RefPtrCb[BestRefOffset] - GlobalColorShift[1], MaxValue);
      DstPtrCr[x + i] = (uint16)xClipU((int32)RefPtrCr[BestRefOffset] - GlobalColorShift[2], MaxValue);
    }
  } //x

  if(Width16 < Width) { xGenShftCompRowTail(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights, Width16); }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_AVX512
