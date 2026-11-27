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
#experimental features
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

#broken features
set(PMBB_ENABLE_STALLED FALSE CACHE BOOL "Enable stalled features")
message(STATUS "PMBB_ENABLE_STALLED      = ${PMBB_ENABLE_STALLED}")
set(PMBB_ENABLE_STALLED_VALUE $<BOOL:${PMBB_ENABLE_STALLED}>)

#broken features
set(PMBB_ENABLE_BROKEN FALSE CACHE BOOL "Enable broken features")
message(STATUS "PMBB_ENABLE_BROKEN       = ${PMBB_ENABLE_BROKEN}")
set(PMBB_ENABLE_BROKEN_VALUE $<BOOL:${PMBB_ENABLE_BROKEN}>)

# set c++ standard requirements
set(CMAKE_CXX_STANDARD_REQUIRED ON ) 
set(CMAKE_CXX_EXTENSIONS        OFF)

# compile everything position independent (even static libraries)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE )

#=========================================================================================================================================
# Compiler
#=========================================================================================================================================
PMBB_ASSIGN_AS_BOOL(PMBB_COMPILER_IS_MSVC         CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
PMBB_ASSIGN_AS_BOOL(PMBB_COMPILER_IS_GCC          CMAKE_CXX_COMPILER_ID MATCHES "GNU" )
PMBB_ASSIGN_AS_BOOL(PMBB_COMPILER_IS_CLANG        ((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")))
PMBB_ASSIGN_AS_BOOL(PMBB_COMPILER_IS_GCC_OR_CLANG (PMBB_COMPILER_IS_GCC OR PMBB_COMPILER_IS_CLANG))

message(STATUS "PMBB_COMPILER_IS_MSVC    = ${PMBB_COMPILER_IS_MSVC}" )
message(STATUS "PMBB_COMPILER_IS_GCC     = ${PMBB_COMPILER_IS_GCC}"  )
message(STATUS "PMBB_COMPILER_IS_CLANG   = ${PMBB_COMPILER_IS_CLANG}")

# mitigate msvc madness
if(PMBB_COMPILER_IS_MSVC)
  #get path to manifest
  set(PMBB_MSVC_MANIFEST_PATH "${CMAKE_CURRENT_LIST_DIR}/msvc/app.manifest")
  #disable - Compiler Warning (level 4) C4324 'structname': structure was padded due to alignment specifier
  add_compile_options(/wd4324) 
  # force static runtime libraries for msvc builds 
  #set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
  add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

#-----------------------------------------------------------------------------------------------------------------------------------------
# set lto
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_is_supported OUTPUT output)
if(lto_is_supported)
  set (CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(PMBB_COMPILER_IS_MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /PROFILE")
elseif(PMBB_COMPILER_IS_GCC_OR_CLANG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

if(PMBB_COMPILER_IS_GCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi -Wno-interference-size")
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
PMBB_ASSIGN_AS_BOOL(PMBB_CPU_ARCH_IS_ARM64 CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$" )
message(STATUS "PMBB_CPU_ARCH_IS_AMD64   = ${PMBB_CPU_ARCH_IS_AMD64}")
message(STATUS "PMBB_CPU_ARCH_IS_ARM64   = ${PMBB_CPU_ARCH_IS_ARM64}")

PMBB_ASSIGN_AS_BOOL(PMBB_OS_IS_WINDOWS     CMAKE_SYSTEM_NAME STREQUAL "Windows")
PMBB_ASSIGN_AS_BOOL(PMBB_OS_IS_LINUX       CMAKE_SYSTEM_NAME STREQUAL "Linux"  )
PMBB_ASSIGN_AS_BOOL(PMBB_OS_IS_ANDROID     CMAKE_SYSTEM_NAME STREQUAL "Android")
PMBB_ASSIGN_AS_BOOL(PMBB_OS_IS_DARWIN      CMAKE_SYSTEM_NAME STREQUAL "Darwin" )
message(STATUS "PMBB_OS_IS_WINDOWS       = ${PMBB_OS_IS_WINDOWS}")
message(STATUS "PMBB_OS_IS_LINUX         = ${PMBB_OS_IS_LINUX}"  )
message(STATUS "PMBB_OS_IS_ANDROID       = ${PMBB_OS_IS_ANDROID}")
message(STATUS "PMBB_OS_IS_DARWIN        = ${PMBB_OS_IS_DARWIN}" )

#=========================================================================================================================================
# Instruction sets & multiarch build
#=========================================================================================================================================
if(PMBB_CPU_ARCH_IS_AMD64 AND (NOT CMAKE_TESTING_ENABLED))
  message(STATUS "${PROJECT_NAME} --> x86-64 Microarchitecture Feature Level (MFL) is available")
  set(PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES TRUE CACHE BOOL "Enable generation of multiarch binnaries"      )
elseif(PMBB_CPU_ARCH_IS_ARM64 AND (PMBB_OS_IS_LINUX) AND (NOT CMAKE_TESTING_ENABLED))
  message(STATUS "${PROJECT_NAME} --> ARM64 Microarchitecture Feature Level (MFL) is available")
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
      if(PMBB_COMPILER_IS_GCC_OR_CLANG)
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
elseif(PMBB_CPU_ARCH_IS_ARM64)
  include(cmake/MicroarchFeatureLevelsARM64.cmake)
  if(PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES)
    set(LIST_MFL "${LIST_MICROARCH_FEATURE_LEVELS_ALL}")
  else()
    if (PMBB_BUILD_WITH_MARCH_NATIVE)
      if(PMBB_COMPILER_IS_GCC_OR_CLANG)
        add_compile_options("-march=native")
      endif()
    endif()
  endif()
else()
  if (PMBB_BUILD_WITH_MARCH_NATIVE)
    if(PMBB_COMPILER_IS_GCC_OR_CLANG)
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
# png read/write library
#=========================================================================================================================================
add_subdirectory(thirdparty/spng    thirdparty-spng)

#=========================================================================================================================================
# library PUT-Multimedia-Building-Blocks
#=========================================================================================================================================
add_subdirectory(libs/pmbb-base    libpmbb-base)
add_subdirectory(libs/pmbb-core    libpmbb-core)
add_subdirectory(libs/pmbb-ivqm    libpmbb-ivqm)
add_subdirectory(libs/pmbb-slst    libpmbb-slst)
add_subdirectory(libs/pmbb-fvgs    libpmbb-fvgs)
add_subdirectory(libs/pmbb-hash    libpmbb-hash)
add_subdirectory(libs/pmbb-gsr     libpmbb-gsr )
add_subdirectory(libs/pmbb-qmau    libpmbb-qmau)
add_subdirectory(libs/pmbb-gsau    libpmbb-gsau)

#=========================================================================================================================================
# apps
#=========================================================================================================================================
add_subdirectory(apps/QMIV       QMIV      )
add_subdirectory(apps/QMIV-GS    QMIV-GS   )
add_subdirectory(apps/QMIV-GSR   QMIV-GSR  )

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
