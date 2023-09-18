#=========================================================================================================================================
# 
#=========================================================================================================================================

set(LIST_MICROARCH_FEATURE_LEVELS_ALL "x86-64" "x86-64-v2" "x86-64-v3" "x86-64-v4")
list(LENGTH LIST_MICROARCH_FEATURE_LEVELS_ALL NUMOF_MICROARCH_FEATURE_LEVELS_ALL)  

#=========================================================================================================================================

function(determine_compile_name_for_MFL COMPILE_NAME MFL)
  #conver to nice name
  if(MFL STREQUAL "x86-64")
    set(COMPILE_TAG "AMD64v1")
  else()  
    string(REPLACE "x86-64-" "AMD64" COMPILE_TAG ${MFL})
  endif()
  
  set(${COMPILE_NAME} ${COMPILE_TAG} PARENT_SCOPE)
endfunction() 

#=========================================================================================================================================

function(determine_compiler_settings_for_MFL COMPILE_OPTIONS COMPILE_DEFINITIONS COMPILE_NAME MFL)
  #check if target is AMD64
  if(NOT ((CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64") OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")))
    message(SEND_ERROR "Can be applied for x86-64 only (CMAKE_SYSTEM_PROCESSOR=&{CMAKE_SYSTEM_PROCESSOR})")
    return()
  endif()
  
  #check if MFL (Microarchitecture Feature Level) is correct
  if(NOTMFL IN_LIST LIST_MICROARCH_FEATURE_LEVELS_ALL)
    message(SEND_ERROR "Wrong MFL=${MFL}")
    return()
  endif()
  
  #GCC >= 11.0
  if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "11.0"))
    set(COMPILE_OPT "-march=${MFL}")
    set(COMPILE_DEF "")
   
  #CLANG >= 12.0
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "12.0"))
    set(COMPILE_OPT "-march=${MFL}")
    set(COMPILE_DEF "")
  
  #older GCC or CLANG
  elseif((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
    if    (MFL STREQUAL "x86-64"   )
      set(COMPILE_OPT "")
    elseif(MFL STREQUAL "x86-64-v2")
      set(COMPILE_OPT "-march=nehalem")
    elseif(MFL STREQUAL "x86-64-v3")
      set(COMPILE_OPT "-march=haswell")
    elseif(MFL STREQUAL "x86-64-v4")
      set(COMPILE_OPT "-march=skylake-avx512")
    endif()
    set(COMPILE_DEF "")
  
  #MSVC
  elseif(MSVC)
    if    (MFL STREQUAL "x86-64"   )
      set(COMPILE_OPT "")
    elseif(MFL STREQUAL "x86-64-v2")
      set(COMPILE_OPT "")
    elseif(MFL STREQUAL "x86-64-v3")
      set(COMPILE_OPT "/arch:AVX2")
    elseif(MFL STREQUAL "x86-64-v4")
      set(COMPILE_OPT "/arch:AVX512")
    endif()
    
    if    (MFL STREQUAL "x86-64"   )
      set(COMPILE_DEF "__SSE__" "__SSE2__")
    elseif(MFL STREQUAL "x86-64-v2")
      set(COMPILE_DEF "__SSE__" "__SSE2__" "__SSE3__" "__SSSE3__" "__SSE4_1__" "__SSE4_2__")
    elseif(MFL STREQUAL "x86-64-v3")
      set(COMPILE_DEF "__SSE__" "__SSE2__" "__SSE3__" "__SSSE3__" "__SSE4_1__" "__SSE4_2__")
    elseif(MFL STREQUAL "x86-64-v4")
      set(COMPILE_DEF "__SSE__" "__SSE2__" "__SSE3__" "__SSSE3__" "__SSE4_1__" "__SSE4_2__")
    endif()
  
  #END  
  endif()
  
  #conver to nice name
  if(MFL STREQUAL "x86-64")
    set(COMPILE_TAG "AMD64v1")
  else()  
    string(REPLACE "x86-64-" "AMD64" COMPILE_TAG ${MFL})
  endif()

  set(${COMPILE_OPTIONS}     ${COMPILE_OPT} PARENT_SCOPE)
  set(${COMPILE_DEFINITIONS} ${COMPILE_DEF} PARENT_SCOPE)
  set(${COMPILE_NAME}        ${COMPILE_TAG} PARENT_SCOPE)
endfunction() 

#=========================================================================================================================================

function(determine_compiler_settings_lists LIST_COMPILE_OPTIONS LIST_COMPILE_DEFINITIONS LIST_MICROARCH_FEATURE_LEVELS_SELECTED)
  message(STATUS "LIST_MICROARCH_FEATURE_LEVELS_SELECTED = ${LIST_MICROARCH_FEATURE_LEVELS_SELECTED}")
  list(LENGTH LIST_MICROARCH_FEATURE_LEVELS_SELECTED NUMOF_MICROARCH_FEATURE_LEVELS_SELECTED)  

  math(EXPR LOOP_RANGE_STOP ${NUMOF_MICROARCH_FEATURE_LEVELS_SELECTED}-1)
  foreach(MFL_IDX RANGE ${LOOP_RANGE_STOP})
    #message(STATUS "MFL_IDX = ${MFL_IDX}")
    list(GET LIST_MICROARCH_FEATURE_LEVELS_SELECTED ${MFL_IDX} MFL)
    #message(STATUS "  MFL = ${MFL}")
    determine_compiler_settings_for_MFL(MFL_CO MFL_CD ${MFL})
    #message(STATUS "  MFL_CO = ${MFL_CO}")
    #message(STATUS "  MFL_CD = ${MFL_CD}")
    list(APPEND LIST_MFL_CO ${MFL_CO})
    list(APPEND LIST_MFL_DO ${MFL_CD})    
  endforeach()
  #message(STATUS "LIST_MFL_CO = ${LIST_MFL_CO}")
  #message(STATUS "LIST_MFL_DO = ${LIST_MFL_DO}")
  set(${LIST_COMPILE_OPTIONS}     ${LIST_MFL_CO} PARENT_SCOPE)
  set(${LIST_COMPILE_DEFINITIONS} ${LIST_MFL_DO} PARENT_SCOPE)

endfunction() 