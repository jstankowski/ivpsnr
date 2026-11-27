/*
    SPDX-FileCopyrightText: 2019-2024 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#define PMBB_xReprojectionUtils_IMPLEMENTATION
#include "xReprojectionUtils.h"

namespace PMBB_NAMESPACE::FVGS {

//===============================================================================================================================================================================================================

void xReprojectionUtils::xReprojectCoordInvZ_STD(flt32V2& TargetUV, const flt32V2& SourceUV, const float SourceInvZ, const xM4x4F* restrict PIP)
{
  const float SourceU = SourceUV.getX();
  const float SourceV = SourceUV.getY();

  float RowX = SourceU * PIP->M[0][0] + SourceV * PIP->M[0][1] + PIP->M[0][2] + SourceInvZ * PIP->M[0][3];
  float RowY = SourceU * PIP->M[1][0] + SourceV * PIP->M[1][1] + PIP->M[1][2] + SourceInvZ * PIP->M[1][3];
  float RowZ = SourceU * PIP->M[2][0] + SourceV * PIP->M[2][1] + PIP->M[2][2] + SourceInvZ * PIP->M[2][3];
  
  float InvRovZ = 1.0f / RowZ;
  float TargetU = RowX * InvRovZ;
  float TargetV = RowY * InvRovZ;  

  TargetUV.set(TargetU, TargetV);
}
void xReprojectionUtils::xReprojectCoordInvZ_STD(int32V2& TargetUV, const int32V2& SourceUV, const float SourceInvZ, const xM4x4F* restrict PIP)
{
  const float SourceX = (float)SourceUV.getX();
  const float SourceY = (float)SourceUV.getY();

  float RowX = SourceX * PIP->M[0][0] + SourceY * PIP->M[0][1] + PIP->M[0][2] + SourceInvZ * PIP->M[0][3];
  float RowY = SourceX * PIP->M[1][0] + SourceY * PIP->M[1][1] + PIP->M[1][2] + SourceInvZ * PIP->M[1][3];
  float RowZ = SourceX * PIP->M[2][0] + SourceY * PIP->M[2][1] + PIP->M[2][2] + SourceInvZ * PIP->M[2][3];

  float InvRovZ = 1.0f / RowZ;
  float TargetU = RowX * InvRovZ;
  float TargetV = RowY * InvRovZ;  

  TargetUV.set((int32)(TargetU + 0.5f), (int32)(TargetV + 0.5f));
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#if X_CAN_USE_SSE
void xReprojectionUtils::xReprojectCoordInvZ_SSE(flt32V2& TargetUV, const flt32V2& SourceUV, const float SourceInvZ, const xM4x4F* restrict PIP)
{
  //form XY1Z vector
  __m128 SourceUV00  = _mm_loadl_pi(_mm_setzero_ps(), (const __m64*)&SourceUV);
  __m128 SourceZ000  = _mm_load_ss(&SourceInvZ);
  __m128 Source1000  = _mm_set_ss(1.0f);
  __m128 Source1Z00  = _mm_unpacklo_ps(Source1000, SourceZ000);
  __m128 SourceUV1Z  = _mm_movelh_ps(SourceUV00, Source1Z00);

  //multiply and add horizontal
  __m128 RowX    = _mm_mul_ps (SourceUV1Z, PIP->R[0]);
  __m128 RowY    = _mm_mul_ps (SourceUV1Z, PIP->R[1]);
  __m128 RowZ    = _mm_mul_ps (SourceUV1Z, PIP->R[2]);
  __m128 RowXXYY = _mm_hadd_ps(RowX   , RowY   );
  __m128 RowZZZZ = _mm_hadd_ps(RowZ   , RowZ   );
  __m128 XYZZ    = _mm_hadd_ps(RowXXYY, RowZZZZ); 

  //divide X and Y components by Z component
  __m128 ZZZZ       = _mm_movehl_ps(XYZZ, XYZZ); // move the Z component from high to low
  __m128 TargetUV11 = _mm_div_ps(XYZZ, ZZZZ);

  //store result
  _mm_storel_pi((__m64*)&TargetUV, TargetUV11);
}
void xReprojectionUtils::xReprojectCoordInvZ_SSE(int32V2& TargetUV, const int32V2& SourceUV, const float SourceInvZ, const xM4x4F* restrict PIP)
{
  //form XY1Z vector
  __m128  SourceUV00  = _mm_cvtepi32_ps(_mm_loadl_epi64((const __m128i*)&SourceUV));
  __m128  SourceZ000  = _mm_load_ss(&SourceInvZ);
  __m128  Source1000  = _mm_set_ss(1.0f);
  __m128  Source1Z00  = _mm_unpacklo_ps(Source1000, SourceZ000);
  __m128  SourceUV1Z  = _mm_movelh_ps(SourceUV00, Source1Z00);

  //multiply and add horizontal
  __m128 RowX    = _mm_mul_ps (SourceUV1Z, PIP->R[0]);
  __m128 RowY    = _mm_mul_ps (SourceUV1Z, PIP->R[1]);
  __m128 RowZ    = _mm_mul_ps (SourceUV1Z, PIP->R[2]);
  __m128 RowXXYY = _mm_hadd_ps(RowX   , RowY   );
  __m128 RowZZZZ = _mm_hadd_ps(RowZ   , RowZ   );
  __m128 XYZZ    = _mm_hadd_ps(RowXXYY, RowZZZZ); 

  //divide X and Y components by Z component
  __m128 ZZZZ       = _mm_movehl_ps(XYZZ, XYZZ); // move the Z component from high to low
  __m128 TargetUV11 = _mm_div_ps(XYZZ, ZZZZ);

  //store result
  _mm_storel_epi64((__m128i*)&TargetUV, _mm_cvtps_epi32(TargetUV11));
}
#endif //X_CAN_USE_SSE

//===============================================================================================================================================================================================================

} //end of namespace AVLib
