// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#pragma once
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <vector>

namespace SpinoCore::Tests {
    [[nodiscard]] inline std::vector<std::uint8_t> SineWav(const float seconds, const int sampleRate = 22050, const float frequency = 440.0f) {
        const auto frames = static_cast<std::uint32_t>(seconds * static_cast<float>(sampleRate));
        const std::uint32_t dataBytes = frames * 2;
        std::vector<std::uint8_t> wav;

        const auto text = [&wav](const char* value) { for (int index = 0; index < 4; ++index) wav.push_back(static_cast<std::uint8_t>(value[index])); };
        const auto u32 = [&wav](const std::uint32_t value) { for (int shift = 0; shift < 32; shift += 8) wav.push_back(static_cast<std::uint8_t>(value >> shift)); };
        const auto u16 = [&wav](const std::uint16_t value) { wav.push_back(static_cast<std::uint8_t>(value)); wav.push_back(static_cast<std::uint8_t>(value >> 8)); };

        text("RIFF");
        u32(36 + dataBytes);
        text("WAVE");
        text("fmt ");
        u32(16);
        u16(1);
        u16(1);
        u32(static_cast<std::uint32_t>(sampleRate));
        u32(static_cast<std::uint32_t>(sampleRate) * 2);
        u16(2);
        u16(16);
        text("data");
        u32(dataBytes);
        for (std::uint32_t frame = 0; frame < frames; ++frame) {
            const double phase = 2.0 * std::numbers::pi * static_cast<double>(frequency) * static_cast<double>(frame) / static_cast<double>(sampleRate);
            u16(static_cast<std::uint16_t>(static_cast<std::int16_t>(std::sin(phase) * 16000.0)));
        }
        return wav;
    }

    [[nodiscard]] inline std::vector<std::uint8_t> SilentMp3(const std::size_t frameCount = 30) {
        constexpr std::size_t FrameBytes = 417;
        std::vector<std::uint8_t> mp3;
        for (std::size_t frame = 0; frame < frameCount; ++frame) {
            mp3.insert(mp3.end(), {0xFF, 0xFB, 0x90, 0xC0});
            mp3.resize(mp3.size() + FrameBytes - 4, 0);
        }
        return mp3;
    }
}
