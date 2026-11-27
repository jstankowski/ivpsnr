#=========================================================================================================================================
# 
#=========================================================================================================================================

if(PMBB_ENABLE_EXPERIMENTAL)
set(LIST_MICROARCH_FEATURE_LEVELS_ALL "ARM64v8p0_SCLR" "ARM64v8p0_AVEC" "ARM64v8p0" "ARM64v8p2" "ARM64v8p2_DPC")
else()
set(LIST_MICROARCH_FEATURE_LEVELS_ALL "ARM64v8p0" "ARM64v8p2")
endif()

list(LENGTH LIST_MICROARCH_FEATURE_LEVELS_ALL NUMOF_MICROARCH_FEATURE_LEVELS_ALL)

#=========================================================================================================================================

function(determine_compile_name_for_MFL COMPILE_NAME MFL)
  set(COMPILE_TAG ${MFL})
  set(${COMPILE_NAME} ${COMPILE_TAG} PARENT_SCOPE)
endfunction() 

#=========================================================================================================================================

function(determine_compiler_settings_for_MFL COMPILE_OPTIONS COMPILE_DEFINITIONS COMPILE_NAME MFL)
  #check if target is AMD64
  if(NOT ((CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64") OR (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")))
    message(SEND_ERROR "Can be applied for x86-64 only (CMAKE_SYSTEM_PROCESSOR=&{CMAKE_SYSTEM_PROCESSOR})")
    return()
  endif()
  
  #check if MFL (Microarchitecture Feature Level) is correct
  if(NOT MFL IN_LIST LIST_MICROARCH_FEATURE_LEVELS_ALL)
    message(SEND_ERROR "Wrong MFL=${MFL}")
    return()
  endif()
  
  #GCC
  if(PMBB_COMPILER_IS_GCC)
    if    (MFL STREQUAL "ARM64v8p0_SCLR")
      set(COMPILE_OPT "-march=armv8-a" "-fno-tree-vectorize" "-fno-tree-loop-vectorize" "-fno-tree-slp-vectorize")
      set(COMPILE_DEF "PMBB_SIMD_ALLOWED=0")
    elseif(MFL STREQUAL "ARM64v8p0_AVEC")
      set(COMPILE_OPT "-march=armv8-a")
      set(COMPILE_DEF "PMBB_SIMD_ALLOWED=0")
    elseif(MFL STREQUAL "ARM64v8p0")
      set(COMPILE_OPT "-march=armv8-a")
      set(COMPILE_DEF "")
    elseif(MFL STREQUAL "ARM64v8p2")
      set(COMPILE_OPT "-march=armv8.2-a")
      set(COMPILE_DEF "")
    elseif(MFL STREQUAL "ARM64v8p2_DPC")
      set(COMPILE_OPT "-march=armv8.2-a+dotprod+crypto")
      set(COMPILE_DEF "")
    endif()
       
  #CLANG
  elseif(PMBB_COMPILER_IS_CLANG)
    if    (MFL STREQUAL "ARM64v8p0_SCLR")
      set(COMPILE_OPT "-march=armv8-a" "-fno-vectorize" "-fno-slp-vectorize")
      set(COMPILE_DEF "PMBB_SIMD_ALLOWED=0")
    elseif(MFL STREQUAL "ARM64v8p0_AVEC")
      set(COMPILE_OPT "-march=armv8-a")
      set(COMPILE_DEF "PMBB_SIMD_ALLOWED=0")
    elseif(MFL STREQUAL "ARM64v8p0")
      set(COMPILE_OPT "-march=armv8-a")
      set(COMPILE_DEF "")
    elseif(MFL STREQUAL "ARM64v8p2")
      set(COMPILE_OPT "-march=armv8.2-a")
      set(COMPILE_DEF "")
    elseif(MFL STREQUAL "ARM64v8p2_DPC")
      set(COMPILE_OPT "-march=armv8.2-a+dotprod+crypto")
      set(COMPILE_DEF "")
    endif()
  
  #END  
  endif()
  
  #convert to nice name
  set(COMPILE_TAG ${MFL})

  set(${COMPILE_OPTIONS}     ${COMPILE_OPT} PARENT_SCOPE)
  set(${COMPILE_DEFINITIONS} ${COMPILE_DEF} PARENT_SCOPE)
  set(${COMPILE_NAME}        ${COMPILE_TAG} PARENT_SCOPE)
endfunction() 

#=========================================================================================================================================
