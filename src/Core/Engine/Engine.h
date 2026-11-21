// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include <optional>

#include "EngineCache.h"
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
        void InitializeModules(const LaunchOptions& launchOptions);
        void SetCacheDefaults();
        void Shutdown() noexcept;

        // Destroyed bottom to top. The game outlives everything it created; the cache dies before the modules,
        // so its textures go while the GL context (owned by the renderer) is still alive. Shutdown() destroys the
        // modules that may hold cached assets before the cache itself.
        std::optional<SDL::Context> mSDLContext{};
        std::unique_ptr<Game::GameApplication> mGame{nullptr};
        EngineModules mEngineModules{};
        EngineCache mEngineCache{};
        EngineOperations mEngineOperations{mEngineModules};
        Runtime::GameLooping<> mGameLooping{};
    };
}
