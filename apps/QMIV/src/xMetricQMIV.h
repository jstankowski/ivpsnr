/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefIVQM.h"
#include "xVec.h"
#include "xTimeUtils.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

enum class eMetric : int32 //values must start from 0 and be continous
{
  UNDEFINED = -1,
  //PSNR - based
       MSE,
      PSNR,
    WSPSNR,
    IVPSNR,
  //SSIM - based
      SSIM,
    MSSSIM,
    IVSSIM,
  IVMSSSIM,
  //must be after last metric
  __NUM  
};

eMetric     xStrToMetric(const std::string_view Metric);
std::string xMetricToStr(eMetric Metric);

//===============================================================================================================================================================================================================

struct xMetricDescr
{
  const eMetric          Metric      ;
  const bool             IsPerCmp    ;
  const bool             IsPerPic    ;
  const bool             IsNormalized;
  const std::string_view Unit        ;
  const std::string_view Description ;

  constexpr xMetricDescr(eMetric Metric_, bool IsPerCmp_, bool IsPerPic_, bool IsNormalized_, std::string_view Unit_, std::string_view Description_)
    : Metric(Metric_), IsPerCmp(IsPerCmp_), IsPerPic(IsPerPic_), IsNormalized(IsNormalized_), Unit(Unit_), Description(Description_) {}
};

struct xMetricInfo
{
  static constexpr int32 MetricsNum       = (int32)eMetric::__NUM;
  static constexpr int32 MaxMetricNameLen = 8;

  static constexpr xMetricDescr Metrics[(int32)eMetric::__NUM] =
  {
    { eMetric::     MSE, true , false, false, "  ", "Mean squared error"                                                              },
    { eMetric::    PSNR, true , false, false, "dB", "Peak Signal-to-Noise Ratio"                                                      },
    { eMetric::  WSPSNR, true , false, false, "dB", "Spherical Weighted - Peak Signal-to-Noise Ratio"                                 },
    { eMetric::  IVPSNR, false, true , false, "dB", "Immersive Video - Peak Signal-to-Noise Ratio"                                    },
    { eMetric::    SSIM, true , false, true , "  ", "Structural Similarity Index Measure"                                             },
    { eMetric::  MSSSIM, true , false, true , "  ", "Multi Scale Structural Similarity Index Measure"                                 },
    { eMetric::  IVSSIM, false, true , true , "  ", "Immersive Video - Structural Similarity Index Measure"                           },
    { eMetric::IVMSSSIM, false, true , true , "  ", "Immersive Video - Multi Scale Structural Similarity Index Measure (experimental)"},
  };
};

//===============================================================================================================================================================================================================

class xMetricStat
{
protected:
  eMetric     m_Metric    = eMetric::UNDEFINED;
  std::string m_SuffixCmp = "";
  std::string m_SuffixPic = "";
  int32V4     m_CmpWeightsAverage = { 0, 0, 0, 0 };
  flt64       m_CmpWeightAverageInvDenom = 0.0;
  bool        m_MaskMode = false;
  uint64      m_SumTicks = 0;
  tDurationMS m_AvgDuration;
  std::vector<flt64V4> m_ValCmp;
  std::vector<flt64  > m_ValPic;
  flt64V4     m_AvgCmp;
  flt64       m_AvgPic;  
  bool        m_AnyFake  = false;
  bool        m_Enabled  = false;

public:
  eMetric getMetric () const { return m_Metric ; }
  bool    getEnabled() const { return m_Enabled; }

  void initMetric  (eMetric Metric, int32 NumFrames);
  void initSuffixes(bool UseMask, bool UseRGB);

  void initCmpWeightsAverage(const int32V4& CmpWeightsAverage);
  void setPerCmpMeric       (const flt64V4& PerCmpMetric, int32 FrameIdx);
  void setPerPicMeric       (flt64 PerPicMetric, int32 FrameIdx) { m_ValPic[FrameIdx] = PerPicMetric; }
  void setAnyFake           (bool AnyFake) { m_AnyFake = AnyFake; }
  void addTicks             (uint64 DurationTicks) { m_SumTicks += DurationTicks; }

  void        calcAvgMetric     (int32 NumFrames);
  std::string formatPerCmpMetric(int32 FrameIdx);
  std::string formatPerPicMetric(int32 FrameIdx);
  std::string formatAvgMetric   (const std::string LineHeader);
  void        calcAvgDuration   (flt64 InvDurationDenominator);
  std::string formatAvgTime     (const std::string LineHeader, tDurationMS PreMetricOps = tDurationMS(0));
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB