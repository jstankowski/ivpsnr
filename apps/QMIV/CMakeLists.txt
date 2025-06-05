cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

#=========================================================================================================================================
# QMIV - Quality Merics for Immersive Video
#=========================================================================================================================================
set(APP_NAME "IVPSNR")

set(APP_DEPENDENCIES "${LIB_PMBB_BASE_NAME}" "${LIB_PMBB_CORE_NAME}" "${LIB_PMBB_SLST_NAME}" "${LIB_PMBB_IVQM_NAME}")
set(APP_THIRD_PARTY  "${THIRDPARTY_SPNG_NAME}")

if(NOT PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES)
  PMBB_setup_app_common_default()  
else() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES
  if(PMBB_GENERATE_SINGLE_APP_WITH_WITH_RUNTIME_DISPATCH)
    PMBB_setup_app_common_multiarch()
  endif() #PMBB_GENERATE_SINGLE_APP_WITH_WITH_RUNTIME_DISPATCH
  if(PMBB_GENERATE_DEDICATED_APPS_FOR_EVERY_MFL)
    PMBB_setup_app_common_dedicated()
  endif() #PMBB_GENERATE_DEDICATED_APPS_FOR_EVERY_MFL 
endif() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES