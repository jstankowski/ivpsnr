set(SRCLIST_COMMON_H src/xCommonDefHASH.h)
set(SRCLIST_COMMON_C                     )

set(SRCLIST_HASH_H src/xSHA1.h   src/xSHA2.h   src/xXXH.h   thirdparty/xxhash.h)
set(SRCLIST_HASH_C src/xSHA1.cpp src/xSHA2.cpp src/xXXH_STD.cpp src/xXXH_SSE.cpp src/xXXH_AVX.cpp src/xXXH_AVX512.cpp)

set(SRCLIST_PIC_H src/xPicHash.h  )
set(SRCLIST_PIC_C src/xPicHash.cpp)

set(SRCLIST_PUBLIC  ${SRCLIST_COMMON_H} ${SRCLIST_HASH_H} ${SRCLIST_PIC_H})
set(SRCLIST_PRIVATE ${SRCLIST_COMMON_C} ${SRCLIST_HASH_C} ${SRCLIST_PIC_C})

target_sources(${PROJECT_NAME} PRIVATE ${SRCLIST_PRIVATE} PUBLIC ${SRCLIST_PUBLIC})
source_group(Common     FILES ${SRCLIST_COMMON_H} ${SRCLIST_COMMON_C})
source_group(Hash       FILES ${SRCLIST_HASH_H}   ${SRCLIST_HASH_C}  )
source_group(Picture    FILES ${SRCLIST_PIC_H}    ${SRCLIST_PIC_C}   )