/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefIVQM.h"
#include "xUtilsAppQM.h"
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
#if X_PMBB_STALLED
  MSIVSSIM,
#endif //X_PMBB_STALLED
     //PVD
       PVD,
#if X_PMBB_EXPERIMENTAL
     IVPVD,
#endif //X_PMBB_EXPERIMENTAL
  //must be after last metric
     __NUM  
};

eMetric     xStrToMetric(const std::string_view Metric);
std::string xMetricToStr(eMetric Metric);

//===============================================================================================================================================================================================================

enum class eClrSpcMtr
{
  UNDEFINED = NOT_VALID,
  YCbCr     = 0,
  RGB       = 1,
  __NUM     = 2,
};

eClrSpcMtr xClrSpcAppToClrSpcMtr(eClrSpcApp ClrSpcApp);

//===============================================================================================================================================================================================================

struct xMetricDescr
{
  const eMetric          Metric      ;
  const bool             IsDefault   ; //D              
  const bool             IsPerCmp    ; //C
  const bool             IsPerPic    ; //P
  const bool             IsNormalized; //N
  const bool             AvgInMSE    ; //M
  const std::string_view Unit        ;
  const std::string_view Description ;

  constexpr xMetricDescr(eMetric Metric_, bool IsDefault_, bool IsPerCmp_, bool IsPerPic_, bool IsNormalized_, bool AvgInMSE_, std::string_view Unit_, std::string_view Description_)
    : Metric(Metric_), IsDefault(IsDefault_), IsPerCmp(IsPerCmp_), IsPerPic(IsPerPic_), IsNormalized(IsNormalized_), AvgInMSE(AvgInMSE_), Unit(Unit_), Description(Description_) {}
};

struct xMetricInfo
{
  static constexpr int32 MetricsNum       = (int32)eMetric::__NUM;
  static constexpr int32 MaxMetricNameLen = 8;

  static constexpr xMetricDescr Metrics[(int32)eMetric::__NUM] =
  {
    //           Metric  D  C  P  N  M  Unit  Description
    { eMetric::     MSE, 0, 1, 0, 0, 0, "  ", "Mean squared error"                                                },
    { eMetric::    PSNR, 1, 1, 0, 0, 1, "dB", "Peak Signal-to-Noise Ratio"                                        },
    { eMetric::  WSPSNR, 0, 1, 0, 0, 1, "dB", "Spherical Weighted - Peak Signal-to-Noise Ratio"                   },
    { eMetric::  IVPSNR, 1, 0, 1, 0, 1, "dB", "Immersive Video - Peak Signal-to-Noise Ratio"                      },
    { eMetric::    SSIM, 0, 1, 0, 1, 0, "  ", "Structural Similarity Index Measure"                               },
    { eMetric::  MSSSIM, 0, 1, 0, 1, 0, "  ", "Multi Scale Structural Similarity Index Measure"                   },
    { eMetric::  IVSSIM, 1, 0, 1, 1, 0, "  ", "Immersive Video - Structural Similarity Index Measure"             },
    { eMetric::IVMSSSIM, 0, 0, 1, 1, 0, "  ", "Immersive Video - Multi Scale Structural Similarity Index Measure" },
#if X_PMBB_STALLED
    { eMetric::MSIVSSIM, 0, 0, 1, 1, 0, "  ", "Multi Scale - Immersive Video Structural Similarity Index Measure" },
#endif //X_PMBB_STALLED
    { eMetric::     PVD, 0, 0, 1, 1, 0, "  ", "pVAR - Perceptual Variance of Distortion"                          },
#if X_PMBB_EXPERIMENTAL
    { eMetric::   IVPVD, 0, 0, 1, 1, 0, "  ", "Immersive Video - Perceptual Variance of Distortion (experimental)"},
#endif //X_PMBB_EXPERIMENTAL
  };
};

//===============================================================================================================================================================================================================

class xMetricStat
{
public:
  static constexpr int32 c_NumClrSpcs = (int32)eClrSpcMtr::__NUM;
protected:
  eMetric     m_Metric    = eMetric::UNDEFINED;
  int32       m_NumFrames = NOT_VALID;
  int32       m_NumViews  = NOT_VALID;
  std::string m_SuffixCmp   [c_NumClrSpcs];
  std::string m_SuffixPic   [c_NumClrSpcs];
  std::string m_SuffixPicMSE[c_NumClrSpcs];
  //component weighting
  int32V4     m_CmpWeightsAverage       [c_NumClrSpcs] = {{0, 0, 0, 0}, {0, 0, 0, 0} };
  flt64       m_CmpWeightAverageInvDenom[c_NumClrSpcs] = { 0.0, 0.0 };
  //time stats
  uint64      m_SumTicks = 0;
  uint64      m_NumIters = 0;
  tDurationMS m_AvgDuration;
  //metric value
  std::vector<flt64V4> m_ValCmp   [c_NumClrSpcs];
  std::vector<flt64  > m_ValPic   [c_NumClrSpcs];
  std::vector<flt64  > m_ValPicMSE[c_NumClrSpcs];
  flt64V4              m_AvgCmp   [c_NumClrSpcs];
  flt64                m_AvgPic   [c_NumClrSpcs];
  flt64                m_AvgPicMSE[c_NumClrSpcs];
  //misc
  bool m_AnyFake  = false;
  bool m_Enabled  = false;

public:
  eMetric getMetric () const { return m_Metric ; }
  bool    getEnabled() const { return m_Enabled; }

  void initMetric  (eMetric Metric, int32 NumFrames, int32 NumViews);
  void initSuffixes(bool UseMask);

  void initCmpWeightsAverage(const int32V4& CmpWeightsAverage, eClrSpcMtr ClrSpc);
  void setPerCmpMeric       (const flt64V4& PerCmpMetric, int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc);
  void setPerPicMeric       (const flt64    PerPicMetric, int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc, const flt64 PerPicMSEMetric = std::numeric_limits<flt64>::quiet_NaN());
  void setAnyFake           (bool AnyFake) { m_AnyFake = AnyFake; }
  void addTicks             (uint64 DurationTicks) { m_SumTicks += DurationTicks; m_NumIters++; }

  void        calcAvgMetric        (int32 NumFrames, int32 NumViews);
  std::string formatPerCmpMetric   (int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc);
  std::string formatPerPicMetric   (int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc);
  std::string formatPerPicMSEMetric(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc);
  std::string formatAvgMetric      (const std::string LineHeader, eClrSpcMtr ClrSpc);
  void        calcAvgDuration      (flt64 InvDurationDenominator);
  std::string formatAvgTime        (const std::string LineHeader, tDurationMS PreMetricOps = tDurationMS(0));

protected:
  const flt64V4& getValCmp   (int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc) const { return m_ValCmp   [(int32)ClrSpc][FrameIdx * m_NumViews + ViewIdx]; }
  flt64          getValPic   (int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc) const { return m_ValPic   [(int32)ClrSpc][FrameIdx * m_NumViews + ViewIdx]; }
  flt64          getValPicMSE(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc) const { return m_ValPicMSE[(int32)ClrSpc][FrameIdx * m_NumViews + ViewIdx]; }

  void setValCmp   (int32 FrameIdx, int32 ViewIdx, const flt64V4& ValCmp   , eClrSpcMtr ClrSpc) { m_ValCmp   [(int32)ClrSpc][FrameIdx * m_NumViews + ViewIdx] = ValCmp   ; }
  void setValPic   (int32 FrameIdx, int32 ViewIdx, const flt64    ValPic   , eClrSpcMtr ClrSpc) { m_ValPic   [(int32)ClrSpc][FrameIdx * m_NumViews + ViewIdx] = ValPic   ; }
  void setValPicMSE(int32 FrameIdx, int32 ViewIdx, const flt64    ValPicMSE, eClrSpcMtr ClrSpc) { m_ValPicMSE[(int32)ClrSpc][FrameIdx * m_NumViews + ViewIdx] = ValPicMSE; }
};

//===============================================================================================================================================================================================================

class xMetricStorage
{
protected:
  std::map<std::pair<eMetric, eClrSpcMtr>, xMetricStat> m_MetricStats;

  void         addMetricStat(eMetric Metric, eClrSpcMtr ColorSpace) { m_MetricStats.try_emplace({ Metric, ColorSpace }); }
  xMetricStat& accessStat   (eMetric Metric, eClrSpcMtr ColorSpace) { return m_MetricStats.at({ Metric, ColorSpace }); }
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB