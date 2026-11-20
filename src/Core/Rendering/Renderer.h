// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/Utils/Size2D.h"
#include "SDL/Structs.h"
#include "Core/Events/EventBus.h"
#include "Core/Events/TypeEvents/WindowEvents.h"
#include "Core/Rendering/Presenter/Presenter.h"

namespace SpinoCore::Core::Rendering {
    class Renderer final {
    public:
        using Duration = Presenter::Duration;

        explicit Renderer(SDL_Window* window, Utils::Size2D windowSize, Events::EventBus& bus);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        void Draw();
        [[nodiscard]] Duration GetFramePeriod() const noexcept { return mPresenter.GetFramePeriod(); }

    private:
        void OnResize(const Events::WindowResizeEvent& event);
        void Resize(Utils::Size2D size, Utils::Size2D pixelSize);

        Presenter mPresenter;
        Events::EventBus::Subscription mResizeSubscription;
    };
}
