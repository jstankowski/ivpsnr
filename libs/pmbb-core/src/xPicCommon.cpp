/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xPic.h"
#include "xPixelOps.h"
#include <cassert>
#include <cstring>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================
// xPicCommon
//===============================================================================================================================================================================================================
void xPicCommon::xInit(int32V2 Size, int32 BitDepth, int32 Margin, int32 NumCmps, int32 PelNumBytes)
{
  assert(Size.getX()>0 && Size.getY()>0 && Margin>=0 && NumCmps>0 && NumCmps<=c_MaxNumCmps);

  m_Size     = Size;
  m_Width    = Size.getX();
  m_Height   = Size.getY();
  m_Margin   = Margin;
  m_Stride   = m_Width + (m_Margin << 1);
  m_BitDepth = BitDepth;
  m_NumCmps  = NumCmps;

  m_BytesPerSample  = m_BitDepth <= 8 ? 1 : 2;
  m_BuffCmpNumPels  = (m_Width + (m_Margin << 1)) * (m_Height + (m_Margin << 1));
  m_BuffCmpNumBytes = m_BuffCmpNumPels * PelNumBytes;

  m_IsMarginExtended = false;
}
void xPicCommon::xUnInit()
{
  m_Size             = {NOT_VALID, NOT_VALID};
  m_Width            = NOT_VALID;
  m_Height           = NOT_VALID;
  m_Margin           = NOT_VALID;
  m_Stride           = NOT_VALID;
  m_BitDepth         = NOT_VALID;
  m_NumCmps          = NOT_VALID;

  m_BytesPerSample   = NOT_VALID;
  m_BuffCmpNumPels   = NOT_VALID;
  m_BuffCmpNumBytes  = NOT_VALID;

  m_POC              = NOT_VALID;
  m_Timestamp        = NOT_VALID;

  m_IsMarginExtended = false;
}

//===============================================================================================================================================================================================================
// xRentalCommon
//===============================================================================================================================================================================================================
xPicCommon* xRentalCommon::xBorrow()
{
  m_Mutex.lock();
  if(m_Buffer.empty()) { xCreateNewUnit(); }
  xPicCommon* Plane = m_Buffer.back();
  m_Buffer.pop_back();
  m_Mutex.unlock();
  return Plane;
}
void xRentalCommon::xGiveback(xPicCommon* Plane)
{
  m_Mutex.lock();
  assert(Plane != nullptr);
  Plane->setPOC(NOT_VALID);
  Plane->setTimestamp(NOT_VALID);
  m_Buffer.push_back(Plane);
  if(m_Buffer.size() > m_SizeLimit) { xDestroyUnit(); }
  m_Mutex.unlock();
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
