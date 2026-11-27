/*
    SPDX-FileCopyrightText: 2025-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xPly.h"
#include "xMemory.h"
#include "xSplatOps.h"
#include "fmt/format.h"
#include "xFileListUtils.h"
#include <iostream>
#include <sstream>
#include <strstream>
#include <unordered_map>
#include "xString.h"

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================

xPly::tRes xPly::readGC(xGaussianCloudP* DstCloud, xStream* Stream, bool HeaderOnly)
{
  if(!Stream->canRead()) { return { eRetv::Error, "Stream does not allow Read" }; }

  m_CameraData.clear();
  m_NumSplats        = NOT_VALID;
  m_NumSphHarmCoeffs = 0;
  m_NumSphHarm       = 0;
  m_PropertyOffset.clear();
  m_AttrIdToOffset.clear();

  std::string Header = xFindHeader(Stream);
  if(Header.empty()) { return { eRetv::Error, "PLY header cannot be found !" }; }

  xInOutResult HeaderRes = xParseHeaderGSC(Header);
  if(!HeaderRes) { return HeaderRes; }

  const int32 SphHarmDegree = xSplatOps::xSphHarmDegreeFromCount(m_NumSphHarm);
  const int32 NumProperties = (int32)m_PropertyOffset.size();

  if(!HeaderOnly) { DstCloud->upsize(m_NumSplats); }
  DstCloud->setNumValidSplats(m_NumSplats  );
  DstCloud->setCameraData    (m_CameraData );
  DstCloud->setSphHarmDegree (SphHarmDegree);
  DstCloud->setSphHarmCount  (m_NumSphHarm );
  DstCloud->setAttCount      ((int32)m_PropertyOffset.size());
  
  if(HeaderOnly) { return eRetv::Success; }

  //read cloud
  std::vector<flt32> TmpBuff(c_NumSplatsInBatch * NumProperties, std::numeric_limits<flt32>::quiet_NaN());

  for(int32 BatchSplatIdx = 0; BatchSplatIdx < m_NumSplats; BatchSplatIdx += c_NumSplatsInBatch)
  {
    int32 NumSplatsInCurrBatch = xMin(c_NumSplatsInBatch, m_NumSplats - BatchSplatIdx);
    bool ReadR = Stream->read(TmpBuff.data(), NumSplatsInCurrBatch * NumProperties * sizeof(flt32));
    if(!ReadR) { return { eRetv::Error, "PLY cloud read error" }; }
    
    for(uint32 AttrIdx = 0; AttrIdx < (uint32)xAttrFLT::COUNT; AttrIdx++)
    {
      const xAttrFLT  AttrId     = (xAttrFLT)AttrIdx;
      const uint32    AttrOffset = m_AttrIdToOffset[AttrId];
      const flt32*    SrcPtr     = TmpBuff.data() + AttrOffset;
      flt32* restrict DstPtr     = DstCloud->getAttrAddr(AttrId);
      for(int32 SrcSplatIdx = 0, DstSplatIdx = BatchSplatIdx; SrcSplatIdx < NumSplatsInCurrBatch; SrcSplatIdx++, DstSplatIdx++)
      {        
        flt32 AttrVal = SrcPtr[SrcSplatIdx * NumProperties];
        DstPtr[DstSplatIdx] = AttrVal;
      }
    }
  }

  return eRetv::Success;
}
xPly::tRes xPly::readGC(xGaussianCloudI* DstCloud, xStream* Stream, bool HeaderOnly)
{
  if(!Stream->canRead()) { return { eRetv::Error, "Stream does not allow Read" }; }

  m_CameraData.clear();
  m_NumSplats        = NOT_VALID;
  m_NumSphHarmCoeffs = 0;
  m_NumSphHarm       = 0;
  m_PropertyOffset.clear();
  m_AttrIdToOffset.clear();

  std::string Header = xFindHeader(Stream);
  if(Header.empty()) { return { eRetv::Error, "PLY header cannot be found !" }; }

  xInOutResult HeaderRes = xParseHeaderGSC(Header);
  if(!HeaderRes) { return HeaderRes; }

  if(!HeaderOnly) { DstCloud->upsize(m_NumSplats); }

  DstCloud->setNumValidSplats(m_NumSplats  );
  DstCloud->setCameraData    (m_CameraData );
  DstCloud->setSphHarmDegree (xSplatOps::xSphHarmDegreeFromCount(m_NumSphHarm));
  DstCloud->setSphHarmCount  (m_NumSphHarm );
  DstCloud->setAttCount      ((int32)m_PropertyOffset.size());
  
  if(HeaderOnly) { return eRetv::Success; }

  //read cloud
  const int32 NumProperties = (int32)m_PropertyOffset.size();
  std::vector<flt32> TmpBuff(c_NumSplatsInBatch * NumProperties, std::numeric_limits<flt32>::quiet_NaN());

  if(m_MatchesPatternR) //fast path for reference cloud pattern
  {
    for(int32 BatchSplatIdx = 0; BatchSplatIdx < m_NumSplats; BatchSplatIdx += c_NumSplatsInBatch)
    {
      int32 NumSplatsInCurrBatch = xMin(c_NumSplatsInBatch, m_NumSplats - BatchSplatIdx);
      bool ReadR = Stream->read(TmpBuff.data(), NumSplatsInCurrBatch * NumProperties * sizeof(flt32));
      if(!ReadR) { return { eRetv::Error, "PLY cloud read error" }; }

      for(int32 SrcSplatIdx = 0, DstSplatIdx = BatchSplatIdx; SrcSplatIdx < NumSplatsInCurrBatch; SrcSplatIdx++, DstSplatIdx++)
      {
        xGaussianObject& GO = DstCloud->getGaussianObjects()[DstSplatIdx];
        const flt32* SrcPtr = TmpBuff.data() + SrcSplatIdx * NumProperties;
        GO.PositionXYX[0] = SrcPtr[ 0];
        GO.PositionXYX[1] = SrcPtr[ 1];
        GO.PositionXYX[2] = SrcPtr[ 2];
        GO.Opacity        = SrcPtr[54];
        GO.ScaleXYZ   [0] = SrcPtr[55];
        GO.ScaleXYZ   [1] = SrcPtr[56];
        GO.ScaleXYZ   [2] = SrcPtr[57];
        GO.RotWXYZ    [0] = SrcPtr[58];
        GO.RotWXYZ    [1] = SrcPtr[59];
        GO.RotWXYZ    [2] = SrcPtr[60];
        GO.RotWXYZ    [3] = SrcPtr[61];
        GO.DC_RGB     [0] = SrcPtr[ 6];
        GO.DC_RGB     [1] = SrcPtr[ 7];
        GO.DC_RGB     [2] = SrcPtr[ 8];
        memcpy(GO.Rest_RGB, SrcPtr + m_AttrIdToOffset[xAttrFLT::HR1], m_NumSphHarmCoeffs * sizeof(flt32));
        if(m_NumSphHarmCoeffs < c_MaxNumSphHarm) { memset(GO.Rest_RGB + m_NumSphHarmCoeffs, 0, (c_MaxNumSphHarm - m_NumSphHarmCoeffs) * sizeof(flt32)); }
      }
    }
  }
  else if(m_MatchesPatternD) //even faster path for decoded cloud pattern
  {
    bool ReadR = Stream->read(DstCloud->getGaussianObjects(), m_NumSplats * sizeof(xGaussianObject));
    if(!ReadR) { return { eRetv::Error, "PLY cloud read error" }; }
  }
  else //fallback path for arbitrary atribute pattern
  {
    for(int32 BatchSplatIdx = 0; BatchSplatIdx < m_NumSplats; BatchSplatIdx += c_NumSplatsInBatch)
    {
      int32 NumSplatsInCurrBatch = xMin(c_NumSplatsInBatch, m_NumSplats - BatchSplatIdx);
      bool ReadR = Stream->read(TmpBuff.data(), NumSplatsInCurrBatch * NumProperties * sizeof(flt32));
      if(!ReadR) { return { eRetv::Error, "PLY cloud read error" }; }

      for(int32 SrcSplatIdx = 0, DstSplatIdx = BatchSplatIdx; SrcSplatIdx < NumSplatsInCurrBatch; SrcSplatIdx++, DstSplatIdx++)
      {
        xGaussianObject& GO = DstCloud->getGaussianObjects()[DstSplatIdx];
        const flt32* SrcPtr = TmpBuff.data() + SrcSplatIdx * NumProperties;
        GO.PositionXYX[0] = SrcPtr[m_AttrIdToOffset[xAttrFLT::PX ]];
        GO.PositionXYX[1] = SrcPtr[m_AttrIdToOffset[xAttrFLT::PY ]];
        GO.PositionXYX[2] = SrcPtr[m_AttrIdToOffset[xAttrFLT::PZ ]];
        GO.Opacity        = SrcPtr[m_AttrIdToOffset[xAttrFLT::OP ]];
        GO.ScaleXYZ   [0] = SrcPtr[m_AttrIdToOffset[xAttrFLT::SX ]];
        GO.ScaleXYZ   [1] = SrcPtr[m_AttrIdToOffset[xAttrFLT::SY ]];
        GO.ScaleXYZ   [2] = SrcPtr[m_AttrIdToOffset[xAttrFLT::SZ ]];
        GO.RotWXYZ    [0] = SrcPtr[m_AttrIdToOffset[xAttrFLT::RW ]];
        GO.RotWXYZ    [1] = SrcPtr[m_AttrIdToOffset[xAttrFLT::RX ]];
        GO.RotWXYZ    [2] = SrcPtr[m_AttrIdToOffset[xAttrFLT::RY ]];
        GO.RotWXYZ    [3] = SrcPtr[m_AttrIdToOffset[xAttrFLT::RZ ]];
        GO.DC_RGB     [0] = SrcPtr[m_AttrIdToOffset[xAttrFLT::HR0]];
        GO.DC_RGB     [1] = SrcPtr[m_AttrIdToOffset[xAttrFLT::HG0]];
        GO.DC_RGB     [2] = SrcPtr[m_AttrIdToOffset[xAttrFLT::HB0]];
        memcpy(GO.Rest_RGB, SrcPtr + m_AttrIdToOffset[xAttrFLT::HR1], m_NumSphHarmCoeffs * sizeof(flt32));
        if(m_NumSphHarmCoeffs < c_MaxNumSphHarm) { memset(GO.Rest_RGB + m_NumSphHarmCoeffs, 0, (c_MaxNumSphHarm - m_NumSphHarmCoeffs) * sizeof(flt32)); }
      }
    }
  }

  return eRetv::Success;
}
xPly::tRes xPly::writeGC(xStream* Stream, const xGaussianCloudP* SrcCloud)
{
  const int32 NumSplats     = SrcCloud->getNumValidSplats();
  const int32 NumSphHarm    = xSplatOps::xSphHarmCountFromDegree(SrcCloud->getSphHarmDegree());
  const int32 NumProperties = 14 + NumSphHarm * 3;

  std::string Header = xFormatHeaderGSC(NumSplats, NumSphHarm);
  //fmt::print(Header);
  Stream->write(Header);

  //a bit dirty trick, but saves a lot of effort with building offset tables
  xParseHeaderGSC(Header);

  assert(m_NumSplats                    == NumSplats    );
  assert((int32)m_PropertyOffset.size() == NumProperties);
  assert(m_NumSphHarm                   == NumSphHarm   );

  std::vector<flt32> TmpBuff(c_NumSplatsInBatch * NumProperties, std::numeric_limits<flt32>::quiet_NaN());

  for(int32 BatchSplatIdx = 0; BatchSplatIdx < m_NumSplats; BatchSplatIdx += c_NumSplatsInBatch)
  {
    int32 NumSplatsInCurrBatch = xMin(c_NumSplatsInBatch, m_NumSplats - BatchSplatIdx);

    for(uint32 AttrIdx = 0; AttrIdx < (uint32)xAttrFLT::COUNT; AttrIdx++)
    {
      const xAttrFLT  AttrId     = (xAttrFLT)AttrIdx;
      const uint32    AttrOffset = m_AttrIdToOffset[AttrId];
      const flt32*    SrcPtr     = SrcCloud->getAttrAddr(AttrId);
      flt32* restrict DstPtr     = TmpBuff.data() + AttrOffset;
      for(int32 DstSplatIdx = 0, SrcSplatIdx = BatchSplatIdx; DstSplatIdx < NumSplatsInCurrBatch; DstSplatIdx++, SrcSplatIdx++)
      {
        flt32 AttrVal = SrcPtr[SrcSplatIdx];
        DstPtr[DstSplatIdx * NumProperties] = AttrVal;
      }
    }

    ////dirty hack for unused nx, ny, and nz
    //for(uint32 FakeOffset = 3; FakeOffset < 6; FakeOffset++)
    //{
    //  flt32* restrict DstPtr = TmpBuff.data() + FakeOffset;
    //  for(int32 DstSplatIdx = 0, SrcSplatIdx = BatchSplatIdx; DstSplatIdx < NumSplatsInCurrBatch; DstSplatIdx++, SrcSplatIdx++)
    //  {
    //    DstPtr[DstSplatIdx * NumProperties] = 0.0f;
    //  }
    //}

    Stream->write(TmpBuff.data(), NumSplatsInCurrBatch * NumProperties * sizeof(flt32));
  }

  return eRetv::Success;
}
xPly::tRes xPly::writePC(xStream* Stream, const xPointCloud* SrcCloud, bool Binary)
{
  const int32 NumPoints = SrcCloud->getNumValidPoints();

  const xPointCloud::tCoordEl* CoordsPtr = SrcCloud->getCoordsPtrRaw();
  const xPointCloud::tColorEl* ColorsPtr = SrcCloud->getColorsPtrRaw();

  std::string Header = xFormatHeaderPC(NumPoints, Binary);
  Stream->write(Header);

  if(Binary)
  {
    const int32 NumProperties = 6;
    std::vector<flt32> TmpBuff(c_NumPointsInBatch * NumProperties, std::numeric_limits<flt32>::quiet_NaN());

    flt32* restrict DstPtr = TmpBuff.data();

    for(int32 BatchPointIdx = 0; BatchPointIdx < NumPoints; BatchPointIdx += c_NumPointsInBatch)
    {
      int32 NumPointsInCurrBatch = xMin(c_NumPointsInBatch, NumPoints - BatchPointIdx);
      for(int32 DstPointIdx = 0, SrcPointIdx = BatchPointIdx; DstPointIdx < NumPointsInCurrBatch; DstPointIdx++, SrcPointIdx++)
      {
        DstPtr[DstPointIdx * NumProperties + 0] = CoordsPtr[(SrcPointIdx << 2) + 0];
        DstPtr[DstPointIdx * NumProperties + 1] = CoordsPtr[(SrcPointIdx << 2) + 1];
        DstPtr[DstPointIdx * NumProperties + 2] = CoordsPtr[(SrcPointIdx << 2) + 2];
        DstPtr[DstPointIdx * NumProperties + 3] = ColorsPtr[(SrcPointIdx << 2) + 0]  * 255;
        DstPtr[DstPointIdx * NumProperties + 4] = ColorsPtr[(SrcPointIdx << 2) + 1]  * 255;
        DstPtr[DstPointIdx * NumProperties + 5] = ColorsPtr[(SrcPointIdx << 2) + 2]  * 255;
      }
      Stream->write(TmpBuff.data(), NumPointsInCurrBatch * NumProperties * sizeof(flt32));
    }
  }
  else
  {
    std::string Buffer; Buffer.reserve(xMax(xMemory::getBestEffortSizePageBase() * 64, xMemory::getRealSizePageHuge()));
    for(int32 i = 0; i < NumPoints; i++)
    {
      fmt::format_to(std::back_inserter(Buffer), "{0:12f} {1:12f} {2:12f} {3:12f} {4:12f} {5:12f}\n",
        CoordsPtr[(i << 2) + 0],
        CoordsPtr[(i << 2) + 1],
        CoordsPtr[(i << 2) + 2],
        ColorsPtr[(i << 2) + 0],
        ColorsPtr[(i << 2) + 1],
        ColorsPtr[(i << 2) + 2]);

      if(Buffer.size() > Buffer.capacity() - xMemory::getBestEffortSizePageBase())
      {
        Stream->write(Buffer);
        Buffer.clear();
      }
    }
  }

  return eRetv::Success;
}
std::string xPly::xFindHeader(xStream* Stream)
{
  static constexpr int64 ReadSize = 4096*16;
  const int64 FileSize = Stream->size();
  std::string TmpBuffer; TmpBuffer.resize(ReadSize);
  int64 NumRead1st = xMin((int64)TmpBuffer.size(), FileSize);
  Stream->read(TmpBuffer.data(), NumRead1st);
  int64 EndPos = TmpBuffer.find("end_header");;
  while(EndPos == NOT_VALID)
  {
    int64 CurrSize = TmpBuffer.size();
    TmpBuffer.resize(CurrSize + ReadSize);
    int64 NumReadNth = xMin((int64)TmpBuffer.size(), FileSize) - CurrSize;
    if(NumReadNth <= 0) { return ""; };
    Stream->read(TmpBuffer.data() + CurrSize, NumReadNth);
    EndPos = TmpBuffer.find("end_header");;
  }
  EndPos = EndPos + 11;
  if(TmpBuffer[EndPos] == '\r' && TmpBuffer[EndPos+1] == '\n') //CRLF file
  {
    EndPos++;
  }
  TmpBuffer.resize(EndPos);
  //fmt::print(TmpBuffer);
  Stream->seekR(EndPos, xStream::eSeek::Beg);
  return TmpBuffer;
}
int32 xPly::xFindPropertyOffset(const std::string& AttributeName)
{
  const auto& Itr = m_PropertyOffset.find(AttributeName);
  if(Itr == m_PropertyOffset.end()) { return NOT_VALID; }
  return Itr->second;
}
bool xPly::xCheckPatternMatch(const std::string_view* Pattern, int32 NumProperties)
{
  bool MatchesPattern = m_PropertyOffset.size() == NumProperties;
  if(MatchesPattern)
  {
    for(int32 i = 0; i < NumProperties; i++)
    {
      std::string PropertyName = (tStr)Pattern[i];
      if(m_PropertyOffset.find((tStr)PropertyName) == m_PropertyOffset.end() || m_PropertyOffset[PropertyName] != i)
      {
        MatchesPattern = false; break;
      }
    }
  }
  return MatchesPattern;
}
xSeqFile::tResult xPly::xParseHeaderGSC(const std::string& Header)
{
  std::stringstream HeaderStream;
  HeaderStream.str(Header);

  std::string Line;

  //1st line have to contain "ply"
  std::getline(HeaderStream, Line);
  if(Line != "ply") { return { eRetv::Error, "not a .ply file" }; }

  //try next line
  std::istream &Result = std::getline(HeaderStream, Line);
  if(!Result) { return { eRetv::Error, "missing header data" }; }

  //try parse comments
  while(Line.find("comment") == 0)
  {
    if(Line.find("camera_position_header") != std::string::npos) //camera position header
    {
      //find the column of every camera parameter by its name
      int32 ParamColumn[c_NumCamParams];
      for(int32 k = 0; k < c_NumCamParams; k++) { ParamColumn[k] = NOT_VALID; }
      int32 NameColumn = NOT_VALID;
      const std::vector<std::string> HeadLineSplit = xString::split(Line, ' ');
      for(int32 i = 2; i < (int32)HeadLineSplit.size(); i++)
      {
        const std::string ColumnName = xString::toLower(HeadLineSplit[i]);
        if(ColumnName == "name") { NameColumn = i; }
        for(int32 k = 0; k < c_NumCamParams; k++) { if(ColumnName == c_CamParamNames[k]) { ParamColumn[k] = i; } }
      }

      std::getline(HeaderStream, Line);
      while(Line.find("camera_position") != std::string::npos)
      {
        const std::vector<std::string> LineSplit = xString::split(Line, ' ');
        const int32 NumColumns = (int32)LineSplit.size();

        flt64 Value[c_NumCamParams];
        for(int32 k = 0; k < c_NumCamParams; k++)
        {
          if(ParamColumn[k] == NOT_VALID || ParamColumn[k] >= NumColumns) { return { eRetv::Error, fmt::format("missing camera_position column {}", c_CamParamNames[k]) }; }
          Value[k] = xString::StrToXXX<flt64>(LineSplit[ParamColumn[k]], -1.0);
        }

        if(NameColumn == NOT_VALID || NameColumn >= NumColumns) { return { eRetv::Error, "missing camera_position column name" }; }
        const tStr Name = LineSplit[NameColumn];

        const int32V2 ImageSize = { (int32)Value[(int32)eCamParam::Width], (int32)Value[(int32)eCamParam::Height] };

        m_CameraData.emplace_back((int32)  Value[(int32)eCamParam::ImgIdx], (int32)Value[(int32)eCamParam::CamIdx],
                                  flt64V3{ Value[(int32)eCamParam::PosX   ], Value[(int32)eCamParam::PosY   ], Value[(int32)eCamParam::PosZ] },
                                  flt64V4{ Value[(int32)eCamParam::QuatW  ], Value[(int32)eCamParam::QuatX  ], Value[(int32)eCamParam::QuatY], Value[(int32)eCamParam::QuatZ] },
                                  flt64V2{ Value[(int32)eCamParam::FocalX ], Value[(int32)eCamParam::FocalY ] },
                                  flt64V2{ Value[(int32)eCamParam::CenterX], Value[(int32)eCamParam::CenterY] }, ImageSize, Name);

        std::getline(HeaderStream, Line);
      }
    }
    else
    {
      return { eRetv::Error, "unknown comment" };
    }
    if(Line.find("comment") == 0) { std::getline(HeaderStream, Line); }
  }

  if(Line != "format binary_little_endian 1.0") { return { eRetv::Error, "only binary_little_endian 1.0 format is accepted" }; }

  std::getline(HeaderStream, Line);
  if(Line.find("element vertex ") != 0) { return { eRetv::Error, "missing vertex count" }; }

  m_NumSplats = std::stoi(Line.substr(std::strlen("element vertex ")));

  if(m_NumSplats <= 0) { return { eRetv::Error, fmt::format("invalid vertex count: {}", m_NumSplats) }; }

  // Parse attribute names
  m_PropertyOffset.clear();
  for(int32 i = 0; ; i++)
  {
    std::getline(HeaderStream, Line);
    if(Line == "end_header") { break; }
    if(Line.substr(0, c_PropertyPatternLen) != c_PropertyPattern) { return { eRetv::Error, fmt::format("unsupported property: [{}]", Line) }; }
    std::string AttrStr = Line.substr(c_PropertyPatternLen);
    m_PropertyOffset[AttrStr] = i;
  }

  m_AttrIdToOffset.clear();
  m_AttrIdToOffset[xAttrFLT::PX ] = xFindPropertyOffset("x"      );
  m_AttrIdToOffset[xAttrFLT::PY ] = xFindPropertyOffset("y"      );
  m_AttrIdToOffset[xAttrFLT::PZ ] = xFindPropertyOffset("z"      );
  m_AttrIdToOffset[xAttrFLT::SX ] = xFindPropertyOffset("scale_0");
  m_AttrIdToOffset[xAttrFLT::SY ] = xFindPropertyOffset("scale_1");
  m_AttrIdToOffset[xAttrFLT::SZ ] = xFindPropertyOffset("scale_2");
  m_AttrIdToOffset[xAttrFLT::RX ] = xFindPropertyOffset("rot_1"  );
  m_AttrIdToOffset[xAttrFLT::RY ] = xFindPropertyOffset("rot_2"  );
  m_AttrIdToOffset[xAttrFLT::RZ ] = xFindPropertyOffset("rot_3"  );
  m_AttrIdToOffset[xAttrFLT::RW ] = xFindPropertyOffset("rot_0"  );
  m_AttrIdToOffset[xAttrFLT::OP ] = xFindPropertyOffset("opacity");
  m_AttrIdToOffset[xAttrFLT::HR0] = xFindPropertyOffset("f_dc_0" );
  m_AttrIdToOffset[xAttrFLT::HG0] = xFindPropertyOffset("f_dc_1" );
  m_AttrIdToOffset[xAttrFLT::HB0] = xFindPropertyOffset("f_dc_2" );

  m_NumSphHarmCoeffs = c_MaxNumSphHarm;
  for(int32 i = 0; i < c_MaxNumSphHarm; i++)
  {
    std::string PropertyName = fmt::format("f_rest_{}", i);
    int32       AttrOffset   = xFindPropertyOffset(PropertyName);
    if(AttrOffset < 0) { m_NumSphHarmCoeffs = i - 1; break; }
  }
  m_NumSphHarm = m_NumSphHarmCoeffs / 3;

  for(int32 i = 0; i < m_NumSphHarm; i++)
  {
    xAttrFLT          AttrIdR = xAttrFLT(int32(xAttrFLT::HR1) + 3 * i);
    xAttrFLT          AttrIdG = xAttrFLT(int32(xAttrFLT::HG1) + 3 * i);
    xAttrFLT          AttrIdB = xAttrFLT(int32(xAttrFLT::HB1) + 3 * i);
    int32 PropertySuffixNumR  = c_SphHarmOrderPCO ? i                    : i * 3 + 0;
    int32 PropertySuffixNumG  = c_SphHarmOrderPCO ? i +     m_NumSphHarm : i * 3 + 1;
    int32 PropertySuffixNumB  = c_SphHarmOrderPCO ? i + 2 * m_NumSphHarm : i * 3 + 2;
    std::string PropertyNameR = fmt::format("f_rest_{}", PropertySuffixNumR);
    std::string PropertyNameG = fmt::format("f_rest_{}", PropertySuffixNumG);
    std::string PropertyNameB = fmt::format("f_rest_{}", PropertySuffixNumB);
    const int32 AttrOffsetR   = xFindPropertyOffset(PropertyNameR);
    const int32 AttrOffsetG   = xFindPropertyOffset(PropertyNameG);
    const int32 AttrOffsetB   = xFindPropertyOffset(PropertyNameB);
    m_AttrIdToOffset[AttrIdR] = AttrOffsetR;
    m_AttrIdToOffset[AttrIdG] = AttrOffsetG;
    m_AttrIdToOffset[AttrIdB] = AttrOffsetB;
  }

  assert(m_AttrIdToOffset.size() == (size_t)xAttrFLT::COUNT);

  //check if attribute matches pattern
  m_MatchesPatternR = xCheckPatternMatch(c_PropertyListPatternR.data(), c_NumPropertiesPatternR);
  m_MatchesPatternD = xCheckPatternMatch(c_PropertyListPatternD.data(), c_NumPropertiesPatternD);

  return eRetv::Success;
}
std::string xPly::xFormatHeaderGSC(int32 NumSplats, int32 NumSphericalHarmonics)
{
  std::string Header; Header.reserve(xMemory::getBestEffortSizePageBase());

  Header += "ply\n";
  Header += "format binary_little_endian 1.0\n";
  //Header += "comment Generated by PMBB\n";
  Header += "element vertex " + std::to_string(NumSplats) + "\n";
  Header += "property float x\n";
  Header += "property float y\n";
  Header += "property float z\n";
  //Header += "property float nx\n";
  //Header += "property float ny\n";
  //Header += "property float nz\n";
  Header += "property float f_dc_0\n";
  Header += "property float f_dc_1\n";
  Header += "property float f_dc_2\n";
  for(int32 i = 0; i < NumSphericalHarmonics * 3; i++) { Header += fmt::format("property float f_rest_{}\n", i); }
  Header += "property float opacity\n";
  Header += "property float scale_0\n";
  Header += "property float scale_1\n";
  Header += "property float scale_2\n";
  Header += "property float rot_0\n";
  Header += "property float rot_1\n";
  Header += "property float rot_2\n";
  Header += "property float rot_3\n";
  Header += "end_header\n";
  
  return Header;
}
std::string xPly::xFormatHeaderPC(int32 NumUnits, bool Binary)
{
  std::string Header; Header.reserve(xMemory::getBestEffortSizePageBase());
  Header += "ply\n";
  if(Binary) { Header += "format binary_little_endian 1.0\n"; }
  else       { Header += "format ascii 1.0\n"; }
  Header += "comment Generated by PMBB\n";
  Header += "element vertex " + std::to_string(NumUnits) + "\n";
  Header += "property float x\n";
  Header += "property float y\n";
  Header += "property float z\n";
  Header += "property float red\n";
  Header += "property float green\n";
  Header += "property float blue\n";
  Header += "end_header\n";
  return Header;
}

//===============================================================================================================================================================================================================

void xSeqPlyList::create(bool CountFramesFromZero, int32 FirstFrameHint, int32 MaxNumFiles)
{ 
  m_1stFrameHint = FirstFrameHint;
  m_MaxNumFiles  = MaxNumFiles;
  m_SingleFile   = false;
  m_CountFrom0   = CountFramesFromZero;
}
xSeqPlyList::tResult xSeqPlyList::readGC(xGaussianCloudP* DstCloud)
{
  if(m_OpMode != eMode::Read) { return { eRetv::Error, "OpMode does not allow Read" }; }
  if(m_OpMode == eMode::Read && m_CurrFrameIdx >= m_NumOfFrames + m_1stFileIdx) { return eRetv::EndOfFile; }
  if(m_SingleFile && m_CurrFrameIdx > 0) { return { eRetv::Error, fmt::format("Attempt to read multiple times from single file File={}", m_FileNamePattern) }; }
  
  const std::string& FrameFileName = xFormatFileName(m_CurrFrameIdx);

  //read cloud
  xStream File(FrameFileName, xStream::eMode::Read);
  if(!File.isValid()) { return eRetv::InexistentFile; }
  tResult Result = m_Ply.readGC(DstCloud, &File, false);
  if(!Result) { return Result; }
  File.closeFile();

  //set POC & update state
  DstCloud->setPOC(m_CurrFrameIdx);
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}
xSeqPlyList::tResult xSeqPlyList::readGC(xGaussianCloudI* DstCloud)
{
  if(m_OpMode != eMode::Read) { return { eRetv::Error, "OpMode does not allow Read" }; }
  if(m_OpMode == eMode::Read && m_CurrFrameIdx >= m_NumOfFrames + m_1stFileIdx) { return eRetv::EndOfFile; }
  if(m_SingleFile && m_CurrFrameIdx > 0) { return { eRetv::Error, fmt::format("Attempt to read multiple times from single file File={}", m_FileNamePattern) }; }
  
  const std::string& FrameFileName = xFormatFileName(m_CurrFrameIdx);

  //read cloud
  xStream File(FrameFileName, xStream::eMode::Read);
  if(!File.isValid()) { return eRetv::InexistentFile; }
  tResult Result = m_Ply.readGC(DstCloud, &File, false);
  if(!Result) { return Result; }
  File.closeFile();

  //set POC & update state
  DstCloud->setPOC(m_CurrFrameIdx);
  m_CurrFrameIdx += 1;

  return eRetv::Success;
}
xSeqPlyList::tResult xSeqPlyList::seekFrame(int32 FrameIdx)
{
  if(FrameIdx < 0 || (m_OpMode == eMode::Read && FrameIdx >= m_NumOfFrames + m_1stFileIdx)) { return eRetv::Error; }
  if(m_OpMode != eMode::Read) { return eRetv::Error; }

  m_CurrFrameIdx = FrameIdx;
  return eRetv::Success;
}
xSeqPlyList::tResult xSeqPlyList::xBackendOpen(tCSR FileNamePattern, eMode OpMode)
{
  m_FileNamePattern = FileNamePattern;

  eFmtSts FormatStatus = analyzeFormatString(FileNamePattern);
  if(FormatStatus == eFmtSts::Malformed) { return { eRetv::WrongArg, fmt::format("Malformed format string FileNamePattern={}", FileNamePattern) }; }

  m_SingleFile = FormatStatus==eFmtSts::NonFmt; //file name pattern does not contain any format field

  switch(OpMode)
  {
  case eMode::Read : return xPlyListOpenRead (); break;
  case eMode::Write: return xPlyListOpenWrite(); break;
  default: return eRetv::NotImplemented; break;
  }
}
xSeqPlyList::tResult xSeqPlyList::xBackendClose()
{
  m_OpMode = eMode::Unknown;

  m_NumOfFrames  = NOT_VALID;
  m_CurrFrameIdx = NOT_VALID;

  return eRetv::Success;
}
xSeqPlyList::tResult xSeqPlyList::xPlyListOpenRead()
{
  if(m_SingleFile) //file does not contain any format field
  {
    m_1stFileIdx   = 0;
    m_CountFrom0   = true;
    m_NumOfFrames  = 1;
    m_CurrFrameIdx = 0;
    return xPlyListFileVerify(m_FileNamePattern);
  }  

  xFileListUtils::tIntRes FirstFrameRes = xFileListUtils::detectFirstFrame(m_FileNamePattern, std::bind(&xSeqPlyList::xPlyListFileVerify, this, std::placeholders::_1), m_1stFrameHint);
  if(!FirstFrameRes.second) { return FirstFrameRes.second; }
  m_1stFileIdx = FirstFrameRes.first;

  xFileListUtils::tIntRes NumFramesRes = xFileListUtils::detectNumFrames(m_FileNamePattern, m_1stFileIdx, m_MaxNumFiles);
  if(!NumFramesRes.second) { return NumFramesRes.second; }
  m_NumOfFrames  = NumFramesRes.first;
  m_CurrFrameIdx = m_CountFrom0 ? 0 : m_1stFileIdx;

  return eRetv::Success;
}
xSeqPlyList::tResult xSeqPlyList::xPlyListOpenWrite()
{
  m_1stFileIdx   = 0;
  m_NumOfFrames  = 0;
  m_CurrFrameIdx = 0;

  return eRetv::Success;
}
xSeqPlyList::tResult xSeqPlyList::xPlyListFileVerify(tCSR FileName)
{
  //Open file
  xStream File(FileName, xStream::eMode::Read);
  if(!File.isValid()) { return eRetv::InexistentFile; }

  xGaussianCloudP Cloud;

  tResult PlyHeaderReadResult = m_Ply.readGC(&Cloud, &File, true);

  File.closeFile();

  return PlyHeaderReadResult;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB