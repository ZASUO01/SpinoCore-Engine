// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO0
#pragma once
#include "SpinoCore/Core/Cameras/Camera.h"
#include "SpinoCore/Math/Math.h"

namespace SpinoCore::Core::Cameras {
    class DefaultCamera final : public Camera {
    public:
        DefaultCamera(const float width, const float height) {
            mEye = Math::Vector3f(3.0f, 3.0f, 3.0f);
            mWidth = width;
            mHeight = height;
            mFov = FOV * Math::Constants::DegToRad<float>;
            mNear = NEAR_PLANE;
            mFar = FAR_PLANE;

            Apply3DProjection();
            Apply3DView();
        }

        ~DefaultCamera() override = default;

        void Update(float /*dt*/) override {}

    private:
        static constexpr float FOV = 45.0f;
        static constexpr float NEAR_PLANE = 0.1f;
        static constexpr float FAR_PLANE = 1000.f;
    };
}
