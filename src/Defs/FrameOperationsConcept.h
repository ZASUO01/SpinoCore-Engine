// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <concepts>

namespace SpinoCore::Defs {
    template <typename T>
    concept IsFrameOperator = requires(T& operations, float deltaTime, float alpha) {
        { operations.ProcessEvents() } -> std::same_as<bool>;
        { operations.UpdateSimulation(deltaTime) } -> std::same_as<void>;
        { operations.InterpolateEntities(alpha) } -> std::same_as<void>;
        { operations.Render() } -> std::same_as<void>;
    };
}