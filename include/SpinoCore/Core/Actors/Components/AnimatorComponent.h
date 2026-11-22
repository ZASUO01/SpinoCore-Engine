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
    class AnimatorComponent final : public Component {
    public:
        explicit AnimatorComponent(Actor& actor, const ComponentInput& input = {});

        void Update(float deltaTime) override;
        void GetDrawCommand(Rendering::ModelDrawList& drawList) override;

        void Play(std::string clipName, bool loop = true);
        void Stop();

        [[nodiscard]] bool IsPlaying() const { return mIsPlaying; }
        [[nodiscard]] bool IsLooping() const { return mIsLooping; }
        [[nodiscard]] const std::string& GetCurrentClip() const { return mClipName; }
        [[nodiscard]] float GetTime() const { return mTime; }
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
