/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xCorrespPixelShiftSSE.h"
#include "xHelpersSIMD.h"

#if X_SIMD_CAN_USE_SSE

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xCorrespPixelShiftSSE::GenShftCompRowI16(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
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

  const __m128i ShiftLm  = _mm_set1_epi16((short)GlobalColorShift[0]), ShiftCb  = _mm_set1_epi16((short)GlobalColorShift[1]), ShiftCr  = _mm_set1_epi16((short)GlobalColorShift[2]);
  const __m128i WeightLm = _mm_set1_epi16((short)CmpWeights[0])      , WeightCb = _mm_set1_epi16((short)CmpWeights[1])      , WeightCr = _mm_set1_epi16((short)CmpWeights[2]);
  const __m128i MaskLo   = _mm_set1_epi32(0x0000FFFF), MaskHi = _mm_set1_epi32((int32)0xFFFF0000);
  const __m128i One      = _mm_set1_epi32(1);
  const __m128i LaneIdxE = _mm_setr_epi32(0, 2, 4, 6), LaneIdxO = _mm_setr_epi32(1, 3, 5, 7);

  for(int32 x = 0; x < Width8; x += 8)
  {
    const __m128i TstLm = _mm_add_epi16(_mm_loadu_si128((const __m128i*)(TstPtrLm + x)), ShiftLm);
    const __m128i TstCb = _mm_add_epi16(_mm_loadu_si128((const __m128i*)(TstPtrCb + x)), ShiftCb);
    const __m128i TstCr = _mm_add_epi16(_mm_loadu_si128((const __m128i*)(TstPtrCr + x)), ShiftCr);
    __m128i BestErrorE  = _mm_set1_epi32(-1), BestErrorO  = BestErrorE;
    __m128i BestOffsetE = _mm_set1_epi32(NOT_VALID), BestOffsetO = BestOffsetE;

    for(int32 dy = -SearchRange; dy <= SearchRange; dy++)
    {
      const int32 RowOffset = (y + dy) * RefStride + x - SearchRange;
      __m128i CandOffset = _mm_set1_epi32(RowOffset);
      for(int32 dx = 0; dx < WindowSize; dx++)
      {
        const int32   Offset  = RowOffset + dx;
        const __m128i DiffLm  = _mm_sub_epi16(TstLm, _mm_loadu_si128((const __m128i*)(RefPtrLm + Offset)));
        const __m128i DiffCb  = _mm_sub_epi16(TstCb, _mm_loadu_si128((const __m128i*)(RefPtrCb + Offset)));
        const __m128i DiffCr  = _mm_sub_epi16(TstCr, _mm_loadu_si128((const __m128i*)(RefPtrCr + Offset)));
        const __m128i WDiffLm = _mm_mullo_epi16(DiffLm, WeightLm);
        const __m128i WDiffCb = _mm_mullo_epi16(DiffCb, WeightCb);
        const __m128i WDiffCr = _mm_mullo_epi16(DiffCr, WeightCr);
        const __m128i PairDE  = _mm_blend_epi16(DiffLm, _mm_slli_epi32(DiffCb, 16), 0xAA);              //int32 j: (Lm[2j  ], Cb[2j  ])
        const __m128i PairDO  = _mm_blend_epi16(_mm_srli_epi32(DiffLm, 16), DiffCb, 0xAA);              //int32 j: (Lm[2j+1], Cb[2j+1])
        const __m128i PairWE  = _mm_blend_epi16(WDiffLm, _mm_slli_epi32(WDiffCb, 16), 0xAA);
        const __m128i PairWO  = _mm_blend_epi16(_mm_srli_epi32(WDiffLm, 16), WDiffCb, 0xAA);
        const __m128i ErrorE  = _mm_add_epi32(_mm_madd_epi16(PairDE, PairWE), _mm_madd_epi16(DiffCr, _mm_and_si128(WDiffCr, MaskLo)));
        const __m128i ErrorO  = _mm_add_epi32(_mm_madd_epi16(PairDO, PairWO), _mm_madd_epi16(DiffCr, _mm_and_si128(WDiffCr, MaskHi)));
        const __m128i NewBestErrorE = _mm_min_epu32(BestErrorE, ErrorE);
        const __m128i NewBestErrorO = _mm_min_epu32(BestErrorO, ErrorO);
        BestOffsetE = _mm_blendv_epi8(CandOffset, BestOffsetE, _mm_cmpeq_epi32(NewBestErrorE, BestErrorE));
        BestOffsetO = _mm_blendv_epi8(CandOffset, BestOffsetO, _mm_cmpeq_epi32(NewBestErrorO, BestErrorO));
        BestErrorE  = NewBestErrorE;
        BestErrorO  = NewBestErrorO;
        CandOffset  = _mm_add_epi32(CandOffset, One);
      } //dx
    } //dy

    alignas(alignof(__m128i)) int32 BestRefOffsetsE[4], BestRefOffsetsO[4];
    _mm_store_si128((__m128i*)BestRefOffsetsE, _mm_add_epi32(BestOffsetE, LaneIdxE));
    _mm_store_si128((__m128i*)BestRefOffsetsO, _mm_add_epi32(BestOffsetO, LaneIdxO));
    for(int32 i = 0; i < 8; i++)
    {
      const int32 BestRefOffset = (i & 1) ? BestRefOffsetsO[i >> 1] : BestRefOffsetsE[i >> 1];
      DstPtrLm[x + i] = (uint16)xClipU((int32)RefPtrLm[BestRefOffset] - GlobalColorShift[0], MaxValue);
      DstPtrCb[x + i] = (uint16)xClipU((int32)RefPtrCb[BestRefOffset] - GlobalColorShift[1], MaxValue);
      DstPtrCr[x + i] = (uint16)xClipU((int32)RefPtrCr[BestRefOffset] - GlobalColorShift[2], MaxValue);
    }
  } //x

  if(Width8 < Width) { xGenShftCompRowTail(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights, Width8); }
}
void xCorrespPixelShiftSSE::GenShftCompRowU32(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 Width  = Tst->getWidth();
  const int32 Width4 = (int32)((uint32)Width & c_MultipleMask4<uint32>);

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

  const __m128i ShiftLm  = _mm_set1_epi32(GlobalColorShift[0]), ShiftCb  = _mm_set1_epi32(GlobalColorShift[1]), ShiftCr  = _mm_set1_epi32(GlobalColorShift[2]);
  const __m128i WeightLm = _mm_set1_epi32(CmpWeights[0])      , WeightCb = _mm_set1_epi32(CmpWeights[1])      , WeightCr = _mm_set1_epi32(CmpWeights[2]);
  const __m128i One      = _mm_set1_epi32(1);
  const __m128i LaneIdx  = _mm_setr_epi32(0, 1, 2, 3);

  for(int32 x = 0; x < Width4; x += 4)
  {
    const __m128i TstLm = _mm_add_epi32(_mm_cvtepu16_epi32(_mm_loadl_epi64((const __m128i*)(TstPtrLm + x))), ShiftLm);
    const __m128i TstCb = _mm_add_epi32(_mm_cvtepu16_epi32(_mm_loadl_epi64((const __m128i*)(TstPtrCb + x))), ShiftCb);
    const __m128i TstCr = _mm_add_epi32(_mm_cvtepu16_epi32(_mm_loadl_epi64((const __m128i*)(TstPtrCr + x))), ShiftCr);
    __m128i BestError  = _mm_set1_epi32(-1); //0xFFFFFFFF
    __m128i BestOffset = _mm_set1_epi32(NOT_VALID);

    for(int32 dy = -SearchRange; dy <= SearchRange; dy++)
    {
      const int32 RowOffset = (y + dy) * RefStride + x - SearchRange;
      __m128i CandOffset = _mm_set1_epi32(RowOffset);
      for(int32 dx = 0; dx < WindowSize; dx++)
      {
        const int32   Offset = RowOffset + dx;
        const __m128i DiffLm = _mm_sub_epi32(TstLm, _mm_cvtepu16_epi32(_mm_loadl_epi64((const __m128i*)(RefPtrLm + Offset))));
        const __m128i DiffCb = _mm_sub_epi32(TstCb, _mm_cvtepu16_epi32(_mm_loadl_epi64((const __m128i*)(RefPtrCb + Offset))));
        const __m128i DiffCr = _mm_sub_epi32(TstCr, _mm_cvtepu16_epi32(_mm_loadl_epi64((const __m128i*)(RefPtrCr + Offset))));
        const __m128i Error  = _mm_add_epi32(_mm_add_epi32(_mm_mullo_epi32(_mm_mullo_epi32(DiffLm, DiffLm), WeightLm),
                                                           _mm_mullo_epi32(_mm_mullo_epi32(DiffCb, DiffCb), WeightCb)),
                                                           _mm_mullo_epi32(_mm_mullo_epi32(DiffCr, DiffCr), WeightCr));
        const __m128i NewBestError = _mm_min_epu32(BestError, Error);
        BestOffset = _mm_blendv_epi8(CandOffset, BestOffset, _mm_cmpeq_epi32(NewBestError, BestError)); //min unchanged -> keep
        BestError  = NewBestError;
        CandOffset = _mm_add_epi32(CandOffset, One);
      } //dx
    } //dy

    alignas(alignof(__m128i)) int32 BestRefOffsets[4];
    _mm_store_si128((__m128i*)BestRefOffsets, _mm_add_epi32(BestOffset, LaneIdx));
    for(int32 i = 0; i < 4; i++)
    {
      const int32 BestRefOffset = BestRefOffsets[i];
      DstPtrLm[x + i] = (uint16)xClipU((int32)RefPtrLm[BestRefOffset] - GlobalColorShift[0], MaxValue);
      DstPtrCb[x + i] = (uint16)xClipU((int32)RefPtrCb[BestRefOffset] - GlobalColorShift[1], MaxValue);
      DstPtrCr[x + i] = (uint16)xClipU((int32)RefPtrCr[BestRefOffset] - GlobalColorShift[2], MaxValue);
    }
  } //x

  if(Width4 < Width) { xGenShftCompRowTail(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights, Width4); }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_SSE
