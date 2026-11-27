set(SRCLIST_COMMON_H src/xCommonDefGSR.h)

set(SRCLIST_EXP_H src/xExponential.h   src/xExponentialAVX.h   src/xExponentialAVX512.h  )

set(SRCLIST_PROJ_H src/xGSR_Projector.h src/xGSR_ProjectorSTD.h  )
set(SRCLIST_PROJ_C                      src/xGSR_ProjectorSTD.cpp)

set(SRCLIST_RAST_H src/xGSR_Rasterizer.h src/xGSR_RasterizerSTD.h   src/xGSR_RasterizerAVX.h   src/xGSR_RasterizerAVX512.h  )
set(SRCLIST_RAST_C src/xGSR_RasterizerSTD.cpp src/xGSR_RasterizerAVX.cpp src/xGSR_RasterizerAVX512.cpp)

set(SRCLIST_REND_H src/xProjectedCloud.h   src/xGSR_Renderer.h  )
set(SRCLIST_REND_C src/xProjectedCloud.cpp src/xGSR_Renderer.cpp)

set(SRCLIST_PUBLIC  ${SRCLIST_COMMON_H} ${SRCLIST_EXP_H} ${SRCLIST_PROJ_H} ${SRCLIST_RAST_H} ${SRCLIST_REND_H})
set(SRCLIST_PRIVATE                                      ${SRCLIST_PROJ_C} ${SRCLIST_RAST_C} ${SRCLIST_REND_C})

target_sources(${PROJECT_NAME} PRIVATE ${SRCLIST_PRIVATE} PUBLIC ${SRCLIST_PUBLIC})
source_group(Common     FILES ${SRCLIST_COMMON_H})
source_group(Exp        FILES ${SRCLIST_EXP_H}  )
source_group(Projector  FILES ${SRCLIST_PROJ_H} ${SRCLIST_PROJ_C})
source_group(Rasterizer FILES ${SRCLIST_RAST_H} ${SRCLIST_RAST_C})
source_group(Renderer   FILES ${SRCLIST_REND_H} ${SRCLIST_REND_C})
