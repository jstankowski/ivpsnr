/*
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xGSR_Renderer.h"
#include "xGSR_Projector.h"
#include "xGSR_Rasterizer.h"
#include "xTimeUtils.h"
#include "xHelpersFLT.h"
#include "xMemory.h"
#include <numeric>
#include <cstring>

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================
// Settings
static        void xRadixSort (int32* Idx, int32 Count, const flt32* Depth);
static        void xStableSort(int32* Idx, int32 Count, const flt32* Depth);
static inline void xSort      (int32* Idx, int32 Count, const flt32* Depth) { xRadixSort(Idx, Count, Depth); }

enum class eSortMode { Sequential, GlobalParallel, PerTile };
constexpr  eSortMode c_SortMode       = eSortMode::PerTile;
constexpr  bool      c_ParallelAssign = true; //false=sequential xAssignTile (default), true=parallel version: count first, then write

//===============================================================================================================================================================================================================

std::string xRenderer::xFrameStats::formatFrameStatsHead()
{
  std::string Result;
  Result += fmt::format("{:>8} {:>4} {:>9} {:>10} {:>10}  {:>8} {:>8} {:>8} {:>8} {:>8} {:>9} {:>8}  {:>8}\n",
    "Frame", "View", "SplatsOrg", "SplatsView", "SplatsUsed", "Preproc", "Project", "Collect", "Sort", "Assign", "Rasterize", "Convert", "Total");
  Result += fmt::format("{:>8} {:>4} {:>9} {:>10} {:>10}  {:>8} {:>8} {:>8} {:>8} {:>8} {:>9} {:>8}  {:>8}\n",
    "", "", "", "", "", "[ms]", "[ms]", "[ms]", "[ms]", "[ms]", "[ms]", "[ms]", "[ms]");
  return Result;
}
std::string xRenderer::xFrameStats::formatFrameStatsData(flt64 TicksPerMilliSec) const
{
  flt64 InvDenom = TicksPerMilliSec == 0.0 ? (flt64)1.0 : (flt64)1.0 / TicksPerMilliSec;

  std::string Result = fmt::format("{:08d} {:>4} {:>9} {:>10} {:>10}  {:8.2f} {:8.2f} {:8.2f} {:8.2f} {:8.2f} {:9.2f} {:8.2f}  {:8.2f}\n",
    FrameIdx          ,
    ViewIdx           ,
    NumSplatsCloud    ,
    NumSplatsProjected,
    NumSplatsProjected,
    Ticks___Preproc * InvDenom,
    Ticks___Project * InvDenom,
    Ticks___Collect * InvDenom,
    Ticks______Sort * InvDenom,
    Ticks____Assign * InvDenom,
    Ticks_Rasterize * InvDenom,
    Ticks___Convert * InvDenom,
    TicksTotalFrame * InvDenom);

  return Result;
}

//===============================================================================================================================================================================================================

void xRenderer::create(int32V2 PictureSize, xThreadPool* ThreadPool)
{
  m_PicSize   = PictureSize;
  m_NumTilesX = xNumUnitsCoveringLength(PictureSize.getX(), c_Log2TileWidth );
  m_NumTilesY = xNumUnitsCoveringLength(PictureSize.getY(), c_Log2TileHeight);
  m_NumTiles  = m_NumTilesX * m_NumTilesY;

  int32 MaxNumTasks = xMax(4096, m_NumTiles);
  createThrdPoolIntf(ThreadPool, MaxNumTasks);

  if(ThreadPool != nullptr) //preallocate tile buffers for each thread
  {
    const int32 NumThreads = ThreadPool->getNumThreads();
    m_TileBuffs.resize(NumThreads, nullptr);
    for(int32 ThreadIdx = 0; ThreadIdx < NumThreads; ThreadIdx++) { m_TileBuffs[ThreadIdx] = (tTileBuff*)xMemory::xAlignedMallocPageAuto(sizeof(tTileBuff)); }
  }

  m_RenderBuf.create(PictureSize, 0);

  m_Tiles.resize(m_NumTiles);
  for(int32 t = 0; t < m_NumTiles; t++) { m_Tiles[t].reserve(1024); }
}
void xRenderer::destroy()
{
  for(int32 ThreadIdx = 0; ThreadIdx < (int32)m_TileBuffs.size(); ThreadIdx++) { xMemory::xAlignedFree(m_TileBuffs[ThreadIdx]); }
  m_TileBuffs.clear();

  m_RenderBuf.destroy();
  m_Tiles.clear();
  destroyThrdPoolIntf();
}
std::string xRenderer::formatAndResetStats(const std::string Prefix, flt64 TicksPerMilliSec)
{
  if(!m_GatherTimeStats) { return "Time stats gathering is disabled!"; }
  if(!m_TotalSynthIters) { return "No time stats gathered!";           }

  flt64 InvDenom = TicksPerMilliSec == 0.0 ? (flt64)1.0 / ((flt64)m_TotalSynthIters) : (flt64)1.0 / ((flt64)m_TotalSynthIters * TicksPerMilliSec);

  std::string TimeStats; TimeStats.reserve(xMemory::getBestEffortSizePageBase());
  TimeStats += Prefix + fmt::format("Processing time {}\n", TicksPerMilliSec == 0.0 ? "[ticks]" : "[ms]");

  TimeStats += Prefix + "  " + fmt::format("SynthIters  = {}\n"    , m_TotalSynthIters);
  TimeStats += Prefix + "  " + fmt::format("TotalFrame  = {:.2f}\n", m_TicksTotalFrame * InvDenom);
  TimeStats += Prefix + "  " + fmt::format("Preproc     = {:.2f}\n", m_Ticks___Preproc * InvDenom);
  TimeStats += Prefix + "  " + fmt::format("Project     = {:.2f}\n", m_Ticks___Project * InvDenom);
  TimeStats += Prefix + "  " + fmt::format("Collect     = {:.2f}\n", m_Ticks___Collect * InvDenom);
  TimeStats += Prefix + "  " + fmt::format("Sort        = {:.2f}\n", m_Ticks______Sort * InvDenom);
  TimeStats += Prefix + "  " + fmt::format("Assign      = {:.2f}\n", m_Ticks____Assign * InvDenom);
  TimeStats += Prefix + "  " + fmt::format("Rasterize   = {:.2f}\n", m_Ticks_Rasterize * InvDenom);
  TimeStats += Prefix + "  " + fmt::format("Convert     = {:.2f}\n", m_Ticks___Convert * InvDenom);

  m_TotalSynthIters = 0;
  m_TicksTotalFrame = 0;
  m_Ticks___Preproc = 0;
  m_Ticks___Project = 0;
  m_Ticks___Collect = 0;
  m_Ticks______Sort = 0;
  m_Ticks____Assign = 0;
  m_Ticks_Rasterize = 0;
  m_Ticks___Convert = 0;

  return TimeStats;
}
void xRenderer::renderFrame(xPicP* DstPic, tPicM* DstMsk, const tGaussI* Cloud, const tCamPar& CamParams)
{
  assert(DstPic != nullptr && Cloud != nullptr);
  if(DstMsk) { assert(DstMsk->isSameSize(DstPic->getSize())); }
  const uint64 T        = m_GatherTimeStats ? xTSC() : 0;
  const bool   UseTiles = m_ThPI->isActive();

  m_Cloud     = Cloud    ;
  m_CamParams = CamParams;

  xPreProc(DstPic, DstMsk);
  xProject();

  if(UseTiles)
  {
    if constexpr(c_SortMode == eSortMode::Sequential)
    {
      xCollectValid  ();
      xSortSequential();
      xAssignTile    ();
    }
    else if(c_SortMode == eSortMode::GlobalParallel)
    {
      xCollectValid      ();
      xSortGlobalParallel();
      xAssignTile        ();
    }
    else if(c_SortMode == eSortMode::PerTile)
    {
      xCollectValid();
      xAssignTile  ();
      xSortPerTile ();
    }
  }
  else
  {
    xCollectValid  ();
    xSortSequential();
  }

  xRasterize(DstPic, DstMsk);
  
  if(!UseTiles) { xConvertToPicP(DstPic, DstMsk, &m_RenderBuf); }

  if(m_GatherTimeStats){ m_TicksTotalFrame += (xTSC() - T); m_TotalSynthIters++; }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xRenderer::preRenderDetailedStats(int32 FrameIdx, int32 ViewIdx, int32 SrcIdx)
{
  m_GatherDetailed = true;
  m_CurrentStats   = new xFrameStats();
  m_CurrentStats->FrameIdx        = FrameIdx         ;
  m_CurrentStats->ViewIdx         = ViewIdx          ;
  m_CurrentStats->SrcIdx          = SrcIdx           ;
  //backup pre-render cumulative ticks counters
  m_CurrentStats->TicksTotalFrame = m_TicksTotalFrame;
  m_CurrentStats->Ticks___Preproc = m_Ticks___Preproc;
  m_CurrentStats->Ticks___Project = m_Ticks___Project;
  m_CurrentStats->Ticks___Collect = m_Ticks___Collect;
  m_CurrentStats->Ticks______Sort = m_Ticks______Sort;
  m_CurrentStats->Ticks____Assign = m_Ticks____Assign;
  m_CurrentStats->Ticks_Rasterize = m_Ticks_Rasterize;
  m_CurrentStats->Ticks___Convert = m_Ticks___Convert;
}
void xRenderer::postRenderDetailedStats()
{
  m_CurrentStats->TicksTotalFrame    = m_TicksTotalFrame - m_CurrentStats->TicksTotalFrame;
  m_CurrentStats->Ticks___Preproc    = m_Ticks___Preproc - m_CurrentStats->Ticks___Preproc;
  m_CurrentStats->Ticks___Project    = m_Ticks___Project - m_CurrentStats->Ticks___Project;
  m_CurrentStats->Ticks___Collect    = m_Ticks___Collect - m_CurrentStats->Ticks___Collect;
  m_CurrentStats->Ticks______Sort    = m_Ticks______Sort - m_CurrentStats->Ticks______Sort;
  m_CurrentStats->Ticks____Assign    = m_Ticks____Assign - m_CurrentStats->Ticks____Assign;
  m_CurrentStats->Ticks_Rasterize    = m_Ticks_Rasterize - m_CurrentStats->Ticks_Rasterize;
  m_CurrentStats->Ticks___Convert    = m_Ticks___Convert - m_CurrentStats->Ticks___Convert;
  m_CurrentStats->NumSplatsCloud     = m_Cloud->getNumValidSplats   ();
  m_CurrentStats->NumSplatsProjected = m_Projected.getNumValidSplats();
  m_DetailedStats.push_back(*m_CurrentStats);
  delete m_CurrentStats; m_CurrentStats = nullptr;
}
std::string xRenderer::formatRenderDetailedStats(flt64 TicksPerMilliSec)
{
  std::string Result;
  Result += xFrameStats::formatFrameStatsHead();

  for(const xFrameStats& DS : m_DetailedStats) { Result += DS.formatFrameStatsData(TicksPerMilliSec); }

  Result += "\n";
  return Result;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xRenderer::xPreProc(xPicP* DstPic, tPicM* DstMsk)
{
  const uint64 T = m_GatherTimeStats ? xTSC() : 0;
  const int32 NumSplats = m_Cloud->getNumValidSplats();
  if(m_ValidMask.size() < NumSplats) { m_ValidMask.resize(NumSplats); }
  m_Projected.upsize(NumSplats);
  DstPic->clear();
  if(DstMsk != nullptr) { DstMsk->clear(); }
  if(m_GatherTimeStats) { m_Ticks___Preproc += (xTSC() - T); }
}
void xRenderer::xProject()
{
  const uint64 T         = m_GatherTimeStats ? xTSC() : 0;
  const int32  NumSplats = m_Cloud->getNumValidSplats();

  if(m_ThPI->isActive())
  {
    const int32 MaxNumTasks   = m_ThPI->getMaxNumTasksInBatch();
    const int32 DefNumBatches = (int32)ceil((flt32)NumSplats / (flt32)c_DefProjBatchSize);
    const int32 SafeBatchSize = (int32)ceil((flt32)NumSplats / (flt32)MaxNumTasks);
    const int32 BatchSize     = DefNumBatches < MaxNumTasks ? c_DefProjBatchSize : SafeBatchSize;

    for(int32 BegIdx = 0; BegIdx < NumSplats; BegIdx += BatchSize)
    {
      const int32 EndIdx = xMin(BegIdx + BatchSize, NumSplats);
      m_ThPI->storeTask([this, BegIdx, EndIdx](int32 /*ThreadIdx*/) { xProjector::ProjectRng(&m_Projected, m_ValidMask.data(), m_Cloud, m_CamParams, m_PicSize, BegIdx, EndIdx); });
    }
    m_ThPI->executeStoredTasks();
  }
  else
  {
    xProjector::ProjectRng(&m_Projected, m_ValidMask.data(), m_Cloud, m_CamParams, m_PicSize, 0, NumSplats);
  }
  if(m_GatherTimeStats) { m_Ticks___Project += (xTSC() - T); }
}
//void xRenderer::xCollectValid()
//{
//  const uint64 T = m_GatherTimeStats ? xTSC() : 0;
//  const int32 NumSplats = m_Cloud->getNumValidSplats();
//  int32* SortedIdx = m_Projected.getSortedIdx();
//
//  uint32 NumValid = 0;
//  for(int32 i = 0; i < NumSplats; i++) { if(m_ValidMask[i]) { SortedIdx[NumValid++] = i; } }
//  m_Projected.setNumValidSplats(NumValid);
//  if(m_GatherTimeStats) { m_Ticks___Collect += (xTSC() - T); }
//}
void xRenderer::xCollectValid()
{
  const uint64 T = m_GatherTimeStats ? xTSC() : 0;
  const int32 NumSplats = m_Cloud->getNumValidSplats();
  int32* SortedIdx = m_Projected.getSortedIdx();
  const xProjectedCloud::xSplatData* SplatData = m_Projected.getSplatData();

  uint32 NumValid = 0;      //geometric-only (depth + AABB), reported as SplatsView - NOT what gets sorted/rasterized below
  uint32 NumAboveAlpha = 0; //geometric + alpha threshold, reported as SplatsUsed - this is what actually gets sorted/rasterized
  for(int32 i = 0; i < NumSplats; i++)
  {
    if(!m_ValidMask[i]) { continue; }
    NumValid++;
    const flt32 AlphaPeak = std::min(SplatData[i].A, c_EffectiveAlphaCap); //peak = splat centre, Gaussian weight 1.0
    if(AlphaPeak < c_AlphaThreshold) { continue; } //below-threshold splats never composite into any pixel - skip Sort/Assign/Rasterize for them entirely
    SortedIdx[NumAboveAlpha++] = i;
  }
  m_Projected.setNumValidSplats(NumAboveAlpha);
  if(m_GatherTimeStats) { m_Ticks___Collect += (xTSC() - T); }
}
void xRenderer::xSortGlobalParallel()
{
  int32*       SortedIdx = m_Projected.getSortedIdx();
  const int32  NumValid  = m_Projected.getNumValidSplats();
  const flt32* Depth     = m_Projected.getDepth();

  const auto CmpFn = [Depth](int32 a, int32 b) { return Depth[a] < Depth[b]; };

  const int32 NumThreads = m_ThPI->isActive() ? m_ThPI->getNumThreads() : 0;

  if(NumThreads < 2) { xSortSequential(); return; }

  const int32 NumChunks = NumThreads;
  const int32 ChunkSize = ((int32)NumValid + NumChunks - 1) / NumChunks;

  for(int32 c = 0; c < NumChunks; c++)
  {
    const int32 Begin = c * ChunkSize;
    const int32 End   = std::min(Begin + ChunkSize, (int32)NumValid);
    if(Begin >= End) { continue; }
    m_ThPI->storeTask([SortedIdx, Begin, End, Depth](int32) { xSort(SortedIdx + Begin, End - Begin, Depth); });
  }
  m_ThPI->executeStoredTasks();

  for(int32 Width = ChunkSize; Width < (int32)NumValid; Width *= 2)
  {
    for(int32 Begin = 0; Begin < (int32)NumValid; Begin += 2 * Width)
    {
      const int32 Mid = std::min(Begin + Width, (int32)NumValid);
      const int32 End = std::min(Begin + 2 * Width, (int32)NumValid);
      if(Mid >= End) { continue; }
      m_ThPI->storeTask([SortedIdx, Begin, Mid, End, CmpFn](int32) { std::inplace_merge(SortedIdx + Begin, SortedIdx + Mid, SortedIdx + End, CmpFn); });
    }
    m_ThPI->executeStoredTasks();
  }
}
void xRenderer::xSortPerTile()
{
  const uint64 T = m_GatherTimeStats ? xTSC() : 0;

  const flt32* Depth = m_Projected.getDepth();

  for(int32 t = 0; t < m_NumTiles; t++)
  {
    if(m_Tiles[t].size() <= 1) { continue; }
    m_ThPI->storeTask([this, t, Depth](int32)
      {
        tTileSplatIdxV& Tile = m_Tiles[t];
        xSort(Tile.data(), (int32)Tile.size(), Depth);
      });
  }
  m_ThPI->executeStoredTasks();

  if(m_GatherTimeStats) { m_Ticks______Sort += (xTSC() - T); }
}
void xRenderer::xSortSequential()
{
  const uint64 T = m_GatherTimeStats ? xTSC() : 0;

  int32*       SortedIdx = m_Projected.getSortedIdx     ();
  const int32  NumValid  = m_Projected.getNumValidSplats();
  const flt32* Depth     = m_Projected.getDepth         ();

  // sort front-to-back: ascending depth (nearest first)
  xSort(SortedIdx, NumValid, Depth); // smaller Z = nearer to camera = rendered first

  if(m_GatherTimeStats) { m_Ticks______Sort += (xTSC() - T); }
}
//convert a splat's pixel AABB to a tile AABB (limited to the picture's tile grid)
static inline void xTileRange(const xProjectedCloud::xBoundingBox& BB, int32 NumTilesX, int32 NumTilesY, int32& TX0, int32& TY0, int32& TX1, int32& TY1)
{
  TX0 = BB.MinX >> c_Log2TileWidth ;
  TY0 = BB.MinY >> c_Log2TileHeight;
  TX1 = std::min(NumTilesX - 1, BB.MaxX >> c_Log2TileWidth );
  TY1 = std::min(NumTilesY - 1, BB.MaxY >> c_Log2TileHeight);
}

void xRenderer::xAssignTile()
{
  using tBBox = xProjectedCloud::xBoundingBox;

  const uint64 T = m_GatherTimeStats ? xTSC() : 0;

  const int32  NumValid   = m_Projected.getNumValidSplats();
  const int32* SortedIdx  = m_Projected.getSortedIdx     ();
  const tBBox* BBoxes     = m_Projected.getBoundingBox   ();
  const int32  NumThreads = m_ThPI->isActive() ? m_ThPI->getNumThreads() : 0;
  const int32  NumTilesX  = m_NumTilesX;
  const int32  NumTilesY  = m_NumTilesY;
  const int32  NumTiles   = m_NumTiles ;

  const auto PrintTileStatsIfNeeded = [this]()
  {
    if(!m_PrintDebug) { return; }
    uint64 SumNumSplats = 0;
    uint64 MinNumSplats = std::numeric_limits<uint64>::max();
    uint64 MaxNumSplats = 0;
    for(const tTileSplatIdxV& Tile : m_Tiles)
    {
      SumNumSplats += Tile.size();
      MinNumSplats = xMin(MinNumSplats, Tile.size());
      MaxNumSplats = xMax(MaxNumSplats, Tile.size());
    }
    flt32 AvgNumSplats = (flt32)SumNumSplats / (flt32)m_NumTiles;
    fmt::print("TileStats Size={}x{} Num={} Splats Avg={} Min={} Max={}\n", c_TileWidth, c_TileHeight, m_NumTiles, AvgNumSplats, MinNumSplats, MaxNumSplats);
  };

  if constexpr(c_ParallelAssign)
  {
    if(NumThreads >= 2 && NumValid > 0)
    {
      const int32 NumChunks  = NumThreads;
      const int32 ChunkSize  = (NumValid + NumThreads - 1) / NumThreads;

      //Phase 1 (parallel): for each chunk, count how many of its splats touch each tile
      std::vector<int32> LocalCounts((size_t)NumChunks * NumTiles, 0);
      for(int32 c = 0; c < NumChunks; c++)
      {
        const int32 Begin = c * ChunkSize;
        const int32 End   = std::min(Begin + ChunkSize, NumValid);
        if(Begin >= End) { continue; }
        int32* Counts = &LocalCounts[(size_t)c * NumTiles];
        m_ThPI->storeTask([SortedIdx, BBoxes, Begin, End, NumTilesX, NumTilesY, Counts](int32)
          {
            for(int32 i = Begin; i < End; i++)
            {
              int32 TX0, TY0, TX1, TY1; xTileRange(BBoxes[SortedIdx[i]], NumTilesX, NumTilesY, TX0, TY0, TX1, TY1);
              for(int32 TY = TY0; TY <= TY1; TY++)
              {
                for(int32 TX = TX0; TX <= TX1; TX++)
                {
                  Counts[xTileIdx(TX, TY, NumTilesX)]++;
                }
              }
            }
          });
      }
      m_ThPI->executeStoredTasks();

      //Phase 2 (sequential, cheap): resize each tile once, then compute each chunk's starting write offset per tile (a prefix sum across chunks)
      std::vector<int32> ChunkOffset((size_t)NumChunks * NumTiles, 0);
      for(int32 Tile = 0; Tile < NumTiles; Tile++)
      {
        int32 Acc = 0;
        for(int32 c = 0; c < NumChunks; c++)
        {
          ChunkOffset[(size_t)c * NumTiles + Tile] = Acc;
          Acc += LocalCounts[(size_t)c * NumTiles + Tile];
        }
        m_Tiles[Tile].resize(Acc);
      }

      //Phase 3 (parallel): write each splat into the position computed in Phase 2. No atomics or locks are needed, because each chunk writes to its own part of each tile
      for(int32 c = 0; c < NumChunks; c++)
      {
        const int32 Begin = c * ChunkSize;
        const int32 End   = std::min(Begin + ChunkSize, NumValid);
        if(Begin >= End) { continue; }
        const int32* Offsets = &ChunkOffset[(size_t)c * NumTiles];
        m_ThPI->storeTask([this, SortedIdx, BBoxes, Begin, End, NumTilesX, NumTilesY, NumTiles, Offsets](int32)
          {
            std::vector<int32> Cursor(Offsets, Offsets + NumTiles);
            for(int32 i = Begin; i < End; i++)
            {
              const int32 ProjIdx = SortedIdx[i];
              int32 TX0, TY0, TX1, TY1; xTileRange(BBoxes[ProjIdx], NumTilesX, NumTilesY, TX0, TY0, TX1, TY1);
              for(int32 TY = TY0; TY <= TY1; TY++)
              {
                for(int32 TX = TX0; TX <= TX1; TX++)
                {
                  const int32 TileIdx = xTileIdx(TX, TY, NumTilesX);
                  m_Tiles[TileIdx][Cursor[TileIdx]++] = ProjIdx;
                }
              }
            }
          });
      }
      m_ThPI->executeStoredTasks();

      if(m_GatherTimeStats) { m_Ticks____Assign += (xTSC() - T); }
      PrintTileStatsIfNeeded();
      return;
    }
  }

  for(int32 t = 0; t < NumTiles; t++) { m_Tiles[t].clear(); }
  for(int32 i = 0; i < NumValid; i++)
  {
    const int32 ProjIdx = SortedIdx[i];
    int32 TX0, TY0, TX1, TY1; xTileRange(BBoxes[ProjIdx], NumTilesX, NumTilesY, TX0, TY0, TX1, TY1);
    for(int32 TY = TY0; TY <= TY1; TY++)
    {
      for(int32 TX = TX0; TX <= TX1; TX++)
      {
        m_Tiles[xTileIdx(TX, TY, NumTilesX)].push_back(ProjIdx);
      }
    }
  }

  if(m_GatherTimeStats) { m_Ticks____Assign += (xTSC() - T); }
  PrintTileStatsIfNeeded();
}
void xRenderer::xRasterize(xPicP* DstPic, tPicM* DstMsk)
{
  const uint64 T = m_GatherTimeStats ? xTSC() : 0;

  const bool UseTiles = m_ThPI->isActive();
  if(UseTiles)
  {
    for(int32 TY = 0; TY < m_NumTilesY; TY++)
    {
      for(int32 TX = 0; TX < m_NumTilesX; TX++)
      {
        const int32 TileIdx = xTileIdx(TX, TY, m_NumTilesX);
        m_ThPI->storeTask([this, DstPic, DstMsk, TX, TY, TileIdx](int32 ThreadIdx) { xRasterizer::RasterizeTile(DstPic, DstMsk, m_Projected, m_Tiles[TileIdx], { TX, TY }, m_TileBuffs[ThreadIdx]); });
      }
    }
    m_ThPI->executeStoredTasks();
  }
  else
  {
    xRasterizer::RasterizePic(&m_RenderBuf, m_Projected);
  }
  if(m_GatherTimeStats) { m_Ticks_Rasterize += (xTSC() - T); }
}
void xRenderer::xConvertToPicP(xPicP* DstPic, tPicM* DstMsk, const tRstBuf* SrcBuf)
{
  const uint64 T = m_GatherTimeStats ? xTSC() : 0;

  const int32  Width      = DstPic->getWidth ();
  const int32  Height     = DstPic->getHeight();

  //convert picture
  {
    const int32  SrcStrideP = SrcBuf->getStride();
    const int32  DstStrideP = DstPic->getStride();
    const int32  MaxValueP  = xBitDepth2MaxValue(DstPic->getBitDepth());

    for(int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
      const flt32*     SrcPtr = SrcBuf->getAddr((eCmp)CmpIdx);
      uint16* restrict DstPtr = DstPic->getAddr((eCmp)CmpIdx);

      for(int32 y = 0; y < Height; y++)
      {
        for(int32 x = 0; x < Width; x++)
        {
          uint16 DstVal = (uint16)xClip(xRoundF32ToI32(SrcPtr[x] * (flt32)MaxValueP), 0, MaxValueP);
          DstPtr[x] = DstVal;
        }
        SrcPtr += SrcStrideP;
        DstPtr += DstStrideP;
      }
    }
  }

  //convert mask
  if(DstMsk != nullptr)
  {
    const int32  SrcStrideM = DstMsk->getStride();
    const int32  DstStrideM = DstMsk->getStride();
    const int32  MaxValueM  = xBitDepth2MaxValue(DstMsk->getBitDepth());

    const flt32*     SrcPtr = SrcBuf->getAddr(eCmp::A);
    uint16* restrict DstPtr = DstMsk->getAddr();

    for(int32 y = 0; y < Height; y++)
    {
      for(int32 x = 0; x < Width; x++)
      {
        uint16 DstVal = (uint16)xClip(xRoundF32ToI32(SrcPtr[x] * (flt32)MaxValueM), 0, MaxValueM);
        DstPtr[x] = DstVal;
      }
      SrcPtr += SrcStrideM;
      DstPtr += DstStrideM;
    }
  }

  if(m_GatherTimeStats) { m_Ticks___Convert += (xTSC() - T); }
}

//===============================================================================================================================================================================================================
// xSort implementations
//===============================================================================================================================================================================================================

//order-preserving float->uint32 key for radix sort (bit-trick manipulation)
static inline uint32 xSortKey(flt32 f)
{
  uint32 Bits; memcpy(&Bits, &f, sizeof(uint32));
  return (Bits & 0x80000000u) ? ~Bits : (Bits | 0x80000000u);
}

//sorts Idx[0..Count) by ascending Depth[Idx[i]]. It is stable, so it keeps the same order as std::stable_sort when two values are equal. It uses 4 passes of 8 bits each, over the uint32 key from xSortKey
static void xRadixSort(int32* Idx, int32 Count, const flt32* Depth)
{
  if(Count < 2) { return; }

  std::vector<uint32> Keys(Count);
  for(int32 i = 0; i < Count; i++) { Keys[i] = xSortKey(Depth[Idx[i]]); }

  std::vector<int32>  TmpIdx (Count);
  std::vector<uint32> TmpKeys(Count);

  int32*  CurIdx  = Idx;         int32*  NxtIdx  = TmpIdx .data();
  uint32* CurKeys = Keys.data(); uint32* NxtKeys = TmpKeys.data();

  for(int32 Shift = 0; Shift < 32; Shift += 8)
  {
    int32 Hist[256] = {};
    for(int32 i = 0; i < Count; i++) { Hist[(CurKeys[i] >> Shift) & 0xFF]++; }
    int32 Offset[256]; int32 Acc = 0;
    for(int32 b = 0; b < 256; b++) { Offset[b] = Acc; Acc += Hist[b]; }
    for(int32 i = 0; i < Count; i++)
    {
      const int32 b = (CurKeys[i] >> Shift) & 0xFF;
      const int32 d = Offset[b]++;
      NxtIdx [d] = CurIdx [i];
      NxtKeys[d] = CurKeys[i];
    }
    std::swap(CurIdx, NxtIdx);
    std::swap(CurKeys, NxtKeys);
  }
  //there are 4 passes (an even number), so the sorted result always ends up back in the original Idx array
}

//std::stable_sort of Idx[0..Count) by ascending Depth[Idx[i]]
static inline void xStableSort(int32* Idx, int32 Count, const flt32* Depth)
{
  std::stable_sort(Idx, Idx + Count, [Depth](int32 a, int32 b) { return Depth[a] < Depth[b]; });
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB_NAMESPACE::GSR
