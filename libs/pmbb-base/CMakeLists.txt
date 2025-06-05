cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

#=========================================================================================================================================
# library PUT-Multimedia-Building-Blocks - Base
#=========================================================================================================================================
set(LIB_CURRENT_NAME "pmbb-base")
set(LIB_PMBB_BASE_NAME "${LIB_CURRENT_NAME}")
set(LIB_PMBB_BASE_NAME "${LIB_CURRENT_NAME}" PARENT_SCOPE)

PMBB_setup_lib_common_default()

#=========================================================================================================================================
# Testing
#=========================================================================================================================================
if(CMAKE_TESTING_ENABLED AND (NOT PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES))

  project (${PROJECT_NAME}-TEST)
  add_executable(${PROJECT_NAME} "")
  target_sources(${PROJECT_NAME} PRIVATE test/test-base.cpp )
  target_compile_features   (${PROJECT_NAME} PRIVATE cxx_std_17)
  target_compile_definitions(${PROJECT_NAME} PRIVATE X_PMBB_EXPERIMENTAL=${PMBB_ENABLE_EXPERIMENTAL_VALUE})
  target_include_directories(${PROJECT_NAME} PRIVATE ${doctest_SOURCE_DIR})
  target_include_directories(${PROJECT_NAME} PRIVATE ${fmtlib_SOURCE_DIR}/include)
  target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_BASE_NAME}_SOURCE_DIR}/src)
  target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_BASE_NAME} )
  target_link_libraries     (${PROJECT_NAME} PRIVATE fmt::fmt Threads::Threads)
  add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

endif()
