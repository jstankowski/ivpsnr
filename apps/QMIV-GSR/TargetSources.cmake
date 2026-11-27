set(SRCLIST_APP src/main_QMIVGSR.cpp src/xAppQMIVGSR.h src/xAppQMIVGSR.cpp)

target_sources(${PROJECT_NAME} PRIVATE ${SRCLIST_APP})
source_group(App FILES ${SRCLIST_APP})
