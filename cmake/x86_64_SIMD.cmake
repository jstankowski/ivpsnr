include(CheckCXXSourceRuns)
include(CheckCXXSourceCompiles)

function (detect_host_x86_64_MFL HOST_x86_64_MFL)

  #=========================================================================================================================================
  # Check for AVX512 support
  if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
    set(CMAKE_REQUIRED_FLAGS "-skylake-avx512")
  elseif(MSVC)
    set(CMAKE_REQUIRED_FLAGS "/arch:AVX512")
  endif()

  check_cxx_source_runs("
    #include <immintrin.h>
    int main()
    {
      __m512i a = _mm512_set_epi32 (-1, 2, -3, 4, -1, 2, -3, 4, 13, -5, 6, -7, 9, 2, -6, 3);
      __m512i result = _mm512_abs_epi32 (a);
      return 0;
    }" 
    DETECTED_AVX512)
    #message(STATUS "DETECTED_AVX512    = ${DETECTED_AVX512}")

  if(${DETECTED_AVX512})
    set(${HOST_x86_64_MFL} "x86-64-v4" PARENT_SCOPE)
    return()
  endif()  

  #=========================================================================================================================================
  # Check for AVX2 support
  if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
    set(CMAKE_REQUIRED_FLAGS "-mavx2")
  elseif(MSVC)
    set(CMAKE_REQUIRED_FLAGS "/arch:AVX2")
  endif()

  check_cxx_source_runs("
    #include <immintrin.h>
    int main()
    {
    __m256i a = _mm256_set_epi32 (-1, 2, -3, 4, -1, 2, -3, 4);
    __m256i result = _mm256_abs_epi32 (a);
    return 0;
    }" 
    DETECTED_AVX2)
    #message(STATUS "DETECTED_AVX2    = ${DETECTED_AVX2}")

  if(${DETECTED_AVX2})
    set(${HOST_x86_64_MFL} "x86-64-v3" PARENT_SCOPE)
    return()
  endif()  

  #=========================================================================================================================================
  # Check for SSE4.2 support
  if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
    set(CMAKE_REQUIRED_FLAGS "-msse4.2")
  elseif(MSVC)
    set(CMAKE_REQUIRED_FLAGS "/arch:SSE4.2")
  endif()

  check_cxx_source_runs("
    #include <emmintrin.h>
    #include <nmmintrin.h>
    int main()
    {
    long long a[2] = {  1, 2 };
    long long b[2] = { -1, 3 };
    long long c[2];
    __m128i va = _mm_loadu_si128((__m128i*)a);
    __m128i vb = _mm_loadu_si128((__m128i*)b);
    __m128i vc = _mm_cmpgt_epi64(va, vb);

    _mm_storeu_si128((__m128i*)c, vc);
    if (c[0] == -1LL && c[1] == 0LL) { return 0; }
    else                             { return 1; }
    }" 
    DETECTED_SSE4_2)
    #message(STATUS "DETECTED_SSE4_2    = ${DETECTED_SSE4_2}")

  if(${DETECTED_SSE4_2})
    set(${HOST_x86_64_MFL} "x86-64-v2" PARENT_SCOPE)
    return()
  endif()

  #=========================================================================================================================================
  # Check for SSE2 support

  if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
    set(CMAKE_REQUIRED_FLAGS "-msse2")
  elseif(MSVC)
    set(CMAKE_REQUIRED_FLAGS "/arch:SSE2")
  endif()

  check_cxx_source_runs("
    #include <emmintrin.h>
    #include <nmmintrin.h>
    int main()
    {
      _mm_setzero_si128();
      return 0;
    }" 
    DETECTED_SSE2)
    #message(STATUS "DETECTED_SSE2    = ${DETECTED_SSE2}")

  if(${DETECTED_SSE2})
    set(${HOST_x86_64_MFL} "x86-64" PARENT_SCOPE)
    return()
  endif()

  set(${HOST_x86_64_MFL} "" PARENT_SCOPE)

endfunction()


