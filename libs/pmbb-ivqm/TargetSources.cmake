set(SRCLIST_COMMON_H src/xCommonDefIVQM.h)

set(SRCLIST_MTC_H src/xMetricCommon.h  )
set(SRCLIST_MTC_C src/xMetricCommon.cpp)

set(SRCLIST_WS_H src/xWeightedSpherically.h  )
set(SRCLIST_WS_C src/xWeightedSpherically.cpp)

set(SRCLIST_GCD_H src/xGlobClrDiff.h  )
set(SRCLIST_GCD_C src/xGlobClrDiff.cpp)

set(SRCLIST_CPS_H src/xCorrespPixelShiftPrms.h src/xCorrespPixelShift.h src/xCorrespPixelShiftSTD.h   src/xCorrespPixelShiftSSE.h   src/xShftCompPic.h  )
set(SRCLIST_CPS_C                                                       src/xCorrespPixelShiftSTD.cpp src/xCorrespPixelShiftSSE.cpp src/xShftCompPic.cpp)

set(SRCLIST_PSNR_H src/xPSNR.h   src/xWSPSNR.h   src/xIVPSNR.h  )
set(SRCLIST_PSNR_C src/xPSNR.cpp src/xWSPSNR.cpp src/xIVPSNR.cpp)


set(SRCLIST_SSIM_H src/xStructSimConsts.h src/xStructSim.h   src/xStructSimSTD.h   src/xStructSimSSE.h   src/xStructSimAVX.h   src/xStructSimAVX512.h   src/xSSIM.h   src/xIVSSIM.h  )
set(SRCLIST_SSIM_C                                           src/xStructSimSTD.cpp src/xStructSimSSE.cpp src/xStructSimAVX.cpp src/xStructSimAVX512.cpp src/xSSIM.cpp src/xIVSSIM.cpp)

set(SRCLIST_PUBLIC  ${SRCLIST_COMMON_H} ${SRCLIST_MTC_H} ${SRCLIST_WS_H} ${SRCLIST_GCD_H} ${SRCLIST_CPS_H} ${SRCLIST_PSNR_H} ${SRCLIST_SSIM_H})
set(SRCLIST_PRIVATE                     ${SRCLIST_MTC_C} ${SRCLIST_WS_C} ${SRCLIST_GCD_C} ${SRCLIST_CPS_C} ${SRCLIST_PSNR_C} ${SRCLIST_SSIM_C})

target_sources(${PROJECT_NAME} PRIVATE ${SRCLIST_PRIVATE} PUBLIC ${SRCLIST_PUBLIC})
source_group(Common FILES ${SRCLIST_COMMON_H} ${SRCLIST_MTC_H} ${SRCLIST_MTC_C})
source_group(WS     FILES ${SRCLIST_WS_H} ${SRCLIST_WS_C})
source_group(GCD    FILES ${SRCLIST_GCD_H} ${SRCLIST_GCD_C})
source_group(CPS    FILES ${SRCLIST_CPS_H} ${SRCLIST_CPS_C})
source_group(PSNR   FILES ${SRCLIST_PSNR_H} ${SRCLIST_PSNR_C})
source_group(SSIM   FILES ${SRCLIST_SSIM_H} ${SRCLIST_SSIM_C})