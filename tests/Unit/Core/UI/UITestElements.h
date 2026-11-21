// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "SpinoCore/Core/UI/Elements/Element.h"
#include "SpinoCore/Core/UI/Screen.h"

namespace SpinoCore::Tests {
    // Emits one command per draw, keyed by layer and draw order, and remembers its own name so
    // the tests can see which elements were drawn.
    class TestElement final : public Core::UI::Elements::Element {
    public:
        explicit TestElement(const Core::UI::Elements::ElementInput& input = {}, std::string name = {},
                             std::vector<std::string>* drawn = nullptr)
            : Element(input), mName(std::move(name)), mDrawn(drawn) {}

        [[nodiscard]] static std::uint64_t KeyFor(const int layer, const int drawOrder) {
            return Core::Rendering::MakeUIDrawKey(layer, drawOrder, 0);
        }

    protected:
        void SetDrawCommand(Core::Rendering::UIDrawList& drawList, const int drawLayer) override {
            drawList.commands.push_back({.key = KeyFor(drawLayer, GetDrawOrder())});
            if (mDrawn) mDrawn->push_back(mName);
        }

    private:
        std::string mName;
        std::vector<std::string>* mDrawn;
    };

    // Records the lifecycle callbacks the manager sends, prefixed with the screen's name.
    class TestScreen final : public Core::UI::Screen {
    public:
        TestScreen(std::string name, std::vector<std::string>& log) : mName(std::move(name)), mLog(log) {}
        ~TestScreen() override { mLog.push_back(mName + ":destroyed"); }

        void OnEnter() override { mLog.push_back(mName + ":enter"); }
        void OnPause() override { mLog.push_back(mName + ":pause"); }
        void OnResume() override { mLog.push_back(mName + ":resume"); }
        void OnExit() override { mLog.push_back(mName + ":exit"); }

    private:
        std::string mName;
        std::vector<std::string>& mLog;
    };
}
