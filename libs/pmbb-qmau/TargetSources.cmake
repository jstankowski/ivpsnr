set(SRCLIST_UTIL_H src/xMetricsAppQM.h   src/xUtilsAppQM.h   src/xAppQM_Common.h  )
set(SRCLIST_UTIL_C src/xMetricsAppQM.cpp src/xUtilsAppQM.cpp src/xAppQM_Common.cpp)

set(SRCLIST_PUBLIC   ${SRCLIST_UTIL_H})
set(SRCLIST_PRIVATE  ${SRCLIST_UTIL_C})

target_sources(${PROJECT_NAME} PRIVATE ${SRCLIST_PRIVATE} PUBLIC ${SRCLIST_PUBLIC})
source_group(Utils  FILES ${SRCLIST_UTIL_H} ${SRCLIST_UTIL_C})