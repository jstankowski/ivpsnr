/*
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xSHA2.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

const std::array<uint32, 64> xSHA2_256::K = { 0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2 };

void xSHA2_256::init()
{ 
  m_BlockLen = 0;
  m_BitLen   = 0;
	m_State[0] = 0x6a09e667;
	m_State[1] = 0xbb67ae85;
	m_State[2] = 0x3c6ef372;
	m_State[3] = 0xa54ff53a;
	m_State[4] = 0x510e527f;
	m_State[5] = 0x9b05688c;
	m_State[6] = 0x1f83d9ab;
	m_State[7] = 0x5be0cd19;
}
void xSHA2_256::update(const uint8* Data, uint64 Length)
{
	for (size_t i = 0 ; i < Length ; i++) 
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
xSHA2_256::tHash xSHA2_256::finish()
{
  tHash Hash;
	pad   (    );
	revert(Hash);
	return Hash;
}
void xSHA2_256::transform()
{
	uint32 maj, xorA, ch, xorE, sum, newA, newE, m[64];
	uint32 state[8];

  for(uint8 i = 0, j = 0; i < 16; i++, j += 4) { m[i] = (m_Data[j] << 24) | (m_Data[j + 1] << 16) | (m_Data[j + 2] << 8) | (m_Data[j + 3]); } // Split data in 32 bit blocks for the 16 first words

  for(uint8 k = 16; k < 64; k++) { m[k] = sig1(m[k - 2]) + m[k - 7] + sig0(m[k - 15]) + m[k - 16]; } // Remaining 48 blocks

  for(uint8 i = 0; i < 8; i++) { state[i] = m_State[i]; }

  for(uint32 i = 0; i < 64; i++)
  {
    maj  = majority(state[0], state[1], state[2]);
    xorA = rotr(state[0], 2) ^ rotr(state[0], 13) ^ rotr(state[0], 22);
    ch   = choose(state[4], state[5], state[6]);
    xorE = rotr(state[4], 6) ^ rotr(state[4], 11) ^ rotr(state[4], 25);
    sum  = m[i] + K[i] + state[7] + ch + xorE;
    newA = xorA + maj + sum;
    newE = state[3] + sum;

    state[7] = state[6];
    state[6] = state[5];
    state[5] = state[4];
    state[4] = newE;
    state[3] = state[2];
    state[2] = state[1];
    state[1] = state[0];
    state[0] = newA;
  }

  for(uint32 i = 0; i < 8; i++) { m_State[i] += state[i]; }
}
void xSHA2_256::pad()
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
void xSHA2_256::revert(tHash& Hash)
{
	// SHA uses big endian byte ordering - Revert all bytes
	for (uint8 i = 0 ; i < 4 ; i++)
  {
		for(uint8 j = 0 ; j < 8 ; j++)
    {
      Hash[i + (j * 4)] = (m_State[j] >> (24 - i * 8)) & 0x000000ff;
		}
	}
}
std::string xSHA2_256::format(const tHash& Hash)
{
  std::string Result; Result.reserve(64);
	for(uint32 i = 0 ; i < 32 ; i++) { Result += fmt::format("{:02X}", Hash[i]); }
	return Result;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
