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
option(SPINOCORE_BUILD_TESTS "Enable testing and build the project tests" OFF)

# ======================================
# Engine library
# ======================================
add_library(${PROJECT_NAME} STATIC)
add_library(SpinoCore::Runtime ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
#include/SpinoCore
    #Core
        #Cameras
            "include/SpinoCore/Core/Cameras/Camera.h"
            "include/SpinoCore/Core/Cameras/CamerasManager.h"
        #Scenes
            "include/SpinoCore/Core/Scenes/Scene.h"
            "include/SpinoCore/Core/Scenes/ScenesManager.h"
            "include/SpinoCore/Core/Scenes/StartContext.h"
        #Window
            "include/SpinoCore/Core/Window/WindowManager.h"
            "include/SpinoCore/Core/Window/WindowProps.h"
            "include/SpinoCore/Core/Window/WindowSize.h"
    #Game
        "include/SpinoCore/Game/GameApplication.h"
        "include/SpinoCore/Game/GameInfo.h"
        "include/SpinoCore/Game/InitializationContext.h"
    #Logs
        "include/SpinoCore/Logs/Logger.h"
        "include/SpinoCore/Logs/LogLevel.h"
        "include/SpinoCore/Logs/LogOptions.h"
        "include/SpinoCore/Logs/LogSink.h"
    #Math
        "include/SpinoCore/Math/Math.h"
        "include/SpinoCore/Math/Matrix.h"
        "include/SpinoCore/Math/MatrixFactory.h"
        "include/SpinoCore/Math/Vector.h"
        "include/SpinoCore/Math/Quaternion.h"
    #Runtime
        "include/SpinoCore/Runtime/LoopingManager.h"
    #Utils
        "include/SpinoCore/Utils/Random.h"
        "include/SpinoCore/Utils/Size2D.h"
    "include/SpinoCore/Launch.h"
    "include/SpinoCore/LaunchOptions.h"
#src
    #Core
        #Cameras
            "src/Core/Cameras/DefaultCamera.h"
            "src/Core/Cameras/EngineCamerasManager.cpp"
            "src/Core/Cameras/EngineCamerasManager.h"
        #Engine
            "src/Core/Engine/Engine.cpp"
            "src/Core/Engine/Engine.h"
            "src/Core/Engine/EngineModules.h"
            "src/Core/Engine/EngineOperations.cpp"
            "src/Core/Engine/EngineOperations.h"
        #Events
            #TypeEvents
                "src/Core/Events/TypeEvents/TypeEvent.h"
                "src/Core/Events/TypeEvents/WindowEvents.h"
            "src/Core/Events/EventBus.h"
            "src/Core/Events/Handler.cpp"
            "src/Core/Events/Handler.h"
        #Rendering
            #Presenter
                "src/Core/Rendering/Presenter/Presenter.cpp"
                "src/Core/Rendering/Presenter/Presenter.h"
                "src/Core/Rendering/Presenter/VSyncGuard.cpp"
                "src/Core/Rendering/Presenter/VSyncGuard.h"
            "src/Core/Rendering/Renderer.cpp"
            "src/Core/Rendering/Renderer.h"
        #Scenes
            "src/Core/Scenes/EngineScenesManager.cpp"
            "src/Core/Scenes/EngineScenesManager.h"
            "src/Core/Scenes/Scene.cpp"
        #Window
            "src/Core/Window/EngineWindow.cpp"
            "src/Core/Window/EngineWindow.h"
            "src/Core/Window/WindowSizeHelpers.h"
    #Graphics
        #Geometry
            "src/Graphics/Geometry/VertexArray.cpp"
            "src/Graphics/Geometry/VertexArray.h"
        #Shders
            #Sources
                "src/Graphics/Shaders/Sources/UIShader.h"
            "src/Graphics/Shaders/Shader.cpp"
            "src/Graphics/Shaders/Shader.h"
            "src/Graphics/Shaders/ShaderHelpers.cpp"
            "src/Graphics/Shaders/ShaderHelpers.h"
    #Logs
        "src/Logs/ConsoleSink.h"
        "src/Logs/LogSystem.cpp"
        "src/Logs/LogSystem.h"
    #Runtime
        "src/Runtime/FrameOperations.h"
        "src/Runtime/GameLooping.h"
    #SDL
        #Helpers
            "src/SDL/Helpers/Helpers.cpp"
            "src/SDL/Helpers/Helpers.h"
        "src/SDL/Context.cpp"
        "src/SDL/Context.h"
        "src/SDL/Structs.h"
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

# ======================================
# Tests
# ======================================
if(SPINOCORE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()