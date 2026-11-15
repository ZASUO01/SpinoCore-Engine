// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "Core/Events/EventBus.h"
#include "SDL/Structs.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Utils/Size2D.h"

namespace SpinoCore::Core::Rendering {
    class Renderer final {
    public:
        explicit Renderer(SDL_Window* window, Utils::Size2D windowSize, Events::EventBus& bus);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        void Draw();

        void SetViewMatrix(const Math::Matrix4f& matrix);
        void SetProjectionMatrix(const Math::Matrix4f& matrix);
    private:
        static constexpr float ORTHO_NEAR_PLANE = -1.0f;
        static constexpr float ORTHO_FAR_PLANE = 1.0f;

        void SetOrthoViewMatrix(float width, float height, float near, float far);
        void Resize(Utils::Size2D size, Utils::Size2D pixelSize);

        SDL::UniqueGLContext mGLContext{nullptr};
        SDL_Window* mSDLWindow{nullptr};

        Math::Matrix4f mViewMatrix{Math::Matrix4f::Identity()};
        Math::Matrix4f mProjectionMatrix{Math::Matrix4f::Identity()};
        Math::Matrix4f mViewProjectionMatrix{Math::Matrix4f::Identity()};
        Math::Matrix4f mOrthoViewProjectionMatrix;
        bool mMinimized{false};

        Events::EventBus::Subscription mResizeSubscription;
        Events::EventBus::Subscription mMinimizeSubscription;
    };
}
