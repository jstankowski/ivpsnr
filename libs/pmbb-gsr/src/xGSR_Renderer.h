/*
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefGSR.h"
#include "xGaussianCloud.h"
#include "xProjectedCloud.h"
#include "xCameraParams.h"
#include "xMultiThreaded.h"

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================

class xRenderer : public xMultiThreaded
{
public:
  class xFrameStats
  {
  public:
    int32  FrameIdx        = NOT_VALID;
    int32  ViewIdx         = NOT_VALID;
    int32  SrcIdx          = NOT_VALID;
    uint64 TicksTotalFrame = 0;
    uint64 Ticks___Preproc = 0;
    uint64 Ticks___Project = 0;
    uint64 Ticks___Collect = 0;
    uint64 Ticks______Sort = 0;
    uint64 Ticks____Assign = 0;
    uint64 Ticks_Rasterize = 0;
    uint64 Ticks___Convert = 0;
    int32  NumSplatsCloud      = 0;
    int32  NumSplatsProjected  = 0;
    int32  NumSplatsAboveAlpha = 0; //of those, splats whose peak (centre) effective alpha clears 

    static std::string formatFrameStatsHead();
    std::string        formatFrameStatsData(flt64 TicksPerMilliSec) const;
  };

public:
  void create     (int32V2 PicSize, xThreadPool* ThreadPool = nullptr);
  void destroy    ();
  void renderFrame(xPicP* DstPic, tPicM* DstMsk, const tGaussI* Cloud, const tCamPar& CamParams); //DstMsk may be nullpth if alpha mask is not needed

protected:
  int32V2        m_PicSize   = { NOT_VALID, NOT_VALID };
  int32          m_NumTilesX = NOT_VALID;
  int32          m_NumTilesY = NOT_VALID;
  int32          m_NumTiles  = NOT_VALID;
  tTileSplatIdxM m_Tiles    ; // [NumTilesX * NumTilesY]
  tTileBuffPV    m_TileBuffs; //preallocated tile buffers for each thread

  const tGaussI*  m_Cloud = nullptr;
  tCamPar         m_CamParams;
  xProjectedCloud m_Projected;
  tValidV         m_ValidMask;
  tRstBuf         m_RenderBuf;

  //basic time stats
protected:
  bool   m_PrintDebug      = false;
  bool   m_GatherTimeStats = false;
  int64  m_TotalSynthIters = 0;
  uint64 m_TicksTotalFrame = 0;
  uint64 m_Ticks___Preproc = 0;
  uint64 m_Ticks___Project = 0;
  uint64 m_Ticks___Collect = 0;
  uint64 m_Ticks______Sort = 0;
  uint64 m_Ticks____Assign = 0;
  uint64 m_Ticks_Rasterize = 0;  
  uint64 m_Ticks___Convert = 0;  
public:
  void        setGatherTimeStats (bool GatherTimeStats) { m_GatherTimeStats = GatherTimeStats; }
  bool        getGatherTimeStats () const { return m_GatherTimeStats; }
  std::string formatAndResetStats(const std::string Prefix, flt64 TicksPerMilliSec);

//detailed frame stats
protected:
  bool                     m_GatherDetailed = false;  
  xFrameStats*             m_CurrentStats ;
  std::vector<xFrameStats> m_DetailedStats;
public:
  void        preRenderDetailedStats   (int32 FrameIdx, int32 ViewIdx, int32 SrcIdx); //SrcIdx = 0/1 Ref/Tst
  void        postRenderDetailedStats  ();
  std::string formatRenderDetailedStats(flt64 TicksPerMilliSec);

//internal routines
protected:
  void xPreProc(xPicP* DstPic, tPicM* DstMsk);

  void xProject();

  void xCollectValid();

  void xSortSequential    ();
  void xSortGlobalParallel();
  void xSortPerTile       ();

  void xAssignTile();

  void xRasterize (xPicP* DstPic, tPicM* DstMsk);

  void xConvertToPicP(xPicP* DstPic, tPicM* DstMsk, const tRstBuf* SrcBuf);

  static inline int32 xTileIdx(int32 TX, int32 TY, int32 NumTilesX) { return TY * NumTilesX + TX; }

};

//===============================================================================================================================================================================================================

} //end of namespace PMBB_NAMESPACE::GSR
