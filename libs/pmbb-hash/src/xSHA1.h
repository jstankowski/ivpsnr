/*
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefHASH.h"
#include <array>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xSHA1
{
public:
  using tHash = std::array<uint8, 20>;
  static constexpr int32 c_InputLength = 64;

  static constexpr size_t BLOCK_INTS  = 16            ; //number of 32bit integers per SHA1 block
  static constexpr size_t BLOCK_BYTES = BLOCK_INTS * 4;

protected:
  uint8  m_Data[c_InputLength];
  uint32 m_BlockLen;
  uint64 m_BitLen;
  uint32 m_State[5] ;
  uint64 m_Transforms;

public:
  xSHA1() { init(); }
  void  init  ();
  void  update(const uint8* Data, uint64 Length);
  tHash finish();

  static std::string format(const tHash& Hash);

protected:
  static uint32_t rol(const uint32_t value, const size_t bits) { return (value << bits) | (value >> (32 - bits)); }
  static uint32_t blk(const uint32_t block[BLOCK_INTS], const size_t i) { return rol(block[(i + 13) & 15] ^ block[(i + 8) & 15] ^ block[(i + 2) & 15] ^ block[i], 1); }
  static void R0(const uint32_t* block, const uint32_t v, uint32_t& w, const uint32_t x, const uint32_t y, uint32_t& z, const size_t i) { z += ((w & (x ^ y)) ^ y) + block[i] + 0x5a827999 + rol(v, 5); w = rol(w, 30); }
  static void R1(uint32_t* block, const uint32_t v, uint32_t& w, const uint32_t x, const uint32_t y, uint32_t& z, const size_t i) { block[i] = blk(block, i); z += ((w & (x ^ y)) ^ y) + block[i] + 0x5a827999 + rol(v, 5); w = rol(w, 30); }
  static void R2(uint32_t* block, const uint32_t v, uint32_t& w, const uint32_t x, const uint32_t y, uint32_t& z, const size_t i) { block[i] = blk(block, i); z += (w ^ x ^ y) + block[i] + 0x6ed9eba1 + rol(v, 5);  w = rol(w, 30); }
  static void R3(uint32_t* block, const uint32_t v, uint32_t& w, const uint32_t x, const uint32_t y, uint32_t& z, const size_t i) { block[i] = blk(block, i); z += (((w | x) & y) | (w & x)) + block[i] + 0x8f1bbcdc + rol(v, 5); w = rol(w, 30); }
  static void R4(uint32_t* block, const uint32_t v, uint32_t& w, const uint32_t x, const uint32_t y, uint32_t& z, const size_t i) { block[i] = blk(block, i); z += (w ^ x ^ y) + block[i] + 0xca62c1d6 + rol(v, 5); w = rol(w, 30); }

  void transform();
  void pad      ();
  void revert   (tHash& hash);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB