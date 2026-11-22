// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <span>
#include <vector>

namespace SpinoCore::Sound::Resources {
    inline constexpr int ClipSampleRate = 48000;
    inline constexpr int ClipChannels = 2;

    class AudioClip final {
    public:
        AudioClip();
        explicit AudioClip(std::vector<float> interleavedStereo);

        [[nodiscard]] std::span<const float> GetSamples() const noexcept { return mSamples; }
        [[nodiscard]] std::size_t GetFrameCount() const noexcept { return mSamples.size() / ClipChannels; }
        [[nodiscard]] float GetDuration() const noexcept { return static_cast<float>(GetFrameCount()) / static_cast<float>(ClipSampleRate); }

    private:
        std::vector<float> mSamples;
    };
}
