/*
    SPDX-FileCopyrightText: 2026 Błażej Szydełko <blazej.szydelko@put.poznan.pl>
    SPDX-FileCopyrightText: 2026 Jakub Stankowski <jakub.stankowski@put.poznan.pl>
    SPDX-License-Identifier: BSD-3-Clause
*/

#include "xCommonDefGSR.h"

#if X_PMBB_GSR_DISABLE_FP_CONTRACT_IN_PROJECTOR
X_PMBB_DISABLE_FP_CONTRACT_BEG;
#endif

#include "xGSR_ProjectorSTD.h"
#include "xCameraModel.h"    // xRotationMat, xExtrinsicMat, xIntrinsicMat
#include "xExponential.h"
#include <cmath>
#include <algorithm>

namespace PMBB_NAMESPACE::GSR {

//===============================================================================================================================================================================================================

static inline flt32 xExp(flt32 x) { return xExponential<X_PMBB_GSR_EXP_BOUND_CHECKING, X_PMBB_GSR_EXP_USE_FMA>::PreciseExp(x); }
static inline flt64 xExp(flt64 x) { return xExponential<X_PMBB_GSR_EXP_BOUND_CHECKING, X_PMBB_GSR_EXP_USE_FMA>::PreciseExp(x); }

//===============================================================================================================================================================================================================

void xProjectorSTD::ProjectRng(xProjectedCloud* Projected, uint8* ValidMask, const tGaussI* Cloud, const tCamPar& CamParams, int32V2 PicSize, int32 BegIdx, int32 EndIdx)
{
  using tFlt   = tProjInternalFloatType;
  using tFltV3 = xVec3<tFlt>;

  // camera parameters
  const auto FocalLen = CamParams.getIntrinsics().getFocalLengths  (); // fx, fy
  const auto PrinciPt = CamParams.getIntrinsics().getPrincipalPoint(); // cx, cy
  const tFlt  Fx = (tFlt)FocalLen.getX(), Fy = (tFlt)FocalLen.getY();
  const tFlt  Cx = (tFlt)PrinciPt.getX(), Cy = (tFlt)PrinciPt.getY();
  const int32 W  = PicSize .getX(), H  = PicSize .getY();

  const auto& ExtLoc = CamParams.getExtrinsicsLocal();
  // Rotation matrix W (3x3) - transforms 3D point from world to camera space (upper-left of ExtrinsicsLocal)
  const tFlt W00 = (tFlt)ExtLoc.at(0,0), W01 = (tFlt)ExtLoc.at(0,1), W02 = (tFlt)ExtLoc.at(0,2);
  const tFlt W10 = (tFlt)ExtLoc.at(1,0), W11 = (tFlt)ExtLoc.at(1,1), W12 = (tFlt)ExtLoc.at(1,2);
  const tFlt W20 = (tFlt)ExtLoc.at(2,0), W21 = (tFlt)ExtLoc.at(2,1), W22 = (tFlt)ExtLoc.at(2,2);
  // Translation vector t (3x1) - ExtrinsicsLocal column 3 = -R*T_world
  const tFlt TX  = (tFlt)ExtLoc.at(0,3), TY  = (tFlt)ExtLoc.at(1,3), TZ  = (tFlt)ExtLoc.at(2,3);

  // MGS mode - clamp limit for x/z and y/z (1.3 * half-FOV in normalised coords)
  const tFlt LimitX = (tFlt)1.3 * ((tFlt)0.5 * (flt32)W) / Fx;
  const tFlt LimitY = (tFlt)1.3 * ((tFlt)0.5 * (flt32)H) / Fy;

  for(int32 SplatIdx = BegIdx; SplatIdx < EndIdx; SplatIdx++)
  {
    const FVGS::xGaussianObject& GaussObj = Cloud->getGaussianObjects()[SplatIdx];

    // 1. Transform world position to camera space
    const tFlt Px = GaussObj.PositionXYX[0], Py = GaussObj.PositionXYX[1], Pz = GaussObj.PositionXYX[2];
    const tFlt CamX = W00*Px + W01*Py + W02*Pz + TX;
    const tFlt CamY = W10*Px + W11*Py + W12*Pz + TY;
    const tFlt CamZ = W20*Px + W21*Py + W22*Pz + TZ;

    // 2. Frustum cull
    const tFlt Depth = CamZ - c_DepthOffset;
    if(Depth <= c_PlaneNear || CamZ >= c_PlaneFar) { ValidMask[SplatIdx] = 0; continue; }

    // 3. Project center to screen (Screen position)
    const tFlt InvCamZ  = 1.0f / CamZ;
    const tFlt ScreenX  = Fx * CamX * InvCamZ + Cx;
    const tFlt ScreenY  = Fy * CamY * InvCamZ + Cy;

    // 4. Calc splat rotation matrix from quaternion (x,y,z,w) (Quaternion -> rotation matrix)
    const tFlt qw = GaussObj.RotWXYZ[0], qx = GaussObj.RotWXYZ[1], qy = GaussObj.RotWXYZ[2], qz = GaussObj.RotWXYZ[3];
    const tFlt qLen = std::sqrt(qx*qx + qy*qy + qz*qz + qw*qw); // normalize quaternion
    const tFlt qxi = qx/qLen, qyi = qy/qLen, qzi = qz/qLen, qwi = qw/qLen;
    const tFlt xx=qxi*qxi, yy=qyi*qyi, zz=qzi*qzi;
    const tFlt xy=qxi*qyi, xz=qxi*qzi, yz=qyi*qzi;
    const tFlt wx=qwi*qxi, wy=qwi*qyi, wz=qwi*qzi;
    const tFlt Rs00=1-2*(yy+zz), Rs01=2*(xy-wz)  , Rs02=2*(xz+wy)  ; // R_splat (column-major stored as R[row][col]) ??????
    const tFlt Rs10=2*(xy+wz)  , Rs11=1-2*(xx+zz), Rs12=2*(yz-wx)  ;
    const tFlt Rs20=2*(xz-wy)  , Rs21=2*(yz+wx)  , Rs22=1-2*(xx+yy);

    // 5. Calc 3D covariance Sigma_3D = R_splat * diag(s^2) * R_splat^T // s = exp(log_scale)
    const tFlt Sx  = xExp(GaussObj.ScaleXYZ[0]), Sy = xExp(GaussObj.ScaleXYZ[1]), Sz = xExp(GaussObj.ScaleXYZ[2]);
    const tFlt Sx2 = Sx*Sx, Sy2 = Sy*Sy, Sz2 = Sz*Sz;
    const tFlt S00 = Rs00*Rs00*Sx2 + Rs01*Rs01*Sy2 + Rs02*Rs02*Sz2; // Sigma_3D[r][c] = sum_k Rs[r][k] * sk^2 * Rs[c][k]
    const tFlt S01 = Rs00*Rs10*Sx2 + Rs01*Rs11*Sy2 + Rs02*Rs12*Sz2;
    const tFlt S02 = Rs00*Rs20*Sx2 + Rs01*Rs21*Sy2 + Rs02*Rs22*Sz2;
    const tFlt S11 = Rs10*Rs10*Sx2 + Rs11*Rs11*Sy2 + Rs12*Rs12*Sz2;
    const tFlt S12 = Rs10*Rs20*Sx2 + Rs11*Rs21*Sy2 + Rs12*Rs22*Sz2;
    const tFlt S22 = Rs20*Rs20*Sx2 + Rs21*Rs21*Sy2 + Rs22*Rs22*Sz2;
    // Sigma_3D is symmetric, so S10=S01, S20=S02, S21=S12

    // 6. Project 3D covariance to 2D
      // Sigma_2D = J * W * Sigma_3D * W^T * J^T
      // J (2x3) = perspective Jacobian at (CamX, CamY, CamZ):
      //   J = [Fx/CamZ,       0, -Fx*CamX/CamZ^2]
      //       [      0, Fy/CamZ, -Fy*CamY/CamZ^2]
      // W (3x3) = camera rotation (world->camera)
      // T = J * W  (2x3)      
      // T[row][col] = J[row][0]*W[0][col] + J[row][1]*W[1][col] + J[row][2]*W[2][col]

      // Perspective Jacobian with mpeg-compat clamping:
      //    tX = CamZ * clamp(CamX/CamZ, -LimitX, LimitX)  (= CamX when not clamped)
      //    J = [Fx/CamZ,       0, -Fx*tX/CamZ^2]
      //        [0,       Fy/CamZ, -Fy*tY/CamZ^2]

    const tFlt InvCamZ2 = InvCamZ * InvCamZ;
    tFlt tX = CamX;
    tFlt tY = CamY;
    if constexpr (c_ProjClamping)
    {
      const tFlt XoZ = CamX * InvCamZ;
      const tFlt YoZ = CamY * InvCamZ;
      tX  = CamZ * std::max(-LimitX, std::min(LimitX, XoZ));
      tY  = CamZ * std::max(-LimitY, std::min(LimitY, YoZ));
    }
    const tFlt J00 = Fx * InvCamZ, J02 = -Fx * tX * InvCamZ2;
    const tFlt J11 = Fy * InvCamZ, J12 = -Fy * tY * InvCamZ2;
    // row 0: J[0][0]=J00, J[0][1]=0, J[0][2]=J02
    const tFlt T00 = J00 * W00 + J02 * W20;
    const tFlt T01 = J00 * W01 + J02 * W21;
    const tFlt T02 = J00 * W02 + J02 * W22;
    // row 1: J[1][0]=0, J[1][1]=J11, J[1][2]=J12
    const tFlt T10 = J11 * W10 + J12 * W20;
    const tFlt T11 = J11 * W11 + J12 * W21;
    const tFlt T12 = J11 * W12 + J12 * W22;

    // Sigma_2D = T * Sigma_3D * T^T
    tFlt ST0_0 = S00 * T00 + S01 * T01 + S02 * T02;
    tFlt ST0_1 = S01 * T00 + S11 * T01 + S12 * T02;
    tFlt ST0_2 = S02 * T00 + S12 * T01 + S22 * T02;
    tFlt ST1_0 = S00 * T10 + S01 * T11 + S02 * T12;
    tFlt ST1_1 = S01 * T10 + S11 * T11 + S12 * T12;
    tFlt ST1_2 = S02 * T10 + S12 * T11 + S22 * T12;
    tFlt CovA  = T00 * ST0_0 + T01 * ST0_1 + T02 * ST0_2;
    tFlt CovB  = T00 * ST1_0 + T01 * ST1_1 + T02 * ST1_2;
    tFlt CovC  = T10 * ST1_0 + T11 * ST1_1 + T12 * ST1_2;

    // Regularize diagonal
    CovA += c_CovRegularize;
    CovC += c_CovRegularize;

    // 7. Invert 2D covariance (Invert Sigma_2D)
    tFlt det_ac = CovA * CovC;
    tFlt det_bb = CovB * CovB;
    const tFlt Det = std::max(det_ac - det_bb, 1.0e-10f);
    const tFlt InvDet  = 1.0f / Det;
    const tFlt InvCovA =  CovC * InvDet;
    const tFlt InvCovB = -CovB * InvDet;
    const tFlt InvCovC =  CovA * InvDet;

    // 8. Compute bounding box from largest eigenvalue of Sigma_2D 
    const int32 PicWidthMinus1  = W - 1;
    const int32 PicWidthHeight1 = H - 1;
    const int32 RadiusX         = (int32)std::ceil(std::sqrt(CovA) * c_CullSigmas);
    const int32 RadiusY         = (int32)std::ceil(std::sqrt(CovC) * c_CullSigmas);
    const int32 BoundingBoxX0   = (int32)std::floor(ScreenX) - RadiusX;
    const int32 BoundingBoxY0   = (int32)std::floor(ScreenY) - RadiusY;
    const int32 BoundingBoxX1   = (int32)std::ceil (ScreenX) + RadiusX;
    const int32 BoundingBoxY1   = (int32)std::ceil (ScreenY) + RadiusY;
    // Cull if bounding box is entirely outside image 
    if(BoundingBoxX1 < 0 || BoundingBoxX0 > PicWidthMinus1 || BoundingBoxY1 < 0 || BoundingBoxY0 > PicWidthHeight1) { ValidMask[SplatIdx] = 0; continue; }
    const int32 BoundingBoxImgX0  = xClip(BoundingBoxX0, 0, PicWidthMinus1 );
    const int32 BoundingBoxImgY0  = xClip(BoundingBoxY0, 0, PicWidthHeight1);
    const int32 BoundingBoxImgX1  = xClip(BoundingBoxX1, 0, PicWidthMinus1 );
    const int32 BoundingBoxImgY1  = xClip(BoundingBoxY1, 0, PicWidthHeight1);

    // 9. Evaluate SH color
    // 3DGS SH coefficients are trained with world-space direction from camera to Gaussian.
    // CamXYZ = R_w2c * (P - pos), so R_w2c^T * CamXYZ = P - pos = world-space cam-to-gauss vector.
    const tFlt DirLen = std::sqrt(CamX*CamX + CamY*CamY + CamZ*CamZ);
    const tFlt WX = W00*CamX + W10*CamY + W20*CamZ;
    const tFlt WY = W01*CamX + W11*CamY + W21*CamZ;
    const tFlt WZ = W02*CamX + W12*CamY + W22*CamZ;
    const tFltV3 ViewDir = { WX/DirLen, WY/DirLen, WZ/DirLen };
    const tFltV3 Color   = xEvalSphericalHarmonics(Cloud, SplatIdx, ViewDir);

    // 10. Opacity: sigmoid(OP)
    const tFlt Alpha = 1.0f / (1.0f + xExp(-GaussObj.Opacity));

    // Store to projected cloud
    Projected->setDepth      (SplatIdx, (flt32)Depth);
    Projected->setBoundingBox(SplatIdx, (uint16)BoundingBoxImgX0, (uint16)BoundingBoxImgY0, (uint16)BoundingBoxImgX1, (uint16)BoundingBoxImgY1);
    Projected->setSplatData  (SplatIdx, (flt32)ScreenX, (flt32)ScreenY, (flt32)InvCovA, (flt32)InvCovB, (flt32)InvCovC, (flt32)Color.getX(), (flt32)Color.getY(), (flt32)Color.getZ(), (flt32)Alpha);
    ValidMask[SplatIdx] = 1;
  } //end of loop over SplatIdx
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

xProjectorSTD::tFltV3 xProjectorSTD::xEvalSphericalHarmonics(const tGaussI* Cloud, int32 SplatIdx, tFltV3 ViewDir)
{
  using namespace FVGS;
  const xGaussianObject& GaussObj = Cloud->getGaussianObjects()[SplatIdx];

  const int32 NumSphHarm = Cloud->getNumSphHarm(); // number of AC coefficients
  tFltV3 Color = xEvaluateAtDirection(ViewDir.getX(), ViewDir.getY(), ViewDir.getZ(), NumSphHarm, GaussObj.DC_RGB, GaussObj.Rest_RGB, GaussObj.Rest_RGB + NumSphHarm, GaussObj.Rest_RGB + 2*NumSphHarm);
  // Add 0.5 DC offset (same convention as 3DGS training)
  return { std::max(0.0f, Color.getX() + 0.5f), std::max(0.0f, Color.getY() + 0.5f), std::max(0.0f, Color.getZ() + 0.5f) };
}
xProjectorSTD::tFltV3 xProjectorSTD::xEvaluateAtDirection(tFlt vx, tFlt vy, tFlt vz, int32 SphHarmDegree, const tFlt* DC_RGB, const tFlt* SH_R, const tFlt* SH_G, const tFlt* SH_B)
{
  tFlt r = C0 * DC_RGB[0];
  tFlt g = C0 * DC_RGB[1];
  tFlt b = C0 * DC_RGB[2];

  if(SphHarmDegree > 0)
  {
    const tFlt xx = vx * vx, yy = vy * vy, zz = vz * vz;
    const tFlt xy = vx * vy, yz = vy * vz, xz = vx * vz;

    r += C1 * (-vy * SH_R[0] + vz * SH_R[1] - vx * SH_R[2]);
    g += C1 * (-vy * SH_G[0] + vz * SH_G[1] - vx * SH_G[2]);
    b += C1 * (-vy * SH_B[0] + vz * SH_B[1] - vx * SH_B[2]);

    r += C2_0*xy*SH_R[3] + C2_1*yz*SH_R[4] + C2_2*(2*zz-xx-yy)*SH_R[5] + C2_3*xz*SH_R[6] + C2_4*(xx-yy)*SH_R[7];
    g += C2_0*xy*SH_G[3] + C2_1*yz*SH_G[4] + C2_2*(2*zz-xx-yy)*SH_G[5] + C2_3*xz*SH_G[6] + C2_4*(xx-yy)*SH_G[7];
    b += C2_0*xy*SH_B[3] + C2_1*yz*SH_B[4] + C2_2*(2*zz-xx-yy)*SH_B[5] + C2_3*xz*SH_B[6] + C2_4*(xx-yy)*SH_B[7];

    r += C3_0*vy*(3*xx-yy)*SH_R[8]  + C3_1*xy*vz*SH_R[9] + C3_2*vy*(4*zz-xx-yy)*SH_R[10]
       + C3_3*vz*(2*zz-3*xx-3*yy)*SH_R[11] + C3_4*vx*(4*zz-xx-yy)*SH_R[12] + C3_5*vz*(xx-yy)*SH_R[13] + C3_6*vx*(xx-3*yy)*SH_R[14];
    g += C3_0*vy*(3*xx-yy)*SH_G[8]  + C3_1*xy*vz*SH_G[9] + C3_2*vy*(4*zz-xx-yy)*SH_G[10]
       + C3_3*vz*(2*zz-3*xx-3*yy)*SH_G[11] + C3_4*vx*(4*zz-xx-yy)*SH_G[12] + C3_5*vz*(xx-yy)*SH_G[13] + C3_6*vx*(xx-3*yy)*SH_G[14];
    b += C3_0*vy*(3*xx-yy)*SH_B[8]  + C3_1*xy*vz*SH_B[9] + C3_2*vy*(4*zz-xx-yy)*SH_B[10]
       + C3_3*vz*(2*zz-3*xx-3*yy)*SH_B[11] + C3_4*vx*(4*zz-xx-yy)*SH_B[12] + C3_5*vz*(xx-yy)*SH_B[13] + C3_6*vx*(xx-3*yy)*SH_B[14];
  }

  return { r, g, b };
}

//=============================================================================================================================================================================

} //end of namespace PMBB::GSR
