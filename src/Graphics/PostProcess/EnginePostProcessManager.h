// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include "SpinoCore/Graphics/PostProcess.h"
#include "SpinoCore/Graphics/PostProcessManager.h"

namespace SpinoCore::Graphics::Shaders {
    class Shader;
}

namespace SpinoCore::Graphics {
    class EnginePostProcessManager final : public PostProcessManager {
    public:
        EnginePostProcessManager();
        ~EnginePostProcessManager() override;

        EnginePostProcessManager(const EnginePostProcessManager&) = delete;
        EnginePostProcessManager& operator=(const EnginePostProcessManager&) = delete;
        EnginePostProcessManager(EnginePostProcessManager&&) = delete;
        EnginePostProcessManager& operator=(EnginePostProcessManager&&) = delete;

        bool AddPostProcess(std::string name, PostProcess postProcess) override;
        bool SetCurrent(std::string_view name) override;
        void ToggleActive() override;
        bool SetUniform(std::string_view name, std::size_t index, const UniformValue& value) override;
        void CleanUp() override;

        [[nodiscard]] bool IsActive() const override;
        [[nodiscard]] const std::string& GetCurrent() const override { return mCurrent; }
        [[nodiscard]] bool Contains(std::string_view name) const override;

        [[nodiscard]] const Shaders::Shader& PrepareCurrentShader();
        [[nodiscard]] std::span<const PostProcessUniform> GetCurrentUniforms() const;

    private:
        struct Entry {
            PostProcess postProcess;
            std::unique_ptr<Shaders::Shader> shader{};
            bool failed{false};
        };

        void AddDefaults();
        [[nodiscard]] const Shaders::Shader& Passthrough();

        std::map<std::string, Entry, std::less<>> mEntries;
        std::string mCurrent{};
        bool mActive{false};
        std::unique_ptr<Shaders::Shader> mPassthrough{};
    };
}
