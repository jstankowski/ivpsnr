/*
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefGSR.h"
#include "xMemory.h"

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================
// xProjectedCloudH - Hybrid (SoA/AoS) container for per-Gaussian data after projection
//===============================================================================================================================================================================================================
class xProjectedCloud
{
public:
  struct xBoundingBox //integer AABB (Axis Aligned Bounding Box) of the 3-sigma ellipse on screen (clamped to image bounds) 
  {
    uint16 MinX;
    uint16 MinY;
    uint16 MaxX;
    uint16 MaxY;
  };

  struct xSplatData
  {
    //2D screen position 
    flt32 ScreenX;
    flt32 ScreenY;
    //inverse 2D covariance [ic_A ic_B; ic_B ic_C] = Sigma_2D^{-1} stored so rasterizer can compute d^T * Sigma^{-1} * d directly
    flt32 InvCovA;
    flt32 InvCovB;
    flt32 InvCovC;
    //color from SH evaluation (linear, not clamped) 
    flt32 R;
    flt32 G;
    flt32 B;
    //opacity: sigmoid(OP) 
    flt32 A;
  };

protected:
  int32  m_MaxNumSplats   = 0;
  int32  m_NumValidSplats = 0; // filled by projector after culling

  flt32*        m_Depth       = nullptr; //depth (camera space Z, positive = in front) 
  xBoundingBox* m_BoundingBox = nullptr; //integer AABB (Axis Aligned Bounding Box)
  xSplatData*   m_SplatData   = nullptr;
  int32*        m_SortedIdx   = nullptr; //sort output: indices of valid splats ordered back-to-front

public:
  void upsize (int32 MaxNumSplats);
  void destroy();

  //buffer state
  int32 getMaxNumSplats  (       ) const { return m_MaxNumSplats  ; }
  int32 getNumValidSplats(       ) const { return m_NumValidSplats; }
  void  setNumValidSplats(int32 N)       { m_NumValidSplats = N   ; }

  void setDepth      (int32 Idx, flt32  V) { m_Depth[Idx] = V; }
  void setBoundingBox(int32 Idx, uint16 X0, uint16 Y0, uint16 X1, uint16 Y1) { xBoundingBox& BB = m_BoundingBox[Idx]; BB.MinX = X0; BB.MinY = Y0; BB.MaxX = X1; BB.MaxY = Y1; }
  void setSplatData  (int32 Idx, flt32 ScreenX, flt32 ScreenY, flt32 InvCovA, flt32 InvCovB, flt32 InvCovC, flt32 R, flt32 G, flt32 B, flt32 A) { xSplatData& SD = m_SplatData[Idx]; SD.ScreenX = ScreenX; SD.ScreenY = ScreenY; SD.InvCovA = InvCovA; SD.InvCovB = InvCovB; SD.InvCovC = InvCovC; SD.R = R; SD.G = G; SD.B = B; SD.A = A; }

  const flt32*        getDepth      () const { return m_Depth      ; }
  const xBoundingBox* getBoundingBox() const { return m_BoundingBox; }
  const xSplatData*   getSplatData  () const { return m_SplatData  ; }

        int32* getSortedIdx ()       { return m_SortedIdx;  }
  const int32* getSortedIdx () const { return m_SortedIdx;  }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB::GSR
