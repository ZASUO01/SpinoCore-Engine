// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>

namespace SpinoCore::App {
    class UserApplication;
}

namespace SpinoCore {
    class Engine {
    public:
        class ConstructorKey{ friend class Engine; ConstructorKey() = default; };
        Engine(ConstructorKey, std::unique_ptr<App::UserApplication> app);
        ~Engine();

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&) = delete;
        Engine& operator=(Engine&&) = delete;

        static void Launch(std::unique_ptr<App::UserApplication> app);
    private:
        [[nodiscard]] bool Initialize();
        [[nodiscard]] bool InitializeResources();
        void Run();
        void Shutdown();

        struct Context;
        std::unique_ptr<Context> mContext;
        std::unique_ptr<App::UserApplication> mApp;
    };
}