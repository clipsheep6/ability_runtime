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
#include "ability_manager_service.h"
#include "system_ability_definition.h"
#include "bundlemgr/mock_bundle_manager.h"
#include "sa_mgr_client.h"
#include "parameter.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string TASK_NAME_START_SYSTEM_APP = "StartSystemApplication";
const std::string TASK_NAME_SUBSCRIBE_BACKGROUND_TASK = "SubscribeBackgroundTask";
const std::string TASK_NAME_START_RESIDENT_APPS = "StartResidentApps";
const std::string TASK_NAME_INIT_STARTUP_FLAG = "InitStartupFlag";
}

class AbilityServiceStartTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

public:
    std::shared_ptr<AbilityManagerService> aams_ {nullptr};
};

void AbilityServiceStartTest::SetUpTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}
void AbilityServiceStartTest::TearDownTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void AbilityServiceStartTest::SetUp()
{
    aams_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
}

void AbilityServiceStartTest::TearDown()
{
    aams_->OnStop();
}

/*
 * Feature: AbilityManagerService
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: AbilityManager startup & stop
 * EnvConditions: NA
 * CaseDescription: Verify if AbilityManagerService startup & stop successfully.
 */
HWTEST_F(AbilityServiceStartTest, StartUp_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ability_manager_service_startup_001 start";
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START, aams_->QueryServiceState());
    aams_->OnStart();
    auto handler = aams_->GetEventHandler();
    ASSERT_NE(handler, nullptr);
    handler->RemoveTask(TASK_NAME_START_SYSTEM_APP);
    handler->RemoveTask(TASK_NAME_SUBSCRIBE_BACKGROUND_TASK);
    handler->RemoveTask(TASK_NAME_START_RESIDENT_APPS);
    handler->RemoveTask(TASK_NAME_INIT_STARTUP_FLAG);

    EXPECT_EQ(ServiceRunningState::STATE_RUNNING, aams_->QueryServiceState());
    aams_->OnStop();
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START, aams_->QueryServiceState());
    GTEST_LOG_(INFO) << "ability_manager_service_startup_001 end";
}

/*
 * Feature: AbilityManagerService
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: AbilityManager startup two times
 * EnvConditions: NA
 * CaseDescription: Verify if AbilityManagerService startup & stop successfully.
 */
HWTEST_F(AbilityServiceStartTest, StartUp_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ability_manager_service_startup_002 start";
    aams_->OnStart();
    auto handler = aams_->GetEventHandler();
    ASSERT_NE(handler, nullptr);
    handler->RemoveTask(TASK_NAME_START_SYSTEM_APP);
    handler->RemoveTask(TASK_NAME_SUBSCRIBE_BACKGROUND_TASK);
    handler->RemoveTask(TASK_NAME_START_RESIDENT_APPS);
    handler->RemoveTask(TASK_NAME_INIT_STARTUP_FLAG);

    aams_->OnStart();
    handler = aams_->GetEventHandler();
    ASSERT_NE(handler, nullptr);
    handler->RemoveTask(TASK_NAME_START_SYSTEM_APP);
    handler->RemoveTask(TASK_NAME_SUBSCRIBE_BACKGROUND_TASK);
    handler->RemoveTask(TASK_NAME_START_RESIDENT_APPS);
    handler->RemoveTask(TASK_NAME_INIT_STARTUP_FLAG);

    EXPECT_EQ(ServiceRunningState::STATE_RUNNING, aams_->QueryServiceState());
    aams_->OnStop();
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START, aams_->QueryServiceState());
    GTEST_LOG_(INFO) << "ability_manager_service_startup_002 end";
}

/*
 * Feature: AbilityManagerService
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: AbilityManager stop
 * EnvConditions: NA
 * CaseDescription: Verify if AbilityManagerService stop successfully.
 */
HWTEST_F(AbilityServiceStartTest, StartUp_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ability_manager_service_startup_003 start";
    aams_->OnStop();
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START, aams_->QueryServiceState());
    GTEST_LOG_(INFO) << "ability_manager_service_startup_003 end";
}

/*
 * Feature: AbilityManagerService
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: AbilityManager stop again
 * EnvConditions: NA
 * CaseDescription: Verify if AbilityManagerService stop successfully.
 */
HWTEST_F(AbilityServiceStartTest, StartUp_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ability_manager_service_startup_004 start";
    aams_->OnStart();
    auto handler = aams_->GetEventHandler();
    ASSERT_NE(handler, nullptr);
    handler->RemoveTask(TASK_NAME_START_SYSTEM_APP);
    handler->RemoveTask(TASK_NAME_SUBSCRIBE_BACKGROUND_TASK);
    handler->RemoveTask(TASK_NAME_START_RESIDENT_APPS);
    handler->RemoveTask(TASK_NAME_INIT_STARTUP_FLAG);

    aams_->OnStop();
    aams_->OnStop();
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START, aams_->QueryServiceState());
    GTEST_LOG_(INFO) << "ability_manager_service_startup_004 end";
}

/*
 * Feature: AbilityManagerService
 * Function: Service
 * SubFunction: NA
 * FunctionPoints: AbilityManager start & stop 10 times
 * EnvConditions: NA
 * CaseDescription: Verify if AbilityManagerService start & stop successfully.
 */
HWTEST_F(AbilityServiceStartTest, StartUp_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ability_manager_service_startup_005 start";
    for (int i = 0; i < 10; i++) {
        aams_->OnStart();
        auto handler = aams_->GetEventHandler();
        ASSERT_NE(handler, nullptr);
        handler->RemoveTask(TASK_NAME_START_SYSTEM_APP);
        handler->RemoveTask(TASK_NAME_SUBSCRIBE_BACKGROUND_TASK);
        handler->RemoveTask(TASK_NAME_START_RESIDENT_APPS);
        handler->RemoveTask(TASK_NAME_INIT_STARTUP_FLAG);
        GTEST_LOG_(INFO) << "start " << i << "times";
        EXPECT_EQ(ServiceRunningState::STATE_RUNNING, aams_->QueryServiceState());
        aams_->OnStop();
        GTEST_LOG_(INFO) << "stop " << i << "times";
        EXPECT_EQ(ServiceRunningState::STATE_NOT_START, aams_->QueryServiceState());
    }
    GTEST_LOG_(INFO) << "ability_manager_service_startup_005 end";
}

/**
 * @tc.name: AbilityServiceStartTest_StartUpEvent_001
 * @tc.desc: OnStart
 * @tc.type: FUNC
 * @tc.require: issueI5JZEI
 */
HWTEST_F(AbilityServiceStartTest, StartUpEvent_001, TestSize.Level1)
{
    aams_->OnStart();
    const int bufferLen = 128;
    char paramOutBuf[bufferLen] = {0};
    const char *hookMode = "true";
    int ret = GetParameter("bootevent.bootanimation.started", "", paramOutBuf, bufferLen);
    EXPECT_TRUE(strncmp(paramOutBuf, hookMode, strlen(hookMode)) == 0);

    ret = GetParameter("bootevent.appfwk.ready", "", paramOutBuf, bufferLen);
    EXPECT_TRUE(strncmp(paramOutBuf, hookMode, strlen(hookMode)) == 0);
    aams_->OnStop();
}
}  // namespace AAFwk
}  // namespace OHOS
