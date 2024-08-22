/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xSeqPNG.h"
#include "xFile.h"
#include "spng.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xSeqPNG::create(int32V2 Size, int32 MaxNumFiles)
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

  m_MaxNumFiles = MaxNumFiles;
}
void xSeqPNG::destroy()
{
  m_OpMode = eMode::Unknown;

  m_Size           = { NOT_VALID, NOT_VALID };
  m_BitDepth       = NOT_VALID;
  m_BytesPerSample = NOT_VALID;
  m_ChromaFormat   = eCrF::INVALID;

  m_PackedCmpNumPels  = NOT_VALID;
  m_PackedCmpNumBytes = NOT_VALID;

  if(m_Packed) { xMemory::xAlignedFree(m_Packed); m_Packed = nullptr; }

}
xSeqBase::tResult xSeqPNG::xBackendOpen(tCSR FileName, eMode /*OpMode*/)
{
  int32 StartFrame = NOT_VALID;
  for(int32 i = 0; i <= 1; i++)
  {
    std::string FileNameI = fmt::format(FileName, i);
    if(xFile::exists(FileNameI)) { StartFrame = i; }
  }
  if(StartFrame == NOT_VALID) { return { eRetv::Error, "First Idx=(0 or 1) not found"}; }
 
  int32 NumFrames = 0;
  for(int32 i = StartFrame; i < m_MaxNumFiles; i++) //start form 0 or 1 TODO
  {
    std::string FrameFileName = fmt::format(FileName, i);
    if(xFile::exists(FrameFileName))
    {
      m_FrameFiles.emplace_back(std::move(FrameFileName));
      NumFrames++;
    }
    else { break; }
  }
  if(NumFrames == 0) { return eRetv::Error; }
  
  m_NumOfFrames  = NumFrames;
  m_CurrFrameIdx = 0;
  
  return eRetv::Success;
}
xSeqBase::tResult xSeqPNG::xBackendClose()
{
  m_OpMode = eMode::Unknown;

  m_FrameFiles.clear();

  m_NumOfFrames  = NOT_VALID;
  m_CurrFrameIdx = NOT_VALID;

  return eRetv::Success;
}
xSeqBase::tResult xSeqPNG::xBackendRead(uint8* PackedFrame)
{
  const std::string& FrameFileName = m_FrameFiles.at(m_CurrFrameIdx);

  spng_ctx* Ctx = spng_ctx_new(0);

  //open file
  FILE* File = fopen(FrameFileName.c_str(), "rb");
  if(File == nullptr) { return eRetv::Error; }

  int32 Res = spng_set_png_file(Ctx, File);
  if(Res) { return { eRetv::Error, fmt::format("spng_set_png_file Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }
  
  Res = spng_decode_chunks(Ctx);
  if(Res) { return { eRetv::Error, fmt::format("spng_decode_chunks Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }

  size_t ExpectedSizeOrg = 0;
  Res = spng_decoded_image_size(Ctx, SPNG_FMT_PNG, &ExpectedSizeOrg);
  if(Res) { return { eRetv::Error, fmt::format("spng_decoded_image_size Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }

  size_t ExpectedSizeRGB8 = 0;
  Res = spng_decoded_image_size(Ctx, SPNG_FMT_RGBA8, &ExpectedSizeRGB8);
  if(Res) { return { eRetv::Error, fmt::format("spng_decoded_image_size Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }

  spng_ihdr IHDR;
  Res = spng_get_ihdr(Ctx, &IHDR);
  if(Res) { return { eRetv::Error, fmt::format("spng_get_ihdr Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }

  if(m_Size.getX() != (int32)IHDR.width    ) { return { eRetv::Error, "Width does not match"}; }
  if(m_Size.getY() != (int32)IHDR.height   ) { return { eRetv::Error, "Height does not match"}; }
  if(m_BitDepth    != (int32)IHDR.bit_depth) { return { eRetv::Error, "BitDepth does not match"}; }

  byte* TmpBuff = (byte*)malloc(ExpectedSizeRGB8);
  Res = spng_decode_image(Ctx, TmpBuff, ExpectedSizeRGB8, SPNG_FMT_RGB8, 0);
  if(Res) { return { eRetv::Error, fmt::format("spng_decode_image Ret={} RetS={} File={}", Res, spng_strerror(Res), FrameFileName) }; }
  fclose(File);

  uint8* DstPtrR = PackedFrame;
  uint8* DstPtrG = PackedFrame + m_PackedCmpNumPels;
  uint8* DstPtrB = PackedFrame + (m_PackedCmpNumPels << 1);

  for(int32 i = 0, j = 0; i < m_PackedCmpNumPels; i++, j += 3)
  {
    uint8 R = TmpBuff[j + 0];
    uint8 G = TmpBuff[j + 1];
    uint8 B = TmpBuff[j + 2];
    DstPtrR[i] = R;
    DstPtrG[i] = G;
    DstPtrB[i] = B;
  }

  free(TmpBuff);

  spng_ctx_free(Ctx);

  return eRetv::Success;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
