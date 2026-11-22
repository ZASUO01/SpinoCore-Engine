// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <span>
#include <string>
#include <vector>
#include "Component.h"
#include "SpinoCore/Math/Matrix.h"

namespace SpinoCore::Core::Actors {
    class Actor;
}

namespace SpinoCore::Core::Rendering {
    struct ModelDrawList;
}

namespace SpinoCore::Core::Actors::Components {
    /// @brief Plays the skeletal animation clips of the ModelComponent of the same actor.
    class AnimatorComponent final : public Component {
    public:
        /// @brief Creates an animator with no clip playing.
        /// @param actor Owner; it needs a ModelComponent with a skinned model.
        /// @param input Initial settings.
        explicit AnimatorComponent(Actor& actor, const ComponentInput& input = {});

        /// @brief Advances the clip and computes the bone matrices.
        /// @param deltaTime Simulation step in seconds.
        void Update(float deltaTime) override;
        /// @brief Draws nothing; the ModelComponent reads the bone matrices.
        void GetDrawCommand(Rendering::ModelDrawList& drawList) override;

        /// @brief Starts a clip from the beginning.
        /// @param clipName Name of a clip of the model.
        /// @param loop Whether the clip restarts at the end.
        void Play(std::string clipName, bool loop = true);
        /// @brief Freezes the current pose.
        void Stop();

        /// @return Whether the clip time is advancing.
        [[nodiscard]] bool IsPlaying() const { return mIsPlaying; }
        /// @return Whether the clip restarts at the end.
        [[nodiscard]] bool IsLooping() const { return mIsLooping; }
        /// @return The name of the current clip, or an empty string.
        [[nodiscard]] const std::string& GetCurrentClip() const { return mClipName; }
        /// @return The time inside the current clip, in seconds.
        [[nodiscard]] float GetTime() const { return mTime; }
        /// @return The skinning matrix of each bone for the current pose.
        [[nodiscard]] std::span<const Math::Matrix4f> GetBoneMatrices() const { return mBoneMatrices; }

    private:
        void Advance(float deltaTime, float duration);

        std::string mClipName{};
        float mTime{0.0f};
        bool mIsLooping{true};
        bool mIsPlaying{false};
        std::vector<Math::Matrix4f> mBoneMatrices{};
    };
}
