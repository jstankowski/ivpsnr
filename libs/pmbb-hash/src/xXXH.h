/*
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefHASH.h"
#include <array>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xXXH128_Cmn
{
public:
  using tHash = std::array<uint8, 16>;

protected:
  void* m_State = nullptr;

public:
  static std::string format(const tHash& Hash);
};

//===============================================================================================================================================================================================================

class xXXH128_STD : public xXXH128_Cmn
{
public:
  xXXH128_STD ();
  ~xXXH128_STD();
  void  init  ();
  void  update(const uint8* Data, uint64 Length);
  tHash finish();

  static tHash calc(const uint8* Data, uint64 Length);
};

//=====================================================================================================================================================================================

#if X_SIMD_CAN_USE_SSE
#define X_CAN_USE_SSE 1
class xXXH128_SSE : public xXXH128_Cmn
{
public:
  xXXH128_SSE ();
  ~xXXH128_SSE();
  void  init  ();
  void  update(const uint8* Data, uint64 Length);
  tHash finish();

  static tHash calc(const uint8* Data, uint64 Length);
};
#else //X_SIMD_CAN_USE_SSE
#define X_CAN_USE_SSE 0
#endif //X_SIMD_CAN_USE_SSE

//===============================================================================================================================================================================================================

#if X_SIMD_CAN_USE_AVX
#define X_CAN_USE_AVX 1
class xXXH128_AVX : public xXXH128_Cmn
{
public:
  xXXH128_AVX ();
  ~xXXH128_AVX();
  void  init  ();
  void  update(const uint8* Data, uint64 Length);
  tHash finish();

  static tHash calc(const uint8* Data, uint64 Length);
};
#else //X_SIMD_CAN_USE_AVX
#define X_CAN_USE_AVX 0
#endif //X_SIMD_CAN_USE_AVX

//=====================================================================================================================================================================================

#if X_SIMD_CAN_USE_AVX512
#define X_CAN_USE_AVX512 1
class xXXH128_AVX512 : public xXXH128_Cmn
{
public:
  xXXH128_AVX512 ();
  ~xXXH128_AVX512();
  void  init  ();
  void  update(const uint8* Data, uint64 Length);
  tHash finish();

  static tHash calc(const uint8* Data, uint64 Length);
};
#else //X_SIMD_CAN_USE_AVX512
#define X_CAN_USE_AVX512 0
#endif //X_SIMD_CAN_USE_AVX512

//=====================================================================================================================================================================================

//#if X_SIMD_CAN_USE_NEON
//#define X_CAN_USE_NEON 1
//class xXXH128_NEON : public xXXH128_Cmn
//{
//public:
//  xXXH128_NEON ();
//  ~xXXH128_NEON();
//  void  init  ();
//  void  update(const uint8* Data, uint64 Length);
//  tHash finish();
//  tHash calc  ();
//};
//#else //X_SIMD_CAN_USE_NEON
//#define X_CAN_USE_NEON 0
//#endif //X_SIMD_CAN_USE_NEON

//=====================================================================================================================================================================================

#if X_CAN_USE_AVX512
using xXXH128 = xXXH128_AVX512;
#elif X_CAN_USE_AVX
using xXXH128 = xXXH128_AVX;
#elif X_CAN_USE_SSE
using xXXH128 = xXXH128_SSE;
#else
using xXXH128 = xXXH128_STD;
#endif

//===============================================================================================================================================================================================================

#undef X_CAN_USE_AVX512
#undef X_CAN_USE_AVX
#undef X_CAN_USE_SSE

//===============================================================================================================================================================================================================

} //end of namespace PMBB