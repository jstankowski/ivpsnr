set(SRCLIST_APP src/main_QMIV.cpp src/xAppQMIV.h src/xAppQMIV.cpp)

target_sources(${PROJECT_NAME} PRIVATE ${SRCLIST_APP})
source_group(App FILES ${SRCLIST_APP})

