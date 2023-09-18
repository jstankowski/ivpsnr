echo "JPEG compressed"
"../buildL/IVPSNR/IVPSNR" -i0 Fencing_v8_original_1920x1080_yuv420p.yuv -i1 Fencing_v8_origJpgCompressed_1920x1080_yuv420p.yuv -w 1920 -h 1080 -r results.txt

echo "Synthesized"
"../buildL/IVPSNR/IVPSNR" -i0 Fencing_v8_original_1920x1080_yuv420p.yuv -i1 Fencing_v8_synthesized_1920x1080_yuv420p.yuv -w 1920 -h 1080 -r results.txt

echo "Synthesized (with mask)"
"../buildL/IVPSNR/IVPSNR" -i0 Fencing_v8_original_1920x1080_yuv420p.yuv -i1 Fencing_v8_synthesized_1920x1080_yuv420p.yuv -w 1920 -h 1080 -im Fencing_v8_objectsMask_1920x1080_yuv420p.yuv -r results.txt