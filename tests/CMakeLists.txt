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
# ======================================.
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
    )

    target_compile_options(${target} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8 /Zc:__cplusplus>
            $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
    )

    target_link_libraries(${target} PRIVATE SpinoCore::Runtime SpinoCore::json GTest::gtest_main)

    # The engine refuses to start without an Assets folder next to it, so every test binary gets an empty one.
    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:${target}>/Assets")

    # Mesa software drivers (common in CI) report OpenGL 4.5 / GLSL 4.50, but the engine asks for 4.6
    # and its shaders use "#version 460". The overrides only affect Mesa and let the GL-dependent tests run there.
    gtest_discover_tests(${target}
            TEST_PREFIX "${ARG_LABEL}."
            PROPERTIES
                LABELS "${ARG_LABEL}"
                # Each property takes a single value here, because gtest_discover_tests splits list values apart.
                ENVIRONMENT "MESA_GL_VERSION_OVERRIDE=4.6"
                ENVIRONMENT_MODIFICATION "MESA_GLSL_VERSION_OVERRIDE=set:460"
    )
endfunction()

# ======================================
# Test suites
# ======================================
add_subdirectory(Unit)
add_subdirectory(Integration)
add_subdirectory(E2E)

# Build scripts, tested by running them with cmake -P.
add_test(NAME CMake.SyncAssets
        COMMAND "${CMAKE_COMMAND}"
            "-DSYNC_SCRIPT=${PROJECT_SOURCE_DIR}/cmake/SyncAssets.cmake"
            "-DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/SyncAssetsWork"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/CMake/SyncAssetsTests.cmake")
set_tests_properties(CMake.SyncAssets PROPERTIES LABELS "CMake")
