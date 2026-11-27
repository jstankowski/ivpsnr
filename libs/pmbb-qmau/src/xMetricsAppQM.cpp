/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xMetricsAppQM.h"
#include "xString.h"
#include "xKBNS.h"
#include "xPSNR.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

eMetric xStrToMetric(const std::string_view Metric)
{
  std::string MetricU = xString::toUpper(Metric);
  return MetricU ==      "MSE" ? eMetric::     MSE :
         MetricU ==     "PSNR" ? eMetric::    PSNR :
         MetricU ==   "WSPSNR" ? eMetric::  WSPSNR :
         MetricU ==   "IVPSNR" ? eMetric::  IVPSNR :
         MetricU ==     "SSIM" ? eMetric::    SSIM :
         MetricU ==   "MSSSIM" ? eMetric::  MSSSIM :
         MetricU ==   "IVSSIM" ? eMetric::  IVSSIM :
         MetricU == "IVMSSSIM" ? eMetric::IVMSSSIM :
#if X_PMBB_STALLED
         MetricU == "MSIVSSIM" ? eMetric::MSIVSSIM :
#endif //X_PMBB_STALLED
         MetricU ==      "PVD" ? eMetric::     PVD :
#if X_PMBB_EXPERIMENTAL
         MetricU ==    "IVPVD" ? eMetric::   IVPVD :
#endif //X_PMBB_EXPERIMENTAL
                                 eMetric::UNDEFINED;
}
std::string xMetricToStr(eMetric Metric)
{
  switch(Metric)
  {
  case eMetric::MSE     : return       "MSE"; break;
  case eMetric::PSNR    : return      "PSNR"; break;
  case eMetric::WSPSNR  : return    "WSPSNR"; break;
  case eMetric::IVPSNR  : return    "IVPSNR"; break;
  case eMetric::SSIM    : return      "SSIM"; break;
  case eMetric::MSSSIM  : return    "MSSSIM"; break;
  case eMetric::IVSSIM  : return    "IVSSIM"; break;
  case eMetric::IVMSSSIM: return  "IVMSSSIM"; break;
#if X_PMBB_STALLED
  case eMetric::MSIVSSIM: return  "MSIVSSIM"; break;
#endif //X_PMBB_STALLED
  case eMetric::PVD     : return       "PVD"; break;
#if X_PMBB_EXPERIMENTAL
  case eMetric::IVPVD   : return     "IVPVD"; break;
#endif //X_PMBB_EXPERIMENTAL
  default               : return "UNDEFINED"; break;
  }
}

//===============================================================================================================================================================================================================

eClrSpcMtr xClrSpcAppToClrSpcMtr(eClrSpcApp ClrSpcApp)
{
  switch(ClrSpcApp)
  {
  case eClrSpcApp::INVALID: 
    return eClrSpcMtr::UNDEFINED; break;
  case eClrSpcApp::RGB: 
  case eClrSpcApp::BGR: 
  case eClrSpcApp::GBR: 
    return eClrSpcMtr::RGB; break;
  case eClrSpcApp::YCbCr          : 
  case eClrSpcApp::YCbCr_BT601    : 
  case eClrSpcApp::YCbCr_SMPTE170M: 
  case eClrSpcApp::YCbCr_BT709    : 
  case eClrSpcApp::YCbCr_SMPTE240M: 
  case eClrSpcApp::YCbCr_BT2020   : 
    return eClrSpcMtr::YCbCr; break;
  default:
    return eClrSpcMtr::UNDEFINED; break;
  }
}

//===============================================================================================================================================================================================================

void xMetricStat::initMetric(eMetric Metric, int32 NumFrames, int32 NumViews)
{
  m_Metric    = Metric;
  m_NumFrames = NumFrames;
  m_NumViews  = NumViews ;
  for(int32 n = 0; n < c_NumClrSpcs; n++) { m_SuffixCmp[n].clear(); m_SuffixPic[n].clear(); m_SuffixPicMSE[n].clear(); }
  //time stats
  m_SumTicks  = 0;
  m_NumIters  = 0;
  m_AvgDuration = tDurationMS(0);
  //metric value
  constexpr flt64 InitValue = std::numeric_limits<flt64>::quiet_NaN();
  int64 NumEntries = NumFrames * NumViews;
  bool IsPerCmp  = xMetricInfo::Metrics[(int32)m_Metric].IsPerCmp;
  bool AvgInMSE = xMetricInfo::Metrics[(int32)m_Metric].AvgInMSE;
  for(int32 n = 0; n < c_NumClrSpcs; n++)
  {
    if(IsPerCmp ) { m_ValCmp[n].resize(NumEntries, xMakeVec4(InitValue)); }
    if(AvgInMSE) { m_ValPicMSE[n].resize(NumEntries, InitValue); }
    m_ValPic[n].resize(NumEntries, InitValue);
    m_AvgPic[n] = InitValue;
    m_AvgCmp[n] = xMakeVec4(InitValue);
    m_AvgPicMSE[n] = InitValue;
  }
  m_AnyFake  = false;
  m_Enabled  = true;
}
void xMetricStat::initSuffixes(bool UseMask)
{
  if(UseMask)
  {
    m_SuffixCmp[(int32)eClrSpcMtr::YCbCr] = "-M Y:Cb:Cr";
    m_SuffixCmp[(int32)eClrSpcMtr::RGB  ] = "-M R:G:B  ";
  }
  else
  {
    m_SuffixCmp[(int32)eClrSpcMtr::YCbCr] = " Y:Cb:Cr  ";
    m_SuffixCmp[(int32)eClrSpcMtr::RGB  ] = " R:G:B    ";
  }

  bool IsPerPic = xMetricInfo::Metrics[(int32)m_Metric].IsPerPic;
  if(IsPerPic)
  {
    if(UseMask)
    {
      m_SuffixPic[(int32)eClrSpcMtr::YCbCr] = "-M        ";
      m_SuffixPic[(int32)eClrSpcMtr::RGB  ] = "-M RGB    ";
    }
    else
    {
      m_SuffixPic[(int32)eClrSpcMtr::YCbCr] = " YCbCr    ";
      m_SuffixPic[(int32)eClrSpcMtr::RGB  ] = " RGB      ";
    }
  }
  else
  {
    if(UseMask)
    {
      m_SuffixPic[(int32)eClrSpcMtr::YCbCr] = "-M-YCbCr  ";
      m_SuffixPic[(int32)eClrSpcMtr::RGB  ] = "-M-RGB    ";
    }
    else
    {
      m_SuffixPic[(int32)eClrSpcMtr::YCbCr] = " YCbCr    ";
      m_SuffixPic[(int32)eClrSpcMtr::RGB  ] = " RGB      ";
    }
  }

  bool AvgInMSE = xMetricInfo::Metrics[(int32)m_Metric].AvgInMSE;
  if(AvgInMSE)
  {
    if(UseMask)
    {
      m_SuffixPicMSE[(int32)eClrSpcMtr::YCbCr] = IsPerPic ? "-M-MSE    " : "-M-YCbCr-MSE";
      m_SuffixPicMSE[(int32)eClrSpcMtr::RGB  ] = IsPerPic ? "-M RGB-MSE" : "-M-RGB-MSE";
    }
    else
    {
      m_SuffixPicMSE[(int32)eClrSpcMtr::YCbCr] = " YCbCr-MSE";
      m_SuffixPicMSE[(int32)eClrSpcMtr::RGB  ] = " RGB-MSE  ";
    }
  }
}
void xMetricStat::initCmpWeightsAverage(const int32V4& CmpWeightsAverage, eClrSpcMtr ClrSpcMtr)
{
  m_CmpWeightsAverage[(int32)ClrSpcMtr] = CmpWeightsAverage;
  const int32 SumCmpWeightAverage = CmpWeightsAverage.getSum();
  m_CmpWeightAverageInvDenom[(int32)ClrSpcMtr] = 1.0 / (flt64)SumCmpWeightAverage;
}
void xMetricStat::setPerCmpMeric(const flt64V4& PerCmpMetric, int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc)
{
  flt64 PerPicMetric = (PerCmpMetric[0] * m_CmpWeightsAverage[(int32)ClrSpc][0]
                      + PerCmpMetric[1] * m_CmpWeightsAverage[(int32)ClrSpc][1]
                      + PerCmpMetric[2] * m_CmpWeightsAverage[(int32)ClrSpc][2]) * m_CmpWeightAverageInvDenom[(int32)ClrSpc];

  setValCmp(FrameIdx, ViewIdx, PerCmpMetric, ClrSpc);
  setValPic(FrameIdx, ViewIdx, PerPicMetric, ClrSpc);

  if(xMetricInfo::Metrics[(int32)m_Metric].AvgInMSE)
  {
    flt64 PerPicMSEMetric = xPSNR::CalcAvgPSNRinMSEDomain(PerCmpMetric, m_CmpWeightsAverage[(int32)ClrSpc]);
    setValPicMSE(FrameIdx, ViewIdx, PerPicMSEMetric, ClrSpc);
  }
}
void xMetricStat::setPerPicMeric(const flt64 PerPicMetric, int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc, const flt64 PerPicMSEMetric)
{
  setValPic(FrameIdx, ViewIdx, PerPicMetric, ClrSpc);

  if(xMetricInfo::Metrics[(int32)m_Metric].AvgInMSE)
  {
    setValPicMSE(FrameIdx, ViewIdx, PerPicMSEMetric, ClrSpc);
  }
}
void xMetricStat::calcAvgMetric(int32 NumFrames, int32 NumViews)
{
  int64 NumEntries    = NumFrames * NumViews;
  flt64 InvNumEntries = (flt64)1.0 / (flt64)NumEntries;
  for(int32 n = 0; n < c_NumClrSpcs; n++)
  {
    if(!m_ValPic[n].empty()) { m_AvgPic[n] = xKBNS::Accumulate(m_ValPic[n]) * InvNumEntries; }
    if(!m_ValCmp[n].empty()) { m_AvgCmp[n] = xKBNS::Accumulate(m_ValCmp[n]) * InvNumEntries; }
    if(!m_ValPicMSE[n].empty()) { m_AvgPicMSE[n] = xKBNS::Accumulate(m_ValPicMSE[n]) * InvNumEntries; }
  }
}
std::string xMetricStat::formatPerCmpMetric(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc)
{
  const std::string MetricName   = xMetricToStr(m_Metric);
  const bool        IsNormalized = xMetricInfo::Metrics[(int32)m_Metric].IsNormalized;
  const std::string SingleFormat = IsNormalized ? "{:8.6f} " : "{:8.4f} ";

  const flt64V4& ValCmp = getValCmp(FrameIdx, ViewIdx, ClrSpc);

  std::string Result = fmt::format("{:>8}{} ", MetricName, m_SuffixCmp[(int32)ClrSpc]);
  Result += fmt::format(fmt::runtime(SingleFormat + SingleFormat + SingleFormat), ValCmp[0], ValCmp[1], ValCmp[2]);
  return Result;
}
std::string xMetricStat::formatPerPicMetric(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc)
{
  const std::string MetricName   = xMetricToStr(m_Metric);
  const bool        IsNormalized = xMetricInfo::Metrics[(int32)m_Metric].IsNormalized;
  const std::string SingleFormat = IsNormalized ? "{:8.6f} " : "{:8.4f} ";

  const flt64 ValPic = getValPic(FrameIdx, ViewIdx, ClrSpc);

  std::string Result = fmt::format("{:>8}{} ", MetricName, m_SuffixPic[(int32)ClrSpc]);
  Result += fmt::format(fmt::runtime(SingleFormat + "                  "), ValPic);
  return Result;
}
std::string xMetricStat::formatPerPicMSEMetric(int32 FrameIdx, int32 ViewIdx, eClrSpcMtr ClrSpc)
{
  const std::string MetricName   = xMetricToStr(m_Metric);
  const bool        IsNormalized = xMetricInfo::Metrics[(int32)m_Metric].IsNormalized;
  const std::string SingleFormat = IsNormalized ? "{:8.6f} " : "{:8.4f} ";

  const flt64 ValPicMSE = getValPicMSE(FrameIdx, ViewIdx, ClrSpc);

  std::string Result = fmt::format("{:>8}{} ", MetricName, m_SuffixPicMSE[(int32)ClrSpc]);
  Result += fmt::format(fmt::runtime(SingleFormat + "                  "), ValPicMSE);
  return Result;
}
std::string xMetricStat::formatAvgMetric(const std::string LineHeader, eClrSpcMtr ClrSpc)
{
  const std::string      MetricName   = xMetricToStr(m_Metric);
  const bool             IsPerCmp     = xMetricInfo::Metrics[(int32)m_Metric].IsPerCmp    ;
  const bool             IsNormalized = xMetricInfo::Metrics[(int32)m_Metric].IsNormalized;
  const bool             AvgInMSE     = xMetricInfo::Metrics[(int32)m_Metric].AvgInMSE    ;
  const std::string_view Unit         = xMetricInfo::Metrics[(int32)m_Metric].Unit        ;
  const std::string      NameFormat   = LineHeader.empty() ? "{:<8}{} " : "{:>8}{} ";
  const std::string      SingleFormat = IsNormalized ? "{:10.8f} {}  " : "{:10.6f} {}  ";

  std::string Result;

  if(IsPerCmp)
  {
    Result += LineHeader + fmt::format(fmt::runtime(NameFormat), MetricName, m_SuffixCmp[(int32)ClrSpc]);
    Result += fmt::format(fmt::runtime(SingleFormat + SingleFormat + SingleFormat), m_AvgCmp[(int32)ClrSpc][0], Unit, m_AvgCmp[(int32)ClrSpc][1], Unit, m_AvgCmp[(int32)ClrSpc][2], Unit);
    Result += "\n";
  }
   
  Result += LineHeader + fmt::format(fmt::runtime(NameFormat), MetricName, m_SuffixPic[(int32)ClrSpc]);
  Result += fmt::format(fmt::runtime(SingleFormat + "                  "), m_AvgPic[(int32)ClrSpc], Unit);

  if(AvgInMSE)
  {
    Result += "\n";
    Result += LineHeader + fmt::format(fmt::runtime(NameFormat), MetricName, m_SuffixPicMSE[(int32)ClrSpc]);
    Result += fmt::format(fmt::runtime(SingleFormat + "                  "), m_AvgPicMSE[(int32)ClrSpc], Unit);
  }

  return Result;
}
void xMetricStat::calcAvgDuration(flt64 InvDurationDenominator)
{
  m_AvgDuration = tDurationMS(m_SumTicks * InvDurationDenominator);
}
std::string xMetricStat::formatAvgTime(const std::string LineHeader, tDurationMS PreMetricOps)
{
  const std::string      MetricName = xMetricToStr(m_Metric);
  const std::string      Suffix     = "";

  std::string Result = LineHeader;
  if(LineHeader.empty()) { Result += fmt::format("{:<8}{} ", MetricName, Suffix); }
  else                   { Result += fmt::format("{:>8}{} ", MetricName, Suffix); }    

  Result += fmt::format("{:9.2f} ms", m_AvgDuration.count());

  if(PreMetricOps.count() > 0) { Result += fmt::format("   Total {:7.2f} ms", (m_AvgDuration + PreMetricOps).count()); }

  return Result;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB