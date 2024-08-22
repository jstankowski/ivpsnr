cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

#=========================================================================================================================================
# library - spng
#=========================================================================================================================================
set(THIRDPARTY_SPNG_NAME "thirdparty-spng")
set(THIRDPARTY_SPNG_NAME "thirdparty-spng" PARENT_SCOPE)

project(${THIRDPARTY_SPNG_NAME})
add_library(${PROJECT_NAME} OBJECT "")
include(./TargetSources.cmake)
target_compile_features   (${PROJECT_NAME} PRIVATE cxx_std_17)

