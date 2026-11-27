# QMIV-GS
Quality Metrics for Immersive Video - Gaussian Splatting  

----  

## 1. Description

QMIV-GS is the application of the QMIV framework for the objective quality assessment of 3D Gaussian splat (3DGS) representations [3DGS]. It renders the test cloud and the reference cloud with a built-in CPU renderer for the same cameras and calculates quality metrics between the rendered views. No external renderer is needed: the clouds (PLY files) and the camera parameters are the only inputs.

All metrics of the QMIV framework are available: 
  * IV-PSNR    - Immersive Video - Peak Signal-to-Noise Ratio [IVPSNR]
  * IV-SSIM    - Immersive Video - Structural Similarity Index Measure [IVSSIM]
  * IV-MS-SSIM - Immersive Video - Multi Scale Structural Similarity Index Measure [IVMSSSIM]
  * PSNR       - Peak Signal-to-Noise Ratio
  * WS-PSNR    - Spherical Weighted - Peak Signal-to-Noise Ratio
  * SSIM       - Structural Similarity Index Measure
  * MS-SSIM    - Multi Scale Structural Similarity Index Measure
  * pVAR       - Perceptual Variance of Distortion [PVAR] (selected as `PVD` in MetricList)
  * MSE        - Mean Squared Error

Every metric is calculated twice: in RGB (the native output of the renderer) and in YCbCr (BT.709).

The views can be rendered in two modes:
* **input views** (default) - for every frame of the cloud sequence, the views of all cameras from the camera parameters file are rendered from both clouds and compared,
* **posetrace** - the views are rendered along a camera path (pose trace) given as a CSV file, one pose per pair of rendered pictures.

Optionally, the metrics can be calculated in masked mode, with the mask derived during rendering from the alpha (opacity) of the rendered pictures. This restricts the assessment to the rendered content (e.g., an object) and skips the empty background.

The rendered views can be saved to files (RAW YUV/RGB, PNG or BMP).

The authors, the license, the building instructions and the changelog of the QMIV framework are given in [README.md](README.md).

### 1.1. Renderer

The built-in renderer implements the 3DGS rasterization [3DGS]: the splats are projected onto the image plane, assigned to 32×32 pixel tiles, sorted by depth within each tile, and alpha-blended front to back. View-dependent color is evaluated from spherical harmonics of degree up to 3. The rasterization is implemented with AVX2 and AVX512 instructions (a portable implementation is used on other platforms, including ARM64). The rendering is multithreaded.

The renderer is deterministic: the rendered pictures are bit-identical regardless of the number of threads and of the instruction set used (selected by the runtime dispatcher, see Section 4 of [README.md](README.md)). The software prints hashes of the input clouds and of the rendered pictures, which allows to verify that results obtained on different machines are produced from the same data (see Section 3.8).

## 2. Building

QMIV-GS is built together with the other applications of the QMIV framework, as described in Section 4 of [README.md](README.md). The resulting executable is named `QMIV-GS`.

## 3. Usage

### 3.1. Commandline parameters

Commandline parameters are parsed from left to right. Multiple config files are allowed. Moreover, config file parameters can be overridden or added via commandline. Parameters without the "Cmd" form can be set in a config file only.

#### General parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-i0  | InputFile0       | File path - test cloud (PLY). For a sequence of clouds - fmtlib pattern with the {FrameIdx} token, e.g. "cloudT_{FrameIdx:04d}.ply" (see Section 3.3) |
|-i1  | InputFile1       | File path - reference cloud (PLY), as above |
|-cpf | CamParamsFile    | Camera parameters - file path (PLY, INI) or directory path (COLMAP) (see Section 3.4) |
|-cpm | CamParamsFormat  | Format of camera parameters (optional, default=PLY) [PLY, COLMAP, INI] |
|-cpt | CamNameTemplate  | Template of camera section names (optional, used for INI format only, default="[CamIdStr]") |
|-ptf | PoseTraceFile    | File path - CSV pose trace (optional). If provided, the views are rendered along the pose trace instead of the input cameras (see Section 3.5) |
|-ps  | PictureSize      | Size of rendered pictures (WxH) |
|-bd  | BitDepth         | Bit depth of rendered pictures (optional, default=8, range 8-14) |
|-sf0 | StartFrame0      | Start frame of test cloud sequence (optional, auto=-1, default=-1) |
|-sf1 | StartFrame1      | Start frame of reference cloud sequence (optional, auto=-1, default=-1) |
|-nf  | NumberOfFrames   | Number of frames to be processed (optional, all=-1, default=-1) |
|-r   | ResultFile       | Output file path for printing result(s) (optional) |
|-ml  | MetricList       | List of quality metrics to be calculated, must be coma separated, quotes are required. "All" enables all available metrics. [MSE, PSNR, WSPSNR, IVPSNR, SSIM, MSSSIM, IVSSIM, IVMSSSIM, PVD] (optional, default="PSNR, IVPSNR, IVSSIM") |

#### Rendered pictures output parameters

| Cmd | ParamName         | Description |
|:----|:------------------|:------------|
|-s0  | SynthFile0        | File path pattern - pictures rendered from InputFile0 (optional). The pattern can contain {W}, {H} and {ViewIdx} tokens, e.g. "Render_{W}x{H}_V{ViewIdx:02d}.yuv" (see Section 3.7) |
|-s1  | SynthFile1        | File path pattern - pictures rendered from InputFile1 (optional). Both SynthFile0 and SynthFile1 or none have to be provided |
|-sfv | SynthFilePerView  | Use a separate output file for each camera (optional, default=1). Ignored in posetrace mode (one file) |
|-ffs | FileFormatS       | File format of rendered pictures (optional, default=RAW) [RAW, PNG, BMP] |
|-css | ColorSpaceFileS   | Color space of rendered pictures (optional, default=YCbCr_BT709) [RGB, BGR, GBR, YCbCr_BT601, YCbCr_SMPTE170M, YCbCr_BT709, YCbCr_SMPTE240M, YCbCr_BT2020] |
|-cfs | ChromaFormatFileS | Chroma format of rendered pictures (optional, default=420, used for YCbCr only) [400, 420, 422, 444] |

#### Masked mode parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-smm | SynthMaskMode    | Mask derived during rendering (optional, default=None) [None, Occupancy, Alpha] (see Section 3.6) |

#### IV specific parameters

| Cmd | ParamName              | Description |
|:----|:-----------------------|:------------|
|-sr  | SearchRange            | IV-metric search range around center point (optional, default=2 --> 5x5) |
|-unc | UnnoticeableCoef       | IV-metric unnoticeable color difference threshold coeff ("Lm:Cb:Cr:0" or "R:G:B:0" - per component coeff, default="0.01:0.01:0.01:0", quotes are mandatory) |
|     | CmpWeightsSearchRGB    | IV-metric component weights used during search in RGB ("R:G:B:0" - per component integer weights, default="1:1:1:0", quotes are mandatory) |
|     | CmpWeightsSearchYCbCr  | IV-metric component weights used during search in YCbCr ("Lm:Cb:Cr:0" - per component integer weights, default="4:1:1:0", quotes are mandatory) |
|     | CmpWeightsAverageRGB   | IV-metric component weights used during averaging in RGB ("R:G:B:0" - per component integer weights, default="1:1:1:0", quotes are mandatory) |
|     | CmpWeightsAverageYCbCr | IV-metric component weights used during averaging in YCbCr ("Lm:Cb:Cr:0" - per component integer weights, default="4:1:1:0", quotes are mandatory) |

#### Structural similarity specific parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-ssm | StructSimMode    | Calculation mode and structure variant (optional, default=BlockAveraged) [BlockAveraged, RegularGaussianInt, RegularAveraged] |
|-ssb | StructSimBrdExt  | Border extension mode (optional, applies to Regular mode only, default=None) [None, Nearest, Reflect, Mirror, Zero] (see scipy.ndimage.generic_filter) |
|-sss | StructSimStride  | Stride between pixels/windows (optional, default=4) |
|-ssw | StructSimWindow  | Size of structure window (optional, applies to Block modes only, default=8) [8,16,32] |

#### Application parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-nth | NumberOfThreads  | Number of worker threads (optional, default=-2) [special values: 0 = thread pool disabled, -1 = all available threads, -2 = reasonable auto (for QMIV-GS: all available threads)] |
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
| 3 | 2 + computing time (loading, hashing, rendering, writing and metrics, could slightly slow down computations) |
| 4 | 3 + IV specific debug data (GlobalColorShift, R2T+T2R, NumNonMasked) + per-frame hashes of clouds and rendered pictures |
| 9 | stdout flood |

### 3.3. Input clouds

The clouds are read from PLY files in the format used by 3D Gaussian Splatting [3DGS]:
* `format binary_little_endian 1.0`, a single `element vertex` with `float` properties only,
* required properties: `x`, `y`, `z`, `f_dc_0`, `f_dc_1`, `f_dc_2`, `opacity`, `scale_0`, `scale_1`, `scale_2`, `rot_0`, `rot_1`, `rot_2`, `rot_3`,
* optional properties: `f_rest_0` ... `f_rest_44` (spherical harmonics of degree 1 to 3); the degree is detected from the number of `f_rest_*` properties,
* comment lines are allowed only for camera parameters (see Section 3.4, PLY format); any other comment line in the header is reported as an error.

A sequence of clouds (dynamic content) is given by a file name pattern with the `{FrameIdx}` token formatted with fmtlib/C++20 std::format syntax, e.g., `cloud_{FrameIdx:04d}.ply` for the files cloud_0000.ply, cloud_0001.ply, ... A single (static) cloud can be given by its file name without any token.

The first available frame index is detected automatically (StartFrame=-1), and the sequence ends at the first missing index. StartFrame0 and StartFrame1 allow to select different first frames for the test and the reference sequence. By default, all frames available in both sequences are processed.

### 3.4. Camera parameters

The cameras used for rendering are read from CamParamsFile. Three formats are supported (CamParamsFormat):

* **PLY** (default) - camera parameters stored in the header of a PLY file (e.g., one of the input clouds), as comment lines. The first line `comment camera_position_header <column names>` defines the columns, and each following `comment camera_position <values>` line defines one camera. The required columns are: `imageid`, `cameraid`, `pos.x`, `pos.y`, `pos.z`, `quat.w`, `quat.x`, `quat.y`, `quat.z`, `focal.x`, `focal.y`, `center.x`, `center.y`, `width`, `height`, `name` (any order). The intrinsic parameters are defined for the given width and height and are rescaled to PictureSize.
* **COLMAP** - a COLMAP model directory [COLMAP] containing `cameras.txt` and `images.txt` (or `cameras.bin` and `images.bin`). Every image of the model is one camera. Supported camera models: SIMPLE_PINHOLE, PINHOLE, SIMPLE_RADIAL, RADIAL, OPENCV, FULL_OPENCV. The intrinsic parameters are used as they are, so PictureSize should be equal to the resolution of the cameras in the model.
* **INI** - an INI file with one section per camera. The section name is defined by CamNameTemplate, in which `[CamIdStr]` is replaced by the camera identifier (by default the section name is the camera identifier). Every section contains: `Resolution` (WxH), `Projection` (Perspective), `Intrinsic` (9 values of the 3x3 intrinsic matrix, row by row), `Extrinsic` (12 values of the 3x4 extrinsic matrix, row by row) and optionally `Distortion` (OpenCV distortion coefficients). The intrinsic parameters are used as they are, so PictureSize should be equal to the resolution of the cameras.

In the input views mode, every camera is rendered for every frame, so the number of rendered picture pairs is NumberOfFrames × NumberOfCameras.

### 3.5. Posetrace mode

If PoseTraceFile is provided, the views are rendered along the camera path defined in the CSV file instead of the cameras from CamParamsFile. Every row of the file defines one pose: `X,Y,Z,Yaw,Pitch,Roll`, where X, Y, Z is the camera position and Yaw, Pitch, Roll are angles in degrees (rotation R = Rz(Yaw)·Ry(Pitch)·Rx(Roll), as in the MIV/TMIV reference frame convention [MIV]). The first row can be a header (it is skipped if it does not contain six numbers).

The intrinsic parameters of all poses are taken from the first camera of CamParamsFile, so CamParamsFile is still required.

For every pose, one picture is rendered from the test cloud and one from the reference cloud. The pose index advances the frame of the cloud sequence. If the pose trace is longer than the cloud sequence, the frames are mirrored back and forth (ping-pong: 0, 1, ..., N-1, N-2, ..., 0, 1, ...). In this mode all processed cloud frames are loaded into memory before rendering.

### 3.6. Masked mode - requirements and notes

In the masked mode (SynthMaskMode other than None), the mask is derived during rendering from the accumulated alpha (opacity) of each pixel. The masks of the test and the reference picture are combined by taking the maximum (i.e., a pixel is taken into account if it is covered in any of the two pictures):
* **Occupancy** - binary mask: pixels with alpha of at least 0.2 are active, the others are inactive,
* **Alpha** - weighting mask: every pixel is weighted by its alpha.

Requirements:
* the structural similarity metrics (SSIM, MS-SSIM, IV-SSIM, IV-MS-SSIM) are not available in the masked mode, so MetricList has to be provided (the default list contains IV-SSIM),
* the masked mode requires `-ssm RegularAveraged -sss 1` (also when no structural similarity metric is calculated).

The metric values calculated in the masked mode are labeled with the `-M` suffix (e.g., `PSNR-M`, `IVPSNR-M`).

### 3.7. Colorspace and output of rendered pictures

The renderer produces RGB pictures. Every selected metric is calculated twice:
* in RGB, with component weights CmpWeightsSearchRGB and CmpWeightsAverageRGB (default "1:1:1:0"),
* in YCbCr (BT.709 conversion), with component weights CmpWeightsSearchYCbCr and CmpWeightsAverageYCbCr (default "4:1:1:0").

Both sets of values are printed and written to ResultFile, labeled as RGB and YCbCr (or R:G:B and Y:Cb:Cr for per-component values).

If SynthFile0 and SynthFile1 are provided, the rendered pictures are written to files:
* the file name pattern can contain `{W}`, `{H}` (picture size) and `{ViewIdx}` (index of the camera) tokens, e.g., `Render_{W}x{H}_V{ViewIdx:02d}.yuv`,
* RAW format: with SynthFilePerView=1 (default), one file is written for each camera, containing all processed frames; with SynthFilePerView=0, all pictures are written into one file; in the posetrace mode, all poses are written into one file,
* PNG and BMP formats: one file is written for each picture, so for more than one frame (or pose) the pattern has to contain a frame index field with doubled braces, e.g., `Render_V{ViewIdx:02d}_{{:04d}}.png` (files Render_V00_0000.png, Render_V00_0001.png, ...),
* ColorSpaceFileS selects RGB (or BGR, GBR order) output or conversion to one of the YCbCr variants; for YCbCr, ChromaFormatFileS selects the chroma format (4:2:0 requires even width and height, 4:2:2 requires even width; this check is done also for RGB output, so for an odd PictureSize use `-cfs 444`),
* BitDepth defines the bit depth of the written pictures (8-14 bits, more than 8 bits are written as 16-bit little endian samples in RAW format).

### 3.8. Reproducibility hashes

At the end of processing, the software prints XXH128 hashes [XXH]:
* `HashCloud0`, `HashCloud1` - hashes of the test and reference clouds, calculated over the decoded splats of all processed frames,
* `HashSynth0`, `HashSynth1` - hashes of all pictures rendered from the test and reference clouds.

The hashes do not depend on the number of threads or the instruction set used, so they can be used to check whether results obtained on different machines are based on identical data and identical rendering. With VerboseLevel=4, the hashes of every frame and every rendered picture are printed.

### 3.9. Examples

#### 3.9.1. Commandline parameters example

Default metrics (PSNR, IV-PSNR and IV-SSIM) of the decoded sequence *dec_XXXX.ply* against the source sequence *src_XXXX.ply*, rendered for all cameras from a COLMAP model in the *sparse* directory, at 1920×1080. Mean metrics will be written into r.txt:  
`QMIV-GS -i0 "dec_{FrameIdx:04d}.ply" -i1 "src_{FrameIdx:04d}.ply" -cpm COLMAP -cpf sparse -ps 1920x1080 -r r.txt`  

As above, with all metrics, for the first 5 frames only:  
`QMIV-GS -i0 "dec_{FrameIdx:04d}.ply" -i1 "src_{FrameIdx:04d}.ply" -cpm COLMAP -cpf sparse -ps 1920x1080 -nf 5 -ml "All" -r r.txt`  

Static clouds, cameras stored in the header of the reference PLY file, rendered at 3840×2160, PSNR and IV-PSNR in masked mode (alpha-based mask):  
`QMIV-GS -i0 dec.ply -i1 src.ply -cpf src.ply -ps 3840x2160 -ml "PSNR, IVPSNR" -smm Alpha -ssm RegularAveraged -sss 1 -r r.txt`  

Pose trace mode (intrinsic parameters from the first camera of the COLMAP model), rendered pictures written to 8-bit YUV 4:2:0 files:  
`QMIV-GS -i0 "dec_{FrameIdx:04d}.ply" -i1 "src_{FrameIdx:04d}.ply" -cpm COLMAP -cpf sparse -ptf posetrace.csv -ps 1920x1080 -s0 "dec_{W}x{H}.yuv" -s1 "src_{W}x{H}.yuv" -r r.txt`  

Views rendered for all cameras written to PNG files, one file per picture:  
`QMIV-GS -i0 "dec_{FrameIdx:04d}.ply" -i1 "src_{FrameIdx:04d}.ply" -cpm COLMAP -cpf sparse -ps 1920x1080 -s0 "dec_V{ViewIdx:02d}_{{:04d}}.png" -s1 "src_V{ViewIdx:02d}_{{:04d}}.png" -ffs PNG -css RGB`  

External config file with some parameters added/overwritten:  
`QMIV-GS -c "config.cfg" -v 3 -nth 8`  

#### 3.9.2. Config file example

In config files, lists (MetricList) are given without quotes.

```ini
#QMIV-GS config example
InputFile0             = "dec_{FrameIdx:04d}.ply"
InputFile1             = "src_{FrameIdx:04d}.ply"
CamParamsFormat        = COLMAP
CamParamsFile          = "sparse"
#PoseTraceFile         = "posetrace.csv"
PictureSize            = 1920x1080
BitDepth               = 8
StartFrame0            = -1
StartFrame1            = -1
NumberOfFrames         = -1
ResultFile             = "QMIV-GS.txt"
MetricList             = PSNR, IVPSNR, IVSSIM, PVD

#SynthFile0            = "dec_{W}x{H}_V{ViewIdx:02d}.yuv"
#SynthFile1            = "src_{W}x{H}_V{ViewIdx:02d}.yuv"
#ColorSpaceFileS       = YCbCr_BT709
#ChromaFormatFileS     = 420

SynthMaskMode          = None

SearchRange            = 2
CmpWeightsSearchYCbCr  = "4:1:1:0"
CmpWeightsAverageYCbCr = "4:1:1:0"
CmpWeightsSearchRGB    = "1:1:1:0"
CmpWeightsAverageRGB   = "1:1:1:0"

NumberOfThreads        = -2
VerboseLevel           = 1
```

## 4. References

* [3DGS] B. Kerbl, G. Kopanas, T. Leimkühler, G. Drettakis, "3D Gaussian Splatting for Real-Time Radiance Field Rendering," ACM Transactions on Graphics, Vol. 42, No. 4, 2023, doi: [10.1145/3592433](https://doi.org/10.1145/3592433)
* [IVPSNR] A. Dziembowski, D. Mieloch, J. Stankowski and A. Grzelka, "IV-PSNR – the objective quality metric for immersive video applications," in IEEE Transactions on Circuits and Systems for Video Technology, doi: [10.1109/TCSVT.2022.3179575](https://doi.org/10.1109/TCSVT.2022.3179575)
* [IVSSIM] A. Dziembowski, W. Nowak, J. Stankowski, "IV-SSIM - The Structural Similarity Metric for Immersive Video", Applied Sciences, Vol. 14, No. 16, Aug 2024, doi: [10.3390/app14167090](https://doi.org/10.3390/app14167090)
* [IVMSSSIM] B. Sojka, J. Stankowski, A. Dziembowski, "Multi-scale structural similarity with geometric alignment for immersive video quality assessment," International Conference on Image Processing, ICIP 2026, Tampere, Finland, 13-17.09.2026
* [PVAR] A. Dziembowski, "pVAR: An ultra low complexity image quality assessment via spatial error variance," IEEE Signal Processing Letters, Vol. 33, 2026, pp. 2710-2714, doi: [10.1109/LSP.2026.3708417](https://doi.org/10.1109/LSP.2026.3708417)
* [COLMAP] J. L. Schönberger, J.-M. Frahm, "Structure-from-Motion Revisited," IEEE Conference on Computer Vision and Pattern Recognition (CVPR), 2016, pp. 4104-4113
* [MIV] ISO/IEC 23090-12, "Information technology — Coded representation of immersive media — Part 12: MPEG immersive video"
* [XXH] xxHash - Extremely fast non-cryptographic hash algorithm, https://xxhash.com
