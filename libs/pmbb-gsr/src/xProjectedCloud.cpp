/*
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xProjectedCloud.h"

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================

void xProjectedCloud::upsize(int32 NumSplats)
{
  if(m_MaxNumSplats >= NumSplats) { return; }

  m_MaxNumSplats   = NumSplats;
  m_NumValidSplats = 0        ;

  if(m_Depth      ) { xMemory::xAlignedFreeNull(m_Depth      ); }
  if(m_BoundingBox) { xMemory::xAlignedFreeNull(m_BoundingBox); }
  if(m_SplatData  ) { xMemory::xAlignedFreeNull(m_SplatData  ); }
  if(m_SortedIdx  ) { xMemory::xAlignedFreeNull(m_SortedIdx  ); }

  m_Depth       = (flt32*       )xMemory::xAlignedMallocPageAuto(NumSplats * sizeof(flt32       ));
  m_BoundingBox = (xBoundingBox*)xMemory::xAlignedMallocPageAuto(NumSplats * sizeof(xBoundingBox));
  m_SplatData   = (xSplatData*  )xMemory::xAlignedMallocPageAuto(NumSplats * sizeof(xSplatData  ));
  m_SortedIdx   = (int32*       )xMemory::xAlignedMallocPageAuto(NumSplats * sizeof(int32       ));
}
void xProjectedCloud::destroy()
{
  m_MaxNumSplats   = 0;
  m_NumValidSplats = 0;

  if(m_Depth      ) { xMemory::xAlignedFreeNull(m_Depth      ); }
  if(m_BoundingBox) { xMemory::xAlignedFreeNull(m_BoundingBox); }
  if(m_SplatData  ) { xMemory::xAlignedFreeNull(m_SplatData  ); }
  if(m_SortedIdx  ) { xMemory::xAlignedFreeNull(m_SortedIdx  ); }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB::GSR
