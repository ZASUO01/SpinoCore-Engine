// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Launch.h"

#include "Engine.h"

namespace SpinoCore {
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


    int Launch(UniqueUserApp app) {
        if (!app) {
            return EXIT_FAILURE;
        }

        const RunningGuard runningGuard; // NOLINT
        if (!runningGuard.IsAcquired()) {
            return EXIT_FAILURE;
        }

        Engine engine{std::move(app)};
        if (!engine.Initialize()) {
            return EXIT_FAILURE;
        }

        engine.Run();

        return EXIT_SUCCESS;
    }
}
