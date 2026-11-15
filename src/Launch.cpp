// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Launch.h"

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
        std::unique_ptr<Game::GameApplication> /**game**/,
        const LaunchOptions& /**launchOptions**/
    ) {
        RunningGuard runningGuard{}; // NOLINT
        if (!runningGuard.IsAcquired()) {
            std::fputs("[LAUNCH] Only one engine instance may run at a time.\n", stderr);
            return EXIT_FAILURE;
        }


        return EXIT_SUCCESS;
    }
}