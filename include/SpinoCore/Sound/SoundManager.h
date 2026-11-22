// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <string_view>

namespace SpinoCore::Sound {
    struct SoundHandle {
        std::uint32_t slot{0};
        std::uint32_t generation{0};

        [[nodiscard]] constexpr bool IsValid() const noexcept { return generation != 0; }
    };

    class SoundManager {
    public:
        virtual ~SoundManager() = default;

        virtual SoundHandle Play(std::string_view clipId, bool loop = false, float volume = 1.0f) = 0;
        virtual SoundHandle PlayFor(std::string_view clipId, float seconds, bool loop = true, float volume = 1.0f) = 0;

        virtual bool Stop(SoundHandle handle) = 0;
        virtual bool Pause(SoundHandle handle) = 0;
        virtual bool Resume(SoundHandle handle) = 0;
        virtual void StopAll() = 0;

        [[nodiscard]] virtual bool IsPlaying(SoundHandle handle) const = 0;
        [[nodiscard]] virtual bool IsAnyPlaying() const = 0;

        virtual bool SetVolume(SoundHandle handle, float volume) = 0;
        virtual void SetMasterVolume(float volume) = 0;
        [[nodiscard]] virtual float GetMasterVolume() const = 0;

        virtual void CleanUp() = 0;
    };
}
