cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
set(PROJECT_NAME "IVPSNR-SoftwareX")
project(${PROJECT_NAME} CXX)

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
# force static runtime libraries for msvc builds
if(MSVC)
  set(variables CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
  foreach(variable ${variables})
    if(${variable} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
    endif()
  endforeach()
endif()

# set c++17
set(CMAKE_CXX_STANDARD          17 )
set(CMAKE_CXX_STANDARD_REQUIRED ON ) 
set(CMAKE_CXX_EXTENSIONS        OFF)

# compile everything position independent (even static libraries)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE )

# set gcc tools
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  #message(STATUS "CMAKE_CXX_COMPILER_VERSION = ${CMAKE_CXX_COMPILER_VERSION}")
  string(REGEX REPLACE "^([0-9]+)\.[0-9]+\.[0-9]+$" "\\1" GCC_VERSION_MAJOR ${CMAKE_CXX_COMPILER_VERSION})
  SET (CMAKE_AR      "/usr/bin/gcc-ar-${GCC_VERSION_MAJOR}")
  #set(CMAKE_STATIC_LINKER_FLAGS "rcsT")
  #SET(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> rcsT <TARGET> <LINK_FLAGS> <OBJECTS>")
  #SET(CMAKE_CXX_ARCHIVE_FINISH   true)
endif()

# set clang tools
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  #message(STATUS "CMAKE_CXX_COMPILER_VERSION = ${CMAKE_CXX_COMPILER_VERSION}")
  string(REGEX REPLACE "^([0-9]+)\.[0-9]+\.[0-9]+$" "\\1" CLANG_VERSION_MAJOR ${CMAKE_CXX_COMPILER_VERSION})
  SET (CMAKE_AR      "/usr/bin/llvm-ar-${CLANG_VERSION_MAJOR}")
  SET (CMAKE_LINKER  "/usr/bin/clang++-${CLANG_VERSION_MAJOR}")
  SET (CMAKE_NM      "/usr/bin/llvm-nm-${CLANG_VERSION_MAJOR}")
  SET (CMAKE_OBJDUMP "/usr/bin/llvm-objdump-${CLANG_VERSION_MAJOR}")
  SET (CMAKE_RANLIB  "/usr/bin/llvm-ranlib-${CLANG_VERSION_MAJOR}")
endif()

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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -mtune=generic")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")
endif()

#=========================================================================================================================================
# Architecture
#=========================================================================================================================================
message(STATUS "CMAKE_GENERATOR          = ${CMAKE_GENERATOR}")
message(STATUS "CMAKE_GENERATOR_PLATFORM = ${CMAKE_GENERATOR_PLATFORM}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR   = ${CMAKE_SYSTEM_PROCESSOR}")

if((CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64") OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64"))
  set (X_IS_AMD64 TRUE)
else()
  set(X_IS_AMD64 FALSE)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set (X_IS_64BIT TRUE)
else()
  set(X_IS_64BIT FALSE)
endif()

#=========================================================================================================================================
# Instruction sets & multiarch build
#=========================================================================================================================================
if(X_IS_AMD64 AND X_IS_64BIT AND (NOT CMAKE_TESTING_ENABLED))
  message(STATUS "${PROJECT_NAME} --> x86-64 Microarchitecture Feature Level (MFL) is available")
  set(PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES FALSE CACHE BOOL "Enable generation of multiarch binnaries"      )
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

if(X_IS_AMD64 AND X_IS_64BIT)
  include(cmake/MicroarchFeatureLevels.cmake)
  if(PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES)
    set(LIST_MFL "${LIST_MICROARCH_FEATURE_LEVELS_ALL}")
  else()
    if (PMBB_BUILD_WITH_MARCH_NATIVE)
      if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
        add_compile_options("-march=native")
      endif()
    else()
      determine_compiler_settings_for_MFL(MFL_CO MFL_CD MFL_CN "x86-64-v2")
      message(STATUS "MFL_CO = ${MFL_CO}")
      message(STATUS "MFL_CD = ${MFL_CD}")
      add_compile_options    (${MFL_CO})
      add_compile_definitions(${MFL_CD})
    endif()
  endif()
endif()

if(PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES)
  message(STATUS "${PROJECT_NAME} --> x86-64 Microarchitecture Feature Level (MFL) related parameters")
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
# library PUT-Multimedia-Building-Blocks
#=========================================================================================================================================
add_subdirectory(libs/pmbb-base libpmbb-base)
add_subdirectory(libs/pmbb-core libpmbb-core)
add_subdirectory(libs/pmbb-ivqm libpmbb-ivqm)

#=========================================================================================================================================
# apps
#=========================================================================================================================================
add_subdirectory(apps/IVPSNR IVPSNR)

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
