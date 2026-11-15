// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineCamerasManager.h"

#include <ranges>

#include "DefaultCamera.h"
#include "Core/Events/TypeEvents/WindowEvents.h"

namespace SpinoCore::Core::Cameras {
    EngineCamerasManager::EngineCamerasManager(const Utils::Size2D windowSize, Rendering::Renderer &renderer, Events::EventBus& bus)
    :mRenderer(renderer)
    ,mViewportSize(windowSize)
    {
        auto defaultCamera = std::make_unique<DefaultCamera>(static_cast<float>(windowSize.width), static_cast<float>(windowSize.height));

        mCameras.emplace("default", std::move(defaultCamera));
        mCurrentCameraName = "default";
        mCurrentCamera = mCameras["default"].get();
        UpdateRenderer();

        mResizeSubscription = bus.Listen<Events::WindowResizeEvent, &EngineCamerasManager::OnResize>(this);
    }

    EngineCamerasManager::~EngineCamerasManager() = default;

    void EngineCamerasManager::AddCamera(std::unique_ptr<Camera> camera, const std::string& name) {
        if (const auto it = mCameras.find(name); it == mCameras.end()) {
            camera->SetViewportSize(static_cast<float>(mViewportSize.width), static_cast<float>(mViewportSize.height));
            mCameras.emplace(name, std::move(camera));
        }
    }

    void EngineCamerasManager::SetCurrentCamera(const std::string& name) {
        if (const auto it = mCameras.find(name); it != mCameras.end()) {
            mCurrentCamera = it->second.get();
            mCurrentCameraName = name;
            UpdateRenderer();
        }
    }

    void EngineCamerasManager::RemoveCamera(const std::string& name) {
        if (name == "default") return;
        
        if (const auto it = mCameras.find(name); it != mCameras.end()) {
            if (mCurrentCameraName == name) {
                mCurrentCamera = mCameras["default"].get();
                mCurrentCameraName = "default";
                UpdateRenderer();
            }
            mCameras.erase(it);
        }
    }

    Camera *EngineCamerasManager::GetCamera(const std::string& name) {
        if (const auto it = mCameras.find(name); it != mCameras.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    void EngineCamerasManager::Update(const float deltaTime) const {
        if (mCurrentCamera) {
            mCurrentCamera->Update(deltaTime);
            UpdateRenderer();
        }
    }

    void EngineCamerasManager::UpdateRenderer() const {
        mRenderer.SetCamera(mCurrentCamera->GetViewMatrix(), mCurrentCamera->GetProjectionMatrix());
    }

    void EngineCamerasManager::OnResize(Events::WindowResizeEvent& event) {
        const Utils::Size2D size = event.size;
        if (size.width <= 0 || size.height <= 0) return;

        mViewportSize = size;
        for (const auto &camera: mCameras | std::views::values) {
            camera->SetViewportSize(static_cast<float>(size.width), static_cast<float>(size.height));
        }

        if (mCurrentCamera) UpdateRenderer();
    }
}
