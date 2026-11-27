/*
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xSHA1.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

void xSHA1::init()
{
  m_BlockLen = 0;
  m_BitLen   = 0;
  /* SHA1 initialization constants */
  m_State[0] = 0x67452301;
  m_State[1] = 0xefcdab89;
  m_State[2] = 0x98badcfe;
  m_State[3] = 0x10325476;
  m_State[4] = 0xc3d2e1f0;
}
void xSHA1::update(const uint8* Data, uint64 Length)
{
  for(size_t i = 0; i < Length; i++)
  {
    m_Data[m_BlockLen++] = Data[i];
    if(m_BlockLen == c_InputLength)
    {
      transform();

      // End of the block
      m_BitLen += 512;
      m_BlockLen = 0;
    }
  }
}
xSHA1::tHash xSHA1::finish()
{
  tHash Hash;
  pad();
  revert(Hash);
  return Hash;
}
void xSHA1::transform()
{
  uint32_t block[16];

  for(size_t i = 0; i < BLOCK_INTS; i++)
  {
    block[i] = (m_Data[4 * i + 3] & 0xff)
      | (m_Data[4 * i + 2] & 0xff) << 8
      | (m_Data[4 * i + 1] & 0xff) << 16
      | (m_Data[4 * i + 0] & 0xff) << 24;
  }

  /* Copy digest[] to working vars */
  uint32_t a = m_State[0];
  uint32_t b = m_State[1];
  uint32_t c = m_State[2];
  uint32_t d = m_State[3];
  uint32_t e = m_State[4];

  /* 4 rounds of 20 operations each. Loop unrolled. */
  R0(block, a, b, c, d, e,  0);
  R0(block, e, a, b, c, d,  1);
  R0(block, d, e, a, b, c,  2);
  R0(block, c, d, e, a, b,  3);
  R0(block, b, c, d, e, a,  4);
  R0(block, a, b, c, d, e,  5);
  R0(block, e, a, b, c, d,  6);
  R0(block, d, e, a, b, c,  7);
  R0(block, c, d, e, a, b,  8);
  R0(block, b, c, d, e, a,  9);
  R0(block, a, b, c, d, e, 10);
  R0(block, e, a, b, c, d, 11);
  R0(block, d, e, a, b, c, 12);
  R0(block, c, d, e, a, b, 13);
  R0(block, b, c, d, e, a, 14);
  R0(block, a, b, c, d, e, 15);
  R1(block, e, a, b, c, d,  0);
  R1(block, d, e, a, b, c,  1);
  R1(block, c, d, e, a, b,  2);
  R1(block, b, c, d, e, a,  3);
  R2(block, a, b, c, d, e,  4);
  R2(block, e, a, b, c, d,  5);
  R2(block, d, e, a, b, c,  6);
  R2(block, c, d, e, a, b,  7);
  R2(block, b, c, d, e, a,  8);
  R2(block, a, b, c, d, e,  9);
  R2(block, e, a, b, c, d, 10);
  R2(block, d, e, a, b, c, 11);
  R2(block, c, d, e, a, b, 12);
  R2(block, b, c, d, e, a, 13);
  R2(block, a, b, c, d, e, 14);
  R2(block, e, a, b, c, d, 15);
  R2(block, d, e, a, b, c,  0);
  R2(block, c, d, e, a, b,  1);
  R2(block, b, c, d, e, a,  2);
  R2(block, a, b, c, d, e,  3);
  R2(block, e, a, b, c, d,  4);
  R2(block, d, e, a, b, c,  5);
  R2(block, c, d, e, a, b,  6);
  R2(block, b, c, d, e, a,  7);
  R3(block, a, b, c, d, e,  8);
  R3(block, e, a, b, c, d,  9);
  R3(block, d, e, a, b, c, 10);
  R3(block, c, d, e, a, b, 11);
  R3(block, b, c, d, e, a, 12);
  R3(block, a, b, c, d, e, 13);
  R3(block, e, a, b, c, d, 14);
  R3(block, d, e, a, b, c, 15);
  R3(block, c, d, e, a, b,  0);
  R3(block, b, c, d, e, a,  1);
  R3(block, a, b, c, d, e,  2);
  R3(block, e, a, b, c, d,  3);
  R3(block, d, e, a, b, c,  4);
  R3(block, c, d, e, a, b,  5);
  R3(block, b, c, d, e, a,  6);
  R3(block, a, b, c, d, e,  7);
  R3(block, e, a, b, c, d,  8);
  R3(block, d, e, a, b, c,  9);
  R3(block, c, d, e, a, b, 10);
  R3(block, b, c, d, e, a, 11);
  R4(block, a, b, c, d, e, 12);
  R4(block, e, a, b, c, d, 13);
  R4(block, d, e, a, b, c, 14);
  R4(block, c, d, e, a, b, 15);
  R4(block, b, c, d, e, a,  0);
  R4(block, a, b, c, d, e,  1);
  R4(block, e, a, b, c, d,  2);
  R4(block, d, e, a, b, c,  3);
  R4(block, c, d, e, a, b,  4);
  R4(block, b, c, d, e, a,  5);
  R4(block, a, b, c, d, e,  6);
  R4(block, e, a, b, c, d,  7);
  R4(block, d, e, a, b, c,  8);
  R4(block, c, d, e, a, b,  9);
  R4(block, b, c, d, e, a, 10);
  R4(block, a, b, c, d, e, 11);
  R4(block, e, a, b, c, d, 12);
  R4(block, d, e, a, b, c, 13);
  R4(block, c, d, e, a, b, 14);
  R4(block, b, c, d, e, a, 15);

  // Add the working vars back into digest[]
  m_State[0] += a;
  m_State[1] += b;
  m_State[2] += c;
  m_State[3] += d;
  m_State[4] += e;
}
void xSHA1::pad()
{
	uint64_t i = m_BlockLen;
	uint8_t end = m_BlockLen < 56 ? 56 : 64;

	m_Data[i++] = 0x80; // Append a bit 1
	while (i < end) 
  {
		m_Data[i++] = 0x00; // Pad with zeros
	}

	if(m_BlockLen >= 56)
  {
		transform();
		memset(m_Data, 0, 56);
	}

	// Append to the padding the total message's length in bits and transform
	m_BitLen += m_BlockLen * 8;
	m_Data[63] = (uint8)(m_BitLen      );
	m_Data[62] = (uint8)(m_BitLen >>  8);
	m_Data[61] = (uint8)(m_BitLen >> 16);
	m_Data[60] = (uint8)(m_BitLen >> 24);
	m_Data[59] = (uint8)(m_BitLen >> 32);
	m_Data[58] = (uint8)(m_BitLen >> 40);
	m_Data[57] = (uint8)(m_BitLen >> 48);
	m_Data[56] = (uint8)(m_BitLen >> 56);
	transform();
}
void xSHA1::revert(tHash& Hash)
{
	// SHA uses big endian byte ordering - Revert all bytes
	for (uint8 i = 0 ; i < 4 ; i++)
  {
		for(uint8 j = 0 ; j < 5 ; j++)
    {
      Hash[i + (j * 4)] = (m_State[j] >> (24 - i * 8)) & 0x000000ff;
		}
	}
}
std::string xSHA1::format(const tHash& Hash)
{
  std::string Result; Result.reserve(40);
	for(uint32 i = 0 ; i < 20 ; i++) { Result += fmt::format("{:02X}", Hash[i]); }
	return Result;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB