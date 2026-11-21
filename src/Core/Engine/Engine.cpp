// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Engine.h"
#include <memory>
#include <utility>
#include "SpinoCore/Core/Scenes/StartContext.h"
#include "SpinoCore/Game/InitializationContext.h"
#include "Core/Scenes/EngineScenesManager.h"
#include "Files/EngineFileManager.h"
#include "Graphics/Resources/Texture.h"
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

        const Game::InitializationContext gameInitializationContext {
            .windowManager = *mEngineModules.window,
            .scenesManager = *mEngineModules.scenesManager
        };
        mGame->Initialize(gameInitializationContext);
    }

    void Engine::InitializeModules(const LaunchOptions &launchOptions) {
        Files::FileManagerInput fileManagerInput {.textureCache = mEngineCache.textureCache};
        mEngineModules.fileManager.emplace(fileManagerInput);

        mEngineModules.window.emplace(launchOptions.windowProps);

        auto window = mEngineModules.window->GetSDLWindow();
        auto size = mEngineModules.window->GetSize();
        mEngineModules.renderer.emplace(window, size, mEngineModules.eventBus);

        mEngineModules.camerasManager.emplace(size, *mEngineModules.renderer, mEngineModules.eventBus);

        const Scenes::StartContext sceneStartContext{
            .fileManager = *mEngineModules.fileManager,
            .camerasManager = *mEngineModules.camerasManager,
            .screensManager = mEngineModules.screensManager
        };

        Scenes::ScenesManagerInput scenesManagerInput {
            .startContext = sceneStartContext
        };
        mEngineModules.scenesManager.emplace(scenesManagerInput);
    }

    void Engine::Shutdown() noexcept {
        mEngineModules.scenesManager.reset();
        mEngineModules.screensManager.Clear();
        mEngineModules.fileManager.reset();
    }

    void Engine::SetCacheDefaults() {
        mEngineCache.textureCache.SetDefault(std::make_unique<Graphics::Resources::Texture>());
    }
}
