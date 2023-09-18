/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski   <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2018-2019 Adrian Dziembowski <adrian.dziembowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xBaseIV.h"
#include <cassert>
#include <numeric>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xBaseIV
//===============================================================================================================================================================================================================

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q planar
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int32V4 xBaseIV::xCalcDistAsymmetricRow(const xPicP* Ref, const xPicP* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32  Width     = Tst->getWidth();
  const int32  TstStride = Tst->getStride();
  const int32  TstOffset = y * TstStride;

  int32V4 RowDist = { 0, 0, 0, 0 };

  const uint16* TstPtrY = Tst->getAddr(eCmp::LM) + TstOffset;
  const uint16* TstPtrU = Tst->getAddr(eCmp::CB) + TstOffset;
  const uint16* TstPtrV = Tst->getAddr(eCmp::CR) + TstOffset;

  for(int32 x = 0; x < Width; x++)
  {
    const int32V4 CurrTstValue  = int32V4((int32)(TstPtrY[x]), (int32)(TstPtrU[x]), (int32)(TstPtrV[x]), 0) + GlobalColorShift;
    const int32   BestRefOffset = xFindBestPixelWithinBlock(Ref, CurrTstValue, x, y, SearchRange, CmpWeights);

    for(uint32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
      const uint16* RefAddr = Ref->getAddr((eCmp)CmpIdx);
      int32 Diff = CurrTstValue[CmpIdx] - (int32)(RefAddr[BestRefOffset]);
      int32 Dist = xPow2(Diff);
      RowDist[CmpIdx] += Dist;
    }
  }//x

  return RowDist;
}
int32 xBaseIV::xFindBestPixelWithinBlock(const xPicP* Ref, const int32V4& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 BegY = CenterY - SearchRange;
  const int32 EndY = CenterY + SearchRange;
  const int32 BegX = CenterX - SearchRange;
  const int32 EndX = CenterX + SearchRange;

  const uint16* RefPtrY = Ref->getAddr  (eCmp::LM);
  const uint16* RefPtrU = Ref->getAddr  (eCmp::CB);
  const uint16* RefPtrV = Ref->getAddr  (eCmp::CR);
  const int32   Stride  = Ref->getStride();

  int32 BestError  = std::numeric_limits<int32>::max();
  int32 BestOffset = NOT_VALID;

  for(int32 y = BegY; y <= EndY; y++)
  {
    for(int32 x = BegX; x <= EndX; x++)
    {
      const int32 Offset = y * Stride + x;
      const int32 DistY  = xPow2(TstPel[0] - (int32)(RefPtrY[Offset]));
      const int32 DistU  = xPow2(TstPel[1] - (int32)(RefPtrU[Offset]));
      const int32 DistV  = xPow2(TstPel[2] - (int32)(RefPtrV[Offset]));
      if constexpr (c_UseRuntimeCmpWeights)
      {
        const int32 Error = DistY * CmpWeights[0] + DistU * CmpWeights[1] + DistV * CmpWeights[2];
        if(Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
      else
      {
        const int32 Error = (DistY << 2) + DistU + DistV;
        if (Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
    } //x
  } //y

  return BestOffset;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q interleaved - STD
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int32V4 xBaseIV::xCalcDistAsymmetricRow_STD(const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32  Width     = Tst->getWidth ();
  const int32  TstStride = Tst->getStride();
  const int32  TstOffset = y * TstStride;

  int32V4 RowDist = { 0, 0, 0, 0 };

  const uint16V4* TstPtr  = Tst->getAddr() + TstOffset;
        
  for(int32 x = 0; x < Width; x++)
  {
    const int32V4 CurrTstValue  = (int32V4)(TstPtr[x]) + GlobalColorShift;
    const int32   BestRefOffset = xFindBestPixelWithinBlock_STD(Ref, CurrTstValue, x, y, SearchRange, CmpWeights);
    const int32V4 Diff = CurrTstValue - (int32V4)(Ref->getAddr()[BestRefOffset]);
    const int32V4 Dist = Diff.getVecPow2();
    RowDist += Dist;
  }//x

  return RowDist;
}
int32 xBaseIV::xFindBestPixelWithinBlock_STD(const xPicI* Ref, const int32V4& TstPel, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32 BegY = CenterY - SearchRange;
  const int32 EndY = CenterY + SearchRange;
  const int32 BegX = CenterX - SearchRange;
  const int32 EndX = CenterX + SearchRange;

  const uint16V4* RefPtr = Ref->getAddr  ();
  const int32     Stride = Ref->getStride();

  int32 BestError  = std::numeric_limits<int32>::max();
  int32 BestOffset = NOT_VALID;

  for(int32 y = BegY; y <= EndY; y++)
  {
    for(int32 x = BegX; x <= EndX; x++)
    {
      const int32   Offset = y * Stride + x;
      const int32V4 RefPel = (int32V4)(RefPtr[Offset]);
      const int32V4 Dist   = (TstPel - RefPel).getVecPow2();
      if constexpr (c_UseRuntimeCmpWeights)
      {
        const int32 Error = (Dist * CmpWeights).getSum();
        if (Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
      else
      {
        const int32 Error = (Dist[0] << 2) + Dist[1] + Dist[2];
        if (Error < BestError) { BestError = Error; BestOffset = Offset; }
      }
    } //x
  } //y

  return BestOffset;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// asymetric Q interleaved - SSE
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#if X_IVPSNR_CAN_USE_SSE
int32V4 xBaseIV::xCalcDistAsymmetricRow_SSE(const xPicI* Ref, const xPicI* Tst, const int32 y, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights)
{
  const int32  Width     = Tst->getWidth();
  const int32  TstStride = Tst->getStride();
  const int32  TstOffset = y * TstStride;
  const __m128i CmpWeightsV       = _mm_loadu_si128((__m128i*) &CmpWeights);
  const __m128i GlobalColorShiftV = _mm_loadu_si128((__m128i*) &GlobalColorShift);

  const uint16V4* TstPtr = Tst->getAddr() + TstOffset;
  __m128i RowDistV = _mm_setzero_si128();
  for (int32 x = 0; x < Width; x++)
  {
    __m128i TstU16V  = _mm_loadl_epi64((__m128i*)(TstPtr + x));
    __m128i TstV     = _mm_add_epi32(_mm_unpacklo_epi16(TstU16V, _mm_setzero_si128()), GlobalColorShiftV);
    __m128i BestDist = xCalcDistWithinBlock_SSE(Ref, TstV, x, y, SearchRange, CmpWeightsV);
    RowDistV = _mm_add_epi32(RowDistV, BestDist);
  }//x

  int32V4 RowDist;
  _mm_storeu_si128((__m128i*)&RowDist, RowDistV);
  return RowDist;
}
__m128i xBaseIV::xCalcDistWithinBlock_SSE(const xPicI* Ref, const __m128i& TstPelV, const int32 CenterX, const int32 CenterY, const int32 SearchRange, const __m128i& CmpWeightsV)
{
  const int32 WindowSize = 2 * SearchRange + 1;
  const int32 BegY = CenterY - SearchRange;
  const int32 BegX = CenterX - SearchRange;

  const int32     Stride = Ref->getStride();
  const uint16V4* RefPtr = Ref->getAddr() + BegY * Stride + BegX;

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
#endif //X_IVPSNR_CAN_USE_SSE

//===============================================================================================================================================================================================================

} //end of namespace PMBB