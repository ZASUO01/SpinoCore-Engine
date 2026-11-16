// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <memory>
#include <vector>
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::UI::Elements {
    struct ElementInput {
        Math::Vector2f anchor{0.0f, 0.0f};
        Math::Vector2f offSet{0.0f, 0.0f};
        Math::Vector2f size{20.0f, 20.0f};
        float scale{1.0f};
        float angleDegrees{0.0f};
        bool visible{true};
        int drawOrder{100};
    };

    class UIElement {
    public:
        explicit UIElement(const ElementInput& input = {});
        virtual ~UIElement();

        UIElement(const UIElement&) = delete;
        UIElement& operator=(const UIElement&) = delete;
        UIElement(UIElement&&) = delete;
        UIElement& operator=(UIElement&&) = delete;

        void SetAnchor(const Math::Vector2f& anchor) { mAnchor = anchor; MarkTransformDirty(); }
        const Math::Vector2f& GetAnchor() const { return mAnchor; }

        void SetOffset(const Math::Vector2f& offset) { mOffset = offset; MarkTransformDirty(); }
        const Math::Vector2f& GetOffset() const { return mOffset; }

        void SetSize(const Math::Vector2f& size) { mSize = size; MarkTransformDirty(); }
        const Math::Vector2f& GetSize() const { return mSize; }

        void SetScale(const float scale) { mScale = scale; MarkTransformDirty(); }
        float GetScale() const { return mScale; }

        void SetAngleDegrees(const float angle) { mAngleDegrees = angle; MarkTransformDirty(); }
        float GetAngleDegrees() const { return mAngleDegrees; }

        void SetIsVisible(const bool visible) { mIsVisible = visible; }
        bool IsVisible() const { return mIsVisible; }

        void SetDrawOrder(const int drawOrder) { if (drawOrder > 0) mDrawOrder = drawOrder; }
        int GetDrawOrder() const { return mDrawOrder; }

        UIElement& AddChild(std::unique_ptr<UIElement>&& child);
        std::unique_ptr<UIElement> RemoveChild(const UIElement& child);

        UIElement* GetParent() const { return mParent; }
        const std::vector<std::unique_ptr<UIElement>>& GetChildren() const { return mChildren; }

        const Math::Matrix4f& GetWorldTransform();
        const Math::Matrix4f& GetTransform();

    private:
        void MarkTransformDirty();
        void UpdateTransforms();
        bool IsSelfOrAncestor(const UIElement& element) const;

        Math::Vector2f mAnchor;
        Math::Vector2f mOffset;
        Math::Vector2f mSize;
        float mScale;
        float mAngleDegrees;
        bool mIsVisible;
        int mDrawOrder;

        UIElement* mParent{nullptr};
        std::vector<std::unique_ptr<UIElement>> mChildren;

        Math::Matrix4f mWorldTransform{Math::Matrix4f::Identity()};
        Math::Matrix4f mTransform{Math::Matrix4f::Identity()};
        bool mIsTransformDirty{true};
    };
}
