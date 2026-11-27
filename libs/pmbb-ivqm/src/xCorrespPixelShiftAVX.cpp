/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xCorrespPixelShiftAVX.h"
#include "xHelpersSIMD.h"

#if X_SIMD_CAN_USE_AVX

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xCorrespPixelShiftAVX::GenShftCompRowI16(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
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

  const __m256i ShiftLm  = _mm256_set1_epi16((short)GlobalColorShift[0]), ShiftCb  = _mm256_set1_epi16((short)GlobalColorShift[1]), ShiftCr  = _mm256_set1_epi16((short)GlobalColorShift[2]);
  const __m256i WeightLm = _mm256_set1_epi16((short)CmpWeights[0])      , WeightCb = _mm256_set1_epi16((short)CmpWeights[1])      , WeightCr = _mm256_set1_epi16((short)CmpWeights[2]);
  const __m256i MaskLo   = _mm256_set1_epi32(0x0000FFFF), MaskHi = _mm256_set1_epi32((int32)0xFFFF0000);
  const __m256i One      = _mm256_set1_epi32(1);
  const __m256i LaneIdxE = _mm256_setr_epi32(0, 2, 4, 6, 8, 10, 12, 14), LaneIdxO = _mm256_setr_epi32(1, 3, 5, 7, 9, 11, 13, 15);

  for(int32 x = 0; x < Width16; x += 16)
  {
    const __m256i TstLm = _mm256_add_epi16(_mm256_loadu_si256((const __m256i*)(TstPtrLm + x)), ShiftLm);
    const __m256i TstCb = _mm256_add_epi16(_mm256_loadu_si256((const __m256i*)(TstPtrCb + x)), ShiftCb);
    const __m256i TstCr = _mm256_add_epi16(_mm256_loadu_si256((const __m256i*)(TstPtrCr + x)), ShiftCr);
    __m256i BestErrorE  = _mm256_set1_epi32(-1), BestErrorO  = BestErrorE;
    __m256i BestOffsetE = _mm256_set1_epi32(NOT_VALID), BestOffsetO = BestOffsetE;

    for(int32 dy = -SearchRange; dy <= SearchRange; dy++)
    {
      const int32 RowOffset = (y + dy) * RefStride + x - SearchRange;
      __m256i CandOffset = _mm256_set1_epi32(RowOffset);
      for(int32 dx = 0; dx < WindowSize; dx++)
      {
        const int32   Offset  = RowOffset + dx;
        const __m256i DiffLm  = _mm256_sub_epi16(TstLm, _mm256_loadu_si256((const __m256i*)(RefPtrLm + Offset)));
        const __m256i DiffCb  = _mm256_sub_epi16(TstCb, _mm256_loadu_si256((const __m256i*)(RefPtrCb + Offset)));
        const __m256i DiffCr  = _mm256_sub_epi16(TstCr, _mm256_loadu_si256((const __m256i*)(RefPtrCr + Offset)));
        const __m256i WDiffLm = _mm256_mullo_epi16(DiffLm, WeightLm);
        const __m256i WDiffCb = _mm256_mullo_epi16(DiffCb, WeightCb);
        const __m256i WDiffCr = _mm256_mullo_epi16(DiffCr, WeightCr);
        const __m256i PairDE  = _mm256_blend_epi16(DiffLm, _mm256_slli_epi32(DiffCb, 16), 0xAA);
        const __m256i PairDO  = _mm256_blend_epi16(_mm256_srli_epi32(DiffLm, 16), DiffCb, 0xAA);
        const __m256i PairWE  = _mm256_blend_epi16(WDiffLm, _mm256_slli_epi32(WDiffCb, 16), 0xAA);
        const __m256i PairWO  = _mm256_blend_epi16(_mm256_srli_epi32(WDiffLm, 16), WDiffCb, 0xAA);
        const __m256i ErrorE  = _mm256_add_epi32(_mm256_madd_epi16(PairDE, PairWE), _mm256_madd_epi16(DiffCr, _mm256_and_si256(WDiffCr, MaskLo)));
        const __m256i ErrorO  = _mm256_add_epi32(_mm256_madd_epi16(PairDO, PairWO), _mm256_madd_epi16(DiffCr, _mm256_and_si256(WDiffCr, MaskHi)));
        const __m256i NewBestErrorE = _mm256_min_epu32(BestErrorE, ErrorE);
        const __m256i NewBestErrorO = _mm256_min_epu32(BestErrorO, ErrorO);
        BestOffsetE = _mm256_blendv_epi8(CandOffset, BestOffsetE, _mm256_cmpeq_epi32(NewBestErrorE, BestErrorE));
        BestOffsetO = _mm256_blendv_epi8(CandOffset, BestOffsetO, _mm256_cmpeq_epi32(NewBestErrorO, BestErrorO));
        BestErrorE  = NewBestErrorE;
        BestErrorO  = NewBestErrorO;
        CandOffset  = _mm256_add_epi32(CandOffset, One);
      } //dx
    } //dy

    alignas(alignof(__m256i)) int32 BestRefOffsetsE[8], BestRefOffsetsO[8];
    _mm256_store_si256((__m256i*)BestRefOffsetsE, _mm256_add_epi32(BestOffsetE, LaneIdxE));
    _mm256_store_si256((__m256i*)BestRefOffsetsO, _mm256_add_epi32(BestOffsetO, LaneIdxO));
    for(int32 i = 0; i < 16; i++)
    {
      const int32 BestRefOffset = (i & 1) ? BestRefOffsetsO[i >> 1] : BestRefOffsetsE[i >> 1];
      DstPtrLm[x + i] = (uint16)xClipU((int32)RefPtrLm[BestRefOffset] - GlobalColorShift[0], MaxValue);
      DstPtrCb[x + i] = (uint16)xClipU((int32)RefPtrCb[BestRefOffset] - GlobalColorShift[1], MaxValue);
      DstPtrCr[x + i] = (uint16)xClipU((int32)RefPtrCr[BestRefOffset] - GlobalColorShift[2], MaxValue);
    }
  } //x

  if(Width16 < Width) { xGenShftCompRowTail(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights, Width16); }
}

void xCorrespPixelShiftAVX::GenShftCompRowU32(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
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

  const __m256i ShiftLm  = _mm256_set1_epi32(GlobalColorShift[0]), ShiftCb  = _mm256_set1_epi32(GlobalColorShift[1]), ShiftCr  = _mm256_set1_epi32(GlobalColorShift[2]);
  const __m256i WeightLm = _mm256_set1_epi32(CmpWeights[0])      , WeightCb = _mm256_set1_epi32(CmpWeights[1])      , WeightCr = _mm256_set1_epi32(CmpWeights[2]);
  const __m256i One      = _mm256_set1_epi32(1);
  const __m256i LaneIdx  = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);

  for(int32 x = 0; x < Width8; x += 8)
  {
    const __m256i TstLm = _mm256_add_epi32(_mm256_cvtepu16_epi32(_mm_loadu_si128((const __m128i*)(TstPtrLm + x))), ShiftLm);
    const __m256i TstCb = _mm256_add_epi32(_mm256_cvtepu16_epi32(_mm_loadu_si128((const __m128i*)(TstPtrCb + x))), ShiftCb);
    const __m256i TstCr = _mm256_add_epi32(_mm256_cvtepu16_epi32(_mm_loadu_si128((const __m128i*)(TstPtrCr + x))), ShiftCr);
    __m256i BestError  = _mm256_set1_epi32(-1); //0xFFFFFFFF
    __m256i BestOffset = _mm256_set1_epi32(NOT_VALID);

    for(int32 dy = -SearchRange; dy <= SearchRange; dy++)
    {
      const int32 RowOffset = (y + dy) * RefStride + x - SearchRange;
      __m256i CandOffset = _mm256_set1_epi32(RowOffset);
      for(int32 dx = 0; dx < WindowSize; dx++)
      {
        const int32   Offset = RowOffset + dx;
        const __m256i DiffLm = _mm256_sub_epi32(TstLm, _mm256_cvtepu16_epi32(_mm_loadu_si128((const __m128i*)(RefPtrLm + Offset))));
        const __m256i DiffCb = _mm256_sub_epi32(TstCb, _mm256_cvtepu16_epi32(_mm_loadu_si128((const __m128i*)(RefPtrCb + Offset))));
        const __m256i DiffCr = _mm256_sub_epi32(TstCr, _mm256_cvtepu16_epi32(_mm_loadu_si128((const __m128i*)(RefPtrCr + Offset))));
        const __m256i Error  = _mm256_add_epi32(_mm256_add_epi32(_mm256_mullo_epi32(_mm256_mullo_epi32(DiffLm, DiffLm), WeightLm),
                                                                 _mm256_mullo_epi32(_mm256_mullo_epi32(DiffCb, DiffCb), WeightCb)),
                                                                 _mm256_mullo_epi32(_mm256_mullo_epi32(DiffCr, DiffCr), WeightCr));
        const __m256i NewBestError = _mm256_min_epu32(BestError, Error);
        BestOffset = _mm256_blendv_epi8(CandOffset, BestOffset, _mm256_cmpeq_epi32(NewBestError, BestError));
        BestError  = NewBestError;
        CandOffset = _mm256_add_epi32(CandOffset, One);
      } //dx
    } //dy

    alignas(alignof(__m256i)) int32 BestRefOffsets[8];
    _mm256_store_si256((__m256i*)BestRefOffsets, _mm256_add_epi32(BestOffset, LaneIdx));
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

#endif //X_SIMD_CAN_USE_AVX
