// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>

namespace SpinoCore::Sound::Synth {
    class Timbre;
    enum class SynthMode : std::uint8_t;

    class SynthManager {
    public:
        virtual ~SynthManager() = default;

        virtual bool Register(std::string name, std::unique_ptr<Timbre> timbre, SynthMode mode) = 0;
        virtual bool Unregister(std::string_view name) = 0;

        virtual bool NoteOn(std::string_view name, int midiNote, float velocity = 1.0f, float duration = 0.0f) = 0;
        virtual bool NoteOff(std::string_view name, int midiNote) = 0;
        virtual bool PlayChord(std::string_view name, std::span<const int> midiNotes, float velocity = 1.0f, float duration = 0.0f) = 0;
        virtual bool Release(std::string_view name) = 0;
        virtual void StopAll() = 0;

        virtual bool SetVolume(std::string_view name, float volume) = 0;
        virtual void SetMasterVolume(float volume) = 0;
        [[nodiscard]] virtual float GetMasterVolume() const = 0;

        [[nodiscard]] virtual bool Contains(std::string_view name) const = 0;
        virtual void CleanUp() = 0;
    };
}
