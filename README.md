# IV-PSNR software

## 1. Description

The software calculates IV-PSNR, WS-PSNR and PSNR metrics.  

The idea behind the IV-PSNR mertric, its detailed description and evaluation can be found in the paper [IVPSNR]:  
**A. Dziembowski, D. Mieloch, J. Stankowski and A. Grzelka, "IV-PSNR – the objective quality metric for immersive video applications," in IEEE Transactions on Circuits and Systems for Video Technology, doi: [10.1109/TCSVT.2022.3179575](https://doi.org/10.1109/TCSVT.2022.3179575). [Available on authors webpage](http://multimedia.edu.pl/?page=publication&section=IV-PSNR---the-objective-quality-metric-for-immersive-video-applications).**

## 2. Authors

* Jakub Stankowski   - Poznan University of Technology, Poznań, Poland  
* Adrian Dziembowski - Poznan University of Technology, Poznań, Poland

## 3. License

```txt
The copyright in this software is being made available under the BSD
License, included below. This software may be subject to other third party
and contributor rights, including patent rights, and no such rights are
granted under this license.

Copyright (c) 2019-2023, Jakub Stankowski & Adrian Dziembowski, All rights reserved.

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

Building the IV-PSNR software requires using CMake (https://cmake.org/) and C++17 conformant compiler (e.g., GCC >= 8.0, clang >= 5.0, MSVC >= 19.15). For user convenience, we prepared a set of scripts for easy "one click" configure and build:
* *configure_and_build.bat* - for Windows users
* *configure_and_build.sh* - for Unix/Linux users

The IV-PSNR application and its build system is designed to create fastest possible binary. On x86-64 microarchitectures the build system can create four version of compiled application, each optimized for one predefined x86-64 Microarchitecture Feature Levels [x86-64, x86-64-v2, x86-64-v3, x86-64-v4] (defined in https://gitlab.com/x86-psABIs/x86-64-ABI). The final binary consists of this four optimized variants and a runtime dynamic dispatcher. The dispatcher uses CPUID instruction to detect available instruction set extensions and selects the fastest possible code path. 

The IV-PSNR CMake project defines the following parameters:
| Variable | Type | Description |
| :------- | :--- | :---------- |
| `PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES`        | BOOL | Enables generation of multiple code paths, optimized for each variant of x86-64 Microarchitecture Feature Levels. |
| `PMBB_GENERATE_SINGLE_APP_WITH_WITH_RUNTIME_DISPATCH` | BOOL | Enables building single application with runtime dynamic dispatch. Requires `PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES=True`. |
| `PMBB_GENERATE_DEDICATED_APPS_FOR_EVERY_MFL`          | BOOL | Enables building multiple applications, each optimized for selected x86-64 Microarchitecture Feature Level. Requires `PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES=True`. |
| `PMBB_BUILD_WITH_MARCH_NATIVE`                        | BOOL | Enable option to force compiler to tune generated code for the micro-architecture and ISA extensions of the host CPU. Conflicts with `PMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES`. Generated binary is not portable. |

## 5. Usage

### 5.1. Commandline parameters

Commandline parameters are parsed from left to right. Multiple config files are allowed. Moreover config file parameters can be override or added via commandline.  

#### General parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-i0  | InputFile0       | YUV file path - reference |
|-i1  | InputFile1       | YUV file path - tested |
|-w   | PictureWidth     | Width of sequence |
|-h   | PictureHeight    | Height of sequence |
|-bd  | BitDepth         | Bit depth (optional, default 8, up to 14) |
|-cf  | ChromaFormat     | Chroma format (optional, default 420) [420, 444] |
|-s0  | StartFrame0      | Start frame (optional, default 0) |
|-s1  | StartFrame1      | Start frame (optional, default 0) |
|-l   | NumberOfFrames   | Number of frames to be processed (optional, default -1=all) |
|-r   | ResultFile       | Output file path for printing result(s) (optional) |

#### Masked (weighted) mode parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-im  | InputFileM       | YUV file path - mask (optional, same resolution as InputFile0 and InputFile1) |
|-bdm | BitDepthM        | Bit depth for mask     (optional, default=BitDepth, up to 16) |
|-cfm | ChromaFormatM    | Chroma format for mask (optional, default=ChromaFormat) [400, 420, 444] |

#### Equirectangular parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-erp | Equirectangular  | Equirectangular sequence (flag, default disabled) |
|-lor | LonRangeDeg      | Range for ERP sequence in degrees - Longitudinal (optional, default 360) |
|-lar | LatRangeDeg      | Range for ERP sequence in degrees - Lateral (optional, default 180) |

#### IV-PSNR specific parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-sr  | SearchRange      | IV-PSNR search range around center point (optional, default 2=5x5) |
|-cws | ComponentWeights | IV-PSNR component weights ("Lm:Cb:Cr:0" - per component integer weight, default "4:1:1:0", quotes are required, requires USE_RUNTIME_CMPWEIGHTS=1) |
|-unc | UnnoticeableCoef | IV-PSNR unnoticeable color difference threshold coeff ("Lm:Cb:Cr:0" - per component coeff, default "0.01:0.01:0.01:0", quotes are required) |

#### WS-PSNR specific parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-ws8 | Legacy8bitWSPSNR | Use 1020 as peak value for 10-bps videos in WSPSNR metric (provides compatibility with original WSPSNR implementation, optional, default=0) |

#### Application parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-t   | NumberOfThreads  | Number of worker threads (optional, default -1=all, suggested 4-8, 0=disables internal thread pool) |
|-ilp | InterleavedPic   | Use additional image buffer with interleaved layout for IV-PSNR, (improves performance at a cost of increased memory usage, optional, default=1) |
|-ipa | InvalidPelAction | Select action taken if invalid pixel value is detected (optional, default STOP) [SKIP - disable pixel value checking, WARN - print warning and ignore, STOP - stop execution, CNCL - try to conceal by clipping to bit depth range] |
|-v   | VerboseLevel     | Verbose level (optional, default=1) |

#### External config file

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
| -c  | n/a              | Valid path to external config file - in INI format (optional). Multiple config files can be provided by using multiple "-c" arguments. Config files are processed in arguments order. Content of config files are merged while repeating values are overwritten. |

### 5.2. Verbose level

| Value | Behavior |
|:------|:---------|
| 0 | final PSNR, WSPSNR, IVPSNR values only |
| 1 | 0 + configuration + detected frame numbers |
| 2 | 1 + argc/argv + frame level PSNR, WSPSNR, IVPSNR |
| 3 | 2 + computing time (LOAD, PSNR, WSPSNR, IVPSNR) (uses high_resolution_clock, could slightly slow down computations) |
| 4 | 3 + IVPSNR specific debug data (GlobalColorShift, R2T+T2R, NumNonMasked) |

### 5.3. Compile-time parameters

| Parameter name | Default value | Description |
|:------------|:--------------|:------------|
| USE_SIMD               | 1 | use SIMD (to be precise... use SSE 4.1 or AVX2) |
| USE_KBNS               | 1 | use Kahan-Babuška-Neumaier floating point summation algorithm (reduces accumulation errors) |
| USE_RUNTIME_CMPWEIGHTS | 1 | use component weights provided at runtime |

### 5.4. Examples

#### 5.4.1. Commandline parameters example

IV-PSNR of *SA_ref.yuv* and *SA_test.yuv*. Sequence resolution is 4096×2048, YUV420, 10 bits per sample. Sequence format is ERP. Mean IV-PSNR calculated for the first 20 frames will be written into IV-PSNR.txt:  
`IVPSNR -i0 SA_ref.yuv -i1 SA_test.yuv -w 4096 -h 2048 -bd 10 -erp -l 20 -o IV-PSNR.txt`  

IV-PSNR of *SD_ref.yuv* and *SD_test.yuv*. Sequence resolution is 2048×1088, YUV420, 8 bits per sample. Sequence format is perspective. Mean IV-PSNR calculated for all frames will be written into results.txt:  
`IVPSNR -i0 SD_ref.yuv -i1 SD_test.yuv -o results.txt -w 2048 -h 1088`  

IV-PSNR of *SC_ref.yuv* and *SC_test.yuv*. Sequence resolution is 4096×2048, YUV420, 10 bits per sample. Sequence format is ERP, with lateral range equal to 90°. Mean IV-PSNR calculated for 5 frames (frames 0-4 of reference video and 10-14 of test video) will be written into o.txt:  
`IVPSNR -i0 SC_ref.yuv -i1 SC_test.yuv -w 4096 -h 2048 -erp -lar 90 -l 5 -s1 10 -o o.txt`

External config file:  
`IVPSNR -c "config.cfg"`  

External config files with some parameters added/overwritten:  
`IVPSNR -c "config1st.cfg" -c "config2nd.cfg" -v 1 -t 4`  

#### 5.4.2. Config file example

```ini
InputFile0      = "SA_ref.yuv"
InputFile1      = "SA_test.yuv"
PictureWidth    = 4096
PictureHeight   = 2048
BitDepth        = 10
ChromaFormat    = 420
VerboseLevel    = 3
OutputFile      = "IV-PSNR.txt"
```

### 5.5. Pixel values checking notes (InvalidPelAction)
Before calculating the IV-PSNR metric the software scans the content of YUV file in order to evaluate if all pixel values are in range `[0, MaxVal]` where `MaxVal = (1<<BitDepth) - 1`. If invalid pel is detected the software can take following actions based on InvalidPelAction parameter value:
* SKIP - disable pixel value checking
* WARN - print warning and ignore invalid pel values (may lead to unreliable IV-PSNR, WS-PSNR and PSNR metrics value)
* STOP - print warning and stop execution
* CNCL - print warning and try to conceal the pel value by clipping to highest value within bit depth range

### 5.6. Masked mode - requirements and notes

* Resolution of the mask file has to be identical as input file.
* Allowed mask values are `0` (interpreted as inactive pixel) and `(1<<BitDepthM)-1)` (interpreted as active pixel). Behavior for other values is undefined at this moment.
* The data processing functions for masked mode are not implemented with the use of SIMD instructions.

## 6. Changelog

### v5.0 [M64727]

* added detection invalid pel values (higher than `(1<<BitDepth) - 1`) and possibility to choose taken action (see InvalidPelAction parameter)
* new cmake-based build system with simultaneous build of four variants of x86-64 Microarchitecture Feature Level and runtime dynamic dispatch
* added warning for settings influencing performance or breaking conformance with IV-PSNR metric defined in [M54279]

### v4.0 [M59974]

* SIMD (SSE 4.1) implementation of IV-PSNR metric calculation (for interleaved picture buffers)
* wider SIMD (AVX2) implementation for most data processing functions
* runtime adjustable component weights for IV-PSNR metric
* adjustable search range for IV-PSNR metric
* adjustable unnoticable color difference threshold coeff for IV-PSNR metric
* reading parameters from config file
* protection against StartFrame >= DetectedFrames
* writing error messages to stdout and stderr
* non-performance critical parameters moved from compile-time to run-time selection
* added mask file option

### v3.0 [M55752]

* enabled INTERPROCEDURAL_OPTIMIZATION and assumed x86-64 Microarchitecture Feature Level >= x86-64-v2
* new implementation picture I/O
* reduced filesystem burden (avoid repetitive open-seek-read-close cycles)
* use of interleaved picture layout for IVPSNR calculation
* SIMD (SSE 4.1) implementation for most data processing functions
* dedicated thread pool instead of OpenMP directives (due to high OpenMP overhead)

### v2.1.1 [no reference]

* bugfix

### v2.1 [M54896]

* support for parallel processing (using OpenMP)
* addition of PSNR and WS-PSNR [Sun17] values outputting
* fixed WS-weight calculation for ERP sequences with non-180 lateral range
* changed commandline arguments formatting
* addition of detection of corrupted YUV files
* change in compile-time parameters:
    * VERBOSE_LEVEL is now a commandline parameter
    * WSPSNR_PEAK_VALUE_8BIT flag added (default = enabled), when enabled, the signal peak value for WS-PSNR computation is set to `255 << (BitDepth - 8)`. Otherwise, it is equal to `(1<<BitDepth) - 1`

### v2.0 [M54279]

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

### v1.0 [M45093]

* first release


## 7. References

* [IVPSNR] A. Dziembowski, D. Mieloch, J. Stankowski and A. Grzelka, "IV-PSNR – the objective quality metric for immersive video applications," in IEEE Transactions on Circuits and Systems for Video Technology, doi: [10.1109/TCSVT.2022.3179575](https://doi.org/10.1109/TCSVT.2022.3179575). [Available on authors webpage](http://multimedia.edu.pl/?page=publication&section=IV-PSNR---the-objective-quality-metric-for-immersive-video-applications)
* [M64727] J. Stankowski, A. Dziembowski, "Optimized IV-PSNR software with invalid pixel detection", ISO/IEC JTC1/SC29/WG04 MPEG/M64727, October 2023, Hannover, Germany.
* [M59974] J. Stankowski, A. Dziembowski, "Improved IV-PSNR software", ISO/IEC JTC1/SC29/WG04 MPEG/M59974, July 2021, Online.
* [M55752] J. Stankowski, A. Dziembowski, "Slightly faster IVPSNR", ISO/IEC JTC1/SC29/WG04 MPEG/M55752, January 2021, Online.
* [M54896] J. Stankowski, A. Dziembowski, "Even faster implementation of IV-PSNR software", ISO/IEC JTC1/SC29/WG04 MPEG/M54896, October 2020, Online.
* [M54279] J. Stankowski, A. Dziembowski, "[MPEG-I Visual] Fast implementation of IV-PSNR software", ISO/IEC JTC1/SC29/WG11 MPEG/M54279, July 2020, Online.
* [M48093] A. Dziembowski, M. Domański, "[MPEG-I Visual] Objective quality metric for immersive video", ISO/IEC JTC1/SC29/WG11 MPEG/M48093, July 2019, Göteborg, Sweden.