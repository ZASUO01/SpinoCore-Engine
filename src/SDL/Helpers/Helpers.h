// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <filesystem>
#include <span>
#include "SDL/Structs.h"

struct SDL_Window;

namespace SpinoCore::SDL::Helpers {
    [[nodiscard]] UniqueSDLWindow CreateSDLWindow(const char* title, int width, int height) noexcept;
    [[nodiscard]] UniqueGLContext CreateGLContext(SDL_Window* window) noexcept;
    [[nodiscard]] UniqueSDLSurface CreateFromImage(const std::filesystem::path& path) noexcept;
    [[nodiscard]] UniqueSDLSurface CreateFromImage(std::span<const std::uint8_t> encodedImage) noexcept;
    [[nodiscard]] UniqueTTFFont CreateTTFFont(const std::filesystem::path& path, float pointSize) noexcept;
    [[nodiscard]] UniqueTTFFont CreateTTFFont(std::span<const unsigned char> fontFile, float pointSize) noexcept;
}
