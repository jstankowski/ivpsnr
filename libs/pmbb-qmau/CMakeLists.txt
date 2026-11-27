cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

#=========================================================================================================================================
# library PMBB-QMAU - Poznan-Multimedia-Building-Blocks - Video Quality Metrics Application Utilities
#=========================================================================================================================================
set(LIB_CURRENT_NAME "pmbb-qmau")
set(LIB_DEPENDENCIES "${LIB_PMBB_BASE_NAME}" "${LIB_PMBB_CORE_NAME}" "${LIB_PMBB_SLST_NAME}" "${LIB_PMBB_IVQM_NAME}")

set(LIB_PMBB_QMAU_NAME "${LIB_CURRENT_NAME}")
set(LIB_PMBB_QMAU_NAME "${LIB_CURRENT_NAME}" PARENT_SCOPE)

if(NOT PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES)  
  PMBB_setup_lib_common_default()
else() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES
  PMBB_setup_lib_common_multiarch()
endif() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES

