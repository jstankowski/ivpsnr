/*
    SPDX-FileCopyrightText: 2024-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xTestUtilsIVQM.h"
#include "xDistortion.h"
#include "xTestUtils.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xTestUtilsIVQM::addConst(uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 Value)
{
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    { 
      Dst[x] = (uint16)(Src[x] + Value);
    }
    Src += SrcStride;
    Dst += DstStride;
  }
}

uint32 xTestUtilsIVQM::addBlockNoise(uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height, int32 /*BitDepth*/, uint32 State)
{
  static constexpr int32 c_Log2BlockSize = 2;
  static constexpr int32 c_BlockSize     = 1<<c_Log2BlockSize;

  for(int32 y = 0; y < Height; y += c_BlockSize)
  {
    uint16*       RowDst = Dst + y * DstStride;
    const uint16* RowSrc = Src + y * SrcStride;

    for(int32 x = 0; x < Width; x += c_BlockSize)
    {
      uint16*       BlkDst = RowDst + x;
      const uint16* BlkSrc = RowSrc + x;
      for(int32 by = 0; by < c_BlockSize; by++)
      {
        for(int32 bx = 0; bx < c_BlockSize; bx++)
        {
          BlkDst[by*DstStride + bx] = BlkSrc[by*SrcStride + bx];
        }
      }

      State = xTestUtils::xXorShift32(State);

      uint32 MoveY     = (State & 0b00000000000000000000000000001111) >> 0;
      uint32 MoveX     = (State & 0b00000000000000000000000011110000) >> 4;
      uint32 AddCoordY = (State & 0b00000000000000000000001100000000) >> 8;
      uint32 AddCoordX = (State & 0b00000000000000000011000000000000) >> 12;
      uint32 AddValue  = (State & 0b00000000000000110000000000000000) >> 16;
      
      if(MoveY & 0x1110)
      {
        int32 MoveDir = MoveY & 0x1 ? 1 : -1;
        BlkSrc += SrcStride * MoveDir;
      }

      if(MoveX & 0x1110)
      {
        int32 MoveDir = MoveX & 0x1 ? 1 : -1;
        BlkSrc += MoveDir;
      }

      for(int32 by = 1; by < c_BlockSize-1; by++)
      {
        for(int32 bx = 1; bx < c_BlockSize-1; bx++)
        {
          BlkDst[by * DstStride + bx] = BlkSrc[by * SrcStride + bx];
        }
      }

      int32 Delta = ((int32)AddValue) - 1;

      BlkDst[AddCoordY * DstStride + AddCoordX] = (uint16)(BlkDst[AddCoordY * DstStride + AddCoordX] + Delta);
    }
  }

  return State;
}

uint64V4 xTestUtilsIVQM::calcPicSSD(const xPicP* Tst, const xPicP* Ref)
{
  const int32   Width     = Ref->getWidth   ();
  const int32   Height    = Ref->getHeight  ();
  const int32   TstStride = Tst->getStride  ();
  const int32   RefStride = Ref->getStride  ();
  const int32   BitDepth  = Ref->getBitDepth();

  uint64V4 SSDs = xMakeVec4<uint64>(0);
  SSDs[0] = xDistortion::CalcSSD(Tst->getAddr(eCmp::C0), Ref->getAddr(eCmp::C0), TstStride, RefStride, Width, Height, BitDepth);
  SSDs[1] = xDistortion::CalcSSD(Tst->getAddr(eCmp::C1), Ref->getAddr(eCmp::C1), TstStride, RefStride, Width, Height, BitDepth);
  SSDs[2] = xDistortion::CalcSSD(Tst->getAddr(eCmp::C2), Ref->getAddr(eCmp::C2), TstStride, RefStride, Width, Height, BitDepth);

  return SSDs;
}

uint64V4 xTestUtilsIVQM::calcPicSSD(const xPicI* Tst, const xPicI* Ref)
{
  const int32     Width     = Ref->getWidth   ();
  const int32     Height    = Ref->getHeight  ();
  const uint16V4* TstPtr    = Tst->getAddr    ();
  const uint16V4* RefPtr    = Ref->getAddr    ();
  const int32     TstStride = Tst->getStride  ();
  const int32     RefStride = Ref->getStride  ();

  uint64V4 SSDs = xMakeVec4<uint64>(0);

  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Width; x++)
    {
      int64V4 Diff = (int64V4)(TstPtr[x]) - (int64V4)(RefPtr[x]);
      int64V4 SSD = Diff * Diff;
      SSDs += (uint64V4)SSD;
    }
    TstPtr += TstStride;
    RefPtr += RefStride;
  }

  return SSDs;
}

uint64V4 xTestCalcDistAsymmetricPicP(const xPicP* Tst, const xPicP* OrgP, const int32V4& GlobalColorDiff, int32 SearchRange, const int32V4& CmpWeightsSearch, fCalcDistAsymmetricRowP CalcDistAsymmetricRow)
{
  uint64V4 DistsV4 = xMakeVec4<uint64>(0);
  for(int32 y = 0; y < OrgP->getHeight(); y++)
  {
    DistsV4 += CalcDistAsymmetricRow(Tst, OrgP, y, GlobalColorDiff, SearchRange, CmpWeightsSearch);
  }
  return DistsV4;
}
uint64V4 xTestCalcDistAsymmetricPicI(const xPicI* Tst, const xPicI* OrgP, const int32V4& GlobalColorDiff, int32 SearchRange, const int32V4& CmpWeightsSearch, fCalcDistAsymmetricRowI CalcDistAsymmetricRow)
{
  uint64V4 DistsV4 = xMakeVec4<uint64>(0);
  for(int32 y = 0; y < OrgP->getHeight(); y++)
  {
    DistsV4 += CalcDistAsymmetricRow(Tst, OrgP, y, GlobalColorDiff, SearchRange, CmpWeightsSearch);
  }
  return DistsV4;
}
void xTestGenShftCompPicP(xPicP* DstRef, const xPicP* Ref, const xPicP* Tst, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, fGenShftCompRowP GenShftCompRow)
{
  for(int32 y = 0; y < Ref->getHeight(); y++)
  {
    GenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights);
  }
}
void xTestGenShftCompPicI(xPicI* DstRef, const xPicI* Ref, const xPicI* Tst, const int32V4& GlobalColorShift, const int32 SearchRange, const int32V4& CmpWeights, fGenShftCompRowI GenShftCompRow)
{
  for(int32 y = 0; y < Ref->getHeight(); y++)
  {
    GenShftCompRow(DstRef, Ref, Tst, y, GlobalColorShift, SearchRange, CmpWeights);
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB