// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Engine.h"

#include "Config/Constants.h"
#include "SpinoCore/App/AppModules.h"
#include "SpinoCore/App/UserApplication.h"
#include "SpinoCore/Logs/Logger.h"
#include "Core/Engine/APIModules.h"
#include "Core/Engine/CoreModules.h"
#include "Core/Engine/ModulesInitializer.h"
#include "Logs/LogSystem.h"


namespace SpinoCore {
    struct Engine::Context {
        Core::Engine::APIModules apiModules;
        Core::Engine::CoreModules coreModules;
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

        auto& apiModules = mContext->apiModules;
        if (auto& coreModules = mContext->coreModules; !Core::Engine::ModulesInitializer::Initialize(apiModules, coreModules)) {
            Logs::Logger::Error("[ENGINE] Failed to initialize modules.");
            return false;
        }
        Logs::Logger::Info("[ENGINE] Modules initialized successfully.");

        if (!InitializeResources()) return false;
        Logs::Logger::Info("[ENGINE] Resources initialized successfully.");

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

        mApp.reset();
        mContext.reset();

        Logs::Logger::Force("[ENGINE] Finished successfully.");
        Logs::LogSystem::Shutdown();
    }

    bool Engine::InitializeResources() const {
        using namespace Config::Constants::FileSystem;

        const auto& fileManager = mContext->apiModules.fileManager;
        if (!fileManager) {
            Logs::Logger::Error("[ENGINE] Failed to initialize resources. File Manager not initialized.");
            return false;
        }

        if (!fileManager->LoadFilesSync(BASE_MANIFEST_PATH.data())) return false;

        return true;
    }
}
