/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "xCommonDefPMBB-CORE.h"
#include "xVec.h"

//portable implementation
#include "xPixelOpsSTD.h"

//SSE implementation
#if X_SIMD_CAN_USE_SSE && __has_include("xPixelOpsSSE.h")
#define X_CAN_USE_SSE 1
#include "xPixelOpsSSE.h"
#else
#define X_CAN_USE_SSE 0
#endif

//AVX implementation
#if X_SIMD_CAN_USE_AVX && __has_include("xPixelOpsAVX.h")
#define X_CAN_USE_AVX 1
#include "xPixelOpsAVX.h"
#else
#define X_CAN_USE_AVX 0
#endif

//AVX512 implementation
#if X_SIMD_CAN_USE_AVX512 && __has_include("xDistortionAVX512.h")
#define X_CAN_USE_AVX512 1
#include "xPixelOpsAVX512.h"
#else
#define X_CAN_USE_AVX512 0
#endif

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xPixelOps
{
public:
  //===============================================================================================================================================================================================================
  // Copy & fill
  //===============================================================================================================================================================================================================
  template <typename PelType> static inline void Copy(PelType* restrict Dst, const PelType* Src, int32 Area);
  template <typename PelType> static inline void Copy(PelType* restrict Dst, const PelType* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height);
  template <typename PelType> static inline void CopyPart(PelType* restrict Dst, const PelType* Src, int32 DstStride, int32 SrcStride, int32V2 DstCoord, int32V2 SrcCoord, int32V2 Size);

  template <typename PelType> static inline void Fill(PelType* restrict Dst, const PelType Value, int32 Area);

public:  
  static inline bool  FindBroken   (const uint16* Src, int32 Stride, int32 Width, int32 Height, int32 BitDepth) { return xPixelOpsSTD::FindBroken   (Src, Stride, Width, Height, BitDepth); }
  static inline void  ConcealBroken(uint16*       Ptr, int32 Stride, int32 Width, int32 Height, int32 BitDepth) { return xPixelOpsSTD::ConcealBroken(Ptr, Stride, Width, Height, BitDepth); }

  static inline void  ExtendMargin (uint16* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin) { xPixelOpsSTD::ExtendMargin(Addr, Stride, Width, Height, Margin); }

#if   X_CAN_USE_AVX512
  
  static inline void  Cvt            (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsAVX512::Cvt          (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void  Cvt            (uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsAVX512::Cvt          (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void  UpsampleHV     (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX512::UpsampleHV   (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  DownsampleHV   (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::DownsampleHV(Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtUpsampleHV  (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX512::CvtUpsampleHV(Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtDownsampleHV(uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::CvtDownsampleHV(Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  UpsampleH      (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::UpsampleH      (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtUpsampleH   (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::CvtUpsampleH   (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  DownsampleH    (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::DownsampleH    (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtDownsampleH (uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::CvtDownsampleH (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }  
  static inline bool  CheckValues    (const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth) { return xPixelOpsAVX512::CheckValues(Src, SrcStride, Width, Height, BitDepth); }
  static inline void  AOS4fromSOA3   (uint16* DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height) { xPixelOpsAVX::AOS4fromSOA3(DstABCD, SrcA, SrcB, SrcC, ValueD, DstStride, SrcStride, Width, Height); }
  static inline void  SOA3fromAOS4   (uint16* DstA, uint16* DstB, uint16* DstC, const uint16* SrcABCD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height) { xPixelOpsAVX::SOA3fromAOS4(DstA, DstB, DstC, SrcABCD, DstStride, SrcStride, Width, Height); }
  static inline int32 CountNonZero   (const uint16* Src, int32 SrcStride, int32 Width, int32 Height) { return xPixelOpsAVX512::CountNonZero(Src, SrcStride, Width, Height); }

#elif X_CAN_USE_AVX
  
  static inline void  Cvt            (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsAVX::Cvt            (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void  Cvt            (uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsAVX::Cvt            (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void  UpsampleHV     (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::UpsampleHV     (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  DownsampleHV   (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::DownsampleHV   (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtUpsampleHV  (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::CvtUpsampleHV  (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtDownsampleHV(uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::CvtDownsampleHV(Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  UpsampleH      (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::UpsampleH      (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtUpsampleH   (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::CvtUpsampleH   (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  DownsampleH    (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::DownsampleH    (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtDownsampleH (uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsAVX::CvtDownsampleH (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }  
  static inline bool  CheckValues  (const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth) { return xPixelOpsAVX::CheckValues(Src, SrcStride, Width, Height, BitDepth); }
  static inline void  AOS4fromSOA3 (uint16* DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height) { xPixelOpsAVX::AOS4fromSOA3(DstABCD, SrcA, SrcB, SrcC, ValueD, DstStride, SrcStride, Width, Height); }
  static inline void  SOA3fromAOS4 (uint16* DstA, uint16* DstB, uint16* DstC, const uint16* SrcABCD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height) { xPixelOpsAVX::SOA3fromAOS4(DstA, DstB, DstC, SrcABCD, DstStride, SrcStride, Width, Height); }
  static inline int32 CountNonZero (const uint16* Src, int32 SrcStride, int32 Width, int32 Height) { return xPixelOpsAVX::CountNonZero(Src, SrcStride, Width, Height); }

#elif X_CAN_USE_SSE

  static inline void  Cvt            (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsSSE::Cvt            (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void  Cvt            (uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsSSE::Cvt            (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void  UpsampleHV     (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSSE::UpsampleHV     (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  DownsampleHV   (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSSE::DownsampleHV   (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtUpsampleHV  (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSSE::CvtUpsampleHV  (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtDownsampleHV(uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSSE::CvtDownsampleHV(Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  UpsampleH      (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSSE::UpsampleH      (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtUpsampleH   (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSSE::CvtUpsampleH   (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  DownsampleH    (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSSE::DownsampleH    (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtDownsampleH (uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSSE::CvtDownsampleH (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }  
  static inline bool  CheckValues  (const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth) { return xPixelOpsSSE::CheckValues(Src, SrcStride, Width, Height, BitDepth); }
  static inline void  AOS4fromSOA3 (uint16* DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height) { xPixelOpsSSE::AOS4fromSOA3(DstABCD, SrcA, SrcB, SrcC, ValueD, DstStride, SrcStride, Width, Height); }
  static inline void  SOA3fromAOS4 (uint16* DstA, uint16* DstB, uint16* DstC, const uint16* SrcABCD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height) { xPixelOpsSSE::SOA3fromAOS4(DstA, DstB, DstC, SrcABCD, DstStride, SrcStride, Width, Height); }
  static inline int32 CountNonZero (const uint16* Src, int32 SrcStride, int32 Width, int32 Height) { return xPixelOpsSSE::CountNonZero(Src, SrcStride, Width, Height); }

#else //X_CAN_USE_???

  static inline void  Cvt            (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsSTD::Cvt            (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void  Cvt            (uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsSTD::Cvt            (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void  UpsampleHV     (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::UpsampleHV     (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  DownsampleHV   (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::DownsampleHV   (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtUpsampleHV  (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::CvtUpsampleHV  (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtDownsampleHV(uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::CvtDownsampleHV(Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  UpsampleH      (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::UpsampleH      (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtUpsampleH   (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::CvtUpsampleH   (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  DownsampleH    (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::DownsampleH    (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void  CvtDownsampleH (uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::CvtDownsampleH (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }  
  static inline bool  CheckValues  (const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth) { return xPixelOpsSTD::CheckValues(Src, SrcStride, Width, Height, BitDepth); }
  static inline void  AOS4fromSOA3 (uint16* DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height) { xPixelOpsSTD::AOS4fromSOA3(DstABCD, SrcA, SrcB, SrcC, ValueD, DstStride, SrcStride, Width, Height); }
  static inline void  SOA3fromAOS4 (uint16* DstA, uint16* DstB, uint16* DstC, const uint16* SrcABCD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height) { xPixelOpsSTD::SOA3fromAOS4(DstA, DstB, DstC, SrcABCD, DstStride, SrcStride, Width, Height); }
  static inline int32 CountNonZero (const uint16* Src, int32 SrcStride, int32 Width, int32 Height) { return xPixelOpsSTD::CountNonZero(Src, SrcStride, Width, Height); }

#endif //X_CAN_USE_???
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Copy & fill
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <typename PelType> void xPixelOps::Copy(PelType* restrict Dst, const PelType* Src, int32 Area)
{
  ::memcpy(Dst, Src, Area * sizeof(PelType));
}
template <typename PelType> void xPixelOps::Copy(PelType* restrict Dst, const PelType* Src, int32 DstStride, int32 SrcStride, int32 Width, int32 Height)
{
  for(int32 y = 0; y < Height; y++)
  {
    ::memcpy(Dst, Src, Width * sizeof(PelType));
    Src += SrcStride; Dst += DstStride;
  }
}
template <typename PelType> void xPixelOps::CopyPart(PelType* restrict Dst, const PelType* Src, int32 DstStride, int32 SrcStride, int32V2 DstCoord, int32V2 SrcCoord, int32V2 Size)
{
  for(int32 y = 0; y < Size.getY(); y++)
  {
    const PelType* const SrcLine = Src + (SrcCoord.getY() + y) * SrcStride + SrcCoord.getX();
          PelType* const DstLine = Dst + (DstCoord.getY() + y) * DstStride + DstCoord.getX();
    ::memcpy(DstLine, SrcLine, Size.getX() * sizeof(PelType));
  }
}
template <typename PelType> inline void xPixelOps::Fill(PelType* restrict Dst, const PelType Value, int32 Area)
{
  if constexpr(std::is_integral_v<PelType> && sizeof(PelType) == 1)
  {
    memset(Dst, Value, Area);
  }
  else
  {
    if(Value == (PelType)0) { memset           (Dst, 0    , Area*sizeof(PelType)); }
    else                    { xMemsetX<PelType>(Dst, Value, Area                ); }
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB

#undef X_CAN_USE_SSE
#undef X_CAN_USE_AVX
#undef X_CAN_USE_AVX512