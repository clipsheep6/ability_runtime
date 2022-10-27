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
#include "ability_connect_manager.h"
#include "ams_configuration_parameter.h"
#undef private
#undef protected

#include "app_process_data.h"
#include "system_ability_definition.h"
#include "ability_manager_errors.h"
#include "hilog_wrapper.h"
#include "sa_mgr_client.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID_U100 = 100;
const std::string COMPONENT_STARTUP_NEW_RULES = "component.startup.newRules";
}  // namespace
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
class AbilityManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    AbilityRequest GenerateAbilityRequest(const std::string &deviceName, const std::string &abilityName,
        const std::string &appName, const std::string &bundleName, const std::string &moduleName);

public:
    inline static std::shared_ptr<AbilityManagerService> abilityMs_ {nullptr};
    AbilityRequest abilityRequest_ {};
    Want want_ {};
};

AbilityRequest AbilityManagerServiceTest::GenerateAbilityRequest(const std::string &deviceName,
    const std::string &abilityName, const std::string &appName, const std::string &bundleName,
    const std::string &moduleName)
{
    ElementName element(deviceName, bundleName, abilityName, moduleName);
    want_.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    ApplicationInfo appinfo;
    appinfo.name = appName;
    appinfo.bundleName = bundleName;
    abilityInfo.applicationInfo = appinfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want_;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    return abilityRequest;
}

void AbilityManagerServiceTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceTest SetUpTestCase called";
    abilityMs_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    abilityMs_->OnStart();
    WaitUntilTaskFinished();
}

void AbilityManagerServiceTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceTest TearDownTestCase called";
    abilityMs_->OnStop();
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
    OHOS::DelayedSingleton<AbilityManagerService>::DestroyInstance();
}

void AbilityManagerServiceTest::SetUp()
{
    // generate ability request
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    abilityRequest_ = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
}

void AbilityManagerServiceTest::TearDown()
{}

/*
 * Feature: AbilityManagerService
 * Function: CheckOptExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckOptExtensionAbility
 */
HWTEST_F(AbilityManagerServiceTest, CheckOptExtensionAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckOptExtensionAbility_001 start");
    abilityRequest_.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    auto result = abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, USER_ID_U100,
        AppExecFwk::ExtensionAbilityType::SERVICE);
    EXPECT_EQ(OHOS::ERR_OK, result);
    HILOG_INFO("AbilityManagerServiceTest CheckOptExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckOptExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckOptExtensionAbility
 */
HWTEST_F(AbilityManagerServiceTest, CheckOptExtensionAbility_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckOptExtensionAbility_002 start");
    abilityRequest_.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::FORM;
    auto result = abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, USER_ID_U100,
        AppExecFwk::ExtensionAbilityType::FORM);
    EXPECT_EQ(CHECK_PERMISSION_FAILED, result);
    HILOG_INFO("AbilityManagerServiceTest CheckOptExtensionAbility_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByCall
 */
HWTEST_F(AbilityManagerServiceTest, StartAbilityByCall_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest StartAbilityByCall_001 start");
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IAbilityConnection> connect = nullptr;
    EXPECT_EQ(abilityMs_->StartAbilityByCall(want, connect, callerToken), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest StartAbilityByCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUser
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUser
 */
HWTEST_F(AbilityManagerServiceTest, StartUser_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest StartUser_001 start");
    EXPECT_EQ(abilityMs_->StartUser(USER_ID_U100), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest StartUser_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopUser
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopUser
 */
HWTEST_F(AbilityManagerServiceTest, StopUser_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest StopUser_001 start");
    EXPECT_EQ(abilityMs_->StopUser(USER_ID_U100, nullptr), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest StopUser_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallServicePermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallServicePermission
 */
HWTEST_F(AbilityManagerServiceTest, CheckCallServicePermission_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckCallServicePermission_001 start");
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(abilityRequest_), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest CheckCallServicePermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallDataAbilityPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallDataAbilityPermission
 */
HWTEST_F(AbilityManagerServiceTest, CheckCallDataAbilityPermission_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckCallDataAbilityPermission_001 start");
    abilityRequest_.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    EXPECT_EQ(abilityMs_->CheckCallDataAbilityPermission(abilityRequest_), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest CheckCallDataAbilityPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallDataAbilityPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallDataAbilityPermission
 */
HWTEST_F(AbilityManagerServiceTest, CheckCallDataAbilityPermission_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckCallDataAbilityPermission_002 start");
    SetParameter(COMPONENT_STARTUP_NEW_RULES.c_str(), "true");
    abilityRequest_.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityMs_->Init();
    EXPECT_EQ(abilityMs_->CheckCallDataAbilityPermission(abilityRequest_), CHECK_PERMISSION_FAILED);
    SetParameter(COMPONENT_STARTUP_NEW_RULES.c_str(), "false");
    HILOG_INFO("AbilityManagerServiceTest CheckCallDataAbilityPermission_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallOtherExtensionPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallOtherExtensionPermission
 */
HWTEST_F(AbilityManagerServiceTest, CheckCallOtherExtensionPermission_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckCallOtherExtensionPermission_001 start");
    bool oldFlag = abilityMs_->startUpNewRule_;
    abilityMs_->startUpNewRule_ = true;
    EXPECT_EQ(abilityMs_->CheckCallOtherExtensionPermission(abilityRequest_), ERR_OK);
    abilityMs_->startUpNewRule_ = oldFlag;
    HILOG_INFO("AbilityManagerServiceTest CheckCallOtherExtensionPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallOtherExtensionPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallOtherExtensionPermission
 */
HWTEST_F(AbilityManagerServiceTest, CheckCallOtherExtensionPermission_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckCallOtherExtensionPermission_002 start");
    abilityRequest_.abilityInfo.visible = false;
    bool oldFlag = abilityMs_->startUpNewRule_;
    abilityMs_->startUpNewRule_ = true;
    EXPECT_EQ(abilityMs_->CheckCallOtherExtensionPermission(abilityRequest_), CHECK_PERMISSION_FAILED);
    abilityMs_->startUpNewRule_ = oldFlag;
    HILOG_INFO("AbilityManagerServiceTest CheckCallOtherExtensionPermission_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallAbilityPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallAbilityPermission
 */
HWTEST_F(AbilityManagerServiceTest, CheckCallAbilityPermission_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckCallAbilityPermission_001 start");
    bool oldFlag = abilityMs_->startUpNewRule_;
    abilityMs_->startUpNewRule_ = true;
    EXPECT_EQ(abilityMs_->CheckCallAbilityPermission(abilityRequest_), ERR_OK);
    abilityMs_->startUpNewRule_ = oldFlag;
    HILOG_INFO("AbilityManagerServiceTest CheckCallAbilityPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallAbilityPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallAbilityPermission
 */
HWTEST_F(AbilityManagerServiceTest, CheckCallAbilityPermission_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckCallAbilityPermission_002 start");
    bool oldFlag = abilityMs_->startUpNewRule_;
    abilityMs_->startUpNewRule_ = false;
    EXPECT_EQ(abilityMs_->CheckCallAbilityPermission(abilityRequest_), ERR_OK);
    abilityMs_->startUpNewRule_ = oldFlag;
    HILOG_INFO("AbilityManagerServiceTest CheckCallAbilityPermission_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallAbilityPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallAbilityPermission
 */
HWTEST_F(AbilityManagerServiceTest, CheckCallAbilityPermission_003, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckCallAbilityPermission_003 start");
    abilityRequest_.abilityInfo.visible = false;
    bool oldFlag = abilityMs_->startUpNewRule_;
    abilityMs_->startUpNewRule_ = true;
    EXPECT_EQ(abilityMs_->CheckCallAbilityPermission(abilityRequest_), CHECK_PERMISSION_FAILED);
    abilityMs_->startUpNewRule_ = oldFlag;
    HILOG_INFO("AbilityManagerServiceTest CheckCallAbilityPermission_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallAbilityPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallAbilityPermission
 */
HWTEST_F(AbilityManagerServiceTest, CheckCallAbilityPermission_004, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckCallAbilityPermission_004 start");
    EXPECT_EQ(abilityMs_->CheckCallAbilityPermission(abilityRequest_), ERR_OK);
    HILOG_INFO("AbilityManagerServiceTest CheckCallAbilityPermission_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckStartByCallPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckStartByCallPermission
 */
HWTEST_F(AbilityManagerServiceTest, CheckStartByCallPermission_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckStartByCallPermission_001 start");
    EXPECT_EQ(abilityMs_->CheckStartByCallPermission(abilityRequest_), RESOLVE_CALL_ABILITY_TYPE_ERR);
    HILOG_INFO("AbilityManagerServiceTest CheckStartByCallPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckStartByCallPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckStartByCallPermission
 */
HWTEST_F(AbilityManagerServiceTest, CheckStartByCallPermission_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckStartByCallPermission_002 start");
    abilityRequest_.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest_.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    EXPECT_EQ(abilityMs_->CheckStartByCallPermission(abilityRequest_), RESOLVE_CALL_NO_PERMISSIONS);
    
    abilityRequest_.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest_.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    EXPECT_EQ(abilityMs_->CheckStartByCallPermission(abilityRequest_), RESOLVE_CALL_ABILITY_TYPE_ERR);
    
    abilityRequest_.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest_.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    EXPECT_EQ(abilityMs_->CheckStartByCallPermission(abilityRequest_), RESOLVE_CALL_ABILITY_TYPE_ERR);
    
    abilityRequest_.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest_.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    EXPECT_EQ(abilityMs_->CheckStartByCallPermission(abilityRequest_), RESOLVE_CALL_ABILITY_TYPE_ERR);
    HILOG_INFO("AbilityManagerServiceTest CheckStartByCallPermission_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallerPermissionOldRule
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallerPermissionOldRule
 */
HWTEST_F(AbilityManagerServiceTest, CheckCallerPermissionOldRule_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckCallerPermissionOldRule_001 start");
    EXPECT_EQ(abilityMs_->CheckCallerPermissionOldRule(abilityRequest_, true), RESOLVE_CALL_NO_PERMISSIONS);
    EXPECT_EQ(abilityMs_->CheckCallerPermissionOldRule(abilityRequest_, false), ERR_OK);
    HILOG_INFO("AbilityManagerServiceTest CheckCallerPermissionOldRule_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallerPermissionOldRule
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallerPermissionOldRule
 */
HWTEST_F(AbilityManagerServiceTest, CheckCallerPermissionOldRule_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckCallerPermissionOldRule_002 start");
    abilityRequest_.callerUid = USER_ID_U100;
    EXPECT_EQ(abilityMs_->CheckCallerPermissionOldRule(abilityRequest_, true), RESOLVE_CALL_NO_PERMISSIONS);
    HILOG_INFO("AbilityManagerServiceTest CheckCallerPermissionOldRule_002 end");
}
}  // namespace AAFwk
}  // namespace OHOS
