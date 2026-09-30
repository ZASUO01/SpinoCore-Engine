// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <concepts>
#include <memory>
#include "FileSystem/Mount/MountPoint.h"

namespace SpinoCore::Defs {
    template<typename T, typename... Args>
     concept HasStaticCreate = std::derived_from<T, FileSystem::Mount::MountPoint> && requires(Args&&... args) {
        { T::Create(std::forward<Args>(args)...) } -> std::same_as<std::unique_ptr<T>>;
     };
}