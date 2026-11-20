// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "EngineOperations.h"
#include "Core/Events/Handler.h"
#include <memory>
#include "Core/UI/Elements/Rect.h"

namespace SpinoCore::Core::Engine {
    EngineOperations::EngineOperations(EngineModules& modules, Events::EventBus& bus)
    :mModules(modules)
    ,mEventBus(bus)
    {
        // Test element
        auto rect = std::make_unique<UI::Elements::Rect>(UI::Elements::ElementInput{.size = {50.0f, 50.0f}});
        rect->SetBackgroundColor({1.0f, 0.0f, 0.0f, 1.0f});
        rect->SetOpacity(1.5f);
        mUIElements.push_back(std::move(rect));
    }
    EngineOperations::~EngineOperations() = default;

    bool EngineOperations::ProcessEvents() {
        return Events::Handler::PollEvents(mEventBus);
    }

    bool EngineOperations::PrepareSimulation() {
        return true;
    }

    void EngineOperations::ProcessInputs() {
    }

    void EngineOperations::UpdateSimulation(const float deltaTime) {
        mModules.camerasManager->Update(deltaTime);
    }

    void EngineOperations::Interpolate(float /*alpha*/) {

    }

    void EngineOperations::Render() {
        auto& drawList = mModules.renderer->GetUIDrawList();
        for (const auto& element : mUIElements) element->GetDrawCommand(drawList);
        mModules.renderer->Draw();
    }

    Runtime::FrameOperations::Duration EngineOperations::GetFramePeriod() {
        return  mModules.renderer->GetFramePeriod();
    }
}

