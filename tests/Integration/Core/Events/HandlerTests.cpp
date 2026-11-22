// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <optional>
#include <vector>
#include <gtest/gtest.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include "Core/Events/EventBus.h"
#include "Core/Events/Handler.h"
#include "Core/Events/TypeEvents/WindowEvents.h"
#include "SpinoCore/Core/Input/InputState.h"
#include "SDL/Context.h"
#include "SDL/Structs.h"
#include "SpinoCore/Game/GameInfo.h"
#include "SpinoCore/Utils/Size2D.h"

namespace {
    using SpinoCore::Core::Events::EventBus;
    using SpinoCore::Core::Events::Handler;
    using SpinoCore::Core::Events::WindowPresentationChangedEvent;
    using SpinoCore::Core::Events::WindowResizeEvent;
    using SpinoCore::Core::Events::WindowVisibilityChangedEvent;
    using SpinoCore::Core::Input::InputState;
    using SpinoCore::Core::Input::MouseButton;
    using SpinoCore::Game::GameInfo;
    using SpinoCore::SDL::Context;
    using SpinoCore::SDL::UniqueSDLWindow;
    using SpinoCore::Utils::Size2D;

    struct ReceivedEvents final {
        std::vector<Size2D> resizes{};
        std::vector<bool> visibility{};
        int presentationChanges{0};
    };

    class HandlerTests : public testing::Test {
    protected:
        void SetUp() override {
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
            SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
            context.emplace(GameInfo{});
            window.reset(SDL_CreateWindow("Handler tests", 320, 240, 0));
            ASSERT_NE(window, nullptr);

            SDL_PumpEvents();
            SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST);

            resizeSubscription = bus.Listen<WindowResizeEvent>([](WindowResizeEvent& event,
                void* received) {
                static_cast<ReceivedEvents*>(received)->resizes.push_back(event.size);
            }, &received);
            visibilitySubscription = bus.Listen<WindowVisibilityChangedEvent>([](WindowVisibilityChangedEvent& event,
                void* received) {
                static_cast<ReceivedEvents*>(received)->visibility.push_back(event.visible);
            }, &received);
            presentationSubscription = bus.Listen<WindowPresentationChangedEvent>([](WindowPresentationChangedEvent&, void* received) {
                ++static_cast<ReceivedEvents*>(received)->presentationChanges;
            }, &received);
        }

        void PushWindowEvent(const SDL_EventType type) const {
            SDL_Event event{};
            event.type = type;
            event.window.windowID = SDL_GetWindowID(window.get());
            SDL_PushEvent(&event);
        }

        void PushKeyEvent(const SDL_Scancode key, const bool down) const {
            SDL_Event event{};
            event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
            event.key.windowID = SDL_GetWindowID(window.get());
            event.key.scancode = key;
            event.key.down = down;
            SDL_PushEvent(&event);
        }

        void PushMouseButtonEvent(const Uint8 button, const bool down) const {
            SDL_Event event{};
            event.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
            event.button.windowID = SDL_GetWindowID(window.get());
            event.button.button = button;
            event.button.down = down;
            SDL_PushEvent(&event);
        }

        void PushMouseMotionEvent(const float x, const float y) const {
            SDL_Event event{};
            event.type = SDL_EVENT_MOUSE_MOTION;
            event.motion.windowID = SDL_GetWindowID(window.get());
            event.motion.x = x;
            event.motion.y = y;
            SDL_PushEvent(&event);
        }

        std::optional<Context> context{};
        UniqueSDLWindow window{nullptr};
        EventBus bus{};
        InputState input{};
        ReceivedEvents received{};
        EventBus::Subscription resizeSubscription{};
        EventBus::Subscription visibilitySubscription{};
        EventBus::Subscription presentationSubscription{};
    };

    TEST_F(HandlerTests, KeepsRunningWhenNoQuitIsRequested) {
        EXPECT_TRUE(Handler::PollEvents(bus, input));
    }

    TEST_F(HandlerTests, StopsRunningWhenQuitIsRequested) {
        SDL_Event quit{};
        quit.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&quit);

        EXPECT_FALSE(Handler::PollEvents(bus, input));
    }

    TEST_F(HandlerTests, StopsRunningWhenTheWindowIsAskedToClose) {
        PushWindowEvent(SDL_EVENT_WINDOW_CLOSE_REQUESTED);

        EXPECT_FALSE(Handler::PollEvents(bus, input));
    }

    TEST_F(HandlerTests, SeveralResizesInOnePollBecomeASingleResizeEvent) {
        PushWindowEvent(SDL_EVENT_WINDOW_RESIZED);
        PushWindowEvent(SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED);
        PushWindowEvent(SDL_EVENT_WINDOW_RESIZED);

        (void)Handler::PollEvents(bus, input);

        EXPECT_EQ(received.resizes, (std::vector<Size2D>{{.width = 320, .height = 240}}));
    }

    TEST_F(HandlerTests, MinimizingTheWindowReportsItAsHidden) {
        PushWindowEvent(SDL_EVENT_WINDOW_MINIMIZED);

        (void)Handler::PollEvents(bus, input);

        EXPECT_EQ(received.visibility, std::vector{false});
    }

    TEST_F(HandlerTests, RestoringTheWindowReportsItAsVisible) {
        PushWindowEvent(SDL_EVENT_WINDOW_RESTORED);

        (void)Handler::PollEvents(bus, input);

        EXPECT_EQ(received.visibility, std::vector{true});
    }

    TEST_F(HandlerTests, EnteringFullscreenReportsAPresentationChange) {
        PushWindowEvent(SDL_EVENT_WINDOW_ENTER_FULLSCREEN);

        (void)Handler::PollEvents(bus, input);

        EXPECT_EQ(received.presentationChanges, 1);
    }

    TEST_F(HandlerTests, APressedKeyIsAvailableAfterPolling) {
        PushKeyEvent(SDL_SCANCODE_SPACE, true);

        (void)Handler::PollEvents(bus, input);

        EXPECT_TRUE(input.WasKeyPressed(SDL_SCANCODE_SPACE));
    }

    TEST_F(HandlerTests, AKeyPressFromAPreviousPollIsNoLongerReportedAsPressed) {
        PushKeyEvent(SDL_SCANCODE_SPACE, true);
        (void)Handler::PollEvents(bus, input);

        (void)Handler::PollEvents(bus, input);

        EXPECT_FALSE(input.WasKeyPressed(SDL_SCANCODE_SPACE));
    }

    TEST_F(HandlerTests, AMouseClickIsAvailableAfterPolling) {
        PushMouseButtonEvent(SDL_BUTTON_LEFT, true);

        (void)Handler::PollEvents(bus, input);

        EXPECT_TRUE(input.WasMouseButtonPressed(MouseButton::Left));
    }

    TEST_F(HandlerTests, TheMousePositionIsAvailableAfterPolling) {
        PushMouseMotionEvent(150.0f, 90.0f);

        (void)Handler::PollEvents(bus, input);

        EXPECT_FLOAT_EQ(input.GetMousePosition().x, 150.0f);
        EXPECT_FLOAT_EQ(input.GetMousePosition().y, 90.0f);
    }
}
