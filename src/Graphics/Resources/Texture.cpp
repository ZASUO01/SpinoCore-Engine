// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Texture.h"
#include <array>
#include <cstdint>
#include <stdexcept>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include <glad/glad.h>
#include "SDL/Helpers/Helpers.h"

namespace SpinoCore::Graphics::Resources {
    Texture::Texture() {
        CreateBlank();
    }

    Texture::Texture(const std::filesystem::path &imagePath) {
        CreateFromImage(imagePath);
    }

    Texture::~Texture() {
        if (mTextureID != 0) {
            glDeleteTextures(1, &mTextureID);
            mTextureID = 0;
        }
    }

    void Texture::SetActive(const std::uint32_t slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, mTextureID);
    }

    void Texture::CreateBlank() {
        static constexpr std::array<std::uint8_t, 4> white{255, 255, 255, 255};

        if (!Upload(white.data(), Utils::Size2D{1, 1}, 1)) {
            throw std::runtime_error("[Texture] Failed to create the blank texture.");
        }
    }

    void Texture::CreateFromImage(const std::filesystem::path &imagePath) {
        const auto surface = SDL::Helpers::CreateFromImage(imagePath);
        if (!surface) {
            throw std::runtime_error("[Texture] Failed to load image: " + imagePath.string());
        }

        if (const int pixelsPerLine = surface->pitch / SDL_BYTESPERPIXEL(surface->format);
            !Upload(surface->pixels, Utils::Size2D{surface->w, surface->h}, pixelsPerLine)) {
            throw std::runtime_error("[Texture] Failed to load image: " + imagePath.string());
        }
    }

    bool Texture::Upload(const void* rgbaPixels, const Utils::Size2D size, const int pixelsPerLine) {
        while (glGetError() != GL_NO_ERROR) {}

        glGenTextures(1, &mTextureID);
        glBindTexture(GL_TEXTURE_2D, mTextureID);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, pixelsPerLine);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.width, size.height,
                0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        const bool uploaded = glGetError() == GL_NO_ERROR;
        glBindTexture(GL_TEXTURE_2D, 0);

        if (!uploaded) {
            glDeleteTextures(1, &mTextureID);
            mTextureID = 0;
            return false;
        }

        mSize = size;
        return true;
    }
}
