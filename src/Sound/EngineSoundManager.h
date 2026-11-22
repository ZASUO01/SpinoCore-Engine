// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include "Assets/AssetCache.h"
#include "Sound/Synth/CommandQueue.h"
#include "SpinoCore/Sound/SoundManager.h"

struct SDL_AudioStream;

namespace SpinoCore::Sound::Synth {
    enum class AudioOutput : std::uint8_t;
}

namespace SpinoCore::Sound {
    namespace Resources {
        class AudioClip;
    }

    class EngineSoundManager final : public SoundManager {
    public:
        static constexpr std::size_t MaxSounds = 16;

        EngineSoundManager(const Assets::AssetCache<Resources::AudioClip>* clips, Synth::AudioOutput output);
        ~EngineSoundManager() override;

        EngineSoundManager(const EngineSoundManager&) = delete;
        EngineSoundManager& operator=(const EngineSoundManager&) = delete;
        EngineSoundManager(EngineSoundManager&&) = delete;
        EngineSoundManager& operator=(EngineSoundManager&&) = delete;

        SoundHandle Play(std::string_view clipId, bool loop, float volume) override;
        SoundHandle PlayFor(std::string_view clipId, float seconds, bool loop, float volume) override;

        bool Stop(SoundHandle handle) override;
        bool Pause(SoundHandle handle) override;
        bool Resume(SoundHandle handle) override;
        void StopAll() override;

        [[nodiscard]] bool IsPlaying(SoundHandle handle) const override;
        [[nodiscard]] bool IsAnyPlaying() const override;

        bool SetVolume(SoundHandle handle, float volume) override;
        void SetMasterVolume(float volume) override;
        [[nodiscard]] float GetMasterVolume() const override { return mMasterVolume.load(std::memory_order_relaxed); }

        void CleanUp() override;

        void RenderMix(std::span<float> interleavedStereo);
        void FeedStream(SDL_AudioStream* stream, int requestedBytes);

        [[nodiscard]] bool HasDevice() const noexcept { return mStream != nullptr; }

    private:
        enum class CommandType : std::uint8_t { Play, Stop, Pause, Resume, SetVolume, StopAll };

        struct Command {
            CommandType type{CommandType::StopAll};
            std::uint32_t slot{0};
            std::uint32_t generation{0};
            const Resources::AudioClip* clip{nullptr};
            bool loop{false};
            std::int64_t frameLimit{-1};
            float volume{1.0f};
        };

        struct Voice {
            const Resources::AudioClip* clip{nullptr};
            std::size_t frame{0};
            std::int64_t framesLeft{-1};
            float volume{1.0f};
            std::uint32_t generation{0};
            bool loop{false};
            bool paused{false};
            bool active{false};
        };

        struct Slot {
            std::uint32_t generation{0};
            std::uint64_t startedAt{0};
            bool active{false};
            bool paused{false};
        };

        static constexpr std::size_t QueueCapacity = 256;
        static constexpr std::size_t ScratchFrames = 512;

        SoundHandle Start(std::string_view clipId, bool loop, std::int64_t frameLimit, float volume);
        [[nodiscard]] bool IsBusy(std::size_t slot) const;
        [[nodiscard]] bool Owns(SoundHandle handle) const;
        [[nodiscard]] std::size_t ChooseSlot() const;
        bool Enqueue(const Command& command);
        void Apply(const Command& command);
        void Finish(std::size_t slot);
        void DrainCommands();
        void LockAudio() const;
        void UnlockAudio() const;

        const Assets::AssetCache<Resources::AudioClip>* mClips;
        Synth::AudioOutput mOutput;
        SDL_AudioStream* mStream{nullptr};

        std::array<Slot, MaxSounds> mSlots{};
        std::uint64_t mClock{0};
        std::array<Voice, MaxSounds> mVoices{};
        std::array<std::atomic<std::uint32_t>, MaxSounds> mFinished{};

        Synth::CommandQueue<Command, QueueCapacity> mCommands;
        std::atomic<float> mMasterVolume{1.0f};
        std::array<float, ScratchFrames * 2> mScratch{};
    };
}
