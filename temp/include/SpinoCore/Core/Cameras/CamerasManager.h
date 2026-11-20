// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include <string>
#include "Camera.h"

namespace SpinoCore::Core::Cameras {
    class CamerasManager {
    public:
        virtual ~CamerasManager() = default;

        virtual void AddCamera(std::unique_ptr<Camera> camera, const std::string &name) = 0;
        [[nodiscard]] virtual Camera* GetCamera(const std::string &name) = 0;
        virtual void SetCurrentCamera(const std::string& name) = 0;
        virtual void RemoveCamera(const std::string& name) = 0;
    };
}
