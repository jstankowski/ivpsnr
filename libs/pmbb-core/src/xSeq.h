/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefCORE.h"
#include "xStream.h"
#include "xPic.h"
#include "xPlane.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xSeqCommon
{
public:
  using tCSR = const std::string&;

  enum class eMode : int32 { Unknown, Read, Write, Append };
  enum class [[nodiscard]] eRetv : int32 { Success, EndOfFile, Error, WrongArg, NotImplemented };

  static std::string_view RetvToStr(eRetv Result)
  {
    switch(Result)
    {
      case eRetv::Success       : return "Success"        ; break;
      case eRetv::EndOfFile     : return "EndOfFile"      ; break;
      case eRetv::Error         : return "Error"          ; break;
      case eRetv::WrongArg      : return "WrongArg"       ; break;
      case eRetv::NotImplemented: return "NotImplemented" ; break;
      default:                    return "Unknown"        ; break;
    }
  }

  class tResult
  {
  protected:
    eRetv       m_Result;
    std::string m_Message;

  public:
    tResult(eRetv Result, const std::string Message = std::string()) : m_Result(Result), m_Message(Message) {}

    explicit operator bool       () const { return m_Result == eRetv::Success; }
    explicit operator std::string() const { return format(); }

    std::string format() const
    {
      std::string Msg = fmt::format("xSeqErrorType=<<{}>> ", RetvToStr(m_Result));
      if(!m_Message.empty()) { Msg += fmt::format("Message=<<{}>> ", m_Message); }
      return Msg;
    }

    inline bool operator== (const eRetv Res) const { return m_Result == Res; }
    inline bool operator!= (const eRetv Res) const { return m_Result != Res; }
  };

protected:
  eMode    m_OpMode     = eMode::Unknown;

  int32V2  m_Size            = { NOT_VALID, NOT_VALID };
  int32    m_BitDepth        = NOT_VALID;
  int32    m_BytesPerSample  = NOT_VALID;
  eCrF     m_ChromaFormat    = eCrF::INVALID;

  uint8*   m_Packed = nullptr;
           
  int32    m_PackedCmpNumPels  = NOT_VALID;
  int32    m_PackedCmpNumBytes = NOT_VALID;
  int32    m_PackedImgNumBytes = NOT_VALID;

  int32    m_NumOfFrames     = NOT_VALID;
  int32    m_CurrFrameIdx    = NOT_VALID;

  bool     m_LoopReading     = false;
  bool     m_FlushAfterWrite = false;

public:
  inline eMode   getOpMode  () const { return m_OpMode; }

  inline int32V2 getSize    () const { return m_Size         ; }
  inline int32   getWidth   () const { return m_Size.getX()  ; }
  inline int32   getHeight  () const { return m_Size.getY()  ; }
  inline int32   getArea    () const { return m_Size.getMul(); }
  inline int32   getBitDepth() const { return m_BitDepth     ; }

  inline int32 getOneFrameSize() const { return m_PackedImgNumBytes; }

  inline int32 getNumOfFrames () const { return m_NumOfFrames ; }
  inline int32 getCurrFrameIdx() const { return m_CurrFrameIdx; }

  inline void setFlushAfterWrite(bool FlushAfterWrite)       { m_FlushAfterWrite = FlushAfterWrite; }
  inline bool getFlushAfterWrite(                    ) const { return m_FlushAfterWrite;            }
};

//===============================================================================================================================================================================================================

class xSeqBase : public xSeqCommon
{
protected:

public:
  xSeqBase() {};
  virtual ~xSeqBase() { }
  virtual void destroy() = 0;

  inline bool allowsRead () { return xBackendAllowsRead (); }
  inline bool allowsWrite() { return xBackendAllowsWrite(); }
  inline bool allowsSeek () { return xBackendAllowsSeek (); }

  bool isModeAllowed(eMode OpMode);

  tResult openFile  (tCSR, eMode OpMode);
  tResult closeFile ();

  tResult readFrame (xPicP*       Pic);
  tResult writeFrame(const xPicP* Pic);
  tResult readFrame (xPlane<uint8>*       Plane);
  tResult writeFrame(const xPlane<uint8>* Plane);
  tResult readFrame (xPlane<uint16>*       Plane);
  tResult writeFrame(const xPlane<uint16>* Plane);

  tResult seekFrame (int32 FrameNumber);
  tResult skipFrame (int32 NumFrames  );


protected:
  bool xUnpackFrame(      xPicP* Pic);
  bool xPackFrame  (const xPicP* Pic);
  bool xUnpackFrame(      xPlane<uint8>* Pic);
  bool xPackFrame  (const xPlane<uint8>* Pic);
  bool xUnpackFrame(      xPlane<uint16>* Pic);
  bool xPackFrame  (const xPlane<uint16>* Pic);

protected:
  virtual bool    xBackendAllowsRead  () const = 0; //requires xBackendRead, xBackendSkip
  virtual bool    xBackendAllowsWrite () const = 0; //requires xBackendWrite
  virtual bool    xBackendAllowsSeek  () const = 0; //requires xBackendSeek
  virtual tResult xBackendOpen        (tCSR FileName, eMode OpMode) = 0;
  virtual tResult xBackendClose       (                           ) = 0;
  virtual tResult xBackendRead        (uint8* PackedFrame) = 0;
  virtual tResult xBackendWrite       (uint8* PackedFrame) = 0;
  virtual tResult xBackendSeek        (int32 FrameNumber ) = 0;
  virtual tResult xBackendSkip        (int32 NumFrames   ) = 0;
};

//===============================================================================================================================================================================================================

class xSeq : public xSeqBase
{
protected:
  xStream* m_Stream = nullptr;

public:
  xSeq() { };
  xSeq(int32V2 Size, int32 BitDepth, eCrF ChromaFormat) { create(Size, BitDepth, ChromaFormat); }
  virtual ~xSeq() { destroy(); }

  void         create (int32V2 Size, int32 BitDepth, eCrF ChromaFormat);
  virtual void destroy() final;

  tResult bindStream(xStream* Stream, const eMode OpMode);
  tResult dropStream();

protected:
  virtual bool    xBackendAllowsRead  () const final { return true; }
  virtual bool    xBackendAllowsWrite () const final { return true; }
  virtual bool    xBackendAllowsSeek  () const final { return true; }
  virtual tResult xBackendOpen        (tCSR FileName, eMode OpMode) final ;
  virtual tResult xBackendClose       (                           ) final ;
  virtual tResult xBackendRead        (uint8* PackedFrame) final ;
  virtual tResult xBackendWrite       (uint8* PackedFrame) final ;
  virtual tResult xBackendSeek        (int32 FrameNumber ) final ;
  virtual tResult xBackendSkip        (int32 NumFrames   ) final ;

public:
  static int32 calcSingleFrameSize(int32V2 Size, int32 BitDepth, eCrF ChromaFormat);
  static int32 calcNumFramesInFile(int32V2 Size, int32 BitDepth, eCrF ChromaFormat, int64 FileSize);
  static tResult dumpFrame(const xPicP* Pic, const std::string& FileName, eCrF ChromaFormat, bool Append); //slow stateless write for debug purposes
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
