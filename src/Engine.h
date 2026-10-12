// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>

#include "Core/Engine/CoreModules.h"
#include "Runtime/FrameOperations.h"
#include "Runtime/LoopDriver.h"
#include "Runtime/SDLClock.h"
#include "SDL/Context.h"
#include "SpinoCore/App/UserApplication.h"

namespace SpinoCore {
    using UniqueUserApp = std::unique_ptr<App::UserApplication>;

    class Engine final {
    public:
        explicit Engine(UniqueUserApp app) noexcept;
        ~Engine();

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&) = delete;
        Engine& operator=(Engine&&) = delete;

        [[nodiscard]] bool Initialize();
        void Run();

    private:
        void Shutdown();

        std::optional<SDL::Context> mSDLContext;
        Core::Engine::CoreModules mCoreModules;
        Runtime::LoopDriver<Runtime::SDLClock> mLoopDriver{};
        Runtime::FrameOperations mOperations{mCoreModules};
        UniqueUserApp mApp{nullptr};
    };
}