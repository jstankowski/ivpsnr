message(STATUS "${PROJECT_NAME} --> trying to fetch https://github.com/fmtlib/fmt.git")
include(FetchContent)
FetchContent_Declare(fmtlib
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        9.1.0
  GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(fmtlib)
target_compile_features(fmt PRIVATE cxx_std_17)
message(STATUS "${PROJECT_NAME} --> fetch DONE")

 