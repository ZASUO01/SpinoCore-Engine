// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <string_view>

/// @brief Audio: sound clips and the synthesizer.
namespace SpinoCore::Sound {
    /// @brief Identifies one playing sound; it stops being valid when the sound ends or its slot is reused.
    struct SoundHandle {
        /// Playback slot.
        std::uint32_t slot{0};
        /// Use of the slot this handle refers to; 0 means no sound.
        std::uint32_t generation{0};

        /// @return Whether the handle came from a sound that started.
        [[nodiscard]] constexpr bool IsValid() const noexcept { return generation != 0; }
    };

    /// @brief Plays clips from the sound cache on up to 16 slots.
    ///
    /// When every slot is busy, a new sound replaces the oldest one. While sounds play,
    /// the synthesizer volume is lowered.
    class SoundManager {
    public:
        virtual ~SoundManager() = default;

        /// @brief Starts a clip.
        /// @param clipId Name of the clip in the sound cache.
        /// @param loop Whether the clip restarts at the end.
        /// @param volume Volume of this sound; negative values become 0.
        /// @return The handle of the sound, or an invalid handle when the clip is unknown.
        virtual SoundHandle Play(std::string_view clipId, bool loop = false, float volume = 1.0f) = 0;
        /// @brief Starts a clip that stops after a given time.
        /// @param clipId Name of the clip in the sound cache.
        /// @param seconds Play time; values of 0 or less play nothing.
        /// @param loop Whether the clip restarts until the time ends.
        /// @param volume Volume of this sound.
        /// @return The handle of the sound, or an invalid handle.
        virtual SoundHandle PlayFor(std::string_view clipId, float seconds, bool loop = true, float volume = 1.0f) = 0;

        /// @brief Stops a sound.
        /// @param handle Sound to stop.
        /// @return Whether the handle referred to a sound of this manager.
        virtual bool Stop(SoundHandle handle) = 0;
        /// @brief Pauses a sound.
        /// @param handle Sound to pause.
        /// @return Whether the handle referred to a playing sound.
        virtual bool Pause(SoundHandle handle) = 0;
        /// @brief Resumes a paused sound.
        /// @param handle Sound to resume.
        /// @return Whether the handle referred to a paused sound.
        virtual bool Resume(SoundHandle handle) = 0;
        /// @brief Stops every sound.
        virtual void StopAll() = 0;

        /// @param handle Sound to check.
        /// @return Whether the sound is still playing.
        [[nodiscard]] virtual bool IsPlaying(SoundHandle handle) const = 0;
        /// @return Whether any sound is playing.
        [[nodiscard]] virtual bool IsAnyPlaying() const = 0;

        /// @brief Changes the volume of a sound.
        /// @param handle Sound to change.
        /// @param volume New volume; negative values become 0.
        /// @return Whether the handle referred to a sound of this manager.
        virtual bool SetVolume(SoundHandle handle, float volume) = 0;
        /// @brief Changes the volume of every sound.
        /// @param volume Value clamped to 0..1.
        virtual void SetMasterVolume(float volume) = 0;
        /// @return The master volume.
        [[nodiscard]] virtual float GetMasterVolume() const = 0;

        /// @brief Stops every sound at once; the engine calls it between scenes.
        virtual void CleanUp() = 0;
    };
}
