// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <algorithm>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <vector>
#include "SynthUtils.h"

namespace SpinoCore::Sound::Synth {
    class Timbre {
    public:
        struct Config {
            WaveformType osc1Type = WaveformType::Sine;
            WaveformType osc2Type = WaveformType::Sine;
            float oscMix   = 0.0f;
            float detune   = 0.0f;

            float lfoRate   = 0.0f;
            float lfoAmount = 0.0f;

            float filterCutoff      = 20000.0f;
            float filterResonance   = 0.0f;
            float filterEnvAmount   = 0.0f;

            float attack    = 0.0f;
            float decay     = 0.0f;
            float sustain   = 1.0f;
            float release   = 0.0f;

            float delayTime     = 0.0f;
            float delayFeedback = 0.0f;
            float delayMix      = 0.0f;

            float reverbSize    = 0.0f;
            float reverbMix     = 0.0f;
            float volume = 1.0f;
        };

        Timbre();
        virtual ~Timbre();

        virtual void SetupInstrument() = 0;

        template<typename T>
        static std::unique_ptr<T> CreateUnique() {
            static_assert(std::is_base_of_v<Timbre, T>, "T must derive from Timbre");
            auto instrument = std::make_unique<T>();
            instrument->SetupInstrument();

            return instrument;
        }

        void Prepare(int sampleRate);
        void ApplyEffects(float& sampleL, float& sampleR);

        [[nodiscard]] const Config& GetConfig() const { return mConfig; }
        [[nodiscard]] int GetSampleRate() const { return mSampleRate; }
        void SetVolume(const float volume) { mConfig.volume = std::clamp(volume, 0.0f, 1.0f); }

    protected:
        Config mConfig;

    private:
        void ApplyDelay(float& sampleL, float& sampleR);
        void ApplyReverb(float& sampleL, float& sampleR);

        int mSampleRate{48000};

        std::vector<float> mDelayBufferL;
        std::vector<float> mDelayBufferR;
        std::size_t mDelayWriteIndex = 0;

        std::vector<float> mReverbBufferL;
        std::vector<float> mReverbBufferR;
        std::size_t mReverbWriteIndex = 0;
    };
}
