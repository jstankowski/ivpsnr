/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

//===============================================================================================================================================================================================================
// MSVC workaround
//===============================================================================================================================================================================================================
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

//===============================================================================================================================================================================================================
// Base includes
//===============================================================================================================================================================================================================
#include <type_traits>
#include <algorithm>
#include <string>
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
#include <fmt/ranges.h>
#include <fmt/std.h>
#undef FMT_USE_WINDOWS_H

//===============================================================================================================================================================================================================
// Settings
//===============================================================================================================================================================================================================
#ifndef X_PMBB_EXPERIMENTAL
#define X_PMBB_EXPERIMENTAL 0
#endif

//===============================================================================================================================================================================================================
// Hard coded constrains
//===============================================================================================================================================================================================================
#define NOT_VALID  -1

//===============================================================================================================================================================================================================
// System section
//===============================================================================================================================================================================================================
#if (defined(WIN32) || defined(_WIN32) || defined(_WIN64)) //broken window
  #define X_PMBB_OPERATING_SYSTEM_WINDOWS 1
  #define X_PMBB_OPERATING_SYSTEM_NAME "Windows"
#elif defined(__linux__) //penguin
  #define X_PMBB_OPERATING_SYSTEM_LINUX 1
  #define X_PMBB_OPERATING_SYSTEM_NAME "Linux"
#elif (defined(__unix__) || defined(__unix) || defined(unix)) //classic unix + some BSDs
  #define X_PMBB_OPERATING_SYSTEM_UNIX 1
  #define X_PMBB_OPERATING_SYSTEM_NAME "Unix"
#elif (defined(__APPLE__) || defined(__MACH__)) //rotten fruit
  #define X_PMBB_OPERATING_SYSTEM_MACOS 1
  #define X_PMBB_OPERATING_SYSTEM_NAME "MACOS"
#else
  #error "Unknown target operating system"
  #define X_PMBB_OPERATING_SYSTEM_NAME "  "
#endif

//===============================================================================================================================================================================================================
// Compiler section
//===============================================================================================================================================================================================================
#if defined __INTEL_COMPILER
  #define X_PMBB_COMPILER_ICC 1
  #define X_PMBB_COMPILER_VER  __INTEL_COMPILER
  #define X_PMBB_COMPILER_NAME "intel_compiler"
#elif defined __clang__
  #define X_PMBB_COMPILER_CLANG 1
  #define X_PMBB_COMPILER_VER  __clang_major__
  #define X_PMBB_COMPILER_NAME "clang"
#elif defined __GNUC__
  #define X_PMBB_COMPILER_GCC 1
  #define X_PMBB_COMPILER_VER  __GNUC__
  #define X_PMBB_COMPILER_NAME "gcc"
#elif defined _MSC_VER
  #define X_PMBB_COMPILER_MSVC 1
  #define X_PMBB_COMPILER_VER  _MSC_VER
  #define X_PMBB_COMPILER_NAME "msvc"
#else
  #define X_PMBB_COMPILER_VER  0
  #define X_PMBB_COMPILER_NAME "unknown"
#endif

//===============================================================================================================================================================================================================
// architecture section
//===============================================================================================================================================================================================================
#if defined(_M_X64) || defined (__x86_64__)
  #define X_PMBB_ARCH_AMD64 1
  #define X_PMBB_ARCH_NAME  "AMD64"
#else
  #define X_PMBB_ARCH_NAME ""
#endif

//===============================================================================================================================================================================================================
// Arch includes
//===============================================================================================================================================================================================================
#if defined(X_PMBB_ARCH_AMD64)
#  if defined(X_PMBB_COMPILER_MSVC)
#    include <intrin.h>
#  else
#    include <x86intrin.h>
#  endif
#endif

//===============================================================================================================================================================================================================
// language version
//===============================================================================================================================================================================================================
#if defined(_MSVC_LANG)
#  define X_PMBB_CPUSPLUS_VER _MSVC_LANG
#else  // All other compilers.
#  define X_PMBB_CPUSPLUS_VER __cplusplus
#endif

#if X_PMBB_CPUSPLUS_VER < 201703
#  error Please compile for C++17 or higher
#endif

#if X_PMBB_CPUSPLUS_VER >= 202002L
#  define X_PMBB_CPP20 1
#else
#  define X_PMBB_CPP20 0
#endif

//===============================================================================================================================================================================================================
// C99 restrict pointers support
//===============================================================================================================================================================================================================
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

//===============================================================================================================================================================================================================
// Integers anf float types
//===============================================================================================================================================================================================================
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


//===============================================================================================================================================================================================================
// TRACER
//===============================================================================================================================================================================================================
#define X_PMBB_TRACE_ENABLE 0

#if X_PMBB_TRACE_ENABLE

#  ifdef X_PMBB_COMPILER_MSVC
#    define X_PMBB_TRACE              fmt::print("#  T={}  {} {} {}\n", std::this_thread::get_id(), __FUNCSIG__, __FILE__, __LINE__); std::fflush(stdout);
#    define X_PMBB_TRACE_T(TraceText) fmt::print("#  T={}  {}  {} {} {}\n", std::this_thread::get_id(), TraceText, __FUNCSIG__, __FILE__, __LINE__); std::fflush(stdout);
#  else
#    define X_PMBB_TRACE(TraceText)\
       {\
         fmt::print("#  {}  {} {} {}\n", TraceText, __PRETTY_FUNCTION__, __FILE__, __LINE__); \
         std::fflush(stdout); \
       }
#  endif
#else //X_PMBB_TRACE_ENABLE
#  define X_PMBB_TRACE(Verbose, Text)
#endif //X_PMBB_TRACE_ENABLE


//===============================================================================================================================================================================================================

