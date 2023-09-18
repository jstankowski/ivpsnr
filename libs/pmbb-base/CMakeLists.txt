cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

#=========================================================================================================================================
# library PUT-Multimedia-Building-Blocks - Base
#=========================================================================================================================================
set(LIB_PMBB_BASE_NAME "pmbb-base")
set(LIB_PMBB_BASE_NAME "pmbb-base" PARENT_SCOPE)

project("${LIB_PMBB_BASE_NAME}")
add_library(${PROJECT_NAME} OBJECT "")
include(./TargetSources.cmake)
target_compile_features   (${PROJECT_NAME} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME} PRIVATE ${fmtlib_SOURCE_DIR}/include)

#=========================================================================================================================================
# Testing
#=========================================================================================================================================
if(CMAKE_TESTING_ENABLED AND (NOT PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES))

  project (${PROJECT_NAME}-TEST)
  add_executable(${PROJECT_NAME} "")
  target_sources(${PROJECT_NAME} PRIVATE test/test-base.cpp )
  target_compile_features   (${PROJECT_NAME} PRIVATE cxx_std_17)
  target_include_directories(${PROJECT_NAME} PRIVATE ${doctest_SOURCE_DIR})
  target_include_directories(${PROJECT_NAME} PRIVATE ${fmtlib_SOURCE_DIR}/include)
  target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_BASE_NAME}_SOURCE_DIR}/src)
  target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_BASE_NAME} )
  target_link_libraries     (${PROJECT_NAME} PRIVATE fmt::fmt Threads::Threads)
  add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

endif()
