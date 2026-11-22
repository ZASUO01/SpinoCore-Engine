// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>
#include "SDL/Structs.h"

struct SDL_Window;

namespace SpinoCore::SDL::Helpers {
    struct DecodedAudio final {
        std::vector<float> samples;
        int channels{0};
        int sampleRate{0};
        float duration{0.0f};
    };

    [[nodiscard]] UniqueSDLWindow CreateSDLWindow(const char* title, int width, int height) noexcept;
    [[nodiscard]] UniqueGLContext CreateGLContext(SDL_Window* window) noexcept;
    [[nodiscard]] UniqueSDLSurface CreateFromImage(const std::filesystem::path& path);
    [[nodiscard]] UniqueSDLSurface CreateFromImage(std::span<const std::uint8_t> encodedImage) noexcept;
    [[nodiscard]] UniqueTTFFont CreateTTFFont(const std::filesystem::path& path, float pointSize);
    [[nodiscard]] UniqueTTFFont CreateTTFFont(std::span<const unsigned char> fontFile, float pointSize) noexcept;
    [[nodiscard]] std::optional<DecodedAudio> LoadAudio(std::span<const std::uint8_t> encodedAudio, int sampleRate, int channels);
    [[nodiscard]] std::optional<DecodedAudio> LoadAudio(const std::filesystem::path& path, int sampleRate, int channels);
}
