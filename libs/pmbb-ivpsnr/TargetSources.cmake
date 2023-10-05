set(SRCLIST_COMMON_H src/xCommonDefIVPSNR.h src/xMetricCommon.h)

set(SRCLIST_GCS_H src/xGlobalColorShift.h  )
set(SRCLIST_GCS_C src/xGlobalColorShift.cpp)

set(SRCLIST_CPS_H src/xCorrespPixelShift.h src/xCorrespPixelShiftSTD.h src/xCorrespPixelShiftSSE.h)
set(SRCLIST_CPS_C src/xCorrespPixelShift.cpp src/xCorrespPixelShiftSTD.cpp src/xCorrespPixelShiftSSE.cpp)

set(SRCLIST_IVPSNR_H src/xPSNR.h src/xWSPSNR.h src/xIVPSNR.h)
set(SRCLIST_IVPSNR_C src/xPSNR.cpp src/xWSPSNR.cpp src/xIVPSNR.cpp src/xIVPSNRM.cpp)

set(SRCLIST_PUBLIC  ${SRCLIST_COMMON_H} ${SRCLIST_GCS_H} ${SRCLIST_CPS_H} ${SRCLIST_IVPSNR_H})
set(SRCLIST_PRIVATE                     ${SRCLIST_GCS_C} ${SRCLIST_CPS_C} ${SRCLIST_IVPSNR_C})


target_sources(${PROJECT_NAME} PRIVATE ${SRCLIST_PRIVATE} PUBLIC ${SRCLIST_PUBLIC})
source_group(Common FILES ${SRCLIST_COMMON_H})
source_group(GCS    FILES ${SRCLIST_GCS_H} ${SRCLIST_GCS_C})
source_group(CPS    FILES ${SRCLIST_CPS_H} ${SRCLIST_CPS_C})
source_group(IVPSNR FILES ${SRCLIST_IVPSNR_H} ${SRCLIST_IVPSNR_C})