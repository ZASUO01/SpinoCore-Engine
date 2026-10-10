// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Engine.h"

namespace SpinoCore {
    Engine::Engine(UniqueUserApp app) noexcept :mApp(std::move(app)) {}
    Engine::~Engine() {
        Shutdown();
    }

    bool Engine::Initialize() {
        return true;
    }

    void Engine::Run() {

    }

    void Engine::Shutdown() {
        mApp.reset();
    }
}
