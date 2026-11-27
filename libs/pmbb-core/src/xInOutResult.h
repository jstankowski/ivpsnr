/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefCORE.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

enum class [[nodiscard]] eRetv : int32
{ 
  Success       ,  
  Error         ,
  EndOfFile     ,
  InexistentFile,
  CorruptedFile ,
  WrongArg      ,
  NotImplemented,
};

std::string_view RetvToStr(eRetv Result);

//===============================================================================================================================================================================================================

class xInOutResult
{
protected:
  eRetv       m_Result;
  std::string m_Message;

public:
  xInOutResult(eRetv Result, const std::string Message = std::string()) : m_Result(Result), m_Message(Message) {}

  explicit operator bool       () const { return m_Result == eRetv::Success; }
  explicit operator std::string() const { return format(); }

  std::string format() const;

  inline bool operator== (const eRetv Res) const { return m_Result == Res; }
  inline bool operator!= (const eRetv Res) const { return m_Result != Res; }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB
