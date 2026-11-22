// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Lighting {
    struct GlobalLightSettings {
        Math::Vector3f position{4.0f, 8.0f, 4.0f};
        Math::Vector3f target{Math::Vector3f::Zero()};
        Math::Vector3f color{Math::Vector3f::One()};
        float intensity{1.0f};
        Math::Vector3f ambientColor{0.25f, 0.25f, 0.28f};
        float shadowExtent{10.0f};
        float shadowNear{0.1f};
        float shadowFar{50.0f};
    };

    class GlobalLight final {
    public:
        explicit GlobalLight(bool enabled = false, const GlobalLightSettings& settings = {});

        [[nodiscard]] bool IsEnabled() const noexcept { return mEnabled; }

        void SetPosition(const Math::Vector3f& position);
        void SetHeight(float height);
        void SetTarget(const Math::Vector3f& target);
        void SetColor(const Math::Vector3f& color) { mSettings.color = color; }
        void SetIntensity(const float intensity) { mSettings.intensity = intensity; }
        void SetAmbientColor(const Math::Vector3f& color) { mSettings.ambientColor = color; }
        void SetShadowExtent(float extent);
        void SetShadowRange(float nearPlane, float farPlane);

        [[nodiscard]] const GlobalLightSettings& GetSettings() const noexcept { return mSettings; }
        [[nodiscard]] Math::Vector3f GetDirection() const;
        [[nodiscard]] const Math::Matrix4f& GetViewMatrix() const noexcept { return mView; }
        [[nodiscard]] const Math::Matrix4f& GetProjectionMatrix() const noexcept { return mProjection; }
        [[nodiscard]] const Math::Matrix4f& GetViewProjection() const noexcept { return mViewProjection; }

    private:
        void UpdateMatrices();

        bool mEnabled;
        GlobalLightSettings mSettings;
        Math::Matrix4f mView{Math::Matrix4f::Identity()};
        Math::Matrix4f mProjection{Math::Matrix4f::Identity()};
        Math::Matrix4f mViewProjection{Math::Matrix4f::Identity()};
    };
}
