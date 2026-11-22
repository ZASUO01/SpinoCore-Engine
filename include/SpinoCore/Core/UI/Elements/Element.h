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

/// @brief UI elements: rectangles, images, text, buttons, input fields, selects and the cursor.
namespace SpinoCore::Core::UI::Elements {
    /// @brief Initial settings shared by every element.
    ///
    /// Positions are in window pixels with the origin at the top-left corner.
    struct ElementInput {
        /// Point the element is placed from; children ignore it and use the parent center.
        Math::Vector2f anchor{0.0f, 0.0f};
        /// Position of the element center relative to the anchor.
        Math::Vector2f offSet{0.0f, 0.0f};
        /// Width and height before scaling.
        Math::Vector2f size{20.0f, 20.0f};
        /// Uniform scale; children inherit it.
        float scale{1.0f};
        /// Rotation of the element around its center.
        float angleDegrees{0.0f};
        /// Rotation of the offset around the anchor; it also rotates the element.
        float anchorAngleDegrees{0.0f};
        /// Whether the element is drawn.
        bool visible{true};
        /// Whether the element receives input.
        bool enabled{true};
        /// Draw order inside the screen; higher values are drawn on top.
        int drawOrder{100};
    };

    /// @brief Base class of UI elements.
    ///
    /// Elements form a tree: children are placed relative to their parent center and
    /// are drawn and receive input after it.
    class Element {
    public:
        /// Deepest level a child can have in a tree.
        static constexpr std::uint8_t MAX_DEPTH = 4;

        /// @brief Creates an element.
        /// @param input Initial settings.
        explicit Element(const ElementInput& input = {});
        /// @brief Destroys the element and its children.
        virtual ~Element();

        Element(const Element&) = delete;
        Element& operator=(const Element&) = delete;
        Element(Element&&) = delete;
        Element& operator=(Element&&) = delete;

        /// @brief Moves the anchor.
        /// @param anchor New anchor in pixels.
        void SetAnchor(const Math::Vector2f& anchor) { mAnchor = anchor; SetTransformDirty(); }
        /// @return The anchor in pixels.
        [[nodiscard]] const Math::Vector2f& GetAnchor() const { return mAnchor; }

        /// @brief Moves the element relative to its anchor.
        /// @param offset New offset in pixels.
        void SetOffset(const Math::Vector2f& offset) { mOffset = offset; SetTransformDirty(); }
        /// @return The offset from the anchor.
        [[nodiscard]] const Math::Vector2f& GetOffset() const { return mOffset; }

        /// @brief Resizes the element.
        /// @param size New width and height before scaling.
        void SetSize(const Math::Vector2f& size) { mSize = size; SetTransformDirty(); }
        /// @return The size before scaling.
        [[nodiscard]] const Math::Vector2f& GetSize() const { return mSize; }

        /// @brief Changes the uniform scale.
        /// @param scale New scale.
        void SetScale(const float scale) { mScale = scale; SetTransformDirty(); }
        /// @return The uniform scale.
        [[nodiscard]] float GetScale() const { return mScale; }
        /// @return The size times the scale of the element.
        [[nodiscard]] Math::Vector2f GetScaledSize() const { return {mSize.x * mScale, mSize.y * mScale}; }

        /// @brief Rotates the element around its center.
        /// @param angle Angle in degrees.
        void SetAngleDegrees(const float angle) { mAngleDegrees = angle; SetTransformDirty(); }
        /// @return The rotation around the center, in degrees.
        [[nodiscard]] float GetAngleDegrees() const { return mAngleDegrees; }

        /// @brief Rotates the element around its anchor.
        /// @param angle Angle in degrees.
        void SetAnchorAngleDegrees(const float angle) { mAnchorAngleDegrees = angle; SetTransformDirty(); }
        /// @return The rotation around the anchor, in degrees.
        [[nodiscard]] float GetAnchorAngleDegrees() const { return mAnchorAngleDegrees; }

        /// @brief Shows or hides the element.
        /// @param visible true to draw it.
        void SetIsVisible(const bool visible) { mIsVisible = visible; }
        /// @return Whether the element is drawn.
        [[nodiscard]] bool IsVisible() const { return mIsVisible; }

        /// @brief Enables or disables input for the element.
        /// @param enabled true to receive input.
        void SetIsEnabled(const bool enabled) { mIsEnabled = enabled; }
        /// @return Whether the element receives input.
        [[nodiscard]] bool IsEnabled() const { return mIsEnabled; }

        /// @brief Changes the draw order.
        /// @param drawOrder New order; values below 1 are ignored.
        void SetDrawOrder(const int drawOrder) { if (drawOrder > 0) mDrawOrder = drawOrder; }
        /// @return The draw order.
        [[nodiscard]] int GetDrawOrder() const { return mDrawOrder; }

        /// @brief Attaches a child placed relative to this element.
        /// @param child Element to own.
        /// @return The attached child.
        /// @throws std::invalid_argument When the child is null, already has a parent, is an ancestor of this element or would pass MAX_DEPTH.
        Element& AddChild(std::unique_ptr<Element>&& child);
        /// @brief Detaches a child, which becomes a root element.
        /// @param child Child to detach.
        /// @return The child, or nullptr when it is not a child of this element.
        std::unique_ptr<Element> RemoveChild(const Element& child);

        /// @return The parent, or nullptr for a root element.
        [[nodiscard]] Element* GetParent() const { return mParent; }
        /// @return The level in the tree; root elements are at 0.
        [[nodiscard]] std::uint8_t GetDepth() const { return mDepth; }
        /// @return The children.
        [[nodiscard]] const std::vector<std::unique_ptr<Element>>& GetChildren() const { return mChildren; }

        /// @return Whether this element holds the keyboard focus.
        [[nodiscard]] virtual bool IsFocused() const { return false; }
        /// @return Whether this element or one of its children holds the keyboard focus.
        [[nodiscard]] bool HasFocus() const;

        /// @param point Point in window pixels.
        /// @return Whether the point is inside the rotated and scaled rectangle of the element.
        [[nodiscard]] bool Contains(const Math::Vector2f& point);

        /// @return The matrix that maps the unit square to the element on screen.
        [[nodiscard]] const Math::Matrix4f& GetTransform();
        /// @return The position, rotation and scale of the element center on screen.
        [[nodiscard]] const Math::Matrix4f& GetWorldFrame();
        /// @return The scale including every parent.
        [[nodiscard]] float GetWorldScale();

        /// @brief Handles input for this element and then for its children.
        /// @param state Input of the current frame.
        void ProcessInput(const Input::InputState& state);
        /// @brief Adds this element and its children to the UI draw list.
        /// @param drawList List that receives the commands.
        /// @param drawLayer Position of the screen in the stack.
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
