// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Launch.h"

#include "Core/Engine/Engine.h"
#include "Logs/LogSystem.h"
#include "SpinoCore/Logs/Logger.h"

namespace SpinoCore {
    namespace {
        std::atomic_flag gEngineRunning{};

        class RunningGuard final {
        public:
            RunningGuard() noexcept :mAcquired(!gEngineRunning.test_and_set()) {}
            ~RunningGuard() {
                if (mAcquired) gEngineRunning.clear();
            }

            RunningGuard(const RunningGuard&) = delete;
            RunningGuard& operator=(const RunningGuard&) = delete;
            RunningGuard(RunningGuard&&) = delete;
            RunningGuard& operator=(RunningGuard&&) = delete;

            [[nodiscard]] bool IsAcquired() const noexcept { return mAcquired; }

        private:
            bool mAcquired{false};
        };
    }


    int Launch(
        std::unique_ptr<Game::GameApplication> game,
        const LaunchOptions& launchOptions,
        Logs::LogOptions logOptions
    ) {
        RunningGuard runningGuard{}; // NOLINT
        if (!runningGuard.IsAcquired()) {
            std::fputs("[LAUNCH] Only one engine instance may run at a time.\n", stderr);
            return EXIT_FAILURE;
        }

        std::optional<Logs::LogSystem> logSystem; // NOLINT
        try {
            logSystem.emplace(std::move(logOptions.sink));
            Logs::Logger::SetLevel(logOptions.level);
        } catch (const std::exception& e) {
            std::fprintf(stderr, "[LAUNCH] Failed to initialize logging: %s\n", e.what());
            return EXIT_FAILURE;
        }

        if (!game) {
            Logs::Logger::Fatal("[LAUNCH] Empty game application");
            return EXIT_FAILURE;
        }

        try {
            Core::Engine::Engine engine{ std::move(game), launchOptions };
            engine.Run();
        }catch(std::exception& e) {
            Logs::Logger::Fatal("The engine execution failed due exception: {}", e.what());
            return EXIT_FAILURE;
        }catch (...) {
            Logs::Logger::Fatal("The engine execution failed due non-standard exception.");
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
}
