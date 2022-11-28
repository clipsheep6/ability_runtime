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
#include "connection_observer_errors.h"
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
const int32_t USER_ID_INVALID = -1;
const int32_t USER_ID_U0 = 0;
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
    sptr<Token> MockToken();

    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

public:
    inline static std::shared_ptr<AbilityManagerService> abilityMs_{ nullptr };
    AbilityRequest abilityRequest_{};
    Want want_{};
};

sptr<Token> AbilityManagerServiceTest::MockToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

AbilityRequest AbilityManagerServiceTest::GenerateAbilityRequest(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
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

    abilityRequest_.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    EXPECT_EQ(ERR_WRONG_INTERFACE_CALL, abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, USER_ID_U100,
        AppExecFwk::ExtensionAbilityType::FORM));

    abilityRequest_.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::DATASHARE;
    EXPECT_EQ(ERR_WRONG_INTERFACE_CALL, abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, USER_ID_U100,
        AppExecFwk::ExtensionAbilityType::FORM));

    abilityRequest_.abilityInfo.applicationInfo.accessTokenId = -1;
    EXPECT_EQ(ERR_WRONG_INTERFACE_CALL, abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, USER_ID_U100,
        AppExecFwk::ExtensionAbilityType::FORM));

    HILOG_INFO("AbilityManagerServiceTest CheckOptExtensionAbility_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckOptExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckOptExtensionAbility
 */
HWTEST_F(AbilityManagerServiceTest, CheckOptExtensionAbility_003, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckOptExtensionAbility_003 start");
    abilityRequest_.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    auto result = abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, USER_ID_U100,
        AppExecFwk::ExtensionAbilityType::FORM);
    EXPECT_EQ(ERR_WRONG_INTERFACE_CALL, result);
    HILOG_INFO("AbilityManagerServiceTest CheckOptExtensionAbility_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckOptExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckOptExtensionAbility
 */
HWTEST_F(AbilityManagerServiceTest, CheckOptExtensionAbility_004, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckOptExtensionAbility_004 start");
    abilityRequest_.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::DATASHARE;
    auto result = abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, USER_ID_U100,
        AppExecFwk::ExtensionAbilityType::FORM);
    EXPECT_EQ(ERR_WRONG_INTERFACE_CALL, result);
    HILOG_INFO("AbilityManagerServiceTest CheckOptExtensionAbility_004 end");
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
    EXPECT_EQ(abilityMs_->CheckCallOtherExtensionPermission(abilityRequest_), CHECK_PERMISSION_FAILED);
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
    EXPECT_EQ(abilityMs_->CheckCallAbilityPermission(abilityRequest_), ABILITY_VISIBLE_FALSE_DENY_REQUEST);
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

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 */
HWTEST_F(AbilityManagerServiceTest, StartAbility_001, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t userId = USER_ID_U100;
    int requestCode = 0;
    EXPECT_EQ(abilityMs_->StartAbility(want, callerToken, userId, requestCode), CHECK_PERMISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 */
HWTEST_F(AbilityManagerServiceTest, StartAbility_002, TestSize.Level1)
{
    Want want;
    AbilityStartSetting abilityStartSetting;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 0;
    EXPECT_EQ(abilityMs_->StartAbility(want, abilityStartSetting, callerToken, USER_ID_U100, requestCode),
        CHECK_PERMISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: IsBackgroundTaskUid
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsBackgroundTaskUid
 */
HWTEST_F(AbilityManagerServiceTest, IsBackgroundTaskUid_001, TestSize.Level1)
{
    int uid = USER_ID_U100;
    EXPECT_FALSE(abilityMs_->IsBackgroundTaskUid(uid));
}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartExtensionAbility
 */
HWTEST_F(AbilityManagerServiceTest, StartExtensionAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest StartExtensionAbility_001 start");
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    abilityMs_->userController_ = std::make_shared<UserController>();
    EXPECT_EQ(abilityMs_->StartExtensionAbility(want, callerToken, USER_ID_U0,
        AppExecFwk::ExtensionAbilityType::SERVICE), ERR_IMPLICIT_START_ABILITY_FAIL);

    callerToken = MockToken();
    EXPECT_EQ(abilityMs_->StartExtensionAbility(want, callerToken, USER_ID_U100,
        AppExecFwk::ExtensionAbilityType::SERVICE), CHECK_PERMISSION_FAILED);

    EXPECT_EQ(abilityMs_->StartExtensionAbility(want, callerToken, USER_ID_INVALID,
        AppExecFwk::ExtensionAbilityType::SERVICE), ERR_INVALID_CALLER);

    callerToken = nullptr;
    EXPECT_EQ(abilityMs_->StartExtensionAbility(want, callerToken, USER_ID_U100,
        AppExecFwk::ExtensionAbilityType::SERVICE), CHECK_PERMISSION_FAILED);

    HILOG_INFO("AbilityManagerServiceTest StartExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbility
 */
HWTEST_F(AbilityManagerServiceTest, StopExtensionAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest StopExtensionAbility_001 start");
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, callerToken, USER_ID_U100,
        AppExecFwk::ExtensionAbilityType::SERVICE), CHECK_PERMISSION_FAILED);

    callerToken = MockToken();
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, callerToken, USER_ID_U100,
        AppExecFwk::ExtensionAbilityType::SERVICE), CHECK_PERMISSION_FAILED);

    HILOG_INFO("AbilityManagerServiceTest StopExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GrantUriPermission
 */
HWTEST_F(AbilityManagerServiceTest, GrantUriPermission_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GrantUriPermission_001 start");
    Want want;
    int32_t validUserId = 101;
    uint32_t targetTokenId = 1;
    abilityMs_->GrantUriPermission(want, validUserId, targetTokenId);
    HILOG_INFO("AbilityManagerServiceTest GrantUriPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbility
 */
HWTEST_F(AbilityManagerServiceTest, TerminateAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest TerminateAbility_001 start");
    sptr<IRemoteObject> token = nullptr;
    int resultCode = 1;
    Want* resultWant = nullptr;
    EXPECT_EQ(abilityMs_->TerminateAbility(token, resultCode, resultWant), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->TerminateAbility(MockToken(), resultCode, resultWant), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest TerminateAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CloseAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CloseAbility
 */
HWTEST_F(AbilityManagerServiceTest, CloseAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CloseAbility_001 start");
    sptr<IRemoteObject> token = nullptr;
    int resultCode = 1;
    Want* resultWant = nullptr;
    EXPECT_EQ(abilityMs_->CloseAbility(token, resultCode, resultWant), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->CloseAbility(MockToken(), resultCode, resultWant), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest CloseAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbilityWithFlag
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityWithFlag
 */
HWTEST_F(AbilityManagerServiceTest, TerminateAbilityWithFlag_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest TerminateAbilityWithFlag_001 start");
    Want* resultWant = nullptr;
    EXPECT_EQ(abilityMs_->TerminateAbilityWithFlag(nullptr, 1, resultWant, true), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->TerminateAbilityWithFlag(MockToken(), 1, resultWant, true), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest TerminateAbilityWithFlag_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SendResultToAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendResultToAbility
 */
HWTEST_F(AbilityManagerServiceTest, SendResultToAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest SendResultToAbility_001 start");
    int32_t requestCode = 1;
    int32_t resultCode = 1;
    Want resultWant;
    EXPECT_EQ(abilityMs_->SendResultToAbility(requestCode, resultCode, resultWant), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest SendResultToAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartRemoteAbility
 */
HWTEST_F(AbilityManagerServiceTest, StartRemoteAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest StartRemoteAbility_001 start");
    Want want;
    EXPECT_EQ(abilityMs_->StartRemoteAbility(want, 1, 1, nullptr), DMS_PERMISSION_DENIED);
    EXPECT_EQ(abilityMs_->StartRemoteAbility(want, 1, 1, MockToken()), DMS_PERMISSION_DENIED);
    HILOG_INFO("AbilityManagerServiceTest StartRemoteAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckIsRemote
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckIsRemote
 */
HWTEST_F(AbilityManagerServiceTest, CheckIsRemote_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckIsRemote_001 start");
    EXPECT_FALSE(abilityMs_->CheckIsRemote(""));
    std::string deviceId = "test";
    EXPECT_FALSE(abilityMs_->CheckIsRemote(deviceId));
    HILOG_INFO("AbilityManagerServiceTest CheckIsRemote_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckIfOperateRemote
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckIfOperateRemote
 */
HWTEST_F(AbilityManagerServiceTest, CheckIfOperateRemote_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckIfOperateRemote_001 start");
    Want want;
    EXPECT_FALSE(abilityMs_->CheckIfOperateRemote(want));
    HILOG_INFO("AbilityManagerServiceTest CheckIfOperateRemote_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbilityByCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityByCaller
 */
HWTEST_F(AbilityManagerServiceTest, TerminateAbilityByCaller_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest TerminateAbilityByCaller_001 start");
    EXPECT_EQ(abilityMs_->TerminateAbilityByCaller(nullptr, 1), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->TerminateAbilityByCaller(MockToken(), 1), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest TerminateAbilityByCaller_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeAbility
 */
HWTEST_F(AbilityManagerServiceTest, MinimizeAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest MinimizeAbility_001 start");
    EXPECT_EQ(abilityMs_->MinimizeAbility(nullptr, true), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->MinimizeAbility(MockToken(), true), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest MinimizeAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 */
HWTEST_F(AbilityManagerServiceTest, ConnectAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ConnectAbility_001 start");
    Want want;
    EXPECT_EQ(abilityMs_->ConnectAbility(want, nullptr, nullptr, 100), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest ConnectAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbilityCommon
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbilityCommon
 */
HWTEST_F(AbilityManagerServiceTest, ConnectAbilityCommon_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ConnectAbilityCommon_001 start");
    Want want;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::SERVICE;
    EXPECT_EQ(abilityMs_->ConnectAbilityCommon(want, nullptr, nullptr, extensionType, 100), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest ConnectAbilityCommon_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectAbility
 */
HWTEST_F(AbilityManagerServiceTest, DisconnectAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest DisconnectAbility_001 start");
    EXPECT_EQ(abilityMs_->DisconnectAbility(nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest DisconnectAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectLocalAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectLocalAbility
 */
HWTEST_F(AbilityManagerServiceTest, ConnectLocalAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ConnectLocalAbility_001 start");
    Want want;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::SERVICE;
    EXPECT_EQ(abilityMs_->ConnectLocalAbility(want, 0, nullptr, nullptr, extensionType), RESOLVE_ABILITY_ERR);
    HILOG_INFO("AbilityManagerServiceTest ConnectLocalAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectRemoteAbility
 */
HWTEST_F(AbilityManagerServiceTest, ConnectRemoteAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ConnectRemoteAbility_001 start");
    Want want;
    EXPECT_EQ(abilityMs_->ConnectRemoteAbility(want, nullptr, nullptr), ERR_NULL_OBJECT);
    HILOG_INFO("AbilityManagerServiceTest ConnectRemoteAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectRemoteAbility
 */
HWTEST_F(AbilityManagerServiceTest, DisconnectRemoteAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest DisconnectRemoteAbility_001 start");
    EXPECT_EQ(abilityMs_->DisconnectRemoteAbility(nullptr), ERR_NULL_OBJECT);
    HILOG_INFO("AbilityManagerServiceTest DisconnectRemoteAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueMission
 */
HWTEST_F(AbilityManagerServiceTest, ContinueMission_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ContinueMission_001 start");
    std::string srcDeviceId = "test";
    std::string dstDeviceId = "test";
    AAFwk::WantParams wantParams;
    EXPECT_EQ(abilityMs_->ContinueMission(srcDeviceId, dstDeviceId, 1, nullptr, wantParams),
        CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest ContinueMission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueAbility
 */
HWTEST_F(AbilityManagerServiceTest, ContinueAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ContinueAbility_001 start");
    std::string deviceId = "test";
    EXPECT_EQ(abilityMs_->ContinueAbility(deviceId, 1, 1), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest ContinueAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartContinuation
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartContinuation
 */
HWTEST_F(AbilityManagerServiceTest, StartContinuation_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest StartContinuation_001 start");
    Want want;
    EXPECT_EQ(abilityMs_->StartContinuation(want, nullptr, 1), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest StartContinuation_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyCompleteContinuation
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyCompleteContinuation
 */
HWTEST_F(AbilityManagerServiceTest, NotifyCompleteContinuation_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest NotifyCompleteContinuation_001 start");
    std::string deviceId = "test";
    abilityMs_->NotifyCompleteContinuation(deviceId, 1, true);
    HILOG_INFO("AbilityManagerServiceTest NotifyCompleteContinuation_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyContinuationResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyContinuationResult
 */
HWTEST_F(AbilityManagerServiceTest, NotifyContinuationResult_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest NotifyContinuationResult_001 start");
    EXPECT_EQ(abilityMs_->NotifyContinuationResult(1, 1), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest NotifyContinuationResult_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartSyncRemoteMissions
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartSyncRemoteMissions
 */
HWTEST_F(AbilityManagerServiceTest, StartSyncRemoteMissions_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest StartSyncRemoteMissions_001 start");
    std::string devId = "test";
    EXPECT_EQ(abilityMs_->StartSyncRemoteMissions(devId, true, 1), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest StartSyncRemoteMissions_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopSyncRemoteMissions
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopSyncRemoteMissions
 */
HWTEST_F(AbilityManagerServiceTest, StopSyncRemoteMissions_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest StopSyncRemoteMissions_001 start");
    std::string devId = "test";
    EXPECT_EQ(abilityMs_->StopSyncRemoteMissions(devId), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest StopSyncRemoteMissions_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterObserver
 */
HWTEST_F(AbilityManagerServiceTest, RegisterObserver_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest RegisterObserver_001 start");
    EXPECT_EQ(abilityMs_->RegisterObserver(nullptr), AbilityRuntime::ERR_INVALID_OBSERVER);
    HILOG_INFO("AbilityManagerServiceTest RegisterObserver_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnregisterObserver
 */
HWTEST_F(AbilityManagerServiceTest, UnregisterObserver_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest UnregisterObserver_001 start");
    EXPECT_EQ(abilityMs_->UnregisterObserver(nullptr), 0);
    HILOG_INFO("AbilityManagerServiceTest UnregisterObserver_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDlpConnectionInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDlpConnectionInfos
 */
HWTEST_F(AbilityManagerServiceTest, GetDlpConnectionInfos_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetDlpConnectionInfos_001 start");
    std::vector<AbilityRuntime::DlpConnectionInfo> infos;
    EXPECT_EQ(abilityMs_->GetDlpConnectionInfos(infos), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest GetDlpConnectionInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterMissionListener
 */
HWTEST_F(AbilityManagerServiceTest, RegisterMissionListener_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest RegisterMissionListener_001 start");
    std::string deviceId = "test";
    EXPECT_EQ(abilityMs_->RegisterMissionListener(deviceId, nullptr), REGISTER_REMOTE_MISSION_LISTENER_FAIL);
    HILOG_INFO("AbilityManagerServiceTest RegisterMissionListener_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UnRegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnRegisterMissionListener
 */
HWTEST_F(AbilityManagerServiceTest, UnRegisterMissionListener_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest UnRegisterMissionListener_001 start");
    std::string deviceId = "test";
    EXPECT_EQ(abilityMs_->UnRegisterMissionListener(deviceId, nullptr), REGISTER_REMOTE_MISSION_LISTENER_FAIL);
    HILOG_INFO("AbilityManagerServiceTest UnRegisterMissionListener_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RemoveAllServiceRecord
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RemoveAllServiceRecord
 */
HWTEST_F(AbilityManagerServiceTest, RemoveAllServiceRecord_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest RemoveAllServiceRecord_001 start");
    abilityMs_->RemoveAllServiceRecord();
    HILOG_INFO("AbilityManagerServiceTest RemoveAllServiceRecord_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetWantSender
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSender
 */
HWTEST_F(AbilityManagerServiceTest, GetWantSender_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetWantSender_001 start");
    WantSenderInfo wantSenderInfo;
    EXPECT_EQ(abilityMs_->GetWantSender(wantSenderInfo, nullptr), nullptr);
    HILOG_INFO("AbilityManagerServiceTest GetWantSender_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SendWantSender
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendWantSender
 */
HWTEST_F(AbilityManagerServiceTest, SendWantSender_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest SendWantSender_001 start");
    SenderInfo senderInfo;
    EXPECT_EQ(abilityMs_->SendWantSender(nullptr, senderInfo), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest SendWantSender_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CancelWantSender
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CancelWantSender
 */
HWTEST_F(AbilityManagerServiceTest, CancelWantSender_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CancelWantSender_001 start");
    abilityMs_->CancelWantSender(nullptr);
    HILOG_INFO("AbilityManagerServiceTest CancelWantSender_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantUid
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantUid
 */
HWTEST_F(AbilityManagerServiceTest, GetPendingWantUid_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetPendingWantUid_001 start");
    EXPECT_EQ(abilityMs_->GetPendingWantUid(nullptr), -1);
    HILOG_INFO("AbilityManagerServiceTest GetPendingWantUid_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantUserId
 */
HWTEST_F(AbilityManagerServiceTest, GetPendingWantUserId_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetPendingWantUserId_001 start");
    EXPECT_EQ(abilityMs_->GetPendingWantUserId(nullptr), -1);
    HILOG_INFO("AbilityManagerServiceTest GetPendingWantUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantBundleName
 */
HWTEST_F(AbilityManagerServiceTest, GetPendingWantBundleName_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetPendingWantBundleName_001 start");
    EXPECT_EQ(abilityMs_->GetPendingWantBundleName(nullptr), "");
    HILOG_INFO("AbilityManagerServiceTest GetPendingWantBundleName_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantCode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantCode
 */
HWTEST_F(AbilityManagerServiceTest, GetPendingWantCode_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetPendingWantCode_001 start");
    EXPECT_EQ(abilityMs_->GetPendingWantCode(nullptr), -1);
    HILOG_INFO("AbilityManagerServiceTest GetPendingWantCode_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantType
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantType
 */
HWTEST_F(AbilityManagerServiceTest, GetPendingWantType_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetPendingWantType_001 start");
    EXPECT_EQ(abilityMs_->GetPendingWantType(nullptr), -1);
    HILOG_INFO("AbilityManagerServiceTest GetPendingWantType_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterCancelListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterCancelListener
 */
HWTEST_F(AbilityManagerServiceTest, RegisterCancelListener_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest RegisterCancelListener_001 start");
    abilityMs_->RegisterCancelListener(nullptr, nullptr);
    HILOG_INFO("AbilityManagerServiceTest RegisterCancelListener_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UnregisterCancelListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnregisterCancelListener
 */
HWTEST_F(AbilityManagerServiceTest, UnregisterCancelListener_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest UnregisterCancelListener_001 start");
    abilityMs_->UnregisterCancelListener(nullptr, nullptr);
    HILOG_INFO("AbilityManagerServiceTest UnregisterCancelListener_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingRequestWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingRequestWant
 */
HWTEST_F(AbilityManagerServiceTest, GetPendingRequestWant_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetPendingRequestWant_001 start");
    std::shared_ptr<Want> want;
    EXPECT_EQ(abilityMs_->GetPendingRequestWant(nullptr, want), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest GetPendingRequestWant_001 end");
}
}  // namespace AAFwk
}  // namespace OHOS
