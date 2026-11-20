// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Cameras {
    class Camera {
    public:
        Camera() = default;
        virtual ~Camera() = default;

        virtual void Update(float deltaTime) = 0;
        [[nodiscard]] const Math::Matrix4f& GetViewMatrix() const { return mViewMatrix; }
        [[nodiscard]] const Math::Matrix4f& GetProjectionMatrix() const { return mProjectionMatrix; }

        void SetViewportSize(const float width, const float height) {
            mWidth = width;
            mHeight = height;
            Apply3DProjection();
        }

    protected:
        void Apply3DView() {
            mViewMatrix = Math::MatrixFactory::CreateLookAt(mEye, mTarget, mUp);
        }

        void Apply3DProjection() {
            mProjectionMatrix = Math::MatrixFactory::CreatePerspectiveFOV(mFov, mWidth, mHeight, mNear, mFar);
        }

        Math::Vector3f mEye{Math::Vector3f::One()};
        Math::Vector3f mTarget{Math::Vector3f::Zero()};
        Math::Vector3f mUp{Math::Vector3f::UnitY()};

        float mWidth{0.0f};
        float mHeight{0.0f};
        float mFov{0.0};
        float mNear{0.0f};
        float mFar{0.0f};

        Math::Matrix4f mViewMatrix{Math::Matrix4f::Identity()};
        Math::Matrix4f mProjectionMatrix{Math::Matrix4f::Identity()};
    };
}
