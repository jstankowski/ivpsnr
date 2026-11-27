/*
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#include "xPicHash.h"
#include "xXXH.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

xXXH128::tHash xPicHash::CalcPicHash(const xPicP& Pic)
{
  const int32 Width  = Pic.getWidth ();
  const int32 Height = Pic.getHeight();
  const int32 Stride = Pic.getStride();

  xXXH128 XXH128;
  XXH128.init();

  for(int32 CmpIdx = 0; CmpIdx < Pic.getNumCmps(); CmpIdx++)
  {
    const uint16* Ptr = Pic.getAddr((eCmp)CmpIdx);

    for(int32 y = 0; y < Height; y++)
    {
      XXH128.update((uint8*)Ptr, Width * sizeof(uint16));
      Ptr += Stride;
    }
  }

  xXXH128::tHash Hash = XXH128.finish();
  return Hash;
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB