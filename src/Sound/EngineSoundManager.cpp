// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineSoundManager.h"
#include <algorithm>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_error.h>
#include "Sound/Resources/AudioClip.h"
#include "Sound/Synth/EngineSynthManager.h"
#include "SpinoCore/Logs/Logger.h"

namespace SpinoCore::Sound {
    namespace {
        void SDLCALL OnAudioRequested(void* userdata, SDL_AudioStream* stream, const int additionalAmount, int) {
            static_cast<EngineSoundManager*>(userdata)->FeedStream(stream, additionalAmount);
        }
    }

    EngineSoundManager::EngineSoundManager(const Assets::AssetCache<Resources::AudioClip>* clips, const Synth::AudioOutput output)
    :mClips(clips)
    ,mOutput(output)
    {
        if (mOutput == Synth::AudioOutput::Offline) return;

        const SDL_AudioSpec spec{.format = SDL_AUDIO_F32, .channels = Resources::ClipChannels, .freq = Resources::ClipSampleRate};
        mStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, OnAudioRequested, this);
        if (!mStream) {
            Logs::Logger::Warn("[SOUND] No audio device available, sounds are disabled: {}", SDL_GetError());
            return;
        }
        SDL_ResumeAudioStreamDevice(mStream);
    }

    EngineSoundManager::~EngineSoundManager() {
        if (mStream) {
            SDL_DestroyAudioStream(mStream);
            mStream = nullptr;
        }
    }

    SoundHandle EngineSoundManager::Play(const std::string_view clipId, const bool loop, const float volume) {
        return Start(clipId, loop, -1, volume);
    }

    SoundHandle EngineSoundManager::PlayFor(const std::string_view clipId, const float seconds, const bool loop, const float volume) {
        if (seconds <= 0.0f) return {};
        return Start(clipId, loop, static_cast<std::int64_t>(seconds * static_cast<float>(Resources::ClipSampleRate)), volume);
    }

    bool EngineSoundManager::Stop(const SoundHandle handle) {
        if (!Owns(handle)) return false;
        mSlots[handle.slot].active = false;
        return Enqueue(Command{.type = CommandType::Stop, .slot = handle.slot, .generation = handle.generation});
    }

    bool EngineSoundManager::Pause(const SoundHandle handle) {
        if (!Owns(handle) || mSlots[handle.slot].paused) return false;
        mSlots[handle.slot].paused = true;
        return Enqueue(Command{.type = CommandType::Pause, .slot = handle.slot, .generation = handle.generation});
    }

    bool EngineSoundManager::Resume(const SoundHandle handle) {
        if (!Owns(handle) || !mSlots[handle.slot].paused) return false;
        mSlots[handle.slot].paused = false;
        return Enqueue(Command{.type = CommandType::Resume, .slot = handle.slot, .generation = handle.generation});
    }

    void EngineSoundManager::StopAll() {
        for (Slot& slot : mSlots) slot.active = false;
        Enqueue(Command{.type = CommandType::StopAll});
    }

    bool EngineSoundManager::IsPlaying(const SoundHandle handle) const {
        return Owns(handle) && !mSlots[handle.slot].paused;
    }

    bool EngineSoundManager::IsAnyPlaying() const {
        for (std::size_t slot = 0; slot < MaxSounds; ++slot) {
            if (IsBusy(slot) && !mSlots[slot].paused) return true;
        }
        return false;
    }

    bool EngineSoundManager::SetVolume(const SoundHandle handle, const float volume) {
        if (!Owns(handle)) return false;
        return Enqueue(Command{.type = CommandType::SetVolume, .slot = handle.slot, .generation = handle.generation, .clip = nullptr,
                               .loop = false, .frameLimit = -1, .volume = std::max(volume, 0.0f)});
    }

    void EngineSoundManager::SetMasterVolume(const float volume) {
        mMasterVolume.store(std::clamp(volume, 0.0f, 1.0f), std::memory_order_relaxed);
    }

    void EngineSoundManager::CleanUp() {
        LockAudio();
        DrainCommands();
        for (std::size_t slot = 0; slot < MaxSounds; ++slot) Finish(slot);
        UnlockAudio();
        for (Slot& slot : mSlots) {
            slot.active = false;
            slot.paused = false;
        }
    }

    void EngineSoundManager::RenderMix(const std::span<float> interleavedStereo) {
        DrainCommands();
        std::ranges::fill(interleavedStereo, 0.0f);

        const std::size_t frames = interleavedStereo.size() / Resources::ClipChannels;
        for (std::size_t slot = 0; slot < MaxSounds; ++slot) {
            Voice& voice = mVoices[slot];
            if (!voice.active || voice.paused) continue;

            const std::span<const float> samples = voice.clip->GetSamples();
            const std::size_t clipFrames = voice.clip->GetFrameCount();
            for (std::size_t frame = 0; frame < frames && voice.active; ++frame) {
                if (voice.frame >= clipFrames) {
                    if (!voice.loop || clipFrames == 0) {
                        Finish(slot);
                        break;
                    }
                    voice.frame = 0;
                }
                if (voice.framesLeft == 0) {
                    Finish(slot);
                    break;
                }

                interleavedStereo[frame * 2] += samples[voice.frame * 2] * voice.volume;
                interleavedStereo[frame * 2 + 1] += samples[voice.frame * 2 + 1] * voice.volume;
                ++voice.frame;
                if (voice.framesLeft > 0) --voice.framesLeft;
            }
        }

        const float master = mMasterVolume.load(std::memory_order_relaxed);
        for (float& sample : interleavedStereo) sample = std::clamp(sample * master, -1.0f, 1.0f);
    }

    void EngineSoundManager::FeedStream(SDL_AudioStream* stream, const int requestedBytes) {
        constexpr int FrameBytes = Resources::ClipChannels * static_cast<int>(sizeof(float));
        int remainingFrames = requestedBytes / FrameBytes;

        while (remainingFrames > 0) {
            const auto frames = static_cast<std::size_t>(std::min(remainingFrames, static_cast<int>(ScratchFrames)));
            const std::span<float> block = std::span(mScratch).first(frames * Resources::ClipChannels);
            RenderMix(block);
            SDL_PutAudioStreamData(stream, block.data(), static_cast<int>(block.size_bytes()));
            remainingFrames -= static_cast<int>(frames);
        }
    }

    SoundHandle EngineSoundManager::Start(const std::string_view clipId, const bool loop, const std::int64_t frameLimit, const float volume) {
        if (mOutput == Synth::AudioOutput::Device && !mStream) return {};
        if (!mClips || !mClips->Contains(clipId)) {
            Logs::Logger::Warn(R"([SOUND] There is no sound named "{}".)", clipId);
            return {};
        }

        const std::size_t slot = ChooseSlot();
        Slot& state = mSlots[slot];
        state.generation = state.generation == UINT32_MAX ? 1 : state.generation + 1;
        state.startedAt = ++mClock;
        state.active = true;
        state.paused = false;

        const auto index = static_cast<std::uint32_t>(slot);
        const Command command{.type = CommandType::Play, .slot = index, .generation = state.generation, .clip = &mClips->Get(clipId),
                              .loop = loop, .frameLimit = frameLimit, .volume = std::max(volume, 0.0f)};
        if (!Enqueue(command)) {
            state.active = false;
            return {};
        }
        return SoundHandle{.slot = index, .generation = state.generation};
    }

    bool EngineSoundManager::IsBusy(const std::size_t slot) const {
        return mSlots[slot].active && mFinished[slot].load(std::memory_order_acquire) != mSlots[slot].generation;
    }

    bool EngineSoundManager::Owns(const SoundHandle handle) const {
        return handle.IsValid() && handle.slot < MaxSounds && mSlots[handle.slot].generation == handle.generation && IsBusy(handle.slot);
    }

    std::size_t EngineSoundManager::ChooseSlot() const {
        std::size_t oldest = 0;
        for (std::size_t slot = 0; slot < MaxSounds; ++slot) {
            if (!IsBusy(slot)) return slot;
            if (mSlots[slot].startedAt < mSlots[oldest].startedAt) oldest = slot;
        }
        return oldest;
    }

    bool EngineSoundManager::Enqueue(const Command& command) {
        return mCommands.Push(command);
    }

    void EngineSoundManager::Apply(const Command& command) {
        if (command.type == CommandType::StopAll) {
            for (std::size_t slot = 0; slot < MaxSounds; ++slot) Finish(slot);
            return;
        }

        Voice& voice = mVoices[command.slot];
        if (command.type == CommandType::Play) {
            voice = Voice{.clip = command.clip, .frame = 0, .framesLeft = command.frameLimit, .volume = command.volume,
                          .generation = command.generation, .loop = command.loop, .paused = false, .active = true};
            return;
        }
        if (!voice.active || voice.generation != command.generation) return;

        switch (command.type) {
            case CommandType::Stop: Finish(command.slot); break;
            case CommandType::Pause: voice.paused = true; break;
            case CommandType::Resume: voice.paused = false; break;
            case CommandType::SetVolume: voice.volume = command.volume; break;
            case CommandType::Play:
            case CommandType::StopAll: break;
        }
    }

    void EngineSoundManager::Finish(const std::size_t slot) {
        Voice& voice = mVoices[slot];
        if (voice.active) mFinished[slot].store(voice.generation, std::memory_order_release);
        voice.active = false;
    }

    void EngineSoundManager::DrainCommands() {
        Command command{};
        while (mCommands.Pop(command)) Apply(command);
    }

    void EngineSoundManager::LockAudio() const {
        if (mStream) SDL_LockAudioStream(mStream);
    }

    void EngineSoundManager::UnlockAudio() const {
        if (mStream) SDL_UnlockAudioStream(mStream);
    }
}
