set(SRCLIST_COMMON_H src/xCommonDefCORE.h src/xMiscUtilsCORE.h  )
set(SRCLIST_COMMON_C                      src/xMiscUtilsCORE.cpp)

set(SRCLIST_DIST_H src/xDistortion.h src/xDistortionSTD.h   src/xDistortionSSE.h   src/xDistortionAVX.h   src/xDistortionAVX512.h   src/xDistortionNEON.h  )
set(SRCLIST_DIST_C                   src/xDistortionSTD.cpp src/xDistortionSSE.cpp src/xDistortionAVX.cpp src/xDistortionAVX512.cpp src/xDistortionNEON.cpp)

set(SRCLIST_PIXOPS_H src/xPixelOps.h src/xPixelOpsBase.h src/xPixelOpsSTD.h   src/xPixelOpsSSE.h   src/xPixelOpsAVX.h   src/xPixelOpsAVX512.h   src/xPixelOpsNEON.h   src/xMarginOps.h  )
set(SRCLIST_PIXOPS_C                                     src/xPixelOpsSTD.cpp src/xPixelOpsSSE.cpp src/xPixelOpsAVX.cpp src/xPixelOpsAVX512.cpp src/xPixelOpsNEON.cpp                   )

set(SRCLIST_CLR_H src/xColorSpaceCoeff.h   src/xColorSpace.h   src/xColorSpaceSTD.h   src/xColorSpaceSSE.h   src/xColorSpaceAVX.h   src/xColorSpaceAVX512.h   src/xColorSpaceNEON.h  )
set(SRCLIST_CLR_C src/xColorSpaceCoeff.cpp src/xColorSpace.cpp src/xColorSpaceSTD.cpp src/xColorSpaceSSE.cpp src/xColorSpaceAVX.cpp src/xColorSpaceAVX512.cpp src/xColorSpaceNEON.cpp)

set(SRCLIST_PIC_H src/xPicCommon.h   src/xPic.h   src/xPlane.h  )
set(SRCLIST_PIC_C src/xPicCommon.cpp src/xPic.cpp src/xPlane.cpp)

set(SRCLIST_BFR_H src/xBfrCommon.h src/xBfrPic.h  )
set(SRCLIST_BFR_C                  src/xBfrPic.cpp)

set(SRCLIST_THREAD_H src/xEvent.h src/xQueue.h src/xRing.h src/xThreadPool.h   src/xMultiThreaded.h)
set(SRCLIST_THREAD_C                                       src/xThreadPool.cpp                     )

set(SRCLIST_IO_H src/xInOutResult.h   src/xSeq.h   src/xStream.h   src/xFileListUtils.h  )
set(SRCLIST_IO_C src/xInOutResult.cpp src/xSeq.cpp src/xStream.cpp src/xFileListUtils.cpp)

set(SRCLIST_MATH_H src/xKBNS.h   src/xMatrix.h   src/xMatrixUtils.h   src/xQuaternion.h  )
set(SRCLIST_MATH_C src/xKBNS.cpp src/xMatrix.cpp src/xMatrixUtils.cpp src/xQuaternion.cpp)

set(SRCLIST_UTILS_H src/xVec.h src/xHelpersSIMD.h src/xHelpersFLT.h src/xFmtScn.h   src/xTestUtils.h   src/xTimeUtils.h  )
set(SRCLIST_UTILS_C                                                 src/xFmtScn.cpp src/xTestUtils.cpp src/xTimeUtils.cpp)

set(SRCLIST_PUBLIC  ${SRCLIST_COMMON_H} ${SRCLIST_DIST_H} ${SRCLIST_PIXOPS_H} ${SRCLIST_CLR_H} ${SRCLIST_PIC_H} ${SRCLIST_THREAD_H} ${SRCLIST_IO_H} ${SRCLIST_MATH_H} ${SRCLIST_UTILS_H})
set(SRCLIST_PRIVATE ${SRCLIST_COMMON_C} ${SRCLIST_DIST_C} ${SRCLIST_PIXOPS_C} ${SRCLIST_CLR_C} ${SRCLIST_PIC_C} ${SRCLIST_THREAD_C} ${SRCLIST_IO_C} ${SRCLIST_MATH_C} ${SRCLIST_UTILS_C})

target_sources(${PROJECT_NAME} PRIVATE ${SRCLIST_PRIVATE} PUBLIC ${SRCLIST_PUBLIC})
source_group(Common     FILES ${SRCLIST_COMMON_H} ${SRCLIST_COMMON_C})
source_group(Distortion FILES ${SRCLIST_DIST_H}   ${SRCLIST_DIST_C}  )
source_group(PixelOps   FILES ${SRCLIST_PIXOPS_H} ${SRCLIST_PIXOPS_C})
source_group(ClrSpc     FILES ${SRCLIST_CLR_H}    ${SRCLIST_CLR_C}   )
source_group(Picture    FILES ${SRCLIST_PIC_H}    ${SRCLIST_PIC_C}   )
source_group(Thread     FILES ${SRCLIST_THREAD_H} ${SRCLIST_THREAD_C})
source_group(IO         FILES ${SRCLIST_IO_H}     ${SRCLIST_IO_C}    )
source_group(Math       FILES ${SRCLIST_MATH_H}   ${SRCLIST_MATH_C}  )
source_group(Utils      FILES ${SRCLIST_UTILS_H}  ${SRCLIST_UTILS_C} )

