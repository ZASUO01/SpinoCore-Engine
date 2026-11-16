// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <string>

namespace SpinoCore::Graphics::Shaders {
    [[nodiscard]] std::uint32_t BuildFromStrings(const std::string& vertexShaderSource, const std::string& fragmentShaderSource);
}
