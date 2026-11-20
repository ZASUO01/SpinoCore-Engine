// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <optional>
#include <vector>
#include <gtest/gtest.h>
#include <SDL3/SDL.h>
#include "Core/Events/EventBus.h"
#include "Core/Events/Handler.h"
#include "Core/Events/TypeEvents/WindowEvents.h"
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

            resizeSubscription = bus.Listen<WindowResizeEvent>([](WindowResizeEvent& event, // NOLINT
                void* received) {
                static_cast<ReceivedEvents*>(received)->resizes.push_back(event.size);
            }, &received);
            visibilitySubscription = bus.Listen<WindowVisibilityChangedEvent>([](WindowVisibilityChangedEvent& event, // NOLINT
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

        std::optional<Context> context{};
        UniqueSDLWindow window{nullptr};
        EventBus bus{};
        ReceivedEvents received{};
        EventBus::Subscription resizeSubscription{};
        EventBus::Subscription visibilitySubscription{};
        EventBus::Subscription presentationSubscription{};
    };

    TEST_F(HandlerTests, KeepsRunningWhenNoQuitIsRequested) {
        EXPECT_TRUE(Handler::PollEvents(bus));
    }

    TEST_F(HandlerTests, StopsRunningWhenQuitIsRequested) {
        SDL_Event quit{};
        quit.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&quit);

        EXPECT_FALSE(Handler::PollEvents(bus));
    }

    TEST_F(HandlerTests, StopsRunningWhenTheWindowIsAskedToClose) {
        PushWindowEvent(SDL_EVENT_WINDOW_CLOSE_REQUESTED);

        EXPECT_FALSE(Handler::PollEvents(bus));
    }

    TEST_F(HandlerTests, SeveralResizesInOnePollBecomeASingleResizeEvent) {
        PushWindowEvent(SDL_EVENT_WINDOW_RESIZED);
        PushWindowEvent(SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED);
        PushWindowEvent(SDL_EVENT_WINDOW_RESIZED);

        (void)Handler::PollEvents(bus);

        EXPECT_EQ(received.resizes, (std::vector<Size2D>{{.width = 320, .height = 240}}));
    }

    TEST_F(HandlerTests, MinimizingTheWindowReportsItAsHidden) {
        PushWindowEvent(SDL_EVENT_WINDOW_MINIMIZED);

        (void)Handler::PollEvents(bus);

        EXPECT_EQ(received.visibility, std::vector{false});
    }

    TEST_F(HandlerTests, RestoringTheWindowReportsItAsVisible) {
        PushWindowEvent(SDL_EVENT_WINDOW_RESTORED);

        (void)Handler::PollEvents(bus);

        EXPECT_EQ(received.visibility, std::vector{true});
    }

    TEST_F(HandlerTests, EnteringFullscreenReportsAPresentationChange) {
        PushWindowEvent(SDL_EVENT_WINDOW_ENTER_FULLSCREEN);

        (void)Handler::PollEvents(bus);

        EXPECT_EQ(received.presentationChanges, 1);
    }
}
