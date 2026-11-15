// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <unordered_map>

#include "Core/Events/EventBus.h"
#include "Core/Rendering/Renderer.h"
#include "SpinoCore/Core/Cameras/CamerasManager.h"

namespace SpinoCore::Core::Cameras {
    class EngineCamerasManager :public CamerasManager{
    public:
        explicit EngineCamerasManager(Utils::Size2D windowSize, Rendering::Renderer& renderer, Events::EventBus& bus);
        ~EngineCamerasManager() override;

        EngineCamerasManager(const EngineCamerasManager&) = delete;
        EngineCamerasManager& operator=(const EngineCamerasManager&) = delete;
        EngineCamerasManager(EngineCamerasManager&&) = delete;
        EngineCamerasManager& operator=(EngineCamerasManager&&) = delete;

        void AddCamera(std::unique_ptr<Camera> camera, const std::string &name) override;
        void SetCurrentCamera(const std::string &name) override;
        [[nodiscard]] Camera *GetCamera(const std::string &name) override;
        void RemoveCamera(const std::string &name) override;

        void Update(float deltaTime) const;

    private:
        void UpdateRenderer() const;
        void Resize(Utils::Size2D size);

        Rendering::Renderer& mRenderer;
        Camera *mCurrentCamera{nullptr};
        std::string mCurrentCameraName;
        std::unordered_map<std::string, std::unique_ptr<Camera>> mCameras;
        Utils::Size2D mViewportSize{};

        Events::EventBus::Subscription mResizeSubscription;
    };
}