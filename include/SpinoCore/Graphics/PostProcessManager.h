// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include "PostProcess.h"

namespace SpinoCore::Graphics {
    /// @brief Keeps post-processes by name; at most one is applied at a time.
    class PostProcessManager {
    public:
        virtual ~PostProcessManager() = default;

        /// @brief Adds a post-process; its shader is compiled when it is first used.
        /// @param name Unique name.
        /// @param postProcess Shader and uniforms.
        /// @return false when the name is empty or used, there are too many uniforms or a value does not match its type.
        virtual bool AddPostProcess(std::string name, PostProcess postProcess) = 0;
        /// @brief Selects the post-process to apply.
        /// @param name Name of a post-process.
        /// @return Whether the name exists.
        virtual bool SetCurrent(std::string_view name) = 0;
        /// @brief Turns the current post-process on or off.
        virtual void ToggleActive() = 0;
        /// @brief Changes a user uniform.
        /// @param name Name of the post-process.
        /// @param index Index of the uniform.
        /// @param value New value with the declared type.
        /// @return Whether the uniform was changed.
        virtual bool SetUniform(std::string_view name, std::size_t index, const UniformValue& value) = 0;
        /// @brief Removes the added post-processes, keeps the default ones and turns post-processing off.
        virtual void CleanUp() = 0;

        /// @return Whether the current post-process is applied.
        [[nodiscard]] virtual bool IsActive() const = 0;
        /// @return The name of the current post-process, or an empty string.
        [[nodiscard]] virtual const std::string& GetCurrent() const = 0;
        /// @param name Name of a post-process.
        /// @return Whether it exists.
        [[nodiscard]] virtual bool Contains(std::string_view name) const = 0;
    };
}
