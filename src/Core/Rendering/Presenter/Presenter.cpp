// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Presenter.h"
#include <stdexcept>
#include <utility>
#include <SDL3/SDL_video.h>
#include "glad/glad.h"
#include "SDL/Helpers/Helpers.h"

namespace SpinoCore::Core::Rendering {
    namespace {
        constexpr int VSyncOn = 1;
        constexpr int VSyncOff = 0;
    }

    Presenter::Presenter(SDL_Window* window, Events::EventBus& bus) {
        auto glContext = SDL::Helpers::CreateGLContext(window);
        if (!glContext) throw std::runtime_error("[RENDERER] Failed to create OpenGL context");

        mSDLWindow = window;
        mGLContext = std::move(glContext);

        if (!SDL_GL_MakeCurrent(window, static_cast<SDL_GLContext>(mGLContext.get()))) {
            throw std::runtime_error("[RENDERER] Failed to make current GL context");
        }

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
            throw std::runtime_error("[RENDERER] Failed to init GLAD.");
        }

        if (!SDL_GL_SetSwapInterval(VSyncOn)) {
            mVSyncGuard.OnEnableFailed(Clock::now());
        }
        mDisplayRefreshRate = DisplayRefreshRate();

        mVisibilitySubscription = bus.Listen<Events::WindowVisibilityChangedEvent, &Presenter::OnVisibilityChanged>(this);
        mPresentationSubscription = bus.Listen<Events::WindowPresentationChangedEvent, &Presenter::OnPresentationChanged>(this);
    }

    Presenter::~Presenter() = default;

    void Presenter::Present() {
        SDL_GL_SwapWindow(mSDLWindow);

        const auto now = Clock::now();
        ApplyVSyncAction(mVSyncGuard.OnFrame(now, mDisplayRefreshRate), now);
    }

    Presenter::Duration Presenter::GetFramePeriod() const noexcept {
        return mVSyncGuard.MinimumFramePeriod();
    }

    void Presenter::OnVisibilityChanged(const Events::WindowVisibilityChangedEvent& event) {
        mVisible = event.visible;
        const auto now = Clock::now();
        ApplyVSyncAction(mVSyncGuard.OnWindowVisibility(event.visible, now), now);
    }

    void Presenter::OnPresentationChanged(Events::WindowPresentationChangedEvent&) {
        mDisplayRefreshRate = DisplayRefreshRate();
        const auto now = Clock::now();
        ApplyVSyncAction(mVSyncGuard.OnPresentationChanged(now), now);
    }

    void Presenter::ApplyVSyncAction(const VSyncAction action, const Clock::time_point now) noexcept {
        switch (action) {
            case VSyncAction::None:
                break;
            case VSyncAction::Disable:
                SDL_GL_SetSwapInterval(VSyncOff);
                break;
            case VSyncAction::Enable:
                if (!SDL_GL_SetSwapInterval(VSyncOn)) mVSyncGuard.OnEnableFailed(now);
                break;
        }
    }

    float Presenter::DisplayRefreshRate() const noexcept {
        const auto display = SDL_GetDisplayForWindow(mSDLWindow);
        const SDL_DisplayMode* displayMode = SDL_GetCurrentDisplayMode(display);
        return displayMode ? displayMode->refresh_rate : 0.0f;
    }
}
