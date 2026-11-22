// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include "SpinoCore/Core/Actors/Components/Component.h"

namespace SpinoCore::Core::Actors::Components {
    Component::Component(Actor &actor, const ComponentInput &input)
    :mOwner(actor)
    ,mUpdateOrder(input.mUpdateOrder)
    ,mDrawOrder(input.mDrawOrder)
    ,mIsEnabled(input.mIsEnabled)
    {}
}
