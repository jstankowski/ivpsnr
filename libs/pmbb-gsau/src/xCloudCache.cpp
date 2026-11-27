/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xCloudCache.h"
#include "xUtilsAppGS.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xCloudCache::create(int32 NumClouds)
{ 
  m_Clouds.resize(NumClouds, nullptr);
  for(int32 f = 0; f < NumClouds; f++) { m_Clouds[f] = new FVGS::xGaussianCloudI; }
}
void xCloudCache::destroy()
{
  for(int32 f = 0; f < (int32)m_Clouds.size(); f++) { delete m_Clouds[f]; }
  m_Clouds.clear();
}
xSeqPic::tResult xCloudCache::load(FVGS::xSeqPlyList* SeqPly, std::string InputFile, int32 StartFrame, bool PrintDebug)
{
  const int32 NumFrames = (int32)m_Clouds.size();
  for(int32 f = 0; f < NumFrames; f++)
  {
    xSeqPic::tResult ReadResult = SeqPly->readGC(m_Clouds[f]);
    if(!ReadResult) { xErrMsg::printError(fmt::format("ERROR --> PoseTrace cloud cache read error ({}) {}", xFileListUtils::formatNamedPattern(InputFile, StartFrame + f, fmt::arg("FrameIdx", StartFrame + f)), ReadResult.format())); return eRetv::Error; }
    if(PrintDebug) { fmt::print("Reading cloud frame {}/{}\n", f + 1, NumFrames); }
  }
  return eRetv::Success;
}
void xCloudCache::hash(xXXH128& HashCloud)
{
  const int32 NumFrames = (int32)m_Clouds.size();
  for(int32 f = 0; f < NumFrames; f++)
  {
    xXXH128::tHash ImmHash = CalcCloudHash(m_Clouds[f]);
    HashCloud.update(ImmHash.data(), ImmHash.size());
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB