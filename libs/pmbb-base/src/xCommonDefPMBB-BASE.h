/*
    SPDX-FileCopyrightText: 2019-2023 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

//=============================================================================================================================================================================
// MSVC workaround
//=============================================================================================================================================================================
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

//=============================================================================================================================================================================
// base includes
//=============================================================================================================================================================================
#include <type_traits>
#include <algorithm>
#include <string>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <cinttypes>
#include <cfloat>
#include <cassert>

#define FMT_USE_WINDOWS_H 0
#include <fmt/format.h>
#include <fmt/printf.h>
#include <fmt/ostream.h>
#undef FMT_USE_WINDOWS_H

//=============================================================================================================================================================================
// Hard coded constrains
//=============================================================================================================================================================================
#define NOT_VALID  -1

//=============================================================================================================================================================================
// System section
//=============================================================================================================================================================================
#if (defined(WIN32) || defined(_WIN32) || defined(_WIN64))
  #define X_SYSTEM_WINDOWS 1
#elif defined(__linux__)
  #define X_SYSTEM_LINUX 1
#else
  #error "Unknown system"
#endif

//=============================================================================================================================================================================
// Compiler section
//=============================================================================================================================================================================
#if defined __INTEL_COMPILER
  #define X_COMPILER_ICC 1
  #define X_COMPILER_VER  __INTEL_COMPILER
  #define X_COMPILER_NAME "Intel Compiler"
#elif defined __clang__
  #define X_COMPILER_CLANG 1
  #define X_COMPILER_VER  __clang_major__
  #define X_COMPILER_NAME "Clang"
#elif defined __GNUC__
  #define X_COMPILER_GCC 1
  #define X_COMPILER_VER  __GNUC__
  #define X_COMPILER_NAME "GNU Compiler Collection"
#elif defined _MSC_VER
  #define X_COMPILER_MSVC 1
  #define X_COMPILER_VER  _MSC_VER
  #define X_COMPILER_NAME "MS Visual Studio"
#else
  #define X_COMPILER_VER  
  #define X_COMPILER_NAME "unknown"
#endif

//=============================================================================================================================================================================
// C99 restrict pointers support
//=============================================================================================================================================================================
#if defined(_MSC_VER) && _MSC_VER >= 1400 //MSVC
#define restrict __restrict
#elif defined(__GNUC__) && ((__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)) //GCC
#ifdef __cplusplus
#define restrict __restrict
#endif
#ifdef __STDC_VERSION__ //STANDARD_C_1990
#define restrict __restrict
#endif
#if (__STDC_VERSION__ >= 199409L) //STANDARD_C_1994
#define restrict __restrict
#endif
#if (__STDC_VERSION__ >= 199901L) //STANDARD_C_1999
//restrict defined
#endif
#elif defined(__STDC__) //STANDARD_C_1989
#define restrict
#endif

//=============================================================================================================================================================================
// Integers anf float types
//=============================================================================================================================================================================
namespace PMBB_BASE {

typedef  int64_t   int64;
typedef  int32_t   int32;
typedef  int16_t   int16;
typedef  int8_t    int8;

typedef  uint64_t  uint64;
typedef  uint32_t  uint32;
typedef  uint16_t  uint16;
typedef  uint8_t   uint8;

typedef  uint8     byte;

typedef  float     flt32;
typedef  double    flt64;

typedef  uintptr_t uintPtr;
typedef  size_t    uintSize;

} //end of namespace PMBB_BASE

//=============================================================================================================================================================================
// Limits
//=============================================================================================================================================================================
namespace PMBB_BASE {

static constexpr int64  int64_max  = INT64_MAX;
static constexpr int64  int64_min  = INT64_MIN;
static constexpr int32  int32_max  = INT32_MAX;
static constexpr int32  int32_min  = INT32_MIN;
static constexpr int16  int16_max  = INT16_MAX;
static constexpr int16  int16_min  = INT16_MIN;
static constexpr int8   int8_max   = INT8_MAX ;
static constexpr int8   int8_min   = INT8_MIN ;

static constexpr uint64 uint64_max = UINT64_MAX;
static constexpr uint64 uint64_min = 0         ;
static constexpr uint32 uint32_max = UINT32_MAX;
static constexpr uint32 uint32_min = 0         ;
static constexpr uint16 uint16_max = UINT16_MAX;
static constexpr uint16 uint16_min = 0         ;
static constexpr uint8  uint8_max  = UINT8_MAX ;
static constexpr uint8  uint8_min  = 0         ;

static constexpr flt32 flt32_max = std::numeric_limits<flt32>::max    ();
static constexpr flt32 flt32_min = std::numeric_limits<flt32>::lowest ();
static constexpr flt32 flt32_eps = std::numeric_limits<flt32>::epsilon();

static constexpr flt64 flt64_max = std::numeric_limits<flt64>::max    ();
static constexpr flt64 flt64_min = std::numeric_limits<flt64>::lowest ();
static constexpr flt64 flt64_eps = std::numeric_limits<flt64>::epsilon();

static constexpr uintSize uintSize_max = SIZE_MAX;
static constexpr uintSize uintSize_min = 0;

} //end of namespace PMBB_BASE

//=============================================================================================================================================================================

