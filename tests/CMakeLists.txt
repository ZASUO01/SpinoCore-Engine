# ======================================
# GoogleTest
# ======================================
include(FetchContent)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY "https://github.com/google/googletest.git"
        GIT_TAG        "v1.17.0"
        GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# ======================================
# Helper to create a test executable
# ======================================
# spinocore_add_test(<target> LABEL <label>)
# Collects every .cpp under the calling directory. If there is none yet,
# the target is skipped so the suite folder can stay empty.
function(spinocore_add_test target)
    cmake_parse_arguments(ARG "" "LABEL" "" ${ARGN})

    file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    if(NOT test_sources)
        message(STATUS "SpinoCore: no ${ARG_LABEL} tests found, skipping ${target}")
        return()
    endif()

    add_executable(${target} ${test_sources})

    target_compile_features(${target} PRIVATE cxx_std_20)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF FOLDER "SpinoCore/Tests")

    target_include_directories(${target}
            PRIVATE
            "${PROJECT_SOURCE_DIR}/src"
            "${PROJECT_SOURCE_DIR}/vendor"
    )

    target_compile_options(${target} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8 /Zc:__cplusplus>
            $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
    )

    target_link_libraries(${target} PRIVATE SpinoCore::Runtime GTest::gtest_main)

    gtest_discover_tests(${target}
            TEST_PREFIX "${ARG_LABEL}."
            PROPERTIES LABELS "${ARG_LABEL}"
    )
endfunction()

# ======================================
# Test suites
# ======================================
add_subdirectory(Unit)
add_subdirectory(Integration)
add_subdirectory(E2E)
