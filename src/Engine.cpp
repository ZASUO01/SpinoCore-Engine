// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Engine.h"
#include "SpinoCore/Logs/Logger.h"
#include "Logs/LogSystem.h"
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
        Logs::LogSystem::Initialize();
        Logs::Logger::Force("[ENGINE] Initializing...");
        if (!InitializeResources()) return false;

        App::AppModules appModules = {};
        mApp->Initialize(appModules);

        Logs::Logger::Force("[ENGINE] Initialized successfully.");
        return true;
    }

    void Engine::Run() {
        Logs::Logger::Force("[ENGINE] Running...");
        Logs::Logger::Force("[ENGINE] Stopped.");
    }

    void Engine::Shutdown() {
        Logs::Logger::Force("[ENGINE] Shutting down...");
        Logs::Logger::Force("[ENGINE] Finished successfully.");
        Logs::LogSystem::Shutdown();
    }

    bool Engine::InitializeResources() {
        return true;
    }
}
