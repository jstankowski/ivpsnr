/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xSeqLST.h"
#include "xFile.h"
#include "spng.h"
#include "xMemory.h"
#include "xErrMsg.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xSeqImgList
//===============================================================================================================================================================================================================
void xSeqImgList::create(int32V2 Size, int32 MaxNumFiles)
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

  m_MaxNumFiles = MaxNumFiles;
  m_SingleFile  = false;
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
}
xSeqCommon::tResult xSeqImgList::xBackendOpen(tCSR FileNamePattern, eMode OpMode)
{
  m_FileNamePattern = FileNamePattern;

  m_SingleFile = (xFormatFileName(0) == FileNamePattern); //file name pattern does not contain any format field

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
    m_NumOfFrames  = 1;
    m_CurrFrameIdx = 0;
    return xImgListFileVerify(m_FileNamePattern);
  }

  int32 StartFrame = NOT_VALID;

  for(int32 i = 0; i <= 1; i++)
  {
    std::string FileNameI = xFormatFileName(i);
    if(xFile::exists(FileNameI))
    { 
      StartFrame = i;
      tResult Result = xImgListFileVerify(FileNameI);
      if(!Result) { return Result; }
    }
  }
  if(StartFrame == NOT_VALID) { return { eRetv::Error, "First Idx=(0 or 1) not found"}; }
  m_1stFileIdx = StartFrame;
 
  int32 NumFrames = 0;
  for(int32 i = StartFrame; i < m_MaxNumFiles; i++) //start form 0 or 1 TODO
  {
    std::string FrameFileName = xFormatFileName(i);
    if(xFile::exists(FrameFileName)) { NumFrames++; }
    else                             { break;       }
  }
  if(NumFrames == 0) { return eRetv::Error; }
  
  m_NumOfFrames  = NumFrames;
  m_CurrFrameIdx = 0;
  
  return eRetv::Success;
}
xSeqCommon::tResult xSeqImgList::xImgListOpenWrite()
{
  m_1stFileIdx   = 0;
  m_NumOfFrames  = 0;
  m_CurrFrameIdx = 0;

  return eRetv::Success;
}

//===============================================================================================================================================================================================================
// xSeqPNG
//===============================================================================================================================================================================================================
void xSeqPNG::create(int32V2 Size, int32 MaxNumFiles)
{
  xSeqImgList::create(Size, MaxNumFiles);
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
  if(File == nullptr) { return eRetv::Error; }

  int32 Res = spng_set_png_file(Ctx, File);
  if(Res) { return { eRetv::Error, fmt::format("spng_set_png_file Ret={} RetS={} File={}", Res, spng_strerror(Res), FileName) }; }
  
  Res = spng_decode_chunks(Ctx);
  if(Res) { return { eRetv::Error, fmt::format("spng_decode_chunks Ret={} RetS={} File={}", Res, spng_strerror(Res), FileName) }; }

  size_t ExpectedSizeOrg = 0;
  Res = spng_decoded_image_size(Ctx, SPNG_FMT_PNG, &ExpectedSizeOrg);
  if(Res) { return { eRetv::Error, fmt::format("spng_decoded_image_size Ret={} RetS={} File={}", Res, spng_strerror(Res), FileName) }; }

  size_t ExpectedSizeRGB8 = 0;
  Res = spng_decoded_image_size(Ctx, SPNG_FMT_RGB8, &ExpectedSizeRGB8);
  if(Res) { return { eRetv::Error, fmt::format("spng_decoded_image_size Ret={} RetS={} File={}", Res, spng_strerror(Res), FileName) }; }

  spng_ihdr IHDR;
  Res = spng_get_ihdr(Ctx, &IHDR);
  if(Res) { return { eRetv::Error, fmt::format("spng_get_ihdr Ret={} RetS={} File={}", Res, spng_strerror(Res), FileName) }; }

  if(m_Size.getX() != (int32)IHDR.width    ) { return { eRetv::Error, "Width does not match"   }; }
  if(m_Size.getY() != (int32)IHDR.height   ) { return { eRetv::Error, "Height does not match"  }; }
  if(m_BitDepth    != (int32)IHDR.bit_depth) { return { eRetv::Error, "BitDepth does not match"}; }

  spng_ctx_free(Ctx);

  return eRetv::Success;
}
xSeqCommon::tResult xSeqPNG::xImgListFileRead(uint8* PackedFrame)
{
  const std::string& FrameFileName = xFormatFileName(m_CurrFrameIdx + m_1stFileIdx);

  spng_ctx* Ctx = spng_ctx_new(0);

  //open file
  FILE* File = fopen(FrameFileName.c_str(), "rb");
  if(File == nullptr) { return eRetv::Error; }

  int32 Res = spng_set_png_file(Ctx, File);
  if(Res) { return { eRetv::Error, fmt::format("spng_set_png_file Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }
  
  Res = spng_decode_chunks(Ctx);
  if(Res) { return { eRetv::Error, fmt::format("spng_decode_chunks Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }

  spng_ihdr IHDR;
  Res = spng_get_ihdr(Ctx, &IHDR);
  if(Res) { return { eRetv::Error, fmt::format("spng_get_ihdr Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }

  if(m_Size.getX() != (int32)IHDR.width    ) { return { eRetv::Error, "Width does not match"   }; }
  if(m_Size.getY() != (int32)IHDR.height   ) { return { eRetv::Error, "Height does not match"  }; }
  if(m_BitDepth    != (int32)IHDR.bit_depth) { return { eRetv::Error, "BitDepth does not match"}; }

  size_t ExpectedSizeOrg = 0;
  Res = spng_decoded_image_size(Ctx, SPNG_FMT_PNG, &ExpectedSizeOrg);
  if(Res) { return { eRetv::Error, fmt::format("spng_decoded_image_size Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }

  size_t ExpectedSizeRGB8  = 0;
  Res = spng_decoded_image_size(Ctx, SPNG_FMT_RGB8, &ExpectedSizeRGB8);
  if(Res) { return { eRetv::Error, fmt::format("spng_decoded_image_size Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }
  if((int32)ExpectedSizeRGB8 != m_TmpBuffSize) { return { eRetv::Error, fmt::format("Returned spng_decoded_image_size does not match buffer size Len={} Size={} File={}", ExpectedSizeRGB8, m_TmpBuffSize, FrameFileName) }; }

  Res = spng_decode_image(Ctx, m_TmpBuffPtr, m_TmpBuffSize, SPNG_FMT_RGB8, 0);
  if(Res) { return { eRetv::Error, fmt::format("spng_decode_image Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }
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

} //end of namespace PMBB
