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
    /// @brief Draws a model from the model cache with the actor transform.
    class ModelComponent final : public Component {
    public:
        /// @brief Creates a model component.
        /// @param actor Owner of the component.
        /// @param modelId Name of the model in the cache; an unknown name draws the default model.
        /// @param input Initial settings.
        ModelComponent(Actor& actor, std::string modelId, const ComponentInput& input = {});

        /// @brief Does nothing; the model is resolved while drawing.
        void Update(float deltaTime) override;
        /// @brief Adds the model, its materials and the animator pose to the draw list.
        /// @param drawList List that receives the command.
        void GetDrawCommand(Rendering::ModelDrawList& drawList) override;

        /// @return The name of the model in the cache.
        [[nodiscard]] const std::string& GetModelId() const { return mModelId; }
        /// @brief Changes the model.
        /// @param modelId Name of the model in the cache.
        void SetModelId(std::string modelId);

        /// @return The model resolved on the last draw, or nullptr before the first draw.
        [[nodiscard]] const Graphics::Resources::Model* GetModel() const { return mModel; }

    private:
        void ResolveModel(const Rendering::ModelDrawList& drawList);

        std::string mModelId;
        const Graphics::Resources::Model* mModel{nullptr};
        const void* mCachedFrom{nullptr};
        std::uint64_t mCachedVersion{0};
    };
}
