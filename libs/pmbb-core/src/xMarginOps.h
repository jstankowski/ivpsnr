/*
    SPDX-FileCopyrightText: 2019-2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "xCommonDefCORE.h"

namespace PMBB_NAMESPACE {

//===============================================================================================================================================================================================================

class xMarginOps
{
public:
  template <typename PelType> static void ExtendMargin        (PelType* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin, PelType Constant, eMrgExt Mode);
  template <typename PelType> static void ExtendMarginNearest (PelType* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin                  ); //( a a a | a b c d | d d d )
  template <typename PelType> static void ExtendMarginReflect (PelType* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin                  ); //( c b a | a b c d | d c b )
  template <typename PelType> static void ExtendMarginMirror  (PelType* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin                  ); //( d c b | a b c d | c b a )
  template <typename PelType> static void ExtendMarginConstant(PelType* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin, PelType Constant); //( k k k | a b c d | k k k )
  template <typename PelType> static void ExtendMarginZero    (PelType* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin                  ); //( 0 0 0 | a b c d | 0 0 0 )
};

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

template <typename PelType> void xMarginOps::ExtendMargin(PelType* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin, PelType Constant, eMrgExt Mode)
{
  assert(Margin < Width && Margin < Height);

  switch(Mode)
  {
  case eMrgExt::None     : return                                                                      ; break;
  case eMrgExt::Nearest  : ExtendMarginNearest <PelType>(Addr, Stride, Width, Height, Margin          ); break;
  case eMrgExt::Reflect  : ExtendMarginReflect <PelType>(Addr, Stride, Width, Height, Margin          ); break;
  case eMrgExt::Mirror   : ExtendMarginMirror  <PelType>(Addr, Stride, Width, Height, Margin          ); break;
  case eMrgExt::Constant : ExtendMarginConstant<PelType>(Addr, Stride, Width, Height, Margin, Constant); break;
  case eMrgExt::Zero     : ExtendMarginZero    <PelType>(Addr, Stride, Width, Height, Margin          ); break;
  default                : assert(0); abort(); break;
  }
}
template <typename PelType> void xMarginOps::ExtendMarginNearest(PelType* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin)
{
  //left/right
  PelType* TmpAddr = Addr;
  for(int32 y = 0; y < Height; y++)
  {
    const PelType Left  = TmpAddr[0        ];
    const PelType Right = TmpAddr[Width - 1];
    for(int32 x = 0; x < Margin; x++)
    {
      TmpAddr[x - Margin] = Left ; //left
      TmpAddr[x + Width ] = Right; //right
    }
    TmpAddr += Stride;
  }

  const int32 EffectiveWidth = Width + (Margin << 1);

  //below
  const PelType* const SrcAddrB = Addr + ((Height - 1) * Stride) - Margin;
  PelType* restrict    DstAddrB = Addr + ((Height    ) * Stride) - Margin;  
  for(int32 y = 0; y < Margin; y++)
  {
    xMemcpyX(DstAddrB + y * Stride, SrcAddrB, EffectiveWidth);
  }

  //above
  const PelType* const SrcAddrA = Addr                     - Margin;
  PelType* restrict    DstAddrA = Addr - (Margin * Stride) - Margin;
  for(int32 y = 0; y < Margin; y++)
  {
    xMemcpyX(DstAddrA + y * Stride, SrcAddrA, EffectiveWidth);
  }
}
template <typename PelType> void xMarginOps::ExtendMarginReflect(PelType* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin)
{
  //above
  const PelType*    SrcAddrA = Addr;
  PelType* restrict DstAddrA = Addr - Stride;
  for(int32 y = 0; y < Margin; y++)
  {
    xMemcpyX(DstAddrA - (y * Stride), SrcAddrA + (y * Stride), Width);
  }

  //left/right
  PelType* RowAddr = Addr;
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Margin; x++)
    {
      RowAddr[-x - 1   ] = RowAddr[x            ]; //left
      RowAddr[Width + x] = RowAddr[Width - 1 - x]; //right
    }
    RowAddr += Stride;
  }

  //below
  const PelType*    SrcAddrB = Addr + ((Height - 1) * Stride);
  PelType* restrict DstAddrB = Addr + ((Height    ) * Stride);
  for(int32 y = 0; y < Margin; y++)
  {
    xMemcpyX(DstAddrB + y * Stride, SrcAddrB - y * Stride, Width);
  }

  //corners
  PelType* AddrTLC = Addr;
  for(int32 y = 0; y < Margin; y++)
  {
    for(int32 x = 0; x < Margin; x++)
    {
      AddrTLC[-(y + 1)     * Stride - (x + 1)  ] = AddrTLC[               y * Stride + x            ];
      AddrTLC[-(y + 1)     * Stride + Width + x] = AddrTLC[               y * Stride + Width - x - 1];
      AddrTLC[(Height + y) * Stride - (x + 1)  ] = AddrTLC[(Height - y - 1) * Stride + x            ];
      AddrTLC[(Height + y) * Stride + Width + x] = AddrTLC[(Height - y - 1) * Stride + Width - x - 1];
    }
  }
}
template <typename PelType> void xMarginOps::ExtendMarginMirror(PelType* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin)
{
  //above
  const PelType*    SrcAddrA = Addr + Stride;
  PelType* restrict DstAddrA = Addr - Stride;
  for(int32 y = 0; y < Margin; y++)
  {
    xMemcpyX(DstAddrA - (y * Stride), SrcAddrA + (y * Stride), Width);
  }

  //left/right
  PelType* RowAddr = Addr;
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Margin; x++)
    {
      RowAddr[-x - 1   ] = RowAddr[x + 1        ]; //left
      RowAddr[Width + x] = RowAddr[Width - 2 - x]; //right
    }
    RowAddr += Stride;
  }

  //below
  const PelType*    SrcAddrB = Addr + ((Height - 2) * Stride);
  PelType* restrict DstAddrB = Addr + (Height       * Stride);
  for(int32 y = 0; y < Margin; y++)
  {
    xMemcpyX(DstAddrB + y * Stride, SrcAddrB - y * Stride, Width);
  }

  //corners
  PelType* AddrTLC = Addr;
  for(int32 y = 0; y < Margin; y++)
  {
    for(int32 x = 0; x < Margin; x++)
    {
      AddrTLC[-(y + 1)     * Stride - (x + 1)  ] = AddrTLC[(y + 1)          * Stride + x + 1        ];
      AddrTLC[-(y + 1)     * Stride + Width + x] = AddrTLC[(y + 1)          * Stride + Width - x - 2];
      AddrTLC[(Height + y) * Stride - (x + 1)  ] = AddrTLC[(Height - y - 2) * Stride + x + 1        ];
      AddrTLC[(Height + y) * Stride + Width + x] = AddrTLC[(Height - y - 2) * Stride + Width - x - 2];
    }
  }
}
template <typename PelType> void xMarginOps::ExtendMarginConstant(PelType* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin, PelType Constant)
{
  //left/right
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Margin; x++)
    {
      Addr[x - Margin] = Constant;
      Addr[x + Width ] = Constant;
    }
    Addr += Stride;
  }

  const int32 EffectiveWidth = Width + (Margin << 1);

  //below
  Addr -= (Stride + Margin);
  for(int32 y = 0; y < Margin; y++)
  {
    xMemsetX(Addr + (y + 1) * Stride, Constant, EffectiveWidth);
  }
  //above
  Addr -= ((Height - 1) * Stride);
  for(int32 y = 0; y < Margin; y++)
  {
    xMemsetX(Addr - (y + 1) * Stride, Constant, EffectiveWidth);
  }
}
template <typename PelType> void xMarginOps::ExtendMarginZero(PelType* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin)
{
  //left/right
  for(int32 y = 0; y < Height; y++)
  {
    for(int32 x = 0; x < Margin; x++)
    {
      Addr[x - Margin] = 0;
      Addr[x + Width ] = 0;
    }
    Addr += Stride;
  }

  const int32 EffectiveWidth = Width + (Margin << 1);

  //below
  Addr -= (Stride + Margin);
  for(int32 y = 0; y < Margin; y++)
  {
    ::memset(Addr + (y + 1) * Stride, 0, sizeof(PelType) * EffectiveWidth);
  }
  //above
  Addr -= ((Height - 1) * Stride);
  for(int32 y = 0; y < Margin; y++)
  {
    ::memset(Addr - (y + 1) * Stride, 0, sizeof(PelType) * EffectiveWidth);
  }
}

//===============================================================================================================================================================================================================

} //end of namespace PMBB
