/*
    SPDX-FileCopyrightText: 2019-2025 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "xCommonDefCORE.h"

using namespace PMBB_NAMESPACE;

//===============================================================================================================================================================================================================

uint32 xTestFastLog2(uint32 Value)
{
  for(uint32 i = 31; i >= 0; i--)
  {
    if(Value & (1 << i)) { return i; }
  }
  return std::numeric_limits<uint32>::max(); // Undefined
}

void testFastLog2()
{
  for(uint32 i = 1; i < 65536; i++)
  {
    uint32 Ref = xTestFastLog2(i);
    uint32 Tst = xFastLog2    (i);
    CHECK(Ref == Tst);
  }

  for(uint32 p = 0; p < 32; p++)
  {
    uint32 i = 1 << p;
    uint32 Ref = xTestFastLog2(i);
    uint32 Tst = xFastLog2(i);
    CHECK(Ref == Tst);
  }
}

//===============================================================================================================================================================================================================

TEST_CASE("testFastLog2")
{
  testFastLog2();
}

//===============================================================================================================================================================================================================
