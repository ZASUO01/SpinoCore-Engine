// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "VSyncGuard.h"
#include "Core/Events/EventBus.h"
#include "Core/Events/TypeEvents/WindowEvents.h"
#include "SDL/Structs.h"

namespace SpinoCore::Core::Rendering {
    class Presenter final {
    public:
        using Clock = VSyncGuard::Clock;
        using Duration = VSyncGuard::Duration;

        Presenter(SDL_Window* window, Events::EventBus& bus);
        ~Presenter();

        Presenter(const Presenter&) = delete;
        Presenter& operator=(const Presenter&) = delete;
        Presenter(Presenter&&) = delete;
        Presenter& operator=(Presenter&&) = delete;

        void Present();
        [[nodiscard]] bool IsVisible() const noexcept { return mVisible; }
        [[nodiscard]] Duration GetFramePeriod() const noexcept;

    private:
        void OnVisibilityChanged(const Events::WindowVisibilityChangedEvent& event);
        void OnPresentationChanged(Events::WindowPresentationChangedEvent& event);

        void ApplyVSyncAction(VSyncAction action, Clock::time_point now) noexcept;
        [[nodiscard]] float DisplayRefreshRate() const noexcept;

        SDL::UniqueGLContext mGLContext{nullptr};
        SDL_Window* mSDLWindow{nullptr};
        VSyncGuard mVSyncGuard{};
        float mDisplayRefreshRate{0.0f};
        bool mVisible{true};

        Events::EventBus::Subscription mVisibilitySubscription;
        Events::EventBus::Subscription mPresentationSubscription;
    };
}
