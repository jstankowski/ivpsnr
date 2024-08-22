/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

// MSVC workaround
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "xCommonDefCORE.h"
#include <cstdio>
#include <string>
#include <cerrno>
#include <fstream>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xStream
{
public:
  using tCStr = const std::string;
  using tStrm = std::iostream;
  enum class eDirF : int32 { None = 0, Read = 1, Write = 2, Both = 3 };
  enum class eMode : int32 { Unknown, Read, Write, Append };
  enum class eSeek : int32 { Beg = std::ios_base::beg, Cur = std::ios_base::cur, End = std::ios_base::end};

protected:
  std::string m_FilePath = std::string();
  eDirF       m_StrmDirF = eDirF::None;
  tStrm*      m_Stream   = nullptr;
  bool        m_OwnFStrm = false;

public:
  inline        xStream(                                     ) {                               }
  inline        xStream(tCStr& FilePath, const eMode OpenMode) { openFile(FilePath, OpenMode); }
  inline       ~xStream(                                     ) { closeFile();                  }

         bool   openFile   (tCStr& FilePath, const eMode OpenMode);
         void   closeFile  ();

  inline tCStr& getFilePath() const { return m_FilePath; }

         bool   bindStream  (tStrm* Stream, const eDirF StrmDirF);
         tStrm* unbindStream();

  inline bool   isValid () const { return (m_Stream != nullptr && m_Stream->good()); }
  inline bool   canRead () const { return (int32)m_StrmDirF & (int32)eDirF::Read ; }
  inline bool   canWrite() const { return (int32)m_StrmDirF & (int32)eDirF::Write; }

  inline bool   read (void*       Memmory, uint32 Length) { m_Stream->read (reinterpret_cast<      char*>(Memmory), Length); return !m_Stream->fail(); }
  inline bool   write(const void* Memmory, uint32 Length) { m_Stream->write(reinterpret_cast<const char*>(Memmory), Length); return !m_Stream->fail(); }
  inline bool   write(const std::string& String         ) { m_Stream->write(String.c_str()                 , String.size()); return !m_Stream->fail(); }
  inline bool   skip (                     uint32 Length) { return seekR(Length, eSeek::Cur); }

         int64  tellR() { return m_Stream->tellg(); }
         int64  tellW() { return m_Stream->tellg(); }

         int64  size ();
         int64  sizeR();
         int64  sizeW();
         
         bool   end ();
         bool   endR();
         bool   endW();

         bool   seekR(int64 Position, eSeek SeekMode);
         bool   seekW(int64 Position, eSeek SeekMode);
         
  inline int32  sync () { return canRead() ? m_Stream->sync() : NOT_VALID; }
  inline void   flush() { if(canWrite()) { m_Stream->flush(); } }

protected:
  inline void   xPrintError() { fmt::print("ERROR {}", std::strerror(errno)); }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
