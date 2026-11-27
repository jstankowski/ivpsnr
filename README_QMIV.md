# QMIV
Quality Metrics for Immersive Video  

----  

## 1. Description

QMIV is the application of the QMIV framework that calculates number of quality metrics between two video sequences, especially related to assessment of immersive video quality. The QMIV is a successor of IV-PSNR software.

The list of immersive video quality metrics includes: 
  * IV-PSNR    - Immersive Video - Peak Signal-to-Noise Ratio [IVPSNR]
  * IV-SSIM    - Immersive Video - Structural Similarity Index Measure [IVSSIM]
  * IV-MS-SSIM - Immersive Video - Multi Scale Structural Similarity Index Measure [IVMSSSIM]

In addition the software is able to calculate following "general purpose" metrics:
  * PSNR    - Peak Signal-to-Noise Ratio
  * WS-PSNR - Spherical Weighted - Peak Signal-to-Noise Ratio
  * SSIM    - Structural Similarity Index Measure
  * MS-SSIM - Multi Scale Structural Similarity Index Measure
  * pVAR    - Perceptual Variance of Distortion [PVAR] (selected as `PVD` in MetricList)

The papers describing the metrics and the QMIV software, as well as the authors, the license, the building instructions and the changelog of the QMIV framework are given in [README.md](README.md).

## 2. Building

QMIV is built together with the other applications of the QMIV framework, as described in Section 4 of [README.md](README.md). The resulting executable is named `QMIV`.

## 3. Usage

### 3.1. Commandline parameters

Commandline parameters are parsed from left to right. Multiple config files are allowed. Moreover, config file parameters can be overridden or added via commandline.  

#### General parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-i0  | InputFile0       | File path - input sequence 0 |
|-i1  | InputFile1       | File path - input sequence 1 |
|-ff  | FileFormat       | Format of input sequence (optional, default=RAW) [RAW, PNG, BMP] |
|-ps  | PictureSize      | Size of input sequences (WxH) |
|-pw  | PictureWidth     | Width of input sequence |
|-ph  | PictureHeight    | Height of input sequence |
|-pf  | PictureFormat    | Picture format as defined by FFMPEG pix_fmt i.e. yuv420p10le |
|-bd  | BitDepth         | Bit depth (optional, default=8, up to 14) |
|-cf  | ChromaFormat     | Chroma format (optional, default=420) [420, 422, 444] |
|-sf0 | StartFrame0      | Start frame 0 (optional, auto=-1, default=-1) |
|-sf1 | StartFrame1      | Start frame 1 (optional, auto=-1, default=-1) |
|-nf  | NumberOfFrames   | Number of frames to be processed (optional, all=-1, default=-1) |
|-r   | ResultFile       | Output file path for printing result(s) (optional) |
|-ml  | MetricList       | List of quality metrics to be calculated, must be coma separated, quotes are required. "All" enables all available metrics. [MSE, PSNR, WSPSNR, IVPSNR, SSIM, MSSSIM, IVSSIM, IVMSSSIM, PVD] (optional, default="PSNR, IVPSNR, IVSSIM") |

PictureSize parameter can be used interchangeably with PictureWidth, PictureHeight pair. If PictureSize parameter is present the PictureWidth and PictureHeight arguments are ignored.
PictureFormat parameter can be used interchangeably with BitDepth, ChromaFormat pair. If PictureFormat parameter is present the BitDepth and, ChromaFormat arguments are ignored.
StartFrame equal to -1 (auto) selects the first available frame (0 for RAW files, the first existing index for PNG/BMP file lists).

#### Masked (weighted) mode parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-ifm | InputFileM       | File path - mask          (optional, same resolution as InputFile0 and InputFile1) |
|-ffm | FileFormatM      | Format of mask            (optional, default=RAW) [RAW, PNG, BMP] |
|-bdm | BitDepthM        | Bit depth for mask        (optional, default=BitDepth, range 8-14) |
|-cfm | ChromaFormatM    | Chroma format for mask    (optional, default=ChromaFormat) [400, 420, 422, 444] |
|-sfm | StartFrameM      | Start frame of mask       (optional, auto=-1, default=-1) |
|-mtm | MaskTransMode    | Mask preprocessing mode   (optional, default=None) [None, Threshold] |
|-mtp | MaskTransParam   | Mask preprocessing parameter - threshold for Threshold mode (optional, default=128) |

#### Auxiliary output parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|     | ShftCompPicFile0 | File path - output of shift-compensated pictures (SCP) of input sequence 0, as used by the IV-metrics (optional, for debugging; config file only; both or none have to be provided) |
|     | ShftCompPicFile1 | File path - output of shift-compensated pictures (SCP) of input sequence 1, as above |

#### Equirectangular parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-erp | Equirectangular  | Equirectangular sequence (flag, default disabled) |
|-lor | LonRangeDeg      | Range for ERP sequence in degrees - Longitudinal (optional, default=360) |
|-lar | LatRangeDeg      | Range for ERP sequence in degrees - Lateral (optional, default=180) |

####  Colorspace parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-csi | ColorSpaceInput  | Color space of input file             (optional, default=YCbCr for RAW, RGB for PNG/BMP) |
|-csm | ColorSpaceMetric | Color space used to calculate metrics (optional, default=ColorSpaceInput) |

If ColorSpaceInput!=ColorSpaceMetric the software performs on-demand conversion (RGB-->YCbCr or YCbCr-->RGB). Conversion requires specific YCbCr color space parameters. [RGB, BGR, GBR, YCbCr, YCbCr_BT601, YCbCr_SMPTE170M, YCbCr_BT709, YCbCr_SMPTE240M, YCbCr_BT2020]

#### IV specific parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-sr  | SearchRange      | IV-metric search range around center point (optional, default=2 --> 5x5) |
|-cws | CmpWeightsSearch | IV-metric component weights used during search ("Lm:Cb:Cr:0" or "R:G:B:0" - per component integer weights, default="4:1:1:0", quotes are mandatory) |
|-cwa | CmpWeightsAverage| IV-metric component weights used during averaging ("Lm:Cb:Cr:0" or "R:G:B:0" - per component integer weights, default="4:1:1:0", quotes are mandatory) |
|-unc | UnnoticeableCoef | IV-metric unnoticeable color difference threshold coeff ("Lm:Cb:Cr:0" or "R:G:B:0" - per component coeff, default="0.01:0.01:0.01:0", quotes are mandatory) |

#### Structural similarity specific parameters
| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-ssm | StructSimMode    | Calculation mode and structure variant (optional, default=BlockAveraged) [BlockAveraged, RegularGaussianInt, RegularAveraged] (RegularGaussianFlt and BlockGaussianInt are available in experimental builds only) |
|-ssb | StructSimBrdExt  | Border extension mode (optional, applies to Regular mode only, default=None) [None, Nearest, Reflect, Mirror, Zero] (see scipy.ndimage.generic_filter) |
|-sss | StructSimStride  | Stride between pixels/windows (optional, default=4) |
|-ssw | StructSimWindow  | Size of structure window (optional, applies to Block modes only, default=8) [8,16,32] |

#### Validation parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-ipa | InvalidPelActn   | Select action taken if invalid pixel value is detected (optional, default=STOP) [SKIP = disable pixel value checking, WARN = print warning and ignore, STOP = stop execution, CNCL = try to conceal by clipping to bit depth range] |
|-nma | NameMismatchActn | Select action taken if parameters derived from filename are different than provided as input parameters. Checks resolution, bit depth and chroma format. (optional, default=WARN) [SKIP = disable checking, WARN = print warning and ignore, STOP = stop execution] |

#### Application parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-nth | NumberOfThreads  | Number of worker threads (optional, default=-2) [special values: 0 = thread pool disabled, -1 = all available threads, -2 = reasonable auto] |
|-v   | VerboseLevel     | Verbose level (optional, default=1) |

#### External config file

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
| -c  | n/a              | Valid path to external config file - in INI format (optional). Multiple config files can be provided by using multiple "-c" arguments. Config files are processed in arguments order. Content of config files are merged while repeating values are overwritten. |

#### Dynamic dispatcher parameters
| Cmd                | Description |
|:-------------------|:------------|
| --DispatchForce    | Force dispatcher to selected microarchitecture (optional, default=UNDEFINED) [x86-64, x86-64-v2, x86-64-v3, x86-64-v4]. Forcing a selection of microarchitecture level not supported by CPU will lead to "illegal instruction" exception. |
| --DispatchVerbose  | Verbose level for runtime dispatch module (optional, default=0) |

### 3.2. Verbose level

| Value | Behavior |
|:------|:---------|
| 0 | final (average) metric values only |
| 1 | 0 + configuration + detected frame numbers |
| 2 | 1 + argc/argv + frame level metric values |
| 3 | 2 + computing time (could slightly slow down computations) |
| 4 | 3 + IV specific debug data (GlobalColorShift, R2T+T2R, NumNonMasked) |
| 9 | stdout flood |

### 3.3. Examples

#### 3.3.1. Commandline parameters example

Metrics (PSNR, WS-PSNR, IV-PSNR, and IV-SSIM) of *SA_ref.yuv* and *SA_test.yuv*. Sequence resolution is 4096×2048, YUV420, 10 bits per sample. Sequence format is ERP. Mean metrics calculated for the first 20 frames will be written into QMIV.txt:  
`QMIV -i0 SA_ref.yuv -i1 SA_test.yuv -pw 4096 -ph 2048 -bd 10 -erp -nf 20 -r QMIV.txt`  
`QMIV -i0 SA_ref.yuv -i1 SA_test.yuv -ps 4096x2048 -pf yuv420p10le -erp -nf 20 -r QMIV.txt`  

Metrics (all available) of *SD_ref.yuv* and *SD_test.yuv*. Sequence resolution is 2048×1088, YUV420, 8 bits per sample. Sequence format is perspective. Mean metrics calculated for all frames will be written into results.txt:  
`QMIV -i0 SD_ref.yuv -i1 SD_test.yuv -r results.txt -pw 2048 -ph 1088 -ml "All"`  

Metrics (PSNR, WS-PSNR and IV-PSNR) of *SC_ref.yuv* and *SC_test.yuv*. Sequence resolution is 4096×2048, YUV420, 10 bits per sample. Sequence format is ERP, with lateral range equal to 90°. Mean IV-PSNR calculated for 5 frames (frames 0-4 of reference video and 10-14 of test video) will be written into o.txt:  
`QMIV -i0 SC_ref.yuv -i1 SC_test.yuv -pw 4096 -ph 2048 -bd 10 -erp -lar 90 -nf 5 -sf1 10 -ml "PSNR, WSPSNR, IVPSNR" -r o.txt`

External config file:  
`QMIV -c "config.cfg"`  

External config files with some parameters added/overwritten:  
`QMIV -c "config1st.cfg" -c "config2nd.cfg" -v 1 -nth 4`  

#### 3.3.2. Config file examples

```ini
#QMIV config example 1
InputFile0      = "SA_ref.yuv"
InputFile1      = "SA_test.yuv"
PictureWidth    = 4096
PictureHeight   = 2048
BitDepth        = 10
ChromaFormat    = 420
VerboseLevel    = 3
ResultFile      = "result.txt"
```

```ini
#QMIV config example 2
InputFile0        = "SA_ref.yuv"
InputFile1        = "SA_test.yuv"
FileFormat        = RAW
PictureSize       = 4096x2048
#PictureWidth     = 4096
#PictureHeight    = 2048
PictureFormat     = yuv420p10le
#BitDepth         = 10
#ChromaFormat     = 420
StartFrame0       = 0         
StartFrame1       = 0
NumberOfFrames    = -1
ResultFile        = "QMIV.txt"
MetricList        = "all"

#InputFileM       = "SA_mask.yuv"
#BitDepthM        = 10
#ChromaFormatM    = 420

Equirectangular   = 0
#LonRangeDeg      = 360
#LatRangeDeg      = 180

#ColorSpaceInput  = YCbCr
#ColorSpaceMetric = YCbCr

SearchRange       = 2
CmpWeightsSearch  = "4:1:1:0"
CmpWeightsAverage = "4:1:1:0"
#UnnoticeableCoef = "0:0:0:0"

NumberOfThreads   = 12
VerboseLevel      = 3
```

### 3.4. Pixel values checking notes (InvalidPelActn)

Before calculating any metric the software scans the content of YUV file in order to evaluate if all pixel values are in range `[0, MaxVal]` where `MaxVal = (1<<BitDepth) - 1`. If invalid pel is detected the software can take following actions based on InvalidPelActn parameter value:
* SKIP - disable pixel value checking
* WARN - print warning and ignore invalid pel values (may lead to unreliable metrics value)
* STOP - print warning and stop execution
* CNCL - print warning and try to conceal the pel value by clipping to highest value within bit depth range

### 3.5. Filename (filepath) parameters mismatch checking notes (NameMismatchActn)

Before opening the YUV file the software tries to derive important video parameters (resolution, bit depth and chroma format) from file name and file path. If mismatch between parameters provided from commandline (or config file) and derived values is detected, the software can take following actions based on NameMismatchActn parameter value:
* SKIP - disable checking
* WARN - print warning and ignore the mismatch
* STOP - print warning and stop execution

### 3.6. Masked mode - requirements and notes

* Resolution of the mask file has to be identical as input file.
* Allowed mask values are `0` (interpreted as inactive pixel) and `(1<<BitDepthM)-1)` (interpreted as active pixel). Behavior for other values is undefined at this moment.
* A mask with other values can be converted into a binary mask with `-mtm Threshold`: mask values greater than or equal to MaskTransParam (`-mtp`, default 128) become active pixels, the others inactive.
* The data processing functions for masked mode are not implemented with the use of SIMD instructions leading to slower computations.

### 3.7. Colorspace  modes - requirements and notes

Optional mode of the QMIV allows to define or convert the colorspace of input sequences and/or colorspace used to calculate metrics. 

The default behavior is to assume generic YCbCr colorspace for both input and metric and perform no conversion. The metric values are decorated with YCbCr or Y:Cb:Cr suffix.

The second option is to use RGB input files and calculate metric in RGB. In this case the QMIV performs no conversion and output metrics labeled as RGB, i.e., PSNR R:G:B PSNR-RGB. To enable this mode use `-csi RGB` input argument. 

When calculating metrics in RGB mode, component weights should be set equal for all three components:

```ini
CmpWeightsAverage = "1:1:1:0"
CmpWeightsSearch  = "1:1:1:0"
```

This can be archived using config file or commandline (`-cwa "1:1:1:0" -cws "1:1:1:0"`).

The other path assumes on-demand colorspace conversion. The software is able to perform RGB-to-YCbCr or YCbCr-to-RGB conversion. In this case, a specific variant of YCbCr colorspace (BT601, SMPTE170M, BT709, SMPTE240M, BT2020) have to be defined for input or metric. The valid arguments for YCbCr colorspace variants are [YCbCr_BT601, YCbCr_SMPTE170M, YCbCr_BT709, YCbCr_SMPTE240M, YCbCr_BT2020].

For example, if one wants to use input sequence in BT709 colorspace and calculate metrics in RGB, the following parameters should be used: `-csi YCbCr_BT709 -csm RGB -cwa "1:1:1:0" -cws "1:1:1:0"`.

### 3.8. PNG/BMP mode

Optional mode of the QMIV allows to calculate the metrics not for video files, but for lists of indexed images in the PNG or BMP format. 

The name of input file should contain format string as defined in C++20 std::format [ISO/IEC 14882:2020] in the form of `{:d}` with optional modifiers. The file name is determined by formatting input string using image index.

The software expects lists of consequently numbered files. The first file index can be equal to 0 or 1. The last file is detected by checking the existence of consecutive files. The first missing index is treated as the end of the file list.

Examples:
* The list of files {img001.png, img002.png, img003.png} should be specified as `img{:03d}.png`. 
* The list of files {img000.png, img001.png, img002.png, img004.png}, specified as `img{:03d}.png`, will be processed for 0,1, and 2 indexes only. The `img002.png` will be detected as the last image in list.
* The list of files {img001.bmp, img002.bmp, img003.bmp} should be specified as `img{:03d}.bmp`. 

### 3.9. pVAR metric - notes

The pVAR metric [PVAR] is selected by the `PVD` token in MetricList (e.g., `-ml "PSNR, IVPSNR, PVD"`) and is reported in the output with the `PVD` label. It evaluates the spatial variance of the error instead of its absolute magnitude (which makes it robust to global luminance shifts) and maps it to the quality score using a perceptually motivated saturation function. The score is normalized: higher values mean better quality and identical pictures give 1. The metric uses integer arithmetic only, so its value is identical on every platform and code path, and its computational cost is similar to PSNR.


## 4. References

* [IVPSNR] A. Dziembowski, D. Mieloch, J. Stankowski and A. Grzelka, "IV-PSNR - the objective quality metric for immersive video applications," in IEEE Transactions on Circuits and Systems for Video Technology, doi: [10.1109/TCSVT.2022.3179575](https://doi.org/10.1109/TCSVT.2022.3179575). [Available on authors webpage](http://multimedia.edu.pl/?page=publication&section=IV-PSNR---the-objective-quality-metric-for-immersive-video-applications)
* [IVSSIM] A. Dziembowski, W. Nowak, J. Stankowski, "IV-SSIM - The Structural Similarity Metric for Immersive Video", Applied Sciences, Vol. 14, No. 16, Aug 2024, doi: [10.3390/app14167090](https://doi.org/10.3390/app14167090). [Available on authors webpage](http://multimedia.edu.pl/?page=publication&section=IV-SSIM----The-Structural-Similarity-Metric-for-Immersive-Video)
* [IVMSSSIM] B. Sojka, J. Stankowski, A. Dziembowski, "Multi-scale structural similarity with geometric alignment for immersive video quality assessment," International Conference on Image Processing, ICIP 2026, Tampere, Finland, 13-17.09.2026
* [PVAR] A. Dziembowski, "pVAR: An ultra low complexity image quality assessment via spatial error variance," IEEE Signal Processing Letters, Vol. 33, 2026, pp. 2710-2714, doi: [10.1109/LSP.2026.3708417](https://doi.org/10.1109/LSP.2026.3708417)
