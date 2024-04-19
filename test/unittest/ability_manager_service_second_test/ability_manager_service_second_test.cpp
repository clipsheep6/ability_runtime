/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "ability_connect_manager.h"
#include "ability_connection.h"
#include "ability_start_setting.h"
#include "recovery_param.h"
#undef private
#undef protected

#include "ability_manager_errors.h"
#include "connection_observer_errors.h"
#include "hilog_tag_wrapper.h"
#include "session/host/include/session.h"
#include "scene_board_judgement.h"
#include "mock_sa_call.h"

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
}  // namespace
class AbilityManagerServiceSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

public:
    AbilityRequest abilityRequest_{};
    Want want_{};
};

AbilityRequest AbilityManagerServiceSecondTest::GenerateAbilityRequest(const std::string& deviceName,
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

void AbilityManagerServiceSecondTest::SetUpTestCase() {}

void AbilityManagerServiceSecondTest::TearDownTestCase() {}

void AbilityManagerServiceSecondTest::SetUp() {}

void AbilityManagerServiceSecondTest::TearDown() {}

/*
 * Feature: AbilityManagerService
 * Function: StartSwitchUserDialog
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartSwitchUserDialog
 */
HWTEST_F(AbilityManagerServiceSecondTest, StartSwitchUserDialog_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartSwitchUserDialog_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->StartSwitchUserDialog();

    abilityMs_->userController_ = nullptr;
    abilityMs_->StartSwitchUserDialog();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartSwitchUserDialog_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartSwitchUserDialogInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartSwitchUserDialogInner
 */
HWTEST_F(AbilityManagerServiceSecondTest, StartSwitchUserDialogInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartSwitchUserDialogInner_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->StartSwitchUserDialogInner(want, 100);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartSwitchUserDialogInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopSwitchUserDialog
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopSwitchUserDialog
 */
HWTEST_F(AbilityManagerServiceSecondTest, StopSwitchUserDialog_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StopSwitchUserDialog_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->StopSwitchUserDialog();

    abilityMs_->userController_ = nullptr;
    abilityMs_->StopSwitchUserDialog();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StopSwitchUserDialog_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopSwitchUserDialogInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopSwitchUserDialogInner
 */
HWTEST_F(AbilityManagerServiceSecondTest, StopSwitchUserDialogInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StopSwitchUserDialogInner_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->StopSwitchUserDialogInner(want, 100);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StopSwitchUserDialogInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckOptExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckOptExtensionAbility
 */
HWTEST_F(AbilityManagerServiceSecondTest, CheckOptExtensionAbility_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckOptExtensionAbility_002 start");
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
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckOptExtensionAbility_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckOptExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckOptExtensionAbility
 */
HWTEST_F(AbilityManagerServiceSecondTest, CheckOptExtensionAbility_003, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckOptExtensionAbility_003 start");
    abilityRequest_.abilityInfo.type = AbilityType::PAGE;
    auto result = abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, USER_ID_U100,
        ExtensionAbilityType::FORM);
    EXPECT_EQ(ERR_WRONG_INTERFACE_CALL, result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckOptExtensionAbility_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckOptExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckOptExtensionAbility
 */
HWTEST_F(AbilityManagerServiceSecondTest, CheckOptExtensionAbility_004, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckOptExtensionAbility_004 start");
    abilityRequest_.abilityInfo.extensionAbilityType = ExtensionAbilityType::DATASHARE;
    auto result = abilityMs_->CheckOptExtensionAbility(want_, abilityRequest_, USER_ID_U100,
        ExtensionAbilityType::FORM);
    EXPECT_EQ(ERR_WRONG_INTERFACE_CALL, result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckOptExtensionAbility_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByCall
 */
HWTEST_F(AbilityManagerServiceSecondTest, StartAbilityByCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartAbilityByCall_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_EQ(abilityMs_->StartAbilityByCall(want, nullptr, nullptr), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartAbilityByCall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallDataAbilityPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallDataAbilityPermission
 */
HWTEST_F(AbilityManagerServiceSecondTest, CheckCallDataAbilityPermission_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckCallDataAbilityPermission_001 start");
    AbilityRequest abilityRequest;
    EXPECT_EQ(abilityMs_->CheckCallDataAbilityPermission(abilityRequest, false), ERR_INVALID_VALUE);
    abilityRequest.abilityInfo.applicationInfo.name = "test";
    EXPECT_EQ(abilityMs_->CheckCallDataAbilityPermission(abilityRequest, false), ERR_INVALID_VALUE);

    abilityRequest.abilityInfo.applicationInfo.name = "test";
    abilityRequest.abilityInfo.applicationInfo.bundleName = "test";
    abilityRequest.abilityInfo.type = AbilityType::SERVICE;
    EXPECT_EQ(abilityMs_->CheckCallDataAbilityPermission(abilityRequest, false), ERR_WRONG_INTERFACE_CALL);

    abilityRequest_.abilityInfo.type = AbilityType::DATA;
    abilityMs_->startUpNewRule_ = false;
    EXPECT_EQ(abilityMs_->CheckCallDataAbilityPermission(abilityRequest, false), ERR_WRONG_INTERFACE_CALL);
    abilityMs_->startUpNewRule_ = true;

    abilityRequest_.abilityInfo.type = AbilityType::DATA;
    EXPECT_EQ(abilityMs_->CheckCallDataAbilityPermission(abilityRequest_, false), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckCallDataAbilityPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallDataAbilityPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallDataAbilityPermission
 */
HWTEST_F(AbilityManagerServiceSecondTest, CheckCallDataAbilityPermission_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckCallDataAbilityPermission_002 start");
    abilityRequest_.abilityInfo.type = AbilityType::DATA;
    abilityMs_->Init();
    EXPECT_EQ(abilityMs_->CheckCallDataAbilityPermission(abilityRequest_, false), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckCallDataAbilityPermission_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallOtherExtensionPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallOtherExtensionPermission
 */
HWTEST_F(AbilityManagerServiceSecondTest, CheckCallOtherExtensionPermission_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckCallOtherExtensionPermission_001 start");
    bool oldFlag = abilityMs_->startUpNewRule_;
    abilityMs_->startUpNewRule_ = true;
    EXPECT_EQ(abilityMs_->CheckCallOtherExtensionPermission(abilityRequest_), CHECK_PERMISSION_FAILED);
    abilityMs_->startUpNewRule_ = oldFlag;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckCallOtherExtensionPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallOtherExtensionPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallOtherExtensionPermission
 */
HWTEST_F(AbilityManagerServiceSecondTest, CheckCallOtherExtensionPermission_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckCallOtherExtensionPermission_002 start");
    abilityRequest_.abilityInfo.visible = false;
    bool oldFlag = abilityMs_->startUpNewRule_;
    abilityMs_->startUpNewRule_ = true;
    EXPECT_EQ(abilityMs_->CheckCallOtherExtensionPermission(abilityRequest_), CHECK_PERMISSION_FAILED);
    abilityMs_->startUpNewRule_ = oldFlag;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckCallOtherExtensionPermission_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckStartByCallPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckStartByCallPermission
 */
HWTEST_F(AbilityManagerServiceSecondTest, CheckStartByCallPermission_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckStartByCallPermission_001 start");
    EXPECT_EQ(abilityMs_->CheckStartByCallPermission(abilityRequest_), RESOLVE_CALL_ABILITY_TYPE_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckStartByCallPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 */
HWTEST_F(AbilityManagerServiceSecondTest, StartAbility_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
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
HWTEST_F(AbilityManagerServiceSecondTest, StartAbility_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    AbilityStartSetting abilityStartSetting;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 0;
    EXPECT_EQ(abilityMs_->StartAbility(want, abilityStartSetting, callerToken, 0, requestCode),
        CHECK_PERMISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCaller
 */
HWTEST_F(AbilityManagerServiceSecondTest, StartAbilityAsCaller_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartAbilityAsCaller_001 start");
    Want want;
    int requestCode = 0;
    sptr<IRemoteObject> callerToken = nullptr;
    EXPECT_EQ(abilityMs_->StartAbility(want, callerToken, USER_ID_U100, requestCode), CHECK_PERMISSION_FAILED);

    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    EXPECT_EQ(abilityMs_->StartAbilityAsCaller(want, callerToken, nullptr, USER_ID_U100, requestCode),
        ERR_INVALID_CONTINUATION_FLAG);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartAbilityAsCaller_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCaller
 */
HWTEST_F(AbilityManagerServiceSecondTest, StartAbilityAsCaller_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartAbilityAsCaller_002 start");
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 0;
    EXPECT_EQ(abilityMs_->StartAbilityAsCaller(want, startOptions, callerToken, nullptr, USER_ID_U100, requestCode),
    CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartAbilityAsCaller_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForResultAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityForResultAsCaller
 */
HWTEST_F(AbilityManagerServiceSecondTest, StartAbilityForResultAsCaller_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartAbilityForResultAsCaller_001 start");
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 0;
    EXPECT_EQ(abilityMs_->StartAbilityForResultAsCaller(want, callerToken, requestCode, USER_ID_U100), ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartAbilityForResultAsCaller_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForResultAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityForResultAsCaller
 */
HWTEST_F(AbilityManagerServiceSecondTest, StartAbilityForResultAsCaller_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartAbilityForResultAsCaller_002 start");
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 0;
    EXPECT_EQ(abilityMs_->StartAbilityForResultAsCaller(want, startOptions, callerToken, requestCode, USER_ID_U100),
        ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartAbilityForResultAsCaller_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsBackgroundTaskUid
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsBackgroundTaskUid
 */
HWTEST_F(AbilityManagerServiceSecondTest, IsBackgroundTaskUid_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int uid = USER_ID_U100;
    EXPECT_FALSE(abilityMs_->IsBackgroundTaskUid(uid));
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbility
 */
HWTEST_F(AbilityManagerServiceSecondTest, TerminateAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest TerminateAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want* resultWant = nullptr;
    EXPECT_EQ(abilityMs_->TerminateAbility(nullptr, 1, resultWant), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest TerminateAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CloseAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CloseAbility
 */
HWTEST_F(AbilityManagerServiceSecondTest, CloseAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CloseAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want* resultWant = nullptr;
    EXPECT_EQ(abilityMs_->CloseAbility(nullptr, 1, resultWant), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CloseAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SendResultToAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendResultToAbility
 */
HWTEST_F(AbilityManagerServiceSecondTest, SendResultToAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest SendResultToAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t requestCode = 1;
    int32_t resultCode = 1;
    Want resultWant;
    EXPECT_EQ(abilityMs_->SendResultToAbility(requestCode, resultCode, resultWant), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest SendResultToAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckIsRemote
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckIsRemote
 */
HWTEST_F(AbilityManagerServiceSecondTest, CheckIsRemote_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckIsRemote_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_FALSE(abilityMs_->CheckIsRemote(""));
    std::string deviceId = "test";
    EXPECT_FALSE(abilityMs_->CheckIsRemote(deviceId));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckIsRemote_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckIfOperateRemote
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckIfOperateRemote
 */
HWTEST_F(AbilityManagerServiceSecondTest, CheckIfOperateRemote_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckIfOperateRemote_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_FALSE(abilityMs_->CheckIfOperateRemote(want));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckIfOperateRemote_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CloseUIAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CloseUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceSecondTest, CloseUIAbilityBySCB_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CloseUIAbilityBySCB_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(abilityMs_->CloseUIAbilityBySCB(nullptr), ERR_INVALID_VALUE);

    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    EXPECT_EQ(abilityMs_->CloseUIAbilityBySCB(sessionInfo), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 */
HWTEST_F(AbilityManagerServiceSecondTest, ConnectAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ConnectAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_EQ(abilityMs_->ConnectAbility(want, nullptr, nullptr, 100), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ConnectAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbilityCommon
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbilityCommon
 */
HWTEST_F(AbilityManagerServiceSecondTest, ConnectAbilityCommon_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ConnectAbilityCommon_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ExtensionAbilityType extensionType = ExtensionAbilityType::SERVICE;
    EXPECT_EQ(abilityMs_->ConnectAbilityCommon(want, nullptr, nullptr, extensionType, 100), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ConnectAbilityCommon_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectAbility
 */
HWTEST_F(AbilityManagerServiceSecondTest, DisconnectAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DisconnectAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->DisconnectAbility(nullptr), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DisconnectAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectLocalAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectLocalAbility
 */
HWTEST_F(AbilityManagerServiceSecondTest, ConnectLocalAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ConnectLocalAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ExtensionAbilityType extensionType = ExtensionAbilityType::SERVICE;
    EXPECT_EQ(abilityMs_->ConnectLocalAbility(want, 100, nullptr, nullptr, extensionType), ERR_CROSS_USER);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ConnectLocalAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectRemoteAbility
 */
HWTEST_F(AbilityManagerServiceSecondTest, DisconnectRemoteAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DisconnectRemoteAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->DisconnectRemoteAbility(nullptr), ERR_NULL_OBJECT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DisconnectRemoteAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueMission
 */
HWTEST_F(AbilityManagerServiceSecondTest, ContinueMission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ContinueMission_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string srcDeviceId = "test";
    std::string dstDeviceId = "test";
    AAFwk::WantParams wantParams;
    EXPECT_EQ(abilityMs_->ContinueMission(srcDeviceId, dstDeviceId, 1, nullptr, wantParams),
        CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ContinueMission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueMissionBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueMissionBundleName
 */
HWTEST_F(AbilityManagerServiceSecondTest, ContinueMissionBundleName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ContinueMissionBundleName_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string srcDeviceId = "test";
    std::string dstDeviceId = "test";
    AAFwk::WantParams wantParams;
    EXPECT_EQ(abilityMs_->ContinueMission(srcDeviceId, dstDeviceId, "bundleName", nullptr, wantParams),
        CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ContinueMissionBundleName_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueAbility
 */
HWTEST_F(AbilityManagerServiceSecondTest, ContinueAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ContinueAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    std::string deviceId = "test";
    EXPECT_EQ(abilityMs_->ContinueAbility(deviceId, 1, 1), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ContinueAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartContinuation
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartContinuation
 */
HWTEST_F(AbilityManagerServiceSecondTest, StartContinuation_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartContinuation_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    Want want;
    EXPECT_EQ(abilityMs_->StartContinuation(want, nullptr, 1), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartContinuation_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyCompleteContinuation
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyCompleteContinuation
 */
HWTEST_F(AbilityManagerServiceSecondTest, NotifyCompleteContinuation_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest NotifyCompleteContinuation_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string deviceId = "test";
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->NotifyCompleteContinuation(deviceId, 1, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest NotifyCompleteContinuation_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyContinuationResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyContinuationResult
 */
HWTEST_F(AbilityManagerServiceSecondTest, NotifyContinuationResult_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest NotifyContinuationResult_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(abilityMs_->NotifyContinuationResult(1, 1), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest NotifyContinuationResult_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartSyncRemoteMissions
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartSyncRemoteMissions
 */
HWTEST_F(AbilityManagerServiceSecondTest, StartSyncRemoteMissions_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartSyncRemoteMissions_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string devId = "test";
    EXPECT_EQ(abilityMs_->StartSyncRemoteMissions(devId, true, 1), CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StartSyncRemoteMissions_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopSyncRemoteMissions
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopSyncRemoteMissions
 */
HWTEST_F(AbilityManagerServiceSecondTest, StopSyncRemoteMissions_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StopSyncRemoteMissions_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string devId = "test";
    EXPECT_EQ(abilityMs_->StopSyncRemoteMissions(devId), CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest StopSyncRemoteMissions_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterObserver
 */
HWTEST_F(AbilityManagerServiceSecondTest, RegisterObserver_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest RegisterObserver_001 start");
    AAFwk::IsMockSaCall::IsMockCheckObserverCallerPermission();
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->RegisterObserver(nullptr);
    EXPECT_NE(abilityMs_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest RegisterObserver_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnregisterObserver
 */
HWTEST_F(AbilityManagerServiceSecondTest, UnregisterObserver_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest UnregisterObserver_001 start");
    AAFwk::IsMockSaCall::IsMockCheckObserverCallerPermission();
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->UnregisterObserver(nullptr);
    EXPECT_NE(abilityMs_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest UnregisterObserver_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterMissionListener
 */
HWTEST_F(AbilityManagerServiceSecondTest, RegisterMissionListener_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest RegisterMissionListener_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string deviceId = "test";
    EXPECT_EQ(abilityMs_->RegisterMissionListener(deviceId, nullptr), REGISTER_REMOTE_MISSION_LISTENER_FAIL);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest RegisterMissionListener_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterMissionListener
 */
HWTEST_F(AbilityManagerServiceSecondTest, RegisterMissionListener_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest RegisterMissionListener_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    auto temp_ = abilityMs_->subManagersHelper_->currentMissionListManager_;
    abilityMs_->subManagersHelper_->currentMissionListManager_ = nullptr;
    EXPECT_EQ(abilityMs_->RegisterMissionListener(nullptr), ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest RegisterMissionListener_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UnRegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnRegisterMissionListener
 */
HWTEST_F(AbilityManagerServiceSecondTest, UnRegisterMissionListener_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest UnRegisterMissionListener_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string deviceId = "test";
    EXPECT_EQ(abilityMs_->UnRegisterMissionListener(deviceId, nullptr), REGISTER_REMOTE_MISSION_LISTENER_FAIL);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest UnRegisterMissionListener_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UnRegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnRegisterMissionListener
 */
HWTEST_F(AbilityManagerServiceSecondTest, UnRegisterMissionListener_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest UnRegisterMissionListener_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    auto temp_ = abilityMs_->subManagersHelper_->currentMissionListManager_;
    abilityMs_->subManagersHelper_->currentMissionListManager_ = nullptr;
    EXPECT_EQ(abilityMs_->UnRegisterMissionListener(nullptr), ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest UnRegisterMissionListener_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetWantSender
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSender
 */
HWTEST_F(AbilityManagerServiceSecondTest, GetWantSender_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetWantSender_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    WantSenderInfo wantSenderInfo;
    auto temp = abilityMs_->subManagersHelper_->currentPendingWantManager_;
    abilityMs_->subManagersHelper_->currentPendingWantManager_ = nullptr;
    EXPECT_EQ(abilityMs_->GetWantSender(wantSenderInfo, nullptr), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetWantSender_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SendWantSender
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendWantSender
 */
HWTEST_F(AbilityManagerServiceSecondTest, SendWantSender_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest SendWantSender_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    SenderInfo senderInfo;
    EXPECT_EQ(abilityMs_->SendWantSender(nullptr, senderInfo), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest SendWantSender_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CancelWantSender
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CancelWantSender
 */
HWTEST_F(AbilityManagerServiceSecondTest, CancelWantSender_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CancelWantSender_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->CancelWantSender(nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CancelWantSender_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantUid
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantUid
 */
HWTEST_F(AbilityManagerServiceSecondTest, GetPendingWantUid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetPendingWantUid_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    auto temp = abilityMs_->subManagersHelper_->currentPendingWantManager_;
    abilityMs_->subManagersHelper_->currentPendingWantManager_.reset();
    EXPECT_EQ(abilityMs_->GetPendingWantUid(nullptr), -1);

    abilityMs_->subManagersHelper_->currentPendingWantManager_ = temp;
    EXPECT_EQ(abilityMs_->GetPendingWantUid(nullptr), -1);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetPendingWantUid_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantUserId
 */
HWTEST_F(AbilityManagerServiceSecondTest, GetPendingWantUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetPendingWantUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    auto temp = abilityMs_->subManagersHelper_->currentPendingWantManager_;
    abilityMs_->subManagersHelper_->currentPendingWantManager_.reset();
    EXPECT_EQ(abilityMs_->GetPendingWantUserId(nullptr), -1);

    abilityMs_->subManagersHelper_->currentPendingWantManager_ = temp;
    EXPECT_EQ(abilityMs_->GetPendingWantUserId(nullptr), -1);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetPendingWantUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantBundleName
 */
HWTEST_F(AbilityManagerServiceSecondTest, GetPendingWantBundleName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetPendingWantBundleName_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetPendingWantBundleName(nullptr), "");
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetPendingWantBundleName_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantCode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantCode
 */
HWTEST_F(AbilityManagerServiceSecondTest, GetPendingWantCode_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetPendingWantCode_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    auto temp = abilityMs_->subManagersHelper_->currentPendingWantManager_;
    abilityMs_->subManagersHelper_->currentPendingWantManager_.reset();
    EXPECT_EQ(abilityMs_->GetPendingWantCode(nullptr), -1);

    abilityMs_->subManagersHelper_->currentPendingWantManager_ = temp;
    EXPECT_EQ(abilityMs_->GetPendingWantCode(nullptr), -1);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetPendingWantCode_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantType
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantType
 */
HWTEST_F(AbilityManagerServiceSecondTest, GetPendingWantType_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetPendingWantType_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    auto temp = abilityMs_->subManagersHelper_->currentPendingWantManager_;
    abilityMs_->subManagersHelper_->currentPendingWantManager_.reset();
    EXPECT_EQ(abilityMs_->GetPendingWantType(nullptr), -1);

    abilityMs_->subManagersHelper_->currentPendingWantManager_ = temp;
    EXPECT_EQ(abilityMs_->GetPendingWantType(nullptr), -1);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetPendingWantType_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterCancelListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterCancelListener
 */
HWTEST_F(AbilityManagerServiceSecondTest, RegisterCancelListener_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest RegisterCancelListener_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->RegisterCancelListener(nullptr, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest RegisterCancelListener_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UnregisterCancelListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnregisterCancelListener
 */
HWTEST_F(AbilityManagerServiceSecondTest, UnregisterCancelListener_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest UnregisterCancelListener_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->UnregisterCancelListener(nullptr, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest UnregisterCancelListener_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingRequestWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingRequestWant
 */
HWTEST_F(AbilityManagerServiceSecondTest, GetPendingRequestWant_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetPendingRequestWant_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::shared_ptr<Want> want;
    EXPECT_EQ(abilityMs_->GetPendingRequestWant(nullptr, want), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetPendingRequestWant_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: LockMissionForCleanup
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService LockMissionForCleanup
 */
HWTEST_F(AbilityManagerServiceSecondTest, LockMissionForCleanup_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest LockMissionForCleanup_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_->LockMissionForCleanup(1), CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest LockMissionForCleanup_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UnlockMissionForCleanup
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnlockMissionForCleanup
 */
HWTEST_F(AbilityManagerServiceSecondTest, UnlockMissionForCleanup_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest UnlockMissionForCleanup_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_->UnlockMissionForCleanup(1), CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest UnlockMissionForCleanup_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionInfos
 */
HWTEST_F(AbilityManagerServiceSecondTest, GetMissionInfos_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetMissionInfos_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<MissionInfo> missionInfos;
    EXPECT_NE(abilityMs_->GetMissionInfos("", 10, missionInfos), CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetMissionInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetRemoteMissionInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetRemoteMissionInfos
 */
HWTEST_F(AbilityManagerServiceSecondTest, GetRemoteMissionInfos_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetRemoteMissionInfos_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<MissionInfo> missionInfos;
    EXPECT_EQ(abilityMs_->GetRemoteMissionInfos("", 10, missionInfos), INVALID_PARAMETERS_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetRemoteMissionInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionInfo
 */
HWTEST_F(AbilityManagerServiceSecondTest, GetMissionInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetMissionInfo_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    MissionInfo missionInfo;
    EXPECT_NE(abilityMs_->GetMissionInfo("", 10, missionInfo), CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetMissionInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetRemoteMissionInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetRemoteMissionInfo
 */
HWTEST_F(AbilityManagerServiceSecondTest, GetRemoteMissionInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetRemoteMissionInfo_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    MissionInfo missionInfo;
    EXPECT_EQ(abilityMs_->GetRemoteMissionInfo("", 10, missionInfo), INVALID_PARAMETERS_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetRemoteMissionInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CleanMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CleanMission
 */
HWTEST_F(AbilityManagerServiceSecondTest, CleanMission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CleanMission_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_->CleanMission(1), CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CleanMission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToFront
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToFront
 */
HWTEST_F(AbilityManagerServiceSecondTest, MoveMissionToFront_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest MoveMissionToFront_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->MoveMissionToFront(100), CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest MoveMissionToFront_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToFront
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToFront
 */
HWTEST_F(AbilityManagerServiceSecondTest, MoveMissionToFront_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest MoveMissionToFront_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    StartOptions startOptions;
    EXPECT_EQ(abilityMs_->MoveMissionToFront(100, startOptions), CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest MoveMissionToFront_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToFront
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToFront
 */
HWTEST_F(AbilityManagerServiceSecondTest, MoveMissionToFront_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest MoveMissionToFront_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_ != nullptr);
    StartOptions startOptions;
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(abilityMs_->MoveMissionToFront(100, startOptions), CHECK_PERMISSION_FAILED);
    }
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest MoveMissionToFront_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionsToForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToFront
 */
HWTEST_F(AbilityManagerServiceSecondTest, MoveMissionsToForeground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest MoveMissionsToForeground_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->MoveMissionsToForeground({1, 2, 3}, 1), CHECK_PERMISSION_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest MoveMissionsToForeground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionsToBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToFront
 */
HWTEST_F(AbilityManagerServiceSecondTest, MoveMissionsToBackground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest MoveMissionsToBackground_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<int32_t> rs;
    EXPECT_EQ(abilityMs_->MoveMissionsToBackground({1, 2, 3}, rs), CHECK_PERMISSION_FAILED);
    EXPECT_TRUE(rs.empty());
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest MoveMissionsToBackground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsAbilityControllerStartById
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsAbilityControllerStartById
 */
HWTEST_F(AbilityManagerServiceSecondTest, IsAbilityControllerStartById_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest IsAbilityControllerStartById_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_TRUE(abilityMs_->IsAbilityControllerStartById(1));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest IsAbilityControllerStartById_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetConnectRecordListByCallback
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectRecordListByCallback
 */
HWTEST_F(AbilityManagerServiceSecondTest, GetConnectRecordListByCallback_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetConnectRecordListByCallback_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    auto temp = abilityMs_->subManagersHelper_->currentConnectManager_;
    abilityMs_->subManagersHelper_->currentConnectManager_.reset();
    std::list<std::shared_ptr<ConnectionRecord>> connectList;
    EXPECT_EQ(abilityMs_->GetConnectRecordListByCallback(nullptr), connectList);
    abilityMs_->subManagersHelper_->currentConnectManager_ = temp;
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetConnectRecordListByCallback_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 */
HWTEST_F(AbilityManagerServiceSecondTest, AcquireDataAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest AcquireDataAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Uri uri("test");
    EXPECT_EQ(abilityMs_->AcquireDataAbility(uri, true, nullptr), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest AcquireDataAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 */
HWTEST_F(AbilityManagerServiceSecondTest, ReleaseDataAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ReleaseDataAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->ReleaseDataAbility(nullptr, nullptr), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ReleaseDataAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AttachAbilityThread
 */
HWTEST_F(AbilityManagerServiceSecondTest, AttachAbilityThread_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest AttachAbilityThread_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(abilityMs_->AttachAbilityThread(nullptr, nullptr), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest AttachAbilityThread_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpInner
 */
HWTEST_F(AbilityManagerServiceSecondTest, DumpInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DumpInner_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    std::vector<std::string> info;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->DumpInner("", info);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DumpInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpMissionListInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpMissionListInner
 */
HWTEST_F(AbilityManagerServiceSecondTest, DumpMissionListInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DumpMissionListInner_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    std::vector<std::string> info;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->DumpMissionListInner("", info);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DumpMissionListInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpMissionInfosInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpMissionInfosInner
 */
HWTEST_F(AbilityManagerServiceSecondTest, DumpMissionInfosInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DumpMissionInfosInner_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    std::vector<std::string> info;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->DumpMissionInfosInner("", info);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DumpMissionInfosInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpMissionInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpMissionInner
 */
HWTEST_F(AbilityManagerServiceSecondTest, DumpMissionInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DumpMissionInner_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    std::vector<std::string> info;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->DumpMissionInner("", info);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DumpMissionInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpStateInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpStateInner
 */
HWTEST_F(AbilityManagerServiceSecondTest, DumpStateInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DumpStateInner_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    std::vector<std::string> info;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->DumpStateInner("", info);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DumpStateInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DataDumpStateInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DataDumpStateInner
 */
HWTEST_F(AbilityManagerServiceSecondTest, DataDumpStateInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DataDumpStateInner_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<std::string> info;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->DataDumpStateInner("", info);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DataDumpStateInner_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpState
 */
HWTEST_F(AbilityManagerServiceSecondTest, DumpState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DumpState_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<std::string> info;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->DumpState("", info);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DumpState_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DumpSysState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpSysState
 */
HWTEST_F(AbilityManagerServiceSecondTest, DumpSysState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DumpSysState_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<std::string> info;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->DumpSysState("", info, true, true, 100);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest DumpSysState_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AbilityTransitionDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AbilityTransitionDone
 */
HWTEST_F(AbilityManagerServiceSecondTest, AbilityTransitionDone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest AbilityTransitionDone_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    PacMap saveData;
    EXPECT_EQ(abilityMs_->AbilityTransitionDone(nullptr, 1, saveData), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest AbilityTransitionDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnAppStateChanged
 */
HWTEST_F(AbilityManagerServiceSecondTest, OnAppStateChanged_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest OnAppStateChanged_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    abilityMs_->subManagersHelper_->currentUIAbilityManager_ = std::make_shared<UIAbilityLifecycleManager>();
    AppInfo info;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->OnAppStateChanged(info);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest OnAppStateChanged_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetTaskHandler
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetTaskHandler
 */
HWTEST_F(AbilityManagerServiceSecondTest, GetTaskHandler_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetTaskHandler_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    EXPECT_EQ(abilityMs_->GetTaskHandler(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetTaskHandler_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetEventHandler
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetEventHandler
 */
HWTEST_F(AbilityManagerServiceSecondTest, GetEventHandler_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetEventHandler_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs_, nullptr);
    EXPECT_EQ(abilityMs_->GetEventHandler(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetEventHandler_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetUserId
 */
HWTEST_F(AbilityManagerServiceSecondTest, GetUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetUserId_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_->GetUserId(), 100);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GetUserId_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GenerateAbilityRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GenerateAbilityRequest
 */
HWTEST_F(AbilityManagerServiceSecondTest, GenerateAbilityRequest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GenerateAbilityRequest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    AbilityRequest request;
    EXPECT_EQ(abilityMs_->GenerateAbilityRequest(want, 1, request, nullptr, 100), RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GenerateAbilityRequest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GenerateExtensionAbilityRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GenerateExtensionAbilityRequest
 */
HWTEST_F(AbilityManagerServiceSecondTest, GenerateExtensionAbilityRequest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GenerateExtensionAbilityRequest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    AbilityRequest request;
    EXPECT_EQ(abilityMs_->GenerateExtensionAbilityRequest(want, request, nullptr, 100), RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest GenerateExtensionAbilityRequest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: KillProcess
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillProcess
 */
HWTEST_F(AbilityManagerServiceSecondTest, KillProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest KillProcess_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->KillProcess("test"), GET_BUNDLE_INFO_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest KillProcess_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ClearUpApplicationData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ClearUpApplicationData
 */
HWTEST_F(AbilityManagerServiceSecondTest, ClearUpApplicationData_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ClearUpApplicationData_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->ClearUpApplicationData("test"), CLEAR_APPLICATION_DATA_FAIL);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ClearUpApplicationData_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: PreLoadAppDataAbilities
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService PreLoadAppDataAbilities
 */
HWTEST_F(AbilityManagerServiceSecondTest, PreLoadAppDataAbilities_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest PreLoadAppDataAbilities_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->PreLoadAppDataAbilities("", 100), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->PreLoadAppDataAbilities("test", -1), ERR_INVALID_STATE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest PreLoadAppDataAbilities_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsSystemUiApp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsSystemUiApp
 */
HWTEST_F(AbilityManagerServiceSecondTest, IsSystemUiApp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest IsSystemUiApp_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
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
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest IsSystemUiApp_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsSystemUI
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsSystemUI
 */
HWTEST_F(AbilityManagerServiceSecondTest, IsSystemUI_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest IsSystemUI_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::string bundleName = "bundleName";
    EXPECT_FALSE(abilityMs_->IsSystemUI(bundleName));

    bundleName = AbilityConfig::SYSTEM_UI_BUNDLE_NAME;
    EXPECT_TRUE(abilityMs_->IsSystemUI(bundleName));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest IsSystemUI_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckSenderWantInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSenderWantInfo
 */
HWTEST_F(AbilityManagerServiceSecondTest, CheckSenderWantInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckSenderWantInfo_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.bundleName = "badeName";
    int32_t callerUid = 0;
    EXPECT_TRUE(abilityMs_->CheckSenderWantInfo(callerUid, wantSenderInfo));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest CheckSenderWantInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ParseJsonFromBoot
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ParseJsonFromBoot
 */
HWTEST_F(AbilityManagerServiceSecondTest, ParseJsonFromBoot_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ParseJsonFromBoot_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    const std::string file = "/etc/efficiency_manager/prevent_startability_whitelist.json";
    nlohmann::json whiteListJsonObj;
    abilityMs_->ParseJsonFromBoot(whiteListJsonObj, file, "white_list");
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ParseJsonFromBoot_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsInWhiteList
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsInWhiteList
 */
HWTEST_F(AbilityManagerServiceSecondTest, IsInWhiteList_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest IsInWhiteList_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    const std::string callerBundleName = "com.example.amstest";
    const std::string calleeBundleName = "com.exmple.testserviceextensionability";
    const std::string calleeAbilityName = "ServiceextensionAbility";
    abilityMs_->IsInWhiteList(callerBundleName, calleeBundleName, calleeAbilityName);
    EXPECT_TRUE(abilityMs_ != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest IsInWhiteList_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ShouldPreventStartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ShouldPreventStartAbility
 */
HWTEST_F(AbilityManagerServiceSecondTest, ShouldPreventStartAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ShouldPreventStartAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    const std::string deviceName = "";
    const std::string abilityName = "EntryAbility";
    const std::string appName = "amstest";
    const std::string bundleName = "com.example.amstest";
    const std::string moduleName = "entry";
    AbilityRequest abilityRequest = AbilityManagerServiceSecondTest::GenerateAbilityRequest(deviceName,
        abilityName, appName, bundleName, moduleName);
    EXPECT_FALSE(abilityMs_->ShouldPreventStartAbility(abilityRequest));
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSecondTest ShouldPreventStartAbility_001 end");
}
}  // namespace AAFwk
}  // namespace OHOS
