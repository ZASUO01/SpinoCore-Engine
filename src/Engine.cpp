// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Engine.h"
#include "SpinoCore/App/AppModules.h"
#include "SpinoCore/App/UserApplication.h"

namespace SpinoCore {
    struct Engine::Context {

    };

    Engine::Engine(ConstructorKey, std::unique_ptr<App::UserApplication> app)
    :mContext(std::make_unique<Context>())
    ,mApp(std::move(app)){}

    Engine::~Engine() {
        Shutdown();
    }

    void Engine::Launch(std::unique_ptr<App::UserApplication> app) {
        const auto engine = std::make_unique<Engine>(ConstructorKey{}, std::move(app));

        if (!engine->Initialize()) {
            return;
        }

        engine->Run();
    }

    bool Engine::Initialize() {
        if (!InitializeResources()) return false;

        App::AppModules appModules = {};
        mApp->Initialize(appModules);

        return true;
    }

    void Engine::Run() {}

    void Engine::Shutdown() {}

    bool Engine::InitializeResources() {
        return true;
    }
}
