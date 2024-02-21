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
#include "ability_connect_manager.h"
#include "ability_connection.h"
#include "ability_start_setting.h"
#include "recovery_param.h"
#undef private
#undef protected
#include "ability_manager_errors.h"
#include "connection_observer_errors.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "mock_ability_controller.h"
#include "mock_app_debug_listener_stub.h"
#include "session/host/include/session.h"
#include "mock_ability_manager_collaborator.h"
#include "mock_prepare_terminate_callback.h"
#include "mock_sa_call.h"
#include "scene_board_judgement.h"

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
class AbilityManagerServiceFirstTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    sptr<Token> MockToken(AbilityType);
    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);

public:
    AbilityRequest abilityRequest_{};
    Want want_{};
};

std::shared_ptr<AbilityRecord> AbilityManagerServiceFirstTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> AbilityManagerServiceFirstTest::MockToken(AbilityType abilityType)
{
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

sptr<SessionInfo> AbilityManagerServiceFirstTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        HILOG_ERROR("sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

void AbilityManagerServiceFirstTest::SetUpTestCase() {}

void AbilityManagerServiceFirstTest::TearDownTestCase() {}

void AbilityManagerServiceFirstTest::SetUp() {}

void AbilityManagerServiceFirstTest::TearDown() {}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartExtensionAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StartExtensionAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    auto ret = abilityMs_->StartExtensionAbility(want, nullptr, 0, ExtensionAbilityType::SERVICE);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    ret = abilityMs_->StartExtensionAbility(want, MockToken(AbilityType::PAGE), 100, ExtensionAbilityType::SERVICE);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    ret = abilityMs_->StartExtensionAbility(want, MockToken(AbilityType::PAGE), -1, ExtensionAbilityType::SERVICE);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    ret = abilityMs_->StartExtensionAbility(want, nullptr, 100, ExtensionAbilityType::SERVICE);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);

    HILOG_INFO("AbilityManagerServiceFirstTest StartExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckOptExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckOptExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckOptExtensionAbility_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    HILOG_INFO("AbilityManagerServiceFirstTest CheckOptExtensionAbility_001 start");
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
    HILOG_INFO("AbilityManagerServiceFirstTest CheckOptExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallAbilityPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallAbilityPermission
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckCallAbilityPermission_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    HILOG_INFO("AbilityManagerServiceFirstTest CheckCallAbilityPermission_001 start");
    AbilityRequest request;
    EXPECT_TRUE(abilityMs_->startUpNewRule_);
    abilityMs_->startUpNewRule_ = false;
    request.abilityInfo.visible = true;
    EXPECT_EQ(abilityMs_->CheckCallAbilityPermission(abilityRequest_), ERR_INVALID_VALUE);

    abilityMs_->startUpNewRule_ = true;
    EXPECT_EQ(abilityMs_->CheckCallAbilityPermission(abilityRequest_), ERR_INVALID_VALUE);

    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->CheckCallAbilityPermission(abilityRequest_), 1);
    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceFirstTest CheckCallAbilityPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckCallServicePermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckCallServicePermission
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckCallServicePermission_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    HILOG_INFO("AbilityManagerServiceFirstTest CheckCallServicePermission_001 start");
    AbilityRequest request;
    abilityMs_->startUpNewRule_ = false;
    EXPECT_FALSE(abilityMs_->startUpNewRule_);
    request.abilityInfo.visible = true;
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(request), ERR_INVALID_VALUE);

    abilityMs_->startUpNewRule_ = true;
    request.abilityInfo.isStageBasedModel = false;
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(request), ERR_INVALID_VALUE);

    request.abilityInfo.isStageBasedModel = true;
    request.abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(request), ERR_OK);

    request.abilityInfo.extensionAbilityType = ExtensionAbilityType::DATASHARE;
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(request), ERR_OK);

    request.abilityInfo.extensionAbilityType = ExtensionAbilityType::FILESHARE;
    EXPECT_EQ(abilityMs_->CheckCallServicePermission(request), CHECK_PERMISSION_FAILED);
    abilityMs_->startUpNewRule_ = false;
    HILOG_INFO("AbilityManagerServiceFirstTest CheckCallServicePermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckStartByCallPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckStartByCallPermission
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckStartByCallPermission_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    HILOG_INFO("AbilityManagerServiceFirstTest CheckStartByCallPermission_002 start");
    abilityRequest_.abilityInfo.type = AbilityType::PAGE;
    EXPECT_EQ(abilityMs_->CheckStartByCallPermission(abilityRequest_), ERR_INVALID_VALUE);

    abilityRequest_.abilityInfo.type = AbilityType::DATA;
    EXPECT_EQ(abilityMs_->CheckStartByCallPermission(abilityRequest_), RESOLVE_CALL_ABILITY_TYPE_ERR);
    HILOG_INFO("AbilityManagerServiceFirstTest CheckStartByCallPermission_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetDlpConnectionInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDlpConnectionInfos
 */
HWTEST_F(AbilityManagerServiceFirstTest, GetDlpConnectionInfos_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest GetDlpConnectionInfos_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    std::vector<AbilityRuntime::DlpConnectionInfo> infos;
    EXPECT_EQ(abilityMs_->GetDlpConnectionInfos(infos), CHECK_PERMISSION_FAILED);

    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->GetDlpConnectionInfos(infos), ERR_OK);

    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceFirstTest GetDlpConnectionInfos_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterSnapshotHandler
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterSnapshotHandler
 */
HWTEST_F(AbilityManagerServiceFirstTest, RegisterSnapshotHandler_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest RegisterSnapshotHandler_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->RegisterSnapshotHandler(nullptr), 0);

    auto temp = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_.reset();
    EXPECT_EQ(abilityMs_->RegisterSnapshotHandler(nullptr), ERR_OK);
    HILOG_INFO("AbilityManagerServiceFirstTest RegisterSnapshotHandler_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionSnapshot
 */
HWTEST_F(AbilityManagerServiceFirstTest, GetMissionSnapshot_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest GetMissionSnapshot_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    MissionSnapshot missionSnapshot;
    EXPECT_EQ(abilityMs_->GetMissionSnapshot("", 1, missionSnapshot, true), CHECK_PERMISSION_FAILED);

    MyFlag::flag_ = 1;
    auto temp = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_.reset();
    EXPECT_EQ(abilityMs_->GetMissionSnapshot("", 1, missionSnapshot, true), INNER_ERR);
    abilityMs_->currentMissionListManager_ = temp;

    EXPECT_EQ(abilityMs_->GetMissionSnapshot("", 1, missionSnapshot, true), INNER_ERR);
    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceFirstTest GetMissionSnapshot_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SetAbilityController
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetAbilityController
 */
HWTEST_F(AbilityManagerServiceFirstTest, SetAbilityController_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest SetAbilityController_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->SetAbilityController(nullptr, true), CHECK_PERMISSION_FAILED);

    auto temp1 = abilityMs_->abilityController_;
    auto temp2 = abilityMs_->controllerIsAStabilityTest_;
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->SetAbilityController(nullptr, true), ERR_OK);
    abilityMs_->abilityController_ = temp1;
    abilityMs_->controllerIsAStabilityTest_ = temp2;
    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceFirstTest SetAbilityController_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SendANRProcessID
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendANRProcessID
 */
HWTEST_F(AbilityManagerServiceFirstTest, SendANRProcessID_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest SendANRProcessID_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    MyFlag::flag_ = 0;
    EXPECT_EQ(abilityMs_->SendANRProcessID(100), CHECK_PERMISSION_FAILED);

    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->SendANRProcessID(100), ERR_INVALID_VALUE);

    MyFlag::flag_ = 2;
    EXPECT_EQ(abilityMs_->SendANRProcessID(100), ERR_INVALID_VALUE);

    MyFlag::flag_ = 3;
    EXPECT_EQ(abilityMs_->SendANRProcessID(100), ERR_INVALID_VALUE);

    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceFirstTest SendANRProcessID_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckStaticCfgPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckStaticCfgPermission
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckStaticCfgPermission_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest CheckStaticCfgPermission_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AppExecFwk::AbilityInfo abilityInfo;
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityInfo, false, -1),
        AppExecFwk::Constants::PERMISSION_GRANTED);

    MyFlag::flag_ = 0;
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityInfo, false, -1),
        AppExecFwk::Constants::PERMISSION_GRANTED);

    abilityInfo.applicationInfo.accessTokenId = 0;
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityInfo, false, -1), ERR_OK);

    // abilityInfo.permissions is empty
    abilityInfo.applicationInfo.accessTokenId = -1;
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityInfo, false, -1),
        AppExecFwk::Constants::PERMISSION_GRANTED);

    // abilityInfo.permissions is not empty
    abilityInfo.permissions.push_back("test1");
    abilityInfo.permissions.push_back("test2");
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityInfo, false, -1),
        AppExecFwk::Constants::PERMISSION_NOT_GRANTED);

    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::DATASHARE;
    abilityInfo.readPermission = "test";
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityInfo, false, -1),
        AppExecFwk::Constants::PERMISSION_NOT_GRANTED);

    abilityInfo.readPermission.clear();
    abilityInfo.writePermission = "test";
    EXPECT_EQ(abilityMs_->CheckStaticCfgPermission(abilityInfo, false, -1),
        AppExecFwk::Constants::PERMISSION_NOT_GRANTED);
    HILOG_INFO("AbilityManagerServiceFirstTest CheckStaticCfgPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AddStartControlParam
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AddStartControlParam
 */
HWTEST_F(AbilityManagerServiceFirstTest, AddStartControlParam_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest AddStartControlParam_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->AddStartControlParam(want, nullptr), ERR_OK);

    MyFlag::flag_ = 2;
    EXPECT_EQ(abilityMs_->AddStartControlParam(want, nullptr), ERR_OK);

    MyFlag::flag_ = 0;
    EXPECT_EQ(abilityMs_->AddStartControlParam(want, nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest AddStartControlParam_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StopServiceAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StopServiceAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_EQ(abilityMs_->StopServiceAbility(want, 100), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest StopServiceAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StopExtensionAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StopExtensionAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, nullptr, USER_ID_U100, ExtensionAbilityType::SERVICE),
        CHECK_PERMISSION_FAILED);

    auto callerToken = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, callerToken, USER_ID_U100, ExtensionAbilityType::SERVICE),
        CHECK_PERMISSION_FAILED);

    HILOG_INFO("AbilityManagerServiceFirstTest StopExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbilityWithFlag
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityWithFlag
 */
HWTEST_F(AbilityManagerServiceFirstTest, TerminateAbilityWithFlag_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest TerminateAbilityWithFlag_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want* resultWant = nullptr;
    EXPECT_EQ(abilityMs_->TerminateAbilityWithFlag(nullptr, 1, resultWant, true), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->TerminateAbilityWithFlag(MockToken(AbilityType::PAGE), 1, resultWant, true),
        ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest TerminateAbilityWithFlag_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartRemoteAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartRemoteAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StartRemoteAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    Want want;
    // AddStartControlParam
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->StartRemoteAbility(want, 1, 1, callerToken), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest StartRemoteAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartRemoteAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartRemoteAbility_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StartRemoteAbility_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    Want want;
    // AddStartControlParam
    EXPECT_EQ(abilityMs_->StartRemoteAbility(want, 1, 1, nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest StartRemoteAbility_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartRemoteAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartRemoteAbility_003, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StartRemoteAbility_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    Want want;
    MyFlag::flag_ = 1;
    unsigned int flag = 0x00000800;
    want.SetFlags(flag);
    auto temp = abilityMs_->freeInstallManager_;
    abilityMs_->freeInstallManager_.reset();
    EXPECT_EQ(abilityMs_->StartRemoteAbility(want, 1, 1, nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest StartRemoteAbility_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartRemoteAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartRemoteAbility_004, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StartRemoteAbility_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    Want want;
    want.SetFlags(0);
    want.SetParam("ohos.aafwk.param.startAbilityForResult", true);
    EXPECT_EQ(abilityMs_->StartRemoteAbility(want, 1, 1, nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest StartRemoteAbility_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, MinimizeAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest MinimizeAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->MinimizeAbility(nullptr, true), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->MinimizeAbility(MockToken(AbilityType::PAGE), true), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest MinimizeAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeUIAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceFirstTest, MinimizeUIAbilityBySCB_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest MinimizeUIAbilityBySCB_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(abilityMs_->MinimizeUIAbilityBySCB(nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest MinimizeUIAbilityBySCB_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeUIAbilityBySCB
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeUIAbilityBySCB
 */
HWTEST_F(AbilityManagerServiceFirstTest, MinimizeUIAbilityBySCB_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest MinimizeUIAbilityBySCB_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    EXPECT_EQ(abilityMs_->MinimizeUIAbilityBySCB(sessionInfo), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest MinimizeUIAbilityBySCB_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionIdByToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionIdByToken
 */
HWTEST_F(AbilityManagerServiceFirstTest, GetMissionIdByToken_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest GetMissionIdByToken_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    EXPECT_EQ(abilityMs_->GetMissionIdByToken(nullptr), ERR_INVALID_VALUE);
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_EQ(abilityMs_->GetMissionIdByToken(token), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest GetMissionIdByToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetServiceRecordByElementName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetServiceRecordByElementName
 */
HWTEST_F(AbilityManagerServiceFirstTest, GetServiceRecordByElementName_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest GetServiceRecordByElementName_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->GetServiceRecordByElementName(""), nullptr);
    abilityMs_->connectManager_ = std::make_shared<AbilityConnectManager>(100);
    abilityMs_->connectManager_->serviceMap_.insert({"test", MockAbilityRecord(AbilityType::PAGE)});
    EXPECT_NE(abilityMs_->GetServiceRecordByElementName("test"), nullptr);
    HILOG_INFO("AbilityManagerServiceFirstTest GetServiceRecordByElementName_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDone
 */
HWTEST_F(AbilityManagerServiceFirstTest, ScheduleConnectAbilityDone_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest ScheduleConnectAbilityDone_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->ScheduleConnectAbilityDone(nullptr, nullptr), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->ScheduleConnectAbilityDone(MockToken(AbilityType::PAGE), nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest ScheduleConnectAbilityDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityDone
 */
HWTEST_F(AbilityManagerServiceFirstTest, ScheduleCommandAbilityDone_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest ScheduleCommandAbilityDone_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->ScheduleCommandAbilityDone(nullptr), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->ScheduleCommandAbilityDone(MockToken(AbilityType::PAGE)), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest ScheduleCommandAbilityDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OnAbilityRequestDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnAbilityRequestDone
 */
HWTEST_F(AbilityManagerServiceFirstTest, OnAbilityRequestDone_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest OnAbilityRequestDone_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->OnAbilityRequestDone(nullptr, 1);
    abilityMs_->OnAbilityRequestDone(MockToken(AbilityType::DATA), 1);
    abilityMs_->OnAbilityRequestDone(MockToken(AbilityType::UNKNOWN), 1);
    HILOG_INFO("AbilityManagerServiceFirstTest OnAbilityRequestDone_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionIdByAbilityToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionIdByAbilityToken
 */
HWTEST_F(AbilityManagerServiceFirstTest, GetMissionIdByAbilityToken_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest GetMissionIdByAbilityToken_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(abilityMs_->GetMissionIdByAbilityToken(nullptr), -1);

    EXPECT_EQ(abilityMs_->GetMissionIdByAbilityToken(MockToken(AbilityType::PAGE)), -1);
    HILOG_INFO("AbilityManagerServiceFirstTest GetMissionIdByAbilityToken_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: EnableRecoverAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService EnableRecoverAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, EnableRecoverAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest EnableRecoverAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->EnableRecoverAbility(nullptr);
    abilityMs_->EnableRecoverAbility(MockToken(AbilityType::PAGE));
    HILOG_INFO("AbilityManagerServiceFirstTest EnableRecoverAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RecoverAbilityRestart
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RecoverAbilityRestart
 */
HWTEST_F(AbilityManagerServiceFirstTest, RecoverAbilityRestart_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest RecoverAbilityRestart_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->RecoverAbilityRestart(want);
    HILOG_INFO("AbilityManagerServiceFirstTest RecoverAbilityRestart_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUserTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUserTest
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartUserTest_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StartUserTest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_EQ(abilityMs_->StartUserTest(want, nullptr), ERR_INVALID_VALUE);

    sptr<IRemoteObject> observer = MockToken(AbilityType::PAGE);
    EXPECT_EQ(abilityMs_->StartUserTest(want, nullptr), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest StartUserTest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: FinishUserTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService FinishUserTest
 */
HWTEST_F(AbilityManagerServiceFirstTest, FinishUserTest_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest FinishUserTest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->FinishUserTest("", 1, ""), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->FinishUserTest("", 1, "test"), INNER_ERR);
    HILOG_INFO("AbilityManagerServiceFirstTest FinishUserTest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DelegatorDoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceFirstTest, DelegatorDoAbilityForeground_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest DelegatorDoAbilityForeground_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(abilityMs_->DelegatorDoAbilityForeground(nullptr), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->DelegatorDoAbilityForeground(MockToken(AbilityType::PAGE)), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest DelegatorDoAbilityForeground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DelegatorDoAbilityBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityBackground
 */
HWTEST_F(AbilityManagerServiceFirstTest, DelegatorDoAbilityBackground_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest DelegatorDoAbilityBackground_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(abilityMs_->DelegatorDoAbilityBackground(nullptr), ERR_INVALID_VALUE);
    }
    HILOG_INFO("AbilityManagerServiceFirstTest DelegatorDoAbilityBackground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityForeground
 */
HWTEST_F(AbilityManagerServiceFirstTest, DoAbilityForeground_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest DoAbilityForeground_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->DoAbilityForeground(nullptr, 1), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->DoAbilityForeground(MockToken(AbilityType::PAGE), 1), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest DoAbilityForeground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityBackground
 */
HWTEST_F(AbilityManagerServiceFirstTest, DoAbilityBackground_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest DoAbilityBackground_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->DoAbilityBackground(nullptr, 1), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->DoAbilityBackground(MockToken(AbilityType::PAGE), 1), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest DoAbilityBackground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsAbilityControllerStart
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsAbilityControllerStart
 */
HWTEST_F(AbilityManagerServiceFirstTest, IsAbilityControllerStart_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest IsAbilityControllerStart_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
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
    HILOG_INFO("AbilityManagerServiceFirstTest IsAbilityControllerStart_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsAbilityControllerForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsAbilityControllerForeground
 */
HWTEST_F(AbilityManagerServiceFirstTest, IsAbilityControllerForeground_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
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
    HILOG_INFO("AbilityManagerServiceFirstTest IsAbilityControllerForeground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: DelegatorMoveMissionToFront
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorMoveMissionToFront
 */
HWTEST_F(AbilityManagerServiceFirstTest, DelegatorMoveMissionToFront_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest DelegatorMoveMissionToFront_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    auto temp = abilityMs_->currentMissionListManager_;
    abilityMs_->currentMissionListManager_.reset();
    EXPECT_EQ(abilityMs_->DelegatorMoveMissionToFront(1), ERR_NO_INIT);
    HILOG_INFO("AbilityManagerServiceFirstTest DelegatorMoveMissionToFront_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateCallerInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateCallerInfo
 */
HWTEST_F(AbilityManagerServiceFirstTest, UpdateCallerInfo_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest UpdateCallerInfo_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    sptr<IRemoteObject> callerToken = MockToken(AbilityType::PAGE);
    ASSERT_NE(abilityMs_, nullptr);
    abilityMs_->UpdateCallerInfo(want, callerToken);
    HILOG_INFO("AbilityManagerServiceFirstTest UpdateCallerInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: IsCallFromBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsCallFromBackground
 */
HWTEST_F(AbilityManagerServiceFirstTest, IsCallFromBackground_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest IsCallFromBackground_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
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
    HILOG_INFO("AbilityManagerServiceFirstTest IsCallFromBackground_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartUIExtensionAbility_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_EQ(abilityMs_->StartUIExtensionAbility(nullptr, 100),
        ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartUIExtensionAbility_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_EQ(abilityMs_->StartUIExtensionAbility(MockSessionInfo(0), USER_ID_U100), CHECK_PERMISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, TerminateUIExtensionAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest TerminateUIExtensionAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want* resultWant = nullptr;
    EXPECT_EQ(abilityMs_->TerminateUIExtensionAbility(nullptr, 1, resultWant), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->TerminateUIExtensionAbility(MockSessionInfo(0), 1, resultWant), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest TerminateUIExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeUIExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeUIExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, MinimizeUIExtensionAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest MinimizeUIExtensionAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->MinimizeUIExtensionAbility(nullptr, true), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->MinimizeUIExtensionAbility(nullptr, false), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->MinimizeUIExtensionAbility(MockSessionInfo(0), true), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->MinimizeUIExtensionAbility(MockSessionInfo(0), false), ERR_INVALID_VALUE);
    HILOG_INFO("AbilityManagerServiceFirstTest MinimizeUIExtensionAbility_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StopExtensionAbility_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StopExtensionAbility_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want{};
    ElementName element("device", "com.ix.hiservcie", "ServiceAbility", "entry");
    want.SetElement(element);
    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityRecord->appIndex_ = -1;
    abilityRecord->applicationInfo_.bundleName = "com.ix.hiservcie";
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, abilityRecord->GetToken(), -1, ExtensionAbilityType::SERVICE),
        ERR_INVALID_CALLER);
    HILOG_INFO("AbilityManagerServiceFirstTest StopExtensionAbility_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StopExtensionAbility_003, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StopExtensionAbility_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want{};
    ElementName element("device", "com.ix.hiservcie", "ServiceAbility", "entry");
    want.SetElement(element);
    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityRecord->appIndex_ = -1;
    abilityRecord->applicationInfo_.bundleName = "com.ix.hiservcie";
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, abilityRecord->GetToken(), -1, ExtensionAbilityType::SERVICE),
        RESOLVE_ABILITY_ERR);
    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceFirstTest StopExtensionAbility_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StopExtensionAbility_004, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StopExtensionAbility_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want{};
    ElementName element("", "com.ix.hiservcie", "ServiceAbility", "entry");
    want.SetElement(element);
    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityRecord->appIndex_ = -1;
    abilityRecord->applicationInfo_.bundleName = "com.ix.hiservcie";
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, nullptr, -1, ExtensionAbilityType::SERVICE),
        RESOLVE_ABILITY_ERR);
    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceFirstTest StopExtensionAbility_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StopExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, StopExtensionAbility_005, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StopExtensionAbility_005 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want{};
    ElementName element("", "com.ix.hiservcie", "ServiceAbility", "entry");
    want.SetElement(element);
    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityRecord->appIndex_ = -1;
    abilityRecord->applicationInfo_.bundleName = "com.ix.hiservcie";
    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->StopExtensionAbility(want, abilityRecord->GetToken(), -1, ExtensionAbilityType::SERVICE),
        RESOLVE_ABILITY_ERR);
    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceFirstTest StopExtensionAbility_005 end");
}

/*
 * Feature: AbilityManagerService
 * Function: RecordAppExitReason
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RecordAppExitReason
 */
HWTEST_F(AbilityManagerServiceFirstTest, RecordAppExitReason_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest RecordAppExitReason_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(abilityMs_->RecordAppExitReason(REASON_JS_ERROR), ERR_NULL_OBJECT);
    }
    HILOG_INFO("AbilityManagerServiceFirstTest RecordAppExitReason_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ForceExitApp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ForceExitApp
 */
HWTEST_F(AbilityManagerServiceFirstTest, ForceExitApp_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest ForceExitApp_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    int32_t pid = 0;
    EXPECT_EQ(abilityMs_->ForceExitApp(pid, REASON_JS_ERROR), ERR_PERMISSION_DENIED);
    HILOG_INFO("AbilityManagerServiceFirstTest ForceExitApp_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityDone
 * @tc.require: AR000I7F9D
 */
HWTEST_F(AbilityManagerServiceFirstTest, ScheduleCommandAbilityWindowDone_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    sptr<SessionInfo> session = new (std::nothrow) SessionInfo();
    EXPECT_EQ(abilityMs_->ScheduleCommandAbilityWindowDone(
        nullptr, session, WIN_CMD_FOREGROUND, ABILITY_CMD_FOREGROUND), ERR_INVALID_VALUE);
    EXPECT_EQ(abilityMs_->ScheduleCommandAbilityWindowDone(
        MockToken(AbilityType::EXTENSION), session, WIN_CMD_FOREGROUND, ABILITY_CMD_FOREGROUND), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * CaseDescription:
 * IsStartFreeInstall = 0, selfFreeInstallEnable = 0 (callerToken = nullptr)
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartAbilityInnerFreeInstall_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility", "");
    want.SetElement(element);
    MyFlag::flag_ = 1;
    auto result = abilityMs_->StartAbilityInner(want, nullptr, -1, -1, false);
    MyFlag::flag_ = 0;
    EXPECT_EQ(ERR_INVALID_VALUE, result);
    abilityMs_->OnStop();
    HILOG_INFO("AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * CaseDescription:
 * IsStartFreeInstall = 0, selfFreeInstallEnable = 0 (ModuleName = "")
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartAbilityInnerFreeInstall_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility", "");
    want.SetElement(element);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    const int32_t userId = -1;
    const int requestCode = 0;
    MyFlag::flag_ = 1;
    auto result = abilityMs_->StartAbilityInner(want, callerToken, requestCode, userId, false);
    MyFlag::flag_ = 0;
    abilityMs_->OnStop();
    EXPECT_EQ(ERR_INVALID_VALUE, result);
    HILOG_INFO("AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * CaseDescription:
 * IsStartFreeInstall = 0, selfFreeInstallEnable = 0 (BundleName != callerBundlerName)
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartAbilityInnerFreeInstall_003, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("", "com.test.demo1", "MainAbility", "Entry");
    want.SetElement(element);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    const int32_t userId = -1;
    const int requestCode = 0;
    MyFlag::flag_ = 1;
    auto result = abilityMs_->StartAbilityInner(want, callerToken, requestCode, userId, false);
    MyFlag::flag_ = 0;
    abilityMs_->OnStop();
    EXPECT_EQ(ERR_INVALID_VALUE, result);
    HILOG_INFO("AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * CaseDescription:
 * IsStartFreeInstall = 0, selfFreeInstallEnable = 1 (Ability not exist)
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartAbilityInnerFreeInstall_004, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility1", "Entry");
    want.SetElement(element);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    const int32_t userId = -1;
    const int requestCode = 0;
    MyFlag::flag_ = 1;
    auto result = abilityMs_->StartAbilityInner(want, callerToken, requestCode, userId, false);
    MyFlag::flag_ = 0;
    abilityMs_->OnStop();
    EXPECT_EQ(ERR_INVALID_VALUE, result);
    HILOG_INFO("AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * CaseDescription:
 * IsStartFreeInstall = 0, selfFreeInstallEnable = 1
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartAbilityInnerFreeInstall_005, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_005 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility", "Entry");
    want.SetElement(element);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityRecord->abilityInfo_.bundleName = "com.test.demo";
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    const int32_t userId = -1;
    const int requestCode = 0;
    MyFlag::flag_ = 1;
    auto result = abilityMs_->StartAbilityInner(want, callerToken, requestCode, userId, false);
    MyFlag::flag_ = 0;
    abilityMs_->OnStop();
    EXPECT_EQ(ERR_INVALID_VALUE, result);
    HILOG_INFO("AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_005 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * CaseDescription:
 * IsStartFreeInstall = 1, selfFreeInstallEnable = 0
 */
HWTEST_F(AbilityManagerServiceFirstTest, StartAbilityInnerFreeInstall_006, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_006 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    want.SetFlags(Want::FLAG_INSTALL_ON_DEMAND);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityRecord->abilityInfo_.bundleName = "com.test.demo";
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    const int32_t userId = -1;
    const int requestCode = 0;
    MyFlag::flag_ = 1;
    auto result = abilityMs_->StartAbilityInner(want, callerToken, requestCode, userId, false);
    MyFlag::flag_ = 0;
    abilityMs_->OnStop();
    EXPECT_EQ(ERR_INVALID_VALUE, result);
    HILOG_INFO("AbilityManagerServiceFirstTest StartAbilityInnerFreeInstall_006 end");
}

/*
 * Feature: AbilityManagerService
 * Function: AppRecoverKill
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AppRecoverKill
 * @tc.require: AR000I7F9D
 */
HWTEST_F(AbilityManagerServiceFirstTest, AppRecoverKill_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    pid_t pid = 8145;
    abilityMs_->AppRecoverKill(pid, StateReason::CPP_CRASH);
    abilityMs_->AppRecoverKill(pid, StateReason::JS_ERROR);
    abilityMs_->AppRecoverKill(pid, StateReason::LIFECYCLE);
    abilityMs_->AppRecoverKill(pid, StateReason::APP_FREEZE);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterIAbilityManagerCollaborator
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterIAbilityManagerCollaborator
 * @tc.require: issueI7LF4X
 */
HWTEST_F(AbilityManagerServiceFirstTest, RegisterIAbilityManagerCollaborator_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    sptr<IAbilityManagerCollaborator> broker = new (std::nothrow) MockAbilityManagerCollaborator();
    int32_t type = CollaboratorType::RESERVE_TYPE;
    int32_t res = abilityMs_->RegisterIAbilityManagerCollaborator(type, broker);
    EXPECT_EQ(res, CHECK_PERMISSION_FAILED);
    EXPECT_EQ(abilityMs_->collaboratorMap_.size(), 0);

    auto broker1 = abilityMs_->GetCollaborator(type);
    EXPECT_EQ(broker1, nullptr);

    res = abilityMs_->UnregisterIAbilityManagerCollaborator(type);
    EXPECT_EQ(res, CHECK_PERMISSION_FAILED);
    EXPECT_EQ(abilityMs_->collaboratorMap_.size(), 0);
}

/*
 * Feature: AbilityManagerService
 * Function: SetLockedState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetLockedState
 */
HWTEST_F(AbilityManagerServiceFirstTest, SetLockedState_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    abilityMs_->SetLockedState(1, true);
}

/*
 * Feature: AbilityManagerService
 * Function: VerifyAccountPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService VerifyAccountPermission
 */
HWTEST_F(AbilityManagerServiceFirstTest, VerifyAccountPermission_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest VerifyAccountPermission_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_EQ(abilityMs_->VerifyAccountPermission(-1), ERR_OK);
    EXPECT_EQ(abilityMs_->VerifyAccountPermission(0), ERR_OK);
    HILOG_INFO("AbilityManagerServiceFirstTest VerifyAccountPermission_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: CheckPrepareTerminateEnable
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckPrepareTerminateEnable
 * @tc.require: issueI7SX12
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckPrepareTerminateEnable_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->isPrepareTerminateEnable_ = false;
    bool res = abilityMs_->CheckPrepareTerminateEnable();
    EXPECT_FALSE(res);
}

/*
 * Feature: AbilityManagerService
 * Function: CheckPrepareTerminateEnable
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckPrepareTerminateEnable
 * @tc.require: issueI7SX12
 */
HWTEST_F(AbilityManagerServiceFirstTest, CheckPrepareTerminateEnable_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->isPrepareTerminateEnable_ = true;
    bool res = abilityMs_->CheckPrepareTerminateEnable();
    EXPECT_TRUE(res);
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectRemoteAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectRemoteAbility
 */
HWTEST_F(AbilityManagerServiceFirstTest, ConnectRemoteAbility_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceFirstTest ConnectRemoteAbility_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    EXPECT_EQ(abilityMs_->ConnectRemoteAbility(want, nullptr, nullptr), ERR_INVALID_VALUE);

    MyFlag::flag_ = 1;
    EXPECT_EQ(abilityMs_->ConnectRemoteAbility(want, nullptr, nullptr), ERR_NULL_OBJECT);
    MyFlag::flag_ = 0;
    HILOG_INFO("AbilityManagerServiceFirstTest ConnectRemoteAbility_001 end");
}

/**
 * @tc.name: RegisterAppDebugListener_001
 * @tc.desc: Test the state of RegisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, RegisterAppDebugListener_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<AppExecFwk::MockAppDebugListenerStub> listener;
    auto result = abilityMs_->RegisterAppDebugListener(listener);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: UnregisterAppDebugListener_001
 * @tc.desc: Test the state of UnregisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, UnregisterAppDebugListener_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<AppExecFwk::MockAppDebugListenerStub> listener;
    auto result = abilityMs_->UnregisterAppDebugListener(listener);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: AttachAppDebug_001
 * @tc.desc: Test the state of AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, AttachAppDebug_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::string bundleName;
    auto result = abilityMs_->AttachAppDebug(bundleName);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: DetachAppDebug_001
 * @tc.desc: Test the state of DetachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, DetachAppDebug_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::string bundleName;
    auto result = abilityMs_->DetachAppDebug(bundleName);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: GetForegroundUIAbilities_001
 * @tc.desc: Test function GetForegroundUIAbilities when dosen't have permission.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, GetForegroundUIAbilities_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::vector<AppExecFwk::AbilityStateData> list;
    auto res = abilityMs_->GetForegroundUIAbilities(list);
    EXPECT_EQ(res, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: AbilityManagerService
 * Function: GenerateEmbeddableUIAbilityRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GenerateEmbeddableUIAbilityRequest
 */
HWTEST_F(AbilityManagerServiceFirstTest, GenerateEmbeddableUIAbilityRequest_001, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceSecondTest GenerateEmbeddableUIAbilityRequest_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    want.SetParam("ScreenMode", 1);
    AbilityRequest request;
    auto res = abilityMs_->GenerateEmbeddableUIAbilityRequest(want, request, nullptr, USER_ID_U100);
    EXPECT_EQ(res, RESOLVE_ABILITY_ERR);
    HILOG_INFO("AbilityManagerServiceSecondTest GenerateEmbeddableUIAbilityRequest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GenerateEmbeddableUIAbilityRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GenerateEmbeddableUIAbilityRequest
 */
HWTEST_F(AbilityManagerServiceFirstTest, GenerateEmbeddableUIAbilityRequest_002, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerServiceSecondTest GenerateEmbeddableUIAbilityRequest_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    AbilityRequest request;
    auto res = abilityMs_->GenerateEmbeddableUIAbilityRequest(want, request, nullptr, USER_ID_U100);
    EXPECT_EQ(res, RESOLVE_ABILITY_ERR);
    HILOG_INFO("AbilityManagerServiceSecondTest GenerateEmbeddableUIAbilityRequest_002 end");
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_RegisterAutoStartupSystemCallback_0100
 * @tc.desc: Test the state of RegisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, RegisterAutoStartupSystemCallback_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = nullptr;
    sptr<IRemoteObject> callback = nullptr;
    auto result = abilityMs_->RegisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_RegisterAutoStartupSystemCallback_0200
 * @tc.desc: Test the state of RegisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, RegisterAutoStartupSystemCallback_0200, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = std::make_shared<AbilityRuntime::AbilityAutoStartupService>();
    EXPECT_NE(abilityMs_->abilityAutoStartupService_, nullptr);
    sptr<IRemoteObject> callback = nullptr;
    auto result = abilityMs_->RegisterAutoStartupSystemCallback(callback);
    EXPECT_NE(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_UnregisterAutoStartupSystemCallback_0100
 * @tc.desc: Test the state of UnregisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, UnregisterAutoStartupSystemCallback_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = nullptr;
    sptr<IRemoteObject> callback = nullptr;
    auto result = abilityMs_->UnregisterAutoStartupSystemCallback(callback);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_UnregisterAutoStartupSystemCallback_0200
 * @tc.desc: Test the state of UnregisterAutoStartupSystemCallback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, UnregisterAutoStartupSystemCallback_0200, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = std::make_shared<AbilityRuntime::AbilityAutoStartupService>();
    EXPECT_NE(abilityMs_->abilityAutoStartupService_, nullptr);
    sptr<IRemoteObject> callback = nullptr;
    auto result = abilityMs_->UnregisterAutoStartupSystemCallback(callback);
    EXPECT_NE(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_SetApplicationAutoStartup_0100
 * @tc.desc: Test the state of SetApplicationAutoStartup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, SetApplicationAutoStartup_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = nullptr;
    AutoStartupInfo info;
    auto result = abilityMs_->SetApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_SetApplicationAutoStartup_0200
 * @tc.desc: Test the state of SetApplicationAutoStartup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, SetApplicationAutoStartup_0200, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = std::make_shared<AbilityRuntime::AbilityAutoStartupService>();
    EXPECT_NE(abilityMs_->abilityAutoStartupService_, nullptr);
    AutoStartupInfo info;
    auto result = abilityMs_->SetApplicationAutoStartup(info);
    EXPECT_NE(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_CancelApplicationAutoStartup_0100
 * @tc.desc: Test the state of CancelApplicationAutoStartup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, CancelApplicationAutoStartup_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = nullptr;
    AutoStartupInfo info;
    auto result = abilityMs_->CancelApplicationAutoStartup(info);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_CancelApplicationAutoStartup_0200
 * @tc.desc: Test the state of CancelApplicationAutoStartup
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, CancelApplicationAutoStartup_0200, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = std::make_shared<AbilityRuntime::AbilityAutoStartupService>();
    EXPECT_NE(abilityMs_->abilityAutoStartupService_, nullptr);
    AutoStartupInfo info;
    auto result = abilityMs_->CancelApplicationAutoStartup(info);
    EXPECT_NE(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_QueryAllAutoStartupApplications_0100
 * @tc.desc: Test the state of QueryAllAutoStartupApplications
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, QueryAllAutoStartupApplications_0100, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = nullptr;
    std::vector<AutoStartupInfo> infoList;
    auto result = abilityMs_->QueryAllAutoStartupApplications(infoList);
    EXPECT_EQ(result, ERR_NO_INIT);
}

/**
 * @tc.name: AbilityManagerServiceFirstTest_QueryAllAutoStartupApplications_0200
 * @tc.desc: Test the state of QueryAllAutoStartupApplications
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceFirstTest, QueryAllAutoStartupApplications_0200, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->abilityAutoStartupService_ = std::make_shared<AbilityRuntime::AbilityAutoStartupService>();
    EXPECT_NE(abilityMs_->abilityAutoStartupService_, nullptr);
    std::vector<AutoStartupInfo> infoList;
    auto result = abilityMs_->QueryAllAutoStartupApplications(infoList);
    EXPECT_NE(result, ERR_NO_INIT);
}
} // namespace AAFwk
} // namespace OHOS
