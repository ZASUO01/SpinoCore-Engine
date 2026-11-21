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
        #UI
            #Elements
                "include/SpinoCore/Core/UI/Elements/Cursor.h"
                "include/SpinoCore/Core/UI/Elements/Element.h"
                "include/SpinoCore/Core/UI/Elements/Image.h"
                "include/SpinoCore/Core/UI/Elements/Rect.h"
            "include/SpinoCore/Core/UI/Screen.h"
            "include/SpinoCore/Core/UI/ScreensManager.h"
        #Window
            "include/SpinoCore/Core/Window/WindowManager.h"
            "include/SpinoCore/Core/Window/WindowProps.h"
            "include/SpinoCore/Core/Window/WindowSize.h"
    #Files
        "include/SpinoCore/Files/FileManager.h"
    #Game
        "include/SpinoCore/Game/GameApplication.h"
        "include/SpinoCore/Game/GameInfo.h"
        "include/SpinoCore/Game/InitializationContext.h"
    #Graphics
        #Resources
            "include/SpinoCore/Graphics/Resources/FontSize.h"
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
    #Assets
        "src/Assets/AssetCache.h"
    #Core
        #Cameras
            "src/Core/Cameras/DefaultCamera.h"
            "src/Core/Cameras/EngineCamerasManager.cpp"
            "src/Core/Cameras/EngineCamerasManager.h"
        #Engine
            "src/Core/Engine/Engine.cpp"
            "src/Core/Engine/Engine.h"
            "src/Core/Engine/EngineCache.h"
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
        #Input
            "src/Core/Input/InputState.cpp"
            "src/Core/Input/InputState.h"
        #Rendering
            #Drawer
                "src/Core/Rendering/Drawer/DrawCommands.h"
                "src/Core/Rendering/Drawer/Drawer.cpp"
                "src/Core/Rendering/Drawer/Drawer.h"
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
        #UI
            #Elements
                "src/Core/UI/Elements/Cursor.cpp"
                "src/Core/UI/Elements/Element.cpp"
                "src/Core/UI/Elements/Image.cpp"
                "src/Core/UI/Elements/Rect.cpp"
            "src/Core/UI/EngineScreensManager.cpp"
            "src/Core/UI/EngineScreensManager.h"
            "src/Core/UI/Screen.cpp"
        #Window
            "src/Core/Window/EngineWindow.cpp"
            "src/Core/Window/EngineWindow.h"
            "src/Core/Window/WindowSizeHelpers.h"
    #Files
        "src/Files/FileHelpers.cpp"
        "src/Files/FileHelpers.h"
        "src/Files/EngineFileManager.cpp"
        "src/Files/EngineFileManager.h"
    #Graphics
        #Geometry
            "src/Graphics/Geometry/VertexArray.cpp"
            "src/Graphics/Geometry/VertexArray.h"
        #Resources
            "src/Graphics/Resources/EmbeddedFonts.h"
            "src/Graphics/Resources/Font.cpp"
            "src/Graphics/Resources/Font.h"
            "src/Graphics/Resources/Texture.cpp"
            "src/Graphics/Resources/Texture.h"
        #Shaders
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
        "src/SDL/Structs.cpp"
        "src/SDL/Structs.h"
    #Utils
        "src/Utils/JSONHelpers.cpp"
        "src/Utils/JSONHelpers.h"
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

set(SDLIMAGE_INSTALL OFF)
set(SDLIMAGE_SAMPLES OFF)
set(SDLIMAGE_TESTS OFF)
set(SDLIMAGE_VENDORED OFF)
set(SDLIMAGE_DEPS_SHARED OFF)
set(SDLIMAGE_BACKEND_STB ON)
set(SDLIMAGE_AVIF OFF)
set(SDLIMAGE_JXL OFF)
set(SDLIMAGE_TIF OFF)
set(SDLIMAGE_WEBP OFF)
set(BUILD_SHARED_LIBS OFF)
FetchContent_Declare(
        SDL3_image
        GIT_REPOSITORY "https://github.com/libsdl-org/SDL_image.git"
        GIT_TAG        "release-3.2.0"
        GIT_SHALLOW    TRUE
        GIT_SUBMODULES ""
        EXCLUDE_FROM_ALL
)

set(SDLTTF_INSTALL OFF)
set(SDLTTF_SAMPLES OFF)
set(SDLTTF_STRICT ON)
set(SDLTTF_VENDORED ON)
set(SDLTTF_HARFBUZZ OFF)
set(SDLTTF_PLUTOSVG OFF)
FetchContent_Declare(
        SDL3_ttf
        GIT_REPOSITORY "https://github.com/libsdl-org/SDL_ttf.git"
        GIT_TAG        "release-3.2.2"
        GIT_SHALLOW    TRUE
        GIT_SUBMODULES "external/freetype"
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(SDL3 SDL3_image SDL3_ttf)

# ======================================
# Link libraries
# ======================================
add_subdirectory(vendor/glad)


add_library(SpinoCore-json INTERFACE)
add_library(SpinoCore::json ALIAS SpinoCore-json)
target_include_directories(SpinoCore-json SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
target_compile_features(SpinoCore-json INTERFACE cxx_std_11)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3-static SDL3_image::SDL3_image-static SDL3_ttf::SDL3_ttf-static SpinoCore::glad SpinoCore::json)

# ======================================
# Resources compiled into the engine
# ======================================
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedFile.cmake")
spinocore_embed_file(${PROJECT_NAME}
        INPUT "${CMAKE_CURRENT_SOURCE_DIR}/resources/fonts/Inter-Regular-Latin1.otf"
        NAMESPACE SpinoCore::Graphics::Resources::Embedded
        NAME DefaultFontData)

# ======================================
# Game assets (spinocore_add_assets)
# ======================================
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/SpinoCoreAssets.cmake")

# ======================================
# Tests
# ======================================
if(SPINOCORE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()