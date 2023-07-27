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
#undef private
#undef protected

#include "ability_manager_errors.h"
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
class AbilityManagerServiceAnrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static constexpr int TEST_WAIT_TIME = 100000;
};

void AbilityManagerServiceAnrTest::SetUpTestCase() {}

void AbilityManagerServiceAnrTest::TearDownTestCase() {}

void AbilityManagerServiceAnrTest::SetUp() {}

void AbilityManagerServiceAnrTest::TearDown() {}

/*
 * Feature: AbilityManagerService
 * Function: SendANRProcessID
 * SubFunction: NA
 * FunctionPoints: Kill anr process
 * EnvConditions: NA
 * CaseDescription: Fork a new process, call SendANRProcessID func in new process id
 * click close button, kill the new process
 */
HWTEST_F(AbilityManagerServiceAnrTest, SendANRProcessID_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    pid_t pid;
    if ((pid = fork()) == 0) {
        for (;;) {
        }
    } else {
        Ace::UIServiceMgrClient::GetInstance()->SetDialogCheckState(pid, EVENT_CLOSE_CODE);
        auto result = abilityMs_->SendANRProcessID(pid);
        sleep(6);
        if (result == ERR_OK) {
            EXPECT_FALSE(Ace::UIServiceMgrClient::GetInstance()->GetAppRunningState());
        }
        kill(pid, SIGKILL);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: SendANRProcessID
 * SubFunction: NA
 * FunctionPoints: Waiting anr process
 * EnvConditions: NA
 * CaseDescription: Fork a new process, call SendANRProcessID func in new process id
 * click waiting button, do not kill the new process
 */
HWTEST_F(AbilityManagerServiceAnrTest, SendANRProcessID_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    pid_t pid;
    if ((pid = fork()) == 0) {
        for (;;) {
            usleep(500);
        }
    } else {
        Ace::UIServiceMgrClient::GetInstance()->SetDialogCheckState(pid, EVENT_WAITING_CODE);
        auto result = abilityMs_->SendANRProcessID(pid);
        sleep(6);
        if (result == ERR_OK) {
            EXPECT_TRUE(Ace::UIServiceMgrClient::GetInstance()->GetAppRunningState());
        }
        (void)kill(pid, SIGKILL);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: SendANRProcessID
 * SubFunction: NA
 * FunctionPoints: Waiting anr process
 * EnvConditions: NA
 * CaseDescription: create a new exception process, call SendANRProcessID func
 * click waiting button, do not kill the new process
 */
HWTEST_F(AbilityManagerServiceAnrTest, SendANRProcessID_003, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    pid_t pid = -1;
    auto result = abilityMs_->SendANRProcessID(pid);
    sleep(6);
    EXPECT_TRUE(result == ERR_INVALID_VALUE);
}
}
}
