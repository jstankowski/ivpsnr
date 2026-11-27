/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xSeq.h"
#include "xPly.h"
#include "xXXH.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xCloudCache
{
protected:
  std::vector<FVGS::xGaussianCloudI*> m_Clouds;

public:
  void create (int32 NumClouds);
  void destroy();

  xSeqPic::tResult load(FVGS::xSeqPlyList* SeqPly, std::string InputFile, int32 StartFrame, bool PrintDebug);
  void             hash(xXXH128& HashCloud);
  const FVGS::xGaussianCloudI* getCloud(int32 Idx) const { return m_Clouds.at(Idx); }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB