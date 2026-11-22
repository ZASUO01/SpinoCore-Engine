// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include "CommandQueue.h"
#include "SpinoCore/Sound/Synth/SynthManager.h"
#include "SpinoCore/Sound/Synth/Synthesizer.h"

struct SDL_AudioStream;

namespace SpinoCore::Sound::Synth {
    enum class AudioOutput : std::uint8_t {
        Device,
        Offline
    };

    class EngineSynthManager final : public SynthManager {
    public:
        static constexpr int OfflineSampleRate = 48000;

        explicit EngineSynthManager(AudioOutput output = AudioOutput::Device);
        ~EngineSynthManager() override;

        EngineSynthManager(const EngineSynthManager&) = delete;
        EngineSynthManager& operator=(const EngineSynthManager&) = delete;
        EngineSynthManager(EngineSynthManager&&) = delete;
        EngineSynthManager& operator=(EngineSynthManager&&) = delete;

        bool Register(std::string name, std::unique_ptr<Timbre> timbre, SynthMode mode) override;
        bool Unregister(std::string_view name) override;

        bool NoteOn(std::string_view name, int midiNote, float velocity, float duration) override;
        bool NoteOff(std::string_view name, int midiNote) override;
        bool PlayChord(std::string_view name, std::span<const int> midiNotes, float velocity, float duration) override;
        bool Release(std::string_view name) override;
        void StopAll() override;

        bool SetVolume(std::string_view name, float volume) override;
        void SetMasterVolume(float volume) override;
        [[nodiscard]] float GetMasterVolume() const override { return mMasterVolume.load(std::memory_order_relaxed); }

        [[nodiscard]] bool Contains(std::string_view name) const override;
        void CleanUp() override;

        void RenderMix(std::span<float> interleavedStereo);
        void FeedStream(SDL_AudioStream* stream, int requestedBytes);

        [[nodiscard]] bool HasDevice() const noexcept { return mStream != nullptr; }
        [[nodiscard]] int GetSampleRate() const noexcept { return mSampleRate; }
        [[nodiscard]] const Synthesizer* Find(std::string_view name) const;

    private:
        enum class CommandType : std::uint8_t { NoteOn, NoteOff, Chord, Release, StopAll, SetVolume };

        struct Command {
            CommandType type{CommandType::StopAll};
            Synthesizer* synth{nullptr};
            std::array<int, MaxVoices> notes{};
            std::uint8_t noteCount{0};
            float velocity{1.0f};
            float duration{0.0f};
        };

        struct Entry {
            std::string name;
            std::unique_ptr<Synthesizer> synth;
        };

        static constexpr std::size_t QueueCapacity = 256;
        static constexpr std::size_t ScratchFrames = 512;

        [[nodiscard]] Synthesizer* FindSynth(std::string_view name) const;
        bool Enqueue(const Command& command);
        static void Apply(const Command& command);
        void DrainCommands();
        void LockAudio() const;
        void UnlockAudio() const;

        AudioOutput mOutput;
        SDL_AudioStream* mStream{nullptr};
        int mSampleRate{OfflineSampleRate};
        std::vector<Entry> mSynths;
        CommandQueue<Command, QueueCapacity> mCommands;
        std::atomic<float> mMasterVolume{1.0f};
        std::array<float, ScratchFrames * 2> mScratch{};
    };
}
