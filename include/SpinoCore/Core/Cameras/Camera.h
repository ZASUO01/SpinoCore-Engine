// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Vector.h"

/// @brief Cameras that give the view and projection used to draw the 3D scene.
namespace SpinoCore::Core::Cameras {
    /// @brief Base class of cameras.
    ///
    /// Subclasses set the protected eye, target, up and lens values and call the protected
    /// Apply3DView and Apply3DProjection helpers to rebuild the matrices.
    class Camera {
    public:
        Camera() = default;
        virtual ~Camera() = default;

        /// @brief Called once per simulation step for the current camera.
        /// @param deltaTime Simulation step in seconds.
        virtual void Update(float deltaTime) = 0;
        /// @return The view matrix.
        [[nodiscard]] const Math::Matrix4f& GetViewMatrix() const { return mViewMatrix; }
        /// @return The projection matrix.
        [[nodiscard]] const Math::Matrix4f& GetProjectionMatrix() const { return mProjectionMatrix; }

        /// @brief Updates the aspect ratio and rebuilds the projection; the engine calls it on window resizes.
        /// @param width Viewport width in pixels.
        /// @param height Viewport height in pixels.
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
