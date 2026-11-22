// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Sound/Synth/Synthesizer.h"
#include <algorithm>
#include <cmath>
#include <utility>
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Sound/Synth/Timbre.h"

namespace SpinoCore::Sound::Synth {
    namespace {
        constexpr float MinimumRelease = 0.002f;
        constexpr float DetuneSpread = 0.005f;

        [[nodiscard]] float BasicWave(const WaveformType type, const double phase) {
            const auto p = static_cast<float>(phase);
            switch (type) {
                case WaveformType::Sine:
                    return Math::Operations::Sin(p * Math::Constants::TwoPI<float>);
                case WaveformType::Square:
                    return p < 0.5f ? 1.0f : -1.0f;
                case WaveformType::Sawtooth:
                    return p * 2.0f - 1.0f;
                case WaveformType::Triangle:
                    if (p < 0.25f) return p * 4.0f;
                    if (p < 0.75f) return 2.0f - p * 4.0f;
                    return p * 4.0f - 4.0f;
            }
            return 0.0f;
        }

        void Advance(double& phase, const double step) {
            phase += step;
            phase -= std::floor(phase);
        }
    }

    Synthesizer::Synthesizer(std::unique_ptr<Timbre> timbre, const SynthMode mode, const int sampleRate)
    :mTimbre(std::move(timbre))
    ,mMode(mode)
    ,mVoiceCapacity(mode == SynthMode::Monophonic ? 1 : MaxVoices)
    ,mSampleRate(std::max(sampleRate, 1))
    ,mSampleTimeStep(1.0f / static_cast<float>(mSampleRate))
    {
        if (mTimbre) mTimbre->Prepare(mSampleRate);
    }

    Synthesizer::~Synthesizer() = default;

    void Synthesizer::SetSampleRate(const int sampleRate) {
        mSampleRate = std::max(sampleRate, 1);
        mSampleTimeStep = 1.0f / static_cast<float>(mSampleRate);
        if (mTimbre) mTimbre->Prepare(mSampleRate);
    }

    void Synthesizer::SetTimbre(std::unique_ptr<Timbre> timbre) {
        mTimbre = std::move(timbre);
        if (mTimbre) mTimbre->Prepare(mSampleRate);
        StopAll();
    }

    bool Synthesizer::NoteOn(const int midiNote, const float velocity, const float duration) {
        if (!mTimbre || midiNote < MinMidiNote || midiNote > MaxMidiNote) return false;
        Start(VoiceFor(midiNote), midiNote, velocity, duration);
        return true;
    }

    bool Synthesizer::PlayNote(const Note note, const int octave, const float duration) {
        return NoteOn(ToMidi(note, octave), 1.0f, duration);
    }

    void Synthesizer::NoteOff(const int midiNote) {
        for (std::size_t index = 0; index < mVoiceCapacity; ++index) {
            Voice& voice = mVoices[index];
            if (voice.note == midiNote && voice.stage != EnvelopeStage::Idle && voice.stage != EnvelopeStage::Release) Release(voice);
        }
    }

    bool Synthesizer::PlayChord(const std::span<const int> midiNotes, const float velocity, const float duration) {
        if (!mTimbre || midiNotes.empty() || midiNotes.size() > mVoiceCapacity) return false;
        if (std::ranges::any_of(midiNotes, [](const int note) { return note < MinMidiNote || note > MaxMidiNote; })) return false;

        for (const int note : midiNotes) Start(VoiceFor(note), note, velocity, duration);
        return true;
    }

    void Synthesizer::ReleaseAll() {
        for (std::size_t index = 0; index < mVoiceCapacity; ++index) {
            Voice& voice = mVoices[index];
            if (voice.stage != EnvelopeStage::Idle && voice.stage != EnvelopeStage::Release) Release(voice);
        }
    }

    void Synthesizer::StopAll() {
        for (Voice& voice : mVoices) voice = Voice{};
    }

    std::size_t Synthesizer::GetActiveVoiceCount() const {
        return static_cast<std::size_t>(std::count_if(mVoices.begin(), mVoices.begin() + static_cast<std::ptrdiff_t>(mVoiceCapacity),
                                                      [](const Voice& voice) { return voice.stage != EnvelopeStage::Idle; }));
    }

    void Synthesizer::Render(const std::span<float> interleavedStereo) {
        if (!mTimbre) return;

        const float voiceGain = mMode == SynthMode::Monophonic ? 1.0f : 1.0f / std::sqrt(static_cast<float>(MaxVoices));
        const std::size_t frames = interleavedStereo.size() / 2;

        for (std::size_t frame = 0; frame < frames; ++frame) {
            float sampleL = 0.0f;
            float sampleR = 0.0f;
            for (std::size_t index = 0; index < mVoiceCapacity; ++index) {
                if (mVoices[index].stage != EnvelopeStage::Idle) RenderVoice(mVoices[index], sampleL, sampleR);
            }

            sampleL *= voiceGain;
            sampleR *= voiceGain;
            mTimbre->ApplyEffects(sampleL, sampleR);

            interleavedStereo[frame * 2] += sampleL * mVolume;
            interleavedStereo[frame * 2 + 1] += sampleR * mVolume;
        }
    }

    Synthesizer::Voice& Synthesizer::VoiceFor(const int midiNote) {
        if (mMode == SynthMode::Monophonic) return mVoices[0];

        const auto voices = std::span(mVoices).first(mVoiceCapacity);
        if (const auto same = std::ranges::find_if(voices, [midiNote](const Voice& voice) {
                return voice.note == midiNote && voice.stage != EnvelopeStage::Idle;
            }); same != voices.end()) {
            return *same;
        }
        if (const auto idle = std::ranges::find_if(voices, [](const Voice& voice) { return voice.stage == EnvelopeStage::Idle; }); idle != voices.end()) {
            return *idle;
        }

        Voice* quietestReleasing = nullptr;
        for (Voice& voice : voices) {
            if (voice.stage == EnvelopeStage::Release && (!quietestReleasing || voice.level < quietestReleasing->level)) quietestReleasing = &voice;
        }
        if (quietestReleasing) return *quietestReleasing;

        return *std::ranges::min_element(voices, {}, &Voice::startedAt);
    }

    void Synthesizer::Start(Voice& voice, const int midiNote, const float velocity, const float duration) {
        if (voice.stage == EnvelopeStage::Idle) voice = Voice{};

        voice.note = midiNote;
        voice.frequency = MidiToFrequency(midiNote);
        voice.velocity = std::clamp(velocity, 0.0f, 1.0f);
        voice.remaining = duration;
        voice.stage = EnvelopeStage::Attack;
        voice.startedAt = ++mClock;
    }

    void Synthesizer::Release(Voice& voice) const {
        voice.stage = EnvelopeStage::Release;
        voice.releaseRate = voice.level / std::max(mTimbre->GetConfig().release, MinimumRelease);
    }

    void Synthesizer::StepEnvelope(Voice& voice) const {
        const auto& config = mTimbre->GetConfig();
        const float sustain = std::clamp(config.sustain, 0.0f, 1.0f);

        if (voice.remaining > 0.0f) {
            voice.remaining -= mSampleTimeStep;
            if (voice.remaining <= 0.0f && voice.stage != EnvelopeStage::Release) Release(voice);
        }

        switch (voice.stage) {
            case EnvelopeStage::Attack:
                voice.level = config.attack > 0.0f ? voice.level + mSampleTimeStep / config.attack : 1.0f;
                if (voice.level >= 1.0f) {
                    voice.level = 1.0f;
                    voice.stage = EnvelopeStage::Decay;
                }
                break;
            case EnvelopeStage::Decay:
                voice.level = config.decay > 0.0f ? voice.level - mSampleTimeStep * (1.0f - sustain) / config.decay : sustain;
                if (voice.level <= sustain) {
                    voice.level = sustain;
                    voice.stage = EnvelopeStage::Sustain;
                }
                break;
            case EnvelopeStage::Sustain:
                voice.level = sustain;
                break;
            case EnvelopeStage::Release:
                voice.level -= voice.releaseRate * mSampleTimeStep;
                if (voice.level <= 0.0f) voice = Voice{};
                break;
            case EnvelopeStage::Idle:
                break;
        }
    }

    void Synthesizer::RenderVoice(Voice& voice, float& sampleL, float& sampleR) const {
        const auto& config = mTimbre->GetConfig();

        const double step = static_cast<double>(voice.frequency) * static_cast<double>(mSampleTimeStep);
        const double spread = static_cast<double>(config.detune * DetuneSpread);
        Advance(voice.phaseL, step * (1.0 - spread));
        Advance(voice.phaseR, step * (1.0 + spread));

        const float mix = std::clamp(config.oscMix, 0.0f, 1.0f);
        float left = std::lerp(BasicWave(config.osc1Type, voice.phaseL), BasicWave(config.osc2Type, voice.phaseL), mix);
        float right = std::lerp(BasicWave(config.osc1Type, voice.phaseR), BasicWave(config.osc2Type, voice.phaseR), mix);

        StepEnvelope(voice);
        const float amplitude = voice.level * voice.velocity * std::clamp(config.volume, 0.0f, 1.0f);
        left *= amplitude;
        right *= amplitude;

        if (config.lfoAmount > 0.0f && config.lfoRate > 0.0f) {
            voice.lfoPhase += config.lfoRate * mSampleTimeStep;
            voice.lfoPhase -= std::floor(voice.lfoPhase);
            const float tremolo = 1.0f - std::abs(Math::Operations::Sin(voice.lfoPhase * Math::Constants::TwoPI<float>)) * config.lfoAmount * 0.5f;
            left *= tremolo;
            right *= tremolo;
        }

        if (config.filterCutoff < 20000.0f) {
            const float omega = Math::Constants::TwoPI<float> * std::clamp(config.filterCutoff, 20.0f, 20000.0f) * mSampleTimeStep;
            const float alpha = omega / (omega + 1.0f);
            voice.filterL += alpha * (left - voice.filterL);
            voice.filterR += alpha * (right - voice.filterR);
            left = voice.filterL;
            right = voice.filterR;
        }

        sampleL += left;
        sampleR += right;
    }
}
