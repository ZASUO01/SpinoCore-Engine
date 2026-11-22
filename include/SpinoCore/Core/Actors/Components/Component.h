// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once

namespace SpinoCore::Core::Actors {
    class Actor;
}

namespace SpinoCore::Core::Rendering {
    struct ModelDrawList;
}

namespace SpinoCore::Core::Actors::Components {
    struct ComponentInput {
        int mUpdateOrder{100};
        int mDrawOrder{100};
        bool mIsEnabled{true};
    };

    class Component {
    public:
        explicit Component(Actor &actor, const ComponentInput& input = {});
        virtual ~Component() = default;

        virtual void Update(float deltaTime) = 0;
        virtual void GetDrawCommand(Rendering::ModelDrawList& drawList) = 0;

        [[nodiscard]] int GetUpdateOrder() const { return mUpdateOrder; }
        void SetUpdateOrder(const int updateOrder) { mUpdateOrder = updateOrder; }

        [[nodiscard]] int GetDrawOrder() const { return mDrawOrder; }
        void SetDrawOrder(const int drawOrder) { mDrawOrder = drawOrder; }

        void SetEnabled(const bool enabled) { mIsEnabled = enabled; }
        [[nodiscard]] bool IsEnabled() const { return mIsEnabled; }

        [[nodiscard]] Actor* GetActor() const { return &mOwner; }

    private:
        Actor& mOwner;
        int mUpdateOrder{100};
        int mDrawOrder{100};
        bool mIsEnabled{true};
    };
}