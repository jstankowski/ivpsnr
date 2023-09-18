#!/usr/bin/env bash
rm -rf buildLT
cmake -S ./ -B buildLT -DCMAKE_BUILD_TYPE=Release -DPMBB_GENERATE_TESTING:BOOL=True -DPMBB_BUILD_WITH_MARCH_NATIVE:BOOL=True -DPMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES:BOOL=False
cmake --build buildLT --clean-first --config Release --parallel
env CTEST_OUTPUT_ON_FAILURE=1 CTEST_PARALLEL_LEVEL=8 cmake --build buildLT --target test