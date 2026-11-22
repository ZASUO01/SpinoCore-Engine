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
    /// @brief Sound of an instrument; subclasses fill mConfig in SetupInstrument.
    class Timbre {
    public:
        /// @brief Parameters of the sound.
        struct Config {
            /// Waveform of the first oscillator.
            WaveformType osc1Type = WaveformType::Sine;
            /// Waveform of the second oscillator.
            WaveformType osc2Type = WaveformType::Sine;
            /// Mix from 0 (first oscillator) to 1 (second oscillator).
            float oscMix   = 0.0f;
            /// Spread between the left and right channels, which widens the sound.
            float detune   = 0.0f;

            /// Speed of the vibrato in hertz.
            float lfoRate   = 0.0f;
            /// Depth of the vibrato.
            float lfoAmount = 0.0f;

            /// Low-pass filter cutoff in hertz.
            float filterCutoff      = 20000.0f;
            /// Emphasis around the cutoff.
            float filterResonance   = 0.0f;
            /// How much the envelope opens the filter.
            float filterEnvAmount   = 0.0f;

            /// Seconds to reach full volume.
            float attack    = 0.0f;
            /// Seconds to fall to the sustain level.
            float decay     = 0.0f;
            /// Level held while the note is on, from 0 to 1.
            float sustain   = 1.0f;
            /// Seconds to fade out after the note is released.
            float release   = 0.0f;

            /// Echo delay in seconds, up to 2.
            float delayTime     = 0.0f;
            /// Amount of each echo fed back.
            float delayFeedback = 0.0f;
            /// Echo level.
            float delayMix      = 0.0f;

            /// Size of the reverb.
            float reverbSize    = 0.0f;
            /// Reverb level.
            float reverbMix     = 0.0f;
            /// Volume from 0 to 1.
            float volume = 1.0f;
        };

        /// @brief Creates a timbre with the default configuration.
        Timbre() = default;
        /// @brief Destroys the timbre.
        virtual ~Timbre() = default;

        /// @brief Fills mConfig with the parameters of the instrument.
        virtual void SetupInstrument() = 0;

        /// @brief Creates a timbre and calls SetupInstrument.
        /// @tparam T Subclass of Timbre.
        /// @return The configured timbre.
        template<typename T>
        static std::unique_ptr<T> CreateUnique() {
            static_assert(std::is_base_of_v<Timbre, T>, "T must derive from Timbre");
            auto instrument = std::make_unique<T>();
            instrument->SetupInstrument();

            return instrument;
        }

        /// @brief Allocates the effect buffers for a sample rate.
        /// @param sampleRate Sample rate in hertz.
        void Prepare(int sampleRate);
        /// @brief Applies the delay and the reverb to one stereo sample.
        /// @param sampleL Left sample, changed in place.
        /// @param sampleR Right sample, changed in place.
        void ApplyEffects(float& sampleL, float& sampleR);

        /// @return The sound parameters.
        [[nodiscard]] const Config& GetConfig() const { return mConfig; }
        /// @return The sample rate given to Prepare.
        [[nodiscard]] int GetSampleRate() const { return mSampleRate; }
        /// @brief Changes the volume.
        /// @param volume Value clamped to 0..1.
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
