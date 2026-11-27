# QMIV framework (former IV-PSNR software)
Quality Merics for Immersive Video

----  

The QMIV framework (successor of the IV-PSNR software) is a set of applications for the objective quality assessment of immersive video and of 3D Gaussian splat representations.

| Application | Description | Documentation |
|:------------|:------------|:--------------|
| QMIV             | Calculates quality metrics (IV-PSNR, IV-SSIM, IV-MS-SSIM, PSNR, WS-PSNR, SSIM, MS-SSIM, pVAR) between two video sequences (YUV, PNG or BMP) | [README_QMIV.md](README_QMIV.md) |
| QMIV-GS          | Renders test and reference 3D Gaussian splat clouds with a built-in CPU renderer and calculates quality metrics between the rendered views | [README_QMIV-GS.md](README_QMIV-GS.md) |
| QMIV-GS-renderer | Renders views of a 3D Gaussian splat cloud and writes them to files (YUV, PNG or BMP) | [README_QMIV-GSR.md](README_QMIV-GSR.md) |

All applications are built together with CMake (see Section 4), share the implementation of the metrics and of the renderer, and accept the same style of commandline parameters and config files. The documents of the applications describe their parameters and usage. This document contains the information common to all of them: the metrics, the authors, the license, the building instructions and the changelog.

## 1. Metrics

The list of immersive video quality metrics includes: 
  * IV-PSNR    - Immersive Video - Peak Signal-to-Noise Ratio [IVPSNR]
  * IV-SSIM    - Immersive Video - Structural Similarity Index Measure [IVSSIM]
  * IV-MS-SSIM - Immersive Video - Multi Scale Structural Similarity Index Measure [IVMSSSIM]

In addition the software is able to calculate following "general purpose" metrics:
  * PSNR    - Peak Signal-to-Noise Ratio
  * WS-PSNR - Spherical Weighted - Peak Signal-to-Noise Ratio [Sun17]
  * SSIM    - Structural Similarity Index Measure
  * MS-SSIM - Multi Scale Structural Similarity Index Measure
  * pVAR    - Perceptual Variance of Distortion [PVAR] (selected as `PVD` in MetricList)

The idea behind the IV-PSNR metric, its detailed description and evaluation can be found in the paper [IVPSNR]:  
**A. Dziembowski, D. Mieloch, J. Stankowski and A. Grzelka, "IV-PSNR – the objective quality metric for immersive video applications," in IEEE Transactions on Circuits and Systems for Video Technology, doi: [10.1109/TCSVT.2022.3179575](https://doi.org/10.1109/TCSVT.2022.3179575). [Available on authors webpage](http://multimedia.edu.pl/?page=publication&section=IV-PSNR---the-objective-quality-metric-for-immersive-video-applications).**

The idea behind the IV-SSIM metric, its detailed description and evaluation can be found in the paper [IVSSIM]:  
**A. Dziembowski, W. Nowak, J. Stankowski, "IV-SSIM - The Structural Similarity Metric for Immersive Video", Applied Sciences, Vol. 14, No. 16, Aug 2024, doi: [10.3390/app14167090](https://doi.org/10.3390/app14167090). [Available on authors webpage](http://multimedia.edu.pl/?page=publication&section=IV-SSIM----The-Structural-Similarity-Metric-for-Immersive-Video).**

The idea behind the IV-MS-SSIM metric, its detailed description and evaluation can be found in the paper [IVMSSSIM]:  
**B. Sojka, J. Stankowski, A. Dziembowski, "Multi-scale structural similarity with geometric alignment for immersive video quality assessment", International Conference on Image Processing, ICIP 2026, Tampere, Finland, 13-17.09.2026. [Available on authors webpage](http://multimedia.edu.pl/?page=publication&section=Multi-scale-structural-similarity-with-geometric-alignment-for-immersive-video-quality-assessment).**

The idea behind the pVAR metric, its detailed description and evaluation can be found in the paper [PVAR]:  
**A. Dziembowski, "pVAR: An ultra low complexity image quality assessment via spatial error variance", IEEE Signal Processing Letters, Vol. 33, 2026, pp. 2710-2714, doi: [10.1109/LSP.2026.3708417](https://doi.org/10.1109/LSP.2026.3708417).**

The QMIV software (former IV-PSNR software) and its architecture is described in following papers [IVSOFT1],[IVSOFT2],[IVSOFT3].  

## 2. Authors

* Jakub Stankowski   - Poznan University of Technology (PUT), Poznań, Poland  
* Błażej Szydełko    - Poznan University of Technology (PUT), Poznań, Poland  
* Adrian Dziembowski - Poznan University of Technology (PUT), Poznań, Poland

Credits - PUT students and graduates who contributed to this project:

* Weronika Nowak (IV-SSIM co-development and prototyping)
* Patrycja Kaźmierczak (ARM64 support & NEON optimized implementation) 

## 3. License

### 3.1. TLDR

3-Clause BSD License

### 3.2. Full text

```txt
The copyright in this software is being made available under the BSD
License, included below. This software may be subject to other third party
and contributor rights, including patent rights, and no such rights are
granted under this license.

Copyright (c) 2019-2025, Jakub Stankowski & Adrian Dziembowski, All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the ISO/IEC nor the names of its contributors may
   be used to endorse or promote products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
```

## 4. Building 

Building the QMIV framework requires using CMake (https://cmake.org/) and C++17 conformant compiler (e.g., GCC >= 10.0, clang >= 13.0, MSVC >= 19.15). All applications are built together, the resulting executables are named `QMIV`, `QMIV-GS` and `QMIV-GS-renderer`.

The applications of the QMIV framework and its build system are designed to create fastest possible binaries.   
* On x86-64 microarchitecture the build system can create four version of compiled application, each optimized for one predefined x86-64 Microarchitecture Feature Levels [x86-64, x86-64-v2, x86-64-v3, x86-64-v4] (defined in https://gitlab.com/x86-psABIs/x86-64-ABI). The final binary consists of this four optimized variants and a runtime dynamic dispatcher. The dispatcher uses CPUID instruction to detect available instruction set extensions and selects the fastest possible code path.   
* On ARM64 (AArch64) microarchitecture the build system can create two version of compiled application, optimized for ARM64v8.0 and ARM64v8.2. The final binary consists of this two optimized variants and a runtime dynamic dispatcher. The dispatcher uses ARM64 ELF hwcaps to detect available instruction set extensions and selects the fastest possible code path. This feature is available when building for Linux operating system only.   

The QMIV CMake project defines the following parameters:

| Variable | Type | Description |
| :------- | :--- | :---------- |
| `PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES`        | BOOL | Enables generation of multiple code paths, optimized for each variant of x86-64 Microarchitecture Feature Levels. |
| `PMBB_GENERATE_SINGLE_APP_WITH_WITH_RUNTIME_DISPATCH` | BOOL | Enables building single application with runtime dynamic dispatch. Requires `PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES=True`. |
| `PMBB_GENERATE_DEDICATED_APPS_FOR_EVERY_MFL`          | BOOL | Enables building multiple applications, each optimized for selected x86-64 Microarchitecture Feature Level. Requires `PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES=True`. |
| `PMBB_BUILD_WITH_MARCH_NATIVE`                        | BOOL | Enable option to force compiler to tune generated code for the micro-architecture and ISA extensions of the host CPU. This option works only when GCC or clang compiler is used since MSVC is unable to adapt native architecture. Conflicts with `PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES`. Generated binary is not portable across different microarchitecures. |
| `PMBB_GENERATE_TESTING`                               | BOOL | Enables generation of unit tests for critical data processing routines. |

For users convenience, we prepared a set of scripts for easy "one click" configure and build:
* *configure_and_build.bat* - for Windows users
* *configure_and_build.sh* - for Unix/Linux users

For developers convenience, we prepared a set of scripts for easy "one click" configure in development mode - without multi-architecture build and dynamic dispatch:
* *configure_for_developement.bat* - for Windows users
* *configure_for_developement.sh* - for Unix/Linux users

The *configure_and_build* and *configure_for_developement* uses different set of CMake parameters:

| Variable | configure_and_build | configure_for_developement |
| :------- | :------------------ | :------------------------- |
| `PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES`        | True  | False |
| `PMBB_GENERATE_SINGLE_APP_WITH_WITH_RUNTIME_DISPATCH` | True  | False |
| `PMBB_GENERATE_DEDICATED_APPS_FOR_EVERY_MFL`          | False | False |
| `PMBB_BUILD_WITH_MARCH_NATIVE`                        | False | True  |
| `PMBB_GENERATE_TESTING`                               | False | True  |

### 4.1. Compile-time parameters

| Parameter name | Default value | Description |
|:------------|:--------------|:------------|
| PMBB_USE_SIMD  | 1 | use SIMD (to be precise... use SSE 4.1 or AVX2 or AVX512 or NEON), defined in libs/pmbb-core/src/xCommonDefCORE.h |

## 5. Changelog

### IV-PSNR v9.0 (QMIV v4.0)
* added applications for the assessment of 3D Gaussian splat representations:
  * QMIV-GS - renders the test and reference clouds with a built-in CPU renderer and calculates the metrics of the rendered views (see [README_QMIV-GS.md](README_QMIV-GS.md)):
    * built-in CPU renderer of 3D Gaussian splats (tile-based, AVX2/AVX512 implementation, deterministic)
    * camera parameters in PLY, COLMAP and INI formats
    * input views mode and posetrace mode
    * masked mode with masks derived during rendering (Occupancy, Alpha)
    * metrics calculated in RGB and YCbCr
    * output of rendered pictures (RAW, PNG, BMP; RGB or YCbCr)
    * calculation of hashes of input clouds and rendered pictures (xxHash - XXH128)
  * QMIV-GS-renderer - renders views of a single cloud with the renderer of QMIV-GS and writes them to files (see [README_QMIV-GSR.md](README_QMIV-GSR.md))
* added pVAR metric (`PVD` in MetricList) [PVAR]
* faster calculation of IV-metrics (IV-PSNR, IV-SSIM, IV-MS-SSIM) - the search for corresponding pixels operates on planar pictures and processes several pixels at once (SSE4.1, AVX2, AVX512, NEON)
* reduced memory usage
* fixed calculation of MS-SSIM metric(s) for small pictures - avoid scaling below 32x32px
* faster calculation of IV-SSIM and IV-MS-SSIM metrics by algorithmic optimization and wider SIMD coverage
* faster calculation of SSIM metric(s) by using multi-block AVX512 implementation
* added additional variant of SSIM-related metrics calculation: StructSimBrdExt
* added support for ARM64 microarchitecture (AArch64 mode)
* added implements using NEON SIMD extension for most performance critical routines 
  * ARM NOEN SIMD optimized routines coverage is on par with x86-64 SIMD (SSE4.1, AVX2, and AVX512)
* cmake-based build system with support for runtime dynamic dispatch for ARM64 CPUs
  * by default build for ARMv8.0 and ARMv8.2 with straightforward  extension possibilities
* added possibility to read BMP file or list of BMP files
* improved test coverage
* several small performance improvements & minor bugfixes

### IV-PSNR v8.0 (based on QMIV v3.0)

* added calculation of IV-MS-SSIM (Immersive Video - Multi Scale SSIM)
* added several variants of SSIM-related metrics calculation:
  * Structural similarity mode - allows to change mode and windowing approach:
    * RegularGaussianFlt, RegularGaussianInt, RegularAveraged - regular 11x11 mode
    * BlockGaussianInt, BlockAveraged - block-based mode
    * two implementations of Gaussian window (floating point based and integer based with quantized Gaussian filter coefficients) or simplified averaging window
  * Structural similarity stride - allows to calculate SSIM-related metrics every N pixels
  * Structural similarity window size - allows to provide window size for block mode SSIM
* added new parameters: StructSimMode, StructSimStride, StructSimWindow
* switching default approach for SSIM-related metrics form (Mode=RegularGaussianFlt,Stride=1 - as defined by authors of SSIM metric) to (Mode=BlockAveraged,Stride=1,Window=8 - similar to approach used by FFMPEG) - this change reduces computational complexity while increasing correlation with MOS
* faster calculation of SSIM-based metrics by algorithmic optimization and wider SIMD coverage
  * fast SIMD (SSE4.1, AVX2, and AVX512) implementation for BlockAveraged SSIM mode
* added possibility to read BMP file or list of BMP files
* fixed calculation of MS-SSIM metric(s) for small pictures - avoid scaling below 32x32px
* better performance for high core count CPUs
  * reduced multi-threading overhead by switching to new thread pool API
  * task batching for per-row operations
  * reduced thread pool overhead
* overhauled build system and simplified cmake files
* improved test coverage
* several small performance improvements & minor bugfixes

### IV-PSNR v7.1

* added calculation of MS-SSIM (Multi Scale SSIM) metric 

### IV-PSNR v7.0 (based on QMIV v1.0) [N0535]

* first release of QMIV framework which is descendant of previously developed IV-PSNR software v6.0
* changes relative to IV-PSNR software v6.0:
  * **introduces a set of Structural Similarity related metrics - SSIM and IV-SSIM**
  * improved arguments parsing and validation
  * PictureSize argument introduced as (optional) alternative to PictureWidth, PictureHeight pair
  * PictureFormat argument introduced as (optional) alternative to BitDepth, ChromaFormat pair
  * improved consistency of selected "cmd" arguments:
    * PictureWidth `-w`-->`-pw`
    * PictureHeight `-h`-->`-ph`
    * NumberOfFrames `-l`-->`-nf`
    * ResultFile `-o`-->`-r`
    * NumberOfThreads `-t`-->`-nth`
  * reduced overhead of computing time measurement (switched from `std::chrono::high_resolution_clock` to `RDTSCP`)
  * increased precision of printed metric values
* overhaul of IVPSNR v6.0 RGB mode into generic colorspace mode, including:
  * consistent metric suffixes
  * RGB to YCbCr and YCbCr to RGB conversion
  * RGB passthrough mode
  * CalcMetricInRGB and ColorSpace arguments replaced by more general set of arguments (ColorSpaceInput, ColorSpaceMetric)
* input defined as PNG file or list of PNG files.

### IV-PSNR v6.0 [M68222]

* yet another general overhaul of entire software structure 
* ComponentWeights parameter split into two new parameters CmpWeightsSearch and CmpWeightsAverage
* added option to calculate metric in RGB color space:
  * the YCbCr input sequence is converted to RGB and metric is calculated on RGB pictures
  * CalcMetricInRGB option added to enable this mode
  * ColorSpace option added to select color space for YCbCr-->RGB conversion [BT601, BT709, BT2020]
* added fast SIMD implementation of Kahan-Babuška-Neumaier accumulation 
* added "reasonable auto" mode for number of threads selection and set it as default behavior (since using all available threads on 128 core machine was slightly unwise)
* added possibility to overwrite dynamic dispatcher decision and manuały select code path
* added weighted PSNR-YCbCr, WSPSNR-YCbCr, PSNR-RGB, and WSPSNR-RGB output

### IV-PSNR v5.0 [M64727]

* general overhaul of entire software structure 
* new cmake-based build system with simultaneous build of four variants of x86-64 Microarchitecture Feature Level and runtime dynamic dispatch
* added unit tests for basic data processing routines
* added detection invalid pel values (higher than `(1<<BitDepth) - 1`) and possibility to choose taken action (see InvalidPelAction parameter)
* added warning for settings influencing performance or breaking conformance with IV-PSNR metric defined in [M54279]
* added detection of mismatch between file name and provided parameters (resolution, bit depth and chroma format)
* added usage of hugepages on Linux-based systems (using madvise)
* added support for chroma format 4:2:2
* more data processing functions implemented using AVX2
* wider SIMD (AVX512) implementation for some data processing functions 

### IV-PSNR v4.0 [M59974]

* SIMD (SSE 4.1) implementation of IV-PSNR metric calculation (for interleaved picture buffers)
* wider SIMD (AVX2) implementation for most data processing functions
* runtime adjustable component weights for IV-PSNR metric
* adjustable search range for IV-PSNR metric
* adjustable unnoticeable color difference threshold coeff for IV-PSNR metric
* reading parameters from config file
* protection against StartFrame >= DetectedFrames
* writing error messages to stdout and stderr
* non-performance critical parameters moved from compile-time to run-time selection
* added mask file option

### IV-PSNR v3.0 [M55752]

* enabled INTERPROCEDURAL_OPTIMIZATION and assumed x86-64 Microarchitecture Feature Level >= x86-64-v2
* new implementation picture I/O
* reduced filesystem burden (avoid repetitive open-seek-read-close cycles)
* use of interleaved picture layout for IVPSNR calculation
* SIMD (SSE 4.1) implementation for most data processing functions
* dedicated thread pool instead of OpenMP directives (due to high OpenMP overhead)

### IV-PSNR v2.1.1 [no reference]

* bugfix

### IV-PSNR v2.1 [M54896]

* support for parallel processing (using OpenMP)
* addition of PSNR and WS-PSNR [Sun17] values outputting
* fixed WS-weight calculation for ERP sequences with non-180 lateral range
* changed commandline arguments formatting
* addition of detection of corrupted YUV files
* change in compile-time parameters:
    * VERBOSE_LEVEL is now a commandline parameter
    * WSPSNR_PEAK_VALUE_8BIT flag added (default = enabled), when enabled, the signal peak value for WS-PSNR computation is set to `255 << (BitDepth - 8)`. Otherwise, it is equal to `(1<<BitDepth) - 1`

### IV-PSNR v2.0 [M54279]

* addition of (rOff) and (tOff) commandline parameters
* removal of redundant GCD calculations
* usage of uint16_t data type and 4:4:4 chroma format for internal picture storage
* new implementation of pixel-level processing steps
* reduction of filesystem burden by coalescing read
* detection of read errors – causes application to exit returning EXIT_FAILURE
* implementation of Kahan-Babuška-Neumaier accumulation
* improved conversion of 8bps input sequences
* improved interpolation for input sequences with 4:2:0 chroma format
* addition of 3 compile-time parameters:
    * VERBOSE_LEVEL – controls number of per-frame printing; default = 0
    * USE_KBNS – enables the Kahan-Babuška-Neumaier accumulation; default = enabled
    * USE_FIXED_WEIGHTS – enables faster 5×5 block search with fixed component weight (equal to 4:1:1); default = enabled
* fixed possibility of reading from unallocated memory region during 5×5 block search
* fixed GCD values rounding and clipping

### IV-PSNR v1.0 [M48093]

* first IV-PSNR release

## 6. References

* [IVPSNR] A. Dziembowski, D. Mieloch, J. Stankowski and A. Grzelka, "IV-PSNR – the objective quality metric for immersive video applications," in IEEE Transactions on Circuits and Systems for Video Technology, doi: [10.1109/TCSVT.2022.3179575](https://doi.org/10.1109/TCSVT.2022.3179575). [Available on authors webpage](http://multimedia.edu.pl/?page=publication&section=IV-PSNR---the-objective-quality-metric-for-immersive-video-applications)
* [IVSSIM] A. Dziembowski, W. Nowak, J. Stankowski, "IV-SSIM - The Structural Similarity Metric for Immersive Video", Applied Sciences, Vol. 14, No. 16, Aug 2024, doi: [10.3390/app14167090](https://doi.org/10.3390/app14167090). [Available on authors webpage](http://multimedia.edu.pl/?page=publication&section=IV-SSIM----The-Structural-Similarity-Metric-for-Immersive-Video)
* [IVMSSSIM] B. Sojka, J. Stankowski, A. Dziembowski, "Multi-scale structural similarity with geometric alignment for immersive video quality assessment," International Conference on Image Processing, ICIP 2026, Tampere, Finland, 13-17.09.2026
* [PVAR] A. Dziembowski, "pVAR: An ultra low complexity image quality assessment via spatial error variance," IEEE Signal Processing Letters, Vol. 33, 2026, pp. 2710-2714, doi: [10.1109/LSP.2026.3708417](https://doi.org/10.1109/LSP.2026.3708417)
* [Sun17] Y. Sun, A. Lu, L. Yu, "Weighted-to-Spherically-Uniform Quality Evaluation for Omnidirectional Video," IEEE Signal Processing Letters, Vol. 24, No. 9, 2017, pp. 1408-1412, doi: [10.1109/LSP.2017.2720693](https://doi.org/10.1109/LSP.2017.2720693)
* [IVSOFT1] J. Stankowski, A. Dziembowski, "IV-PSNR: Software for immersive video objective quality evaluation," SoftwareX, Volume 24, 2023, doi: [10.1016/j.softx.2023.101592](https://doi.org/10.1016/j.softx.2023.101592)
* [IVSOFT2] J. Stankowski, A. Dziembowski, "Version [7.1] – [IV-PSNR: Software for immersive video objective quality evaluation]," SoftwareX, Volume 28, 2024, doi: [10.1016/j.softx.2024.101961](https://doi.org/10.1016/j.softx.2024.101961)
* [IVSOFT3] J. Stankowski, A. Dziembowski, "Version [8.0] – [IV-PSNR: Software for immersive video objective quality evaluation]," SoftwareX, Volume 33, 2026, 102487
* [M79068] B. Szydełko, J. Stankowski, A. Dziembowski, "Unified QMIV framework with optimized renderer for Gaussian splats", ISO/IEC JTC1/SC29/WG7 MPEG 3DGH/M79068, October 2026, Hangzhou, China.
* [N0580] J. Stankowski, A. Dziembowski, "Software manual of QMIV 2", ISO/IEC JTC1/SC29/WG04 MPEG VC/N0580, October 2024, Antalya, Turkey.
* [N0535] J. Stankowski, A. Dziembowski, "Software manual of QMIV", ISO/IEC JTC1/SC29/WG04 MPEG VC/N0535, July 2024, Sapporo, Japan.
* [M68222] J. Stankowski, A. Dziembowski, "The final version of the IV-PSNR software", ISO/IEC JTC1/SC29/WG04 MPEG VC/M68222, July 2024, Sapporo, Japan.
* [M64727] J. Stankowski, A. Dziembowski, "Optimized IV-PSNR software with invalid pixel detection", ISO/IEC JTC1/SC29/WG04 MPEG VC/M64727, October 2023, Hannover, Germany.
* [M59974] J. Stankowski, A. Dziembowski, "Improved IV-PSNR software", ISO/IEC JTC1/SC29/WG04 MPEG VC/M59974, July 2021, Online.
* [M55752] J. Stankowski, A. Dziembowski, "Slightly faster IVPSNR", ISO/IEC JTC1/SC29/WG04 MPEG VC/M55752, January 2021, Online.
* [M54896] J. Stankowski, A. Dziembowski, "Even faster implementation of IV-PSNR software", ISO/IEC JTC1/SC29/WG04 MPEG VC/M54896, October 2020, Online.
* [M54279] J. Stankowski, A. Dziembowski, "[MPEG-I Visual] Fast implementation of IV-PSNR software", ISO/IEC JTC1/SC29/WG11 MPEG/M54279, July 2020, Online.
* [M48093] A. Dziembowski, M. Domański, "[MPEG-I Visual] Objective quality metric for immersive video", ISO/IEC JTC1/SC29/WG11 MPEG/M48093, July 2019, Göteborg, Sweden.