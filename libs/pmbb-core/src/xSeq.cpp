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

void xSeq::create(int32V2 Size, int32 BitDepth, int32 ChromaFormat)
{
  m_Width  = Size.getX();
  m_Height = Size.getY();

  m_BitsPerSample  = BitDepth;
  m_BytesPerSample = m_BitsPerSample <= 8 ? 1 : 2;
  m_ChromaFormat   = ChromaFormat;

  m_FileCmpNumPels  = m_Width * m_Height;
  m_FileCmpNumBytes = m_Width * m_Height * m_BytesPerSample;

  switch(m_ChromaFormat)
  {
    case 444: m_FileImgNumBytes = 3 * m_FileCmpNumBytes; break;
    case 422: m_FileImgNumBytes = m_FileCmpNumBytes << 1; break;
    case 420: m_FileImgNumBytes = m_FileCmpNumBytes + (m_FileCmpNumBytes >> 1); break;
    case 400: m_FileImgNumBytes = m_FileCmpNumBytes; break;
    default: assert(0);
  }

  m_FileBuffer = (uint8*)xMemory::xAlignedMallocPageAuto(m_FileImgNumBytes);
}
void xSeq::destroy()
{
  m_OpMode = eMode::Unknown;
  m_Stream = nullptr;

  m_Width  = NOT_VALID;
  m_Height = NOT_VALID;

  m_BitsPerSample  = NOT_VALID;
  m_BytesPerSample = NOT_VALID;
  m_ChromaFormat   = NOT_VALID;

  m_FileCmpNumPels  = NOT_VALID;
  m_FileCmpNumBytes = NOT_VALID;

  if(m_FileBuffer) { xMemory::xAlignedFree(m_FileBuffer); m_FileBuffer = nullptr; }
}
xSeq::tResult xSeq::openFile(const std::string& FileName, eMode OpMode)
{
  m_OpMode = OpMode;
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
    m_NumOfFrames  = calcNumFramesInFile({ m_Width, m_Height }, m_BitsPerSample, m_ChromaFormat, FileSize);
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
xSeq::tResult xSeq::closeFile()
{
  m_Stream->closeFile(); delete(m_Stream); m_Stream = nullptr;
  m_OpMode = eMode::Unknown;

  m_NumOfFrames  = NOT_VALID;
  m_CurrFrameIdx = NOT_VALID;

  return eRetv::Success;
}
xSeq::tResult xSeq::bindStream(xStream* Stream, const eMode OpMode)
{
  if(Stream == nullptr || !Stream->isValid()) { return eRetv::WrongArg; }
  if(m_OpMode != eMode::Read && m_OpMode != eMode::Write && m_OpMode != eMode::Append) { return eRetv::WrongArg; }
  if(m_Stream != nullptr) { return eRetv::Error; }
  
  m_Stream = Stream;
  m_OpMode = OpMode;

  int64 FileSize = m_Stream->size();
  m_NumOfFrames  = calcNumFramesInFile({ m_Width, m_Height }, m_BitsPerSample, m_ChromaFormat, FileSize);
  m_CurrFrameIdx = 0;

  return eRetv::Success;
}
xSeq::tResult xSeq::dropStream()
{
  m_Stream = nullptr;
  m_OpMode = eMode::Unknown;

  m_NumOfFrames = NOT_VALID;
  m_CurrFrameIdx = NOT_VALID;
  
  return eRetv::Success;
}
xSeq::tResult xSeq::seekFrame(int32 FrameNumber)
{
  if(m_OpMode == eMode::Read && FrameNumber >= m_NumOfFrames) { return eRetv::WrongArg; }
  if(m_OpMode != eMode::Read) { return eRetv::Error; }

  //seek frame
  uintSize Offset = (uintSize)m_FileImgNumBytes * (uintSize)FrameNumber;
  bool SeekResult = m_Stream->seekR(Offset, xStream::eSeek::Beg);
  if(!SeekResult) { return eRetv::Error; }

  //update state
  m_CurrFrameIdx = FrameNumber;

  return eRetv::Success;
}
xSeq::tResult xSeq::readFrame(xPicP* Pic)
{
  if(m_OpMode == eMode::Read && m_CurrFrameIdx >= m_NumOfFrames) { return eRetv::EndOfFile; }
  if(m_OpMode != eMode::Read) { return eRetv::Error; }

  //read frame
  bool ReadOK = m_Stream->read(m_FileBuffer, m_FileImgNumBytes);
  if(!ReadOK) { return eRetv::Error; }

  //unpack frame
  bool Unpacked = xUnpackFrame(Pic);
  if(!Unpacked) { return eRetv::Error; }

  //update state
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}
xSeq::tResult xSeq::writeFrame(const xPicP* Pic, bool Flush)
{
  if(m_OpMode != eMode::Write && m_OpMode != eMode::Append) { return eRetv::Error; }

  //pack frame
  bool Packed = xPackFrame(Pic);
  if(!Packed) { return eRetv::Error; }

  //write frame
  bool WriteOK = m_Stream->write(m_FileBuffer, m_FileImgNumBytes);
  if(!WriteOK) { return eRetv::Error; }
  if(Flush) { m_Stream->flush(); }

  //update state
  m_NumOfFrames  += 1;
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}
xSeq::tResult xSeq::readFrame(xPlane<uint16>* Plane)
{
  if(m_OpMode == eMode::Read && m_CurrFrameIdx >= m_NumOfFrames) { return eRetv::EndOfFile; }
  if(m_OpMode != eMode::Read) { return eRetv::Error; }

  //read frame
  bool ReadOK = m_Stream->read(m_FileBuffer, m_FileImgNumBytes);
  if(!ReadOK) { return eRetv::Error; }

  //unpack frame
  bool Unpacked = xUnpackFrame(Plane);
  if(!Unpacked) { return eRetv::Error; }

  //update state
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}
xSeq::tResult xSeq::writeFrame(const xPlane<uint16>* Plane, bool Flush)
{
  if(m_OpMode != eMode::Write && m_OpMode != eMode::Append) { return eRetv::Error; }

  //pack frame
  bool Packed = xPackFrame(Plane);
  if(!Packed) { return eRetv::Error; }

  //write frame
  bool WriteOK = m_Stream->write(m_FileBuffer, m_FileImgNumBytes);
  if(!WriteOK) { return eRetv::Error; }
  if(Flush) { m_Stream->flush(); }

  //update state
  m_NumOfFrames  += 1;
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool xSeq::xUnpackFrame(xPicP* Pic)
{
  uint16* PtrLm      = Pic->getAddr  (eCmp::LM);
  uint16* PtrCb      = Pic->getAddr  (eCmp::CB);
  uint16* PtrCr      = Pic->getAddr  (eCmp::CR);
  const int32 Stride = Pic->getStride();

  //process luma
  if(m_BytesPerSample == 1) { xPixelOps::Cvt (PtrLm, m_FileBuffer           , Stride, m_Width, m_Width, m_Height); }
  else                      { xPixelOps::Copy(PtrLm, (uint16*)(m_FileBuffer), Stride, m_Width, m_Width, m_Height); }

  //process chroma (if there is any chroma)
  if(m_ChromaFormat > 400)
  {
    const uint8* ChromaPtr = m_FileBuffer + m_FileCmpNumBytes;

    if(m_ChromaFormat == 420)
    {
      const int32 ChromaFileCmpNumBytes = m_FileCmpNumBytes >> 2;
      const int32 ChromaFileStride      = m_Width >> 1;
      if(m_BytesPerSample == 1)
      {
        xPixelOps::CvtUpsampleHV(PtrCb, ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::CvtUpsampleHV(PtrCr, ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
      }
      else
      {
        xPixelOps::UpsampleHV(PtrCb, (uint16*)ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::UpsampleHV(PtrCr, (uint16*)ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
      }
    }
    else if(m_ChromaFormat == 422)
    {
      const int32 ChromaFileCmpNumBytes = m_FileCmpNumBytes >> 1;
      const int32 ChromaFileStride      = m_Width >> 1;
      if(m_BytesPerSample == 1)
      {
        xPixelOps::CvtUpsampleH(PtrCb, ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::CvtUpsampleH(PtrCr, ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
      }
      else
      {
        xPixelOps::UpsampleH(PtrCb, (uint16*)ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::UpsampleH(PtrCr, (uint16*)ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
      }
    }
    else if(m_ChromaFormat == 444)
    {
      if(m_BytesPerSample == 1)
      { 
        xPixelOps::Cvt(PtrCb, ChromaPtr, Stride, m_Width, m_Width, m_Height);
        ChromaPtr += m_FileCmpNumBytes;
        xPixelOps::Cvt(PtrCr, ChromaPtr, Stride, m_Width, m_Width, m_Height);
      }
      else
      { 
        xPixelOps::Copy(PtrCb, (uint16*)ChromaPtr, Stride, m_Width, m_Width, m_Height);
        ChromaPtr += m_FileCmpNumBytes;
        xPixelOps::Copy(PtrCr, (uint16*)ChromaPtr, Stride, m_Width, m_Width, m_Height);
      }
    }
    else { return false; }
  }
  return true;
}
bool xSeq::xPackFrame(const xPicP* Pic)
{
  const uint16* PtrLm  = Pic->getAddr  (eCmp::LM);
  const uint16* PtrCb  = Pic->getAddr  (eCmp::CB);
  const uint16* PtrCr  = Pic->getAddr  (eCmp::CR);
  const int32   Stride = Pic->getStride();

  //process luma
  if(m_BytesPerSample == 1) { xPixelOps::Cvt (m_FileBuffer           , PtrLm, m_Width, Stride, m_Width, m_Height); }
  else                      { xPixelOps::Copy((uint16*)(m_FileBuffer), PtrLm, m_Width, Stride, m_Width, m_Height); }

  //process chroma (if there is any chroma)
  if(m_ChromaFormat > 400)
  {
    uint8* ChromaPtr = m_FileBuffer + m_FileCmpNumBytes;
    if(m_ChromaFormat == 420)
    {
      const int32 ChromaFileCmpNumBytes = m_FileCmpNumBytes >> 2;
      const int32 ChromaFileStride      = m_Width >> 1;
      if(m_BytesPerSample == 1)
      {
        xPixelOps::CvtDownsampleHV(ChromaPtr, PtrCb, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::CvtDownsampleHV(ChromaPtr, PtrCr, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
      }
      else
      {
        xPixelOps::DownsampleHV((uint16*)ChromaPtr, PtrCb, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::DownsampleHV((uint16*)ChromaPtr, PtrCr, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
      }
    }
    if(m_ChromaFormat == 422)
    {
      const int32 ChromaFileCmpNumBytes = m_FileCmpNumBytes >> 1;
      const int32 ChromaFileStride      = m_Width >> 1;
      if(m_BytesPerSample == 1)
      {
        xPixelOps::CvtDownsampleH(ChromaPtr, PtrCb, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::CvtDownsampleH(ChromaPtr, PtrCr, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
      }
      else
      {
        xPixelOps::DownsampleH((uint16*)ChromaPtr, PtrCb, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
        ChromaPtr += ChromaFileCmpNumBytes;
        xPixelOps::DownsampleH((uint16*)ChromaPtr, PtrCr, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
      }
    }
    else if(m_ChromaFormat == 444)
    {
      if(m_BytesPerSample == 1)
      { 
        xPixelOps::Cvt(ChromaPtr, PtrCb, m_Width, Stride, m_Width, m_Height);
        ChromaPtr += m_FileCmpNumBytes;
        xPixelOps::Cvt(ChromaPtr, PtrCr, m_Width, Stride, m_Width, m_Height);
      }
      else
      { 
        xPixelOps::Copy((uint16*)ChromaPtr, PtrCb, m_Width, Stride, m_Width, m_Height);
        ChromaPtr += m_FileCmpNumBytes;
        xPixelOps::Copy((uint16*)ChromaPtr, PtrCr, m_Width, Stride, m_Width, m_Height);
      }
    }
    else { return false; }
  }
  return true;
}
bool xSeq::xUnpackFrame(xPlane<uint16>* Pic)
{
  uint16* PtrLm      = Pic->getAddr  ();
  const int32 Stride = Pic->getStride();

  //process luma
  if(m_BytesPerSample == 1) { xPixelOps::Cvt (PtrLm, m_FileBuffer           , Stride, m_Width, m_Width, m_Height); }
  else                      { xPixelOps::Copy(PtrLm, (uint16*)(m_FileBuffer), Stride, m_Width, m_Width, m_Height); }

  return true;
}
bool xSeq::xPackFrame(const xPlane<uint16>* Pic)
{
  const uint16* PtrLm  = Pic->getAddr  ();
  const int32   Stride = Pic->getStride();

  //process luma
  if(m_BytesPerSample == 1) { xPixelOps::Cvt (m_FileBuffer           , PtrLm, m_Width, Stride, m_Width, m_Height); }
  else                      { xPixelOps::Copy((uint16*)(m_FileBuffer), PtrLm, m_Width, Stride, m_Width, m_Height); }

  //process chroma
  uint8* ChromaPtr = m_FileBuffer + m_FileCmpNumBytes;
  int32  CromaNumPels = 0;

  switch(m_ChromaFormat)
  {
    case 444: CromaNumPels = m_FileCmpNumPels << 1; break;
    case 422: CromaNumPels = m_FileCmpNumPels     ; break;
    case 420: CromaNumPels = m_FileCmpNumPels >> 1; break;
    case 400: CromaNumPels = 0                    ; break;
    default: assert(0); return false;
  }
  if(CromaNumPels)
  {
    if(m_BytesPerSample == 1) { xMemsetX((uint8* )ChromaPtr, (uint8 )xBitDepth2MidValue(m_BitsPerSample), CromaNumPels); }
    else                      { xMemsetX((uint16*)ChromaPtr, (uint16)xBitDepth2MidValue(m_BitsPerSample), CromaNumPels); }
  }

  return true;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

int32 xSeq::calcNumFramesInFile(int32V2 Size, int32 BitDepth, int32 ChromaFormat, int64 FileSize)
{
  int32 BytesPerSample  = BitDepth <= 8 ? 1 : 2;
  int32 FileCmpNumPels  = Size.getMul();
  int32 FileCmpNumBytes = FileCmpNumPels * BytesPerSample;

  int32 FileImgNumBytes = NOT_VALID;
  switch(ChromaFormat)
  {
    case 444: FileImgNumBytes = 3 * FileCmpNumBytes; break;
    case 422: FileImgNumBytes = 2 * FileCmpNumBytes; break;
    case 420: FileImgNumBytes = FileCmpNumBytes + (FileCmpNumBytes >> 1); break;
    case 400: FileImgNumBytes = FileCmpNumBytes; break;
    default: assert(0);
  }

  return (int32)(FileSize / FileImgNumBytes);
}
xSeq::tResult xSeq::dumpFrame(const xPicP* Pic, const std::string& FileName, int32 ChromaFormat, bool Append)
{
  xSeq Seq(Pic->getSize(), Pic->getBitDepth(), ChromaFormat);
  tResult ResultOpen = Seq.openFile(FileName, Append ? eMode::Append : eMode::Write);
  if(ResultOpen  != eRetv::Success) { return ResultOpen ; }
  tResult ResultWrite = Seq.writeFrame(Pic);
  if(ResultWrite != eRetv::Success) { return ResultWrite; }
  tResult ResultClose = Seq.closeFile();
  if(ResultClose != eRetv::Success) { return ResultClose; }
  return eRetv::Success;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
