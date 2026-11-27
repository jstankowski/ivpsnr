/*
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xCommonDefHASH.h"
#include <array>

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xSHA2_256
{
public:
  using tHash = std::array<uint8, 32>;
  static constexpr int32 c_InputLength = 64;

protected:
  static const std::array<uint32, 64> K;

  uint8  m_Data[c_InputLength];
  uint32 m_BlockLen;
  uint64 m_BitLen  ;
  uint32 m_State[8]; //A, B, C, D, E, F, G, H

public:
  xSHA2_256() { init(); }
  void  init  ();
  void  update(const uint8* Data, uint64 Length);
  tHash finish();

  static std::string format(const tHash& Hash);

protected:
  static uint32 rotr    (uint32 x, uint32 n          ) { return (x >> n) | (x << (32 - n)); }
  static uint32 choose  (uint32 e, uint32 f, uint32 g) { return (e & f) ^ (~e & g); }
  static uint32 majority(uint32 a, uint32 b, uint32 c) { return (a & (b | c)) | (b & c); }
  static uint32 sig0    (uint32 x                    ) { return rotr(x,  7) ^ rotr(x, 18) ^ (x >>  3); }
  static uint32 sig1    (uint32 x                    ) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

  void transform();
  void pad      ();
  void revert   (tHash& hash);
};

//===============================================================================================================================================================================================================

} //end of namespace PMBB