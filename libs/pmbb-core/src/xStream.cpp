/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xStream.h"

namespace PMBB_NAMESPACE {

//=============================================================================================================================================================================

bool xStream::openFile(tCStr& FilePath, const eMode OpenMode)
{
  if(m_Stream != nullptr || FilePath=="" || OpenMode==eMode::Unknown) { return false; };
  
  eDirF                   StrmDirF  = eDirF::None;
  std::ios_base::openmode OpenFlags = std::ios_base::binary;
  if(OpenMode == eMode::Read  ) { StrmDirF = eDirF::Read ; OpenFlags |= std::ios_base::in;                       }
  if(OpenMode == eMode::Write ) { StrmDirF = eDirF::Write; OpenFlags |= std::ios_base::out;                      }
  if(OpenMode == eMode::Append) { StrmDirF = eDirF::Write; OpenFlags |= std::ios_base::out | std::ios_base::ate; }

  std::fstream* FileHandle = new std::fstream();
  FileHandle->open(FilePath, OpenFlags);
  bool Result = FileHandle->is_open() && FileHandle->good();
  if(Result)
  {
    m_FilePath = FilePath;
    m_StrmDirF = StrmDirF;
    m_Stream   = FileHandle;
    m_OwnFStrm = true;
  }
  else { delete(FileHandle); FileHandle = nullptr; xPrintError(); fmt::print("{}\n", FilePath); }
  return Result;
}
void xStream::closeFile()
{
  if(m_Stream == nullptr || !m_OwnFStrm) { return; }
  std::fstream* FileHandle = reinterpret_cast<std::fstream*>(m_Stream);
  if(FileHandle->is_open()) { FileHandle->close(); }
  delete(m_Stream); 
  m_FilePath.clear();
  m_StrmDirF = eDirF::None;
  m_Stream   = nullptr;
  m_OwnFStrm = false;
}
bool xStream::bindStream(tStrm* Stream, const eDirF StrmDirF)
{
  assert(Stream != nullptr); if(m_Stream) { return false; }
  m_FilePath = "## Unknown path for externaly provided iostream ##";
  m_StrmDirF = StrmDirF;
  m_Stream   = Stream;
  m_OwnFStrm = false;
  return true;
}
xStream::tStrm* xStream::unbindStream()
{
  if(m_Stream == nullptr || m_OwnFStrm) { return nullptr; }
  tStrm* Tmp = m_Stream;
  m_FilePath.clear();
  m_StrmDirF = eDirF::None;
  m_Stream   = nullptr;
  m_OwnFStrm = false;
  return Tmp;
}
int64 xStream::size()
{
  bool canR = canRead ();
  bool canW = canWrite();
  if(canR && canW) { return NOT_VALID;}
  else if(canR   ) { return sizeR();  }
  else if(canW   ) { return sizeW();  }
  else             { return NOT_VALID;}
}
int64 xStream::sizeR()
{
  if(m_Stream == nullptr || !canRead()) { return NOT_VALID; }
  std::streampos CurPosition = m_Stream->tellg();
  m_Stream->seekg(0, std::ios_base::end);
  std::streampos EndPosition  = m_Stream->tellg();
  if(EndPosition != CurPosition) { m_Stream->seekg(CurPosition, std::ios_base::beg); }
  int64 Size = EndPosition;
  return Size;
}
int64 xStream::sizeW()
{
  if(m_Stream == nullptr || !canWrite()) { return NOT_VALID; }
  std::streampos CurPosition = m_Stream->tellp();
  m_Stream->seekp(0, std::ios_base::end);
  std::streampos EndPosition = m_Stream->tellp();
  if(EndPosition != CurPosition) { m_Stream->seekp(CurPosition, std::ios_base::beg); }
  int64 Size = EndPosition;
  return Size;
}
bool xStream::end()
{
  bool canR = canRead ();
  bool canW = canWrite();
  if(canR && canW) { return false;}
  else if(canR   ) { return endR();  }
  else if(canW   ) { return endW();  }
  else             { return false;}
}
bool xStream::endR()
{
  if(m_Stream == nullptr || !canRead()) { return false; }
  std::streampos CurPosition = m_Stream->tellg();
  m_Stream->seekg(0, std::ios_base::end);
  std::streampos EndPosition = m_Stream->tellg();
  if(CurPosition == EndPosition) { return true; }
  m_Stream->seekg(CurPosition, std::ios_base::beg);
  return false;
}
bool xStream::endW()
{
  if(m_Stream == nullptr || !canRead()) { return false; }
  std::streampos CurPosition = m_Stream->tellp();
  m_Stream->seekp(0, std::ios_base::end);
  std::streampos EndPosition = m_Stream->tellp();
  if(CurPosition == EndPosition) { return true; }
  m_Stream->seekp(CurPosition, std::ios_base::beg);
  return false;
}
bool xStream::seekR(int64 Position, eSeek SeekMode)
{
  if(m_Stream == nullptr || !canRead()) { return false; }
  std::ios_base::seekdir SeekDir = (std::ios_base::seekdir)SeekMode;
  reinterpret_cast<std::istream*>(m_Stream)->seekg(Position, SeekDir);
  bool Result = m_Stream->good();
  if(!Result) { xPrintError(); }
  return Result;
}
bool xStream::seekW(int64 Position, eSeek SeekMode)
{
  if(m_Stream == nullptr || !canWrite()) { return false; }
  std::ios_base::seekdir SeekDir = (std::ios_base::seekdir)SeekMode;
  reinterpret_cast<std::ostream*>(m_Stream)->seekp(Position, SeekDir);
  bool Result = m_Stream->good();
  if(!Result) { xPrintError(); }
  return Result;
}

//=============================================================================================================================================================================

} //end of namespace PMBB
