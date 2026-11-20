// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include <optional>
#include "SpinoCore/Game/GameApplication.h"
#include "EngineModules.h"
#include "EngineOperations.h"
#include "Runtime/GameLooping.h"
#include "SDL/Context.h"

namespace SpinoCore {
    struct LaunchOptions;
}

namespace SpinoCore::Core::Engine {
    class Engine final {
    public:
        Engine(std::unique_ptr<Game::GameApplication> game, const LaunchOptions& launchOptions);
        ~Engine();

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&) = delete;
        Engine& operator=(Engine&&) = delete;

        void Run();

    private:
        void Initialize(const LaunchOptions& launchOptions);

        std::optional<SDL::Context> mSDLContext{};
        EngineModules mEngineModules{};
        EngineOperations mEngineOperations{mEngineModules};
        Runtime::GameLooping<> mGameLooping{};
        std::unique_ptr<Game::GameApplication> mGame{nullptr};
    };
}
