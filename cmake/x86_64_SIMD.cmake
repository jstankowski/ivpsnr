include(CheckCXXSourceRuns)
include(CheckCXXSourceCompiles)

function (detect_host_x86_64_MFL HOST_x86_64_MFL)

  #=========================================================================================================================================
  # Check for znver4 support 
  if(PMBB_ENABLE_EXPERIMENTAL)
  check_cxx_source_runs([=[
    #include <iostream>
    #include <vector>
    #include <string>
    #include <cstring>
    #include <cstdint>

    #if defined(__GNUC__) || defined(__clang__)
    #include <cpuid.h>
    #elif defined(_MSC_VER)
    #include <intrin.h>
    #endif

    void xCPUID(uint32_t RegistersTable[4], uint32_t Leaf, uint32_t SubLeaf=0)
    {
    #if defined(__GNUC__) || defined(__clang__)    
      __get_cpuid_count(Leaf, SubLeaf, RegistersTable + c_RegEAX, RegistersTable + c_RegEBX, RegistersTable + c_RegECX, RegistersTable + c_RegEDX);
    #elif defined(_MSC_VER)
      __cpuidex((int*)RegistersTable, Leaf, SubLeaf);
    #else
      #error "Unknown compiler"
    #endif
    }

    int main() 
    {      
      uint32_t REG[4];

      //Check vendor
      xCPUID(REG, 0);
      char Vendor[13] = { 0};
      std::memcpy(&Vendor[0], &REG[1], 4);
      std::memcpy(&Vendor[4], &REG[3], 4);
      std::memcpy(&Vendor[8], &REG[2], 4);
      std::cout << "Vendor " << Vendor << std::endl;
      if (std::string(Vendor) != "AuthenticAMD") return 1;

      //Check family and model
      xCPUID(REG, 1);
      uint32_t Data       = REG[0];
      uint32_t FamilyExt  = (Data >> 20) & 0xFF;
      uint32_t ModelExt   = (Data >> 16) & 0xF;
      uint32_t FamilyBase = (Data >>  8) & 0xF;
      uint32_t ModelBase  = (Data >>  4) & 0xF;
      uint32_t Family     = FamilyBase + (FamilyBase == 15 ? FamilyExt : 0);
      uint32_t Model      = FamilyBase == 6 || FamilyBase == 15 ? (ModelExt << 4) + ModelBase : ModelBase;

      if (Family == 0x19)
      {
        if((Model >= 0x10 && Model <= 0x1F) || (Model >= 0x60 && Model <= 0xAF))
        {
          return 0;
        }
      }
      else if (Family > 0x19)
      {
        return 0;
      }
      return 1;
    }
    ]=] DETECTED_ZNVER4)
    #message(STATUS "DETECTED_ZNVER4    = ${DETECTED_ZNVER4}")

  if(${DETECTED_ZNVER4})
    set(${HOST_x86_64_MFL} "ZenVer4" PARENT_SCOPE)
    return()
  endif() 
  endif() #PMBB_ENABLE_EXPERIMENTAL

  #=========================================================================================================================================
  # Check for AVX512 support
  if(PMBB_COMPILER_IS_GCC_OR_CLANG)
    set(CMAKE_REQUIRED_FLAGS "-skylake-avx512")
  elseif(PMBB_COMPILER_IS_MSVC)
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
  if(PMBB_COMPILER_IS_GCC_OR_CLANG)
    set(CMAKE_REQUIRED_FLAGS "-mavx2")
  elseif(PMBB_COMPILER_IS_MSVC)
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
  if(PMBB_COMPILER_IS_GCC_OR_CLANG)
    set(CMAKE_REQUIRED_FLAGS "-msse4.2")
  elseif(PMBB_COMPILER_IS_MSVC)
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

  if(PMBB_COMPILER_IS_GCC_OR_CLANG)
    set(CMAKE_REQUIRED_FLAGS "-msse2")
  elseif(PMBB_COMPILER_IS_MSVC)
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


