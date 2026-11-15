// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Engine.h"
#include "SpinoCore/Game/InitializationContext.h"
#include "SpinoCore/Logs/Logger.h"

namespace SpinoCore::Core::Engine {
    Engine::Engine(std::unique_ptr<Game::GameApplication> game, const LaunchOptions &options) :mGame(std::move(game))
    {
        Initialize(options);
    }

    Engine::~Engine() {
        Shutdown();
    }

    void Engine::Run() {
        mGameLooping.Run(mEngineOperations);
    }

    void Engine::Initialize(const LaunchOptions& options) {
        mSDLContext.emplace(options.gameInfo);

        mEngineModules.window.emplace(options.windowProps);
        auto window = mEngineModules.window->GetSDLWindow();
        auto size = mEngineModules.window->GetSize();
        mEngineModules.renderer.emplace(window, size, mEventBus);
        mEngineModules.camerasManager.emplace(size, *mEngineModules.renderer, mEventBus);

        constexpr Game::InitializationContext ctx{};
        mGame->Initialize(ctx);

        Logs::Logger::Info("The application has been initialized.");
    }

    void Engine::Shutdown() {
        Logs::Logger::Info("The application has been shut down.");
    }
}
