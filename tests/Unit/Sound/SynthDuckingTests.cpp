// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <algorithm>
#include <cmath>
#include <vector>
#include <gtest/gtest.h>
#include "Sound/Synth/EngineSynthManager.h"
#include "SpinoCore/Sound/Synth/SynthManager.h"
#include "SpinoCore/Sound/Synth/Synthesizer.h"
#include "SpinoCore/Sound/Synth/Timbre.h"

namespace {
    using SpinoCore::Sound::Synth::AudioOutput;
    using SpinoCore::Sound::Synth::EngineSynthManager;
    using SpinoCore::Sound::Synth::SynthManager;
    using SpinoCore::Sound::Synth::SynthMode;
    using SpinoCore::Sound::Synth::Timbre;

    class SteadyTone final : public Timbre {
    public:
        void SetupInstrument() override {
            mConfig.volume = 0.5f;
        }
    };

    [[nodiscard]] float PeakOf(EngineSynthManager& manager) {
        std::vector<float> samples(2048, 0.0f);
        manager.RenderMix(samples);
        float peak = 0.0f;
        for (const float sample : samples) peak = std::max(peak, std::abs(sample));
        return peak;
    }

    class SynthDuckingTests : public testing::Test {
    protected:
        void SetUp() override {
            manager.Register("lead", Timbre::CreateUnique<SteadyTone>(), SynthMode::Monophonic);
            synths.NoteOn("lead", 69);
            undocked = PeakOf(manager);
        }

        EngineSynthManager manager{AudioOutput::Offline};
        SynthManager& synths{manager};
        float undocked{0.0f};
    };

    TEST_F(SynthDuckingTests, WhileDuckedTheSynthIsQuieter) {
        manager.SetDucking(0.6f);
        static_cast<void>(PeakOf(manager));

        EXPECT_NEAR(PeakOf(manager), undocked * 0.6f, 0.02f);
    }

    TEST_F(SynthDuckingTests, WhileDuckedTheSynthKeepsSounding) {
        manager.SetDucking(0.6f);

        EXPECT_GT(PeakOf(manager), 0.0f);
    }

    TEST_F(SynthDuckingTests, TheDuckingFadesInsteadOfJumping) {
        manager.SetDucking(0.0f);
        std::vector<float> samples(2048, 0.0f);

        manager.RenderMix(samples);

        EXPECT_GT(std::abs(samples[2]), 0.0f);
    }

    TEST_F(SynthDuckingTests, WithoutDuckingTheSynthIsBackToFullLevel) {
        manager.SetDucking(0.6f);
        static_cast<void>(PeakOf(manager));
        manager.SetDucking(1.0f);
        static_cast<void>(PeakOf(manager));

        EXPECT_NEAR(PeakOf(manager), undocked, 0.02f);
    }
}
