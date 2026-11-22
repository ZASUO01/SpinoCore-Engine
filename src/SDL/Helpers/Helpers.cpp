// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SDL/Helpers/Helpers.h"
#include <array>
#include <cstddef>
#include <string>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "SDL/Structs.h"

namespace SpinoCore::SDL::Helpers {
    UniqueSDLWindow CreateSDLWindow(const char *title, const int width, const int height) noexcept {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
#ifndef NDEBUG
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

        return UniqueSDLWindow(SDL_CreateWindow(title, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN));
    }

    UniqueGLContext CreateGLContext(SDL_Window* window) noexcept {
        return UniqueGLContext(SDL_GL_CreateContext(window));
    }

    UniqueSDLSurface CreateFromImage(const std::span<const std::uint8_t> encodedImage) noexcept {
        if (encodedImage.empty()) return nullptr;

        SDL_IOStream* stream = SDL_IOFromConstMem(encodedImage.data(), encodedImage.size());
        if (!stream) return nullptr;

        UniqueSDLSurface loaded{IMG_Load_IO(stream, true)};
        if (!loaded) return nullptr;

        if (loaded->format == SDL_PIXELFORMAT_RGBA32) return loaded;
        return UniqueSDLSurface{SDL_ConvertSurface(loaded.get(), SDL_PIXELFORMAT_RGBA32)};
    }

    UniqueSDLSurface CreateFromImage(const std::filesystem::path &path) noexcept {
        const std::u8string utf8 = path.u8string();
        const auto cpath = reinterpret_cast<const char*>(utf8.c_str());

        UniqueSDLSurface loaded{IMG_Load(cpath)};
        if (!loaded) return nullptr;

        if (loaded->format == SDL_PIXELFORMAT_RGBA32) return loaded;

        UniqueSDLSurface converted{SDL_ConvertSurface(loaded.get(), SDL_PIXELFORMAT_RGBA32)};
        if (!converted) return nullptr;

        return converted;
    }

    UniqueTTFFont CreateTTFFont(const std::filesystem::path &path, const float pointSize) noexcept {
        const std::u8string utf8 = path.u8string();
        const auto cpath = reinterpret_cast<const char*>(utf8.c_str());

        return UniqueTTFFont(TTF_OpenFont(cpath, pointSize));
    }

    UniqueTTFFont CreateTTFFont(const std::span<const unsigned char> fontFile, const float pointSize) noexcept {
        SDL_IOStream* stream = SDL_IOFromConstMem(fontFile.data(), fontFile.size());
        if (!stream) return nullptr;

        return UniqueTTFFont(TTF_OpenFontIO(stream, true, pointSize));
    }

    std::optional<DecodedAudio> LoadAudio(const std::span<const std::uint8_t> encodedAudio, const int sampleRate, const int channels) {
        if (encodedAudio.empty() || sampleRate <= 0 || channels <= 0 || !MIX_Init()) return std::nullopt;

        SDL_IOStream* stream = SDL_IOFromConstMem(encodedAudio.data(), encodedAudio.size());
        MIX_AudioDecoder* decoder = stream ? MIX_CreateAudioDecoder_IO(stream, true, 0) : nullptr;
        if (!decoder) {
            MIX_Quit();
            return std::nullopt;
        }

        const SDL_AudioSpec spec{.format = SDL_AUDIO_F32, .channels = channels, .freq = sampleRate};
        DecodedAudio audio{.samples = {}, .channels = channels, .sampleRate = sampleRate, .duration = 0.0f};
        std::array<float, 4096> block{};
        bool failed = false;

        while (true) {
            const int bytes = MIX_DecodeAudio(decoder, block.data(), static_cast<int>(sizeof(block)), &spec);
            if (bytes < 0) failed = true;
            if (bytes <= 0) break;
            audio.samples.insert(audio.samples.end(), block.begin(), block.begin() + bytes / static_cast<int>(sizeof(float)));
        }

        MIX_DestroyAudioDecoder(decoder);
        MIX_Quit();
        if (failed || audio.samples.empty()) return std::nullopt;

        audio.duration = static_cast<float>(audio.samples.size() / static_cast<std::size_t>(channels)) / static_cast<float>(sampleRate);
        return audio;
    }

    std::optional<DecodedAudio> LoadAudio(const std::filesystem::path& path, const int sampleRate, const int channels) {
        const std::u8string utf8 = path.u8string();
        std::size_t size = 0;
        void* contents = SDL_LoadFile(reinterpret_cast<const char*>(utf8.c_str()), &size);
        if (!contents) return std::nullopt;

        const std::span bytes(static_cast<const std::uint8_t*>(contents), size);
        std::optional<DecodedAudio> audio = LoadAudio(bytes, sampleRate, channels);
        SDL_free(contents);
        return audio;
    }
}
