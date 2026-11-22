// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include "../Graphics/GLTestContext.h"
#include "../../Support/GLTFBuilder.h"
#include "Assets/AssetCache.h"
#include "Files/EngineFileManager.h"
#include "Graphics/Resources/Font.h"
#include "Graphics/Resources/Model.h"
#include "Graphics/Resources/Texture.h"
#include "SpinoCore/Files/FileManager.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    namespace fs = std::filesystem;
    using SpinoCore::Assets::AssetCache;
    using SpinoCore::Files::EngineFileManager;
    using SpinoCore::Files::FileManagerInput;
    using SpinoCore::Graphics::Resources::Font;
    using SpinoCore::Graphics::Resources::Model;
    using SpinoCore::Graphics::Resources::Texture;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::Tests::Base64;
    using SpinoCore::Tests::GLTFBuilder;
    using Json = nlohmann::json;

    class EngineFileManagerModelTests : public SpinoCore::Tests::GLTestContext {
    protected:
        void SetUp() override {
            GLTestContext::SetUp();
            if (HasFatalFailure()) return;
            textureCache.emplace(std::make_unique<const Texture>());
            modelCache.emplace(std::make_unique<const Model>());
            fileManager.emplace(FileManagerInput{.textureCache = *textureCache, .fontCache = fontCache, .modelCache = *modelCache});
            folder = fs::path("EngineFileManagerModelTests") / testing::UnitTest::GetInstance()->current_test_info()->name();
            fs::create_directories(fileManager->GetRootPath() / folder);
        }

        void TearDown() override {
            if (fileManager) {
                std::error_code ignored;
                fs::remove_all(fileManager->GetRootPath() / folder, ignored);
            }
            fileManager.reset();
            modelCache.reset();
            textureCache.reset();
            GLTestContext::TearDown();
        }

        void WriteText(const fs::path& relative, const std::string& text) const {
            std::ofstream(fileManager->GetRootPath() / relative, std::ios::binary) << text;
        }

        [[nodiscard]] fs::path WriteImage(const std::string& name) const {
            SDL_Surface* surface = SDL_CreateSurface(2, 2, SDL_PIXELFORMAT_RGBA32);
            EXPECT_NE(surface, nullptr) << SDL_GetError();
            const fs::path relative = folder / name;
            EXPECT_TRUE(SDL_SaveBMP(surface, (fileManager->GetRootPath() / relative).string().c_str())) << SDL_GetError();
            SDL_DestroySurface(surface);
            return relative;
        }

        [[nodiscard]] std::vector<std::uint8_t> ReadBytes(const fs::path& relative) const {
            std::ifstream file(fileManager->GetRootPath() / relative, std::ios::binary);
            return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
        }

        [[nodiscard]] fs::path WriteTriangle(const std::string& name, const std::string& imageUri = {}) const {
            GLTFBuilder builder;
            const auto positions = builder.AddFloats({0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f}, "VEC3", 3);
            builder.gltf["nodes"] = Json::array({{{"mesh", 0}}});
            Json material = {{"pbrMetallicRoughness", {{"baseColorFactor", {0.2, 0.4, 0.6, 1.0}}}}};
            if (!imageUri.empty()) {
                builder.gltf["images"] = Json::array({{{"uri", imageUri}}});
                builder.gltf["textures"] = Json::array({{{"source", 0}}});
                material["pbrMetallicRoughness"]["baseColorTexture"] = {{"index", 0}};
            }
            builder.gltf["materials"] = Json::array({material});
            builder.gltf["meshes"] = Json::array({{{"primitives", Json::array({{{"attributes", {{"POSITION", positions}}}, {"material", 0}}})}}});

            const auto bytes = builder.Build();
            const fs::path relative = folder / name;
            WriteText(relative, std::string(bytes.begin(), bytes.end()));
            return relative;
        }

        [[nodiscard]] fs::path WriteManifest(const Json& asset) const {
            const fs::path relative = folder / "manifest.json";
            WriteText(relative, Json{{"version", "1.0"}, {"assets", Json::array({asset})}}.dump());
            return relative;
        }

        void ProcessUntilReady() {
            for (int frame = 0; frame < 1'000 && !fileManager->IsReady(); ++frame) fileManager->ProcessRequests();
        }

        [[nodiscard]] const Model::Material& FirstMaterialOf(const std::string& id) const {
            return modelCache->Get(id).GetMaterials().front();
        }

        std::optional<AssetCache<Texture>> textureCache{};
        std::optional<AssetCache<Model>> modelCache{};
        AssetCache<Font> fontCache{};
        std::optional<EngineFileManager> fileManager{};
        fs::path folder;
    };

    TEST_F(EngineFileManagerModelTests, AModelListedInTheManifestIsCachedUnderItsName) {
        const fs::path model = WriteTriangle("hero.gltf");
        const fs::path manifest = WriteManifest({{"name", "hero"}, {"type", "Model"}, {"path", model.generic_string()}});

        fileManager->LoadFromManifest(manifest);
        ProcessUntilReady();

        EXPECT_TRUE(modelCache->Contains("hero"));
    }

    TEST_F(EngineFileManagerModelTests, AModelKeepsTheBaseColorOfItsGLTFMaterial) {
        const fs::path model = WriteTriangle("hero.gltf");

        fileManager->LoadFromSinglePath(SpinoCore::Files::AssetType::Model, "hero", model);
        ProcessUntilReady();

        EXPECT_TRUE(Vector4f::NearlyEqual(FirstMaterialOf("hero").baseColor, Vector4f{0.2f, 0.4f, 0.6f, 1.0f}));
    }

    TEST_F(EngineFileManagerModelTests, TheManifestTextureIsLoadedIntoTheTextureCache) {
        const fs::path model = WriteTriangle("hero.gltf");
        const fs::path texture = WriteImage("hero_uv.bmp");
        const fs::path manifest = WriteManifest({{"name", "hero"}, {"type", "Model"}, {"path", model.generic_string()},
                                                 {"texture", texture.generic_string()}});

        fileManager->LoadFromManifest(manifest);
        ProcessUntilReady();

        EXPECT_TRUE(textureCache->Contains(texture.generic_string()));
    }

    TEST_F(EngineFileManagerModelTests, TheManifestTextureIsUsedByTheModelMaterials) {
        const fs::path model = WriteTriangle("hero.gltf");
        const fs::path texture = WriteImage("hero_uv.bmp");
        const fs::path manifest = WriteManifest({{"name", "hero"}, {"type", "Model"}, {"path", model.generic_string()},
                                                 {"texture", texture.generic_string()}});

        fileManager->LoadFromManifest(manifest);
        ProcessUntilReady();

        EXPECT_EQ(FirstMaterialOf("hero").textureId, texture.generic_string());
    }

    TEST_F(EngineFileManagerModelTests, AMaterialFileOverridesTheBaseColor) {
        const fs::path model = WriteTriangle("hero.gltf");
        const fs::path material = folder / "hero.material.json";
        WriteText(material, R"({"version": "1.0", "baseColor": [1.0, 0.0, 0.0, 0.5]})");
        const fs::path manifest = WriteManifest({{"name", "hero"}, {"type", "Model"}, {"path", model.generic_string()},
                                                 {"material", material.generic_string()}});

        fileManager->LoadFromManifest(manifest);
        ProcessUntilReady();

        EXPECT_TRUE(Vector4f::NearlyEqual(FirstMaterialOf("hero").baseColor, Vector4f{1.0f, 0.0f, 0.0f, 0.5f}));
    }

    TEST_F(EngineFileManagerModelTests, AMaterialFileCanGiveTheModelATexture) {
        const fs::path model = WriteTriangle("hero.gltf");
        const fs::path texture = WriteImage("hero_uv.bmp");
        const fs::path material = folder / "hero.material.json";
        WriteText(material, Json{{"version", "1.0"}, {"texture", texture.generic_string()}}.dump());
        const fs::path manifest = WriteManifest({{"name", "hero"}, {"type", "Model"}, {"path", model.generic_string()},
                                                 {"material", material.generic_string()}});

        fileManager->LoadFromManifest(manifest);
        ProcessUntilReady();

        EXPECT_EQ(FirstMaterialOf("hero").textureId, texture.generic_string());
    }

    TEST_F(EngineFileManagerModelTests, ATextureReferencedByTheGLTFIsLoadedRelativeToTheModel) {
        const fs::path texture = WriteImage("skin.bmp");
        const fs::path model = WriteTriangle("hero.gltf", "skin.bmp");

        fileManager->LoadFromSinglePath(SpinoCore::Files::AssetType::Model, "hero", model);
        ProcessUntilReady();

        EXPECT_EQ(FirstMaterialOf("hero").textureId, texture.generic_string());
    }

    TEST_F(EngineFileManagerModelTests, AnEmbeddedTextureIsCachedUnderTheModelName) {
        const auto image = ReadBytes(WriteImage("embedded.bmp"));
        const fs::path model = WriteTriangle("hero.gltf", "data:image/bmp;base64," + Base64(image));

        fileManager->LoadFromSinglePath(SpinoCore::Files::AssetType::Model, "hero", model);
        ProcessUntilReady();

        EXPECT_TRUE(textureCache->Contains("hero#0"));
    }

    TEST_F(EngineFileManagerModelTests, AModelThatCannotBeParsedIsReportedAsFailed) {
        const fs::path model = folder / "broken.gltf";
        WriteText(model, "not a model");

        fileManager->LoadFromSinglePath(SpinoCore::Files::AssetType::Model, "broken", model);
        ProcessUntilReady();

        EXPECT_EQ(fileManager->GetFailedLoads(), (std::vector<std::string>{"broken"}));
    }

    TEST_F(EngineFileManagerModelTests, AModelWhoseTextureIsMissingIsReportedAsFailed) {
        const fs::path model = WriteTriangle("hero.gltf");
        const fs::path manifest = WriteManifest({{"name", "hero"}, {"type", "Model"}, {"path", model.generic_string()},
                                                 {"texture", (folder / "missing.png").generic_string()}});

        fileManager->LoadFromManifest(manifest);
        ProcessUntilReady();

        EXPECT_EQ(fileManager->GetFailedLoads(), (std::vector<std::string>{"hero"}));
    }

    TEST_F(EngineFileManagerModelTests, AModelThatFailedFallsBackToTheDefaultModel) {
        const fs::path model = folder / "broken.gltf";
        WriteText(model, "not a model");

        fileManager->LoadFromSinglePath(SpinoCore::Files::AssetType::Model, "broken", model);
        ProcessUntilReady();

        EXPECT_EQ(&modelCache->GetOrDefault("broken"), &modelCache->GetDefault());
    }

    TEST_F(EngineFileManagerModelTests, AnAbsoluteManifestTextureIsRejected) {
        const fs::path model = WriteTriangle("hero.gltf");
        const fs::path manifest = WriteManifest({{"name", "hero"}, {"type", "Model"}, {"path", model.generic_string()},
                                                 {"texture", (fileManager->GetRootPath() / "uv.png").generic_string()}});

        EXPECT_THROW(fileManager->LoadFromManifest(manifest), std::runtime_error);
    }
}
