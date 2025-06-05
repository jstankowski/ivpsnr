cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

#=========================================================================================================================================
# library PUT-Multimedia-Building-Blocks - CORE
#=========================================================================================================================================
set(LIB_CURRENT_NAME "pmbb-core")
set(LIB_DEPENDENCIES "${LIB_PMBB_BASE_NAME}")

set(LIB_PMBB_CORE_NAME "${LIB_CURRENT_NAME}")
set(LIB_PMBB_CORE_NAME "${LIB_CURRENT_NAME}" PARENT_SCOPE)

if(NOT PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES)  
  PMBB_setup_lib_common_default()
else() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES
  PMBB_setup_lib_common_multiarch()
endif() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES

#=========================================================================================================================================
# Testing can be enabled for non-PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES builds only
#=========================================================================================================================================
if(CMAKE_TESTING_ENABLED AND (NOT PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES))
  set(LIST_TESTS "xColorspace" "xCommon" "xDistortion" "xPixelOps" "xKBNS" "xThreadPool")
  PMBB_setup_lib_test()
endif()