// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Sound/Synth/Timbre.h"

namespace SpinoCore::Sound::Synth {
    namespace {
        constexpr float MaxDelaySeconds = 2.0f;
        constexpr float ReverbSeconds = 0.034f;
    }

    void Timbre::Prepare(const int sampleRate) {
        mSampleRate = std::max(sampleRate, 1);

        const auto delaySize = static_cast<std::size_t>(MaxDelaySeconds * static_cast<float>(mSampleRate));
        mDelayBufferL.assign(delaySize, 0.0f);
        mDelayBufferR.assign(delaySize, 0.0f);
        mDelayWriteIndex = 0;

        const auto reverbSize = std::max<std::size_t>(1, static_cast<std::size_t>(ReverbSeconds * static_cast<float>(mSampleRate)));
        mReverbBufferL.assign(reverbSize, 0.0f);
        mReverbBufferR.assign(reverbSize, 0.0f);
        mReverbWriteIndex = 0;
    }

    void Timbre::ApplyEffects(float& sampleL, float& sampleR) {
        ApplyDelay(sampleL, sampleR);
        ApplyReverb(sampleL, sampleR);
    }

    void Timbre::ApplyDelay(float &sampleL, float &sampleR) {
        if (mDelayBufferL.empty() || mConfig.delayMix <= 0.0f || mConfig.delayTime <= 0.0f) return;

        const std::size_t bufferSize = mDelayBufferL.size();
        const std::size_t delaySamples = std::min(static_cast<std::size_t>(mConfig.delayTime * static_cast<float>(mSampleRate)), bufferSize - 1);
        const std::size_t readIndex = (mDelayWriteIndex + bufferSize - delaySamples) % bufferSize;

        const float echoL = mDelayBufferL[readIndex];
        const float echoR = mDelayBufferR[readIndex];
        const float feedback = std::clamp(mConfig.delayFeedback, 0.0f, 0.95f);

        mDelayBufferL[mDelayWriteIndex] = sampleL + echoL * feedback;
        mDelayBufferR[mDelayWriteIndex] = sampleR + echoR * feedback;
        mDelayWriteIndex = (mDelayWriteIndex + 1) % bufferSize;

        sampleL = sampleL * (1.0f - mConfig.delayMix) + echoL * mConfig.delayMix;
        sampleR = sampleR * (1.0f - mConfig.delayMix) + echoR * mConfig.delayMix;
    }

    void Timbre::ApplyReverb(float& sampleL, float& sampleR) {
        if (mReverbBufferL.empty() || mConfig.reverbMix <= 0.0f || mConfig.reverbSize <= 0.0f) return;

        const float gain = std::clamp(mConfig.reverbSize, 0.0f, 0.85f);
        const float delayedL = mReverbBufferL[mReverbWriteIndex];
        const float delayedR = mReverbBufferR[mReverbWriteIndex];

        const float reverbOutL = -gain * sampleL + delayedL;
        const float reverbOutR = -gain * sampleR + delayedR;

        mReverbBufferL[mReverbWriteIndex] = sampleL + gain * delayedL;
        mReverbBufferR[mReverbWriteIndex] = sampleR + gain * delayedR;
        mReverbWriteIndex = (mReverbWriteIndex + 1) % mReverbBufferL.size();

        sampleL = sampleL * (1.0f - mConfig.reverbMix) + reverbOutL * mConfig.reverbMix;
        sampleR = sampleR * (1.0f - mConfig.reverbMix) + reverbOutR * mConfig.reverbMix;
    }
}
