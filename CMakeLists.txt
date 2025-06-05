cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
set(PROJECT_NAME "IVPSNR-SoftwareX")
project(${PROJECT_NAME} CXX)

include(cmake/macros.cmake)

#=========================================================================================================================================
# set verbose compile options
#=========================================================================================================================================
#set(CMAKE_VERBOSE_MAKEFILE ON)

#=========================================================================================================================================
# Testing
#=========================================================================================================================================
set(PMBB_GENERATE_TESTING FALSE  CACHE BOOL "Enable of testing binaries")
if(PMBB_GENERATE_TESTING)
  enable_testing()
endif()

message(STATUS "PMBB_GENERATE_TESTING    = ${PMBB_GENERATE_TESTING}")
message(STATUS "CMAKE_TESTING_ENABLED    = ${CMAKE_TESTING_ENABLED}")

if (CMAKE_TESTING_ENABLED)
  message(STATUS "${PROJECT_NAME} --> BUILD_TESTING is enabled")  
  message(STATUS "${PROJECT_NAME} --> BUILD_TESTING disables PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES feature")
  if (NOT PMBB_BUILD_WITH_MARCH_NATIVE)
    message(STATUS "${PROJECT_NAME} --> BUILD_TESTING should be used with PMBB_BUILD_WITH_MARCH_NATIVE option")
  endif()
endif()

#=========================================================================================================================================
# Setup
#=========================================================================================================================================
set(PMBB_ENABLE_EXPERIMENTAL FALSE CACHE BOOL "Enable experimental features")
message(STATUS "PMBB_ENABLE_EXPERIMENTAL = ${PMBB_ENABLE_EXPERIMENTAL}")
set(PMBB_ENABLE_EXPERIMENTAL_VALUE $<BOOL:${PMBB_ENABLE_EXPERIMENTAL}>)

if(PMBB_ENABLE_EXPERIMENTAL AND FALSE)
# detect c++20 or c++17
if    ("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  set(CMAKE_CXX_STANDARD 20)
elseif("cxx_std_17" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  set(CMAKE_CXX_STANDARD 17)
endif ()
else()
  set(CMAKE_CXX_STANDARD 17)
endif()

# set c++ standard requirements
set(CMAKE_CXX_STANDARD_REQUIRED ON ) 
set(CMAKE_CXX_EXTENSIONS        OFF)

# compile everything position independent (even static libraries)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE )

# force static runtime libraries for msvc builds 
if(MSVC)
  #set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  #add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
  #add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

#-----------------------------------------------------------------------------------------------------------------------------------------
# set lto
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_is_supported OUTPUT output)
if(lto_is_supported)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=auto")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
  else()
    set (CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endif()

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /PROFILE")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")
endif()

#=========================================================================================================================================
# Target architecture & operating system
#=========================================================================================================================================
message(STATUS "CMAKE_GENERATOR          = ${CMAKE_GENERATOR}")
message(STATUS "CMAKE_GENERATOR_PLATFORM = ${CMAKE_GENERATOR_PLATFORM}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR   = ${CMAKE_SYSTEM_PROCESSOR}")

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "Requires 64 bit CPU" )
endif()

PMBB_ASSIGN_AS_BOOL(PMBB_CPU_ARCH_IS_AMD64 CMAKE_SYSTEM_PROCESSOR MATCHES "^(AMD64|x86_64)$"  )
PMBB_ASSIGN_AS_BOOL(PMBB_OS_IS_WINDOWS     CMAKE_SYSTEM_NAME STREQUAL "Windows")
PMBB_ASSIGN_AS_BOOL(PMBB_OS_IS_LINUX       CMAKE_SYSTEM_NAME STREQUAL "Linux"  )

message(STATUS "PMBB_CPU_ARCH_IS_AMD64   = ${PMBB_CPU_ARCH_IS_AMD64}")
message(STATUS "PMBB_OS_IS_WINDOWS       = ${PMBB_OS_IS_WINDOWS}")
message(STATUS "PMBB_OS_IS_LINUX         = ${PMBB_OS_IS_LINUX}")

#=========================================================================================================================================
# Instruction sets & multiarch build
#=========================================================================================================================================
if(PMBB_CPU_ARCH_IS_AMD64 AND (NOT CMAKE_TESTING_ENABLED))
  message(STATUS "${PROJECT_NAME} --> x86-64 Microarchitecture Feature Level (MFL) is available")
  set(PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES TRUE CACHE BOOL "Enable generation of multiarch binnaries"      )
else()
  set(PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES FALSE CACHE BOOL "Enable generation of multiarch binnaries" FORCE)
endif()

if (PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES)
  set(PMBB_GENERATE_SINGLE_APP_WITH_WITH_RUNTIME_DISPATCH  TRUE  CACHE BOOL "")
  set(PMBB_GENERATE_DEDICATED_APPS_FOR_EVERY_MFL           FALSE CACHE BOOL "")
else()
  set(PMBB_GENERATE_SINGLE_APP_WITH_WITH_RUNTIME_DISPATCH  FALSE CACHE BOOL "" FORCE)
  set(PMBB_GENERATE_DEDICATED_APPS_FOR_EVERY_MFL           FALSE CACHE BOOL "" FORCE)
endif()

message(STATUS "${PROJECT_NAME} --> PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES         = ${PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES}")
message(STATUS "${PROJECT_NAME} --> PMBB_GENERATE_SINGLE_APP_WITH_WITH_RUNTIME_DISPATCH  = ${PMBB_GENERATE_SINGLE_APP_WITH_WITH_RUNTIME_DISPATCH}")
message(STATUS "${PROJECT_NAME} --> PMBB_GENERATE_DEDICATED_APPS_FOR_EVERY_MFL           = ${PMBB_GENERATE_DEDICATED_APPS_FOR_EVERY_MFL}")

if(PMBB_CPU_ARCH_IS_AMD64)
  include(cmake/MicroarchFeatureLevels.cmake)
  if(PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES)
    set(LIST_MFL "${LIST_MICROARCH_FEATURE_LEVELS_ALL}")
  else()
    if (PMBB_BUILD_WITH_MARCH_NATIVE)
      if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
        add_compile_options("-march=native")
      endif()
      if(MSVC)
        include(cmake/x86_64_SIMD.cmake)
        detect_host_x86_64_MFL(HOST_x86_64_MFL)
        message(STATUS "HOST_x86_64_MFL  --> ${HOST_x86_64_MFL}")
        determine_compiler_settings_for_MFL(MFL_CO MFL_CD MFL_CN ${HOST_x86_64_MFL})
        message(STATUS "MFL_CO = ${MFL_CO}")
        message(STATUS "MFL_CD = ${MFL_CD}")
        add_compile_options    (${MFL_CO})
        add_compile_definitions(${MFL_CD})
      endif()
    else()
      determine_compiler_settings_for_MFL(MFL_CO MFL_CD MFL_CN "x86-64-v2")
      message(STATUS "MFL_CO = ${MFL_CO}")
      message(STATUS "MFL_CD = ${MFL_CD}")
      add_compile_options    (${MFL_CO})
      add_compile_definitions(${MFL_CD})
    endif()
  endif()
else()
  if (PMBB_BUILD_WITH_MARCH_NATIVE)
    if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
      add_compile_options("-march=native")
    endif()
  endif()
endif()

if(PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES)
  message(STATUS "${PROJECT_NAME} --> Microarchitecture Feature Level (MFL) related parameters")
  foreach(MFL ${LIST_MFL})
    determine_compiler_settings_for_MFL(MFL_CO MFL_CD MFL_CN ${MFL})
    message(STATUS "  MFL=${MFL} NC=${MFL_CN} CO=[${MFL_CO}] CD=[${MFL_CD}]")
  endforeach()
endif()

#=========================================================================================================================================
# info
#=========================================================================================================================================
message(STATUS "CMAKE_CXX_COMPILER         = ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER_ID      = ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER_VERSION = ${CMAKE_CXX_COMPILER_VERSION}") 
message(STATUS "CMAKE_NM                   = ${CMAKE_NM}")
message(STATUS "CMAKE_AR                   = ${CMAKE_AR}")
message(STATUS "CMAKE_LINKER               = ${CMAKE_LINKER}")
message(STATUS "CMAKE_C_FLAGS              = ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS            = ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_STATIC_LINKER_FLAGS  = ${CMAKE_STATIC_LINKER_FLAGS}")

#=========================================================================================================================================
# threads library
#=========================================================================================================================================
find_package (Threads REQUIRED)

#=========================================================================================================================================
# formatting library {fmt} (https://github.com/fmtlib/fmt)
#=========================================================================================================================================
include(cmake/fmtlib.cmake)

#=========================================================================================================================================
# unit testing framework for C++ (https://github.com/doctest/doctest)
#=========================================================================================================================================
if(PMBB_GENERATE_TESTING)
  include(cmake/doctest.cmake)
endif()

#=========================================================================================================================================
# png read/write library SPNG ()
#=========================================================================================================================================
add_subdirectory(thirdparty/spng    thirdparty-spng)

#=========================================================================================================================================
# library PUT-Multimedia-Building-Blocks
#=========================================================================================================================================
add_subdirectory(libs/pmbb-base    libpmbb-base)
add_subdirectory(libs/pmbb-core    libpmbb-core)
add_subdirectory(libs/pmbb-ivqm    libpmbb-ivqm)
add_subdirectory(libs/pmbb-slst    libpmbb-slst)
#=========================================================================================================================================
# apps
#=========================================================================================================================================
add_subdirectory(apps/QMIV IVPSNR)

#=========================================================================================================================================
# debug
#=========================================================================================================================================
if(0)
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
  message(STATUS "${_variableName}=${${_variableName}}")
endforeach()
endif()

#=========================================================================================================================================
