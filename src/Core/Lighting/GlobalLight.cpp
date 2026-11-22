// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/Lighting/GlobalLight.h"
#include <algorithm>
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Math/MatrixFactory.h"

namespace SpinoCore::Core::Lighting {
    GlobalLight::GlobalLight(const bool enabled, const GlobalLightSettings& settings)
    :mEnabled(enabled)
    ,mSettings(settings)
    {
        UpdateMatrices();
    }

    void GlobalLight::SetPosition(const Math::Vector3f& position) {
        mSettings.position = position;
        UpdateMatrices();
    }

    void GlobalLight::SetHeight(const float height) {
        mSettings.position.y = height;
        UpdateMatrices();
    }

    void GlobalLight::SetTarget(const Math::Vector3f& target) {
        mSettings.target = target;
        UpdateMatrices();
    }

    void GlobalLight::SetShadowExtent(const float extent) {
        mSettings.shadowExtent = std::max(extent, 0.01f);
        UpdateMatrices();
    }

    void GlobalLight::SetShadowRange(const float nearPlane, const float farPlane) {
        mSettings.shadowNear = nearPlane;
        mSettings.shadowFar = std::max(farPlane, nearPlane + 0.01f);
        UpdateMatrices();
    }

    Math::Vector3f GlobalLight::GetDirection() const {
        const Math::Vector3f direction = mSettings.target - mSettings.position;
        return direction.LengthSq() > 0.0f ? direction.Normalized() : Math::Vector3f::NegUnitY();
    }

    void GlobalLight::UpdateMatrices() {
        const Math::Vector3f direction = GetDirection();
        const Math::Vector3f up = Math::Operations::Abs(direction.y) > 0.99f ? Math::Vector3f::UnitZ() : Math::Vector3f::UnitY();
        const bool hasPosition = (mSettings.target - mSettings.position).LengthSq() > 0.0f;
        const Math::Vector3f eye = hasPosition ? mSettings.position : mSettings.target - direction;
        const float extent = mSettings.shadowExtent;

        mView = Math::MatrixFactory::CreateLookAt(eye, mSettings.target, up);
        mProjection = Math::MatrixFactory::CreateOrtho(-extent, extent, -extent, extent, mSettings.shadowNear, mSettings.shadowFar);
        mViewProjection = mProjection * mView;
    }
}
