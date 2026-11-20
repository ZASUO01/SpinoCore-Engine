// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Engine.h"
#include <utility>
#include "SpinoCore/Core/Scenes/StartContext.h"
#include "SpinoCore/Game/InitializationContext.h"
#include "Core/Scenes/EngineScenesManager.h"
#include "SpinoCore/LaunchOptions.h"

namespace SpinoCore::Core::Engine {
    Engine::Engine(std::unique_ptr<Game::GameApplication> game, const LaunchOptions &launchOptions)
    :mGame(std::move(game))
    {
        Initialize(launchOptions);
    }

    void Engine::Run() {
        mGameLooping.Run(mEngineOperations);
    }

    void Engine::Initialize(const LaunchOptions& launchOptions) {
        mSDLContext.emplace(launchOptions.gameInfo);

        constexpr Scenes::StartContext sceneStartContext{};
        Scenes::ScenesManagerInput scenesManagerInput {
            .startContext = sceneStartContext
        };
        mEngineModules.scenesManager.emplace(scenesManagerInput);

        const Game::InitializationContext gameInitializationContext {
            .scenesManager = *mEngineModules.scenesManager
        };
        mGame->Initialize(gameInitializationContext);
    }
}
