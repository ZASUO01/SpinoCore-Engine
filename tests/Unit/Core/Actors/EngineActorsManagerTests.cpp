// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <gtest/gtest.h>
#include "Core/Actors/EngineActorsManager.h"
#include "SpinoCore/Core/Actors/Actor.h"
#include "SpinoCore/Core/Input/InputState.h"

namespace {
    using SpinoCore::Core::Actors::Actor;
    using SpinoCore::Core::Actors::EngineActorsManager;
    using SpinoCore::Core::Input::InputState;
    using Log = std::vector<std::string>;

    class LoggedActor final : public Actor {
    public:
        LoggedActor(Log& log, std::string name, const int updateOrder = 100) : mLog(log), mName(std::move(name)) {
            SetUpdateOrder(updateOrder);
        }

        ~LoggedActor() override { mLog.emplace_back(mName + " destroyed"); }

    protected:
        void OnUpdate(float) override { mLog.emplace_back(mName + " updated"); }
        void OnProcessInput(const InputState&) override { mLog.emplace_back(mName + " got input"); }

    private:
        Log& mLog;
        std::string mName;
    };

    class SpawningActor final : public Actor {
    public:
        SpawningActor(EngineActorsManager& manager, Log& log) : mManager(manager), mLog(log) {}

    protected:
        void OnUpdate(float) override {
            if (!mSpawned) mManager.AddActor(std::make_unique<LoggedActor>(mLog, "spawned"));
            mSpawned = true;
        }

        void OnProcessInput(const InputState&) override {}

    private:
        EngineActorsManager& mManager;
        Log& mLog;
        bool mSpawned{false};
    };

    class EngineActorsManagerTests : public testing::Test {
    protected:
        Log log{};
        EngineActorsManager manager{};
    };

    TEST_F(EngineActorsManagerTests, AddingAnActorReturnsThatActor) {
        manager.AddActor(std::make_unique<LoggedActor>(log, "late", 200));
        auto early = std::make_unique<LoggedActor>(log, "early", 50);
        const Actor* expected = early.get();

        const Actor* added = manager.AddActor(std::move(early));

        EXPECT_EQ(added, expected);
    }

    TEST_F(EngineActorsManagerTests, AddingANullActorIsIgnored) {
        const Actor* added = manager.AddActor(nullptr);

        EXPECT_EQ(added, nullptr);
    }

    TEST_F(EngineActorsManagerTests, ActorsUpdateInTheirUpdateOrder) {
        manager.AddActor(std::make_unique<LoggedActor>(log, "camera", 300));
        manager.AddActor(std::make_unique<LoggedActor>(log, "player", 100));
        manager.AddActor(std::make_unique<LoggedActor>(log, "enemy", 200));

        manager.UpdateActors(0.016f);

        EXPECT_EQ(log, (Log{"player updated", "enemy updated", "camera updated"}));
    }

    TEST_F(EngineActorsManagerTests, PausedActorsAreNotUpdated) {
        Actor* paused = manager.AddActor(std::make_unique<LoggedActor>(log, "paused"));
        paused->SetState(Actor::State::Paused);

        manager.UpdateActors(0.016f);

        EXPECT_TRUE(log.empty());
    }

    TEST_F(EngineActorsManagerTests, DestroyedActorsAreRemovedAfterTheUpdate) {
        Actor* doomed = manager.AddActor(std::make_unique<LoggedActor>(log, "doomed"));
        doomed->SetState(Actor::State::Destroy);

        manager.UpdateActors(0.016f);

        EXPECT_EQ(log, (Log{"doomed destroyed"}));
    }

    TEST_F(EngineActorsManagerTests, RemovingADestroyedActorKeepsTheOthersInUpdateOrder) {
        Actor* first = manager.AddActor(std::make_unique<LoggedActor>(log, "first", 1));
        manager.AddActor(std::make_unique<LoggedActor>(log, "second", 2));
        manager.AddActor(std::make_unique<LoggedActor>(log, "third", 3));
        first->SetState(Actor::State::Destroy);
        manager.UpdateActors(0.016f);
        log.clear();

        manager.UpdateActors(0.016f);

        EXPECT_EQ(log, (Log{"second updated", "third updated"}));
    }

    TEST_F(EngineActorsManagerTests, ActorsAddedDuringAnUpdateWaitForTheNextUpdate) {
        manager.AddActor(std::make_unique<SpawningActor>(manager, log));

        manager.UpdateActors(0.016f);

        EXPECT_TRUE(log.empty());
    }

    TEST_F(EngineActorsManagerTests, ActorsAddedDuringAnUpdateRunOnTheNextUpdate) {
        manager.AddActor(std::make_unique<SpawningActor>(manager, log));
        manager.UpdateActors(0.016f);

        manager.UpdateActors(0.016f);

        EXPECT_EQ(log, (Log{"spawned updated"}));
    }

    TEST_F(EngineActorsManagerTests, OnlyActiveActorsReceiveInput) {
        manager.AddActor(std::make_unique<LoggedActor>(log, "active"));
        Actor* paused = manager.AddActor(std::make_unique<LoggedActor>(log, "paused"));
        paused->SetState(Actor::State::Paused);

        manager.SendInputToActors(InputState{});

        EXPECT_EQ(log, (Log{"active got input"}));
    }

    TEST_F(EngineActorsManagerTests, CleanUpDestroysEveryActor) {
        manager.AddActor(std::make_unique<LoggedActor>(log, "player"));

        manager.CleanUp();

        EXPECT_TRUE(manager.GetActors().empty());
    }

    TEST_F(EngineActorsManagerTests, CleanUpSafelyDestroysParentsBeforeTheirChildren) {
        Actor* parent = manager.AddActor(std::make_unique<LoggedActor>(log, "parent", 1));
        Actor* child = manager.AddActor(std::make_unique<LoggedActor>(log, "child", 2));
        child->SetParent(parent);

        manager.CleanUp();

        EXPECT_EQ(log, (Log{"parent destroyed", "child destroyed"}));
    }
}
