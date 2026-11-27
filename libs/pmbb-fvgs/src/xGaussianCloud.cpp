/*
    SPDX-FileCopyrightText: 2025-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xGaussianCloud.h"
#include "xMemory.h"

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================

void xGaussianCloudP::create(int32 MaxNumSplats)
{
  m_MaxNumSplats   = MaxNumSplats;
  m_NumValidSplats = 0;

  const int32 AttrFLTCount = (int32)xAttrFLT::COUNT;
  for(int32 AttrFLTIdx = 0; AttrFLTIdx < AttrFLTCount; AttrFLTIdx++)
  {
    m_Attributes[AttrFLTIdx] = (flt32*)xMemory::xAlignedMallocPageAuto(m_MaxNumSplats * sizeof(flt32));
  }
}
void xGaussianCloudP::upsize(int32 MaxNumSplats)
{
  if(m_MaxNumSplats >= MaxNumSplats) { return; }

  m_MaxNumSplats   = MaxNumSplats;
  m_NumValidSplats = 0;

  const int32 AttrFLTCount = (size_t)xAttrFLT::COUNT;
  for(int32 AttrFLTIdx = 0; AttrFLTIdx < AttrFLTCount; AttrFLTIdx++)
  {
    if(m_Attributes[AttrFLTIdx] != nullptr) { xMemory::xAlignedFreeNull(m_Attributes[AttrFLTIdx]); }
    m_Attributes[AttrFLTIdx] = (flt32*)xMemory::xAlignedMallocPageAuto(m_MaxNumSplats * sizeof(flt32));
  }
}
void xGaussianCloudP::destroy()
{
  m_MaxNumSplats   = NOT_VALID;
  m_NumValidSplats = NOT_VALID;

  const int32 AttrFLTCount = (size_t)xAttrFLT::COUNT;
  for(int32 AttrFLTIdx = 0; AttrFLTIdx < AttrFLTCount; AttrFLTIdx++)
  {
    if(m_Attributes[AttrFLTIdx] != nullptr) { xMemory::xAlignedFreeNull(m_Attributes[AttrFLTIdx]); }
  }
}
void xGaussianCloudP::clear()
{
  const int32 AttrFLTCount = (int32)xAttrFLT::COUNT;
  for(int32 AttrFLTIdx = 0; AttrFLTIdx < AttrFLTCount; AttrFLTIdx++)
  {
    xMemsetX(m_Attributes[AttrFLTIdx], (flt32)0, m_MaxNumSplats);
  }
}

//===============================================================================================================================================================================================================

void xGaussianCloudI::create(int32 MaxNumSplats)
{
  m_MaxNumSplats   = MaxNumSplats;
  m_NumValidSplats = 0;

  m_GaussianObjects.resize(m_MaxNumSplats);
}
void xGaussianCloudI::upsize(int32 MaxNumSplats)
{
  if(m_MaxNumSplats >= MaxNumSplats) { return; }

  m_MaxNumSplats   = MaxNumSplats;
  m_NumValidSplats = 0;

  m_GaussianObjects.resize(m_MaxNumSplats);
}
void xGaussianCloudI::destroy()
{
  m_GaussianObjects.clear        ();
  m_GaussianObjects.shrink_to_fit();
}
void xGaussianCloudI::clear()
{
  memset(m_GaussianObjects.data(), 0, m_MaxNumSplats * sizeof(xGaussianObject));
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
