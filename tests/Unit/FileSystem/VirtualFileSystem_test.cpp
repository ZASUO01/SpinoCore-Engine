// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <optional>
#include "FileSystem/Core/VirtualFileSystem.h"
#include "FileSystem/Mount/MountPoint.h"
#include "SpinoCore/Logs/Logger.h"

using namespace SpinoCore::FileSystem;

namespace {
    struct MockState {
        bool fakeExists = false;
        bool fakeReadOnly = false;
        bool fakeWrite = false;

        std::optional<std::vector<uint8_t>> fakeRead = std::nullopt;

        mutable std::string lastQueriedPath;
        mutable int readCallCount = 0;
        mutable int existsCallCount = 0;
        mutable int writeCallCount = 0;

        bool fakeInitializeSuccess = true;
    };

    class MockMountPoint : public Mount::MountPoint {
    public:
        [[maybe_unused]] static std::unique_ptr<MockMountPoint> Create(const MockState& state) {
            auto mock = std::make_unique<MockMountPoint>(state);
            if (!mock->Initialize()) return nullptr;
            return mock;
        }

        explicit MockMountPoint(MockState  state) : mMockState(std::move(state)) {}

        [[nodiscard]] bool Exists(const std::string_view localPath) const override {
            mMockState.lastQueriedPath = localPath;
            mMockState.existsCallCount++;
            return mMockState.fakeExists;
        }

        [[nodiscard]] std::optional<std::vector<uint8_t>> Read(const std::string_view localPath) const override {
            mMockState.lastQueriedPath = localPath;
            mMockState.readCallCount++;
            return mMockState.fakeRead;
        }

        bool Write(const std::string_view localPath, const std::vector<uint8_t>& data) override {
            mMockState.lastQueriedPath = localPath;
            mMockState.writeCallCount++;
            return mMockState.fakeWrite;
        }

        [[nodiscard]] bool IsReadOnly() const override {
            return mMockState.fakeReadOnly;
        }

    protected:
        bool Initialize() override { return mMockState.fakeInitializeSuccess; }

    private:
        MockState mMockState;
    };
}

namespace {
    class VirtualFileSystemUnitTest : public testing::Test {
    protected:
        std::unique_ptr<Core::VirtualFileSystem> vfs;

        void SetUp() override {
            vfs = Core::VirtualFileSystem::Create();
        }
    };
}

TEST_F(VirtualFileSystemUnitTest, ShouldFailToMountWithEmptyProtocolOrFailedInit) {
    MockState mock;
    EXPECT_FALSE(vfs->Mount<MockMountPoint>("", mock));

    MockState failMock;
    failMock.fakeInitializeSuccess = false;
    EXPECT_FALSE(vfs->Mount<MockMountPoint>("test", failMock));
}

TEST_F(VirtualFileSystemUnitTest, ShouldReturnFalseForUnmountedProtocol) {
    EXPECT_FALSE(vfs->Exists("test://folder/image.png"));
    EXPECT_FALSE(vfs->Read("test://folder/image.png").has_value());
    EXPECT_FALSE(vfs->Write("test://folder/file.txt", {'D'}));
}

TEST_F(VirtualFileSystemUnitTest, ShouldRejectMalformedVirtualPaths) {
    EXPECT_FALSE(vfs->Exists("invalid_path"));
    EXPECT_FALSE(vfs->Read("invalid_path").has_value());
    EXPECT_FALSE(vfs->Write("invalid_path", {'D'}));

    EXPECT_FALSE(vfs->Exists("proto://"));
}

TEST_F(VirtualFileSystemUnitTest, ShouldReadFromCorrectProtocol) {
    MockState mock;
    mock.fakeExists = true;
    mock.fakeRead = std::vector<uint8_t>{'T', 'E', 'S', 'T'};

    ASSERT_TRUE(vfs->Mount<MockMountPoint>("proto", mock));

    const auto data = vfs->Read("proto://file.json");

    ASSERT_TRUE(data.has_value());
    EXPECT_EQ(data->size(), 4);
}

TEST_F(VirtualFileSystemUnitTest, ShouldRespectMountOrderAndSearchSequentiallyInRead) {
    MockState base;
    base.fakeRead = std::vector<uint8_t>{'B', 'A', 'S', 'E'};

    MockState patch;
    patch.fakeRead = std::nullopt;

    ASSERT_TRUE(vfs->Mount<MockMountPoint>("proto", base));
    ASSERT_TRUE(vfs->Mount<MockMountPoint>("proto", patch));

    const auto data = vfs->Read("proto://folder/file.txt");

    ASSERT_TRUE(data.has_value());
    const std::string result(data->begin(), data->end());
    EXPECT_EQ(result, "BASE");
}

TEST_F(VirtualFileSystemUnitTest, ShouldSearchSequentiallyInExists) {
    MockState base;
    base.fakeExists = true;

    MockState patch;
    patch.fakeExists = false;

    ASSERT_TRUE(vfs->Mount<MockMountPoint>("proto", base));
    ASSERT_TRUE(vfs->Mount<MockMountPoint>("proto", patch));

    EXPECT_TRUE(vfs->Exists("proto://folder/file.txt"));
}

TEST_F(VirtualFileSystemUnitTest, ShouldWriteToFirstWritableMount) {
    MockState base;
    base.fakeReadOnly = true;

    MockState patch1;
    patch1.fakeReadOnly = false;
    patch1.fakeWrite = true;

    MockState patch2;
    patch2.fakeReadOnly = true;

    ASSERT_TRUE(vfs->Mount<MockMountPoint>("proto", base));
    ASSERT_TRUE(vfs->Mount<MockMountPoint>("proto", patch1));
    ASSERT_TRUE(vfs->Mount<MockMountPoint>("proto", patch2));

    EXPECT_TRUE(vfs->Write("proto://folder/file.txt", {'D'}));
}