cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

#=========================================================================================================================================
# library PUT-Multimedia-Building-Blocks - sequence from file list
#=========================================================================================================================================
set(LIB_CURRENT_NAME "pmbb-slst")
set(LIB_PMBB_SLST_NAME "${LIB_CURRENT_NAME}")
set(LIB_PMBB_SLST_NAME "${LIB_CURRENT_NAME}" PARENT_SCOPE)

if(NOT PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES)

  PMBB_setup_lib_common_default()
  target_include_directories(${PROJECT_NAME} PRIVATE ${${THIRDPARTY_SPNG_NAME}_SOURCE_DIR}/src)
  target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_BASE_NAME}_SOURCE_DIR}/src)
  target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_CORE_NAME}_SOURCE_DIR}/src)
  
else() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES

  foreach(MFL ${LIST_MFL})
    PMBB_setup_lib_common_multiarch_per_arch()
    set(LIB_PMBB_CORE_NAME_MFL "${LIB_PMBB_CORE_NAME}_${MFL_CN}")    
    target_include_directories(${PROJECT_NAME} PRIVATE ${${THIRDPARTY_SPNG_NAME}_SOURCE_DIR}/src)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_BASE_NAME}_SOURCE_DIR}/src)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_CORE_NAME_MFL}_SOURCE_DIR}/src)
  endforeach()

  PMBB_setup_lib_common_multiarch_combined()
  
endif() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES