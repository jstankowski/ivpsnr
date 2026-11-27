set(SRCLIST_COMMON_H src/xCommonDefFVGS.h)

set(SRCLIST_REPROJ_H src/xReprojection.h   src/xReprojectionUtils.h  )
set(SRCLIST_REPROJ_C src/xReprojection.cpp src/xReprojectionUtils.cpp)

set(SRCLIST_CAMERA_H src/xCameraModel.h   src/xCameraParams.h   src/xCameraParamsManager.h  )
set(SRCLIST_CAMERA_C src/xCameraModel.cpp src/xCameraParams.cpp src/xCameraParamsManager.cpp)

set(SRCLIST_CLOUD_H src/xPointCloud.h   src/xGaussianCloud.h   src/xPly.h   src/xSplatOps.h   src/xSphHarmUtils.h)
set(SRCLIST_CLOUD_C src/xPointCloud.cpp src/xGaussianCloud.cpp src/xPly.cpp src/xSplatOps.cpp                    )

set(SRCLIST_PUBLIC  ${SRCLIST_COMMON_H} ${SRCLIST_REPROJ_H} ${SRCLIST_CAMERA_H} ${SRCLIST_CLOUD_H})
set(SRCLIST_PRIVATE                     ${SRCLIST_REPROJ_C} ${SRCLIST_CAMERA_C} ${SRCLIST_CLOUD_C})

target_sources(${PROJECT_NAME} PRIVATE ${SRCLIST_PRIVATE} PUBLIC ${SRCLIST_PUBLIC})
source_group(Common FILES ${SRCLIST_COMMON_H})
source_group(Reproj FILES ${SRCLIST_REPROJ_H} ${SRCLIST_REPROJ_C})
source_group(Camera FILES ${SRCLIST_CAMERA_H} ${SRCLIST_CAMERA_C})
source_group(Cloud  FILES ${SRCLIST_CLOUD_H}  ${SRCLIST_CLOUD_C} )
source_group(Cloud  FILES ${SRCLIST_CLOUD_H}  ${SRCLIST_CLOUD_C} )