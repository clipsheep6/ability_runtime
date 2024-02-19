/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtest/gtest.h>
#include "gmock/gmock.h"

#include <thread>
#include <chrono>

#define private public
#define protected public
#include "lifecycle_test_base.h"
#undef private
#undef protected
#include "sa_mgr_client.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class LifecycleTest : public testing::Test, public LifecycleTestBase {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();

    bool StartNextAbility() override;

    int AttachAbility(const OHOS::sptr<OHOS::AAFwk::AbilityScheduler>& scheduler,
        const OHOS::sptr<OHOS::IRemoteObject>& token) override;

    void OnStartabilityAms();
public:
    int startLancherFlag_ = false;

    std::shared_ptr<OHOS::AAFwk::AbilityRecord> launcherAbilityRecord_{ nullptr };  // launcher ability
    OHOS::sptr<OHOS::IRemoteObject> launcherToken_{ nullptr };                      // token of launcher ability
    std::shared_ptr<OHOS::AAFwk::AbilityRecord> nextAbilityRecord_{ nullptr };      // ability being launched
    OHOS::sptr<OHOS::IRemoteObject> nextToken_{ nullptr };                          // token of ability being launched
    OHOS::sptr<OHOS::AAFwk::AbilityScheduler> launcherScheduler_{ nullptr };        // launcher ability thread interface
    OHOS::sptr<OHOS::AAFwk::AbilityScheduler> nextScheduler_{ nullptr };            // next ability thread interface
    std::unique_ptr<LifeTestCommand> command_{ nullptr };                           // test command_ interact with ams_
};

void LifecycleTest::SetUpTestCase() {}

void LifecycleTest::TearDownTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void LifecycleTest::SetUp() {}

void LifecycleTest::TearDown() {}

bool LifecycleTest::StartNextAbility()
{
    return true;
}

int LifecycleTest::AttachAbility(
    const OHOS::sptr<OHOS::AAFwk::AbilityScheduler>& scheduler, const OHOS::sptr<OHOS::IRemoteObject>& token)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    return abilityMs_->AttachAbilityThread(scheduler, token);
}

/*
 * Feature: Lifecycle schedule
 * Function: Lifecycle schedule
 * SubFunction: NA
 * FunctionPoints: AttachAbilityThread
 * EnvConditions:NA
 * CaseDescription: verify AttachAbilityThread parameters.
 * AttachAbilityThread fail if IAbilityScheduler or token is nullptr.
 */
HWTEST_F(LifecycleTest, AAFWK_AbilityMS_StartLauncherAbilityLifeCycle_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (startLancherFlag_) {
        EXPECT_TRUE(abilityMs_);
        EXPECT_TRUE(launcherAbilityRecord_);
        EXPECT_NE(abilityMs_->AttachAbilityThread(nullptr, launcherToken_), 0);
        EXPECT_NE(abilityMs_->AttachAbilityThread(launcherScheduler_, nullptr), 0);
        EXPECT_EQ(launcherAbilityRecord_->GetAbilityState(), OHOS::AAFwk::AbilityState::ACTIVE);
    }
}

/*
 * Feature: Lifecycle schedule
 * Function: Lifecycle schedule
 * SubFunction: NA
 * FunctionPoints: AttachAbilityThread
 * EnvConditions:NA
 * CaseDescription: verify launcher AbilityRecord state_ when AttachAbilityThread success.
 * 1. AbilityState transferred from INITIAL to ACTIVATING.
 * 2. AbilityRecord is attached.
 */
HWTEST_F(LifecycleTest, AAFWK_AbilityMS_StartLauncherAbilityLifeCycle_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (startLancherFlag_) {
        EXPECT_TRUE(abilityMs_);
        EXPECT_TRUE(launcherAbilityRecord_);
        EXPECT_EQ(launcherAbilityRecord_->GetAbilityState(), OHOS::AAFwk::AbilityState::ACTIVE);
        EXPECT_TRUE(launcherScheduler_);
        EXPECT_TRUE(launcherToken_);
        EXPECT_EQ(AttachAbility(launcherScheduler_, launcherToken_), 0);
        EXPECT_EQ(launcherAbilityRecord_->IsReady(), true);
    }
}

/*
 * Feature: Lifecycle schedule
 * Function: Lifecycle schedule
 * SubFunction: NA
 * FunctionPoints: AttachAbilityThread
 * EnvConditions:NA
 * CaseDescription: verify AbilityRecord transition timeout handler.
 */
HWTEST_F(LifecycleTest, AAFWK_AbilityMS_StartLauncherAbilityLifeCycle_003, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (startLancherFlag_) {
        command_->callback_ = false;
        command_->expectState_ = OHOS::AAFwk::AbilityState::ACTIVE;
        command_->state_ = OHOS::AAFwk::AbilityState::INITIAL;
        EXPECT_EQ(AttachAbility(launcherScheduler_, launcherToken_), 0);
        pthread_t tid = 0;
        pthread_create(&tid, nullptr, LifecycleTest::AbilityStartThread, command_.get());
        int ret =
            LifecycleTest::SemTimedWaitMillis(AbilityManagerService::LOAD_TIMEOUT + DELAY_TEST_TIME, command_->sem_);
        EXPECT_NE(ret, 0);
        // check timeout handler
        EXPECT_EQ(launcherAbilityRecord_->GetAbilityState(), OHOS::AAFwk::AbilityState::ACTIVE);
        pthread_join(tid, nullptr);
    }
}

/*
 * Feature: Lifecycle schedule
 * Function: Lifecycle schedule
 * SubFunction: NA
 * FunctionPoints: AttachAbilityThread
 * EnvConditions:NA
 * CaseDescription: verify AbilityTransitionDone parameters.
 * AbilityTransitionDone fail if launcher schedules incorrect Life state_.
 */
HWTEST_F(LifecycleTest, AAFWK_AbilityMS_StartLauncherAbilityLifeCycle_004, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (startLancherFlag_) {
        // AttachAbilityThread done and success
        EXPECT_EQ(AttachAbility(launcherScheduler_, launcherToken_), 0);

        command_->callback_ = true;
        command_->expectState_ = OHOS::AAFwk::AbilityState::ACTIVE;
        command_->abnormalState_ = OHOS::AAFwk::AbilityState::INACTIVE;
        pthread_t tid = 0;
        pthread_create(&tid, nullptr, LifecycleTest::AbilityStartThread, command_.get());
        int ret =
            LifecycleTest::SemTimedWaitMillis(AbilityManagerService::LOAD_TIMEOUT + DELAY_TEST_TIME, command_->sem_);
        if (ret != 0) {
            // check timeout handler
            GTEST_LOG_(INFO) << "timeout. It shouldn't happen.";
            pthread_join(tid, nullptr);
            return;
        }
        pthread_join(tid, nullptr);
    }
}

/*
 * Feature: Lifecycle schedule
 * Function: Lifecycle schedule
 * SubFunction: NA
 * FunctionPoints: AttachAbilityThread
 * EnvConditions:NA
 * CaseDescription: AttachAbilityThread done, verify AbilityRecord state_ when AbilityStartThread success.
 * 1. Life transition from UNDEFINED to ACTIVATING to ACTIVE.
 * 2. AbilityRecord is attached.
 */
HWTEST_F(LifecycleTest, AAFWK_AbilityMS_StartLauncherAbilityLifeCycle_005, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (startLancherFlag_) {
        // AttachAbilityThread done and success
        EXPECT_EQ(AttachAbility(launcherScheduler_, launcherToken_), 0);
        command_->callback_ = true;
        command_->expectState_ = OHOS::AAFwk::AbilityState::ACTIVE;
        command_->state_ = OHOS::AAFwk::AbilityState::INITIAL;
        pthread_t tid = 0;

        pthread_create(&tid, nullptr, LifecycleTest::AbilityStartThread, command_.get());
        int ret =
            LifecycleTest::SemTimedWaitMillis(AbilityManagerService::LOAD_TIMEOUT + DELAY_TEST_TIME, command_->sem_);
        if (ret != 0) {
            // check timeout handler. It won't happen normally.
            GTEST_LOG_(INFO) << "timeout. It shouldn't happen.";
            pthread_join(tid, nullptr);
            return;
        }
        PacMap saveData;
        abilityMs_->AbilityTransitionDone(launcherToken_, command_->state_, saveData);
        if (launcherAbilityRecord_->GetAbilityState() != OHOS::AAFwk::AbilityState::ACTIVE) {
            EXPECT_EQ(launcherAbilityRecord_->GetAbilityState(), OHOS::AAFwk::AbilityState::ACTIVE);
        }
        EXPECT_EQ(launcherAbilityRecord_->IsReady(), true);
        pthread_join(tid, nullptr);
    }
}

/*
 * Feature: Lifecycle schedule
 * Function: Lifecycle schedule
 * SubFunction: NA
 * FunctionPoints: AttachAbilityThread
 * EnvConditions:NA
 * CaseDescription:  hnadeler is timeout
 */
HWTEST_F(LifecycleTest, AAFWK_AbilityMS_StartLauncherAbilityLifeCycle_006, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (startLancherFlag_) {
        command_->callback_ = false;
        command_->expectState_ = OHOS::AAFwk::AbilityState::ACTIVE;
        command_->state_ = OHOS::AAFwk::AbilityState::INITIAL;
        pthread_t tid = 0;
        pthread_create(&tid, nullptr, LifecycleTest::AbilityStartThread, command_.get());
        int ret = LifecycleTest::SemTimedWaitMillis(AbilityManagerService::ACTIVE_TIMEOUT, command_->sem_);
        EXPECT_NE(ret, 0);
        // check AttachAbilityThread timeout handler
        EXPECT_EQ(launcherAbilityRecord_->IsReady(), false);
        pthread_join(tid, nullptr);
    }
}

/*
 * Feature: Lifecycle schedule
 * Function: Lifecycle schedule
 * SubFunction: NA
 * FunctionPoints: AbilityTransitionDone
 * EnvConditions:NA
 * CaseDescription: launcher OnInactive timeout, verify launcher AbilityTransitionDone timeout handler.
 */
HWTEST_F(LifecycleTest, AAFWK_AbilityMS_startAbilityLifeCycle_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (startLancherFlag_) {
        command_->callback_ = false;
        command_->expectState_ = OHOS::AAFwk::AbilityState::INACTIVE;
        command_->state_ = OHOS::AAFwk::AbilityState::INITIAL;
        // launcher is in inactivating process.
        EXPECT_EQ(AttachAbility(launcherScheduler_, launcherToken_), 0);
        EXPECT_TRUE(StartNextAbility());
        launcherAbilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVATING);
        pthread_t tid = 0;
        pthread_create(&tid, nullptr, LifecycleTest::AbilityStartThread, command_.get());
        int ret = LifecycleTest::SemTimedWaitMillis(AbilityManagerService::INACTIVE_TIMEOUT, command_->sem_);
        EXPECT_NE(ret, 0);
        // check AbilityTransitionDone timeout handler
        EXPECT_NE(nextAbilityRecord_->GetAbilityState(), OHOS::AAFwk::AbilityState::INACTIVATING);
        pthread_join(tid, nullptr);
    }
}

/*
 * Feature: Lifecycle schedule
 * Function: Lifecycle schedule
 * SubFunction: NA
 * FunctionPoints: AbilityTransitionDone
 * EnvConditions:NA
 * CaseDescription: verify AbilityTransitionDone parameters.
 * AbilityTransitionDone fail if life state_ is incompatible with
 * OnInactive process. Or launcher schedules incorrect life state_.
 */
HWTEST_F(LifecycleTest, AAFWK_AbilityMS_startAbilityLifeCycle_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (startLancherFlag_) {
        EXPECT_EQ(AttachAbility(launcherScheduler_, launcherToken_), 0);
        EXPECT_TRUE(StartNextAbility());
        // launcher is in inactivating process.
        PacMap saveData;
        EXPECT_NE(abilityMs_->AbilityTransitionDone(launcherToken_, OHOS::AAFwk::AbilityState::ACTIVE, saveData), 0);
        EXPECT_EQ(launcherAbilityRecord_->GetAbilityState(), OHOS::AAFwk::AbilityState::INACTIVATING);
        EXPECT_EQ(nextAbilityRecord_->GetAbilityState(), OHOS::AAFwk::AbilityState::INITIAL);
    }
}

/*
 * Feature: Lifecycle schedule
 * Function: Lifecycle schedule
 * SubFunction: NA
 * FunctionPoints: AttachAbilityThread
 * EnvConditions:NA
 * CaseDescription: launcher OnInactive done, verify new ability AttachAbilityThread timeout handler.
 */
HWTEST_F(LifecycleTest, AAFWK_AbilityMS_startAbilityLifeCycle_003, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (startLancherFlag_) {
        command_->callback_ = false;
        command_->expectState_ = OHOS::AAFwk::AbilityState::ACTIVE;
        command_->state_ = OHOS::AAFwk::AbilityState::INITIAL;
        EXPECT_EQ(AttachAbility(launcherScheduler_, launcherToken_), 0);
        EXPECT_TRUE(StartNextAbility());
        launcherAbilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVATING);
        PacMap saveData;
        EXPECT_EQ(abilityMs_->AbilityTransitionDone(launcherToken_, OHOS::AAFwk::AbilityState::INACTIVE, saveData), 0);
        // launcher oninactive done.
        pthread_t tid = 0;
        pthread_create(&tid, nullptr, LifecycleTest::AbilityStartThread, command_.get());
        int ret = LifecycleTest::SemTimedWaitMillis(
            AbilityManagerService::INACTIVE_TIMEOUT + DELAY_TEST_TIME, command_->sem_);
        EXPECT_NE(ret, 0);
        // check timeout handler
        EXPECT_EQ(nextAbilityRecord_->GetAbilityState(), OHOS::AAFwk::AbilityState::ACTIVATING);
        pthread_join(tid, nullptr);
    }
}

/*
 * Feature: Lifecycle schedule
 * Function: Lifecycle schedule
 * SubFunction: NA
 * FunctionPoints: AbilityTransitionDone
 * EnvConditions:NA
 * CaseDescription: launcher OnInactive done, verify AbilityTransitionDone parameter.
 * AbilityTransitionDone fail if new ability
 * IAbilityScheduler is nullptr.
 */
HWTEST_F(LifecycleTest, AAFWK_AbilityMS_startAbilityLifeCycle_004, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (startLancherFlag_) {
        EXPECT_EQ(AttachAbility(launcherScheduler_, launcherToken_), 0);
        EXPECT_TRUE(StartNextAbility());
        launcherAbilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVATING);
        PacMap saveData;
        EXPECT_EQ(abilityMs_->AbilityTransitionDone(launcherToken_, OHOS::AAFwk::AbilityState::INACTIVE, saveData), 0);
        // launcher oninactive done.
        nextAbilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::INITIAL);
        EXPECT_EQ(AttachAbility(nextScheduler_, nextToken_), 0);
        EXPECT_NE(abilityMs_->AbilityTransitionDone(nullptr, OHOS::AAFwk::AbilityState::ACTIVE, saveData), 0);
    }
}

/*
 * Feature: Lifecycle schedule
 * Function: Lifecycle schedule
 * SubFunction: NA
 * FunctionPoints: AbilityTransitionDone
 * EnvConditions:NA
 * CaseDescription: launcher OnInactive done. verify AbilityTransitionDone parameter.
 * AbilityTransitionDone fail if new ability
 * schedules incorrect state_.
 */
HWTEST_F(LifecycleTest, AAFWK_AbilityMS_startAbilityLifeCycle_005, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (startLancherFlag_) {
        command_->callback_ = true;
        command_->expectState_ = OHOS::AAFwk::AbilityState::ACTIVE;
        command_->abnormalState_ = OHOS::AAFwk::AbilityState::INACTIVE;
        command_->state_ = OHOS::AAFwk::AbilityState::INITIAL;
        EXPECT_EQ(AttachAbility(launcherScheduler_, launcherToken_), 0);
        EXPECT_TRUE(StartNextAbility());
        launcherAbilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVATING);
        PacMap saveData;
        EXPECT_EQ(abilityMs_->AbilityTransitionDone(launcherToken_, OHOS::AAFwk::AbilityState::INACTIVE, saveData), 0);
        // launcher oninactive done.
        nextAbilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::INITIAL);
        EXPECT_EQ(AttachAbility(nextScheduler_, nextToken_), 0);
        pthread_t tid = 0;
        pthread_create(&tid, nullptr, LifecycleTest::AbilityStartThread, command_.get());
        int ret =
            LifecycleTest::SemTimedWaitMillis(AbilityManagerService::LOAD_TIMEOUT + DELAY_TEST_TIME, command_->sem_);
        if (ret != 0) {
            // check timeout handler
            pthread_join(tid, nullptr);
            return;
        }
        pthread_join(tid, nullptr);
    }
}

/*
 * Feature: Lifecycle schedule
 * Function: Lifecycle schedule
 * SubFunction: NA
 * FunctionPoints: AbilityTransitionDone
 * EnvConditions:NA
 * CaseDescription: launcher OnInactive done. verify new ability AbilityTransitionDone timeout handler.
 */
HWTEST_F(LifecycleTest, AAFWK_AbilityMS_startAbilityLifeCycle_006, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (startLancherFlag_) {
        command_->callback_ = false;
        command_->expectState_ = OHOS::AAFwk::AbilityState::ACTIVE;
        command_->state_ = OHOS::AAFwk::AbilityState::INITIAL;
        EXPECT_EQ(AttachAbility(launcherScheduler_, launcherToken_), 0);
        EXPECT_TRUE(StartNextAbility());
        launcherAbilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVATING);
        // launcher oninactive done.
        EXPECT_EQ(AttachAbility(nextScheduler_, nextToken_), 0);
        pthread_t tid = 0;
        pthread_create(&tid, nullptr, LifecycleTest::AbilityStartThread, command_.get());
        int ret =
            LifecycleTest::SemTimedWaitMillis(AbilityManagerService::ACTIVE_TIMEOUT + DELAY_TEST_TIME, command_->sem_);
        EXPECT_NE(ret, 0);
        pthread_join(tid, nullptr);
        return;
    }
}

/*
 * Feature: Lifecycle schedule
 * Function: Lifecycle schedule
 * SubFunction: NA
 * FunctionPoints: AttachAbilityThread AbilityTransitionDone
 * EnvConditions:NA
 * CaseDescription: launcher OnInactive done and starts new ability success. verify new AbilityRecord.
 * 1. Launcher oninactive done and is INACTIVE.
 * 2. new ability is ACTIVE.
 */
HWTEST_F(LifecycleTest, AAFWK_AbilityMS_startAbilityLifeCycle_007, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (startLancherFlag_) {
        command_->callback_ = true;
        command_->expectState_ = OHOS::AAFwk::AbilityState::ACTIVE;
        command_->state_ = OHOS::AAFwk::AbilityState::INITIAL;
        EXPECT_EQ(AttachAbility(launcherScheduler_, launcherToken_), 0);
        EXPECT_TRUE(StartNextAbility());
        launcherAbilityRecord_->SetAbilityState(OHOS::AAFwk::AbilityState::INACTIVATING);
        PacMap saveData;
        EXPECT_EQ(abilityMs_->AbilityTransitionDone(
            launcherToken_, OHOS::AAFwk::AbilityState::INACTIVE, saveData), OHOS::ERR_OK);
        // launcher oninactive done.
        EXPECT_EQ(launcherAbilityRecord_->GetAbilityState(), OHOS::AAFwk::AbilityState::INACTIVE);
        EXPECT_EQ(AttachAbility(nextScheduler_, nextToken_), 0);
        pthread_t tid = 0;
        pthread_create(&tid, nullptr, LifecycleTest::AbilityStartThread, command_.get());
        int ret = LifecycleTest::SemTimedWaitMillis(AbilityManagerService::ACTIVE_TIMEOUT * 2, command_->sem_);
        if (ret != 0) {
            // check timeout handler
            pthread_join(tid, nullptr);
            return;
        }
        pthread_join(tid, nullptr);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
