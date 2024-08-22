
/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xSeq.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xSeqPNG : public xSeqBase
{
protected:
  int32 m_MaxNumFiles = int32_max;
  std::vector<std::string> m_FrameFiles;

public:
  xSeqPNG() {};
  xSeqPNG(int32V2 Size, int32 MaxNumFiles) { create(Size, MaxNumFiles); }
  virtual ~xSeqPNG() { destroy(); }

  void         create (int32V2 Size, int32 MaxNumFiles);
  virtual void destroy() final;

protected:
  virtual bool    xBackendAllowsRead () const final { return true ; }
  virtual bool    xBackendAllowsWrite() const final { return false; }
  virtual bool    xBackendAllowsSeek () const final { return true ; }
  virtual tResult xBackendOpen       (tCSR FileName, eMode OpMode) final ;
  virtual tResult xBackendClose      (                           ) final ;
  virtual tResult xBackendRead       (uint8* PackedFrame) final ;
  virtual tResult xBackendWrite      (uint8*            ) final { return eRetv::NotImplemented; }
  virtual tResult xBackendSeek       (int32 FrameNumber ) final { m_CurrFrameIdx = FrameNumber; return eRetv::Success; }
  virtual tResult xBackendSkip       (int32 NumFrames   ) final { m_CurrFrameIdx += NumFrames ; return eRetv::Success; }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
