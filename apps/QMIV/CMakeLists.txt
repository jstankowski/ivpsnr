cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

#=========================================================================================================================================
# QMIV - Quality Merics for Immersive Video
#=========================================================================================================================================
set(QMIV_NAME "IVPSNR")

if(NOT PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES)

  project("${QMIV_NAME}")

  add_executable(${PROJECT_NAME} "")
  include(./TargetSources.cmake)
  source_group("Source Files" FILES ${PROJECT_SOURCES})

  target_compile_features   (${PROJECT_NAME} PRIVATE cxx_std_17)
  target_include_directories(${PROJECT_NAME} PRIVATE ${fmtlib_SOURCE_DIR}/include)
  target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_BASE_NAME}_SOURCE_DIR}/src)
  target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_CORE_NAME}_SOURCE_DIR}/src)
  target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_SPNG_NAME}_SOURCE_DIR}/src)
  target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_IVQM_NAME}_SOURCE_DIR}/src)
  target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_IVQM_NAME} )
  target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_SPNG_NAME} )
  target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_CORE_NAME} )
  target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_BASE_NAME} )
  target_link_libraries     (${PROJECT_NAME} PRIVATE ${THIRDPARTY_SPNG_NAME} )  
  target_link_libraries     (${PROJECT_NAME} PRIVATE fmt::fmt Threads::Threads)
  
else() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES

  if(PMBB_GENERATE_SINGLE_APP_WITH_WITH_RUNTIME_DISPATCH)
    foreach(MFL ${LIST_MFL})
      determine_compiler_settings_for_MFL(MFL_CO MFL_CD MFL_CN ${MFL})
      project("${QMIV_NAME}_MAIN_${MFL_CN}") 
        
      add_library(${PROJECT_NAME} OBJECT "")
      include(./TargetSources.cmake)
      source_group("Source Files" FILES ${PROJECT_SOURCES})
      
      target_compile_features   (${PROJECT_NAME} PRIVATE cxx_std_17)
      target_compile_options    (${PROJECT_NAME} PRIVATE ${MFL_CO})
      target_compile_definitions(${PROJECT_NAME} PRIVATE ${MFL_CD})
      target_compile_definitions(${PROJECT_NAME} PRIVATE "PMBB_NAMESPACE=PMBB::${MFL_CN}")
      target_compile_definitions(${PROJECT_NAME} PRIVATE "APP_MAIN=main_${MFL_CN}")
      target_include_directories(${PROJECT_NAME} PRIVATE ${fmtlib_SOURCE_DIR}/include)
      target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_BASE_NAME}_SOURCE_DIR}/src)
      target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_CORE_NAME}_SOURCE_DIR}/src)
      target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_SPNG_NAME}_SOURCE_DIR}/src)
      target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_IVQM_NAME}_SOURCE_DIR}/src)
    endforeach()

    project("${QMIV_NAME}") 
    add_executable(${PROJECT_NAME} "")
    target_sources(${PROJECT_NAME} PRIVATE src/main_Dispatch.cpp)
    target_compile_features   (${PROJECT_NAME} PRIVATE cxx_std_17)
    target_include_directories(${PROJECT_NAME} PRIVATE ${fmtlib_SOURCE_DIR}/include)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_BASE_NAME}_SOURCE_DIR}/src)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_CORE_NAME}_SOURCE_DIR}/src)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_SPNG_NAME}_SOURCE_DIR}/src)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_IVQM_NAME}_SOURCE_DIR}/src)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_DISPATCH_NAME}_SOURCE_DIR}/src)
    target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_IVQM_NAME} )
    target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_SPNG_NAME} )
    target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_CORE_NAME} )
    target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_BASE_NAME} )
    target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_DISPATCH_NAME} )
    foreach(MFL ${LIST_MFL})
      determine_compiler_settings_for_MFL(MFL_CO MFL_CD MFL_CN ${MFL})
      target_compile_definitions(${PROJECT_NAME} PRIVATE "BUILD_WITH_${MFL_CN}")
      target_link_libraries     (${PROJECT_NAME} PRIVATE "${QMIV_NAME}_MAIN_${MFL_CN}" )
    endforeach()
    target_link_libraries     (${PROJECT_NAME} PRIVATE ${THIRDPARTY_SPNG_NAME} )  
    target_link_libraries     (${PROJECT_NAME} PRIVATE fmt::fmt Threads::Threads)
  endif() #PMBB_GENERATE_SINGLE_APP_WITH_WITH_RUNTIME_DISPATCH

  if(PMBB_GENERATE_DEDICATED_APPS_FOR_EVERY_MFL)
    foreach(MFL ${LIST_MFL})
      determine_compiler_settings_for_MFL(MFL_CO MFL_CD MFL_CN ${MFL})
      project("${QMIV_NAME}_${MFL_CN}") 
      
      add_executable(${PROJECT_NAME} "")
      include(./TargetSources.cmake)
      source_group("Source Files" FILES ${PROJECT_SOURCES})
    
      target_compile_features   (${PROJECT_NAME} PRIVATE cxx_std_17)
      target_compile_options    (${PROJECT_NAME} PRIVATE ${MFL_CO})
      target_compile_definitions(${PROJECT_NAME} PRIVATE ${MFL_CD})
      target_compile_definitions(${PROJECT_NAME} PRIVATE "PMBB_NAMESPACE=PMBB::${MFL_CN}")
      target_include_directories(${PROJECT_NAME} PRIVATE ${fmtlib_SOURCE_DIR}/include)
      target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_BASE_NAME}_SOURCE_DIR}/src)
      target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_CORE_NAME}_SOURCE_DIR}/src)
      target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_SPNG_NAME}_SOURCE_DIR}/src)
      target_include_directories(${PROJECT_NAME} PRIVATE ${${LIB_PMBB_IVQM_NAME}_SOURCE_DIR}/src)
      target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_IVQM_NAME}_${MFL_CN} )
      target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_SPNG_NAME}_${MFL_CN} )
      target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_CORE_NAME}_${MFL_CN} )
      target_link_libraries     (${PROJECT_NAME} PRIVATE ${LIB_PMBB_BASE_NAME} )
      target_link_libraries     (${PROJECT_NAME} PRIVATE ${THIRDPARTY_SPNG_NAME} )  
      target_link_libraries     (${PROJECT_NAME} PRIVATE fmt::fmt Threads::Threads)
    endforeach()
  endif() #PMBB_GENERATE_DEDICATED_APPS_FOR_EVERY_MFL
  
endif() #PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES