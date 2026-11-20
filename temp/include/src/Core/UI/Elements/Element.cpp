// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "../../../../include/SpinoCore/Core/UI/Elements/Element.h"
#include <algorithm>
#include <stdexcept>
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Math/MatrixFactory.h"

namespace SpinoCore::Core::UI::Elements {
    Element::Element(const ElementInput &input)
    :mAnchor(input.anchor)
    ,mOffset(input.offSet)
    ,mSize(input.size)
    ,mScale(input.scale)
    ,mAngleDegrees(input.angleDegrees)
    ,mAnchorAngleDegrees(input.anchorAngleDegrees)
    ,mIsVisible(input.visible)
    ,mDrawOrder(input.drawOrder) {
    }

    Element::~Element() = default;

    const Math::Matrix4f& Element::GetTransform() {
        if (mIsTransformDirty) UpdateTransform();
        return mTransform;
    }

    const Math::Matrix4f& Element::GetWorldFrame() {
        if (mIsTransformDirty) UpdateTransform();
        return mWorldFrame;
    }

    float Element::GetWorldScale() {
        if (mIsTransformDirty) UpdateTransform();
        return mWorldScale;
    }

    void Element::SetTransformDirty() {
        if (mIsTransformDirty) return;

        mIsTransformDirty = true;
        for (const auto& child : mChildren) child->SetTransformDirty();
    }

    void Element::UpdateTransform() {
        using namespace Math::MatrixFactory;
        using namespace Math::Operations;
        constexpr float degToRad = Math::Constants::DegToRad<float>;

        const Math::Vector2f anchor = mParent ? Math::Vector2f{0.0f, 0.0f} : mAnchor;

        Math::Vector2f center{anchor.x + mOffset.x, anchor.y + mOffset.y};
        if (mAnchorAngleDegrees != 0.0f) {
            const float anchorRadians = mAnchorAngleDegrees * degToRad;
            const float s = Sin(anchorRadians);
            const float c = Cos(anchorRadians);
            center.x = anchor.x + mOffset.x * c - mOffset.y * s;
            center.y = anchor.y + mOffset.x * s + mOffset.y * c;
        }

        const float angleRadians = (mAngleDegrees + mAnchorAngleDegrees) * degToRad;

        const Math::Matrix4f localFrame =
            CreateTranslation(Math::Vector3f{center.x, center.y, 0.0f}) *
            CreateRotationZ(angleRadians) *
            CreateScale(mScale, mScale, 1.0f);

        mWorldFrame = mParent ? mParent->GetWorldFrame() * localFrame : localFrame;
        mWorldScale = mParent ? mParent->GetWorldScale() * mScale : mScale;

        mTransform =
            mWorldFrame *
            CreateScale(mSize.x, mSize.y, 1.0f) *
            CreateTranslation(Math::Vector3f{-0.5f, -0.5f, 0.0f});

        mIsTransformDirty = false;
    }

    Element& Element::AddChild(std::unique_ptr<Element>&& child) {
        if (!child) throw std::invalid_argument("[UI] Child is null.");
        if (child->mParent) throw std::invalid_argument("[UI] Child already has a parent.");

        for (const Element* current = this; current; current = current->mParent) {
            if (current == child.get()) throw std::invalid_argument("[UI] Child is an ancestor of this element.");
        }

        if (mDepth + 1 + child->GetSubtreeHeight() > MAX_DEPTH) {
            throw std::invalid_argument("[UI] Child would exceed the maximum hierarchy depth.");
        }

        child->mParent = this;
        child->SetDepth(mDepth + 1);
        child->SetTransformDirty();
        mChildren.push_back(std::move(child));
        return *mChildren.back();
    }

    std::unique_ptr<Element> Element::RemoveChild(const Element& child) {
        const auto it = std::ranges::find_if(mChildren, [&](const auto& c) { return c.get() == &child; });
        if (it == mChildren.end()) return nullptr;

        std::unique_ptr<Element> removed = std::move(*it);
        mChildren.erase(it);
        removed->mParent = nullptr;
        removed->SetDepth(0);
        removed->SetTransformDirty();
        return removed;
    }

    void Element::SetDepth(const std::uint8_t depth) {
        mDepth = depth;
        for (const auto& child : mChildren) child->SetDepth(depth + 1);
    }

    std::uint8_t Element::GetSubtreeHeight() const {
        std::uint8_t height = 0;
        for (const auto& child : mChildren) height = std::max<std::uint8_t>(height, child->GetSubtreeHeight() + 1);
        return height;
    }

    void Element::GetDrawCommand(Rendering::UIDrawList& drawList) {
        if (!mIsVisible) return;

        SendDrawCommand(drawList);
        for (const auto& child : mChildren) child->GetDrawCommand(drawList);
    }
}
