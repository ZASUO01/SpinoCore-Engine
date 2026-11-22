// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EnginePostProcessManager.h"
#include <algorithm>
#include <exception>
#include <utility>
#include "Graphics/Shaders/Shader.h"
#include "Graphics/Shaders/Sources/PostProcessShader.h"
#include "SpinoCore/Logs/Logger.h"

namespace SpinoCore::Graphics {
    namespace {
        [[nodiscard]] PostProcessUniform FloatUniform(const float value) {
            return PostProcessUniform{.type = UniformType::Float, .value = value};
        }

        [[nodiscard]] std::string WithHeader(const std::string& fragmentShader) {
            return std::string(Shaders::Sources::PostProcessFragmentHeaderSource) + fragmentShader;
        }
    }

    EnginePostProcessManager::EnginePostProcessManager() {
        AddDefaults();
    }

    EnginePostProcessManager::~EnginePostProcessManager() = default;

    bool EnginePostProcessManager::AddPostProcess(std::string name, PostProcess postProcess) {
        if (name.empty() || mEntries.contains(name)) {
            Logs::Logger::Warn(R"([POST PROCESS] "{}" was not added: the name is empty or already used.)", name);
            return false;
        }
        if (postProcess.uniforms.size() > static_cast<std::size_t>(PostProcessLocation::MaxUserUniforms)) {
            Logs::Logger::Warn(R"([POST PROCESS] "{}" was not added: it has more than {} uniforms.)", name, PostProcessLocation::MaxUserUniforms);
            return false;
        }
        if (!std::ranges::all_of(postProcess.uniforms, Matches)) {
            Logs::Logger::Warn(R"([POST PROCESS] "{}" was not added: a uniform value does not match its type.)", name);
            return false;
        }

        mEntries.emplace(std::move(name), Entry{.postProcess = std::move(postProcess)});
        return true;
    }

    bool EnginePostProcessManager::SetCurrent(const std::string_view name) {
        const auto it = mEntries.find(name);
        if (it == mEntries.end()) {
            Logs::Logger::Warn(R"([POST PROCESS] There is no post process named "{}".)", name);
            return false;
        }
        mCurrent = it->first;
        return true;
    }

    void EnginePostProcessManager::ToggleActive() {
        mActive = !mActive;
    }

    bool EnginePostProcessManager::SetUniform(const std::string_view name, const std::size_t index, const UniformValue& value) {
        const auto it = mEntries.find(name);
        if (it == mEntries.end()) return false;

        auto& uniforms = it->second.postProcess.uniforms;
        if (index >= uniforms.size() || static_cast<std::size_t>(uniforms[index].type) != value.index()) return false;

        uniforms[index].value = value;
        return true;
    }

    void EnginePostProcessManager::CleanUp() {
        mEntries.clear();
        mCurrent.clear();
        mActive = false;
        AddDefaults();
    }

    bool EnginePostProcessManager::IsActive() const {
        return mActive && !mCurrent.empty();
    }

    bool EnginePostProcessManager::Contains(const std::string_view name) const {
        return mEntries.contains(name);
    }

    const Shaders::Shader& EnginePostProcessManager::PrepareCurrentShader() {
        const auto it = mEntries.find(mCurrent);
        if (it == mEntries.end()) return Passthrough();

        Entry& entry = it->second;
        if (!entry.shader && !entry.failed) {
            try {
                entry.shader = std::make_unique<Shaders::Shader>(Shaders::Sources::PostProcessVertexShaderSource, WithHeader(entry.postProcess.fragmentShader));
            } catch (const std::exception& e) {
                Logs::Logger::Error(R"([POST PROCESS] "{}" failed to build and was replaced by a passthrough: {})", mCurrent, e.what());
                entry.failed = true;
            }
        }
        return entry.shader ? *entry.shader : Passthrough();
    }

    std::span<const PostProcessUniform> EnginePostProcessManager::GetCurrentUniforms() const {
        const auto it = mEntries.find(mCurrent);
        if (it == mEntries.end() || !it->second.shader) return {};
        return it->second.postProcess.uniforms;
    }

    void EnginePostProcessManager::AddDefaults() {
        using namespace Shaders::Sources;
        AddPostProcess(DefaultPostProcess::Grayscale, PostProcess{.fragmentShader = GrayscaleFragmentSource, .uniforms = {FloatUniform(1.0f)}});
        AddPostProcess(DefaultPostProcess::Vignette, PostProcess{.fragmentShader = VignetteFragmentSource, .uniforms = {FloatUniform(0.8f), FloatUniform(0.5f)}});
        AddPostProcess(DefaultPostProcess::Exposure, PostProcess{.fragmentShader = ExposureFragmentSource, .uniforms = {FloatUniform(1.0f), FloatUniform(1.0f)}});
    }

    const Shaders::Shader& EnginePostProcessManager::Passthrough() {
        if (!mPassthrough) {
            mPassthrough = std::make_unique<Shaders::Shader>(Shaders::Sources::PostProcessVertexShaderSource, WithHeader(Shaders::Sources::PassthroughFragmentSource));
        }
        return *mPassthrough;
    }
}
