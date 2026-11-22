// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <chrono>
#include <functional>
#include <gtest/gtest.h>
#include "Runtime/FrameOperations.h"
#include "Runtime/GameLooping.h"

namespace {
    using namespace std::chrono_literals;
    using SpinoCore::Runtime::FrameOperations;

    struct FakeClock final {
        using duration = std::chrono::nanoseconds;
        using rep = duration::rep;
        using period = duration::period;
        using time_point = std::chrono::time_point<FakeClock>;
        static constexpr bool is_steady = true;

        static time_point now() noexcept { return current; }
        static void Advance(const duration elapsed) noexcept { current += elapsed; }
        static void Reset() noexcept { current = time_point{}; }

        inline static time_point current{};
    };

    using GameLooping = SpinoCore::Runtime::GameLooping<FakeClock>;

    constexpr std::chrono::nanoseconds SimulationStep{1'000'000'000 / 60};

    class ScriptedFrameOperations final : public FrameOperations {
    public:
        int framesToRun{0};
        Duration timePerFrame{0};
        bool simulationReady{true};
        int notReadyFrame{0};
        std::function<void()> onRender{};

        int frameInputsProcessed{0};
        int simulationInputsProcessed{0};
        int simulationInputsDiscarded{0};
        int simulationUpdates{0};
        float lastDeltaTime{0.0f};
        float lastAlpha{0.0f};
        int framesRendered{0};

        bool ProcessEvents() override { return mFramesStarted++ < framesToRun; }
        bool PrepareSimulation() override { return simulationReady && mFramesStarted != notReadyFrame; }
        void ProcessFrameInputs() override { ++frameInputsProcessed; }
        void ProcessSimulationInputs() override { ++simulationInputsProcessed; }
        void DiscardSimulationInputs() override { ++simulationInputsDiscarded; }

        void UpdateSimulation(const float deltaTime) override {
            ++simulationUpdates;
            lastDeltaTime = deltaTime;
        }

        void Interpolate(const float alpha) override { lastAlpha = alpha; }

        void Render() override {
            ++framesRendered;
            FakeClock::Advance(timePerFrame);
            if (onRender) onRender();
        }

        Duration GetFramePeriod() override { return Duration{0}; }

    private:
        int mFramesStarted{0};
    };

    class GameLoopingTests : public testing::Test {
    protected:
        void SetUp() override { FakeClock::Reset(); }
    };

    TEST_F(GameLoopingTests, StopsWithoutRenderingWhenEventsAskToQuit) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 0;

        looping.Run(operations);

        EXPECT_EQ(operations.framesRendered, 0);
    }

    TEST_F(GameLoopingTests, RendersOncePerFrame) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 3;

        looping.Run(operations);

        EXPECT_EQ(operations.framesRendered, 3);
    }

    TEST_F(GameLoopingTests, UpdatesTheSimulationOncePerElapsedStep) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 2;
        operations.timePerFrame = 3 * SimulationStep;

        looping.Run(operations);

        EXPECT_EQ(operations.simulationUpdates, 3);
    }

    TEST_F(GameLoopingTests, ProcessesFrameInputsOncePerFrame) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 2;
        operations.timePerFrame = 3 * SimulationStep;

        looping.Run(operations);

        EXPECT_EQ(operations.frameInputsProcessed, 2);
    }

    TEST_F(GameLoopingTests, ProcessesSimulationInputsOncePerSimulationUpdate) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 2;
        operations.timePerFrame = 3 * SimulationStep;

        looping.Run(operations);

        EXPECT_EQ(operations.simulationInputsProcessed, 3);
    }

    TEST_F(GameLoopingTests, UpdatesTheSimulationWithAFixedSixtiethOfASecond) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 2;
        operations.timePerFrame = 50ms;

        looping.Run(operations);

        EXPECT_NEAR(operations.lastDeltaTime, 1.0f / 60.0f, 1e-6f);
    }

    TEST_F(GameLoopingTests, InterpolatesWithTheFractionOfAStepLeftOver) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 2;
        operations.timePerFrame = SimulationStep + SimulationStep / 2;

        looping.Run(operations);

        EXPECT_NEAR(operations.lastAlpha, 0.5f, 1e-5f);
    }

    TEST_F(GameLoopingTests, LimitsALongFrameToAQuarterSecondOfSimulation) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 2;
        operations.timePerFrame = 1s;

        looping.Run(operations);

        EXPECT_EQ(operations.simulationUpdates, 15);
    }

    TEST_F(GameLoopingTests, DoesNotUpdateTheSimulationWhileItIsNotReady) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 2;
        operations.timePerFrame = 3 * SimulationStep;
        operations.simulationReady = false;

        looping.Run(operations);

        EXPECT_EQ(operations.simulationUpdates, 0);
    }

    TEST_F(GameLoopingTests, DropsLeftoverTimeWhenTheSimulationWasNotReady) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 4;
        operations.timePerFrame = SimulationStep + SimulationStep / 2;
        operations.notReadyFrame = 3;

        looping.Run(operations);

        EXPECT_EQ(operations.simulationUpdates, 2);
    }

    TEST_F(GameLoopingTests, DoesNotUpdateTheSimulationWhilePaused) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 2;
        operations.timePerFrame = 3 * SimulationStep;
        looping.Pause();

        looping.Run(operations);

        EXPECT_EQ(operations.simulationUpdates, 0);
    }

    TEST_F(GameLoopingTests, KeepsRenderingWhilePaused) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 2;
        looping.Pause();

        looping.Run(operations);

        EXPECT_EQ(operations.framesRendered, 2);
    }

    TEST_F(GameLoopingTests, UpdatesTheSimulationAgainAfterResuming) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 2;
        operations.timePerFrame = 3 * SimulationStep;
        looping.Pause();
        looping.Resume();

        looping.Run(operations);

        EXPECT_EQ(operations.simulationUpdates, 3);
    }

    TEST_F(GameLoopingTests, DiscardsSimulationInputsOnEveryFrameThatIsNotReady) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 2;
        operations.timePerFrame = 3 * SimulationStep;
        operations.simulationReady = false;

        looping.Run(operations);

        EXPECT_EQ(operations.simulationInputsDiscarded, 2);
    }

    TEST_F(GameLoopingTests, DiscardsSimulationInputsOnEveryPausedFrame) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 2;
        operations.timePerFrame = 3 * SimulationStep;
        looping.Pause();

        looping.Run(operations);

        EXPECT_EQ(operations.simulationInputsDiscarded, 2);
    }

    TEST_F(GameLoopingTests, KeepsSimulationInputsWhileTheSimulationRuns) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 3;
        operations.timePerFrame = SimulationStep / 2;

        looping.Run(operations);

        EXPECT_EQ(operations.simulationInputsDiscarded, 0);
    }

    TEST_F(GameLoopingTests, StopRequestedDuringAFrameEndsTheLoopAfterThatFrame) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 10;
        operations.onRender = [&looping] { looping.Stop(); };

        looping.Run(operations);

        EXPECT_EQ(operations.framesRendered, 1);
    }

    TEST_F(GameLoopingTests, StopRequestedBeforeRunningEndsTheLoopAfterTheFirstFrame) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 10;
        looping.Stop();

        looping.Run(operations);

        EXPECT_EQ(operations.framesRendered, 1);
    }

    TEST_F(GameLoopingTests, RunsNormallyAgainAfterAStoppedRun) {
        GameLooping looping;
        ScriptedFrameOperations stoppedOperations;
        stoppedOperations.framesToRun = 10;
        stoppedOperations.onRender = [&looping] { looping.Stop(); };
        looping.Run(stoppedOperations);
        ScriptedFrameOperations operations;
        operations.framesToRun = 3;

        looping.Run(operations);

        EXPECT_EQ(operations.framesRendered, 3);
    }

    TEST_F(GameLoopingTests, MeasuresFramesPerSecondOverOneSecondOfFrames) {
        GameLooping looping;
        ScriptedFrameOperations operations;
        operations.framesToRun = 4;
        operations.timePerFrame = 250ms;

        looping.Run(operations);

        EXPECT_FLOAT_EQ(looping.GetMeasuredFPS(), 4.0f);
    }
}
