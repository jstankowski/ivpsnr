/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefSLST.h"
#include "xSeq.h"
#include "xString.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xSeqImgList : public xSeqPic
{
public:
  static constexpr int32 c_DefaultMaxNumFiles = std::numeric_limits<int32>::max() - 1;

protected:
  std::string m_FileNamePattern;
  int32       m_MaxNumFiles = NOT_VALID;
  int32       m_1stFileIdx  = NOT_VALID;
  bool        m_SingleFile  = false;
  int32       m_TmpBuffSize = NOT_VALID;
  byte*       m_TmpBuffPtr  = nullptr;

public:
  virtual void create (int32V2 Size, int32 MaxNumFiles = c_DefaultMaxNumFiles);
  virtual void destroy();

protected:
  virtual bool    xBackendAllowsRead  () const final { return true ; }
  virtual bool    xBackendAllowsWrite () const final { return true ; }
  virtual bool    xBackendAllowsAppend() const final { return false; }
  virtual bool    xBackendAllowsSeek  () const final { return true ; }
  virtual tResult xBackendOpen       (tCSR FileName, eMode OpMode) final ;
  virtual tResult xBackendClose      (                           ) final ;
  virtual tResult xBackendRead       (      uint8* PackedFrame) final;
  virtual tResult xBackendWrite      (const uint8* PackedFrame) final;
  virtual tResult xBackendSeek       (int32 FrameNumber ) final { m_CurrFrameIdx = FrameNumber; return eRetv::Success; }
  virtual tResult xBackendSkip       (int32 NumFrames   ) final { m_CurrFrameIdx += NumFrames ; return eRetv::Success; }
  

protected:
  inline std::string xFormatFileName(int32 FrameIdx) const { return fmt::format(fmt::runtime(m_FileNamePattern), FrameIdx); }
          tResult xImgListOpenRead  ();
          tResult xImgListOpenWrite ();
  virtual tResult xImgListFileVerify(tCSR FileName           ) = 0;
  virtual tResult xImgListFileRead  (      uint8* PackedFrame) = 0;
  virtual tResult xImgListFileWrite (const uint8* PackedFrame) = 0;


};

//===============================================================================================================================================================================================================

class xSeqPNG : public xSeqImgList
{
public:
  xSeqPNG() {};
  xSeqPNG(int32V2 Size, int32 MaxNumFiles) { create(Size, MaxNumFiles); }
  virtual ~xSeqPNG() { destroy(); }

  virtual void create (int32V2 Size, int32 MaxNumFiles = c_DefaultMaxNumFiles);
  virtual void destroy();

protected:
  virtual tResult xImgListFileVerify(tCSR FileName           ) final;
  virtual tResult xImgListFileRead  (      uint8* PackedFrame) final;
  virtual tResult xImgListFileWrite (const uint8* PackedFrame) final;
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
