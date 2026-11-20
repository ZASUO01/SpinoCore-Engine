// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include "SpinoCore/Game/GameApplication.h"
#include "EngineModules.h"

namespace SpinoCore {
    struct LaunchOptions;
}

namespace SpinoCore::Core::Engine {
    class Engine final {
    public:
        Engine(std::unique_ptr<Game::GameApplication> game, const LaunchOptions& launchOptions);
        ~Engine() = default;

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&) = delete;
        Engine& operator=(Engine&&) = delete;

        void Run();

    private:
        void Initialize(const LaunchOptions& launchOptions);

        EngineModules mEngineModules{};
        std::unique_ptr<Game::GameApplication> mGame{nullptr};
    };
}
