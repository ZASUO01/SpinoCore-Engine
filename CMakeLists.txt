cmake_minimum_required(VERSION 3.24)
project("SpinoCore-Engine" VERSION 0.1.0 LANGUAGES CXX C)

# ======================================
# C++ config
# ======================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ======================================
# Project source files
# ======================================
set(ENGINE_SOURCES
#include
    #App
    "include/SpinoCore/App/AppModules.h"
    "include/SpinoCore/App/UserApplication.h"
    #FileSystem
    "include/SpinoCore/FileSystem/FileLoader.h"
    #Logs
    "include/SpinoCore/Logs/Logger.h"
"include/SpinoCore/Engine.h"
#src
    #Core
    "src/Core/Engine/APIModules.h"
    "src/Core/Engine/CoreModules.h"
    "src/Core/Engine/ModulesInitializer.cpp"
    "src/Core/Engine/ModulesInitializer.h"
    #Defs
    "src/Defs/StaticCreateConcept.h"
    #FileSystem"
    "src/FileSystem/Core/VirtualFileSystem.cpp"
    "src/FileSystem/Core/VirtualFileSystem.h"
    "src/FileSystem/Core/VirtualPath.cpp"
    "src/FileSystem/Core/VirtualPath.h"
    "src/FileSystem/Helpers/FileHelpers.cpp"
    "src/FileSystem/Helpers/FileHelpers.h"
    "src/FileSystem/Mount/FolderMount.cpp"
    "src/FileSystem/Mount/FolderMount.h"
    "src/FileSystem/Mount/MountPoint.h"
    "src/FileSystem/FileManager.cpp"
    "src/FileSystem/FileManager.h"
    "src/FileSystem/FileWorker.cpp"
    "src/FileSystem/FileWorker.h"
    #Logs
    "src/Logs/ConsoleSink.h"
    "src/Logs/LogSink.h"
    "src/Logs/LogSystem.cpp"
    "src/Logs/LogSystem.h"
"src/Engine.cpp"
)

#set(EXTERNAL_SOURCES
#        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/src/glad.c"
#        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/nlohmann/json.hpp"
#)

# ======================================
# Static library compiling
# ======================================
add_library(${PROJECT_NAME} STATIC ${ENGINE_SOURCES} ${EXTERNAL_SOURCES})
add_library("SpinoCore::Runtime" ALIAS ${PROJECT_NAME})

# ======================================
# Resources access control
# ======================================
target_include_directories(${PROJECT_NAME}
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
#        "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include"
#        "${CMAKE_CURRENT_SOURCE_DIR}/vendor"
)

# ======================================
# Get 3rd party libraries
# ======================================
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/third_party")

set(SDL_SHARED OFF CACHE BOOL "Disable SDL shared library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Enable SDL static library" FORCE)
set(SDL_TESTS OFF CACHE BOOL "Disable SDL tests" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "Disable SDL examples" FORCE)
set(SDL_DOCS OFF CACHE BOOL "Disable SDL docs" FORCE)
set(SDL_PIPEWIRE OFF CACHE BOOL "" FORCE)
set(SDL_PIPEWIRE_SHARED ON CACHE BOOL "" FORCE)

FetchContent_Declare(
        SDL3
        GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
        GIT_TAG        "release-3.2.0"
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(SDL3)

# ======================================
# Compile properties
# ======================================
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

if(TARGET SDL3-static)
    set_target_properties(SDL3-static PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

# ======================================
# Link libraries
# ======================================
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3-static)

# ======================================
# Compile options
# ======================================
option(BUILD_TESTS "Enable building tests with GoogleTest" OFF)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# ======================================
# cmake scripts
# ======================================
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#include(SyncDirectory)

#target_sync_directory(
#        ${PROJECT_NAME}
#        "${CMAKE_CURRENT_SOURCE_DIR}/@base-resources"
#        "${CMAKE_BINARY_DIR}/@base-resources"
#)