// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

/// @brief Scene lighting.
namespace SpinoCore::Core::Lighting {
    /// @brief Settings of the global light.
    struct GlobalLightSettings {
        /// Position of the light; the light shines from it towards the target.
        Math::Vector3f position{4.0f, 8.0f, 4.0f};
        /// Point the light looks at.
        Math::Vector3f target{Math::Vector3f::Zero()};
        /// Color of the direct light.
        Math::Vector3f color{Math::Vector3f::One()};
        /// Strength of the direct light.
        float intensity{1.0f};
        /// Light added to every surface, including shadowed ones.
        Math::Vector3f ambientColor{0.25f, 0.25f, 0.28f};
        /// Half the width of the square area covered by the shadow map.
        float shadowExtent{10.0f};
        /// Closest distance from the light that casts shadows.
        float shadowNear{0.1f};
        /// Farthest distance from the light that casts shadows.
        float shadowFar{50.0f};
    };

    /// @brief Directional light with an ambient term and a shadow map.
    ///
    /// When disabled, models are drawn with their plain colors.
    class GlobalLight final {
    public:
        /// @brief Creates the light.
        /// @param enabled Whether lighting and shadows are used.
        /// @param settings Initial settings.
        explicit GlobalLight(bool enabled = false, const GlobalLightSettings& settings = {});

        /// @return Whether lighting and shadows are used.
        [[nodiscard]] bool IsEnabled() const noexcept { return mEnabled; }

        /// @brief Moves the light.
        /// @param position New position.
        void SetPosition(const Math::Vector3f& position);
        /// @brief Changes only the height of the light.
        /// @param height New Y position.
        void SetHeight(float height);
        /// @brief Changes the point the light looks at.
        /// @param target New target.
        void SetTarget(const Math::Vector3f& target);
        /// @brief Changes the direct light color.
        /// @param color New color.
        void SetColor(const Math::Vector3f& color) { mSettings.color = color; }
        /// @brief Changes the direct light strength.
        /// @param intensity New strength.
        void SetIntensity(const float intensity) { mSettings.intensity = intensity; }
        /// @brief Changes the ambient light.
        /// @param color New ambient color.
        void SetAmbientColor(const Math::Vector3f& color) { mSettings.ambientColor = color; }
        /// @brief Changes the area covered by shadows.
        /// @param extent Half width of the area; very small values are clamped.
        void SetShadowExtent(float extent);
        /// @brief Changes the depth range that casts shadows.
        /// @param nearPlane Closest distance from the light.
        /// @param farPlane Farthest distance; it is kept beyond the near plane.
        void SetShadowRange(float nearPlane, float farPlane);

        /// @return The current settings.
        [[nodiscard]] const GlobalLightSettings& GetSettings() const noexcept { return mSettings; }
        /// @return The unit direction the light travels, or -Y when position and target are equal.
        [[nodiscard]] Math::Vector3f GetDirection() const;
        /// @return The view matrix of the shadow map.
        [[nodiscard]] const Math::Matrix4f& GetViewMatrix() const noexcept { return mView; }
        /// @return The orthographic projection of the shadow map.
        [[nodiscard]] const Math::Matrix4f& GetProjectionMatrix() const noexcept { return mProjection; }
        /// @return The projection times the view of the shadow map.
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
