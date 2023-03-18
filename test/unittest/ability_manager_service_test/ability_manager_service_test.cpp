/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "mock_permission_verification.h"

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
#include "mock_ability_controller.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;
namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID_U100 = 100;
const int32_t APP_MEMORY_SIZE = 512;
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
    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    sptr<Token> MockToken(AbilityType);
    sptr<SessionInfo> MockSessionInfo(uint64_t persistentId);

    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

public:
    inline static std::shared_ptr<AbilityManagerService> abilityMs_{ nullptr };
    AbilityRequest abilityRequest_{};
    Want want_{};
};

std::shared_ptr<AbilityRecord> AbilityManagerServiceTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> AbilityManagerServiceTest::MockToken(AbilityType abilityType)
{
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

sptr<SessionInfo> AbilityManagerServiceTest::MockSessionInfo(uint64_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        HILOG_ERROR("sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
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
    abilityRequest_.abilityInfo.type = AbilityType::PAGE;
    auto ret = abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, 100, ExtensionAbilityType::FORM);
    EXPECT_EQ(ret, ERR_WRONG_INTERFACE_CALL);

    abilityRequest_.abilityInfo.type = AbilityType::EXTENSION;
    abilityRequest_.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    ret = abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, 100, ExtensionAbilityType::FORM);
    EXPECT_EQ(ret, ERR_WRONG_INTERFACE_CALL);

    abilityRequest_.abilityInfo.type = AbilityType::EXTENSION;
    abilityRequest_.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    MyFlag::flag_ = 0;
    abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, 100, ExtensionAbilityType::SERVICE);
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
    abilityRequest_.abilityInfo.type = AbilityType::EXTENSION;
    abilityRequest_.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    ExtensionAbilityType extensionType = ExtensionAbilityType::DATASHARE;
    auto ret = abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, 100, extensionType);
    EXPECT_EQ(ret, ERR_WRONG_INTERFACE_CALL);

    extensionType = ExtensionAbilityType::SERVICE;
    abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, 100, extensionType);

    abilityRequest_.abilityInfo.extensionAbilityType = ExtensionAbilityType::FORM;
    extensionType = ExtensionAbilityType::FORM;
    abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, 100, extensionType);
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
    abilityRequest_.abilityInfo.type = AbilityType::PAGE;
    auto result = abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, USER_ID_U100,
        ExtensionAbilityType::FORM);
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
    abilityRequest_.abilityInfo.extensionAbilityType = ExtensionAbilityType::DATASHARE;
    auto result = abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, USER_ID_U100,
        ExtensionAbilityType::FORM);
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
    EXPECT_EQ(abilityMs_->StartAbilityByCall(want, nullptr, nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest StartAbilityByCall_001 end");
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
    AbilityRequest request;
    abilityMs_->startUpNewRule_ = false;
    EXPECT_FALSE(abilityMs_->startUpNewRule_);
    request.abilityInfo.visible = true;
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(request), ERR_OK);

    abilityMs_->startUpNewRule_ = true;
    request.abilityInfo.isStageBasedModel = false;
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(request), ERR_INVALID_VALUE);

    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(request), ERR_INVALID_VALUE);

    request.abilityInfo.extensionAbilityType = ExtensionAbilityType::DATASHARE;
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(request), ERR_INVALID_VALUE);

    request.abilityInfo.extensionAbilityType = ExtensionAbilityType::FILESHARE;
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(request), CHECK_PERMISSION_FAILED);
    abilityMs_->startUpNewRule_ = false;
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
    AbilityRequest abilityRequest;
    EXPECT_EQ(abilityMs_->CheckCallDataAbilityPermission(abilityRequest), ERR_INVALID_VALUE);
    abilityRequest.abilityInfo.applicationInfo.name = "test";
    EXPECT_EQ(abilityMs_->CheckCallDataAbilityPermission(abilityRequest), ERR_INVALID_VALUE);

    abilityRequest.abilityInfo.applicationInfo.name = "test";
    abilityRequest.abilityInfo.applicationInfo.bundleName = "test";
    abilityRequest.abilityInfo.type = AbilityType::SERVICE;
    EXPECT_EQ(abilityMs_->CheckCallDataAbilityPermission(abilityRequest), ERR_WRONG_INTERFACE_CALL);

    abilityRequest_.abilityInfo.type = AbilityType::DATA;
    abilityMs_->startUpNewRule_ = false;
    EXPECT_EQ(abilityMs_->CheckCallDataAbilityPermission(abilityRequest), ERR_WRONG_INTERFACE_CALL);
    abilityMs_->startUpNewRule_ = true;

    abilityRequest_.abilityInfo.type = AbilityType::DATA;
    EXPECT_EQ(abilityMs_->CheckCallDataAbilityPermission(abilityRequest_), ERR_INVALID_VALUE);
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
    abilityRequest_.abilityInfo.type = AbilityType::DATA;
    abilityMs_->Init();
    EXPECT_EQ(abilityMs_->CheckCallDataAbilityPermission(abilityRequest_), ERR_INVALID_VALUE);
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
    AbilityRequest request;
    EXPECT_TRUE(abilityMs_->startUpNewRule_);
    abilityMs_->startUpNewRule_ = false;
    request.abilityInfo.visible = true;
    EXPECT_EQ(abilityMs_->CheckCallAbilityPermission(abilityRequest_), ERR_OK);

    abilityMs_->startUpNewRule_ = true;
    EXPECT_EQ(abilityMs_->CheckCallAbilityPermission(abilityRequest_), ERR_INVALID_VALUE);

    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->CheckCallAbilityPermission(abilityRequest_), 1);
    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceTest CheckCallAbilityPermission_001 end");
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
    abilityRequest_.abilityInfo.type = AbilityType::PAGE;
    abilityRequest_.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    EXPECT_EQ(abilityMs_->CheckStartByCallPermission(abilityRequest_), ERR_INVALID_VALUE);

    abilityRequest_.abilityInfo.type = AbilityType::PAGE;
    abilityRequest_.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    EXPECT_EQ(abilityMs_->CheckStartByCallPermission(abilityRequest_), RESOLVE_CALL_ABILITY_TYPE_ERR);

    abilityRequest_.abilityInfo.type = AbilityType::DATA;
    abilityRequest_.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    EXPECT_EQ(abilityMs_->CheckStartByCallPermission(abilityRequest_), RESOLVE_CALL_ABILITY_TYPE_ERR);

    abilityRequest_.abilityInfo.type = AbilityType::DATA;
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
    EXPECT_EQ(abilityMs_->StartAbility(want, nullptr, 100, 0), CHECK_PERMISSION_FAILED);

    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    EXPECT_EQ(abilityMs_->StartAbility(want, nullptr, 100, 0), ERR_INVALID_CONTINUATION_FLAG);
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
 * Function: StartAbilityAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCaller
 */
HWTEST_F(AbilityManagerServiceTest, StartAbilityAsCaller_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest StartAbilityAsCaller_001 start");
    Want want;
    int requestCode = 0;
    sptr<IRemoteObject> callerToken = nullptr;
    EXPECT_EQ(abilityMs_->StartAbility(want, callerToken, USER_ID_U100, requestCode), CHECK_PERMISSION_FAILED);

    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    EXPECT_EQ(abilityMs_->StartAbilityAsCaller(want, callerToken, USER_ID_U100, requestCode),
        ERR_INVALID_CONTINUATION_FLAG);
    HILOG_INFO("AbilityManagerServiceTest StartAbilityAsCaller_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCaller
 */
HWTEST_F(AbilityManagerServiceTest, StartAbilityAsCaller_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest StartAbilityAsCaller_002 start");
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 0;
    EXPECT_EQ(abilityMs_->StartAbilityAsCaller(want, startOptions, callerToken, USER_ID_U100, requestCode),
    CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest StartAbilityAsCaller_002 end");
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
    auto ret = abilityMs_->StartExtensionAbility(want, nullptr, 0, ExtensionAbilityType::SERVICE);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    ret = abilityMs_->StartExtensionAbility(want, MockToken(AbilityType::PAGE), 100, ExtensionAbilityType::SERVICE);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    ret = abilityMs_->StartExtensionAbility(want, MockToken(AbilityType::PAGE), -1, ExtensionAbilityType::SERVICE);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    ret = abilityMs_->StartExtensionAbility(want, nullptr, 100, ExtensionAbilityType::SERVICE);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

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
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, nullptr, USER_ID_U100, ExtensionAbilityType::SERVICE),
        CHECK_PERMISSION_FAILED);

    auto callerToken = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, callerToken, USER_ID_U100, ExtensionAbilityType::SERVICE),
        CHECK_PERMISSION_FAILED);

    HILOG_INFO("AbilityManagerServiceTest StopExtensionAbility_001 end");
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
    Want* resultWant = nullptr;
    EXPECT_EQ(abilityMs_->TerminateAbility(nullptr, 1, resultWant), ERR_INVALID_VALUE);
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
    Want* resultWant = nullptr;
    EXPECT_EQ(abilityMs_->CloseAbility(nullptr, 1, resultWant), ERR_INVALID_VALUE);
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
    EXPECT_EQ(abilityMs_->TerminateAbilityWithFlag(MockToken(AbilityType::PAGE), 1, resultWant, true),
        ERR_INVALID_VALUE);
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
    // AddStartControlParam
    EXPECT_EQ(abilityMs_->StartRemoteAbility(want, 1, 1, nullptr), ERR_INVALID_VALUE);
    
    // IsStartFreeInstall
    MyFlag::flag_ = 1;
    unsigned int flag = 0x00000800;
    want.SetFlags(flag);
    auto temp = abilityMs_->freeInstallManager_;
    abilityMs_->freeInstallManager_.reset();
    EXPECT_EQ(abilityMs_->StartRemoteAbility(want, 1, 1, nullptr), ERR_INVALID_VALUE);

    abilityMs_->freeInstallManager_ = temp;
    EXPECT_EQ(abilityMs_->StartRemoteAbility(want, 1, 1, nullptr), INVALID_PARAMETERS_ERR);

    // GetBoolParam
    want.SetFlags(0);
    want.SetParam("ohos.aafwk.param.startAbilityForResult", true);
    EXPECT_EQ(abilityMs_->StartRemoteAbility(want, 1, 1, nullptr), ERR_INVALID_VALUE);
    
    want.SetParam("test", true);
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->StartRemoteAbility(want, 1, 1, callerToken), ERR_INVALID_VALUE);
    MyFlag::flag_ = 0;
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
    EXPECT_EQ(abilityMs_->TerminateAbilityByCaller(MockToken(AbilityType::PAGE), 1), ERR_INVALID_VALUE);
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
    EXPECT_EQ(abilityMs_->MinimizeAbility(MockToken(AbilityType::PAGE), true), ERR_INVALID_VALUE);
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
    ExtensionAbilityType extensionType = ExtensionAbilityType::SERVICE;
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
    ExtensionAbilityType extensionType = ExtensionAbilityType::SERVICE;
    EXPECT_EQ(abilityMs_->ConnectLocalAbility(want, 100, nullptr, nullptr, extensionType), ERR_CROSS_USER);
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
    EXPECT_EQ(abilityMs_->ConnectRemoteAbility(want, nullptr, nullptr), ERR_INVALID_VALUE);

    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->ConnectRemoteAbility(want, nullptr, nullptr), ERR_NULL_OBJECT);
    MyFlag::flag_ = 0;
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

    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->GetDlpConnectionInfos(infos), ERR_OK);

    MyFlag::flag_ = 0;
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
 * Function: RegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterMissionListener
 */
HWTEST_F(AbilityManagerServiceTest, RegisterMissionListener_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest RegisterMissionListener_002 start");
    auto temp_ = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_ = nullptr;
    EXPECT_EQ(abilityMs_->RegisterMissionListener(nullptr), ERR_NO_INIT);
    abilityMs_->currentMissionListManager_ = temp_;

    EXPECT_EQ(abilityMs_->RegisterMissionListener(nullptr), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest RegisterMissionListener_002 end");
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
 * Function: UnRegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnRegisterMissionListener
 */
HWTEST_F(AbilityManagerServiceTest, UnRegisterMissionListener_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest UnRegisterMissionListener_002 start");
    auto temp_ = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_ = nullptr;
    EXPECT_EQ(abilityMs_->UnRegisterMissionListener(nullptr), ERR_NO_INIT);

    abilityMs_->currentMissionListManager_ = temp_;
    EXPECT_EQ(abilityMs_->UnRegisterMissionListener(nullptr), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest UnRegisterMissionListener_002 end");
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
    auto temp = abilityMs_->pendingWantManager_;
    abilityMs_->pendingWantManager_ = nullptr;
    EXPECT_EQ(abilityMs_->GetWantSender(wantSenderInfo, nullptr), nullptr);

    abilityMs_->pendingWantManager_ = temp;
    wantSenderInfo.userId = -1;
    EXPECT_NE(abilityMs_->GetWantSender(wantSenderInfo, nullptr), nullptr);

    wantSenderInfo.userId = 0;
    wantSenderInfo.bundleName = "test";
    EXPECT_NE(abilityMs_->GetWantSender(wantSenderInfo, nullptr), nullptr);
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
    auto temp = abilityMs_->pendingWantManager_;
    abilityMs_->pendingWantManager_.reset();
    EXPECT_EQ(abilityMs_->GetPendingWantUid(nullptr), -1);

    abilityMs_->pendingWantManager_ = temp;
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
    auto temp = abilityMs_->pendingWantManager_;
    abilityMs_->pendingWantManager_.reset();
    EXPECT_EQ(abilityMs_->GetPendingWantUserId(nullptr), -1);

    abilityMs_->pendingWantManager_ = temp;
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
    auto temp = abilityMs_->pendingWantManager_;
    abilityMs_->pendingWantManager_.reset();
    EXPECT_EQ(abilityMs_->GetPendingWantCode(nullptr), -1);

    abilityMs_->pendingWantManager_ = temp;
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
    auto temp = abilityMs_->pendingWantManager_;
    abilityMs_->pendingWantManager_.reset();
    EXPECT_EQ(abilityMs_->GetPendingWantType(nullptr), -1);

    abilityMs_->pendingWantManager_ = temp;
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

/*
 * Feature: AbilityManagerService
 * Function: LockMissionForCleanup
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService LockMissionForCleanup
 */
HWTEST_F(AbilityManagerServiceTest, LockMissionForCleanup_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest LockMissionForCleanup_001 start");
    EXPECT_EQ(abilityMs_->LockMissionForCleanup(1), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest LockMissionForCleanup_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UnlockMissionForCleanup
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnlockMissionForCleanup
 */
HWTEST_F(AbilityManagerServiceTest, UnlockMissionForCleanup_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest UnlockMissionForCleanup_001 start");
    EXPECT_EQ(abilityMs_->UnlockMissionForCleanup(1), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest UnlockMissionForCleanup_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionInfos
 */
HWTEST_F(AbilityManagerServiceTest, GetMissionInfos_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetMissionInfos_001 start");
    std::vector<MissionInfo> missionInfos;
    EXPECT_EQ(abilityMs_->GetMissionInfos("", 10, missionInfos), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest GetMissionInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetRemoteMissionInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetRemoteMissionInfos
 */
HWTEST_F(AbilityManagerServiceTest, GetRemoteMissionInfos_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetRemoteMissionInfos_001 start");
    std::vector<MissionInfo> missionInfos;
    EXPECT_EQ(abilityMs_->GetRemoteMissionInfos("", 10, missionInfos), INVALID_PARAMETERS_ERR);
    HILOG_INFO("AbilityManagerServiceTest GetRemoteMissionInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionInfo
 */
HWTEST_F(AbilityManagerServiceTest, GetMissionInfo_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetMissionInfo_001 start");
    MissionInfo missionInfo;
    EXPECT_EQ(abilityMs_->GetMissionInfo("", 10, missionInfo), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest GetMissionInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetRemoteMissionInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetRemoteMissionInfo
 */
HWTEST_F(AbilityManagerServiceTest, GetRemoteMissionInfo_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetRemoteMissionInfo_001 start");
    MissionInfo missionInfo;
    EXPECT_EQ(abilityMs_->GetRemoteMissionInfo("", 10, missionInfo), INVALID_PARAMETERS_ERR);
    HILOG_INFO("AbilityManagerServiceTest GetRemoteMissionInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CleanMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CleanMission
 */
HWTEST_F(AbilityManagerServiceTest, CleanMission_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CleanMission_001 start");
    EXPECT_EQ(abilityMs_->CleanMission(1), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest CleanMission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToFront
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToFront
 */
HWTEST_F(AbilityManagerServiceTest, MoveMissionToFront_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest MoveMissionToFront_001 start");
    EXPECT_EQ(abilityMs_->MoveMissionToFront(100), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest MoveMissionToFront_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToFront
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToFront
 */
HWTEST_F(AbilityManagerServiceTest, MoveMissionToFront_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest MoveMissionToFront_002 start");
    StartOptions startOptions;
    EXPECT_EQ(abilityMs_->MoveMissionToFront(100, startOptions), CHECK_PERMISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest MoveMissionToFront_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionIdByToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionIdByToken
 */
HWTEST_F(AbilityManagerServiceTest, GetMissionIdByToken_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetMissionIdByToken_001 start");
    EXPECT_EQ(abilityMs_->GetMissionIdByToken(nullptr), -1);
    EXPECT_EQ(abilityMs_->GetMissionIdByToken(MockToken(AbilityType::PAGE)), -1);
    HILOG_INFO("AbilityManagerServiceTest GetMissionIdByToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsAbilityControllerStartById
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsAbilityControllerStartById
 */
HWTEST_F(AbilityManagerServiceTest, IsAbilityControllerStartById_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest IsAbilityControllerStartById_001 start");
    EXPECT_TRUE(abilityMs_->IsAbilityControllerStartById(1));
    HILOG_INFO("AbilityManagerServiceTest IsAbilityControllerStartById_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetServiceRecordByElementName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetServiceRecordByElementName
 */
HWTEST_F(AbilityManagerServiceTest, GetServiceRecordByElementName_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetServiceRecordByElementName_001 start");
    EXPECT_EQ(abilityMs_->GetServiceRecordByElementName(""), nullptr);
    abilityMs_->connectManager_ = std::make_shared<AbilityConnectManager>(100);
    abilityMs_->connectManager_->serviceMap_.insert({"test", MockAbilityRecord(AbilityType::PAGE)});
    EXPECT_NE(abilityMs_->GetServiceRecordByElementName("test"), nullptr);
    HILOG_INFO("AbilityManagerServiceTest GetServiceRecordByElementName_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetConnectRecordListByCallback
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectRecordListByCallback
 */
HWTEST_F(AbilityManagerServiceTest, GetConnectRecordListByCallback_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetConnectRecordListByCallback_001 start");
    auto temp = abilityMs_->connectManager_;
    abilityMs_->connectManager_.reset();
    std::list<std::shared_ptr<ConnectionRecord>> connectList;
    EXPECT_EQ(abilityMs_->GetConnectRecordListByCallback(nullptr), connectList);
    abilityMs_->connectManager_ = temp;
    HILOG_INFO("AbilityManagerServiceTest GetConnectRecordListByCallback_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 */
HWTEST_F(AbilityManagerServiceTest, AcquireDataAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest AcquireDataAbility_001 start");
    Uri uri("test");
    EXPECT_EQ(abilityMs_->AcquireDataAbility(uri, true, nullptr), nullptr);
    HILOG_INFO("AbilityManagerServiceTest AcquireDataAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 */
HWTEST_F(AbilityManagerServiceTest, ReleaseDataAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ReleaseDataAbility_001 start");
    EXPECT_EQ(abilityMs_->ReleaseDataAbility(nullptr, nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest ReleaseDataAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AttachAbilityThread
 */
HWTEST_F(AbilityManagerServiceTest, AttachAbilityThread_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest AttachAbilityThread_001 start");
    EXPECT_EQ(abilityMs_->AttachAbilityThread(nullptr, nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest AttachAbilityThread_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpInner
 */
HWTEST_F(AbilityManagerServiceTest, DumpInner_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest DumpInner_001 start");
    std::vector<std::string> info;
    abilityMs_->DumpInner("", info);
    HILOG_INFO("AbilityManagerServiceTest DumpInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpMissionListInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpMissionListInner
 */
HWTEST_F(AbilityManagerServiceTest, DumpMissionListInner_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest DumpMissionListInner_001 start");
    std::vector<std::string> info;
    abilityMs_->DumpMissionListInner("", info);
    HILOG_INFO("AbilityManagerServiceTest DumpMissionListInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpMissionInfosInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpMissionInfosInner
 */
HWTEST_F(AbilityManagerServiceTest, DumpMissionInfosInner_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest DumpMissionInfosInner_001 start");
    std::vector<std::string> info;
    abilityMs_->DumpMissionInfosInner("", info);
    HILOG_INFO("AbilityManagerServiceTest DumpMissionInfosInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpMissionInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpMissionInner
 */
HWTEST_F(AbilityManagerServiceTest, DumpMissionInner_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest DumpMissionInner_001 start");
    std::vector<std::string> info;
    abilityMs_->DumpMissionInner("", info);
    HILOG_INFO("AbilityManagerServiceTest DumpMissionInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpStateInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpStateInner
 */
HWTEST_F(AbilityManagerServiceTest, DumpStateInner_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest DumpStateInner_001 start");
    std::vector<std::string> info;
    abilityMs_->DumpStateInner("", info);
    HILOG_INFO("AbilityManagerServiceTest DumpStateInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DataDumpStateInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DataDumpStateInner
 */
HWTEST_F(AbilityManagerServiceTest, DataDumpStateInner_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest DataDumpStateInner_001 start");
    std::vector<std::string> info;
    abilityMs_->DataDumpStateInner("", info);
    HILOG_INFO("AbilityManagerServiceTest DataDumpStateInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpState
 */
HWTEST_F(AbilityManagerServiceTest, DumpState_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest DumpState_001 start");
    std::vector<std::string> info;
    abilityMs_->DumpState("", info);
    HILOG_INFO("AbilityManagerServiceTest DumpState_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpSysState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpSysState
 */
HWTEST_F(AbilityManagerServiceTest, DumpSysState_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest DumpSysState_001 start");
    std::vector<std::string> info;
    abilityMs_->DumpSysState("", info, true, true, 100);
    HILOG_INFO("AbilityManagerServiceTest DumpSysState_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AbilityTransitionDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AbilityTransitionDone
 */
HWTEST_F(AbilityManagerServiceTest, AbilityTransitionDone_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest AbilityTransitionDone_001 start");
    PacMap saveData;
    EXPECT_EQ(abilityMs_->AbilityTransitionDone(nullptr, 1, saveData), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest AbilityTransitionDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDone
 */
HWTEST_F(AbilityManagerServiceTest, ScheduleConnectAbilityDone_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ScheduleConnectAbilityDone_001 start");
    EXPECT_EQ(abilityMs_->ScheduleConnectAbilityDone(nullptr, nullptr), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->ScheduleConnectAbilityDone(MockToken(AbilityType::PAGE), nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest ScheduleConnectAbilityDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityDone
 */
HWTEST_F(AbilityManagerServiceTest, ScheduleCommandAbilityDone_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ScheduleCommandAbilityDone_001 start");
    EXPECT_EQ(abilityMs_->ScheduleCommandAbilityDone(nullptr), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->ScheduleCommandAbilityDone(MockToken(AbilityType::PAGE)), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest ScheduleCommandAbilityDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnAbilityRequestDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnAbilityRequestDone
 */
HWTEST_F(AbilityManagerServiceTest, OnAbilityRequestDone_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest OnAbilityRequestDone_001 start");
    abilityMs_->OnAbilityRequestDone(nullptr, 1);
    abilityMs_->OnAbilityRequestDone(MockToken(AbilityType::DATA), 1);
    abilityMs_->OnAbilityRequestDone(MockToken(AbilityType::UNKNOWN), 1);
    HILOG_INFO("AbilityManagerServiceTest OnAbilityRequestDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnAppStateChanged
 */
HWTEST_F(AbilityManagerServiceTest, OnAppStateChanged_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest OnAppStateChanged_001 start");
    AppInfo info;
    abilityMs_->OnAppStateChanged(info);
    HILOG_INFO("AbilityManagerServiceTest OnAppStateChanged_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetEventHandler
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetEventHandler
 */
HWTEST_F(AbilityManagerServiceTest, GetEventHandler_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetEventHandler_001 start");
    EXPECT_NE(abilityMs_->GetEventHandler(), nullptr);
    HILOG_INFO("AbilityManagerServiceTest GetEventHandler_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: InitMissionListManager
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService InitMissionListManager
 */
HWTEST_F(AbilityManagerServiceTest, InitMissionListManager_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest InitMissionListManager_001 start");
    abilityMs_->InitMissionListManager(100, false);
    HILOG_INFO("AbilityManagerServiceTest InitMissionListManager_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetUserId
 */
HWTEST_F(AbilityManagerServiceTest, GetUserId_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetUserId_001 start");
    EXPECT_NE(abilityMs_->GetUserId(), 100);
    HILOG_INFO("AbilityManagerServiceTest GetUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GenerateAbilityRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GenerateAbilityRequest
 */
HWTEST_F(AbilityManagerServiceTest, GenerateAbilityRequest_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GenerateAbilityRequest_001 start");
    Want want;
    AbilityRequest request;
    EXPECT_EQ(abilityMs_->GenerateAbilityRequest(want, 1, request, nullptr, 100), RESOLVE_ABILITY_ERR);
    HILOG_INFO("AbilityManagerServiceTest GenerateAbilityRequest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GenerateExtensionAbilityRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GenerateExtensionAbilityRequest
 */
HWTEST_F(AbilityManagerServiceTest, GenerateExtensionAbilityRequest_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GenerateExtensionAbilityRequest_001 start");
    Want want;
    AbilityRequest request;
    EXPECT_EQ(abilityMs_->GenerateExtensionAbilityRequest(want, request, nullptr, 100), RESOLVE_ABILITY_ERR);
    HILOG_INFO("AbilityManagerServiceTest GenerateExtensionAbilityRequest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbilityResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityResult
 */
HWTEST_F(AbilityManagerServiceTest, TerminateAbilityResult_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest TerminateAbilityResult_001 start");
    EXPECT_EQ(abilityMs_->TerminateAbilityResult(nullptr, 1), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest TerminateAbilityResult_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 */
HWTEST_F(AbilityManagerServiceTest, StopServiceAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest StopServiceAbility_001 start");
    Want want;
    EXPECT_EQ(abilityMs_->StopServiceAbility(want, 100), ERR_CROSS_USER);
    HILOG_INFO("AbilityManagerServiceTest StopServiceAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMaxRestartNum
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMaxRestartNum
 */
HWTEST_F(AbilityManagerServiceTest, GetMaxRestartNum_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetMaxRestartNum_001 start");
    int max = 0;
    abilityMs_->GetMaxRestartNum(max, true);
    HILOG_INFO("AbilityManagerServiceTest GetMaxRestartNum_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: KillProcess
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillProcess
 */
HWTEST_F(AbilityManagerServiceTest, KillProcess_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest KillProcess_001 start");
    EXPECT_EQ(abilityMs_->KillProcess("test"), GET_BUNDLE_INFO_FAILED);
    HILOG_INFO("AbilityManagerServiceTest KillProcess_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ClearUpApplicationData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ClearUpApplicationData
 */
HWTEST_F(AbilityManagerServiceTest, ClearUpApplicationData_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ClearUpApplicationData_001 start");
    EXPECT_EQ(abilityMs_->ClearUpApplicationData("test"), ERR_OK);
    HILOG_INFO("AbilityManagerServiceTest ClearUpApplicationData_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: PreLoadAppDataAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService PreLoadAppDataAbilities
 */
HWTEST_F(AbilityManagerServiceTest, PreLoadAppDataAbilities_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest PreLoadAppDataAbilities_001 start");
    EXPECT_EQ(abilityMs_->PreLoadAppDataAbilities("", 100), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->PreLoadAppDataAbilities("test", -1), ERR_INVALID_STATE);
    HILOG_INFO("AbilityManagerServiceTest PreLoadAppDataAbilities_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsSystemUiApp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsSystemUiApp
 */
HWTEST_F(AbilityManagerServiceTest, IsSystemUiApp_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest IsSystemUiApp_001 start");
    AppExecFwk::AbilityInfo info;
    info.bundleName = "bundleName";
    EXPECT_FALSE(abilityMs_->IsSystemUiApp(info));

    info.bundleName = AbilityConfig::SYSTEM_UI_BUNDLE_NAME;
    info.name = "test";
    EXPECT_FALSE(abilityMs_->IsSystemUiApp(info));

    info.name = AbilityConfig::SYSTEM_UI_NAVIGATION_BAR;
    EXPECT_TRUE(abilityMs_->IsSystemUiApp(info));

    info.name = AbilityConfig::SYSTEM_UI_STATUS_BAR;
    EXPECT_TRUE(abilityMs_->IsSystemUiApp(info));

    info.name = AbilityConfig::SYSTEM_UI_ABILITY_NAME;
    EXPECT_TRUE(abilityMs_->IsSystemUiApp(info));
    HILOG_INFO("AbilityManagerServiceTest IsSystemUiApp_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsSystemUI
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsSystemUI
 */
HWTEST_F(AbilityManagerServiceTest, IsSystemUI_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest IsSystemUI_001 start");
    std::string bundleName = "bundleName";
    EXPECT_FALSE(abilityMs_->IsSystemUI(bundleName));

    bundleName = AbilityConfig::SYSTEM_UI_BUNDLE_NAME;
    EXPECT_TRUE(abilityMs_->IsSystemUI(bundleName));
    HILOG_INFO("AbilityManagerServiceTest IsSystemUI_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: HandleActiveTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleActiveTimeOut
 */
HWTEST_F(AbilityManagerServiceTest, HandleActiveTimeOut_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest HandleActiveTimeOut_001 start");
    abilityMs_->HandleActiveTimeOut(100);
    HILOG_INFO("AbilityManagerServiceTest HandleActiveTimeOut_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: HandleInactiveTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleInactiveTimeOut
 */
HWTEST_F(AbilityManagerServiceTest, HandleInactiveTimeOut_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest HandleInactiveTimeOut_001 start");
    abilityMs_->HandleInactiveTimeOut(100);
    HILOG_INFO("AbilityManagerServiceTest HandleInactiveTimeOut_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: HandleBackgroundTimeOut
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService HandleBackgroundTimeOut
 */
HWTEST_F(AbilityManagerServiceTest, HandleBackgroundTimeOut_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest HandleBackgroundTimeOut_001 start");
    abilityMs_->HandleBackgroundTimeOut(100);
    HILOG_INFO("AbilityManagerServiceTest HandleBackgroundTimeOut_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: VerificationToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService VerificationToken
 */
HWTEST_F(AbilityManagerServiceTest, VerificationToken_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest VerificationToken_001 start");
    EXPECT_FALSE(abilityMs_->VerificationToken(nullptr));
    HILOG_INFO("AbilityManagerServiceTest VerificationToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: VerificationAllToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService VerificationAllToken
 */
HWTEST_F(AbilityManagerServiceTest, VerificationAllToken_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest VerificationAllToken_001 start");
    EXPECT_FALSE(abilityMs_->VerificationAllToken(nullptr));
    HILOG_INFO("AbilityManagerServiceTest VerificationAllToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityManager
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityManager
 */
HWTEST_F(AbilityManagerServiceTest, GetDataAbilityManager_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetDataAbilityManager_001 start");
    EXPECT_EQ(abilityMs_->GetDataAbilityManager(nullptr), nullptr);
    HILOG_INFO("AbilityManagerServiceTest GetDataAbilityManager_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetListManagerByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetListManagerByUserId
 */
HWTEST_F(AbilityManagerServiceTest, GetListManagerByUserId_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetListManagerByUserId_001 start");
    EXPECT_NE(abilityMs_->GetListManagerByUserId(100), nullptr);

    auto temp = abilityMs_->missionListManagers_;
    abilityMs_->missionListManagers_.clear();
    EXPECT_EQ(abilityMs_->GetListManagerByUserId(100).get(), nullptr);
    abilityMs_->missionListManagers_ = temp;
    HILOG_INFO("AbilityManagerServiceTest GetListManagerByUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityManagerByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityManagerByUserId
 */
HWTEST_F(AbilityManagerServiceTest, GetDataAbilityManagerByUserId_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetDataAbilityManagerByUserId_001 start");
    EXPECT_NE(abilityMs_->GetDataAbilityManagerByUserId(100), nullptr);

    auto temp = abilityMs_->dataAbilityManagers_;
    abilityMs_->dataAbilityManagers_.clear();
    EXPECT_EQ(abilityMs_->GetDataAbilityManagerByUserId(100).get(), nullptr);
    abilityMs_->dataAbilityManagers_ = temp;
    HILOG_INFO("AbilityManagerServiceTest GetDataAbilityManagerByUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetConnectManagerByToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectManagerByToken
 */
HWTEST_F(AbilityManagerServiceTest, GetConnectManagerByToken_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetConnectManagerByToken_001 start");
    EXPECT_EQ(abilityMs_->GetConnectManagerByToken(nullptr), nullptr);
    HILOG_INFO("AbilityManagerServiceTest GetConnectManagerByToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityManagerByToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityManagerByToken
 */
HWTEST_F(AbilityManagerServiceTest, GetDataAbilityManagerByToken_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetDataAbilityManagerByToken_001 start");
    EXPECT_EQ(abilityMs_->GetDataAbilityManagerByToken(nullptr), nullptr);
    HILOG_INFO("AbilityManagerServiceTest GetDataAbilityManagerByToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartResidentApps
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartResidentApps
 */
HWTEST_F(AbilityManagerServiceTest, StartResidentApps_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest StartResidentApps_001 start");
    abilityMs_->StartResidentApps();
    HILOG_INFO("AbilityManagerServiceTest StartResidentApps_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectBmsService
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectBmsService
 */
HWTEST_F(AbilityManagerServiceTest, ConnectBmsService_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ConnectBmsService_001 start");
    abilityMs_->ConnectBmsService();
    HILOG_INFO("AbilityManagerServiceTest ConnectBmsService_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetWantSenderInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSenderInfo
 */
HWTEST_F(AbilityManagerServiceTest, GetWantSenderInfo_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetWantSenderInfo_001 start");
    std::shared_ptr<WantSenderInfo> info;
    EXPECT_EQ(abilityMs_->GetWantSenderInfo(nullptr, info), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest GetWantSenderInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAppMemorySize
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAppMemorySize
 */
HWTEST_F(AbilityManagerServiceTest, GetAppMemorySize_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetAppMemorySize_001 start");
    EXPECT_EQ(abilityMs_->GetAppMemorySize(), APP_MEMORY_SIZE);
    HILOG_INFO("AbilityManagerServiceTest GetAppMemorySize_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsRamConstrainedDevice
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRamConstrainedDevice
 */
HWTEST_F(AbilityManagerServiceTest, IsRamConstrainedDevice_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest IsRamConstrainedDevice_001 start");
    EXPECT_FALSE(abilityMs_->IsRamConstrainedDevice());
    HILOG_INFO("AbilityManagerServiceTest IsRamConstrainedDevice_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionSaveTime
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionSaveTime
 */
HWTEST_F(AbilityManagerServiceTest, GetMissionSaveTime_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetMissionSaveTime_001 start");
    EXPECT_NE(abilityMs_->GetMissionSaveTime(), 0);

    auto temp = abilityMs_->amsConfigResolver_;
    abilityMs_->amsConfigResolver_.reset();
    EXPECT_EQ(abilityMs_->GetMissionSaveTime(), 0);
    abilityMs_->amsConfigResolver_ = temp;
    HILOG_INFO("AbilityManagerServiceTest GetMissionSaveTime_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionIdByAbilityToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionIdByAbilityToken
 */
HWTEST_F(AbilityManagerServiceTest, GetMissionIdByAbilityToken_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetMissionIdByAbilityToken_001 start");
    EXPECT_EQ(abilityMs_->GetMissionIdByAbilityToken(nullptr), -1);

    EXPECT_EQ(abilityMs_->GetMissionIdByAbilityToken(MockToken(AbilityType::PAGE)), -1);
    HILOG_INFO("AbilityManagerServiceTest GetMissionIdByAbilityToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityTokenByMissionId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityTokenByMissionId
 */
HWTEST_F(AbilityManagerServiceTest, GetAbilityTokenByMissionId_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetAbilityTokenByMissionId_001 start");
    EXPECT_EQ(abilityMs_->GetAbilityTokenByMissionId(100), nullptr);

    auto temp = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_.reset();
    EXPECT_EQ(abilityMs_->GetAbilityTokenByMissionId(100), nullptr);
    abilityMs_->currentMissionListManager_ = temp;
    HILOG_INFO("AbilityManagerServiceTest GetAbilityTokenByMissionId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartRemoteAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartRemoteAbilityByCall
 */
HWTEST_F(AbilityManagerServiceTest, StartRemoteAbilityByCall_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest StartRemoteAbilityByCall_001 start");
    Want want;
    EXPECT_EQ(abilityMs_->StartRemoteAbilityByCall(want, nullptr, nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest StartRemoteAbilityByCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseRemoteAbility
 */
HWTEST_F(AbilityManagerServiceTest, ReleaseRemoteAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ReleaseRemoteAbility_001 start");
    AppExecFwk::ElementName element;
    EXPECT_EQ(abilityMs_->ReleaseRemoteAbility(nullptr, element), ERR_NULL_OBJECT);
    HILOG_INFO("AbilityManagerServiceTest ReleaseRemoteAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseCall
 */
HWTEST_F(AbilityManagerServiceTest, ReleaseCall_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ReleaseCall_001 start");
    AppExecFwk::ElementName element;
    EXPECT_EQ(abilityMs_->ReleaseCall(nullptr, element), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest ReleaseCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: JudgeAbilityVisibleControl
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService JudgeAbilityVisibleControl
 */
HWTEST_F(AbilityManagerServiceTest, JudgeAbilityVisibleControl_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest JudgeAbilityVisibleControl_001 start");
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    EXPECT_EQ(abilityMs_->JudgeAbilityVisibleControl(abilityInfo, 100), ERR_OK);

    abilityInfo.visible = false;
    EXPECT_EQ(abilityMs_->JudgeAbilityVisibleControl(abilityInfo, -1), ABILITY_VISIBLE_FALSE_DENY_REQUEST);
    EXPECT_EQ(abilityMs_->JudgeAbilityVisibleControl(abilityInfo, 100), ABILITY_VISIBLE_FALSE_DENY_REQUEST);
    HILOG_INFO("AbilityManagerServiceTest JudgeAbilityVisibleControl_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnAcceptWantResponse
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnAcceptWantResponse
 */
HWTEST_F(AbilityManagerServiceTest, OnAcceptWantResponse_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest OnAcceptWantResponse_001 start");
    AAFwk::Want want;
    abilityMs_->OnAcceptWantResponse(want, "test");

    auto temp = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_.reset();
    abilityMs_->OnAcceptWantResponse(want, "test");
    abilityMs_->currentMissionListManager_ = temp;
    HILOG_INFO("AbilityManagerServiceTest OnAcceptWantResponse_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnStartSpecifiedAbilityTimeoutResponse
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnStartSpecifiedAbilityTimeoutResponse
 */
HWTEST_F(AbilityManagerServiceTest, OnStartSpecifiedAbilityTimeoutResponse_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest OnStartSpecifiedAbilityTimeoutResponse_001 start");
    AAFwk::Want want;
    abilityMs_->OnStartSpecifiedAbilityTimeoutResponse(want);

    auto temp = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_.reset();
    abilityMs_->OnStartSpecifiedAbilityTimeoutResponse(want);
    abilityMs_->currentMissionListManager_ = temp;
    HILOG_INFO("AbilityManagerServiceTest OnStartSpecifiedAbilityTimeoutResponse_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityRunningInfos
 */
HWTEST_F(AbilityManagerServiceTest, GetAbilityRunningInfos_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetAbilityRunningInfos_001 start");
    std::vector<AbilityRunningInfo> info;
    EXPECT_EQ(abilityMs_->GetAbilityRunningInfos(info), ERR_OK);

    auto temp1 = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_.reset();
    EXPECT_EQ(abilityMs_->GetAbilityRunningInfos(info), ERR_INVALID_VALUE);
    abilityMs_->currentMissionListManager_ = temp1;

    auto temp2 = abilityMs_->connectManager_;
    abilityMs_->connectManager_.reset();
    EXPECT_EQ(abilityMs_->GetAbilityRunningInfos(info), ERR_INVALID_VALUE);
    abilityMs_->connectManager_ = temp2;

    auto temp3 = abilityMs_->dataAbilityManager_;
    abilityMs_->dataAbilityManager_.reset();
    EXPECT_EQ(abilityMs_->GetAbilityRunningInfos(info), ERR_INVALID_VALUE);
    abilityMs_->dataAbilityManager_ = temp3;
    HILOG_INFO("AbilityManagerServiceTest GetAbilityRunningInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetExtensionRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetExtensionRunningInfos
 */
HWTEST_F(AbilityManagerServiceTest, GetExtensionRunningInfos_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetExtensionRunningInfos_001 start");
    std::vector<AAFwk::ExtensionRunningInfo> extensionRunningInfo;
    EXPECT_EQ(abilityMs_->GetExtensionRunningInfos(10, extensionRunningInfo), ERR_OK);

    auto temp = abilityMs_->connectManager_;
    abilityMs_->connectManager_.reset();
    EXPECT_EQ(abilityMs_->GetExtensionRunningInfos(10, extensionRunningInfo), ERR_INVALID_VALUE);
    abilityMs_->connectManager_ = temp;
    HILOG_INFO("AbilityManagerServiceTest GetExtensionRunningInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetProcessRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetProcessRunningInfos
 */
HWTEST_F(AbilityManagerServiceTest, GetProcessRunningInfos_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetProcessRunningInfos_001 start");
    std::vector<AppExecFwk::RunningProcessInfo> info;
    EXPECT_EQ(abilityMs_->GetProcessRunningInfos(info), ERR_OK);
    HILOG_INFO("AbilityManagerServiceTest GetProcessRunningInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetProcessRunningInfosByUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetProcessRunningInfosByUserId
 */
HWTEST_F(AbilityManagerServiceTest, GetProcessRunningInfosByUserId_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetProcessRunningInfosByUserId_001 start");
    std::vector<AppExecFwk::RunningProcessInfo> info;
    EXPECT_EQ(abilityMs_->GetProcessRunningInfosByUserId(info, 100), INNER_ERR);
    HILOG_INFO("AbilityManagerServiceTest GetProcessRunningInfosByUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ClearUserData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ClearUserData
 */
HWTEST_F(AbilityManagerServiceTest, ClearUserData_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ClearUserData_001 start");
    abilityMs_->ClearUserData(100);
    HILOG_INFO("AbilityManagerServiceTest ClearUserData_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterSnapshotHandler
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterSnapshotHandler
 */
HWTEST_F(AbilityManagerServiceTest, RegisterSnapshotHandler_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest RegisterSnapshotHandler_001 start");
    EXPECT_EQ(abilityMs_->RegisterSnapshotHandler(nullptr), 0);

    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->RegisterSnapshotHandler(nullptr), 0);

    auto temp = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_.reset();
    EXPECT_EQ(abilityMs_->RegisterSnapshotHandler(nullptr), INNER_ERR);
    abilityMs_->currentMissionListManager_ = temp;

    EXPECT_EQ(abilityMs_->RegisterSnapshotHandler(nullptr), ERR_OK);
    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceTest RegisterSnapshotHandler_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CallRequestDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CallRequestDone
 */
HWTEST_F(AbilityManagerServiceTest, CallRequestDone_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CallRequestDone_001 start");
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> callStub = nullptr;
    abilityMs_->CallRequestDone(token, callStub);
    HILOG_INFO("AbilityManagerServiceTest CallRequestDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionSnapshot
 */
HWTEST_F(AbilityManagerServiceTest, GetMissionSnapshot_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetMissionSnapshot_001 start");
    MissionSnapshot missionSnapshot;
    EXPECT_EQ(abilityMs_->GetMissionSnapshot("", 1, missionSnapshot, true), CHECK_PERMISSION_FAILED);

    MyFlag::flag_ = 1;
    auto temp = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_.reset();
    EXPECT_EQ(abilityMs_->GetMissionSnapshot("", 1, missionSnapshot, true), INNER_ERR);
    abilityMs_->currentMissionListManager_ = temp;

    EXPECT_EQ(abilityMs_->GetMissionSnapshot("", 1, missionSnapshot, true), INNER_ERR);
    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceTest GetMissionSnapshot_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateMissionSnapShot
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateMissionSnapShot
 */
HWTEST_F(AbilityManagerServiceTest, UpdateMissionSnapShot_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest UpdateMissionSnapShot_001 start");
    MissionSnapshot missionSnapshot;
    abilityMs_->UpdateMissionSnapShot(nullptr);

    MyFlag::flag_ = 1;
    abilityMs_->UpdateMissionSnapShot(nullptr);
    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceTest UpdateMissionSnapShot_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: EnableRecoverAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService EnableRecoverAbility
 */
HWTEST_F(AbilityManagerServiceTest, EnableRecoverAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest EnableRecoverAbility_001 start");
    abilityMs_->EnableRecoverAbility(nullptr);
    abilityMs_->EnableRecoverAbility(MockToken(AbilityType::PAGE));
    HILOG_INFO("AbilityManagerServiceTest EnableRecoverAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RecoverAbilityRestart
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RecoverAbilityRestart
 */
HWTEST_F(AbilityManagerServiceTest, RecoverAbilityRestart_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest RecoverAbilityRestart_001 start");
    Want want;
    abilityMs_->RecoverAbilityRestart(want);
    HILOG_INFO("AbilityManagerServiceTest RecoverAbilityRestart_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleRecoverAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleRecoverAbility
 */
HWTEST_F(AbilityManagerServiceTest, ScheduleRecoverAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ScheduleRecoverAbility_001 start");
    abilityMs_->ScheduleRecoverAbility(nullptr, 1);
    sptr<IRemoteObject> token = MockToken(AbilityType::SERVICE);
    abilityMs_->ScheduleRecoverAbility(token, 1);
    HILOG_INFO("AbilityManagerServiceTest ScheduleRecoverAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetRemoteMissionSnapshotInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetRemoteMissionSnapshotInfo
 */
HWTEST_F(AbilityManagerServiceTest, GetRemoteMissionSnapshotInfo_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetRemoteMissionSnapshotInfo_001 start");
    MissionSnapshot missionSnapshot;
    EXPECT_EQ(abilityMs_->GetRemoteMissionSnapshotInfo("", 1, missionSnapshot), ERR_NULL_OBJECT);
    HILOG_INFO("AbilityManagerServiceTest GetRemoteMissionSnapshotInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetValidUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetValidUserId
 */
HWTEST_F(AbilityManagerServiceTest, GetValidUserId_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetValidUserId_001 start");
    MissionSnapshot missionSnapshot;
    EXPECT_EQ(abilityMs_->GetValidUserId(100), 100);
    EXPECT_EQ(abilityMs_->GetValidUserId(0), 0);
    HILOG_INFO("AbilityManagerServiceTest GetValidUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SetAbilityController
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetAbilityController
 */
HWTEST_F(AbilityManagerServiceTest, SetAbilityController_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest SetAbilityController_001 start");
    EXPECT_EQ(abilityMs_->SetAbilityController(nullptr, true), CHECK_PERMISSION_FAILED);

    auto temp1 = abilityMs_->abilityController_;
    auto temp2 = abilityMs_->controllerIsAStabilityTest_;
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->SetAbilityController(nullptr, true), ERR_OK);
    abilityMs_->abilityController_ = temp1;
    abilityMs_->controllerIsAStabilityTest_ = temp2;
    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceTest SetAbilityController_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SendANRProcessID
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendANRProcessID
 */
HWTEST_F(AbilityManagerServiceTest, SendANRProcessID_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest SendANRProcessID_001 start");
    MyFlag::flag_ = 0;
    EXPECT_EQ(abilityMs_->SendANRProcessID(100), CHECK_PERMISSION_FAILED);

    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->SendANRProcessID(100), RESOLVE_ABILITY_ERR);

    MyFlag::flag_ = 2;
    EXPECT_EQ(abilityMs_->SendANRProcessID(100), RESOLVE_ABILITY_ERR);

    MyFlag::flag_ = 3;
    EXPECT_EQ(abilityMs_->SendANRProcessID(100), RESOLVE_ABILITY_ERR);

    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceTest SendANRProcessID_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsRunningInStabilityTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRunningInStabilityTest
 */
HWTEST_F(AbilityManagerServiceTest, IsRunningInStabilityTest_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest IsRunningInStabilityTest_001 start");
    abilityMs_->controllerIsAStabilityTest_ = false;
    EXPECT_FALSE(abilityMs_->IsRunningInStabilityTest());
    HILOG_INFO("AbilityManagerServiceTest IsRunningInStabilityTest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: InitAbilityInfoFromExtension
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService InitAbilityInfoFromExtension
 */
HWTEST_F(AbilityManagerServiceTest, InitAbilityInfoFromExtension_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest InitAbilityInfoFromExtension_001 start");
    ExtensionAbilityInfo extensionInfo;
    AbilityInfo abilityInfo;
    EXPECT_EQ(abilityMs_->InitAbilityInfoFromExtension(extensionInfo, abilityInfo), 0);
    HILOG_INFO("AbilityManagerServiceTest InitAbilityInfoFromExtension_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUserTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUserTest
 */
HWTEST_F(AbilityManagerServiceTest, StartUserTest_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest StartUserTest_001 start");
    Want want;
    EXPECT_EQ(abilityMs_->StartUserTest(want, nullptr), ERR_INVALID_VALUE);

    sptr<IRemoteObject> observer = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->StartUserTest(want, nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest StartUserTest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: FinishUserTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService FinishUserTest
 */
HWTEST_F(AbilityManagerServiceTest, FinishUserTest_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest FinishUserTest_001 start");
    EXPECT_EQ(abilityMs_->FinishUserTest("", 1, ""), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->FinishUserTest("", 1, "test"), INNER_ERR);
    HILOG_INFO("AbilityManagerServiceTest FinishUserTest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DelegatorDoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceTest, DelegatorDoAbilityForeground_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest DelegatorDoAbilityForeground_001 start");
    EXPECT_EQ(abilityMs_->DelegatorDoAbilityForeground(nullptr), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->DelegatorDoAbilityForeground(MockToken(AbilityType::PAGE)), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest DelegatorDoAbilityForeground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DelegatorDoAbilityBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityBackground
 */
HWTEST_F(AbilityManagerServiceTest, DelegatorDoAbilityBackground_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest DelegatorDoAbilityBackground_001 start");
    EXPECT_EQ(abilityMs_->DelegatorDoAbilityBackground(nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest DelegatorDoAbilityBackground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceTest, DoAbilityForeground_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest DoAbilityForeground_001 start");
    EXPECT_EQ(abilityMs_->DoAbilityForeground(nullptr, 1), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->DoAbilityForeground(MockToken(AbilityType::PAGE), 1), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest DoAbilityForeground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityBackground
 */
HWTEST_F(AbilityManagerServiceTest, DoAbilityBackground_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest DoAbilityBackground_001 start");
    EXPECT_EQ(abilityMs_->DoAbilityBackground(nullptr, 1), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->DoAbilityBackground(MockToken(AbilityType::PAGE), 1), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest DoAbilityBackground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsAbilityControllerStart
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsAbilityControllerStart
 */
HWTEST_F(AbilityManagerServiceTest, IsAbilityControllerStart_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest IsAbilityControllerStart_001 start");
    Want want;
    auto temp = abilityMs_->abilityController_;
    abilityMs_->abilityController_ = nullptr;
    EXPECT_TRUE(abilityMs_->IsAbilityControllerStart(want, "test"));

    abilityMs_->abilityController_ = temp;
    abilityMs_->controllerIsAStabilityTest_ = false;
    EXPECT_TRUE(abilityMs_->IsAbilityControllerStart(want, "test"));

    abilityMs_->controllerIsAStabilityTest_ = true;
    auto temp2 = abilityMs_->abilityController_;
    auto mockAbilityController = new MockAbilityController();
    abilityMs_->abilityController_ = mockAbilityController;
    EXPECT_CALL(*mockAbilityController, AllowAbilityStart(_, _)).Times(1).WillOnce(Return(false));
    EXPECT_FALSE(abilityMs_->IsAbilityControllerStart(want, "test"));
    abilityMs_->abilityController_ = temp2;
    HILOG_INFO("AbilityManagerServiceTest IsAbilityControllerStart_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsAbilityControllerForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsAbilityControllerForeground
 */
HWTEST_F(AbilityManagerServiceTest, IsAbilityControllerForeground_001, TestSize.Level1)
{
    auto temp = abilityMs_->abilityController_;
    abilityMs_->abilityController_ = nullptr;
    EXPECT_TRUE(abilityMs_->IsAbilityControllerForeground("test"));

    abilityMs_->abilityController_ = temp;
    abilityMs_->controllerIsAStabilityTest_ = false;
    EXPECT_TRUE(abilityMs_->IsAbilityControllerForeground("test"));

    abilityMs_->controllerIsAStabilityTest_ = true;
    auto temp2 = abilityMs_->abilityController_;
    auto mockAbilityController = new MockAbilityController();
    abilityMs_->abilityController_ = mockAbilityController;
    EXPECT_CALL(*mockAbilityController, AllowAbilityBackground(_)).Times(1).WillOnce(Return(false));
    EXPECT_FALSE(abilityMs_->IsAbilityControllerForeground("test"));
    abilityMs_->abilityController_ = temp2;
    HILOG_INFO("AbilityManagerServiceTest IsAbilityControllerForeground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DelegatorMoveMissionToFront
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorMoveMissionToFront
 */
HWTEST_F(AbilityManagerServiceTest, DelegatorMoveMissionToFront_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest DelegatorMoveMissionToFront_001 start");
    auto temp = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_.reset();
    EXPECT_EQ(abilityMs_->DelegatorMoveMissionToFront(1), ERR_NO_INIT);
    abilityMs_->currentMissionListManager_ = temp;

    EXPECT_EQ(abilityMs_->DelegatorMoveMissionToFront(100), MOVE_MISSION_FAILED);
    EXPECT_EQ(abilityMs_->DelegatorMoveMissionToFront(1), MOVE_MISSION_FAILED);
    HILOG_INFO("AbilityManagerServiceTest DelegatorMoveMissionToFront_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateCallerInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateCallerInfo
 */
HWTEST_F(AbilityManagerServiceTest, UpdateCallerInfo_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest UpdateCallerInfo_001 start");
    Want want;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::PAGE);
    abilityMs_->UpdateCallerInfo(want, callerToken);
    HILOG_INFO("AbilityManagerServiceTest UpdateCallerInfo_001 end");
}

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: ForceTimeoutForTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ForceTimeoutForTest
 */
HWTEST_F(AbilityManagerServiceTest, ForceTimeoutForTest_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest ForceTimeoutForTest_001 start");
    EXPECT_EQ(abilityMs_->ForceTimeoutForTest("", ""), INVALID_DATA);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("clean", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("INITIAL", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("INACTIVE", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("FOREGROUND", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("BACKGROUND", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("TERMINATING", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("COMMAND", ""), ERR_OK);
    EXPECT_TRUE(abilityMs_->ForceTimeoutForTest("test", ""), INVALID_DATA);
    HILOG_INFO("AbilityManagerServiceTest ForceTimeoutForTest_001 end");
}
#endif

/*
 * Feature: AbilityManagerService
 * Function: CheckStaticCfgPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckStaticCfgPermission
 */
HWTEST_F(AbilityManagerServiceTest, CheckStaticCfgPermission_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckStaticCfgPermission_001 start");
    AppExecFwk::AbilityInfo abilityInfo;
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityInfo, false, -1), AppExecFwk::Constants::PERMISSION_GRANTED);

    MyFlag::flag_ = 0;
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityInfo, false, -1), AppExecFwk::Constants::PERMISSION_GRANTED);

    abilityInfo.applicationInfo.accessTokenId = 0;
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityInfo, false, -1), ERR_OK);

    // abilityInfo.permissions is empty
    abilityInfo.applicationInfo.accessTokenId = -1;
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityInfo, false, -1), AppExecFwk::Constants::PERMISSION_GRANTED);

    // abilityInfo.permissions is not empty
    abilityInfo.permissions.push_back("test1");
    abilityInfo.permissions.push_back("test2");
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityInfo, false, -1), AppExecFwk::Constants::PERMISSION_NOT_GRANTED);

    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::DATASHARE;
    abilityInfo.readPermission = "test";
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityInfo, false, -1), AppExecFwk::Constants::PERMISSION_NOT_GRANTED);

    abilityInfo.readPermission.clear();
    abilityInfo.writePermission = "test";
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityInfo, false, -1), AppExecFwk::Constants::PERMISSION_NOT_GRANTED);
    HILOG_INFO("AbilityManagerServiceTest CheckStaticCfgPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: JudgeMultiUserConcurrency
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService JudgeMultiUserConcurrency
 */
HWTEST_F(AbilityManagerServiceTest, JudgeMultiUserConcurrency_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest JudgeMultiUserConcurrency_001 start");
    EXPECT_TRUE(abilityMs_->JudgeMultiUserConcurrency(0));

    auto temp = abilityMs_->userController_;
    abilityMs_->userController_ = nullptr;
    EXPECT_FALSE(abilityMs_->JudgeMultiUserConcurrency(100));
    abilityMs_->userController_ = temp;
    HILOG_INFO("AbilityManagerServiceTest JudgeMultiUserConcurrency_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckWindowMode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckWindowMode
 */
HWTEST_F(AbilityManagerServiceTest, CheckWindowMode_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckWindowMode_001 start");
    auto windowMode = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED;
    std::vector<AppExecFwk::SupportWindowMode> windowModes;
    EXPECT_TRUE(abilityMs_->CheckWindowMode(windowMode, windowModes));

    windowMode = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FULLSCREEN;
    EXPECT_FALSE(abilityMs_->CheckWindowMode(windowMode, windowModes));
    HILOG_INFO("AbilityManagerServiceTest CheckWindowMode_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsNeedTimeoutForTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsNeedTimeoutForTest
 */
HWTEST_F(AbilityManagerServiceTest, IsNeedTimeoutForTest_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest IsNeedTimeoutForTest_001 start");
    EXPECT_FALSE(abilityMs_->IsNeedTimeoutForTest("", ""));
    abilityMs_->timeoutMap_.insert({"state", "abilityName"});
    EXPECT_TRUE(abilityMs_->IsNeedTimeoutForTest("abilityName", "state"));
    abilityMs_->timeoutMap_.clear();
    HILOG_INFO("AbilityManagerServiceTest IsNeedTimeoutForTest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetValidDataAbilityUri
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetValidDataAbilityUri
 */
HWTEST_F(AbilityManagerServiceTest, GetValidDataAbilityUri_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetValidDataAbilityUri_001 start");
    std::string adjustUri;
    EXPECT_FALSE(abilityMs_->GetValidDataAbilityUri("test", adjustUri));

    EXPECT_TRUE(abilityMs_->GetValidDataAbilityUri("//test", adjustUri));
    HILOG_INFO("AbilityManagerServiceTest GetValidDataAbilityUri_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDataAbilityUri
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDataAbilityUri
 */
HWTEST_F(AbilityManagerServiceTest, GetDataAbilityUri_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetDataAbilityUri_001 start");
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::string uri;
    EXPECT_FALSE(abilityMs_->GetDataAbilityUri(abilityInfos, "", uri));
    EXPECT_FALSE(abilityMs_->GetDataAbilityUri(abilityInfos, "test", uri));

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfos.push_back(abilityInfo);
    EXPECT_FALSE(abilityMs_->GetDataAbilityUri(abilityInfos, "", uri));

    abilityInfo.type = AbilityType::DATA;
    abilityInfo.name = "test";
    EXPECT_FALSE(abilityMs_->GetDataAbilityUri(abilityInfos, "test", uri));
    HILOG_INFO("AbilityManagerServiceTest GetDataAbilityUri_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: VerifyAccountPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService VerifyAccountPermission
 */
HWTEST_F(AbilityManagerServiceTest, VerifyAccountPermission_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest VerifyAccountPermission_001 start");
    EXPECT_EQ(abilityMs_->VerifyAccountPermission(-1), ERR_OK);
    EXPECT_EQ(abilityMs_->VerifyAccountPermission(0), ERR_OK);
    HILOG_INFO("AbilityManagerServiceTest VerifyAccountPermission_001 end");
}

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: BlockAmsService
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAmsService
 */
HWTEST_F(AbilityManagerServiceTest, BlockAmsService_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest BlockAmsService_001 start");
    auto temp = abilityMs_->handler_;
    abilityMs_->handler_ = nullptr;
    EXPECT_EQ(abilityMs_->BlockAmsService(), ERR_NO_INIT);

    abilityMs_->handler_ = temp;
    EXPECT_EQ(abilityMs_->BlockAmsService(), ERR_OK);
    HILOG_INFO("AbilityManagerServiceTest BlockAmsService_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: BlockAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAbility
 */
HWTEST_F(AbilityManagerServiceTest, BlockAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest BlockAbility_001 start");
    auto temp = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_ = nullptr;
    EXPECT_EQ(abilityMs_->BlockAbility(1), ERR_OK);

    abilityMs_->currentMissionListManager_ = temp;
    EXPECT_EQ(abilityMs_->BlockAbility(1), ERR_OK);
    HILOG_INFO("AbilityManagerServiceTest BlockAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: BlockAppService
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAppService
 */
HWTEST_F(AbilityManagerServiceTest, BlockAppService_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest BlockAppService_001 start");
    EXPECT_EQ(abilityMs_->BlockAppService(1), ERR_OK);
    HILOG_INFO("AbilityManagerServiceTest BlockAppService_001 end");
}
#endif

/*
 * Feature: AbilityManagerService
 * Function: CreateVerificationInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CreateVerificationInfo
 */
HWTEST_F(AbilityManagerServiceTest, CreateVerificationInfo_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CreateVerificationInfo_001 start");
    AbilityRequest abilityRequest;
    abilityMs_->whiteListassociatedWakeUpFlag_ = false;
    EXPECT_FALSE(abilityMs_->CreateVerificationInfo(abilityRequest).associatedWakeUp);

    abilityMs_->whiteListassociatedWakeUpFlag_ = true;
    abilityRequest.appInfo.bundleName = "com.ohos.settingsdata";
    EXPECT_TRUE(abilityMs_->CreateVerificationInfo(abilityRequest).associatedWakeUp);

    abilityRequest.appInfo.bundleName = "test";
    abilityRequest.appInfo.associatedWakeUp = false;
    EXPECT_FALSE(abilityMs_->CreateVerificationInfo(abilityRequest).associatedWakeUp);
    HILOG_INFO("AbilityManagerServiceTest CreateVerificationInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallerEligibility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallerEligibility
 */
HWTEST_F(AbilityManagerServiceTest, CheckCallerEligibility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest CheckCallerEligibility_001 start");
    AppExecFwk::AbilityInfo abilityInfo;
    EXPECT_FALSE(abilityMs_->CheckCallerEligibility(abilityInfo, 100));

    MyFlag::flag_ = 1;
    EXPECT_TRUE(abilityMs_->CheckCallerEligibility(abilityInfo, 100));
    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceTest CheckCallerEligibility_001 end");
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
 * Function: IsCallFromBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCallFromBackground
 */
HWTEST_F(AbilityManagerServiceTest, IsCallFromBackground_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest IsCallFromBackground_001 start");
    AbilityRequest abilityRequest;
    bool isBackgroundCall = true;

    // IsSACall
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->IsCallFromBackground(abilityRequest, isBackgroundCall), ERR_OK);

    // IsShellCall
    MyFlag::flag_ = 2;
    EXPECT_EQ(abilityMs_->IsCallFromBackground(abilityRequest, isBackgroundCall), ERR_OK);

    // IsStartFreeInstall
    unsigned int flag = 0x00000800;
    abilityRequest.want.SetFlags(flag);
    EXPECT_EQ(abilityMs_->IsCallFromBackground(abilityRequest, isBackgroundCall), ERR_OK);

    // set abilityRequest.callerToken and abilityRequest.want
    MyFlag::flag_ = 0;
    abilityRequest.callerToken = nullptr;
    abilityRequest.want.SetFlags(0);
    EXPECT_EQ(abilityMs_->IsCallFromBackground(abilityRequest, isBackgroundCall), ERR_INVALID_VALUE);

    abilityRequest.callerToken = MockToken(AbilityType::PAGE);
    abilityRequest.want.SetParam("isDelegatorCall", true);
    EXPECT_EQ(abilityMs_->IsCallFromBackground(abilityRequest, isBackgroundCall), ERR_INVALID_VALUE);

    abilityRequest.callerToken = nullptr;
    abilityRequest.want.SetParam("isDelegatorCall", true);
    EXPECT_EQ(abilityMs_->IsCallFromBackground(abilityRequest, isBackgroundCall), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest IsCallFromBackground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsUseNewStartUpRule
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsUseNewStartUpRule
 */
HWTEST_F(AbilityManagerServiceTest, IsUseNewStartUpRule_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest IsUseNewStartUpRule_001 start");
    AbilityRequest abilityRequest;
    EXPECT_FALSE(abilityMs_->startUpNewRule_);
    abilityMs_->startUpNewRule_ = false;
    EXPECT_FALSE(abilityMs_->IsUseNewStartUpRule(abilityRequest));

    abilityMs_->startUpNewRule_ = true;
    MyFlag::flag_ = 1;
    EXPECT_TRUE(abilityMs_->IsUseNewStartUpRule(abilityRequest));
    MyFlag::flag_ = 2;
    EXPECT_TRUE(abilityMs_->IsUseNewStartUpRule(abilityRequest));

    EXPECT_TRUE(abilityMs_->IsUseNewStartUpRule(abilityRequest));
    abilityMs_->startUpNewRule_ = false;
    HILOG_INFO("AbilityManagerServiceTest IsUseNewStartUpRule_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetStartUpNewRuleFlag
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetStartUpNewRuleFlag
 */
HWTEST_F(AbilityManagerServiceTest, GetStartUpNewRuleFlag_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest GetStartUpNewRuleFlag_001 start");
    EXPECT_EQ(abilityMs_->GetStartUpNewRuleFlag(), abilityMs_->startUpNewRule_);
    HILOG_INFO("AbilityManagerServiceTest GetStartUpNewRuleFlag_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AddStartControlParam
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AddStartControlParam
 */
HWTEST_F(AbilityManagerServiceTest, AddStartControlParam_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest AddStartControlParam_001 start");
    Want want;
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->AddStartControlParam(want, nullptr), ERR_OK);

    MyFlag::flag_ = 2;
    EXPECT_EQ(abilityMs_->AddStartControlParam(want, nullptr), ERR_OK);

    MyFlag::flag_ = 0;
    EXPECT_EQ(abilityMs_->AddStartControlParam(want, nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest AddStartControlParam_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsCrossUserCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCrossUserCall
 */
HWTEST_F(AbilityManagerServiceTest, IsCrossUserCall_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest IsCrossUserCall_001 start");
    int32_t userId = -1;
    EXPECT_EQ(abilityMs_->IsCrossUserCall(userId), false);
    HILOG_INFO("AbilityManagerServiceTest IsCrossUserCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsCrossUserCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCrossUserCall
 */
HWTEST_F(AbilityManagerServiceTest, IsCrossUserCall_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest IsCrossUserCall_002 start");
    int32_t userId = 0;
    EXPECT_EQ(abilityMs_->IsCrossUserCall(userId), false);
    HILOG_INFO("AbilityManagerServiceTest IsCrossUserCall_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsCrossUserCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCrossUserCall
 */
HWTEST_F(AbilityManagerServiceTest, IsCrossUserCall_003, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest IsCrossUserCall_003 start");
    int32_t userId = 10;
    EXPECT_EQ(abilityMs_->IsCrossUserCall(userId), true);
    HILOG_INFO("AbilityManagerServiceTest IsCrossUserCall_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsValidMissionIds
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsValidMissionIds
 */
HWTEST_F(AbilityManagerServiceTest, IsValidMissionIds_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest IsValidMissionIds_001 start");
    std::vector<int32_t> missionIds;
    std::vector<MissionVaildResult> results;
    EXPECT_EQ(abilityMs_->IsValidMissionIds(missionIds, results), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest IsValidMissionIds_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsValidMissionIds
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsValidMissionIds
 */
HWTEST_F(AbilityManagerServiceTest, IsValidMissionIds_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest IsValidMissionIds_002 start");
    std::vector<int32_t> missionIds;
    std::vector<MissionVaildResult> results;
    abilityMs_->InitMissionListManager(IPCSkeleton::GetCallingUid() / BASE_USER_RANGE, false);
    EXPECT_EQ(abilityMs_->IsValidMissionIds(missionIds, results), ERR_OK);
    HILOG_INFO("AbilityManagerServiceTest IsValidMissionIds_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceTest, StartUIExtensionAbility_001, TestSize.Level1)
{
    Want want;
    EXPECT_EQ(abilityMs_->StartUIExtensionAbility(want, nullptr, 100, AppExecFwk::ExtensionAbilityType::UI),
        ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceTest, StartUIExtensionAbility_002, TestSize.Level1)
{
    Want want;
    EXPECT_EQ(abilityMs_->StartUIExtensionAbility(want, MockSessionInfo(0), USER_ID_U100, 
        AppExecFwk::ExtensionAbilityType::UI), CHECK_PERMISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceTest, TerminateUIExtensionAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest TerminateUIExtensionAbility_001 start");
    Want* resultWant = nullptr;
    EXPECT_EQ(abilityMs_->TerminateUIExtensionAbility(nullptr, 1, resultWant), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->TerminateUIExtensionAbility(MockSessionInfo(0), 1, resultWant), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest TerminateUIExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceTest, MinimizeUIExtensionAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceTest MinimizeUIExtensionAbility_001 start");
    EXPECT_EQ(abilityMs_->MinimizeUIExtensionAbility(nullptr, true), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->MinimizeUIExtensionAbility(nullptr, false), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->MinimizeUIExtensionAbility(MockSessionInfo(0), true), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->MinimizeUIExtensionAbility(MockSessionInfo(0), false), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceTest MinimizeUIExtensionAbility_001 end");
}
}  // namespace AAFwk
}  // namespace OHOS
