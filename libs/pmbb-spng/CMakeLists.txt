cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

#=========================================================================================================================================
# library PUT-Multimedia-Building-Blocks - sequence png utils
#=========================================================================================================================================
set(LIB_PMBB_SPNG_NAME "pmbb-spng")
set(LIB_PMBB_SPNG_NAME "pmbb-spng" PARENT_SCOPE)

if(NOT PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES)

  project(${LIB_PMBB_SPNG_NAME})
  add_library(${PROJECT_NAME} OBJECT "")
  include(./TargetSources.cmake)
  target_compile_features   (${PROJECT_NAME} PRIVATE cxx_std_17)
  target_include_directories(${PROJECT_NAME} PRIVATE ${fmtlib_SOURCE_DIR}/include)
  target_include_directories(${PROJECT_NAME} PRIVATE ${${THIRDPARTY_SPNG_NAME}_SOURCE_DIR}/src)
  target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_BASE_NAME}_SOURCE_DIR}/src)
  target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_CORE_NAME}_SOURCE_DIR}/src)
  
else() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES

  foreach(MFL ${LIST_MFL})
    determine_compiler_settings_for_MFL(MFL_CO MFL_CD MFL_CN ${MFL})    
    project("${LIB_PMBB_SPNG_NAME}_${MFL_CN}")
    add_library(${PROJECT_NAME} "")
    include(./TargetSources.cmake)
    set(LIB_PMBB_CORE_NAME_MFL "${LIB_PMBB_CORE_NAME}_${MFL_CN}")    
    target_compile_features   (${PROJECT_NAME} PRIVATE cxx_std_17)
    target_compile_options    (${PROJECT_NAME} PRIVATE ${MFL_CO})
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${MFL_CD})
    target_compile_definitions(${PROJECT_NAME} PRIVATE "PMBB_NAMESPACE=PMBB::${MFL_CN}")
    target_include_directories(${PROJECT_NAME} PRIVATE ${fmtlib_SOURCE_DIR}/include)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${THIRDPARTY_SPNG_NAME}_SOURCE_DIR}/src)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_BASE_NAME}_SOURCE_DIR}/src)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_CORE_NAME_MFL}_SOURCE_DIR}/src)
  endforeach()

  project(${LIB_PMBB_SPNG_NAME})
  add_library(${PROJECT_NAME} INTERFACE)
  foreach(MFL ${LIST_MFL})
    determine_compile_name_for_MFL(MFL_CN ${MFL})
    target_link_libraries(${PROJECT_NAME} INTERFACE ${LIB_PMBB_SPNG_NAME}_${MFL_CN} $<TARGET_OBJECTS:${LIB_PMBB_SPNG_NAME}_${MFL_CN}>)
  endforeach()
  
endif() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES