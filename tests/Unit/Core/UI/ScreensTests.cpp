// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <gtest/gtest.h>
#include "UITestElements.h"
#include "Core/Rendering/Drawer/DrawCommands.h"
#include "Core/UI/EngineScreensManager.h"
#include "SpinoCore/Core/UI/Elements/Element.h"

namespace {
    using SpinoCore::Core::Rendering::UIDrawList;
    using SpinoCore::Core::UI::EngineScreensManager;
    using SpinoCore::Core::UI::Elements::Element;
    using SpinoCore::Core::UI::Elements::ElementInput;
    using SpinoCore::Tests::TestElement;
    using SpinoCore::Tests::TestScreen;

    using Log = std::vector<std::string>;

    [[nodiscard]] std::vector<std::uint64_t> Keys(const UIDrawList& drawList) {
        std::vector<std::uint64_t> keys;
        for (const auto& command : drawList.commands) keys.push_back(command.key);
        return keys;
    }

    TEST(ScreenTests, AddElementTakesOwnershipAndReturnsTheElement) {
        Log log;
        TestScreen screen{"menu", log};
        auto element = std::make_unique<TestElement>();
        const Element* address = element.get();

        Element& added = screen.AddElement(std::move(element));

        EXPECT_EQ(&added, address);
        ASSERT_EQ(screen.GetElements().size(), 1u);
        EXPECT_EQ(screen.GetElements()[0].get(), address);
    }

    TEST(ScreenTests, AddingANullElementThrows) {
        Log log;
        TestScreen screen{"menu", log};

        EXPECT_THROW((void)screen.AddElement(nullptr), std::invalid_argument);
        EXPECT_TRUE(screen.GetElements().empty());
    }

    TEST(ScreenTests, RemoveElementGivesBackOwnership) {
        Log log;
        TestScreen screen{"menu", log};
        Element& added = screen.AddElement(std::make_unique<TestElement>());

        const std::unique_ptr<Element> removed = screen.RemoveElement(&added);

        EXPECT_EQ(removed.get(), &added);
        EXPECT_TRUE(screen.GetElements().empty());
    }

    TEST(ScreenTests, RemovingAnUnknownElementReturnsNothing) {
        Log log;
        TestScreen screen{"menu", log};
        TestElement stranger;

        EXPECT_EQ(screen.RemoveElement(&stranger), nullptr);
    }

    class EngineScreensManagerTests : public testing::Test {
    protected:
        // Declared before the manager so it outlives the screens the manager destroys.
        Log log{};
        std::optional<EngineScreensManager> manager{std::in_place};

        TestScreen& Push(const std::string& name) {
            auto screen = std::make_unique<TestScreen>(name, log);
            TestScreen& pushed = *screen;
            manager->PushScreen(std::move(screen));
            return pushed;
        }
    };

    TEST_F(EngineScreensManagerTests, PushingEntersTheNewScreenAndPausesTheOneBelow) {
        Push("menu");
        Push("options");

        EXPECT_EQ(log, (Log{"menu:enter", "menu:pause", "options:enter"}));
    }

    TEST_F(EngineScreensManagerTests, PoppingExitsTheTopScreenAndResumesTheOneBelow) {
        Push("menu");
        Push("options");
        log.clear();

        manager->PopScreen();

        EXPECT_EQ(log, (Log{"options:exit", "options:destroyed", "menu:resume"}));
    }

    TEST_F(EngineScreensManagerTests, PoppingTheLastScreenLeavesNoneAndResumesNothing) {
        Push("menu");
        log.clear();

        manager->PopScreen();

        EXPECT_EQ(log, (Log{"menu:exit", "menu:destroyed"}));
    }

    TEST_F(EngineScreensManagerTests, PoppingWithNoScreensDoesNothing) {
        manager->PopScreen();

        EXPECT_TRUE(log.empty());
    }

    TEST_F(EngineScreensManagerTests, ScreensCanBePushedAgainAfterAllWerePopped) {
        Push("menu");
        manager->PopScreen();
        log.clear();

        Push("hud");

        EXPECT_EQ(log, (Log{"hud:enter"}));
    }

    TEST_F(EngineScreensManagerTests, DestroyingTheManagerDestroysTheTopScreenFirst) {
        Push("menu");
        Push("options");
        log.clear();

        manager.reset();

        EXPECT_EQ(log, (Log{"options:destroyed", "menu:destroyed"}));
    }

    TEST_F(EngineScreensManagerTests, ClearDestroysTheTopScreenFirstWithoutExitCallbacks) {
        Push("menu");
        Push("options");
        log.clear();

        manager->Clear();

        EXPECT_EQ(log, (Log{"options:destroyed", "menu:destroyed"}));
    }

    TEST_F(EngineScreensManagerTests, ScreensCanBePushedAgainAfterClear) {
        Push("menu");
        manager->Clear();
        log.clear();

        Push("hud");
        manager->PopScreen();

        EXPECT_EQ(log, (Log{"hud:enter", "hud:exit", "hud:destroyed"}));
    }

    TEST_F(EngineScreensManagerTests, EachScreenDrawsOnItsOwnLayerFromBottomToTop) {
        (void)Push("menu").AddElement(std::make_unique<TestElement>(ElementInput{.drawOrder = 5}));
        (void)Push("options").AddElement(std::make_unique<TestElement>(ElementInput{.drawOrder = 3}));
        UIDrawList drawList;

        manager->GetDrawCommands(drawList);

        EXPECT_EQ(Keys(drawList), (std::vector{TestElement::KeyFor(1, 5), TestElement::KeyFor(2, 3)}));
        // Anything on a higher layer sorts after everything below it, whatever its draw order.
        EXPECT_LT(TestElement::KeyFor(1, 1000), TestElement::KeyFor(2, 1));
    }

    TEST_F(EngineScreensManagerTests, HiddenElementsAreNotDrawn) {
        Log drawn;
        TestScreen& screen = Push("menu");
        (void)screen.AddElement(std::make_unique<TestElement>(ElementInput{}, "shown", &drawn));
        Element& hidden = screen.AddElement(std::make_unique<TestElement>(ElementInput{}, "hidden", &drawn));
        hidden.SetIsVisible(false);
        UIDrawList drawList;

        manager->GetDrawCommands(drawList);

        EXPECT_EQ(drawn, (Log{"shown"}));
    }

    TEST_F(EngineScreensManagerTests, HiddenScreensAreNotDrawnAndKeepTheirLayer) {
        Log drawn;
        TestScreen& hiddenScreen = Push("menu");
        (void)hiddenScreen.AddElement(std::make_unique<TestElement>(ElementInput{}, "menu-button", &drawn));
        hiddenScreen.SetIsVisible(false);
        (void)Push("hud").AddElement(std::make_unique<TestElement>(ElementInput{.drawOrder = 1}, "hud-bar", &drawn));
        UIDrawList drawList;

        manager->GetDrawCommands(drawList);

        EXPECT_EQ(drawn, (Log{"hud-bar"}));
        EXPECT_EQ(Keys(drawList), (std::vector{TestElement::KeyFor(2, 1)}));
    }

    TEST_F(EngineScreensManagerTests, ElementsOfAScreenAreCollectedInTheOrderTheyWereAdded) {
        Log drawn;
        TestScreen& screen = Push("menu");
        (void)screen.AddElement(std::make_unique<TestElement>(ElementInput{}, "first", &drawn));
        (void)screen.AddElement(std::make_unique<TestElement>(ElementInput{}, "second", &drawn));
        UIDrawList drawList;

        manager->GetDrawCommands(drawList);

        EXPECT_EQ(drawn, (Log{"first", "second"}));
    }
}
