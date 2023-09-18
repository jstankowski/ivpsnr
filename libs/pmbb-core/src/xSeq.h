/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefPMBB-CORE.h"
#include "xStream.h"
#include "xPic.h"

#if __has_include("xPlane.h")
#include "xPlane.h"
#define HAS_XPLANE 1
#else
#define HAS_XPLANE 0
#endif

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xSeq
{
public:
  enum class eMode : int32 { Unknown, Read, Write, Append };
  enum class eRetv : int32 { Success, EndOfFile, Error, WrongArg };

  static std::string_view ResultToString(eRetv Result)
  {
    switch(Result)
    {
      case eRetv::Success  : return "Success"  ; break;
      case eRetv::EndOfFile: return "EndOfFile"; break;
      case eRetv::Error    : return "Error"    ; break;
      case eRetv::WrongArg : return "WrongArg" ; break;
      default:               return "Unknown"  ; break;
    }
  }

  class tResult
  {
  protected:
    const eRetv       m_Result;
    const std::string m_Message;

  public:
    tResult(eRetv Result, const std::string Message = std::string()) : m_Result(Result), m_Message(Message) {}

    explicit operator bool() const { return CheckResult(m_Result); }

    static inline bool CheckResult(eRetv Result)
    {
      if(Result == eRetv::Success) { return true; }
      fmt::print("xSeq error type = {}", ResultToString(Result));
      return false;
    }

    inline bool operator== (const eRetv Res) const { return m_Result == Res; }
    inline bool operator!= (const eRetv Res) const { return m_Result != Res; }
  };

protected:
  eMode    m_OpMode = eMode::Unknown;
  xStream* m_Stream = nullptr;

  int32   m_Width           = NOT_VALID;
  int32   m_Height          = NOT_VALID;

  int32   m_BitsPerSample   = NOT_VALID;
  int32   m_BytesPerSample  = NOT_VALID;
  int32   m_ChromaFormat    = NOT_VALID;

  int32   m_FileCmpNumPels  = NOT_VALID;
  int32   m_FileCmpNumBytes = NOT_VALID;
  int32   m_FileImgNumBytes = NOT_VALID;

  int32   m_NumOfFrames     = NOT_VALID;
  int32   m_CurrFrameIdx    = NOT_VALID;

  uint8*  m_FileBuffer      = nullptr;

public:
  xSeq() { m_FileBuffer = nullptr; };
  xSeq(int32V2 Size, int32 BitDepth, int32 ChromaFormat) { create(Size, BitDepth, ChromaFormat); }
  ~xSeq() { destroy(); }

  void    create    (int32V2 Size, int32 BitDepth, int32 ChromaFormat);
  void    destroy   ();

  tResult openFile  (const std::string& FileName, eMode OpMode);
  tResult closeFile ();
  tResult bindStream(xStream* Stream, const eMode StreamMode);
  tResult dropStream();

  tResult seekFrame (int32 FrameNumber);
  tResult readFrame (xPicP*       Pic);
  tResult writeFrame(const xPicP* Pic, bool Flush = false);
#if HAS_XPLANE
  tResult readFrame (xPlane<uint16>*       Plane);
  tResult writeFrame(const xPlane<uint16>* Plane, bool Flush = false);
#endif //HAS_XPLANE

public:
  inline int32 getWidth   () const { return m_Width           ; }
  inline int32 getHeight  () const { return m_Height          ; }
  inline int32 getArea    () const { return m_Width * m_Height; }
  inline int32 getBitDepth() const { return m_BitsPerSample   ; }

  inline std::string getFilePath() const { return m_Stream->getFilePath(); }
  inline eMode       getOpMode  () const { return m_OpMode;   }
  inline int64       getFileSize()       { return m_Stream->size(); }

  inline int32       getNumOfFrames () const { return m_NumOfFrames ; }
  inline int32       getCurrFrameIdx() const { return m_CurrFrameIdx; }

protected:
  bool xUnpackFrame(      xPicP* Pic);
  bool xPackFrame  (const xPicP* Pic);
#if HAS_XPLANE
  bool xUnpackFrame(      xPlane<uint16>* Pic);
  bool xPackFrame  (const xPlane<uint16>* Pic);
#endif //HAS_XPLANE

public:
  static int32 calcNumFramesInFile(int32V2 Size, int32 BitDepth, int32 ChromaFormat, int64 FileSize);

  static tResult dumpFrame(const xPicP* Pic, const std::string& FileName, int32 ChromaFormat, bool Append); //slow stateless write for debug purposes
};

//===============================================================================================================================================================================================================

#ifndef XSEQ_IMPLEMENTATION
#undef HAS_XPLANE
#endif

} //end of namespace PMBB
