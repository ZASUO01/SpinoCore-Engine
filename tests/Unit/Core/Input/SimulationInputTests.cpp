// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include "Core/Input/SimulationInput.h"
#include "SpinoCore/Core/Input/InputState.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Core::Input::InputState;
    using SpinoCore::Core::Input::SimulationInput;
    using SpinoCore::Math::Vector2f;

    SDL_Event KeyEvent(const SDL_Scancode key, const bool down) {
        SDL_Event event{};
        event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
        event.key.scancode = key;
        event.key.down = down;
        return event;
    }

    SDL_Event MouseMotionEvent(const float xrel, const float yrel) {
        SDL_Event event{};
        event.type = SDL_EVENT_MOUSE_MOTION;
        event.motion.xrel = xrel;
        event.motion.yrel = yrel;
        return event;
    }

    SDL_Event TextEvent(const char* text) {
        SDL_Event event{};
        event.type = SDL_EVENT_TEXT_INPUT;
        event.text.text = text;
        return event;
    }

    class SimulationInputTests : public testing::Test {
    protected:
        void NextFrame() {
            frame.BeginFrame();
        }

        void EndFrame() {
            simulation.BeginFrame(frame);
        }

        InputState frame{};
        SimulationInput simulation{};
    };

    TEST_F(SimulationInputTests, APressIsDeliveredToTheFirstTickOfTheFrame) {
        NextFrame();
        frame.Apply(KeyEvent(SDL_SCANCODE_SPACE, true));
        EndFrame();

        const InputState& tick = simulation.BeginTick();

        EXPECT_TRUE(tick.WasKeyPressed(SDL_SCANCODE_SPACE));
    }

    TEST_F(SimulationInputTests, APressIsNotRepeatedOnLaterTicksOfTheSameFrame) {
        NextFrame();
        frame.Apply(KeyEvent(SDL_SCANCODE_SPACE, true));
        EndFrame();
        static_cast<void>(simulation.BeginTick());

        const InputState& secondTick = simulation.BeginTick();

        EXPECT_FALSE(secondTick.WasKeyPressed(SDL_SCANCODE_SPACE));
    }

    TEST_F(SimulationInputTests, AHeldKeyStaysDownOnEveryTickOfTheFrame) {
        NextFrame();
        frame.Apply(KeyEvent(SDL_SCANCODE_W, true));
        EndFrame();
        static_cast<void>(simulation.BeginTick());

        const InputState& secondTick = simulation.BeginTick();

        EXPECT_TRUE(secondTick.IsKeyDown(SDL_SCANCODE_W));
    }

    TEST_F(SimulationInputTests, APressFromAFrameWithoutTicksReachesTheNextTick) {
        NextFrame();
        frame.Apply(KeyEvent(SDL_SCANCODE_SPACE, true));
        EndFrame();
        NextFrame();
        EndFrame();

        const InputState& tick = simulation.BeginTick();

        EXPECT_TRUE(tick.WasKeyPressed(SDL_SCANCODE_SPACE));
    }

    TEST_F(SimulationInputTests, APressConsumedByATickIsNotDeliveredAgainOnTheNextFrame) {
        NextFrame();
        frame.Apply(KeyEvent(SDL_SCANCODE_SPACE, true));
        EndFrame();
        static_cast<void>(simulation.BeginTick());
        NextFrame();
        EndFrame();

        const InputState& tick = simulation.BeginTick();

        EXPECT_FALSE(tick.WasKeyPressed(SDL_SCANCODE_SPACE));
    }

    TEST_F(SimulationInputTests, AQuickTapInsideOneFrameStillReachesTheSimulationAsAPress) {
        NextFrame();
        frame.Apply(KeyEvent(SDL_SCANCODE_SPACE, true));
        frame.Apply(KeyEvent(SDL_SCANCODE_SPACE, false));
        EndFrame();

        const InputState& tick = simulation.BeginTick();

        EXPECT_TRUE(tick.WasKeyPressed(SDL_SCANCODE_SPACE));
    }

    TEST_F(SimulationInputTests, TheMouseDeltaIsNotRepeatedOnLaterTicksOfTheSameFrame) {
        NextFrame();
        frame.Apply(MouseMotionEvent(5.0f, 2.0f));
        EndFrame();
        static_cast<void>(simulation.BeginTick());

        const InputState& secondTick = simulation.BeginTick();

        EXPECT_TRUE(Vector2f::NearlyEqual(secondTick.GetMouseDelta(), Vector2f::Zero()));
    }

    TEST_F(SimulationInputTests, MouseDeltasFromFramesWithoutTicksAreAddedTogether) {
        NextFrame();
        frame.Apply(MouseMotionEvent(3.0f, 1.0f));
        EndFrame();
        NextFrame();
        frame.Apply(MouseMotionEvent(4.0f, 2.0f));
        EndFrame();

        const InputState& tick = simulation.BeginTick();

        EXPECT_TRUE(Vector2f::NearlyEqual(tick.GetMouseDelta(), Vector2f{7.0f, 3.0f}));
    }

    TEST_F(SimulationInputTests, TextTypedInFramesWithoutTicksIsJoinedInOrder) {
        NextFrame();
        frame.Apply(TextEvent("ab"));
        EndFrame();
        NextFrame();
        frame.Apply(TextEvent("c"));
        EndFrame();

        const InputState& tick = simulation.BeginTick();

        EXPECT_EQ(tick.GetTypedText(), "abc");
    }

    TEST_F(SimulationInputTests, DiscardingDropsPressesThatNoTickConsumed) {
        NextFrame();
        frame.Apply(KeyEvent(SDL_SCANCODE_SPACE, true));
        EndFrame();
        simulation.Discard();

        const InputState& tick = simulation.BeginTick();

        EXPECT_FALSE(tick.WasKeyPressed(SDL_SCANCODE_SPACE));
    }

    TEST_F(SimulationInputTests, DiscardingKeepsHeldKeysDown) {
        NextFrame();
        frame.Apply(KeyEvent(SDL_SCANCODE_W, true));
        EndFrame();
        simulation.Discard();

        const InputState& tick = simulation.BeginTick();

        EXPECT_TRUE(tick.IsKeyDown(SDL_SCANCODE_W));
    }

    TEST_F(SimulationInputTests, TheStateSeenByTheSceneIsTheOneTheTicksRead) {
        NextFrame();
        frame.Apply(KeyEvent(SDL_SCANCODE_SPACE, true));
        EndFrame();

        const InputState& tick = simulation.BeginTick();

        EXPECT_EQ(&simulation.GetState(), &tick);
    }
}
