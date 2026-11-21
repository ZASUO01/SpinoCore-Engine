// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <filesystem>
#include <memory>
#include "SpinoCore/Utils/Size2D.h"

namespace SpinoCore::Graphics::Resources {
    class Texture final {
    public:
        Texture() = default;
        explicit Texture(const std::filesystem::path& imagePath);
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&&) = delete;
        Texture& operator=(Texture&&) = delete;

        void SetActive(std::uint32_t slot = 0) const;
        [[nodiscard]] std::uint32_t GetID() const noexcept { return mTextureID; }
        [[nodiscard]] Utils::Size2D GetSize() const noexcept { return mSize; }

        [[nodiscard]] static std::unique_ptr<Texture> CreateBlankTexture();

        private:
            [[nodiscard]] bool Upload(const void* rgbaPixels, Utils::Size2D size, int pixelsPerLine);

            std::uint32_t mTextureID{0};
            Utils::Size2D mSize{0, 0};
    };
}
