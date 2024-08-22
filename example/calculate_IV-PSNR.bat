echo "JPEG compressed"
"../buildW/IVPSNR/Release/IVPSNR.exe" -i0 Fencing_v8_original_1920x1080_yuv420p.yuv -i1 Fencing_v8_origJpgCompressed_1920x1080_yuv420p.yuv -pw 1920 -ph 1080 -ml "all" -v 4 -r results.txt > logJ.txt

echo "Synthesized"
"../buildW/IVPSNR/Release/IVPSNR.exe" -i0 Fencing_v8_original_1920x1080_yuv420p.yuv -i1 Fencing_v8_synthesized_1920x1080_yuv420p.yuv -pw 1920 -ph 1080 -ml "all" -v 4 -r results.txt > logS.txt

echo "Synthesized (with mask)"
"../buildW/IVPSNR/Release/IVPSNR.exe" -i0 Fencing_v8_original_1920x1080_yuv420p.yuv -i1 Fencing_v8_synthesized_1920x1080_yuv420p.yuv -pw 1920 -ph 1080 -ml "PSNR,IVPSNR" -im Fencing_v8_objectsMask_1920x1080_yuv420p.yuv -v 4 -r results.txt > logM.txt
