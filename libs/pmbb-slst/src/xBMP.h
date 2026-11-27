/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefSLST.h"
#include "xStream.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

#pragma pack(1)
class xBitmapFileHeader
{
public:
  static const uint32 c_HeaderLength = 14;

protected:
  uint16  m_Type     ;
  uint32  m_FileSize ;
  uint16  m_Reserved1;
  uint16  m_Reserved2;
  uint32  m_Offset   ;

public:
  bool   Read    (xStream* Stream);
  bool   Write   (xStream* Stream);
  bool   Validate() const;

public:
  void   setType     (uint16 Type     ){ m_Type = Type; }
  uint16 getType     (                ){ return m_Type; }
  void   setFileSize (uint32 FileSize ){ m_FileSize = FileSize; }
  uint32 getFileSize (                ){ return m_FileSize; }
  void   setReserved1(uint16 Reserved1){ m_Reserved1 = Reserved1; }
  uint16 getReserved1(                ){ return m_Reserved1; }
  void   setReserved2(uint16 Reserved2){ m_Reserved2 = Reserved2; }
  uint16 getReserved2(                ){ return m_Reserved2; }
  void   setOffset   (uint32 Offset   ){ m_Offset = Offset; }
  uint32 getOffset   (                ){ return m_Offset; }
};
#pragma pack()

//===============================================================================================================================================================================================================

#pragma pack(1)
class xBitmapInfoHeader
{
public:
  static const uint32 c_HeaderLength = 40;

protected:
  uint32 m_HeaderSize     ;
  int32  m_Width          ;
  int32  m_Height         ;
  uint16 m_Planes         ;
  uint16 m_BitsPerPixel   ;
  uint32 m_Compression    ;
  uint32 m_ImageSize      ;
  int32  m_PelsPerMeterX  ;
  int32  m_PelsPerMeterY  ;
  uint32 m_ColorsUsed     ;
  uint32 m_ColorsImportant;

public:
  bool   Read    (xStream* Stream);
  bool   Write   (xStream* Stream);
  bool   Validate() const;

public:
  void   setHeaderSize     (uint32 HeaderSize     ){ m_HeaderSize = HeaderSize; }
  uint32 getHeaderSize     (                      ){ return m_HeaderSize; }
  void   setWidth          (int32  Width          ){ m_Width = Width; }
  int32  getWidth          (                      ){ return m_Width; }
  void   setHeight         (int32  Height         ){ m_Height = Height; }
  int32  getHeight         (                      ){ return m_Height; }
  void   setPlanes         (uint16 Planes         ){ m_Planes = Planes; }
  uint16 getPlanes         (                      ){ return m_Planes; }
  void   setBitsPerPixel   (uint16 BitsPerPixel   ){ m_BitsPerPixel = BitsPerPixel; }
  uint16 getBitsPerPixel   (                      ){ return m_BitsPerPixel; }
  void   setCompression    (uint32 Compression    ){ m_Compression = Compression; }
  uint32 getCompression    (                      ){ return m_Compression; }
  void   setImageSize      (uint32 ImageSize      ){ m_ImageSize = ImageSize; }
  uint32 getImageSize      (                      ){ return m_ImageSize; }
  void   setPelsPerMeterX  (int32  PelsPerMeterX  ){ m_PelsPerMeterX = PelsPerMeterX; }
  int32  getPelsPerMeterX  (                      ){ return m_PelsPerMeterX; }
  void   setPelsPerMeterY  (int32  PelsPerMeterY  ){ m_PelsPerMeterY = PelsPerMeterY; }
  int32  getPelsPerMeterY  (                      ){ return m_PelsPerMeterY; }
  void   setColorsUsed     (uint32 ColorsUsed     ){ m_ColorsUsed = ColorsUsed; }
  uint32 getColorsUsed     (                      ){ return m_ColorsUsed; }
  void   setColorsImportant(uint32 ColorsImportant){ m_ColorsImportant = ColorsImportant; }
  uint32 getColorsImportant(                      ){ return m_ColorsImportant; }
};
#pragma pack()

//===============================================================================================================================================================================================================

} //end of namespace PMBB
