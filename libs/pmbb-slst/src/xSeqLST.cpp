/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xSeqLST.h"
#include "xFile.h"
#include "spng.h"
#include "xBMP.h"
#include "xMemory.h"
#include "xErrMsg.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xSeqImgList
//===============================================================================================================================================================================================================
void xSeqImgList::create(int32V2 Size, bool CountFramesFromZero, int32 FirstFrameHint, int32 MaxNumFiles)
{
  m_Size           = Size;
  m_BitDepth       = 8;
  m_BytesPerSample = m_BitDepth <= 8 ? 1 : 2;
  m_ChromaFormat   = eCrF::CF444;

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
  if(m_Packed == nullptr) { xErrMsg::printError(fmt::format("TERRIBLE ERROR --> memory allocation failed in xSeqImgList::create while using xMemory::xAlignedMallocPageAuto({})", m_PackedImgNumBytes)); abort(); }

  m_1stFrameHint = FirstFrameHint;
  m_MaxNumFiles  = MaxNumFiles;
  m_SingleFile   = false;
  m_CountFrom0   = CountFramesFromZero;
}
void xSeqImgList::destroy()
{
  m_OpMode = eMode::Unknown;

  m_Size           = { NOT_VALID, NOT_VALID };
  m_BitDepth       = NOT_VALID;
  m_BytesPerSample = NOT_VALID;
  m_ChromaFormat   = eCrF::INVALID;

  m_PackedCmpNumPels  = NOT_VALID;
  m_PackedCmpNumBytes = NOT_VALID;

  if(m_Packed) { xMemory::xAlignedFree(m_Packed); m_Packed = nullptr; }

  m_MaxNumFiles = NOT_VALID;
  m_SingleFile  = false;
  m_CountFrom0  = false;
}
xSeqCommon::tResult xSeqImgList::xBackendOpen(tCSR FileNamePattern, eMode OpMode)
{
  m_FileNamePattern = FileNamePattern;

  eFmtSts FormatStatus = analyzeFormatString(FileNamePattern);
  if(FormatStatus == eFmtSts::Malformed) { return { eRetv::WrongArg, fmt::format("Malformed format string FileNamePattern={}",FileNamePattern) }; }

  m_SingleFile = FormatStatus==eFmtSts::NonFmt; //file name pattern does not contain any format field

  switch(OpMode)
  {
  case eMode::Read : return xImgListOpenRead (); break;
  case eMode::Write: return xImgListOpenWrite(); break;
  default: return eRetv::NotImplemented; break;
  }
}
xSeqCommon::tResult xSeqImgList::xBackendClose()
{
  m_OpMode = eMode::Unknown;

  m_NumOfFrames  = NOT_VALID;
  m_CurrFrameIdx = NOT_VALID;

  return eRetv::Success;
}
xSeqCommon::tResult xSeqImgList::xBackendRead(uint8* PackedFrame)
{
  if(m_SingleFile && m_CurrFrameIdx > 0) { return { eRetv::Error, fmt::format("Attempt to read multiple times from single file File={}", m_FileNamePattern) }; }
  return xImgListFileRead(PackedFrame);
}
xSeqCommon::tResult xSeqImgList::xBackendWrite(const uint8* PackedFrame)
{
  if(m_SingleFile && m_CurrFrameIdx > 0) { return { eRetv::Error, fmt::format("Attempt to write multiple times into single file File={}", m_FileNamePattern) }; }
  return xImgListFileWrite(PackedFrame);
}
xSeqCommon::tResult xSeqImgList::xImgListOpenRead()
{
  if(m_SingleFile) //file does not contain any format field
  {
    m_1stFrameIdx  = 0;
    m_CountFrom0   = true;
    m_NumOfFrames  = 1;
    m_CurrFrameIdx = 0;
    return xImgListFileVerify(m_FileNamePattern);
  }

  xFileListUtils::tIntRes FirstFrameRes = xFileListUtils::detectFirstFrame(m_FileNamePattern, std::bind(&xSeqImgList::xImgListFileVerify, this, std::placeholders::_1), m_1stFrameHint);
  if(!FirstFrameRes.second) { return FirstFrameRes.second; }
  m_1stFrameIdx = FirstFrameRes.first;

  xFileListUtils::tIntRes NumFramesRes = xFileListUtils::detectNumFrames(m_FileNamePattern, m_1stFrameIdx, m_MaxNumFiles);
  if(!NumFramesRes.second) { return NumFramesRes.second; }
  m_NumOfFrames  = NumFramesRes.first;
  m_CurrFrameIdx = m_CountFrom0 ? 0 : m_1stFrameIdx;
  
  return eRetv::Success;
}
xSeqCommon::tResult xSeqImgList::xImgListOpenWrite()
{
  m_1stFrameIdx  = 0;
  m_NumOfFrames  = 0;
  m_CurrFrameIdx = 0;

  return eRetv::Success;
}

//===============================================================================================================================================================================================================
// xSeqPNG
//===============================================================================================================================================================================================================
void xSeqPNG::create(int32V2 Size, bool CountFramesFromZero, int32 FirstFrameHint, int32 MaxNumFiles)
{
  xSeqImgList::create(Size, CountFramesFromZero, FirstFrameHint, MaxNumFiles);
  m_TmpBuffSize = m_Size.getMul()*3;
  m_TmpBuffPtr  = (uint8*)xMemory::AlignedMalloc(m_TmpBuffSize);
  memset(m_TmpBuffPtr, 0, m_TmpBuffSize);
}
void xSeqPNG::destroy()
{
  xMemory::xAlignedFreeNull(m_TmpBuffPtr);
  m_TmpBuffSize = NOT_VALID;
  xSeqImgList::destroy();
}
xSeqCommon::tResult xSeqPNG::xImgListFileVerify(tCSR FileName)
{
  spng_ctx* Ctx = spng_ctx_new(0);

  //open file
  FILE* File = fopen(FileName.c_str(), "rb");
  if(File == nullptr) { return eRetv::InexistentFile; }

  int32 Res = spng_set_png_file(Ctx, File);
  if(Res) { return { eRetv::CorruptedFile, fmt::format("spng_set_png_file Ret={} RetS={} File={}", Res, spng_strerror(Res), FileName) }; }
  
  Res = spng_decode_chunks(Ctx);
  if(Res) { return { eRetv::CorruptedFile, fmt::format("spng_decode_chunks Ret={} RetS={} File={}", Res, spng_strerror(Res), FileName) }; }

  size_t ExpectedSizeOrg = 0;
  Res = spng_decoded_image_size(Ctx, SPNG_FMT_PNG, &ExpectedSizeOrg);
  if(Res) { return { eRetv::CorruptedFile, fmt::format("spng_decoded_image_size Ret={} RetS={} File={}", Res, spng_strerror(Res), FileName) }; }

  size_t ExpectedSizeRGB8 = 0;
  Res = spng_decoded_image_size(Ctx, SPNG_FMT_RGB8, &ExpectedSizeRGB8);
  if(Res) { return { eRetv::CorruptedFile, fmt::format("spng_decoded_image_size Ret={} RetS={} File={}", Res, spng_strerror(Res), FileName) }; }

  spng_ihdr IHDR;
  Res = spng_get_ihdr(Ctx, &IHDR);
  if(Res) { return { eRetv::CorruptedFile, fmt::format("spng_get_ihdr Ret={} RetS={} File={}", Res, spng_strerror(Res), FileName) }; }

  if(m_Size.getX() != (int32)IHDR.width    ) { return { eRetv::Error, "Width does not match"   }; }
  if(m_Size.getY() != (int32)IHDR.height   ) { return { eRetv::Error, "Height does not match"  }; }
  if(m_BitDepth    != (int32)IHDR.bit_depth) { return { eRetv::Error, "BitDepth does not match"}; }

  spng_ctx_free(Ctx);

  return eRetv::Success;
}
xSeqCommon::tResult xSeqPNG::xImgListFileRead(uint8* PackedFrame)
{
  const int32 FrameIdx = m_CountFrom0 ? m_CurrFrameIdx + m_1stFrameIdx : m_CurrFrameIdx;

  const std::string& FrameFileName = xFormatFileName(FrameIdx);

  spng_ctx* Ctx = spng_ctx_new(0);

  //open file
  FILE* File = fopen(FrameFileName.c_str(), "rb");
  if(File == nullptr) { return eRetv::InexistentFile; }

  int32 Res = spng_set_png_file(Ctx, File);
  if(Res) { return { eRetv::CorruptedFile, fmt::format("spng_set_png_file Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }
  
  Res = spng_decode_chunks(Ctx);
  if(Res) { return { eRetv::CorruptedFile, fmt::format("spng_decode_chunks Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }

  spng_ihdr IHDR;
  Res = spng_get_ihdr(Ctx, &IHDR);
  if(Res) { return { eRetv::CorruptedFile, fmt::format("spng_get_ihdr Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }

  if(m_Size.getX() != (int32)IHDR.width    ) { return { eRetv::Error, "Width does not match"   }; }
  if(m_Size.getY() != (int32)IHDR.height   ) { return { eRetv::Error, "Height does not match"  }; }
  if(m_BitDepth    != (int32)IHDR.bit_depth) { return { eRetv::Error, "BitDepth does not match"}; }

  size_t ExpectedSizeOrg = 0;
  Res = spng_decoded_image_size(Ctx, SPNG_FMT_PNG, &ExpectedSizeOrg);
  if(Res) { return { eRetv::CorruptedFile, fmt::format("spng_decoded_image_size Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }

  size_t ExpectedSizeRGB8  = 0;
  Res = spng_decoded_image_size(Ctx, SPNG_FMT_RGB8, &ExpectedSizeRGB8);
  if(Res) { return { eRetv::CorruptedFile, fmt::format("spng_decoded_image_size Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }
  if((int32)ExpectedSizeRGB8 != m_TmpBuffSize) { return { eRetv::Error, fmt::format("Returned spng_decoded_image_size does not match buffer size Len={} Size={} File={}", ExpectedSizeRGB8, m_TmpBuffSize, FrameFileName) }; }

  Res = spng_decode_image(Ctx, m_TmpBuffPtr, m_TmpBuffSize, SPNG_FMT_RGB8, 0);
  if(Res) { return { eRetv::CorruptedFile, fmt::format("spng_decode_image Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }
  fclose(File);

  uint8* DstPtrR = PackedFrame;
  uint8* DstPtrG = PackedFrame + m_PackedCmpNumPels;
  uint8* DstPtrB = PackedFrame + (m_PackedCmpNumPels << 1);

  for(int32 i = 0, j = 0; i < m_PackedCmpNumPels; i++, j += 3)
  {
    uint8 R = m_TmpBuffPtr[j + 0];
    uint8 G = m_TmpBuffPtr[j + 1];
    uint8 B = m_TmpBuffPtr[j + 2];
    DstPtrR[i] = R;
    DstPtrG[i] = G;
    DstPtrB[i] = B;
  }

  spng_ctx_free(Ctx);

  return eRetv::Success;
}
xSeqCommon::tResult xSeqPNG::xImgListFileWrite(const uint8* PackedFrame)
{
  const uint8* SrcPtrR = PackedFrame;
  const uint8* SrcPtrG = PackedFrame + m_PackedCmpNumPels;
  const uint8* SrcPtrB = PackedFrame + (m_PackedCmpNumPels << 1);

  for(int32 i = 0, j = 0; i < m_PackedCmpNumPels; i++, j += 3)
  {
    uint8 R = SrcPtrR[i];
    uint8 G = SrcPtrG[i];
    uint8 B = SrcPtrB[i];
    m_TmpBuffPtr[j + 0] = R;
    m_TmpBuffPtr[j + 1] = G;
    m_TmpBuffPtr[j + 2] = B;
  }

  const std::string FrameFileName = xFormatFileName(m_CurrFrameIdx);

  spng_ctx* Ctx = spng_ctx_new(SPNG_CTX_ENCODER);

  FILE* File = fopen(FrameFileName.c_str(), "wb");
  if(File == nullptr) { return eRetv::Error; }

  int32 Res = spng_set_png_file(Ctx, File);
  if(Res) { return { eRetv::Error, fmt::format("spng_set_png_file Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }

  spng_ihdr IHDR; memset(&IHDR, 0, sizeof(spng_ihdr));
  IHDR.width      = m_Size.getX();
  IHDR.height     = m_Size.getY();
  IHDR.bit_depth  = (uint8)m_BitDepth;
  IHDR.color_type = SPNG_COLOR_TYPE_TRUECOLOR;
  Res = spng_set_ihdr(Ctx, &IHDR);
  if(Res) { return { eRetv::Error, fmt::format("spng_set_ihdr Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }

  //Res = spng_set_option(Ctx, SPNG_IMG_COMPRESSION_LEVEL, 0);
  //if(Res) { return { eRetv::Error, fmt::format("spng_set_option Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }
  //Res = spng_set_option(Ctx, SPNG_FILTER_CHOICE        , SPNG_DISABLE_FILTERING);
  //if(Res) { return { eRetv::Error, fmt::format("spng_set_option Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }
  Res = spng_encode_image(Ctx, m_TmpBuffPtr, m_TmpBuffSize, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
  if(Res) { return { eRetv::Error, fmt::format("spng_encode_image Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }

  spng_ctx_free(Ctx);
  fclose(File);

  return eRetv::Success;
}

//===============================================================================================================================================================================================================
// xSeqBMP
//===============================================================================================================================================================================================================
void xSeqBMP::create(int32V2 Size, bool CountFramesFromZero, int32 FirstFrameHint, int32 MaxNumFiles)
{
  xSeqImgList::create(Size, CountFramesFromZero, FirstFrameHint, MaxNumFiles);
  m_TmpBuffSize = m_Size.getX() * 4;
  m_TmpBuffPtr = (uint8*)xMemory::AlignedMalloc(m_TmpBuffSize);
  memset(m_TmpBuffPtr, 0, m_TmpBuffSize);
}
void xSeqBMP::destroy()
{
  xMemory::xAlignedFreeNull(m_TmpBuffPtr);
  m_TmpBuffSize = NOT_VALID;
  xSeqImgList::destroy();
}
xSeqCommon::tResult xSeqBMP::xImgListFileVerify(tCSR FileName)
{
  //Open file
  xStream File(FileName, xStream::eMode::Read);
  if(!File.isValid()) { return eRetv::InexistentFile; }

  //Read headers
  xBitmapFileHeader BFH;
  bool ResultBFH = BFH.Read(&File);
  if(!ResultBFH) { return { eRetv::CorruptedFile, "xBitmapFileHeader read failure" }; }
  if(BFH.getType() != 0x4d42 || BFH.getOffset() < xBitmapFileHeader::c_HeaderLength + xBitmapInfoHeader::c_HeaderLength) { return { eRetv::CorruptedFile, "BitmapFileHeader content is invalid" }; }

  xBitmapInfoHeader BIH;
  bool ResultBIH = BIH.Read(&File);
  if(!ResultBIH) { return { eRetv::CorruptedFile, "BitmapInfoHeader read failure (number of color planes != 0)" }; }
  if(BIH.getPlanes      () != 1) { return { eRetv::CorruptedFile, "BitmapInfoHeader content is invalid" }; }
  if(BIH.getBitsPerPixel() != 24 && BIH.getBitsPerPixel() != 32) { return { eRetv::NotImplemented, "Only 24 or 32 bit (non-palette) is suported" }; }
  if(BIH.getCompression () != 0) { return { eRetv::NotImplemented, "Only BI_RGB compression method is supported" }; }

  File.closeFile();

  int32 W    = BIH.getWidth();
  int32 AbsH = xAbs(BIH.getHeight());

  if(m_Size.getX() != (int32)W   ) { return { eRetv::Error, "Width does not match"   }; }
  if(m_Size.getY() != (int32)AbsH) { return { eRetv::Error, "Height does not match"  }; }
  if(m_BitDepth    != (int32)8   ) { return { eRetv::Error, "BitDepth does not match"}; }

  return eRetv::Success;
}
xSeqCommon::tResult xSeqBMP::xImgListFileRead(uint8* PackedFrame)
{
  const std::string& FrameFileName = xFormatFileName(m_CurrFrameIdx + m_1stFrameIdx);

  //Open file
  xStream File(FrameFileName, xStream::eMode::Read);
  if(!File.isValid()) { return eRetv::InexistentFile; }

  //Read headers
  xBitmapFileHeader BFH;
  bool ResultBFH = BFH.Read(&File);
  if(!ResultBFH) { return { eRetv::CorruptedFile, "xBitmapFileHeader read failure" }; }
  if(BFH.getType() != 0x4d42 || BFH.getOffset() < xBitmapFileHeader::c_HeaderLength + xBitmapInfoHeader::c_HeaderLength) { return { eRetv::CorruptedFile, "BitmapFileHeader content is invalid" }; }

  xBitmapInfoHeader BIH;
  bool ResultBIH = BIH.Read(&File);
  if(!ResultBIH) { return { eRetv::CorruptedFile, "BitmapInfoHeader read failure" }; }
  if(BIH.getPlanes() != 1) { return { eRetv::CorruptedFile, "BitmapInfoHeader content is invalid (number of color planes != 0)" }; }
  if(BIH.getBitsPerPixel() != 24 && BIH.getBitsPerPixel() != 32) { return { eRetv::NotImplemented, "Only 24 or 32 bit (non-palette) is suported" }; }
  if(BIH.getCompression() != 0) { return { eRetv::NotImplemented, "Only BI_RGB compression method is supported" }; }

  int32 W    = BIH.getWidth ();
  int32 H    = BIH.getHeight();
  int32 L    = BIH.getBitsPerPixel();
  int32 AbsH = xAbs(H);
  int32 NumC = L == 32 ? 4 : 3;

  if(m_Size.getX() != (int32)W   ) { return { eRetv::Error, "Width does not match"   }; }
  if(m_Size.getY() != (int32)AbsH) { return { eRetv::Error, "Height does not match"  }; }
  if(m_BitDepth    != (int32)8   ) { return { eRetv::Error, "BitDepth does not match"}; }

  //Seek to the image beginning
  File.seekR((uint64)BFH.getOffset(), xStream::eSeek::Beg);

  //Read image
  uint8* DstPtrR = PackedFrame;
  uint8* DstPtrG = PackedFrame + m_PackedCmpNumPels;
  uint8* DstPtrB = PackedFrame + (m_PackedCmpNumPels << 1);
  const int32 Stride = m_Size.getX();  

  int32  LineSize = xRoundUpToNearestMultiple(W * NumC, 2); //4 byte alignment

  if(H > 0)
  {
    for(int32 j = (H - 1); j >= 0; j--)
    {
      uint8* RowPtrR = DstPtrR + j*Stride;
      uint8* RowPtrG = DstPtrG + j*Stride;
      uint8* RowPtrB = DstPtrB + j*Stride;

      bool ResultLine = File.read(m_TmpBuffPtr, LineSize);
      if(!ResultLine) { return { eRetv::Error, "BMP row read error" }; }

      for(int32 i = 0, l = 0; i < W; i++, l += NumC)
      {
        RowPtrB[i] = m_TmpBuffPtr[l + 0];
        RowPtrG[i] = m_TmpBuffPtr[l + 1];
        RowPtrR[i] = m_TmpBuffPtr[l + 2];
      }
    }
  }
  else
  {
    for(int32 j = 0; j < AbsH; j++)
    {
      uint8* RowPtrR = DstPtrR + j * Stride;
      uint8* RowPtrG = DstPtrG + j * Stride;
      uint8* RowPtrB = DstPtrB + j * Stride;

      bool ResultLine = File.read(m_TmpBuffPtr, LineSize);
      if(!ResultLine) { return { eRetv::Error, "BMP row read error" }; }

      for(int32 i = 0, l = 0; i < W; i++, l += NumC)
      {
        RowPtrB[i] = m_TmpBuffPtr[l + 0];
        RowPtrG[i] = m_TmpBuffPtr[l + 1];
        RowPtrR[i] = m_TmpBuffPtr[l + 2];
      }
    }
  }

  File.closeFile();

  return eRetv::Success;
}
xSeqCommon::tResult xSeqBMP::xImgListFileWrite(const uint8* PackedFrame)
{
  const std::string FrameFileName = xFormatFileName(m_CurrFrameIdx);

  //Open file
  xStream File(FrameFileName, xStream::eMode::Write); if(!File.isValid()) { return eRetv::Error; }

  int32 H    = m_Size.getY();
  int32 W    = m_Size.getX();
  int32 NumC = 3;

  int32 LineSize = xRoundUpToNearestMultiple(W * NumC, 2); //4 byte alignment

  xBitmapFileHeader BFH;
  BFH.setType     (0x4d42);
  BFH.setFileSize (xBitmapFileHeader::c_HeaderLength + xBitmapInfoHeader::c_HeaderLength + LineSize*H);
  BFH.setReserved1(0);
  BFH.setReserved2(0);
  BFH.setOffset   (xBitmapFileHeader::c_HeaderLength + xBitmapInfoHeader::c_HeaderLength);
  bool ResultBFH = BFH.Write(&File);
  if(!ResultBFH) { return { eRetv::Error, "xBitmapFileHeader write failure" }; }

  xBitmapInfoHeader BIH;
  BIH.setHeaderSize     (xBitmapInfoHeader::c_HeaderLength);
  BIH.setWidth          (W);
  BIH.setHeight         (H);
  BIH.setPlanes         (1);
  BIH.setBitsPerPixel   ((uint16)(8*NumC));
  BIH.setCompression    (0); //RGB
  BIH.setImageSize      (0);
  BIH.setPelsPerMeterX  (0);
  BIH.setPelsPerMeterY  (0);
  BIH.setColorsUsed     (0);
  BIH.setColorsImportant(0);  
  bool ResultBIH = BIH.Write(&File);
  if(!ResultBIH) { return { eRetv::Error, "BitmapInfoHeader write failure" }; }

  const uint8* SrcPtrR = PackedFrame;
  const uint8* SrcPtrG = PackedFrame + m_PackedCmpNumPels;
  const uint8* SrcPtrB = PackedFrame + (m_PackedCmpNumPels << 1);
  const int32  Stride  = m_Size.getX();

  for(int32 j = (H - 1); j >= 0; j--)
  {
    const uint8* RowPtrR = SrcPtrR + j * Stride;
    const uint8* RowPtrG = SrcPtrG + j * Stride;
    const uint8* RowPtrB = SrcPtrB + j * Stride;

    for(int32 i = 0, l = 0; i < W; i++, l += NumC)
    {
      m_TmpBuffPtr[l + 0] = RowPtrB[i];
      m_TmpBuffPtr[l + 1] = RowPtrG[i];
      m_TmpBuffPtr[l + 2] = RowPtrR[i];
    }
    bool ResultLine = File.write(m_TmpBuffPtr, LineSize);
    if(!ResultLine) { return { eRetv::Error, "BMP row read error" }; }
  }

  File.closeFile();

  return eRetv::Success;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
