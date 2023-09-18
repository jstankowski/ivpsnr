# IV-PSNR software - example

## 1. Description

The provided example allows to quickly test the software and calculate metrics (IV-PSNR, WS-PSNR and PSNR) for provided data.

The test data was selected based on *Fencing* test sequence [Fencing]. Since *Fencing* is a multiview sequence captured for research purposes related to immersive television, the sequence consists of 10 views. We chose one of them - view 8 (v8). In order to keep data size reasonably small only first frame was provided. 

There are a set of four pictures:

| File                                                   | Description |
| :----------------------------------------------------- | :---------- |
| **Fencing_v8_original_1920x1080_yuv420p.yuv**          | 1st frame from original, uncompressed sequence. It is a reference for calculating quality metrics. |
| **Fencing_v8_origJpgCompressed_1920x1080_yuv420p.yuv** | 1st frame from original sequence which was compressed using JPEG. It contains typical distortion (block effect). |
| **Fencing_v8_synthesized_1920x1080_yuv420p.yuv**       | 1st frame from sequence synthesized from other views (v7 and v9). It contains typical synthesis related artifacts i.e. slight color changes and pixel shift. |
| **Fencing_v8_objectsMask_1920x1080_yuv420p.yuv**       | TODO mask |

## 2. Displaying pictures (sequences) provided in "yuv" format

The provided examples are stored as "yuv" files. This is common format for raw (uncompressed) video used by video compression experts (MPEG, JCT-VC, etc.). The easiest way to display the content of such file is to use *ffplay* utility whitch is a port of *FFmpeg* suite. Binary version of FFmpeg (including ffplay) can be downloaded from projects webpage (https://www.ffmpeg.org/download.html).  
* Linux users can install *FFmpeg* from distribution repositories (i.e. in case of Debian-derived distributions user can use the following command: `sudo apt install ffmpeg`).  
* Windows users are encouraged to copy *ffplay.exe* executable to "examples" folder or adding location of *FFmpeg* binaries to path. The path to *FFmpeg* binaries can be added in active *Command Prompt* window by a command `SET PATH=%PATH%;"<PATH_TO_FFMPEG_BINARIES>"` (where <PATH_TO_FFMPEG_BINARIES> needs to be replaced by appropriate location).

The general commandline syntax for displaing "yuv" files is following:  
`ffplay -f rawvideo -pixel_format <PIXEL_FORMAT> -video_size <RESOLUTION> <FILENAME>`  
Since "yuv" files does not contain any metadata (or even any header) all parameters (like resolution, bit depth, pixel format) must be provided externally. 

In order to display attached files following commands can be used:  
`ffplay -f rawvideo -pixel_format yuv420p -video_size 1920x1080 Fencing_v8_original_1920x1080_yuv420p.yuv`  
`ffplay -f rawvideo -pixel_format yuv420p -video_size 1920x1080 Fencing_v8_origJpgCompressed_1920x1080_yuv420p.yuv`  
`ffplay -f rawvideo -pixel_format yuv420p -video_size 1920x1080 Fencing_v8_synthesized_1920x1080_yuv420p.yuv`  
`ffplay -f rawvideo -pixel_format yuv420p -video_size 1920x1080 Fencing_v8_objectsMask_1920x1080_yuv420p.yuv`  

## 3. Calculating metrics

In order to calculate IV-PSNR (as well as WS-PSNR and PSNR) the following scripts should be executed:  
* *calculate_IV-PSNR.bat* - for Windows users
* *calculate_IV-PSNR.sh* - for Unix/Linux users

## 4. Expected results and discussion

### Full frame quality assessment

| File                         | Distortion type          | PSNR-Y     | IV-PSNR    |
| :--------------------------- | :----------------------- | :--------- | :--------- |
| Fencing_v8_origJpgCompressed | strong lossy compression | 34.3362 dB | 39.1335 dB |
| Fencing_v8_synthesized       | synthesis artifacts      | 34.0128 dB | 44.3555 dB |

* The PSNR-Y metric for both files is very similar.
* Fencing_v8_origJpgCompressed is significantly more distorted than Fencing_v8_synthesized.
* The IV-PSNR metric implies, that the quality of synthesized image is better, than for the heavily compressed one. Such a result is in line with subjective quality.

The type distortion introduced by synthesis (*Fencing_v8_synthesized*) and compression (*Fencing_v8_origJpgCompressed*) is visible on differential images calculated by subtracting the distorted image from reference one. The differential images can be calculated using ffmpeg tool and following commands:

Differential image for Fencing_v8_origJpgCompressed:    
`ffmpeg -f rawvideo -pixel_format yuv420p -video_size 1920x1080 -i Fencing_v8_original_1920x1080_yuv420p.yuv -f rawvideo -pixel_format yuv420p -video_size 1920x1080 -i Fencing_v8_origJpgCompressed_1920x1080_yuv420p.yuv -filter_complex "blend=all_expr='(A-B+128)',hue=s=0" -f rawvideo difference_Fencing_v8_origJpgCompressed_1920x1080_yuv420p.yuv`

Differential image for Fencing_v8_synthesized:  
`ffmpeg -f rawvideo -pixel_format yuv420p -video_size 1920x1080 -i Fencing_v8_original_1920x1080_yuv420p.yuv -f rawvideo -pixel_format yuv420p -video_size 1920x1080 -i Fencing_v8_synthesized_1920x1080_yuv420p.yuv -filter_complex "blend=all_expr='(A-B+128)',hue=s=0" -f rawvideo difference_Fencing_v8_synthesized_1920x1080_yuv420p.yuv`

The differential images can be displayed using ffplay tool:  
`ffplay -f rawvideo -pixel_format yuv420p -video_size 1920x1080 difference_Fencing_v8_origJpgCompressed_1920x1080_yuv420p.yuv`  
`ffplay -f rawvideo -pixel_format yuv420p -video_size 1920x1080 difference_Fencing_v8_synthesized_1920x1080_yuv420p.yuv`  

### Masked mode - single object quality assessment

| File                                                                                      | PSNR-Y (M) | IV-PSNR (M)|
| :---------------------------------------------------------------------------------------  | :--------- | :--------- |
| Fencing_v8_synthesized (only objects defined in Fencing_v8_objectsMask_1920x1080_yuv420p) | 39.5414 dB | 50.7716 dB |

* The software outputs masked version of all three metrics: IV-PSNR, WS-PSNR, and PSNR.
* The quality was calculated only for pixels, which have value 255 in the *mask* video (both fencers).

## References

* [Fencing] M. Domański, A. Dziembowski, A. Grzelka, D. Mieloch, O. Stankiewicz, K. Wegner, “Multiview test video sequences for free navigation exploration obtained using pairs of cameras,” ISO/IEC JTC1/SC29/WG11 MPEG2016, M38247, Geneva, Switzerland, 30 May - 03 June 2016