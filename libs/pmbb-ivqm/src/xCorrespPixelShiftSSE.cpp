/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xCorrespPixelShiftSSE.h"

#if X_SIMD_CAN_USE_SSE

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//  asymetric Q interleaved
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint64V4 xCorrespPixelShiftSSE::CalcDistAsymmetricRow(const xPicI* Tst, const xPicI* Ref, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  assert(Tst->isCompatible(Ref));

  const int32  Width     = Tst->getWidth ();
  const int32  TstStride = Tst->getStride();
  const int32  TstOffset = y * TstStride;
  const __m128i CmpWeightsV       = _mm_loadu_si128((__m128i*) &CmpWeights);
  const __m128i GlobalColorShiftV = _mm_loadu_si128((__m128i*) &GlobalColorShift);

  const uint16V4* TstPtr = Tst->getAddr() + TstOffset;
  __m128i RowDistV = _mm_setzero_si128();
  for (int32 x = 0; x < Width; x++)
  {
    __m128i TstU16V  = _mm_loadl_epi64((__m128i*)(TstPtr + x));
    __m128i TstV     = _mm_add_epi32(_mm_unpacklo_epi16(TstU16V, _mm_setzero_si128()), GlobalColorShiftV); //TODO - xc_CLIP_CURR_TST_RANGE
    __m128i BestDist = xCalcDistWithinBlock(TstV, Ref, x, y, SearchRange, CmpWeightsV);
    RowDistV = _mm_add_epi32(RowDistV, BestDist);
  }//x

  int32V4 RowDist;
  _mm_storeu_si128((__m128i*)&RowDist, RowDistV);
  return (uint64V4)RowDist;
}
__m128i xCorrespPixelShiftSSE::xCalcDistWithinBlock(const __m128i& TstPelV, const xPicI* Ref, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const __m128i& CmpWeightsV)
{
  const int32 WindowSize = 2 * SearchRange + 1;
  const int32 BegY = CenterY - SearchRange;
  const int32 BegX = CenterX - SearchRange;

  const int32     Stride = Ref->getStride();
  const uint16V4* RefPtr = Ref->getAddr  () + BegY * Stride + BegX;

  int32   BestError = std::numeric_limits<int32>::max();
  __m128i BestDistV = _mm_setzero_si128();

  for (int32 y = 0; y < WindowSize; y++)
  {
    const uint16V4* RefPtrY = RefPtr + y * Stride;
    for (int32 x = 0; x < WindowSize; x++)
    {
      __m128i RefU16V = _mm_loadl_epi64((__m128i*)(RefPtrY + x));
    //__m128i RefV    = _mm_unpacklo_epi16(RefU16V, _mm_setzero_si128());
      __m128i RefV    = _mm_cvtepu16_epi32(RefU16V);
      __m128i DiffV   = _mm_sub_epi32     (TstPelV, RefV);
      __m128i DistV   = _mm_mullo_epi32   (DiffV, DiffV);
      __m128i ErrorV  = _mm_mullo_epi32   (DistV, CmpWeightsV);
      __m128i Tmp1    = _mm_hadd_epi32    (ErrorV, ErrorV);
      __m128i Tmp2    = _mm_hadd_epi32    (Tmp1, Tmp1);
      int32   Error   = _mm_extract_epi32 (Tmp2, 0);
      if (Error < BestError) { BestError = Error; BestDistV = DistV; }
    } //x
  } //y

  return BestDistV;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// shift-compensated picture generation
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void xCorrespPixelShiftSSE::GenShftCompRow(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 Width     = Tst->getWidth ();
  const int32 TstStride = Tst->getStride();
  const int32 TstOffset = y * TstStride;
  const int32 MaxValue  = DstRef->getMaxPelValue();

  const __m128i CmpWeightsI32V       = _mm_loadu_si128((__m128i*)(&CmpWeights      ));
  const __m128i GlobalColorShiftI32V = _mm_loadu_si128((__m128i*)(&GlobalColorShift));
  const __m128i MaxValueI32V         = _mm_set1_epi32(MaxValue);

  const uint16V4*    TstPtr = Tst   ->getAddr() + TstOffset;
  uint16V4* restrict DstPtr = DstRef->getAddr() + TstOffset;

  for(int32 x = 0; x < Width; x++)
  {
    __m128i TstU16V = _mm_loadl_epi64((__m128i*)(TstPtr + x));
    __m128i TstI32V = _mm_add_epi32(_mm_unpacklo_epi16(TstU16V, _mm_setzero_si128()), GlobalColorShiftI32V); //TODO - xc_CLIP_CURR_TST_RANGE
    __m128i RefI32V = xFindBestPixelWithinBlock(TstI32V, Ref, x, y, SearchRange, CmpWeightsI32V);
    __m128i DstI32V = _mm_min_epi32(_mm_sub_epi32(RefI32V, GlobalColorShiftI32V), MaxValueI32V);
    __m128i DstU16V = _mm_packus_epi32(DstI32V, DstI32V);
    _mm_storel_epi64((__m128i*)(DstPtr + x), DstU16V);
  }//x
}
__m128i xCorrespPixelShiftSSE::xFindBestPixelWithinBlock(const __m128i& TstPelI32V, const xPicI* Ref, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const __m128i& CmpWeightsI32V)
{
  const int32 BegY = CenterY - SearchRange;
  const int32 EndY = CenterY + SearchRange;
  const int32 BegX = CenterX - SearchRange;
  const int32 EndX = CenterX + SearchRange;

  const uint16V4* RefPtr = Ref->getAddr  ();
  const int32     Stride = Ref->getStride();

  int32   BestError     = std::numeric_limits<int32>::max();
  __m128i BestPixelI32V = _mm_setzero_si128();;

  for(int32 y = BegY; y <= EndY; y++)
  {
    for(int32 x = BegX; x <= EndX; x++)
    {
      const int32 Offset = y * Stride + x;

      __m128i RefI32V   = _mm_cvtepu16_epi32(_mm_loadl_epi64((__m128i*)(RefPtr + Offset)));
      __m128i DiffI32V  = _mm_sub_epi32    (TstPelI32V, RefI32V);
      __m128i DistI32V  = _mm_mullo_epi32  (DiffI32V, DiffI32V);
      __m128i ErrorI32V = _mm_mullo_epi32  (DistI32V, CmpWeightsI32V);
      __m128i Tmp1I32   = _mm_hadd_epi32   (ErrorI32V, ErrorI32V);
      __m128i Tmp2I32   = _mm_hadd_epi32   (Tmp1I32, Tmp1I32);
      int32   Error     = _mm_extract_epi32(Tmp2I32, 0);
      if (Error < BestError) { BestError = Error; BestPixelI32V = RefI32V; }
    } //x
  } //y

  return BestPixelI32V;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#endif //X_SIMD_CAN_USE_SSE
