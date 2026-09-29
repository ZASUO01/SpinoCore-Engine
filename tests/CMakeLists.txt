enable_testing()
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(googletest GIT_REPOSITORY "https://github.com/google/googletest.git" GIT_TAG "v1.14.0")
FetchContent_MakeAvailable(googletest)

add_subdirectory(Integration)
add_subdirectory(Unit)