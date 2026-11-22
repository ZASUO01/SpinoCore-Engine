// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <vector>
#include <gtest/gtest.h>
#include "Graphics/Animation/Pose.h"
#include "Graphics/Resources/ModelData.h"
#include "SpinoCore/Math/Math.h"
#include "SpinoCore/Math/Matrix.h"
#include "SpinoCore/Math/MatrixFactory.h"
#include "SpinoCore/Math/Quaternion.h"
#include "SpinoCore/Math/Vector.h"

namespace {
    using SpinoCore::Graphics::Animation::ComputeBindPose;
    using SpinoCore::Graphics::Animation::ComputePose;
    using SpinoCore::Graphics::Animation::Sample;
    using SpinoCore::Graphics::Resources::AnimationClip;
    using SpinoCore::Graphics::Resources::Bone;
    using SpinoCore::Graphics::Resources::BoneTrack;
    using SpinoCore::Graphics::Resources::Interpolation;
    using SpinoCore::Graphics::Resources::Keyframes;
    using SpinoCore::Graphics::Resources::Skeleton;
    using SpinoCore::Math::Matrix4f;
    using SpinoCore::Math::Quaternionf;
    using SpinoCore::Math::Vector3f;
    namespace MatrixFactory = SpinoCore::Math::MatrixFactory;

    constexpr float QuarterTurn = SpinoCore::Math::Constants::PIOver2<float>;

    [[nodiscard]] Keyframes<Vector3f> FromZeroToTen(const Interpolation interpolation = Interpolation::Linear) {
        return Keyframes<Vector3f>{
            .times = {0.0f, 1.0f},
            .values = {Vector3f::Zero(), Vector3f{10.0f, 0.0f, 0.0f}},
            .interpolation = interpolation,
        };
    }

    [[nodiscard]] Skeleton ArmSkeleton() {
        Skeleton skeleton{};
        skeleton.bones.push_back(Bone{.name = "shoulder"});
        skeleton.bones.push_back(Bone{.name = "hand", .parent = 0, .translation = Vector3f{0.0f, 1.0f, 0.0f}});
        skeleton.bones[1].inverseBind = MatrixFactory::CreateTranslation(Vector3f{0.0f, -1.0f, 0.0f});
        skeleton.evaluationOrder = {0, 1};
        return skeleton;
    }

    [[nodiscard]] Vector3f TranslationOf(const Matrix4f& matrix) {
        return MatrixFactory::GetTranslation(matrix);
    }

    TEST(PoseTests, LinearKeyframesAreInterpolatedBetweenKeys) {
        const auto keyframes = FromZeroToTen();

        const Vector3f value = Sample(keyframes, 0.25f, Vector3f::Zero());

        EXPECT_TRUE(Vector3f::NearlyEqual(value, Vector3f{2.5f, 0.0f, 0.0f}));
    }

    TEST(PoseTests, StepKeyframesHoldThePreviousKey) {
        const auto keyframes = FromZeroToTen(Interpolation::Step);

        const Vector3f value = Sample(keyframes, 0.75f, Vector3f::Zero());

        EXPECT_TRUE(Vector3f::NearlyEqual(value, Vector3f::Zero()));
    }

    TEST(PoseTests, TimesBeforeTheFirstKeyUseTheFirstKey) {
        const auto keyframes = FromZeroToTen();

        const Vector3f value = Sample(keyframes, -1.0f, Vector3f{7.0f, 7.0f, 7.0f});

        EXPECT_TRUE(Vector3f::NearlyEqual(value, Vector3f::Zero()));
    }

    TEST(PoseTests, TimesAfterTheLastKeyUseTheLastKey) {
        const auto keyframes = FromZeroToTen();

        const Vector3f value = Sample(keyframes, 5.0f, Vector3f::Zero());

        EXPECT_TRUE(Vector3f::NearlyEqual(value, Vector3f{10.0f, 0.0f, 0.0f}));
    }

    TEST(PoseTests, WithoutKeyframesTheFallbackIsKept) {
        const Keyframes<Vector3f> keyframes{};

        const Vector3f value = Sample(keyframes, 0.5f, Vector3f{1.0f, 2.0f, 3.0f});

        EXPECT_TRUE(Vector3f::NearlyEqual(value, Vector3f{1.0f, 2.0f, 3.0f}));
    }

    TEST(PoseTests, RotationsAreSphericallyInterpolated) {
        const Keyframes<Quaternionf> keyframes{
            .times = {0.0f, 1.0f},
            .values = {Quaternionf::Identity(), Quaternionf{Vector3f::UnitZ(), QuarterTurn}},
            .interpolation = Interpolation::Linear,
        };

        const Quaternionf value = Sample(keyframes, 0.5f, Quaternionf::Identity());

        EXPECT_TRUE(Quaternionf::NearlyEqual(value, Quaternionf{Vector3f::UnitZ(), QuarterTurn / 2.0f}));
    }

    TEST(PoseTests, TheBindPoseCancelsTheInverseBindMatrices) {
        const Skeleton skeleton = ArmSkeleton();
        std::vector<Matrix4f> bones;

        ComputeBindPose(skeleton, bones);

        EXPECT_TRUE(Vector3f::NearlyEqual(TranslationOf(bones[1]), Vector3f::Zero()));
    }

    TEST(PoseTests, MovingAParentBoneMovesItsChild) {
        const Skeleton skeleton = ArmSkeleton();
        AnimationClip clip{.name = "reach", .duration = 1.0f, .tracks = {}};
        clip.tracks.push_back(BoneTrack{.bone = 0, .translation = FromZeroToTen(), .rotation = {}, .scale = {}});
        std::vector<Matrix4f> bones;

        ComputePose(skeleton, clip, 0.5f, bones);

        EXPECT_TRUE(Vector3f::NearlyEqual(TranslationOf(bones[1]), Vector3f{5.0f, 0.0f, 0.0f}));
    }

    TEST(PoseTests, BonesWithoutTracksKeepTheirRestPose) {
        const Skeleton skeleton = ArmSkeleton();
        const AnimationClip clip{.name = "idle", .duration = 1.0f, .tracks = {}};
        std::vector<Matrix4f> bones;

        ComputePose(skeleton, clip, 0.5f, bones);

        EXPECT_TRUE(Vector3f::NearlyEqual(TranslationOf(bones[1]), Vector3f::Zero()));
    }

    TEST(PoseTests, ThePoseHasOneMatrixPerBone) {
        const Skeleton skeleton = ArmSkeleton();
        const AnimationClip clip{.name = "idle", .duration = 1.0f, .tracks = {}};
        std::vector<Matrix4f> bones;

        ComputePose(skeleton, clip, 0.0f, bones);

        EXPECT_EQ(bones.size(), 2u);
    }
}
