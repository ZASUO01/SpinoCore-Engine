// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Engine.h"
#include <memory>
#include <string>
#include <utility>
#include "SpinoCore/Core/Scenes/Contexts.h"
#include "SpinoCore/Game/InitializationContext.h"
#include "Assets/AssetCache.h"
#include "Core/Actors/EngineActorsManager.h"
#include "Core/Cameras/EngineCamerasManager.h"
#include "Core/Input/SimulationInput.h"
#include "Core/Scenes/EngineScenesManager.h"
#include "Core/UI/EngineScreensManager.h"
#include "Core/Window/EngineWindow.h"
#include "Files/EngineFileManager.h"
#include "Core/Rendering/Renderer.h"
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "Graphics/Resources/Font.h"
#include "Sound/EngineSoundManager.h"
#include "Sound/Resources/AudioClip.h"
#include "Sound/Synth/EngineSynthManager.h"
#include "Graphics/Resources/Model.h"
#include "Graphics/Resources/Texture.h"
#include "SpinoCore/Core/Lighting/GlobalLight.h"
#include "SpinoCore/Core/Rendering/BuiltinModels.h"
#include "SpinoCore/Core/Window/WindowProps.h"
#include "SpinoCore/LaunchOptions.h"

namespace SpinoCore::Core::Engine {
    Engine::Engine(std::unique_ptr<Game::GameApplication> game, const LaunchOptions &launchOptions)
    :mGame(std::move(game))
    {
        try {
            Initialize(launchOptions);
        } catch (...) {
            Shutdown();
            throw;
        }
    }

    Engine::~Engine() {
        Shutdown();
    }

    void Engine::Run() {
        mGameLooping.Run(mEngineOperations);
    }

    void Engine::Initialize(const LaunchOptions& launchOptions) {
        mSDLContext.emplace(launchOptions.gameInfo);
        InitializeModules(launchOptions);
        SetCacheDefaults();

        mEngineModules.renderer->GetUIDrawList().textures = &mEngineCache.textureCache;
        mEngineModules.renderer->GetUIDrawList().fonts = &mEngineCache.fontCache;
        mEngineModules.renderer->GetModelDrawList().textures = &mEngineCache.textureCache;
        mEngineModules.renderer->GetModelDrawList().models = &mEngineCache.modelCache;

        const Game::InitializationContext gameInitializationContext {
            .windowManager = *mEngineModules.window,
            .scenesManager = *mEngineModules.scenesManager
        };
        mGame->Initialize(gameInitializationContext);
    }

    void Engine::InitializeModules(const LaunchOptions &launchOptions) {
        const Files::FileManagerInput fileManagerInput {
            .textureCache = mEngineCache.textureCache,
            .fontCache = mEngineCache.fontCache,
            .modelCache = mEngineCache.modelCache,
            .soundCache = mEngineCache.soundCache
        };
        mEngineModules.fileManager.emplace(fileManagerInput);

        mEngineModules.window.emplace(launchOptions.windowProps);
        if (launchOptions.windowProps.hideSystemCursor) Window::EngineWindow::SetSystemCursorVisible(false);

        auto* window = mEngineModules.window->GetSDLWindow();
        auto size = mEngineModules.window->GetSize();
        mEngineModules.renderer.emplace(window, size, mEngineModules.eventBus,
                                        Lighting::GlobalLight{launchOptions.enableGlobalLight, launchOptions.globalLight},
                                        launchOptions.debugColliders);
        mEngineModules.renderer->SetPostProcessManager(&mEngineModules.postProcessManager);

        mEngineModules.camerasManager.emplace(size, *mEngineModules.renderer, mEngineModules.eventBus);

        mEngineModules.synthManager.emplace(Sound::Synth::AudioOutput::Device);
        mEngineModules.soundManager.emplace(&mEngineCache.soundCache, Sound::Synth::AudioOutput::Device);

        const Scenes::StartContext sceneStartContext{
            .fileManager = *mEngineModules.fileManager
        };

        const Scenes::ControlContext sceneControlContext{
            .fileManager = *mEngineModules.fileManager,
            .camerasManager = *mEngineModules.camerasManager,
            .screensManager = mEngineModules.screensManager,
            .windowManager = *mEngineModules.window,
            .actorsManager = mEngineModules.actorsManager,
            .globalLight = mEngineModules.renderer->GetGlobalLight(),
            .postProcessManager = mEngineModules.postProcessManager,
            .synthManager = *mEngineModules.synthManager,
            .soundManager = *mEngineModules.soundManager,
            .simulationInput = mEngineModules.simulationInput.GetState()
        };

        const Scenes::ScenesManagerInput scenesManagerInput {
            .startContext = sceneStartContext,
            .controlContext = sceneControlContext
        };
        mEngineModules.scenesManager.emplace(scenesManagerInput);
    }

    void Engine::Shutdown() noexcept {
        mEngineModules.scenesManager.reset();
        mEngineModules.soundManager.reset();
        mEngineModules.screensManager.Clear();
        mEngineModules.fileManager.reset();

        if (mEngineModules.renderer) {
            mEngineModules.renderer->GetUIDrawList().textures = nullptr;
            mEngineModules.renderer->GetUIDrawList().fonts = nullptr;
            mEngineModules.renderer->GetModelDrawList().textures = nullptr;
            mEngineModules.renderer->GetModelDrawList().models = nullptr;
        }
    }

    void Engine::SetCacheDefaults() {
        mEngineCache.textureCache.SetDefault(std::make_unique<Graphics::Resources::Texture>());
        mEngineCache.fontCache.SetDefault(std::make_unique<Graphics::Resources::Font>());
        mEngineCache.modelCache.SetDefault(std::make_unique<Graphics::Resources::Model>());
        mEngineCache.soundCache.SetDefault(std::make_unique<Sound::Resources::AudioClip>());
        mEngineCache.modelCache.Add(std::string(Rendering::CubeModelId), std::make_unique<Graphics::Resources::Model>());
    }
}
