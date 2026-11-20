// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <cstddef>
#include <memory>
#include <optional>
#include <gtest/gtest.h>
#include <SDL3/SDL.h>
#include "Core/Cameras/EngineCamerasManager.h"
#include "Core/Events/EventBus.h"
#include "Core/Events/TypeEvents/WindowEvents.h"
#include "Core/Rendering/Renderer.h"
#include "Core/Window/EngineWindow.h"
#include "SDL/Context.h"
#include "SpinoCore/Core/Cameras/Camera.h"
#include "SpinoCore/Core/Window/WindowProps.h"
#include "SpinoCore/Core/Window/WindowSize.h"
#include "SpinoCore/Game/GameInfo.h"
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/Vector.h"
#include "SpinoCore/Utils/Size2D.h"

namespace {
    using SpinoCore::Core::Cameras::Camera;
    using SpinoCore::Core::Cameras::EngineCamerasManager;
    using SpinoCore::Core::Events::EventBus;
    using SpinoCore::Core::Events::WindowResizeEvent;
    using SpinoCore::Core::Rendering::Renderer;
    using SpinoCore::Core::Window::EngineWindow;
    using SpinoCore::Core::Window::WINDOW_SIZE_MIN;
    using SpinoCore::Core::Window::WindowProps;
    using SpinoCore::Game::GameInfo;
    using SpinoCore::Math::Matrix4f;
    using SpinoCore::Math::Vector3f;
    using SpinoCore::Math::Vector4f;
    using SpinoCore::SDL::Context;
    using SpinoCore::Utils::Size2D;

    // A camera on the +X axis looking at the origin, that moves one unit further away on every update.
    class TestCamera final : public Camera {
    public:
        explicit TestCamera(int* updates = nullptr) : mUpdates(updates) {
            mEye = Vector3f{10.0f, 0.0f, 0.0f};
            mFov = SpinoCore::Math::Constants::PIOver2<float>;
            mNear = 1.0f;
            mFar = 100.0f;
            Apply3DView();
        }

        void Update(const float deltaTime) override {
            if (mUpdates) ++*mUpdates;
            mEye.x += deltaTime;
            Apply3DView();
        }

    private:
        int* mUpdates;
    };

    [[nodiscard]] bool SameMatrix(const Matrix4f& a, const Matrix4f& b) {
        for (std::size_t i = 0; i < 16; ++i) {
            if (a.data[i] != b.data[i]) return false;
        }
        return true;
    }

    // The aspect ratio the projection was built for: with a vertical FOV, x is scaled by 1 / aspect relative to y.
    [[nodiscard]] float AspectOf(const Matrix4f& projection) { return projection(1, 1) / projection(0, 0); }

    class EngineCamerasManagerTests : public testing::Test {
    protected:
        void SetUp() override {
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
            SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
            context.emplace(GameInfo{});
            window.emplace(WindowProps{.size = WINDOW_SIZE_MIN});
            renderer.emplace(window->GetSDLWindow(), window->GetSize(), bus);
            manager.emplace(window->GetSize(), *renderer, bus);
        }

        void DispatchResize(const Size2D size) {
            WindowResizeEvent event{};
            event.size = size;
            event.pixelSize = size;
            bus.Dispatch(event);
        }

        [[nodiscard]] bool RendererShows(const Camera& camera) const {
            const auto& matrices = renderer->GetCameraMatrices();
            return SameMatrix(matrices.view, camera.GetViewMatrix()) &&
                   SameMatrix(matrices.projection, camera.GetProjectionMatrix());
        }

        std::optional<Context> context{};
        EventBus bus{};
        std::optional<EngineWindow> window{};
        std::optional<Renderer> renderer{};
        std::optional<EngineCamerasManager> manager{};
    };

    TEST_F(EngineCamerasManagerTests, StartsWithADefaultCameraThatTheRendererUses) {
        Camera* defaultCamera = manager->GetCamera("default");

        ASSERT_NE(defaultCamera, nullptr);
        EXPECT_TRUE(RendererShows(*defaultCamera));
    }

    TEST_F(EngineCamerasManagerTests, TheDefaultCameraMatchesTheWindowAspectRatio) {
        const auto size = window->GetSize();

        EXPECT_NEAR(AspectOf(manager->GetCamera("default")->GetProjectionMatrix()),
                    static_cast<float>(size.width) / static_cast<float>(size.height), 1e-5f);
    }

    TEST_F(EngineCamerasManagerTests, AddedCamerasAreSizedToTheWindowButNotMadeCurrent) {
        manager->AddCamera(std::make_unique<TestCamera>(), "test");
        const Camera* added = manager->GetCamera("test");
        const auto size = window->GetSize();

        ASSERT_NE(added, nullptr);
        EXPECT_NEAR(AspectOf(added->GetProjectionMatrix()), static_cast<float>(size.width) / static_cast<float>(size.height), 1e-5f);
        EXPECT_TRUE(RendererShows(*manager->GetCamera("default")));
    }

    TEST_F(EngineCamerasManagerTests, AddingANameThatIsTakenKeepsTheFirstCamera) {
        manager->AddCamera(std::make_unique<TestCamera>(), "test");
        const Camera* first = manager->GetCamera("test");

        manager->AddCamera(std::make_unique<TestCamera>(), "test");

        EXPECT_EQ(manager->GetCamera("test"), first);
    }

    TEST_F(EngineCamerasManagerTests, AnUnknownNameHasNoCamera) {
        EXPECT_EQ(manager->GetCamera("missing"), nullptr);
    }

    TEST_F(EngineCamerasManagerTests, SwitchingCamerasSendsTheNewMatricesToTheRenderer) {
        manager->AddCamera(std::make_unique<TestCamera>(), "test");

        manager->SetCurrentCamera("test");

        EXPECT_TRUE(RendererShows(*manager->GetCamera("test")));
    }

    TEST_F(EngineCamerasManagerTests, SwitchingToAnUnknownCameraKeepsTheCurrentOne) {
        manager->SetCurrentCamera("missing");

        EXPECT_TRUE(RendererShows(*manager->GetCamera("default")));
    }

    TEST_F(EngineCamerasManagerTests, RemovingTheCurrentCameraFallsBackToTheDefault) {
        manager->AddCamera(std::make_unique<TestCamera>(), "test");
        manager->SetCurrentCamera("test");

        manager->RemoveCamera("test");

        EXPECT_EQ(manager->GetCamera("test"), nullptr);
        EXPECT_TRUE(RendererShows(*manager->GetCamera("default")));
    }

    TEST_F(EngineCamerasManagerTests, RemovingAnotherCameraKeepsTheCurrentOne) {
        manager->AddCamera(std::make_unique<TestCamera>(), "current");
        manager->AddCamera(std::make_unique<TestCamera>(), "other");
        manager->SetCurrentCamera("current");

        manager->RemoveCamera("other");

        EXPECT_EQ(manager->GetCamera("other"), nullptr);
        EXPECT_TRUE(RendererShows(*manager->GetCamera("current")));
    }

    TEST_F(EngineCamerasManagerTests, TheDefaultCameraCannotBeRemoved) {
        manager->RemoveCamera("default");

        EXPECT_NE(manager->GetCamera("default"), nullptr);
    }

    TEST_F(EngineCamerasManagerTests, UpdateMovesOnlyTheCurrentCameraAndRefreshesTheRenderer) {
        int currentUpdates = 0;
        int otherUpdates = 0;
        manager->AddCamera(std::make_unique<TestCamera>(&currentUpdates), "current");
        manager->AddCamera(std::make_unique<TestCamera>(&otherUpdates), "other");
        manager->SetCurrentCamera("current");

        manager->Update(2.0f);

        EXPECT_EQ(currentUpdates, 1);
        EXPECT_EQ(otherUpdates, 0);
        const Camera& current = *manager->GetCamera("current");
        EXPECT_TRUE(RendererShows(current));
        // The eye moved from x = 10 to x = 12, so the origin is now 12 units in front of it.
        EXPECT_TRUE(Vector3f::NearlyEqual(current.GetViewMatrix().TransformPoint(Vector3f::Zero()), Vector3f{0.0f, 0.0f, -12.0f}));
    }

    TEST_F(EngineCamerasManagerTests, AWindowResizeUpdatesEveryCameraAndTheRenderer) {
        manager->AddCamera(std::make_unique<TestCamera>(), "test");

        DispatchResize({.width = 900, .height = 300});

        EXPECT_NEAR(AspectOf(manager->GetCamera("default")->GetProjectionMatrix()), 3.0f, 1e-5f);
        EXPECT_NEAR(AspectOf(manager->GetCamera("test")->GetProjectionMatrix()), 3.0f, 1e-5f);
        EXPECT_TRUE(RendererShows(*manager->GetCamera("default")));
    }

    TEST_F(EngineCamerasManagerTests, CamerasAddedAfterAResizeUseTheNewSize) {
        DispatchResize({.width = 900, .height = 300});

        manager->AddCamera(std::make_unique<TestCamera>(), "late");

        EXPECT_NEAR(AspectOf(manager->GetCamera("late")->GetProjectionMatrix()), 3.0f, 1e-5f);
    }

    TEST_F(EngineCamerasManagerTests, AResizeToAnEmptyWindowIsIgnored) {
        const Matrix4f before = manager->GetCamera("default")->GetProjectionMatrix();

        DispatchResize({.width = 0, .height = 0});

        EXPECT_TRUE(SameMatrix(manager->GetCamera("default")->GetProjectionMatrix(), before));
    }

    TEST_F(EngineCamerasManagerTests, AfterTheManagerIsDestroyedResizesNoLongerReachIt) {
        manager.reset();

        // Would touch freed cameras if the subscription outlived the manager (caught by ASan).
        DispatchResize({.width = 900, .height = 300});

        SUCCEED();
    }

    TEST_F(EngineCamerasManagerTests, TheRendererCombinesTheCameraMatricesProjectionAfterView) {
        const auto& matrices = renderer->GetCameraMatrices();
        const Vector4f point{1.0f, 2.0f, -3.0f, 1.0f};

        const Vector4f combined = matrices.viewProjection * point;
        const Vector4f stepByStep = matrices.projection * (matrices.view * point);

        EXPECT_TRUE(SpinoCore::Math::Vector4f::NearlyEqual(combined, stepByStep, 1e-4f));
    }
}
