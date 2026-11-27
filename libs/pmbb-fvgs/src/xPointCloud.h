#pragma once
#include "xCommonDefFVGS.h"

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================

class xPointCloud
{
public:
  using tCoordEl  = flt32;
  using tColorEl  = flt32;
  using tCoordVec = xVec4<tCoordEl>;
  using tColorVec = xVec4<tColorEl>;

protected:
  tCoordEl* m_CoordsXYZA;
  tCoordEl* m_ColorsRGBA;

  int32     m_MaxNumPoints;
  int32     m_NumValidPoints;

public:
  xPointCloud ();
  ~xPointCloud() { destroy(); }
  void  create  (int32 MaxNumPoints);
  void  destroy (                  );
  void  resize  (int32 MaxNumPoints);
  void  reset   (                  ) { m_NumValidPoints = 0; }

  public:
  tCoordEl*        getCoordsPtrRaw()       { return                   m_CoordsXYZA; }
  tCoordVec*       getCoordsPtrVec()       { return (tCoordVec*      )m_CoordsXYZA; }
  const tCoordEl*  getCoordsPtrRaw() const { return                   m_CoordsXYZA; }
  const tCoordVec* getCoordsPtrVec() const { return (const tCoordVec*)m_CoordsXYZA; }

  tColorEl*        getColorsPtrRaw()       { return                   m_ColorsRGBA; }
  tColorVec*       getColorsPtrVec()       { return (tColorVec*      )m_ColorsRGBA; }
  const tColorEl*  getColorsPtrRaw() const { return                   m_ColorsRGBA; }
  const tColorVec* getColorsPtrVec() const { return (const tColorVec*)m_ColorsRGBA; }
  
  int32      getMaxNumPoints     () const { return m_MaxNumPoints; }

  int32      getNumValidPoints   (               ) const { return m_NumValidPoints; }
  void       modifyWritten       (int32 NumPoints) { m_NumValidPoints += NumPoints; }
  tCoordVec* getCoordsWritePtrVec(               ) { return getCoordsPtrVec() + m_NumValidPoints; }
  tColorVec* getColorsWritePtrVec(               ) { return getColorsPtrVec() + m_NumValidPoints; }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB



