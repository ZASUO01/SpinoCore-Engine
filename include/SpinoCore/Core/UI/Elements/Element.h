// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"

namespace SpinoCore::Core::Input {
    class InputState;
}

namespace SpinoCore::Core::Rendering {
    struct UIDrawList;
}

namespace SpinoCore::Core::UI::Elements {
    struct ElementInput {
        Math::Vector2f anchor{0.0f, 0.0f};
        Math::Vector2f offSet{0.0f, 0.0f};
        Math::Vector2f size{20.0f, 20.0f};
        float scale{1.0f};
        float angleDegrees{0.0f};
        float anchorAngleDegrees{0.0f};
        bool visible{true};
        bool enabled{true};
        int drawOrder{100};
    };

    class Element {
    public:
        static constexpr std::uint8_t MAX_DEPTH = 4;

        explicit Element(const ElementInput& input = {});
        virtual ~Element();

        Element(const Element&) = delete;
        Element& operator=(const Element&) = delete;
        Element(Element&&) = delete;
        Element& operator=(Element&&) = delete;

        void SetAnchor(const Math::Vector2f& anchor) { mAnchor = anchor; SetTransformDirty(); }
        [[nodiscard]] const Math::Vector2f& GetAnchor() const { return mAnchor; }

        void SetOffset(const Math::Vector2f& offset) { mOffset = offset; SetTransformDirty(); }
        [[nodiscard]] const Math::Vector2f& GetOffset() const { return mOffset; }

        void SetSize(const Math::Vector2f& size) { mSize = size; SetTransformDirty(); }
        [[nodiscard]] const Math::Vector2f& GetSize() const { return mSize; }

        void SetScale(const float scale) { mScale = scale; SetTransformDirty(); }
        [[nodiscard]] float GetScale() const { return mScale; }
        [[nodiscard]] Math::Vector2f GetScaledSize() const { return {mSize.x * mScale, mSize.y * mScale}; }

        void SetAngleDegrees(const float angle) { mAngleDegrees = angle; SetTransformDirty(); }
        [[nodiscard]] float GetAngleDegrees() const { return mAngleDegrees; }

        void SetAnchorAngleDegrees(const float angle) { mAnchorAngleDegrees = angle; SetTransformDirty(); }
        [[nodiscard]] float GetAnchorAngleDegrees() const { return mAnchorAngleDegrees; }

        void SetIsVisible(const bool visible) { mIsVisible = visible; }
        [[nodiscard]] bool IsVisible() const { return mIsVisible; }

        void SetIsEnabled(const bool enabled) { mIsEnabled = enabled; }
        [[nodiscard]] bool IsEnabled() const { return mIsEnabled; }

        void SetDrawOrder(const int drawOrder) { if (drawOrder > 0) mDrawOrder = drawOrder; }
        [[nodiscard]] int GetDrawOrder() const { return mDrawOrder; }

        Element& AddChild(std::unique_ptr<Element>&& child);
        std::unique_ptr<Element> RemoveChild(const Element& child);

        [[nodiscard]] Element* GetParent() const { return mParent; }
        [[nodiscard]] std::uint8_t GetDepth() const { return mDepth; }
        [[nodiscard]] const std::vector<std::unique_ptr<Element>>& GetChildren() const { return mChildren; }

        // An element holding the keyboard focus (e.g. an InputField being typed in) gets the screen's input alone.
        [[nodiscard]] virtual bool IsFocused() const { return false; }
        // True when this element or any of its children is focused.
        [[nodiscard]] bool HasFocus() const;

        // True when the point (in UI coordinates) is inside the element, following its position, rotation and scale.
        [[nodiscard]] bool Contains(const Math::Vector2f& point);

        [[nodiscard]] const Math::Matrix4f& GetTransform();
        [[nodiscard]] const Math::Matrix4f& GetWorldFrame();
        [[nodiscard]] float GetWorldScale();

        void ProcessInput(const Input::InputState& state);
        void GetDrawCommand(Rendering::UIDrawList& drawList, int drawLayer);

    protected:
        virtual void OnProcessInput(const Input::InputState& state) = 0;
        virtual void SetDrawCommand(Rendering::UIDrawList& drawList, int drawLayer) = 0;
    private:
        void SetTransformDirty();
        void UpdateTransform();
        void SetDepth(std::uint8_t depth);

        [[nodiscard]] std::uint8_t GetSubtreeHeight() const;

        Math::Vector2f mAnchor;
        Math::Vector2f mOffset;
        Math::Vector2f mSize;
        float mScale;
        float mAngleDegrees;
        float mAnchorAngleDegrees;
        bool mIsVisible;
        bool mIsEnabled;
        int mDrawOrder;

        Math::Matrix4f mTransform{Math::Matrix4f::Identity()};
        Math::Matrix4f mWorldFrame{Math::Matrix4f::Identity()};
        float mWorldScale{1.0f};
        bool mIsTransformDirty{true};
        std::uint8_t mDepth{0};

        Element* mParent{nullptr};
        std::vector<std::unique_ptr<Element>> mChildren;
    };
}
