// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "UIElement.h"
#include <algorithm>
#include <stdexcept>
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Math/MatrixFactory.h"

namespace SpinoCore::Core::UI::Elements {
    UIElement::UIElement(const ElementInput &input)
    :mAnchor(input.anchor)
    ,mOffset(input.offSet)
    ,mSize(input.size)
    ,mScale(input.scale)
    ,mAngleDegrees(input.angleDegrees)
    ,mIsVisible(input.visible)
    ,mDrawOrder(input.drawOrder) {}

    UIElement::~UIElement() = default;

    UIElement& UIElement::AddChild(std::unique_ptr<UIElement>&& child) {
        if (!child || IsSelfOrAncestor(*child)) {
            throw std::invalid_argument("[UI] Invalid child element.");
        }

        if (child->mParent) {
            throw std::invalid_argument("[UI] Element already has a parent.");
        }

        child->mParent = this;
        child->MarkTransformDirty();
        mChildren.push_back(std::move(child));
        return *mChildren.back();
    }

    std::unique_ptr<UIElement> UIElement::RemoveChild(const UIElement& child) {
        const auto it = std::ranges::find_if(mChildren, [&](const auto& c) { return c.get() == &child; });
        if (it == mChildren.end()) return nullptr;

        std::unique_ptr<UIElement> removed = std::move(*it);
        mChildren.erase(it);

        removed->mParent = nullptr;
        removed->MarkTransformDirty();
        return removed;
    }

    const Math::Matrix4f& UIElement::GetWorldTransform() {
        if (mIsTransformDirty) UpdateTransforms();
        return mWorldTransform;
    }

    const Math::Matrix4f& UIElement::GetTransform() {
        if (mIsTransformDirty) UpdateTransforms();
        return mTransform;
    }

    void UIElement::MarkTransformDirty() {
        if (mIsTransformDirty) return;

        mIsTransformDirty = true;
        for (const auto& child : mChildren) child->MarkTransformDirty();
    }

    void UIElement::UpdateTransforms() {
        using namespace Math::MatrixFactory;

        const float posX = mAnchor.x + mOffset.x;
        const float posY = mAnchor.y + mOffset.y;

        const float centerX = mSize.x * 0.5f;
        const float centerY = mSize.y * 0.5f;
        const float angleRadians = mAngleDegrees * Math::Constants::DegToRad<float>;

        const Math::Matrix4f local =
            CreateTranslation(Math::Vector3f(posX + centerX, posY + centerY, 0.0f)) *
            CreateRotationZ(angleRadians) *
            CreateScale(mScale, mScale, 1.0f) *
            CreateTranslation(Math::Vector3f(-centerX, -centerY, 0.0f));

        mWorldTransform = mParent ? mParent->GetWorldTransform() * local : local;

        mTransform = mWorldTransform * CreateScale(mSize.x, mSize.y, 1.0f);

        mIsTransformDirty = false;
    }

    bool UIElement::IsSelfOrAncestor(const UIElement& element) const {
        for (auto current = this; current; current = current->mParent) {
            if (current == &element) return true;
        }
        return false;
    }
}
