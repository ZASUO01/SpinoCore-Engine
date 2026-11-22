// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <gtest/gtest.h>
#include "UITestElements.h"
#include "SpinoCore/Core/UI/Elements/Element.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Core::UI::Elements::Element;
    using SpinoCore::Core::UI::Elements::ElementInput;
    using SpinoCore::Math::Matrix4f;
    using SpinoCore::Math::Vector2f;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Tests::TestElement;

    constexpr float Tolerance = 1e-4f;

    [[nodiscard]] Vector2f QuadPoint(Element& element, const float u, const float v) {
        const Vector3f point = element.GetTransform().TransformPoint(Vector3f{u, v, 0.0f});
        return {point.x, point.y};
    }

    [[nodiscard]] Vector2f Center(Element& element) { return QuadPoint(element, 0.5f, 0.5f); }

    [[nodiscard]] testing::AssertionResult Near(const Vector2f& actual, const Vector2f& expected) {
        if (Vector2f::NearlyEqual(actual, expected, Tolerance)) return testing::AssertionSuccess();
        return testing::AssertionFailure() << "got (" << actual.x << ", " << actual.y << "), expected ("
                                           << expected.x << ", " << expected.y << ")";
    }

    [[nodiscard]] std::unique_ptr<Element> MakeElement(const ElementInput& input = {}) {
        return std::make_unique<TestElement>(input);
    }

    TEST(ElementTests, TakesItsInitialStateFromTheInput) {
        const TestElement element{{.anchor = {1.0f, 2.0f}, .offSet = {3.0f, 4.0f}, .size = {50.0f, 10.0f}, .scale = 2.0f,
                                   .angleDegrees = 30.0f, .anchorAngleDegrees = 15.0f, .visible = false, .drawOrder = 7}};

        EXPECT_TRUE(Near(element.GetAnchor(), {1.0f, 2.0f}));
        EXPECT_TRUE(Near(element.GetOffset(), {3.0f, 4.0f}));
        EXPECT_TRUE(Near(element.GetScaledSize(), {100.0f, 20.0f}));
        EXPECT_FLOAT_EQ(element.GetAngleDegrees(), 30.0f);
        EXPECT_FLOAT_EQ(element.GetAnchorAngleDegrees(), 15.0f);
        EXPECT_FALSE(element.IsVisible());
        EXPECT_EQ(element.GetDrawOrder(), 7);
        EXPECT_EQ(element.GetParent(), nullptr);
        EXPECT_EQ(element.GetDepth(), 0);
    }

    TEST(ElementTests, TheQuadIsCenteredOnAnchorPlusOffsetWithTheGivenSize) {
        TestElement element{{.anchor = {100.0f, 50.0f}, .offSet = {10.0f, 5.0f}, .size = {40.0f, 20.0f}}};

        EXPECT_TRUE(Near(Center(element), {110.0f, 55.0f}));
        EXPECT_TRUE(Near(QuadPoint(element, 0.0f, 0.0f), {90.0f, 45.0f}));
        EXPECT_TRUE(Near(QuadPoint(element, 1.0f, 1.0f), {130.0f, 65.0f}));
    }

    TEST(ElementTests, ScaleGrowsTheQuadAroundItsCenter) {
        TestElement element{{.anchor = {100.0f, 100.0f}, .size = {40.0f, 20.0f}, .scale = 2.0f}};

        EXPECT_TRUE(Near(Center(element), {100.0f, 100.0f}));
        EXPECT_TRUE(Near(QuadPoint(element, 1.0f, 1.0f), {140.0f, 120.0f}));
    }

    TEST(ElementTests, RotationTurnsTheQuadAroundItsCenter) {
        TestElement element{{.anchor = {100.0f, 100.0f}, .size = {40.0f, 20.0f}, .angleDegrees = 90.0f}};

        EXPECT_TRUE(Near(Center(element), {100.0f, 100.0f}));

        EXPECT_TRUE(Near(QuadPoint(element, 1.0f, 0.5f), {100.0f, 120.0f}));
    }

    TEST(ElementTests, AnchorAngleSwingsTheOffsetAroundTheAnchorAndTurnsTheElement) {
        TestElement element{{.anchor = {100.0f, 100.0f}, .offSet = {50.0f, 0.0f}, .size = {10.0f, 10.0f},
                             .anchorAngleDegrees = 90.0f}};

        EXPECT_TRUE(Near(Center(element), {100.0f, 150.0f}));
        EXPECT_TRUE(Near(QuadPoint(element, 1.0f, 0.5f), {100.0f, 155.0f}));
    }

    TEST(ElementTests, SettersInvalidateTheCachedTransform) {
        TestElement element{{.anchor = {10.0f, 10.0f}, .size = {10.0f, 10.0f}}};
        (void)element.GetTransform();

        element.SetAnchor({20.0f, 20.0f});
        EXPECT_TRUE(Near(Center(element), {20.0f, 20.0f}));

        element.SetOffset({5.0f, 0.0f});
        EXPECT_TRUE(Near(Center(element), {25.0f, 20.0f}));

        element.SetSize({30.0f, 10.0f});
        EXPECT_TRUE(Near(QuadPoint(element, 1.0f, 0.5f), {40.0f, 20.0f}));

        element.SetScale(2.0f);
        EXPECT_TRUE(Near(QuadPoint(element, 1.0f, 0.5f), {55.0f, 20.0f}));

        element.SetAngleDegrees(90.0f);
        EXPECT_TRUE(Near(QuadPoint(element, 1.0f, 0.5f), {25.0f, 50.0f}));
    }

    TEST(ElementTests, DrawOrderMustBePositive) {
        TestElement element;

        element.SetDrawOrder(5);
        element.SetDrawOrder(0);
        element.SetDrawOrder(-3);

        EXPECT_EQ(element.GetDrawOrder(), 5);
    }

    TEST(ElementHierarchyTests, AddChildTakesOwnershipAndLinksTheParent) {
        TestElement parent;
        auto child = MakeElement();
        const Element* childAddress = child.get();

        Element& added = parent.AddChild(std::move(child));

        EXPECT_EQ(&added, childAddress);
        EXPECT_EQ(added.GetParent(), &parent);
        EXPECT_EQ(added.GetDepth(), 1);
        ASSERT_EQ(parent.GetChildren().size(), 1u);
        EXPECT_EQ(parent.GetChildren()[0].get(), childAddress);
    }

    TEST(ElementHierarchyTests, AddingANullChildThrows) {
        TestElement parent;
        std::unique_ptr<Element> nothing;

        EXPECT_THROW(parent.AddChild(std::move(nothing)), std::invalid_argument);
    }

    TEST(ElementHierarchyTests, AChildIsPlacedRelativeToItsParentCenterAndIgnoresItsOwnAnchor) {
        TestElement parent{{.anchor = {100.0f, 100.0f}, .size = {50.0f, 50.0f}}};
        Element& child = parent.AddChild(MakeElement({.anchor = {999.0f, 999.0f}, .offSet = {10.0f, 0.0f}, .size = {4.0f, 4.0f}}));

        EXPECT_TRUE(Near(Center(child), {110.0f, 100.0f}));
    }

    TEST(ElementHierarchyTests, AChildInheritsTheParentRotationAndScale) {
        TestElement parent{{.anchor = {100.0f, 100.0f}, .size = {50.0f, 50.0f}, .scale = 2.0f, .angleDegrees = 90.0f}};
        Element& child = parent.AddChild(MakeElement({.offSet = {10.0f, 0.0f}, .size = {4.0f, 4.0f}, .scale = 1.5f}));

        EXPECT_TRUE(Near(Center(child), {100.0f, 120.0f}));
        EXPECT_FLOAT_EQ(child.GetWorldScale(), 3.0f);
    }

    TEST(ElementHierarchyTests, MovingAParentMovesItsAlreadyComputedDescendants) {
        TestElement root{{.anchor = {0.0f, 0.0f}}};
        Element& child = root.AddChild(MakeElement({.offSet = {10.0f, 0.0f}}));
        Element& grandchild = child.AddChild(MakeElement({.offSet = {0.0f, 10.0f}}));
        ASSERT_TRUE(Near(Center(grandchild), {10.0f, 10.0f}));

        root.SetAnchor({100.0f, 100.0f});

        EXPECT_TRUE(Near(Center(grandchild), {110.0f, 110.0f}));
        EXPECT_TRUE(Near(Center(child), {110.0f, 100.0f}));
    }

    TEST(ElementHierarchyTests, ChangingAParentAgainAfterOnlyItWasRecomputedStillMovesTheChildren) {
        TestElement root{{.anchor = {0.0f, 0.0f}}};
        Element& child = root.AddChild(MakeElement({.offSet = {10.0f, 0.0f}}));
        (void)Center(child);

        root.SetAnchor({50.0f, 0.0f});
        (void)root.GetTransform();
        root.SetAnchor({100.0f, 0.0f});

        EXPECT_TRUE(Near(Center(child), {110.0f, 0.0f}));
    }

    TEST(ElementHierarchyTests, RemoveChildGivesBackTheChildAndForgetsIt) {
        TestElement root;
        Element& child = root.AddChild(MakeElement());

        const std::unique_ptr<Element> removed = root.RemoveChild(child);

        EXPECT_EQ(removed.get(), &child);
        EXPECT_TRUE(root.GetChildren().empty());
    }

    TEST(ElementHierarchyTests, ARemovedSubtreeBecomesItsOwnRoot) {
        TestElement root;
        Element& child = root.AddChild(MakeElement());
        Element& grandchild = child.AddChild(MakeElement());

        const std::unique_ptr<Element> removed = root.RemoveChild(child);

        EXPECT_EQ(removed->GetParent(), nullptr);
        EXPECT_EQ(removed->GetDepth(), 0);
        EXPECT_EQ(grandchild.GetDepth(), 1);
    }

    TEST(ElementHierarchyTests, ARemovedSubtreeIsPlacedWithoutTheOldParent) {
        TestElement root{{.anchor = {100.0f, 100.0f}}};
        Element& child = root.AddChild(MakeElement({.anchor = {5.0f, 5.0f}}));
        Element& grandchild = child.AddChild(MakeElement());
        (void)Center(grandchild);

        const std::unique_ptr<Element> removed = root.RemoveChild(child);

        EXPECT_TRUE(Near(Center(*removed), {5.0f, 5.0f}));
        EXPECT_TRUE(Near(Center(grandchild), {5.0f, 5.0f}));
    }

    TEST(ElementHierarchyTests, RemovingAnElementThatIsNotAChildReturnsNothing) {
        TestElement root;
        TestElement stranger;

        EXPECT_EQ(root.RemoveChild(stranger), nullptr);
    }

    TEST(ElementHierarchyTests, ARemovedChildCanBeAddedToAnotherParent) {
        TestElement first;
        TestElement second{{.anchor = {50.0f, 0.0f}}};
        Element& child = first.AddChild(MakeElement());

        Element& moved = second.AddChild(first.RemoveChild(child));

        EXPECT_EQ(moved.GetParent(), &second);
        EXPECT_TRUE(Near(Center(moved), {50.0f, 0.0f}));
    }

    TEST(ElementHierarchyTests, AChainCanReachTheMaximumDepthButNotGoPastIt) {
        TestElement root;
        Element* deepest = &root;
        for (std::size_t depth = 1; depth <= Element::MAX_DEPTH; ++depth) {
            deepest = &deepest->AddChild(MakeElement());
        }
        ASSERT_EQ(deepest->GetDepth(), Element::MAX_DEPTH);

        auto oneTooMany = MakeElement();
        EXPECT_THROW(deepest->AddChild(std::move(oneTooMany)), std::invalid_argument);

        EXPECT_NE(oneTooMany, nullptr);
    }

    TEST(ElementHierarchyTests, TheHeightOfTheAddedSubtreeCountsTowardTheDepthLimit) {
        TestElement root;
        Element& level1 = root.AddChild(MakeElement());
        Element& level2 = level1.AddChild(MakeElement());

        auto subtree = MakeElement();
        Element& subLevel1 = subtree->AddChild(MakeElement());
        subLevel1.AddChild(MakeElement());

        EXPECT_THROW(level2.AddChild(std::move(subtree)), std::invalid_argument);
        EXPECT_NO_THROW(level1.AddChild(std::move(subtree)));
    }

    TEST(ElementHierarchyTests, AddingASubtreeUpdatesTheDepthOfEveryDescendant) {
        TestElement root;
        auto subtree = MakeElement();
        Element& inner = subtree->AddChild(MakeElement());

        root.AddChild(std::move(subtree));

        EXPECT_EQ(inner.GetDepth(), 2);
    }
}
