message(STATUS "${PROJECT_NAME} --> trying to fetch https://github.com/doctest/doctest")
include(FetchContent)
FetchContent_Declare(doctest
  GIT_SHALLOW    TRUE
  GIT_REPOSITORY https://github.com/doctest/doctest
  GIT_TAG        v2.4.9
  GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(doctest)
message(STATUS "${PROJECT_NAME} --> fetch DONE")

 