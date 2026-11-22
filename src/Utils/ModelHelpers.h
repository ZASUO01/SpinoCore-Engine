// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <filesystem>
#include <span>
#include "Graphics/Resources/ModelData.h"

namespace SpinoCore::Utils {
    [[nodiscard]] Graphics::Resources::ModelData ParseModel(std::span<const std::uint8_t> data, const std::filesystem::path& modelPath);
}
