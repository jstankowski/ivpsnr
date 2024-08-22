/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define XSEQ_IMPLEMENTATION
#include "xSeq.h"
#include "xPixelOps.h"
#include "xFile.h"
#include "xMemory.h"
#include <cassert>
#include <cstring>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

bool xSeqBase::isModeAllowed(eMode OpMode)
{
  switch(OpMode)
  {
    case eMode::Read  : return allowsRead (); break;
    case eMode::Write : return allowsWrite(); break;
    case eMode::Append: return allowsRead (); break;
    default: return false;
  }
}
xSeqBase::tResult xSeqBase::openFile(tCSR FileName, eMode OpMode)
{
  if(!isModeAllowed(OpMode)) { return eRetv::NotImplemented; }

  m_OpMode = OpMode;
  return xBackendOpen(FileName, OpMode);
}
xSeqBase::tResult xSeqBase::closeFile()
{
  return xBackendClose();
}
xSeqBase::tResult xSeqBase::readFrame(xPicP* Pic)
{
  if(m_OpMode == eMode::Read && m_CurrFrameIdx >= m_NumOfFrames) { return eRetv::EndOfFile; }
  if(m_OpMode != eMode::Read) { return { eRetv::Error, "OpMode does not allow Read"}; }

  //read frame
  tResult Result = xBackendRead(m_Packed);
  if(!Result) { return Result; }

  //unpack frame
  bool Unpacked = xUnpackFrame(Pic);
  if(!Unpacked) { return eRetv::Error; }

  //update state
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}
xSeqBase::tResult xSeqBase::writeFrame(const xPicP* Pic)
{
  if(m_OpMode != eMode::Write && m_OpMode != eMode::Append) { return { eRetv::Error, "OpMode does not allow Write" }; }

  //pack frame
  bool Packed = xPackFrame(Pic);
  if(!Packed) { return eRetv::Error; }

  //write frame
  tResult Result = xBackendWrite(m_Packed);
  if(!Result) { return Result; }

  //update state
  m_NumOfFrames  += 1;
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}
xSeqBase::tResult xSeqBase::readFrame(xPlane<uint8>* Plane)
{
  if(m_OpMode == eMode::Read && m_CurrFrameIdx >= m_NumOfFrames) { return eRetv::EndOfFile; }
  if(m_OpMode != eMode::Read) { return { eRetv::Error, "OpMode does not allow Read" }; }

  //read frame
  tResult Result = xBackendRead(m_Packed);
  if(!Result) { return Result; }

  //unpack frame
  bool Unpacked = xUnpackFrame(Plane);
  if(!Unpacked) { return eRetv::Error; }

  //update state
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}
xSeqBase::tResult xSeqBase::writeFrame(const xPlane<uint8>* Plane)
{
  if(m_OpMode != eMode::Write && m_OpMode != eMode::Append) { return { eRetv::Error, "OpMode does not allow Write" }; }

  //pack frame
  bool Packed = xPackFrame(Plane);
  if(!Packed) { return eRetv::Error; }

  //write frame
  tResult Result = xBackendWrite(m_Packed);
  if(!Result) { return Result; }

  //update state
  m_NumOfFrames  += 1;
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}
xSeqBase::tResult xSeqBase::readFrame(xPlane<uint16>* Plane)
{
  if(m_OpMode == eMode::Read && m_CurrFrameIdx >= m_NumOfFrames) { return eRetv::EndOfFile; }
  if(m_OpMode != eMode::Read) { return { eRetv::Error, "OpMode does not allow Read" }; }

  //read frame
  tResult Result = xBackendRead(m_Packed);
  if(!Result) { return Result; }

  //unpack frame
  bool Unpacked = xUnpackFrame(Plane);
  if(!Unpacked) { return eRetv::Error; }

  //update state
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}
xSeqBase::tResult xSeqBase::writeFrame(const xPlane<uint16>* Plane)
{
  if(m_OpMode != eMode::Write && m_OpMode != eMode::Append) { return { eRetv::Error, "OpMode does not allow Write" }; }

  //pack frame
  bool Packed = xPackFrame(Plane);
  if(!Packed) { return eRetv::Error; }

  //write frame
  tResult Result = xBackendWrite(m_Packed);
  if(!Result) { return Result; }

  //update state
  m_NumOfFrames  += 1;
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}
xSeqBase::tResult xSeqBase::seekFrame(int32 FrameNumber)
{
  if(m_OpMode == eMode::Read && FrameNumber >= m_NumOfFrames) { return eRetv::WrongArg; }
  if(m_OpMode != eMode::Read) { return eRetv::Error; }

  //seek frame
  tResult Result = xBackendSeek(FrameNumber);
  if(!Result) { return Result; }

  //update state
  m_CurrFrameIdx = FrameNumber;

  return eRetv::Success;
}
xSeqBase::tResult xSeqBase::skipFrame(int32 NumFrames)
{
  int32 NewFrameNumber = m_CurrFrameIdx + NumFrames;
  if(m_OpMode == eMode::Read && NewFrameNumber >= m_NumOfFrames) { return eRetv::WrongArg; }
  if(m_OpMode != eMode::Read) { return eRetv::Error; }

  //seek frame
  tResult Result = xBackendSkip(NumFrames);
  if(!Result) { return Result; }

  //update state
  m_CurrFrameIdx += NumFrames;

  return eRetv::Success;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool xSeqBase::xUnpackFrame(xPicP* Pic)
{
  uint16* PtrLm      = Pic->getAddr  (eCmp::LM);
  uint16* PtrCb      = Pic->getAddr  (eCmp::CB);
  uint16* PtrCr      = Pic->getAddr  (eCmp::CR);
  const int32 Stride = Pic->getStride();
  const int32 Width  = m_Size.getX();
  const int32 Height = m_Size.getY();

  //process luma
  if(m_BytesPerSample == 1) { xPixelOps::Cvt (PtrLm, m_Packed           , Stride, Width, Width, Height); }
  else                      { xPixelOps::Copy(PtrLm, (uint16*)(m_Packed), Stride, Width, Width, Height); }

  //process chroma (if there is any chroma)
  if((int32)m_ChromaFormat > (int32)eCrF::CF400)
  {
    const uint8* ChromaPtr = m_Packed + m_PackedCmpNumBytes;

    if(m_ChromaFormat == eCrF::CF420)
    {
      const int32 ChromaFileCmpNumBytes = m_PackedCmpNumBytes >> 2;
      const int32 ChromaFileStride      = Width >> 1;
      if(m_BytesPerSample == 1)
      {
        xPixelOps::CvtUpsampleHV(PtrCb, ChromaPtr, Stride, ChromaFileStride, Width, Height);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::CvtUpsampleHV(PtrCr, ChromaPtr, Stride, ChromaFileStride, Width, Height);
      }
      else
      {
        xPixelOps::UpsampleHV(PtrCb, (uint16*)ChromaPtr, Stride, ChromaFileStride, Width, Height);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::UpsampleHV(PtrCr, (uint16*)ChromaPtr, Stride, ChromaFileStride, Width, Height);
      }
    }
    else if(m_ChromaFormat == eCrF::CF422)
    {
      const int32 ChromaFileCmpNumBytes = m_PackedCmpNumBytes >> 1;
      const int32 ChromaFileStride      = Width >> 1;
      if(m_BytesPerSample == 1)
      {
        xPixelOps::CvtUpsampleH(PtrCb, ChromaPtr, Stride, ChromaFileStride, Width, Height);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::CvtUpsampleH(PtrCr, ChromaPtr, Stride, ChromaFileStride, Width, Height);
      }
      else
      {
        xPixelOps::UpsampleH(PtrCb, (uint16*)ChromaPtr, Stride, ChromaFileStride, Width, Height);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::UpsampleH(PtrCr, (uint16*)ChromaPtr, Stride, ChromaFileStride, Width, Height);
      }
    }
    else if(m_ChromaFormat == eCrF::CF444)
    {
      if(m_BytesPerSample == 1)
      { 
        xPixelOps::Cvt(PtrCb, ChromaPtr, Stride, Width, Width, Height);
        ChromaPtr += m_PackedCmpNumBytes;
        xPixelOps::Cvt(PtrCr, ChromaPtr, Stride, Width, Width, Height);
      }
      else
      { 
        xPixelOps::Copy(PtrCb, (uint16*)ChromaPtr, Stride, Width, Width, Height);
        ChromaPtr += m_PackedCmpNumBytes;
        xPixelOps::Copy(PtrCr, (uint16*)ChromaPtr, Stride, Width, Width, Height);
      }
    }
    else { return false; }
  }
  return true;
}
bool xSeqBase::xPackFrame(const xPicP* Pic)
{
  const uint16* PtrLm  = Pic->getAddr  (eCmp::LM);
  const uint16* PtrCb  = Pic->getAddr  (eCmp::CB);
  const uint16* PtrCr  = Pic->getAddr  (eCmp::CR);
  const int32   Stride = Pic->getStride();
  const int32   Width  = m_Size.getX();
  const int32   Height = m_Size.getY();

  //process luma
  if(m_BytesPerSample == 1) { xPixelOps::Cvt (m_Packed           , PtrLm, Width, Stride, Width, Height); }
  else                      { xPixelOps::Copy((uint16*)(m_Packed), PtrLm, Width, Stride, Width, Height); }

  //process chroma (if there is any chroma)
  if((int32)m_ChromaFormat > (int32)eCrF::CF400)
  {
    uint8* ChromaPtr = m_Packed + m_PackedCmpNumBytes;
    if(m_ChromaFormat == eCrF::CF420)
    {
      const int32 ChromaFileCmpNumBytes = m_PackedCmpNumBytes >> 2;
      const int32 ChromaFileStride      = Width >> 1;
      if(m_BytesPerSample == 1)
      {
        xPixelOps::CvtDownsampleHV(ChromaPtr, PtrCb, ChromaFileStride, Stride, Width>>1, Height>>1);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::CvtDownsampleHV(ChromaPtr, PtrCr, ChromaFileStride, Stride, Width>>1, Height>>1);
      }
      else
      {
        xPixelOps::DownsampleHV((uint16*)ChromaPtr, PtrCb, ChromaFileStride, Stride, Width>>1, Height>>1);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::DownsampleHV((uint16*)ChromaPtr, PtrCr, ChromaFileStride, Stride, Width>>1, Height>>1);
      }
    }
    else if(m_ChromaFormat == eCrF::CF422)
    {
      const int32 ChromaFileCmpNumBytes = m_PackedCmpNumBytes >> 1;
      const int32 ChromaFileStride      = Width >> 1;
      if(m_BytesPerSample == 1)
      {
        xPixelOps::CvtDownsampleH(ChromaPtr, PtrCb, ChromaFileStride, Stride, Width>>1, Height>>1);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::CvtDownsampleH(ChromaPtr, PtrCr, ChromaFileStride, Stride, Width>>1, Height>>1);
      }
      else
      {
        xPixelOps::DownsampleH((uint16*)ChromaPtr, PtrCb, ChromaFileStride, Stride, Width>>1, Height>>1);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::DownsampleH((uint16*)ChromaPtr, PtrCr, ChromaFileStride, Stride, Width>>1, Height>>1);
      }
    }
    else if(m_ChromaFormat == eCrF::CF444)
    {
      if(m_BytesPerSample == 1)
      { 
        xPixelOps::Cvt(ChromaPtr, PtrCb, Width, Stride, Width, Height);
        ChromaPtr += m_PackedCmpNumBytes;
        xPixelOps::Cvt(ChromaPtr, PtrCr, Width, Stride, Width, Height);
      }
      else
      { 
        xPixelOps::Copy((uint16*)ChromaPtr, PtrCb, Width, Stride, Width, Height);
        ChromaPtr += m_PackedCmpNumBytes;
        xPixelOps::Copy((uint16*)ChromaPtr, PtrCr, Width, Stride, Width, Height);
      }
    }
    else { return false; }
  }
  return true;
}
bool xSeqBase::xUnpackFrame(xPlane<uint8>* Pic)
{
  uint8*      PtrLm  = Pic->getAddr  ();
  const int32 Stride = Pic->getStride();
  const int32 Width  = m_Size.getX();
  const int32 Height = m_Size.getY();

  //process luma
  xPixelOps::Copy(PtrLm, m_Packed, Stride, Width, Width, Height);

  return true;
}
bool xSeqBase::xPackFrame(const xPlane<uint8>* Pic)
{
  const uint8* PtrLm  = Pic->getAddr  ();
  const int32  Stride = Pic->getStride();
  const int32  Width  = m_Size.getX();
  const int32  Height = m_Size.getY();

  //process luma
  xPixelOps::Copy(m_Packed, PtrLm, Width, Stride, Width, Height);

  //process chroma
  uint8* ChromaPtr = m_Packed + m_PackedCmpNumBytes;
  int32  CromaNumPels = 0;

  switch(m_ChromaFormat)
  {
    case eCrF::CF444: CromaNumPels = m_PackedCmpNumPels << 1; break;
    case eCrF::CF422: CromaNumPels = m_PackedCmpNumPels     ; break;
    case eCrF::CF420: CromaNumPels = m_PackedCmpNumPels >> 1; break;
    case eCrF::CF400: CromaNumPels = 0                    ; break;
    default: assert(0); return false;
  }
  if(CromaNumPels)
  {
    xMemsetX((uint8*)ChromaPtr, (uint8)xBitDepth2MidValue(m_BitDepth), CromaNumPels);
  }

  return true;
}
bool xSeqBase::xUnpackFrame(xPlane<uint16>* Pic)
{
  uint16* PtrLm      = Pic->getAddr  ();
  const int32 Stride = Pic->getStride();
  const int32 Width  = m_Size.getX();
  const int32 Height = m_Size.getY();

  //process luma
  if(m_BytesPerSample == 1) { xPixelOps::Cvt (PtrLm, m_Packed           , Stride, Width, Width, Height); }
  else                      { xPixelOps::Copy(PtrLm, (uint16*)(m_Packed), Stride, Width, Width, Height); }

  return true;
}
bool xSeqBase::xPackFrame(const xPlane<uint16>* Pic)
{
  const uint16* PtrLm  = Pic->getAddr  ();
  const int32   Stride = Pic->getStride();
  const int32   Width  = m_Size.getX();
  const int32   Height = m_Size.getY();

  //process luma
  if(m_BytesPerSample == 1) { xPixelOps::Cvt (m_Packed           , PtrLm, Width, Stride, Width, Height); }
  else                      { xPixelOps::Copy((uint16*)(m_Packed), PtrLm, Width, Stride, Width, Height); }

  //process chroma
  uint8* ChromaPtr = m_Packed + m_PackedCmpNumBytes;
  int32  CromaNumPels = 0;

  switch(m_ChromaFormat)
  {
    case eCrF::CF444: CromaNumPels = m_PackedCmpNumPels << 1; break;
    case eCrF::CF422: CromaNumPels = m_PackedCmpNumPels     ; break;
    case eCrF::CF420: CromaNumPels = m_PackedCmpNumPels >> 1; break;
    case eCrF::CF400: CromaNumPels = 0                    ; break;
    default: assert(0); return false;
  }
  if(CromaNumPels)
  {
    if(m_BytesPerSample == 1) { xMemsetX((uint8* )ChromaPtr, (uint8 )xBitDepth2MidValue(m_BitDepth), CromaNumPels); }
    else                      { xMemsetX((uint16*)ChromaPtr, (uint16)xBitDepth2MidValue(m_BitDepth), CromaNumPels); }
  }

  return true;
}

//===============================================================================================================================================================================================================

void xSeq::create(int32V2 Size, int32 BitDepth, eCrF ChromaFormat)
{
  m_Size           = Size;
  m_BitDepth       = BitDepth;
  m_BytesPerSample = m_BitDepth <= 8 ? 1 : 2;
  m_ChromaFormat   = ChromaFormat;

  m_PackedCmpNumPels  = m_Size.getMul();
  m_PackedCmpNumBytes = m_PackedCmpNumPels * m_BytesPerSample;

  switch(m_ChromaFormat)
  {
    case eCrF::CF444: m_PackedImgNumBytes = 3 * m_PackedCmpNumBytes; break;
    case eCrF::CF422: m_PackedImgNumBytes = m_PackedCmpNumBytes << 1; break;
    case eCrF::CF420: m_PackedImgNumBytes = m_PackedCmpNumBytes + (m_PackedCmpNumBytes >> 1); break;
    case eCrF::CF400: m_PackedImgNumBytes = m_PackedCmpNumBytes; break;
    default: assert(0);
  }

  m_Packed = (uint8*)xMemory::xAlignedMallocPageAuto(m_PackedImgNumBytes);
}
void xSeq::destroy()
{
  m_OpMode = eMode::Unknown;
  m_Stream = nullptr;

  m_Size           = { NOT_VALID, NOT_VALID };
  m_BitDepth       = NOT_VALID;
  m_BytesPerSample = NOT_VALID;
  m_ChromaFormat   = eCrF::INVALID;

  m_PackedCmpNumPels  = NOT_VALID;
  m_PackedCmpNumBytes = NOT_VALID;

  if(m_Packed) { xMemory::xAlignedFree(m_Packed); m_Packed = nullptr; }

}
xSeq::tResult xSeq::bindStream(xStream* Stream, const eMode OpMode)
{
  if(Stream == nullptr || !Stream->isValid()) { return eRetv::WrongArg; }
  if(m_OpMode != eMode::Read && m_OpMode != eMode::Write && m_OpMode != eMode::Append) { return eRetv::WrongArg; }
  if(m_Stream != nullptr) { return eRetv::Error; }
  
  m_Stream = Stream;
  m_OpMode = OpMode;

  int64 FileSize = m_Stream->size();
  m_NumOfFrames  = (int32)(FileSize / m_PackedImgNumBytes);
  m_CurrFrameIdx = 0;

  return eRetv::Success;
}
xSeq::tResult xSeq::dropStream()
{
  m_Stream = nullptr;
  m_OpMode = eMode::Unknown;

  m_NumOfFrames  = NOT_VALID;
  m_CurrFrameIdx = NOT_VALID;
  
  return eRetv::Success;
}
xSeq::tResult xSeq::xBackendOpen(tCSR FileName, eMode OpMode)
{
  m_Stream = new xStream();
  switch(OpMode)
  {
    case eMode::Read  : m_Stream->openFile(FileName, xStream::eMode::Read  ); break;
    case eMode::Write : m_Stream->openFile(FileName, xStream::eMode::Write ); break;
    case eMode::Append: m_Stream->openFile(FileName, xStream::eMode::Append); break;
    default: return eRetv::WrongArg;
  }

  if(m_Stream->isValid())
  {
    int64 FileSize = m_Stream->size();
    m_NumOfFrames  = (int32)(FileSize / m_PackedImgNumBytes);
    m_CurrFrameIdx = 0;
  }
  else
  {
    m_NumOfFrames  = NOT_VALID;
    m_CurrFrameIdx = NOT_VALID;
  }
  
  eRetv Result = m_Stream->isValid() ? eRetv::Success : eRetv::Error;
  return Result;
}
xSeq::tResult xSeq::xBackendClose()
{
  m_Stream->closeFile(); delete(m_Stream); m_Stream = nullptr;
  m_OpMode = eMode::Unknown;

  m_NumOfFrames  = NOT_VALID;
  m_CurrFrameIdx = NOT_VALID;

  return eRetv::Success;

}
xSeq::tResult xSeq::xBackendRead(uint8* PackedFrame)
{
  bool ReadOK = m_Stream->read(PackedFrame, m_PackedImgNumBytes);
  return ReadOK ? eRetv::Success : eRetv::Error;
}
xSeq::tResult xSeq::xBackendWrite(uint8* PackedFrame)
{
  bool WriteOK = m_Stream->write(PackedFrame, m_PackedImgNumBytes);
  if(!WriteOK) { return eRetv::Error; }
  if(m_FlushAfterWrite) { m_Stream->flush(); }
  return eRetv::Success;
}
xSeq::tResult xSeq::xBackendSeek(int32 FrameNumber)
{
  uintSize Offset = (uintSize)m_PackedImgNumBytes * (uintSize)FrameNumber;
  bool SeekResult = m_Stream->seekR(Offset, xStream::eSeek::Beg);
  if(!SeekResult) { return eRetv::Error; }
  return eRetv::Success;
}
xSeq::tResult xSeq::xBackendSkip(int32 NumFrames)
{
  uintSize Offset = (uintSize)m_PackedImgNumBytes * (uintSize)NumFrames;
  bool SeekResult = m_Stream->seekR(Offset, xStream::eSeek::Cur);
  if(!SeekResult) { return eRetv::Error; }
  return eRetv::Success;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int32 xSeq::calcSingleFrameSize(int32V2 Size, int32 BitDepth, eCrF ChromaFormat)
{
  int32 BytesPerSample  = BitDepth <= 8 ? 1 : 2;
  int32 FileCmpNumPels  = Size.getMul();
  int32 FileCmpNumBytes = FileCmpNumPels * BytesPerSample;

  int32 FileImgNumBytes = NOT_VALID;
  switch(ChromaFormat)
  {
    case eCrF::CF444: FileImgNumBytes = 3 * FileCmpNumBytes; break;
    case eCrF::CF422: FileImgNumBytes = 2 * FileCmpNumBytes; break;
    case eCrF::CF420: FileImgNumBytes = FileCmpNumBytes + (FileCmpNumBytes >> 1); break;
    case eCrF::CF400: FileImgNumBytes = FileCmpNumBytes; break;
    default: assert(0);
  }

  return FileImgNumBytes;
}
int32 xSeq::calcNumFramesInFile(int32V2 Size, int32 BitDepth, eCrF ChromaFormat, int64 FileSize)
{
  int32 FileImgNumBytes = calcSingleFrameSize(Size, BitDepth, ChromaFormat);
  return (int32)(FileSize / FileImgNumBytes);
}
xSeq::tResult xSeq::dumpFrame(const xPicP* Pic, const std::string& FileName, eCrF ChromaFormat, bool Append)
{
  xSeq Seq(Pic->getSize(), Pic->getBitDepth(), ChromaFormat);
  tResult ResultOpen = Seq.openFile(FileName, Append ? eMode::Append : eMode::Write);
  if(!ResultOpen) { return ResultOpen; }
  tResult ResultWrite = Seq.writeFrame(Pic);
  if(!ResultWrite) { return ResultWrite; }
  tResult ResultClose = Seq.closeFile();
  if(!ResultClose) { return ResultClose; }
  return eRetv::Success;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
