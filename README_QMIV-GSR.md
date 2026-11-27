# QMIV-GS-renderer
Gaussian splat renderer of the QMIV framework  

----  

## 1. Description

QMIV-GS-renderer (source directory `apps/QMIV-GSR`) is the application of the QMIV framework that renders views of a 3D Gaussian splat (3DGS) representation [3DGS] and writes them to files. It uses the same built-in CPU renderer as QMIV-GS (the application for the quality assessment of Gaussian splat representations), so the pictures it writes are identical to the pictures QMIV-GS renders and assesses.

The application allows to:
* render views of a cloud or a sequence of clouds for the cameras from a camera parameters file (PLY, COLMAP or INI format),
* render views along a camera path (pose trace) given as a CSV file,
* write the rendered views as RAW (YUV or RGB), PNG or BMP files, e.g., to assess them with other tools or to watch them,
* print XXH128 hashes of the input clouds and of the rendered pictures, which allows to verify that results obtained on different machines are produced from the same data.

The authors, the license, the building instructions and the changelog of the QMIV framework are given in [README.md](README.md).

### 1.1. Renderer

The built-in renderer implements the 3DGS rasterization [3DGS]: the splats are projected onto the image plane, assigned to 32×32 pixel tiles, sorted by depth within each tile, and alpha-blended front to back. View-dependent color is evaluated from spherical harmonics of degree up to 3. The rasterization is implemented with AVX2 and AVX512 instructions (a portable implementation is used on other platforms, including ARM64). The rendering is multithreaded.

The renderer is deterministic: the rendered pictures are bit-identical regardless of the number of threads and of the instruction set used (selected by the runtime dispatcher, see Section 4 of [README.md](README.md)).

## 2. Building

QMIV-GS-renderer is built together with the other applications of the QMIV framework, as described in Section 4 of [README.md](README.md). The resulting executable is named `QMIV-GS-renderer`.

## 3. Usage

### 3.1. Commandline parameters

Commandline parameters are parsed from left to right. Multiple config files are allowed. Moreover, config file parameters can be overridden or added via commandline.  

#### General parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-i   | InputFile        | File path - cloud (PLY). For a sequence of clouds - fmtlib pattern with the {FrameIdx} token, e.g. "cloud_{FrameIdx:04d}.ply" (see Section 3.3) |
|-cpf | CamParamsFile    | Camera parameters - file path (PLY, INI) or directory path (COLMAP) (see Section 3.4) |
|-cpm | CamParamsFormat  | Format of camera parameters (optional, default=PLY) [PLY, COLMAP, INI] |
|-cpt | CamNameTemplate  | Template of camera section names (optional, used for INI format only, default="[CamIdStr]") |
|-ptf | PoseTraceFile    | File path - CSV pose trace (optional). If provided, the views are rendered along the pose trace instead of the input cameras (see Section 3.5) |
|-ps  | PictureSize      | Size of rendered pictures (WxH) |
|-bd  | BitDepth         | Bit depth of rendered pictures (optional, default=8, range 8-14) |
|-sf  | StartFrame       | Start frame (optional, auto=-1, default=-1) |
|-nf  | NumberOfFrames   | Number of frames to be processed (optional, all=-1, default=-1) |

#### Rendered pictures output parameters

| Cmd | ParamName         | Description |
|:----|:------------------|:------------|
|-s   | SynthFile         | File path pattern - rendered pictures (optional, nothing is written if not provided). The pattern can contain {W}, {H} and {ViewIdx} tokens, e.g. "Render_{W}x{H}_V{ViewIdx:02d}.yuv" (see Section 3.6) |
|-sfv | SynthFilePerView  | Use a separate output file for each camera (optional, default=1). Ignored in posetrace mode (one file) |
|-ffs | FileFormatS       | File format of rendered pictures (optional, default=RAW) [RAW, PNG, BMP] |
|-css | ColorSpaceFileS   | Color space of rendered pictures (optional, default=YCbCr_BT709) [RGB, BGR, GBR, YCbCr_BT601, YCbCr_SMPTE170M, YCbCr_BT709, YCbCr_SMPTE240M, YCbCr_BT2020] |
|-cfs | ChromaFormatFileS | Chroma format of rendered pictures (optional, default=420, used for YCbCr only) [400, 420, 422, 444] |
|-gh  | GatherHash        | Calculate hashes of the input cloud(s) and rendered pictures (optional, default=1) (see Section 3.7) |

#### Application parameters

| Cmd | ParamName        | Description |
|:----|:-----------------|:------------|
|-nth | NumberOfThreads  | Number of worker threads (optional, default=-2) [special values: 0 = thread pool disabled, -1 = all available threads, -2 = reasonable auto (for QMIV-GS-renderer: all available threads)] |
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
| 0 | final hash values only |
| 1 | 0 + configuration + detected/processed frame numbers + list of input and output files |
| 2 | 1 + argc/argv + working directory + build info |
| 3 | 2 + computing time (loading, hashing, rendering and writing) |
| 4 | 3 + per-frame debug data (hashes of clouds and rendered pictures, cache loading progress) |

### 3.3. Input clouds

The clouds are read from PLY files in the format used by 3D Gaussian Splatting [3DGS]:
* `format binary_little_endian 1.0`, a single `element vertex` with `float` properties only,
* required properties: `x`, `y`, `z`, `f_dc_0`, `f_dc_1`, `f_dc_2`, `opacity`, `scale_0`, `scale_1`, `scale_2`, `rot_0`, `rot_1`, `rot_2`, `rot_3`,
* optional properties: `f_rest_0` ... `f_rest_44` (spherical harmonics of degree 1 to 3); the degree is detected from the number of `f_rest_*` properties,
* comment lines are allowed only for camera parameters (see Section 3.4, PLY format); any other comment line in the header is reported as an error.

A sequence of clouds (dynamic content) is given by a file name pattern with the `{FrameIdx}` token formatted with fmtlib/C++20 std::format syntax, e.g., `cloud_{FrameIdx:04d}.ply` for the files cloud_0000.ply, cloud_0001.ply, ... A single (static) cloud can be given by its file name without any token.

The first available frame index is detected automatically (StartFrame=-1), and the sequence ends at the first missing index. By default, all available frames are processed.

### 3.4. Camera parameters

The cameras used for rendering are read from CamParamsFile (required). Three formats are supported (CamParamsFormat):

* **PLY** (default) - camera parameters stored in the header of a PLY file (e.g., the input cloud), as comment lines. The first line `comment camera_position_header <column names>` defines the columns, and each following `comment camera_position <values>` line defines one camera. The required columns are: `imageid`, `cameraid`, `pos.x`, `pos.y`, `pos.z`, `quat.w`, `quat.x`, `quat.y`, `quat.z`, `focal.x`, `focal.y`, `center.x`, `center.y`, `width`, `height`, `name` (any order). The intrinsic parameters are defined for the given width and height and are rescaled to PictureSize.
* **COLMAP** - a COLMAP model directory [COLMAP] containing `cameras.txt` and `images.txt` (or `cameras.bin` and `images.bin`). Every image of the model is one camera. Supported camera models: SIMPLE_PINHOLE, PINHOLE, SIMPLE_RADIAL, RADIAL, OPENCV, FULL_OPENCV. The intrinsic parameters are used as they are, so PictureSize should be equal to the resolution of the cameras in the model.
* **INI** - an INI file with one section per camera. The section name is defined by CamNameTemplate, in which `[CamIdStr]` is replaced by the camera identifier (by default the section name is the camera identifier). Every section contains: `Resolution` (WxH), `Projection` (Perspective), `Intrinsic` (9 values of the 3x3 intrinsic matrix, row by row), `Extrinsic` (12 values of the 3x4 extrinsic matrix, row by row) and optionally `Distortion` (OpenCV distortion coefficients). The intrinsic parameters are used as they are, so PictureSize should be equal to the resolution of the cameras.

Every camera is rendered for every frame, so the number of rendered pictures is NumberOfFrames × NumberOfCameras.

### 3.5. Posetrace mode

If PoseTraceFile is provided, the views are rendered along the camera path defined in the CSV file instead of the cameras from CamParamsFile. Every row of the file defines one pose: `X,Y,Z,Yaw,Pitch,Roll`, where X, Y, Z is the camera position and Yaw, Pitch, Roll are angles in degrees (rotation R = Rz(Yaw)·Ry(Pitch)·Rx(Roll), as in the MIV/TMIV reference frame convention [MIV]). The first row can be a header (it is skipped if it does not contain six numbers).

The intrinsic parameters of all poses are taken from the first camera of CamParamsFile, so CamParamsFile is still required.

One picture is rendered for every pose. The pose index advances the frame of the cloud sequence. If the pose trace is longer than the cloud sequence, the frames are mirrored back and forth (ping-pong: 0, 1, ..., N-1, N-2, ..., 0, 1, ...). In this mode all processed cloud frames are loaded into memory before rendering.

### 3.6. Output of rendered pictures

The renderer produces RGB pictures. If SynthFile is provided, they are written to files:
* the file name pattern can contain `{W}`, `{H}` (picture size) and `{ViewIdx}` (index of the camera) tokens, e.g., `Render_{W}x{H}_V{ViewIdx:02d}.yuv`,
* RAW format: with SynthFilePerView=1 (default), one file is written for each camera, containing all processed frames; with SynthFilePerView=0, all pictures are written into one file; in the posetrace mode, all poses are written into one file,
* PNG and BMP formats: one file is written for each picture, so for more than one frame (or pose) the pattern has to contain a frame index field with doubled braces, e.g., `Render_V{ViewIdx:02d}_{{:04d}}.png` (files Render_V00_0000.png, Render_V00_0001.png, ...),
* ColorSpaceFileS selects RGB (or BGR, GBR order) output or conversion to one of the YCbCr variants (default YCbCr BT.709); for YCbCr, ChromaFormatFileS selects the chroma format (4:2:0 requires even width and height, 4:2:2 requires even width; this check is done also for RGB output, so for an odd PictureSize use `-cfs 444`),
* BitDepth defines the bit depth of the written pictures (8-14 bits, more than 8 bits are written as 16-bit little endian samples in RAW format).

If SynthFile is not provided, the views are rendered but not written (e.g., to measure the rendering time or to calculate the hashes).

### 3.7. Reproducibility hashes

At the end of processing (unless GatherHash=0), the software prints XXH128 hashes [XXH]:
* `HashCloud` - hash of the input cloud, calculated over the decoded splats of all processed frames,
* `HashSynth` - hash of all rendered pictures.

The hashes do not depend on the number of threads or the instruction set used, so they can be used to check whether results obtained on different machines are based on identical data and identical rendering. They are calculated in the same way as `HashCloud0`/`HashCloud1` and `HashSynth0`/`HashSynth1` of QMIV-GS, so the hashes of QMIV-GS-renderer and QMIV-GS are equal for the same cloud, cameras and PictureSize. With VerboseLevel=4, the hashes of every frame and every rendered picture are printed.

### 3.8. Examples

#### 3.8.1. Commandline parameters example

Views of the sequence *cloud_XXXX.ply* rendered at 1920×1080 for all cameras from a COLMAP model in the *sparse* directory, written to 8-bit YUV 4:2:0 files, one file per camera:  
`QMIV-GS-renderer -i "cloud_{FrameIdx:04d}.ply" -cpm COLMAP -cpf sparse -ps 1920x1080 -s "Render_{W}x{H}_V{ViewIdx:02d}.yuv"`  

Static cloud, cameras stored in its header, rendered at 3840×2160 and written to RGB PNG files:  
`QMIV-GS-renderer -i cloud.ply -cpf cloud.ply -ps 3840x2160 -s "Render_V{ViewIdx:02d}.png" -ffs PNG -css RGB`  

Pose trace through 10 frames of the sequence, starting from frame 5, written to one 10-bit YUV 4:2:0 file:  
`QMIV-GS-renderer -i "cloud_{FrameIdx:04d}.ply" -sf 5 -nf 10 -cpm COLMAP -cpf sparse -ptf posetrace.csv -ps 1920x1080 -bd 10 -s "PoseTrace_{W}x{H}_10b.yuv"`  

Cameras from an INI file with sections named `v00`, `v01`, ..., rendered without writing (hashes and computing time only):  
`QMIV-GS-renderer -i "cloud_{FrameIdx:04d}.ply" -cpm INI -cpf cameras.cfg -ps 1920x1080 -v 3`  

#### 3.8.2. Config file example

```ini
#QMIV-GS-renderer config example
InputFile         = "cloud_{FrameIdx:04d}.ply"
CamParamsFormat   = COLMAP
CamParamsFile     = "sparse"
#PoseTraceFile    = "posetrace.csv"
PictureSize       = 1920x1080
BitDepth          = 8
StartFrame        = -1
NumberOfFrames    = -1

SynthFile         = "Render_{W}x{H}_V{ViewIdx:02d}.yuv"
SynthFilePerView  = 1
FileFormatS       = RAW
ColorSpaceFileS   = YCbCr_BT709
ChromaFormatFileS = 420

GatherHash        = 1
NumberOfThreads   = -2
VerboseLevel      = 1
```

## 4. References

* [3DGS] B. Kerbl, G. Kopanas, T. Leimkühler, G. Drettakis, "3D Gaussian Splatting for Real-Time Radiance Field Rendering," ACM Transactions on Graphics, Vol. 42, No. 4, 2023, doi: [10.1145/3592433](https://doi.org/10.1145/3592433)
* [COLMAP] J. L. Schönberger, J.-M. Frahm, "Structure-from-Motion Revisited," IEEE Conference on Computer Vision and Pattern Recognition (CVPR), 2016, pp. 4104-4113
* [MIV] ISO/IEC 23090-12, "Information technology — Coded representation of immersive media — Part 12: MPEG immersive video"
* [XXH] xxHash - Extremely fast non-cryptographic hash algorithm, https://xxhash.com
