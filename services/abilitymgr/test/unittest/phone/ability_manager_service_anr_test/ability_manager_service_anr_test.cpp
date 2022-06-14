/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#define private public
#define protected public
#include "ability_manager_service.h"
#include "ability_event_handler.h"
#undef private
#undef protected

#include "app_process_data.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#include "bundlemgr/mock_bundle_manager.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "ui_service_mgr_client_mock.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace {
    const std::string DEVICE_ID = "15010038475446345206a332922cb765";
    const std::string BUNDLE_NAME = "testBundle";
    const std::string NAME = ".testMainAbility";
    const std::string EVENT_WAITING_CODE = "0";
    const std::string EVENT_CLOSE_CODE = "1";
}

namespace OHOS {
namespace AAFwk {
static void WaitUntilTaskFinished()
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    auto handler = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    std::atomic<bool> taskCalled(false);
    auto f = [&taskCalled]() { taskCalled.store(true); };
    if (handler->PostTask(f)) {
        while (!taskCalled.load()) {
            ++count;
            if (count >= maxRetryCount) {
                break;
            }
            usleep(sleepTime);
        }
    }
}

namespace {
const std::string WaitUntilTaskFinishedByTimer = "BundleMgrService";
}  // namespace

class AbilityManagerServiceAnrTest : public testing::Test {
public:
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void OnStartAms();
    void OnStopAms();
    static constexpr int TEST_WAIT_TIME = 100000;

public:
    AbilityRequest abilityRequest_;
    std::shared_ptr<AbilityRecord> abilityRecord_ {nullptr};
    std::shared_ptr<AbilityManagerService> abilityMs_ = DelayedSingleton<AbilityManagerService>::GetInstance();
};

void AbilityManagerServiceAnrTest::OnStartAms()
{
    if (abilityMs_) {
        if (abilityMs_->state_ == ServiceRunningState::STATE_RUNNING) {
            return;
        }

        abilityMs_->state_ = ServiceRunningState::STATE_RUNNING;

        abilityMs_->eventLoop_ = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
        EXPECT_TRUE(abilityMs_->eventLoop_);

        abilityMs_->handler_ = std::make_shared<AbilityEventHandler>(abilityMs_->eventLoop_, abilityMs_);

        abilityMs_->dataAbilityManager_ = std::make_shared<DataAbilityManager>();
        abilityMs_->dataAbilityManagers_.emplace(0, abilityMs_->dataAbilityManager_);
        EXPECT_TRUE(abilityMs_->dataAbilityManager_);

        abilityMs_->amsConfigResolver_ = std::make_shared<AmsConfigurationParameter>();
        EXPECT_TRUE(abilityMs_->amsConfigResolver_);
        abilityMs_->amsConfigResolver_->Parse();

        abilityMs_->pendingWantManager_ = std::make_shared<PendingWantManager>();
        EXPECT_TRUE(abilityMs_->pendingWantManager_);

        abilityMs_->eventLoop_->Run();
        return;
    }

    GTEST_LOG_(INFO) << "OnStart fail";
}

void AbilityManagerServiceAnrTest::OnStopAms()
{
    abilityMs_->OnStop();
}

void AbilityManagerServiceAnrTest::TearDownTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void AbilityManagerServiceAnrTest::SetUp()
{
    OnStartAms();
    WaitUntilTaskFinished();
}

void AbilityManagerServiceAnrTest::TearDown()
{
    OnStopAms();
}

/*
 * Feature: AbilityManagerService
 * Function: SendANRProcessID
 * SubFunction: NA
 * FunctionPoints: Kill anr process
 * EnvConditions: NA
 * CaseDescription: Fork a new process, call SendANRProcessID func in new process id
 * click close buntton, kill the new process
 */
HWTEST_F(AbilityManagerServiceAnrTest, SendANRProcessID_001, TestSize.Level1)
{
    pid_t pid;
    if ((pid=fork()) == 0) {
        for (;;) {
        }
    }
    else {
        Ace::UIServiceMgrClientMock::GetInstance()->SetDialogCheckState(pid, EVENT_CLOSE_CODE);
        auto result = abilityMs_->SendANRProcessID(pid);
        sleep(6);
        result = kill(pid, SIGKILL);
        EXPECT_EQ(result, -1);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: SendANRProcessID
 * SubFunction: NA
 * FunctionPoints: Waiting anr process
 * EnvConditions: NA
 * CaseDescription: Fork a new process, call SendANRProcessID func in new process id
 * click waiting buntton, do not kill the new process
 */
HWTEST_F(AbilityManagerServiceAnrTest, SendANRProcessID_002, TestSize.Level1)
{
    pid_t pid;
    if ((pid=fork()) == 0) {
        for (;;) {
        }
    }
    else {
        Ace::UIServiceMgrClientMock::GetInstance()->SetDialogCheckState(pid, EVENT_WAITING_CODE);
        auto result = abilityMs_->SendANRProcessID(pid);
        sleep(6);
        result = kill(pid, SIGKILL);
        EXPECT_EQ(result, 0);
    }
}
}
}