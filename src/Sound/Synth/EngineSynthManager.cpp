// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineSynthManager.h"
#include <algorithm>
#include <utility>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_error.h>
#include "SpinoCore/Logs/Logger.h"
#include "SpinoCore/Sound/Synth/Timbre.h"

namespace SpinoCore::Sound::Synth {
    namespace {
        constexpr int Channels = 2;
        constexpr std::size_t InitialSynthCapacity = 16;

        void SDLCALL OnAudioRequested(void* userdata, SDL_AudioStream* stream, const int additionalAmount, int) {
            static_cast<EngineSynthManager*>(userdata)->FeedStream(stream, additionalAmount);
        }
    }

    EngineSynthManager::EngineSynthManager(const AudioOutput output)
    :mOutput(output)
    {
        mSynths.reserve(InitialSynthCapacity);
        if (mOutput == AudioOutput::Offline) return;

        SDL_AudioSpec spec{.format = SDL_AUDIO_F32, .channels = Channels, .freq = OfflineSampleRate};
        if (SDL_AudioSpec deviceSpec{}; SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &deviceSpec, nullptr) && deviceSpec.freq > 0) {
            spec.freq = deviceSpec.freq;
        }

        mStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, OnAudioRequested, this);
        if (!mStream) {
            Logs::Logger::Warn("[SYNTH] No audio device available, running silently: {}", SDL_GetError());
            return;
        }

        mSampleRate = spec.freq;
        SDL_ResumeAudioStreamDevice(mStream);
    }

    EngineSynthManager::~EngineSynthManager() {
        if (mStream) {
            SDL_DestroyAudioStream(mStream);
            mStream = nullptr;
        }
        mSynths.clear();
    }

    bool EngineSynthManager::Register(std::string name, std::unique_ptr<Timbre> timbre, const SynthMode mode) {
        if (name.empty() || !timbre || FindSynth(name)) {
            Logs::Logger::Warn(R"([SYNTH] "{}" was not registered: the name is empty or used, or there is no timbre.)", name);
            return false;
        }

        auto synth = std::make_unique<Synthesizer>(std::move(timbre), mode, mSampleRate);
        LockAudio();
        mSynths.push_back(Entry{.name = std::move(name), .synth = std::move(synth)});
        UnlockAudio();
        return true;
    }

    bool EngineSynthManager::Unregister(const std::string_view name) {
        std::unique_ptr<Synthesizer> removed;

        LockAudio();
        DrainCommands();
        if (const auto it = std::ranges::find(mSynths, name, &Entry::name); it != mSynths.end()) {
            removed = std::move(it->synth);
            mSynths.erase(it);
        }
        UnlockAudio();

        return removed != nullptr;
    }

    bool EngineSynthManager::NoteOn(const std::string_view name, const int midiNote, const float velocity, const float duration) {
        Synthesizer* synth = FindSynth(name);
        if (!synth || midiNote < MinMidiNote || midiNote > MaxMidiNote) return false;

        Command command{.type = CommandType::NoteOn, .synth = synth, .notes = {}, .noteCount = 1, .velocity = velocity, .duration = duration};
        command.notes[0] = midiNote;
        return Enqueue(command);
    }

    bool EngineSynthManager::NoteOff(const std::string_view name, const int midiNote) {
        Synthesizer* synth = FindSynth(name);
        if (!synth) return false;

        Command command{.type = CommandType::NoteOff, .synth = synth, .notes = {}, .noteCount = 1};
        command.notes[0] = midiNote;
        return Enqueue(command);
    }

    bool EngineSynthManager::PlayChord(const std::string_view name, const std::span<const int> midiNotes, const float velocity, const float duration) {
        Synthesizer* synth = FindSynth(name);
        if (!synth || midiNotes.empty() || midiNotes.size() > synth->GetVoiceCapacity()) return false;
        if (std::ranges::any_of(midiNotes, [](const int note) { return note < MinMidiNote || note > MaxMidiNote; })) return false;

        Command command{.type = CommandType::Chord, .synth = synth, .notes = {}, .noteCount = static_cast<std::uint8_t>(midiNotes.size()),
                        .velocity = velocity, .duration = duration};
        std::ranges::copy(midiNotes, command.notes.begin());
        return Enqueue(command);
    }

    bool EngineSynthManager::Release(const std::string_view name) {
        Synthesizer* synth = FindSynth(name);
        return synth && Enqueue(Command{.type = CommandType::Release, .synth = synth});
    }

    void EngineSynthManager::StopAll() {
        for (const Entry& entry : mSynths) Enqueue(Command{.type = CommandType::StopAll, .synth = entry.synth.get()});
    }

    bool EngineSynthManager::SetVolume(const std::string_view name, const float volume) {
        Synthesizer* synth = FindSynth(name);
        return synth && Enqueue(Command{.type = CommandType::SetVolume, .synth = synth, .notes = {}, .noteCount = 0, .velocity = volume});
    }

    void EngineSynthManager::SetMasterVolume(const float volume) {
        mMasterVolume.store(std::clamp(volume, 0.0f, 1.0f), std::memory_order_relaxed);
    }

    bool EngineSynthManager::Contains(const std::string_view name) const {
        return FindSynth(name) != nullptr;
    }

    void EngineSynthManager::CleanUp() {
        std::vector<Entry> removed;

        LockAudio();
        DrainCommands();
        removed.swap(mSynths);
        mSynths.reserve(InitialSynthCapacity);
        UnlockAudio();
    }

    void EngineSynthManager::RenderMix(const std::span<float> interleavedStereo) {
        DrainCommands();
        std::ranges::fill(interleavedStereo, 0.0f);
        for (const Entry& entry : mSynths) entry.synth->Render(interleavedStereo);

        const float master = mMasterVolume.load(std::memory_order_relaxed);
        for (float& sample : interleavedStereo) sample = std::clamp(sample * master, -1.0f, 1.0f);
    }

    void EngineSynthManager::FeedStream(SDL_AudioStream* stream, const int requestedBytes) {
        constexpr int FrameBytes = Channels * static_cast<int>(sizeof(float));
        int remainingFrames = requestedBytes / FrameBytes;

        while (remainingFrames > 0) {
            const auto frames = static_cast<std::size_t>(std::min(remainingFrames, static_cast<int>(ScratchFrames)));
            const std::span<float> block = std::span(mScratch).first(frames * Channels);
            RenderMix(block);
            SDL_PutAudioStreamData(stream, block.data(), static_cast<int>(block.size_bytes()));
            remainingFrames -= static_cast<int>(frames);
        }
    }

    const Synthesizer* EngineSynthManager::Find(const std::string_view name) const {
        return FindSynth(name);
    }

    Synthesizer* EngineSynthManager::FindSynth(const std::string_view name) const {
        const auto it = std::ranges::find(mSynths, name, &Entry::name);
        return it != mSynths.end() ? it->synth.get() : nullptr;
    }

    bool EngineSynthManager::Enqueue(const Command& command) {
        if (mOutput == AudioOutput::Device && !mStream) {
            Apply(command);
            return true;
        }
        return mCommands.Push(command);
    }

    void EngineSynthManager::Apply(const Command& command) {
        Synthesizer& synth = *command.synth;
        switch (command.type) {
            case CommandType::NoteOn:
                synth.NoteOn(command.notes[0], command.velocity, command.duration);
                break;
            case CommandType::NoteOff:
                synth.NoteOff(command.notes[0]);
                break;
            case CommandType::Chord:
                synth.PlayChord(std::span(command.notes).first(command.noteCount), command.velocity, command.duration);
                break;
            case CommandType::Release:
                synth.ReleaseAll();
                break;
            case CommandType::StopAll:
                synth.StopAll();
                break;
            case CommandType::SetVolume:
                synth.SetVolume(command.velocity);
                break;
        }
    }

    void EngineSynthManager::DrainCommands() {
        Command command{};
        while (mCommands.Pop(command)) Apply(command);
    }

    void EngineSynthManager::LockAudio() const {
        if (mStream) SDL_LockAudioStream(mStream);
    }

    void EngineSynthManager::UnlockAudio() const {
        if (mStream) SDL_UnlockAudioStream(mStream);
    }
}
