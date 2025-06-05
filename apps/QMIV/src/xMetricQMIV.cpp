/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xMetricQMIV.h"
#include "xString.h"
#include "xKBNS.h"

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
  default               : return "UNDEFINED"; break;
  }
}

//===============================================================================================================================================================================================================

void xMetricStat::initMetric(eMetric Metric, int32 NumFrames)
{
  m_Metric    = Metric;
  m_SuffixCmp = "";
  m_SuffixPic = "";
  m_SumTicks  = 0;
  m_AvgDuration = tDurationMS(0);
  constexpr flt64 InitValue = std::numeric_limits<flt64>::quiet_NaN();
  bool IsPerCmp = xMetricInfo::Metrics[(int32)m_Metric].IsPerCmp;
  if(IsPerCmp) { m_ValCmp.resize(NumFrames, xMakeVec4(InitValue)); }
  m_ValPic.resize(NumFrames, InitValue);    
  m_AvgPic   = InitValue;
  m_AvgCmp   = xMakeVec4(InitValue);
  m_AnyFake  = false;
  m_Enabled  = true;
}
void xMetricStat::initSuffixes(bool UseMask, bool UseRGB)
{
  if     (UseMask && UseRGB) { m_SuffixCmp = "-M R:G:B  "; }
  else if(UseMask          ) { m_SuffixCmp = "-M Y:Cb:Cr"; }
  else if(UseRGB           ) { m_SuffixCmp = " R:G:B    "; }
  else                       { m_SuffixCmp = " Y:Cb:Cr  "; }

  bool IsPerPic = xMetricInfo::Metrics[(int32)m_Metric].IsPerPic;
  if(IsPerPic)
  {
    if     (UseMask && UseRGB) { m_SuffixPic = "-M-RGB    "; }
    else if(UseMask          ) { m_SuffixPic = "-M        "; }
    else if(UseRGB           ) { m_SuffixPic = "-RGB      "; }
    else                       { m_SuffixPic = "          "; }
  }
  else
  {
    if     (UseMask && UseRGB) { m_SuffixPic = "-M-RGB    "; }
    else if(UseMask          ) { m_SuffixPic = "-M-YCbCr  "; }
    else if(UseRGB           ) { m_SuffixPic = "-RGB      "; }
    else                       { m_SuffixPic = "-YCbCr    "; }
  }
}

void xMetricStat::initCmpWeightsAverage(const int32V4& CmpWeightsAverage)
{
  m_CmpWeightsAverage = CmpWeightsAverage;
  const int32 SumCmpWeightAverage = CmpWeightsAverage.getSum();
  m_CmpWeightAverageInvDenom      = 1.0 / (flt64)SumCmpWeightAverage;
}
void xMetricStat::setPerCmpMeric(const flt64V4& PerCmpMetric, int32 FrameIdx)
{ 
  m_ValCmp[FrameIdx] = PerCmpMetric;
  m_ValPic[FrameIdx] = (m_ValCmp[FrameIdx][0] * m_CmpWeightsAverage[0]
                      + m_ValCmp[FrameIdx][1] * m_CmpWeightsAverage[1]
                      + m_ValCmp[FrameIdx][2] * m_CmpWeightsAverage[2]) * m_CmpWeightAverageInvDenom;
}
void xMetricStat::calcAvgMetric(int32 NumFrames)
{
  if(!m_ValPic.empty()) { m_AvgPic = xKBNS::Accumulate(m_ValPic) / (flt64)NumFrames; }
  if(!m_ValCmp.empty()) { m_AvgCmp = xKBNS::Accumulate(m_ValCmp) / (flt64)NumFrames; }
}

std::string xMetricStat::formatPerCmpMetric(int32 FrameIdx)
{
  const std::string MetricName   = xMetricToStr(m_Metric);
  const bool        IsNormalized = xMetricInfo::Metrics[(int32)m_Metric].IsNormalized;
  const std::string SingleFormat = IsNormalized ? "{:8.6f} " : "{:8.4f} ";

  std::string Result = fmt::format("{:>8}{} ", MetricName, m_SuffixCmp);    
  Result += fmt::format(fmt::runtime(SingleFormat + SingleFormat + SingleFormat), m_ValCmp[FrameIdx][0], m_ValCmp[FrameIdx][1], m_ValCmp[FrameIdx][2]);
  return Result;
}

std::string xMetricStat::formatPerPicMetric(int32 FrameIdx)
{
  const std::string MetricName   = xMetricToStr(m_Metric);
  const bool        IsNormalized = xMetricInfo::Metrics[(int32)m_Metric].IsNormalized;
  const std::string SingleFormat = IsNormalized ? "{:8.6f} " : "{:8.4f} ";

  std::string Result = fmt::format("{:>8}{} ", MetricName, m_SuffixPic);    
  Result += fmt::format(fmt::runtime(SingleFormat + "                  "), m_ValPic[FrameIdx]);
  return Result;
}

std::string xMetricStat::formatAvgMetric(const std::string LineHeader)
{
  const std::string      MetricName   = xMetricToStr(m_Metric);
  const bool             IsPerCmp     = xMetricInfo::Metrics[(int32)m_Metric].IsPerCmp    ;
  const bool             IsNormalized = xMetricInfo::Metrics[(int32)m_Metric].IsNormalized;
  const std::string_view Unit         = xMetricInfo::Metrics[(int32)m_Metric].Unit        ;
  const std::string      NameFormat   = LineHeader.empty() ? "{:<8}{} " : "{:>8}{} ";
  const std::string      SingleFormat = IsNormalized ? "{:10.8f} {}  " : "{:10.6f} {}  ";

  std::string Result;

  if(IsPerCmp)
  {
    Result += LineHeader + fmt::format(fmt::runtime(NameFormat), MetricName, m_SuffixCmp);
    Result += fmt::format(fmt::runtime(SingleFormat + SingleFormat + SingleFormat), m_AvgCmp[0], Unit, m_AvgCmp[1], Unit, m_AvgCmp[2], Unit);
    Result += "\n";
  }
   
  Result += LineHeader + fmt::format(fmt::runtime(NameFormat), MetricName, m_SuffixPic);
  Result += fmt::format(fmt::runtime(SingleFormat + "                  "), m_AvgPic, Unit);
 
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