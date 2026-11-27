cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

#=========================================================================================================================================
# library PMBB-GSAU - Poznan-Multimedia-Building-Blocks - Gaussian Splatting Application Utilities
#=========================================================================================================================================
set(LIB_CURRENT_NAME "pmbb-gsau")
set(LIB_DEPENDENCIES "${LIB_PMBB_BASE_NAME}" "${LIB_PMBB_CORE_NAME}" "${LIB_PMBB_FVGS_NAME}" "${LIB_PMBB_GSR_NAME}" "${LIB_PMBB_HASH_NAME}")

set(LIB_PMBB_GSAU_NAME "${LIB_CURRENT_NAME}")
set(LIB_PMBB_GSAU_NAME "${LIB_CURRENT_NAME}" PARENT_SCOPE)

if(NOT PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES)  
  PMBB_setup_lib_common_default()
else() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES
  PMBB_setup_lib_common_multiarch()
endif() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES

