// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <nlohmann/json.hpp>

namespace SpinoCore::Tests {
    [[nodiscard]] inline std::string Base64(const std::vector<std::uint8_t>& bytes) {
        static constexpr std::string_view alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string encoded;
        for (std::size_t index = 0; index < bytes.size(); index += 3) {
            const std::size_t remaining = bytes.size() - index;
            const std::uint32_t chunk = static_cast<std::uint32_t>(bytes[index]) << 16 |
                                        (remaining > 1 ? static_cast<std::uint32_t>(bytes[index + 1]) << 8 : 0u) |
                                        (remaining > 2 ? static_cast<std::uint32_t>(bytes[index + 2]) : 0u);
            encoded.push_back(alphabet[chunk >> 18 & 63u]);
            encoded.push_back(alphabet[chunk >> 12 & 63u]);
            encoded.push_back(remaining > 1 ? alphabet[chunk >> 6 & 63u] : '=');
            encoded.push_back(remaining > 2 ? alphabet[chunk & 63u] : '=');
        }
        return encoded;
    }

    class GLTFBuilder final {
    public:
        GLTFBuilder() {
            gltf["asset"] = {{"version", "2.0"}};
            gltf["bufferViews"] = nlohmann::json::array();
            gltf["accessors"] = nlohmann::json::array();
        }

        std::size_t AddFloats(const std::vector<float>& values, const std::string& type, const std::size_t count) {
            return AddAccessor(values.data(), values.size() * sizeof(float), 5126, type, count);
        }

        std::size_t AddUnsignedBytes(const std::vector<std::uint8_t>& values, const std::string& type, const std::size_t count) {
            return AddAccessor(values.data(), values.size(), 5121, type, count);
        }

        std::size_t AddUnsignedShorts(const std::vector<std::uint16_t>& values) {
            return AddAccessor(values.data(), values.size() * sizeof(std::uint16_t), 5123, "SCALAR", values.size());
        }

        [[nodiscard]] std::vector<std::uint8_t> Build() {
            gltf["buffers"] = nlohmann::json::array({{
                {"byteLength", mBuffer.size()},
                {"uri", "data:application/octet-stream;base64," + Base64(mBuffer)},
            }});
            const std::string text = gltf.dump();
            return {text.begin(), text.end()};
        }

        nlohmann::json gltf;

    private:
        std::size_t AddAccessor(const void* data, const std::size_t byteLength, const int componentType,
                                const std::string& type, const std::size_t count) {
            while (mBuffer.size() % 4 != 0) mBuffer.push_back(0);
            const std::size_t offset = mBuffer.size();
            mBuffer.resize(offset + byteLength);
            std::memcpy(mBuffer.data() + offset, data, byteLength);

            const std::size_t view = gltf["bufferViews"].size();
            gltf["bufferViews"].push_back({{"buffer", 0}, {"byteOffset", offset}, {"byteLength", byteLength}});

            const std::size_t accessor = gltf["accessors"].size();
            gltf["accessors"].push_back({{"bufferView", view}, {"componentType", componentType}, {"count", count}, {"type", type}});
            return accessor;
        }

        std::vector<std::uint8_t> mBuffer;
    };
}
