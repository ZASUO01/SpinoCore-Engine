// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include "Graphics/PostProcess/EnginePostProcessManager.h"
#include "SpinoCore/Graphics/PostProcess.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Graphics::EnginePostProcessManager;
    using SpinoCore::Graphics::PostProcess;
    using SpinoCore::Graphics::PostProcessLocation::MaxUserUniforms;
    using SpinoCore::Graphics::PostProcessUniform;
    using SpinoCore::Graphics::UniformType;
    using SpinoCore::Math::Vector4f;
    namespace DefaultPostProcess = SpinoCore::Graphics::DefaultPostProcess;

    [[nodiscard]] PostProcess TintPostProcess() {
        return PostProcess{
            .fragmentShader = "layout (location = 0) uniform vec4 uTint; void main() { outColor = uTint; }",
            .uniforms = {PostProcessUniform{.type = UniformType::Vec4, .value = Vector4f::One()}},
        };
    }

    TEST(EnginePostProcessManagerTests, StartsWithTheDefaultPostProcesses) {
        const EnginePostProcessManager manager;

        EXPECT_TRUE(manager.Contains(DefaultPostProcess::Grayscale));
        EXPECT_TRUE(manager.Contains(DefaultPostProcess::Vignette));
        EXPECT_TRUE(manager.Contains(DefaultPostProcess::Exposure));
    }

    TEST(EnginePostProcessManagerTests, StartsWithNothingActive) {
        const EnginePostProcessManager manager;

        EXPECT_FALSE(manager.IsActive());
    }

    TEST(EnginePostProcessManagerTests, StartsWithoutACurrentPostProcess) {
        const EnginePostProcessManager manager;

        EXPECT_TRUE(manager.GetCurrent().empty());
    }

    TEST(EnginePostProcessManagerTests, AKnownPostProcessCanBecomeCurrent) {
        EnginePostProcessManager manager;

        manager.SetCurrent(DefaultPostProcess::Grayscale);

        EXPECT_EQ(manager.GetCurrent(), DefaultPostProcess::Grayscale);
    }

    TEST(EnginePostProcessManagerTests, AnUnknownNameIsRejectedAsCurrent) {
        EnginePostProcessManager manager;

        const bool accepted = manager.SetCurrent("Blur");

        EXPECT_FALSE(accepted);
    }

    TEST(EnginePostProcessManagerTests, AnUnknownNameKeepsThePreviousCurrent) {
        EnginePostProcessManager manager;
        manager.SetCurrent(DefaultPostProcess::Vignette);

        manager.SetCurrent("Blur");

        EXPECT_EQ(manager.GetCurrent(), DefaultPostProcess::Vignette);
    }

    TEST(EnginePostProcessManagerTests, TogglingActivatesTheCurrentPostProcess) {
        EnginePostProcessManager manager;
        manager.SetCurrent(DefaultPostProcess::Grayscale);

        manager.ToggleActive();

        EXPECT_TRUE(manager.IsActive());
    }

    TEST(EnginePostProcessManagerTests, TogglingTwiceDeactivates) {
        EnginePostProcessManager manager;
        manager.SetCurrent(DefaultPostProcess::Grayscale);
        manager.ToggleActive();

        manager.ToggleActive();

        EXPECT_FALSE(manager.IsActive());
    }

    TEST(EnginePostProcessManagerTests, NothingIsActiveWithoutACurrentPostProcess) {
        EnginePostProcessManager manager;

        manager.ToggleActive();

        EXPECT_FALSE(manager.IsActive());
    }

    TEST(EnginePostProcessManagerTests, AnAddedPostProcessIsAvailable) {
        EnginePostProcessManager manager;

        manager.AddPostProcess("Tint", TintPostProcess());

        EXPECT_TRUE(manager.Contains("Tint"));
    }

    TEST(EnginePostProcessManagerTests, ANameThatIsAlreadyUsedIsRejected) {
        EnginePostProcessManager manager;

        const bool added = manager.AddPostProcess(DefaultPostProcess::Grayscale, TintPostProcess());

        EXPECT_FALSE(added);
    }

    TEST(EnginePostProcessManagerTests, AUniformWhoseValueDoesNotMatchItsTypeIsRejected) {
        EnginePostProcessManager manager;
        PostProcess mismatched = TintPostProcess();
        mismatched.uniforms[0].type = UniformType::Float;

        const bool added = manager.AddPostProcess("Tint", mismatched);

        EXPECT_FALSE(added);
    }

    TEST(EnginePostProcessManagerTests, MoreUniformsThanTheUserLocationsAreRejected) {
        EnginePostProcessManager manager;
        PostProcess crowded = TintPostProcess();
        crowded.uniforms.resize(MaxUserUniforms + 1, crowded.uniforms[0]);

        const bool added = manager.AddPostProcess("Tint", crowded);

        EXPECT_FALSE(added);
    }

    TEST(EnginePostProcessManagerTests, AUniformCanBeChangedByItsIndex) {
        EnginePostProcessManager manager;
        manager.AddPostProcess("Tint", TintPostProcess());

        const bool changed = manager.SetUniform("Tint", 0, Vector4f{0.0f, 0.0f, 1.0f, 1.0f});

        EXPECT_TRUE(changed);
    }

    TEST(EnginePostProcessManagerTests, AUniformCannotChangeItsType) {
        EnginePostProcessManager manager;
        manager.AddPostProcess("Tint", TintPostProcess());

        const bool changed = manager.SetUniform("Tint", 0, 1.0f);

        EXPECT_FALSE(changed);
    }

    TEST(EnginePostProcessManagerTests, AnIndexPastTheUniformsIsRejected) {
        EnginePostProcessManager manager;
        manager.AddPostProcess("Tint", TintPostProcess());

        const bool changed = manager.SetUniform("Tint", 1, Vector4f::One());

        EXPECT_FALSE(changed);
    }

    TEST(EnginePostProcessManagerTests, CleanUpRemovesAddedPostProcesses) {
        EnginePostProcessManager manager;
        manager.AddPostProcess("Tint", TintPostProcess());

        manager.CleanUp();

        EXPECT_FALSE(manager.Contains("Tint"));
    }

    TEST(EnginePostProcessManagerTests, CleanUpKeepsTheDefaultPostProcesses) {
        EnginePostProcessManager manager;

        manager.CleanUp();

        EXPECT_TRUE(manager.Contains(DefaultPostProcess::Vignette));
    }

    TEST(EnginePostProcessManagerTests, CleanUpDeactivatesAndForgetsTheCurrent) {
        EnginePostProcessManager manager;
        manager.SetCurrent(DefaultPostProcess::Vignette);
        manager.ToggleActive();

        manager.CleanUp();

        EXPECT_FALSE(manager.IsActive());
        EXPECT_TRUE(manager.GetCurrent().empty());
    }
}
