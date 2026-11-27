/*
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xXXH.h"

#define XXH_NO_EXTERNC_GUARD    1 
#define XXH_STATIC_LINKING_ONLY 1 //access advanced declarations
#define XXH_IMPLEMENTATION      1 //access definitions
#define XXH_VECTOR              XXH_SCALAR

namespace {
#include "../thirdparty/xxhash.h" 
}

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

std::string xXXH128_Cmn::format(const tHash& Hash)
{
  std::string Result; Result.reserve(32);
  for(uint32 i = 0; i < 16; i++) { Result += fmt::format("{:02X}", Hash[i]); }
  return Result;
}

//===============================================================================================================================================================================================================

xXXH128_STD::xXXH128_STD()
{
  m_State = XXH3_createState();
}
xXXH128_STD::~xXXH128_STD()
{
  XXH3_freeState((XXH3_state_t*)m_State);
}
void xXXH128_STD::init()
{
  XXH3_128bits_reset((XXH3_state_t*)m_State);
}
void xXXH128_STD::update(const uint8* Data, uint64 Length)
{
  XXH3_128bits_update((XXH3_state_t*)m_State, Data, Length);
}
xXXH128_Cmn::tHash xXXH128_STD::finish()
{
  XXH128_hash_t Digest = XXH3_128bits_digest((XXH3_state_t*)m_State);
  tHash Hash;
  XXH128_canonicalFromHash((XXH128_canonical_t*)Hash.data(), Digest);
  return Hash;
}
xXXH128_Cmn::tHash xXXH128_STD::calc(const uint8* Data, uint64 Length)
{
  XXH128_hash_t Digest = XXH128(Data, Length, 0);
  tHash Hash;
  XXH128_canonicalFromHash((XXH128_canonical_t*)Hash.data(), Digest);
  return Hash;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB