// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace SpinoCore::Assets {
    struct AssetIdHash final {
        using is_transparent = void;

        [[nodiscard]] std::size_t operator()(const std::string_view id) const noexcept {
            return std::hash<std::string_view>{}(id);
        }
    };

    template <typename T>
    concept CacheableAsset = std::is_object_v<T> && !std::is_const_v<T> && !std::is_volatile_v<T>;

    template <CacheableAsset T>
    class AssetCache final {
    public:
        using Pointer = std::unique_ptr<const T>;

        AssetCache() = default;
        explicit AssetCache(Pointer defaultAsset) { SetDefault(std::move(defaultAsset)); }

        void SetDefault(Pointer defaultAsset) {
            if (!defaultAsset) throw std::invalid_argument("[ASSET CACHE] Default asset must not be null.");
            mDefault = std::move(defaultAsset);
            ++mVersion;
        }

        [[nodiscard]] bool HasDefault() const noexcept { return mDefault != nullptr; }

        [[nodiscard]] const T& GetDefault() const {
            if (!mDefault) throw std::logic_error("[ASSET CACHE] No default asset was set.");
            return *mDefault;
        }

        const T& Add(std::string id, Pointer asset) {
            if (!asset) throw std::invalid_argument(std::format("[ASSET CACHE] Asset \"{}\" must not be null.", id));
            return Insert(std::move(id), std::move(asset));
        }

        [[nodiscard]] const T& Get(const std::string_view id) const { return *Find(id); }

        [[nodiscard]] const T& GetOrDefault(const std::string_view id) const {
            const auto it = mAssets.find(id);
            return it != mAssets.end() ? *it->second : GetDefault();
        }

        [[nodiscard]] bool Contains(const std::string_view id) const { return mAssets.contains(id); }

        bool Remove(const std::string_view id) {
            const auto it = mAssets.find(id);
            if (it == mAssets.end()) return false;
            mAssets.erase(it);
            ++mVersion;
            return true;
        }

        void Clear() noexcept {
            mAssets.clear();
            ++mVersion;
        }

        [[nodiscard]] std::size_t Size() const noexcept { return mAssets.size(); }
        [[nodiscard]] bool IsEmpty() const noexcept { return mAssets.empty(); }

        [[nodiscard]] std::uint64_t GetVersion() const noexcept { return mVersion; }

    private:
        const T& Insert(std::string id, Pointer asset) {
            if (id.empty()) throw std::invalid_argument("[ASSET CACHE] Asset id must not be empty.");

            auto [it, inserted] = mAssets.try_emplace(std::move(id), std::move(asset));
            if (!inserted) throw std::invalid_argument(std::format("[ASSET CACHE] Asset \"{}\" is already cached.", it->first));
            ++mVersion;
            return *it->second;
        }

        [[nodiscard]] const Pointer& Find(const std::string_view id) const {
            const auto it = mAssets.find(id);
            if (it == mAssets.end()) throw std::out_of_range(std::format("[ASSET CACHE] Asset \"{}\" is not cached.", id));
            return it->second;
        }

        std::unordered_map<std::string, Pointer, AssetIdHash, std::equal_to<>> mAssets{};
        Pointer mDefault{nullptr};
        std::uint64_t mVersion{0};
    };
}
