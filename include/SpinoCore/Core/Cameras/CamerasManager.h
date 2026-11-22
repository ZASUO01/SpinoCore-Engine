// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include <string>

namespace SpinoCore::Core::Cameras {
    class Camera;

    /// @brief Keeps the scene cameras by name and selects the one used to draw.
    ///
    /// A default camera always exists and is used when no other is current.
    class CamerasManager {
    public:
        virtual ~CamerasManager() = default;

        /// @brief Adds a camera; a name already in use is ignored.
        /// @param camera Camera to own.
        /// @param name Unique name.
        virtual void AddCamera(std::unique_ptr<Camera> camera, const std::string &name) = 0;
        /// @param name Name of the camera.
        /// @return The camera, or nullptr when the name is unknown.
        [[nodiscard]] virtual Camera* GetCamera(const std::string &name) = 0;
        /// @brief Selects the camera used to draw.
        /// @param name Name of a camera that was added.
        virtual void SetCurrentCamera(const std::string& name) = 0;
        /// @brief Removes a camera; removing the current one selects the default camera.
        /// @param name Name of the camera; the default camera cannot be removed.
        virtual void RemoveCamera(const std::string& name) = 0;
        /// @brief Removes every camera except the default one, which becomes current.
        virtual void CleanUp() = 0;
    };
}
