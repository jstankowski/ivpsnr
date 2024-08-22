set(SRCLIST_COMMON_H src/xCommonDefPMBB-BASE.h)

set(SRCLIST_UTILS_H src/xCfgINI.h src/xFile.h src/xMemory.h src/xString.h)
set(SRCLIST_UTILS_C src/xCfgINI.cpp src/xFile.cpp src/xMemory.cpp src/xString.cpp)

set(SRCLIST_PROC_H src/xProcInfo.h  )
set(SRCLIST_PROC_C src/xProcInfo.cpp)

set(SRCLIST_DISPATCH_H src/xDispatch.h)
set(SRCLIST_DISPATCH_C ""             )

set(SRCLIST_PUBLIC  ${SRCLIST_COMMON_H} ${SRCLIST_UTILS_H} ${SRCLIST_PROC_H} ${SRCLIST_DISPATCH_H})
set(SRCLIST_PRIVATE                     ${SRCLIST_UTILS_C} ${SRCLIST_PROC_C} ${SRCLIST_DISPATCH_C})

target_sources(${PROJECT_NAME} PRIVATE ${SRCLIST_PRIVATE} PUBLIC ${SRCLIST_PUBLIC})
source_group(Common    FILES ${SRCLIST_COMMON_H})
source_group(Utils     FILES ${SRCLIST_UTILS_H} ${SRCLIST_UTILS_C})
source_group(Processor FILES ${SRCLIST_PROC_H} ${SRCLIST_PROC_C})
source_group(Dispatch  FILES ${SRCLIST_DISPATCH_H} ${SRCLIST_DISPATCH_C})

