cmake_minimum_required(VERSION 3.24)
project(
        SpinoCore-Engine
        VERSION 0.1.0
        DESCRIPTION "SpinoCore game engine runtime"
        LANGUAGES C CXX
)

# ====================================
# Options
# ====================================
option(SPINOCORE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors in the engine" OFF)

# ======================================
# Engine library
# ======================================
add_library(${PROJECT_NAME} STATIC)
add_library(SpinoCore::Runtime ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
#include/SpinoCore
    "include/SpinoCore/Launch.h"
#src
    "src/Launch.cpp"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF FOLDER "SpinoCore")

# ======================================
# Resources access control
# ======================================
target_include_directories(${PROJECT_NAME}
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor"
)

# ======================================
# Warnings
# ======================================
target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive- /utf-8 /Zc:__cplusplus>
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion>
)

if(SPINOCORE_WARNINGS_AS_ERRORS)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_WARNING_AS_ERROR ON)
endif()

# ======================================
# Get 3rd party libraries
# ======================================
include(FetchContent)

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(SDL_TEST_LIBRARY OFF)
set(SDL_TESTS OFF)
set(SDL_EXAMPLES OFF)
set(SDL_INSTALL OFF)
set(SDL_PIPEWIRE OFF)
FetchContent_Declare(
        SDL3
        GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
        GIT_TAG        "release-3.2.0"
        GIT_SHALLOW    TRUE
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(SDL3)

# ======================================
# Link libraries
# ======================================
add_subdirectory(vendor/glad)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3-static SpinoCore::glad)