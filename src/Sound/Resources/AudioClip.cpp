// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "AudioClip.h"
#include <utility>

namespace SpinoCore::Sound::Resources {
    namespace {
        constexpr std::size_t SilentFrames = ClipSampleRate / 20;
    }

    AudioClip::AudioClip() : mSamples(SilentFrames * ClipChannels, 0.0f) {}

    AudioClip::AudioClip(std::vector<float> interleavedStereo) : mSamples(std::move(interleavedStereo)) {
        if (mSamples.size() % ClipChannels != 0) mSamples.push_back(0.0f);
    }
}
