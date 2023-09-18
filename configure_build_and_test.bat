rmdir /Q /S buildWT
cmake -S ./ -B buildWT -DCMAKE_BUILD_TYPE=Debug -DPMBB_GENERATE_TESTING:BOOL=True -DPMBB_BUILD_WITH_MARCH_NATIVE:BOOL=True -DPMBB_GENERATE_MULTI_MICROARCH_LEVEL_BINARIES:BOOL=False
cmake --build buildWT --clean-first --config Debug --parallel
SET CTEST_OUTPUT_ON_FAILURE=1
SET CTEST_PARALLEL_LEVEL=8
cmake --build buildWT --target RUN_TESTS
pause