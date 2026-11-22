// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include "PostProcess.h"

namespace SpinoCore::Graphics {
    class PostProcessManager {
    public:
        virtual ~PostProcessManager() = default;

        virtual bool AddPostProcess(std::string name, PostProcess postProcess) = 0;
        virtual bool SetCurrent(std::string_view name) = 0;
        virtual void ToggleActive() = 0;
        virtual bool SetUniform(std::string_view name, std::size_t index, const UniformValue& value) = 0;
        virtual void CleanUp() = 0;

        [[nodiscard]] virtual bool IsActive() const = 0;
        [[nodiscard]] virtual const std::string& GetCurrent() const = 0;
        [[nodiscard]] virtual bool Contains(std::string_view name) const = 0;
    };
}
