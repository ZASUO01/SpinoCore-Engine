// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include <optional>
#include "EngineModules.h"
#include "EngineOperations.h"
#include "Runtime/GameLooping.h"
#include "SpinoCore/LaunchOptions.h"
#include "SpinoCore/Game/GameApplication.h"
#include "SDL/Context.h"

namespace SpinoCore::Core::Engine {
    class Engine final {
    public:
        Engine(std::unique_ptr<Game::GameApplication> game, const LaunchOptions& options);
        ~Engine();

        Engine(const Engine&) = delete;
        Engine(Engine&&) = delete;
        Engine& operator=(const Engine&) = delete;
        Engine& operator=(Engine&&) = delete;

        void Run();

    private:
        void Initialize(const LaunchOptions& options);
        static void Shutdown();

        std::optional<SDL::Context> mSDLContext{};
        Events::EventBus mEventBus{};
        EngineModules mEngineModules{};
        EngineOperations mEngineOperations{mEngineModules, mEventBus};
        Runtime::GameLooping<> mGameLooping{};
        std::unique_ptr<Game::GameApplication> mGame{nullptr};
    };
}
