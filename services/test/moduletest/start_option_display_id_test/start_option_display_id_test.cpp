/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "ams_configuration_parameter.h"
#include "ability_stack_manager.h"
#undef private
#undef protected

#include "app_process_data.h"
#include "system_ability_definition.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#include "bundlemgr/mock_bundle_manager.h"
#include "sa_mgr_client.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "os_account_manager.h"
#include "os_account_info.h"
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AccountSA;
namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID_U100 = 100;
constexpr int32_t DISPLAY_ID = 256;

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
}  // namespace

class StartOptionDisplayIdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

public:
    std::shared_ptr<AbilityManagerService> abilityMgrServ_ {nullptr};
};

void StartOptionDisplayIdTest::SetUpTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void StartOptionDisplayIdTest::TearDownTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void StartOptionDisplayIdTest::SetUp()
{
    abilityMgrServ_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    abilityMgrServ_->OnStart();
    WaitUntilTaskFinished();

    abilityMgrServ_->StartUser(USER_ID_U100);
    auto topAbility = abilityMgrServ_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
}

void StartOptionDisplayIdTest::TearDown()
{
    abilityMgrServ_->OnStop();
    OHOS::DelayedSingleton<AbilityManagerService>::DestroyInstance();
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Set displayId, enable ability, get displayId value consistent with the setting
 */
HWTEST_F(StartOptionDisplayIdTest, start_option_001, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    StartOptions option;
    option.SetDisplayID(DISPLAY_ID);
    auto result = abilityMgrServ_->StartAbility(want, option, nullptr, 100, 0);
    WaitUntilTaskFinished();
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto topAbility = abilityMgrServ_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    EXPECT_TRUE(topAbility);
    if (topAbility) {
        auto defualtDisplayId = 0;
        auto displayId = topAbility->GetWant().GetIntParam(Want::PARAM_RESV_DISPLAY_ID, defualtDisplayId);
        EXPECT_EQ(displayId, DISPLAY_ID);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
