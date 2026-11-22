// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <string>
#include "Component.h"

namespace SpinoCore::Graphics::Resources {
    class Model;
}

namespace SpinoCore::Core::Actors {
    class Actor;
}

namespace SpinoCore::Core::Rendering {
    struct ModelDrawList;
}

namespace SpinoCore::Core::Actors::Components {
    class ModelComponent final : public Component {
    public:
        ModelComponent(Actor& actor, std::string modelId, const ComponentInput& input = {});

        void Update(float deltaTime) override;
        void GetDrawCommand(Rendering::ModelDrawList& drawList) override;

        [[nodiscard]] const std::string& GetModelId() const { return mModelId; }
        void SetModelId(std::string modelId);

        [[nodiscard]] const Graphics::Resources::Model* GetModel() const { return mModel; }

    private:
        void ResolveModel(const Rendering::ModelDrawList& drawList);

        std::string mModelId;
        const Graphics::Resources::Model* mModel{nullptr};
        const void* mCachedFrom{nullptr};
        std::uint64_t mCachedVersion{0};
    };
}
