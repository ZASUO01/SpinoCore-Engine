// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
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

        UniqueUserApp mApp{nullptr};
    };
}