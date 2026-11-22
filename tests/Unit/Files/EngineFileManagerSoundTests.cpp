// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <string>
#include <system_error>
#include <vector>
#include <gtest/gtest.h>
#include "../../Support/AudioBuilder.h"
#include "Assets/AssetCache.h"
#include "Files/EngineFileManager.h"
#include "Graphics/Resources/Font.h"
#include "Graphics/Resources/Model.h"
#include "Graphics/Resources/Texture.h"
#include "Sound/Resources/AudioClip.h"

namespace {
    namespace fs = std::filesystem;
    using SpinoCore::Assets::AssetCache;
    using SpinoCore::Files::EngineFileManager;
    using SpinoCore::Files::FileManagerInput;
    using SpinoCore::Graphics::Resources::Font;
    using SpinoCore::Graphics::Resources::Model;
    using SpinoCore::Graphics::Resources::Texture;
    using SpinoCore::Sound::Resources::AudioClip;
    using SpinoCore::Tests::SineWav;

    class EngineFileManagerSoundTests : public testing::Test {
    protected:
        void SetUp() override {
            folder = fs::path("EngineFileManagerSoundTests") / testing::UnitTest::GetInstance()->current_test_info()->name();
            fs::create_directories(fileManager.GetRootPath() / folder);
        }

        void TearDown() override {
            std::error_code ignored;
            fs::remove_all(fileManager.GetRootPath() / folder, ignored);
        }

        void Write(const fs::path& relative, const std::vector<std::uint8_t>& bytes) const {
            std::ofstream(fileManager.GetRootPath() / relative, std::ios::binary)
                .write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        }

        [[nodiscard]] fs::path ManifestWith(const fs::path& sound) const {
            const fs::path manifest = folder / "manifest.json";
            std::ofstream(fileManager.GetRootPath() / manifest)
                << R"({"version": "1.0", "assets": [{"name": "click", "type": "Sound", "path": ")" << sound.generic_string() << R"("}]})";
            return manifest;
        }

        void ProcessUntilReady() {
            for (int frame = 0; frame < 1'000 && !fileManager.IsReady(); ++frame) fileManager.ProcessRequests();
        }

        AssetCache<Texture> textureCache{};
        AssetCache<Font> fontCache{};
        AssetCache<Model> modelCache{};
        AssetCache<AudioClip> soundCache{};
        EngineFileManager fileManager{FileManagerInput{.textureCache = textureCache, .fontCache = fontCache, .modelCache = modelCache, .soundCache = soundCache}};
        fs::path folder;
    };

    TEST_F(EngineFileManagerSoundTests, ASoundInTheManifestIsCachedUnderItsName) {
        const fs::path sound = folder / "click.wav";
        Write(sound, SineWav(0.1f));

        fileManager.LoadFromManifest(ManifestWith(sound));
        ProcessUntilReady();

        EXPECT_TRUE(soundCache.Contains("click"));
    }

    TEST_F(EngineFileManagerSoundTests, ACachedSoundKeepsItsDuration) {
        const fs::path sound = folder / "click.wav";
        Write(sound, SineWav(0.1f));

        fileManager.LoadFromManifest(ManifestWith(sound));
        ProcessUntilReady();

        EXPECT_NEAR(soundCache.Get("click").GetDuration(), 0.1f, 0.005f);
    }

    TEST_F(EngineFileManagerSoundTests, ASoundThatCannotBeDecodedIsReportedAsFailed) {
        const fs::path sound = folder / "broken.wav";
        Write(sound, {'n', 'o', 'p', 'e'});

        fileManager.LoadFromManifest(ManifestWith(sound));
        ProcessUntilReady();

        EXPECT_EQ(fileManager.GetFailedLoads(), (std::vector<std::string>{"click"}));
    }

    TEST_F(EngineFileManagerSoundTests, AMissingSoundIsReportedAsFailed) {
        fileManager.LoadFromManifest(ManifestWith(folder / "missing.wav"));
        ProcessUntilReady();

        EXPECT_EQ(fileManager.GetFailedLoads(), (std::vector<std::string>{"click"}));
    }
}
