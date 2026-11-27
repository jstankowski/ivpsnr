/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xInOutResult.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

std::string_view RetvToStr(eRetv Result)
{
  switch(Result)
  {
    case eRetv::Success       : return "Success"        ; break;
    case eRetv::Error         : return "Error"          ; break;
    case eRetv::EndOfFile     : return "EndOfFile"      ; break;
    case eRetv::InexistentFile: return "InexistentFile" ; break;
    case eRetv::CorruptedFile : return "CorruptedFile"  ; break;
    case eRetv::WrongArg      : return "WrongArg"       ; break;
    case eRetv::NotImplemented: return "NotImplemented" ; break;
    default:                    return "Unknown"        ; break;
  }
}

//===============================================================================================================================================================================================================

std::string xInOutResult::format() const
{
  std::string Msg = fmt::format("xInOutResult=<<{}>> ", RetvToStr(m_Result));
  if(!m_Message.empty()) { Msg += fmt::format("Message=<<{}>> ", m_Message); }
  return Msg;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
