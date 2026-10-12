// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Engine.h"

namespace SpinoCore {
    Engine::Engine(UniqueUserApp app) noexcept :mApp(std::move(app)) {}
    Engine::~Engine() {
        Shutdown();
    }

    bool Engine::Initialize() {

        const SDL::AppMetadata data{.appName = "test", .appVersion = "test", .appIdentifier = "test", .appCreator = "test", .appURL = "test"};
        mSDLContext = SDL::Context::Create(data);
        if (!mSDLContext.has_value()) return false;

        return true;
    }

    void Engine::Run() {
        mLoopDriver.Run(mOperations);
    }

    void Engine::Shutdown() {
        mApp.reset();
    }
}
