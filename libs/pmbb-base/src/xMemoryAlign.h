/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "xCommonDefBASE.h"
#include <new>

#if defined(X_PMBB_ARCH_AMD64)
  #define PMBB_ALIGN_CACHE alignas(64)
#elif defined(X_PMBB_ARCH_ARM64) && defined(X_PMBB_OPERATING_SYSTEM_DARWIN)
  #define PMBB_ALIGN_CACHE alignas(128)
#elif defined(X_PMBB_ARCH_ARM64)
  #define PMBB_ALIGN_CACHE alignas(64)
  //NOTE: ARM64 defines std::hardware_destructive_interference_size as 256 (A64FX uses 256 byte cache lines), however, cores with cache line different than 64 bytes are exotic (Cavium ThunderX - 128 B, Fujitsu A64FX - 256 B).
#else
  #define PMBB_ALIGN_CACHE alignas(std::hardware_destructive_interference_size)
#endif
