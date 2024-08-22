#!/usr/bin/env bash
rm -rf buildL
cmake -S ./ -B buildL -DCMAKE_BUILD_TYPE=Release -DPMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES:BOOL=True -DPMBB_GENERATE_SINGLE_APP_WITH_WITH_RUNTIME_DISPATCH:BOOL=True
cmake --build buildL --clean-first --config Release --parallel $(nproc)