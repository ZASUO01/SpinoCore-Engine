// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>
#include <system_error>
#include <vector>
#include "FileSystem/FileWorker.h"
#include "FileSystem/Core/VirtualFileSystem.h"
#include "FileSystem/Mount/FolderMount.h"

using namespace SpinoCore::FileSystem;
using namespace std::chrono_literals;

namespace {
    class FileWorkerIntegrationTest : public testing::Test {
    protected:
        std::filesystem::path testRoot;
        std::filesystem::path dataDir;
        std::unique_ptr<FileWorker> worker;

        void SetUp() override {
            testRoot = std::filesystem::temp_directory_path() / "SpinoCore_Worker_Test";
            dataDir = testRoot / "data";

            std::error_code ec;
            std::filesystem::create_directories(dataDir, ec);
            ASSERT_FALSE(ec);

            auto vfs = Core::VirtualFileSystem::Create();
            ASSERT_TRUE(vfs->Mount<Mount::FolderMount>("data", dataDir, false));

            worker = FileWorker::Create(std::move(vfs));
        }

        void TearDown() override {
            worker.reset();

            if (std::error_code ec; std::filesystem::exists(testRoot, ec)) {
                std::filesystem::remove_all(testRoot, ec);
            }
        }

        static void CreateRawPhysicalFile(const std::filesystem::path& filepath, const std::string_view content = "ASYNC_READ_OK") {
            std::ofstream out(filepath, std::ios::binary | std::ios::trunc);
            out.write(content.data(), static_cast<std::streamsize>(content.size()));
        }

        [[nodiscard]] static std::optional<FileResponse> WaitForResponse(FileWorker& targetWorker, const std::chrono::milliseconds timeout = 2000ms) {
            const auto start = std::chrono::steady_clock::now();
            while (std::chrono::steady_clock::now() - start < timeout) {
                if (auto res = targetWorker.TryPopResponse()) {
                    return res;
                }
                std::this_thread::sleep_for(2ms);
            }
            return std::nullopt;
        }
    };
}

TEST_F(FileWorkerIntegrationTest, ShouldProcessAsyncReadCorrectly) {
    CreateRawPhysicalFile(dataDir / "config.txt");

    const FileRequest req{
        .id = 101,
        .op = FileOpType::READ,
        .virtualPath = "data://config.txt",
        .data = {}
    };

    worker->RequestAsync(req);

    EXPECT_TRUE(worker->IsBusy());

    const auto response = WaitForResponse(*worker);

    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->id, 101);
    EXPECT_TRUE(response->success);

    const std::string resultStr(response->data.begin(), response->data.end());
    EXPECT_EQ(resultStr, "ASYNC_READ_OK");

    EXPECT_FALSE(worker->IsBusy());
}

TEST_F(FileWorkerIntegrationTest, ShouldProcessAsyncWriteCorrectly) {
    const FileRequest req{
        .id = 202,
        .op = FileOpType::WRITE,
        .virtualPath = "data://save.dat",
        .data = {'S', 'A', 'V', 'E'}
    };

    worker->RequestAsync(req);

    const auto response = WaitForResponse(*worker);

    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->id, 202);
    EXPECT_TRUE(response->success);

    EXPECT_TRUE(std::filesystem::exists(dataDir / "save.dat"));
}

TEST_F(FileWorkerIntegrationTest, ShouldProcessSyncRequestCorrectly) {
    CreateRawPhysicalFile(dataDir / "manifest.json", "{\"version\": 1}");

    const FileRequest req{
        .id = 303,
        .op = FileOpType::READ,
        .virtualPath = "data://manifest.json",
        .data = {}
    };

    const auto response = worker->RequestSync(req);

    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.id, 303);
    const std::string resultStr(response.data.begin(), response.data.end());
    EXPECT_EQ(resultStr, "{\"version\": 1}");

    EXPECT_FALSE(worker->IsBusy());
}

TEST_F(FileWorkerIntegrationTest, ShouldHandleMultipleRequestsInOrder) {
    constexpr int numRequests = 50;

    for (int i = 0; i < numRequests; ++i) {
        FileRequest req{
            .id = static_cast<uint64_t>(i),
            .op = FileOpType::WRITE,
            .virtualPath = "data://file_" + std::to_string(i) + ".bin",
            .data = { static_cast<uint8_t>(i) }
        };
        worker->RequestAsync(req);
    }

    EXPECT_TRUE(worker->IsBusy());

    int responsesReceived = 0;
    const auto start = std::chrono::steady_clock::now();

    while (responsesReceived < numRequests && std::chrono::steady_clock::now() - start < 3000ms) {
        if (const auto res = worker->TryPopResponse()) {
            EXPECT_TRUE(res->success);

            EXPECT_EQ(res->id, static_cast<uint64_t>(responsesReceived));

            responsesReceived++;
        } else {
            std::this_thread::sleep_for(1ms);
        }
    }

    EXPECT_EQ(responsesReceived, numRequests);
    EXPECT_FALSE(worker->IsBusy());
}

TEST_F(FileWorkerIntegrationTest, ShouldHandleConcurrentProducersSafely) {
    constexpr int NUM_THREADS = 5;
    constexpr int REQUESTS_PER_THREAD = 20;

    std::vector<std::jthread> producers;
    for (int i = 0; i < NUM_THREADS; ++i) {
        producers.emplace_back([&, i] {
            for (int j = 0; j < REQUESTS_PER_THREAD; ++j) {
                FileRequest req{
                    .id = static_cast<uint64_t>(i * 100 + j),
                    .op = FileOpType::WRITE,
                    .virtualPath = "data://concurrent_" + std::to_string(i) + "_" + std::to_string(j) + ".bin",
                    .data = {0xFF}
                };
                worker->RequestAsync(req);
            }
        });
    }

    producers.clear();

    int totalResponses = 0;
    const auto start = std::chrono::steady_clock::now();

    while (totalResponses < NUM_THREADS * REQUESTS_PER_THREAD && std::chrono::steady_clock::now() - start < 4000ms) {
        if (const auto res = worker->TryPopResponse()) {
            EXPECT_TRUE(res->success);
            totalResponses++;
        } else {
            std::this_thread::sleep_for(1ms);
        }
    }

    EXPECT_EQ(totalResponses, NUM_THREADS * REQUESTS_PER_THREAD);
    EXPECT_FALSE(worker->IsBusy());
}