// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "Texture.h"
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include <glad/glad.h>
#include "SDL/Helpers/Helpers.h"
#include "SDL/Structs.h"

namespace SpinoCore::Graphics::Resources {
    Texture::Texture() {
        CreateBlank();
    }

    Texture::Texture(const int width, const int height, const CreationType type) {
        if (width <= 0 || height <= 0) throw std::invalid_argument("[Texture] Width and height must be positive.");
        if (!Allocate(nullptr, Utils::Size2D{width, height}, 0, type)) {
            throw std::runtime_error("[Texture] Failed to create the texture.");
        }
    }

    Texture::Texture(const std::filesystem::path &imagePath) {
        CreateFromImage(imagePath);
    }

    Texture::Texture(SDL_Surface *surface) {
        CreateFromSurface(surface);
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

        if (!Allocate(white.data(), Utils::Size2D{1, 1}, 1, CreationType::Empty)) {
            throw std::runtime_error("[Texture] Failed to create the blank texture.");
        }
    }

    void Texture::CreateFromImage(const std::filesystem::path &imagePath) {
        const auto surface = SDL::Helpers::CreateFromImage(imagePath);
        if (!surface) {
            throw std::runtime_error("[Texture] Failed to load image: " + imagePath.string());
        }

        if (const int pixelsPerLine = surface->pitch / SDL_BYTESPERPIXEL(surface->format);
            !Allocate(surface->pixels, Utils::Size2D{surface->w, surface->h}, pixelsPerLine, CreationType::Empty)) {
            throw std::runtime_error("[Texture] Failed to load image: " + imagePath.string());
        }
    }

    void Texture::CreateFromSurface(SDL_Surface *surface) {
        if (!surface) throw std::invalid_argument("[Texture] Cannot create a texture from a null surface.");

        SDL::UniqueSDLSurface converted{};
        if (surface->format != SDL_PIXELFORMAT_RGBA32) {
            converted.reset(SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32));
            if (!converted) throw std::runtime_error("[Texture] Failed to convert the surface to RGBA32.");
            surface = converted.get();
        }

        if (const int pixelsPerLine = surface->pitch / SDL_BYTESPERPIXEL(surface->format);
            !Allocate(surface->pixels, Utils::Size2D{surface->w, surface->h}, pixelsPerLine, CreationType::Empty)) {
            throw std::runtime_error("[Texture] Failed to upload the surface.");
        }
    }

    bool Texture::Allocate(const void* pixels, const Utils::Size2D size, const int pixelsPerLine, const CreationType type) {
        while (glGetError() != GL_NO_ERROR) {}

        glGenTextures(1, &mTextureID);
        glBindTexture(GL_TEXTURE_2D, mTextureID);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, pixelsPerLine);

        if (type == CreationType::Depth) {
            static constexpr std::array<float, 4> border{1.0f, 1.0f, 1.0f, 1.0f};
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, size.width, size.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, pixels);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border.data());
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.width, size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        const bool allocated = glGetError() == GL_NO_ERROR;
        glBindTexture(GL_TEXTURE_2D, 0);

        if (!allocated) {
            glDeleteTextures(1, &mTextureID);
            mTextureID = 0;
            return false;
        }

        mSize = size;
        return true;
    }
}
