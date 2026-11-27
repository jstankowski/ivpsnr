set(SRCLIST_APP src/main_QMIVGS.cpp src/xAppQMIVGS.h src/xAppQMIVGS.cpp)

target_sources(${PROJECT_NAME} PRIVATE ${SRCLIST_APP})
source_group(App FILES ${SRCLIST_APP})

