#include "xPointCloud.h"
#include "xMemory.h"

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================

xPointCloud::xPointCloud()
{
  m_MaxNumPoints   = NOT_VALID;
  m_NumValidPoints = NOT_VALID;
  m_CoordsXYZA     = nullptr;
  m_ColorsRGBA     = nullptr;
}
void xPointCloud::create(int32 MaxNumPoints)
{ 
  m_MaxNumPoints   = MaxNumPoints;
  m_NumValidPoints = 0;

  m_CoordsXYZA = (flt32*)xMemory::xAlignedMallocAuto(m_MaxNumPoints * sizeof(tCoordVec));
  m_ColorsRGBA = (flt32*)xMemory::xAlignedMallocAuto(m_MaxNumPoints * sizeof(tColorVec));
}
void xPointCloud::destroy()
{ 
  m_MaxNumPoints   = NOT_VALID;
  m_NumValidPoints = NOT_VALID;

  if(m_CoordsXYZA != nullptr) { xMemory::xAlignedFreeNull(m_CoordsXYZA); }
  if(m_ColorsRGBA != nullptr) { xMemory::xAlignedFreeNull(m_ColorsRGBA); }
}
void xPointCloud::resize(int32 MaxNumPoints)
{
  if(MaxNumPoints == m_MaxNumPoints) { m_NumValidPoints = 0; return; }
  destroy(            );
  create (MaxNumPoints);
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB



