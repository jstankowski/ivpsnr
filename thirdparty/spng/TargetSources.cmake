set(SRCLIST_SPNG_H  src/spng.h )
set(SRCLIST_SPNG_C  src/spng.c )
set(SRCLIST_MINIZ_H src/miniz.h)
set(SRCLIST_MINIZ_C src/miniz.c)

set(SRCLIST_PUBLIC  ${SRCLIST_SPNG_H} ${SRCLIST_MINIZ_H} )
set(SRCLIST_PRIVATE ${SRCLIST_SPNG_C} ${SRCLIST_MINIZ_C} )

target_sources(${PROJECT_NAME} PRIVATE ${SRCLIST_PRIVATE} PUBLIC ${SRCLIST_PUBLIC})
source_group(SPNG    FILES ${SRCLIST_SPNG_H}  ${SRCLIST_SPNG_C}  )
source_group(MINIZ   FILES ${SRCLIST_MINIZ_H} ${SRCLIST_MINIZ_C} )








