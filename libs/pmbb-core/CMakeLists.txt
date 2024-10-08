cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

#=========================================================================================================================================
# library PUT-Multimedia-Building-Blocks - CORE
#=========================================================================================================================================
set(LIB_PMBB_CORE_NAME "pmbb-core")
set(LIB_PMBB_CORE_NAME "pmbb-core" PARENT_SCOPE)

if(NOT PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES)
  
  project("${LIB_PMBB_CORE_NAME}")
  add_library(${PROJECT_NAME} OBJECT "")
  include(./TargetSources.cmake)
  target_compile_features   (${PROJECT_NAME} PRIVATE cxx_std_17)
  target_include_directories(${PROJECT_NAME} PRIVATE ${fmtlib_SOURCE_DIR}/include)
  target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_BASE_NAME}_SOURCE_DIR}/src)

else() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES

  foreach(MFL ${LIST_MFL})
    determine_compiler_settings_for_MFL(MFL_CO MFL_CD MFL_CN ${MFL})
    project("${LIB_PMBB_CORE_NAME}_${MFL_CN}")    
    add_library(${PROJECT_NAME} OBJECT "")
    include(./TargetSources.cmake)    
    target_compile_features   (${PROJECT_NAME} PRIVATE cxx_std_17)
    target_compile_options    (${PROJECT_NAME} PRIVATE ${MFL_CO})
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${MFL_CD})
    target_compile_definitions(${PROJECT_NAME} PRIVATE "PMBB_NAMESPACE=PMBB::${MFL_CN}")
    target_include_directories(${PROJECT_NAME} PRIVATE ${fmtlib_SOURCE_DIR}/include)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_BASE_NAME}_SOURCE_DIR}/src)
  endforeach()

  project("${LIB_PMBB_CORE_NAME}")
  add_library(${PROJECT_NAME} INTERFACE)
  foreach(MFL ${LIST_MFL})
    determine_compile_name_for_MFL(MFL_CN ${MFL})
    target_link_libraries(${PROJECT_NAME} INTERFACE ${LIB_PMBB_CORE_NAME}_${MFL_CN} $<TARGET_OBJECTS:${LIB_PMBB_CORE_NAME}_${MFL_CN}>)
  endforeach()
  
endif() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES

#=========================================================================================================================================
# Testing can be enabled for non-PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES builds only
#=========================================================================================================================================
if(CMAKE_TESTING_ENABLED AND (NOT PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES))

  set(LIST_TESTS "xColorspace" "xDistortion" "xPixelOps" "xMathUtils")
  foreach(TEST_NAME ${LIST_TESTS})
    project (${LIB_PMBB_CORE_NAME}-TEST-${TEST_NAME})
    add_executable(${PROJECT_NAME} "")
    target_sources(${PROJECT_NAME} PRIVATE test/test-core-${TEST_NAME}.cpp )
    target_compile_features   (${PROJECT_NAME} PRIVATE cxx_std_17)
    target_include_directories(${PROJECT_NAME} PRIVATE ${doctest_SOURCE_DIR})
    target_include_directories(${PROJECT_NAME} PRIVATE ${fmtlib_SOURCE_DIR}/include)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_BASE_NAME}_SOURCE_DIR}/src)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_CORE_NAME}_SOURCE_DIR}/src)
    target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_CORE_NAME} )
    target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_BASE_NAME} )
    target_link_libraries     (${PROJECT_NAME} PRIVATE fmt::fmt Threads::Threads)
    add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
  endforeach()

endif()