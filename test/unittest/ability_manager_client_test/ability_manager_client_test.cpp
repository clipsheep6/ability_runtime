/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "ability_manager_client.h"
#include "ability_manager_stub_mock_test.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
const int NULL_TARGET = 2097154;
const int INIT_VALUE = 0;
const size_t SIZE_ZERO = 0;
const std::string BUNDLE_NAME = "BUNDLE_NAME";
const std::string EMPTY_STRING = "";
}  // namespace

class AbilityManagerClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityManagerClient> client_{ nullptr };
    sptr<AbilityManagerStubTestMock> mock_{ nullptr };
};

void AbilityManagerClientTest::SetUpTestCase(void)
{}
void AbilityManagerClientTest::TearDownTestCase(void)
{}
void AbilityManagerClientTest::TearDown()
{}

void AbilityManagerClientTest::SetUp()
{
    client_ = std::make_shared<AbilityManagerClient>();
    mock_ = new AbilityManagerStubTestMock();
    client_->proxy_ = mock_;
}

/*
 * Feature: AbilityManagerClient
 * Function: SendResultToAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerClient SendResultToAbility
 * EnvConditions: NA
 * CaseDescription: Verify the SendResultToAbility call normal
 */
HWTEST_F(AbilityManagerClientTest, SendResultToAbility_001, TestSize.Level1)
{
    Want want;
    EXPECT_EQ(client_->SendResultToAbility(-1, -1, want), 0);
}

/*
 * Feature: AbilityManagerClient
 * Function: StartAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerClient StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify the StartAbilityByCall call normal
 */
HWTEST_F(AbilityManagerClientTest, StartAbilityByCall_001, TestSize.Level1)
{
    Want want;
    EXPECT_EQ(client_->StartAbilityByCall(want, nullptr, nullptr), 0);
}

/*
 * Feature: AbilityManagerClient
 * Function: StartAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerClient StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify the ReleaseCall call normal
 */
HWTEST_F(AbilityManagerClientTest, ReleaseCall_001, TestSize.Level1)
{
    ElementName element;
    sptr<IAbilityConnection> connect = nullptr;
    EXPECT_EQ(client_->ReleaseCall(connect, element), 0);
}

/**
 * @tc.name: AbilityManagerClient_DumpSysState_0100
 * @tc.desc: DumpSysState
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_DumpSysState_0100, TestSize.Level1)
{
    std::string args = "-a";
    std::vector<std::string> state;
    bool isClient = false;
    bool isUserID = true;

    auto result = client_->DumpSysState(args, state, isClient, isUserID, USER_ID);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(state.size(), SIZE_ZERO);
}

/**
 * @tc.name: AbilityManagerClient_SetMissionIcon_0100
 * @tc.desc: SetMissionIcon
 * @tc.type: FUNC
 * @tc.require: SR000GVIJQ
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_SetMissionIcon_0100, TestSize.Level1)
{
    sptr<IRemoteObject> abilityToken = nullptr;
    std::shared_ptr<OHOS::Media::PixelMap> icon = std::make_shared<OHOS::Media::PixelMap>();

    auto result = client_->SetMissionIcon(abilityToken, icon);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_SetMissionIcon_0200
 * @tc.desc: SetMissionIcon
 * @tc.type: FUNC
 * @tc.require: SR000GVIJQ
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_SetMissionIcon_0200, TestSize.Level1)
{
    sptr<IRemoteObject> abilityToken = new AbilityManagerStubTestMock();
    std::shared_ptr<OHOS::Media::PixelMap> icon = nullptr;

    auto result = client_->SetMissionIcon(abilityToken, icon);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_ScheduleConnectAbilityDone_0100
 * @tc.desc: ScheduleConnectAbilityDone
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientTest, ScheduleConnectAbilityDone_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> remoteObject = nullptr;
    auto result = client_->ScheduleConnectAbilityDone(token, remoteObject);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_ScheduleDisconnectAbilityDone_0100
 * @tc.desc: ScheduleDisconnectAbilityDone
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientTest, ScheduleDisconnectAbilityDone_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto result = client_->ScheduleDisconnectAbilityDone(token);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_StartExtensionAbility_0100
 * @tc.desc: StartExtensionAbility
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientTest, StartExtensionAbility_0100, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t userId = DEFAULT_INVAL_VALUE;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    auto result = client_->StartExtensionAbility(want, callerToken, userId, extensionType);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_StopExtensionAbility_0100
 * @tc.desc: StopExtensionAbility
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientTest, StopExtensionAbility_0100, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t userId = DEFAULT_INVAL_VALUE;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    auto result = client_->StopExtensionAbility(want, callerToken, userId, extensionType);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_TerminateAbility_0100
 * @tc.desc: TerminateAbility
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientTest, TerminateAbility_0100, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = client_->TerminateAbility(callerToken, -1);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_TerminateAbilityResult_0100
 * @tc.desc: TerminateAbilityResult
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientTest, TerminateAbilityResult_0100, TestSize.Level1)
{
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = client_->TerminateAbilityResult(callerToken, 1);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_MinimizeAbility_0100
 * @tc.desc: MinimizeAbility
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientTest, MinimizeAbility_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    bool fromUser = false;
    auto result = client_->MinimizeAbility(token, fromUser);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_DumpState_0100
 * @tc.desc: DumpState
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientTest, DumpState_0100, TestSize.Level1)
{
    std::string myString = "-a";
    std::vector<std::string> state;
    auto result = client_->DumpState(myString, state);
    EXPECT_EQ(ERR_OK, result);
}

#ifdef ABILITY_COMMAND_FOR_TEST
/**
 * @tc.name: AbilityManagerClient_ForceTimeoutForTest_0100
 * @tc.desc: ForceTimeoutForTest
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientTest, ForceTimeoutForTest_0100, TestSize.Level1)
{
    std::string abilityName = "abilityName_test";
    std::string state = "state_test";
    auto result = client_->ForceTimeoutForTest(abilityName, state);
    EXPECT_EQ(ERR_OK, result);
}
#endif

/**
 * @tc.name: AbilityManagerClient_ClearUpApplicationData_0100
 * @tc.desc: ClearUpApplicationData
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientTest, ClearUpApplicationData_0100, TestSize.Level1)
{
    std::string bundleName = "bundleName_test";
    auto result = client_->ClearUpApplicationData(bundleName);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_SendWantSender_0100
 * @tc.desc: SendWantSender
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientTest, SendWantSender_0100, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    SenderInfo senderInfo;
    auto result = client_->SendWantSender(target, senderInfo);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_GetAppMemorySize_0100
 * @tc.desc: GetAppMemorySize
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientTest, GetAppMemorySize_0100, TestSize.Level1)
{
    auto result = client_->GetAppMemorySize();
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_StartContinuation_0100
 * @tc.desc: StartContinuation
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientTest, StartContinuation_0100, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> abilityToken = nullptr;
    auto result = client_->StartContinuation(want, abilityToken, 1);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_NotifyContinuationResult_0100
 * @tc.desc: NotifyContinuationResult
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientTest, NotifyContinuationResult_0100, TestSize.Level1)
{
    auto result = client_->NotifyContinuationResult(1, 1);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_LockMissionForCleanup_0100
 * @tc.desc: LockMissionForCleanup
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientTest, LockMissionForCleanup_0100, TestSize.Level1)
{
    auto result = client_->LockMissionForCleanup(1);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_UnlockMissionForCleanup_0100
 * @tc.desc: UnlockMissionForCleanup
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientTest, UnlockMissionForCleanup_0100, TestSize.Level1)
{
    auto result = client_->UnlockMissionForCleanup(5);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_RegisterMissionListener_0100
 * @tc.desc: RegisterMissionListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientTest, RegisterMissionListener_0100, TestSize.Level1)
{
    sptr<IMissionListener> listener = nullptr;
    auto result = client_->RegisterMissionListener(listener);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_RegisterMissionListener_0200
 * @tc.desc: RegisterMissionListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientTest, RegisterMissionListener_0200, TestSize.Level1)
{
    std::string deviceId = "123";
    sptr<IRemoteMissionListener> listener = nullptr;
    auto result = client_->RegisterMissionListener(deviceId, listener);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_UnRegisterMissionListener_0100
 * @tc.desc: UnRegisterMissionListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientTest, UnRegisterMissionListener_0100, TestSize.Level1)
{
    sptr<IMissionListener> listener = nullptr;
    auto result = client_->UnRegisterMissionListener(listener);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_UnRegisterMissionListener_0200
 * @tc.desc: UnRegisterMissionListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientTest, UnRegisterMissionListener_0200, TestSize.Level1)
{
    std::string deviceId = "123";
    sptr<IRemoteMissionListener> listener = nullptr;
    auto result = client_->UnRegisterMissionListener(deviceId, listener);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_GetMissionInfos_0100
 * @tc.desc: GetMissionInfos
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientTest, GetMissionInfos_0100, TestSize.Level1)
{
    std::string deviceId = "123";
    std::vector<MissionInfo> missionInfos;
    auto result = client_->GetMissionInfos(deviceId, 10, missionInfos);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_GetMissionSnapshot_0100
 * @tc.desc: GetMissionSnapshot
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientTest, GetMissionSnapshot_0100, TestSize.Level1)
{
    std::string deviceId = "123";
    MissionSnapshot snapshot;
    bool isLowResolution = false;
    auto result = client_->GetMissionSnapshot(deviceId, 10, snapshot, isLowResolution);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_CleanMission_0100
 * @tc.desc: CleanMission
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientTest, CleanMission_0100, TestSize.Level1)
{
    auto result = client_->CleanMission(10);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_CleanAllMissions_0100
 * @tc.desc: CleanAllMissions
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientTest, CleanAllMissions_0100, TestSize.Level1)
{
    auto result = client_->CleanAllMissions();
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionToFront_0100
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientTest, MoveMissionToFront_0100, TestSize.Level1)
{
    auto result = client_->MoveMissionToFront(10);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionToFront_0200
 * @tc.desc: MoveMissionToFront
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientTest, MoveMissionToFront_0200, TestSize.Level1)
{
    StartOptions startOptions;
    auto result = client_->MoveMissionToFront(1, startOptions);
    EXPECT_EQ(result, ERR_OK);
}


/**
 * @tc.name: AbilityManagerClient_GetAbilityRunningInfos_0100
 * @tc.desc: GetAbilityRunningInfos
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientTest, GetAbilityRunningInfos_0100, TestSize.Level1)
{
    std::vector<AbilityRunningInfo> myInfo;
    auto result = client_->GetAbilityRunningInfos(myInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_ScheduleCommandAbilityDone_0100
 * @tc.desc: ScheduleCommandAbilityDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, ScheduleCommandAbilityDone_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto result = client_->ScheduleCommandAbilityDone(token);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_CloseAbility_0100
 * @tc.desc: CloseAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, CloseAbility_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    Want resultWant;
    auto result = client_->CloseAbility(token, DEFAULT_INVAL_VALUE, &resultWant);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_ConnectDataShareExtensionAbility_0100
 * @tc.desc: ConnectDataShareExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, ConnectDataShareExtensionAbility_0100, TestSize.Level1)
{
    sptr<IAbilityConnection> connect = nullptr;
    Want want;
    auto result = client_->ConnectDataShareExtensionAbility(want, connect, DEFAULT_INVAL_VALUE);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_ConnectExtensionAbility_0100
 * @tc.desc: ConnectExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, ConnectExtensionAbility_0100, TestSize.Level1)
{
    sptr<IAbilityConnection> connect = nullptr;
    Want want;
    auto result = client_->ConnectExtensionAbility(want, connect, DEFAULT_INVAL_VALUE);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_ReleaseDataAbility_0100
 * @tc.desc: ReleaseDataAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, ReleaseDataAbility_0100, TestSize.Level1)
{
    sptr<IAbilityScheduler> dataAbilityScheduler = nullptr;
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = client_->ReleaseDataAbility(dataAbilityScheduler, callerToken);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_KillProcess_0100
 * @tc.desc: KillProcess
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, KillProcess_0100, TestSize.Level1)
{
    auto result = client_->KillProcess(BUNDLE_NAME);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_GetPendingWantUid_0100
 * @tc.desc: GetPendingWantUid
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, GetPendingWantUid_0100, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    int32_t uid = INIT_VALUE;
    auto result = client_->GetPendingWantUid(target, uid);
    EXPECT_EQ(NULL_TARGET, result);
}

/**
 * @tc.name: AbilityManagerClient_GetPendingWantUserId_0100
 * @tc.desc: GetPendingWantUserId
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, GetPendingWantUserId_0100, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    int32_t userId = INIT_VALUE;
    auto result = client_->GetPendingWantUserId(target, userId);
    EXPECT_EQ(NULL_TARGET, result);
}

/**
 * @tc.name: AbilityManagerClient_GetPendingWantBundleName_0100
 * @tc.desc: GetPendingWantBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, GetPendingWantBundleName_0100, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    std::string bundleName = EMPTY_STRING;
    auto result = client_->GetPendingWantBundleName(target, bundleName);
    EXPECT_EQ(NULL_TARGET, result);
}

/**
 * @tc.name: AbilityManagerClient_GetPendingWantCode_0100
 * @tc.desc: GetPendingWantCode
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, GetPendingWantCode_0100, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    int32_t code = INIT_VALUE;
    auto result = client_->GetPendingWantCode(target, code);
    EXPECT_EQ(NULL_TARGET, result);
}

/**
 * @tc.name: AbilityManagerClient_GetPendingWantType_0100
 * @tc.desc: GetPendingWantType
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, GetPendingWantType_0100, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    int32_t type = INIT_VALUE;
    auto result = client_->GetPendingWantType(target, type);
    EXPECT_EQ(NULL_TARGET, result);
}

/**
 * @tc.name: AbilityManagerClient_GetPendingRequestWant_0100
 * @tc.desc: GetPendingRequestWant
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, GetPendingRequestWant_0100, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    std::shared_ptr<Want> want = nullptr;
    auto result = client_->GetPendingRequestWant(target, want);
    EXPECT_EQ(NULL_TARGET, result);
}

/**
 * @tc.name: AbilityManagerClient_GetWantSenderInfo_0100
 * @tc.desc: GetWantSenderInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, GetWantSenderInfo_0100, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    std::shared_ptr<WantSenderInfo> info = nullptr;
    auto result = client_->GetWantSenderInfo(target, info);
    EXPECT_EQ(NULL_TARGET, result);
}

/**
 * @tc.name: AbilityManagerClient_CancelWantSender_0100
 * @tc.desc: CancelWantSender
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, CancelWantSender_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CancelWantSender_0100 start";
    sptr<IWantSender> sender = nullptr;
    client_->CancelWantSender(sender);

    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "CancelWantSender_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_RegisterCancelListener_0100
 * @tc.desc: RegisterCancelListener
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, RegisterCancelListener_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterCancelListener_0100 start";
    sptr<IWantSender> sender = nullptr;
    sptr<IWantReceiver> receiver = nullptr;
    client_->RegisterCancelListener(sender, receiver);

    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "RegisterCancelListener_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_UnregisterCancelListener_0100
 * @tc.desc: UnregisterCancelListener
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, UnregisterCancelListener_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterCancelListener_0100 start";
    sptr<IWantSender> sender = nullptr;
    sptr<IWantReceiver> receiver = nullptr;
    client_->UnregisterCancelListener(sender, receiver);

    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "UnregisterCancelListener_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_IsRamConstrainedDevice_0100
 * @tc.desc: IsRamConstrainedDevice
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, IsRamConstrainedDevice_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsRamConstrainedDevice_0100 start";
    auto result = client_->IsRamConstrainedDevice();

    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "IsRamConstrainedDevice_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_ContinueMission_0100
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, ContinueMission_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ContinueMission_0100 start";
    std::string srcDeviceId = "123";
    std::string dstDeviceId = "ABC";
    int32_t missionId = 5;
    sptr<IRemoteObject> callBack = nullptr;
    WantParams wantParams;
    auto result = client_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callBack, wantParams);

    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ContinueMission_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_NotifyCompleteContinuation_0100
 * @tc.desc: NotifyCompleteContinuation
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, NotifyCompleteContinuation_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyCompleteContinuation_0100 start";
    std::string deviceId = BUNDLE_NAME;
    int32_t sessionId = 1;
    bool isSuccess = true;
    client_->NotifyCompleteContinuation(deviceId, sessionId, isSuccess);

    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "NotifyCompleteContinuation_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_GetMissionIdByToken_0100
 * @tc.desc: GetMissionIdByToken
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, GetMissionIdByToken_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMissionIdByToken_0100 start";
    sptr<IRemoteObject> token = nullptr;
    int32_t missionId = 1;
    auto result = client_->GetMissionIdByToken(token, missionId);

    EXPECT_EQ(result, MISSION_NOT_FOUND);
    GTEST_LOG_(INFO) << "GetMissionIdByToken_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_GetExtensionRunningInfos_0100
 * @tc.desc: GetExtensionRunningInfos
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, GetExtensionRunningInfos_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetExtensionRunningInfos_0100 start";
    EXPECT_CALL(*mock_, GetExtensionRunningInfos(_, _))
        .Times(1)
        .WillOnce(Return(0));
    int upperLimit = 1;
    std::vector<ExtensionRunningInfo> info;
    auto result = client_->GetExtensionRunningInfos(upperLimit, info);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "GetExtensionRunningInfos_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_GetProcessRunningInfos_0100
 * @tc.desc: GetProcessRunningInfos
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, GetProcessRunningInfos_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetProcessRunningInfos_0100 start";
    EXPECT_CALL(*mock_, GetProcessRunningInfos(_))
        .Times(1)
        .WillOnce(Return(0));
    std::vector<RunningProcessInfo> info;
    auto result = client_->GetProcessRunningInfos(info);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "GetProcessRunningInfos_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StartSyncRemoteMissions_0100
 * @tc.desc: StartSyncRemoteMissions
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, StartSyncRemoteMissions_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncRemoteMissions_0100 start";
    std::string devId = BUNDLE_NAME;
    bool fixConflict = true;
    int64_t tag = 1;
    auto result = client_->StartSyncRemoteMissions(devId, fixConflict, tag);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "StartSyncRemoteMissions_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StopSyncRemoteMissions_0100
 * @tc.desc: StopSyncRemoteMissions
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, StopSyncRemoteMissions_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopSyncRemoteMissions_0100 start";
    std::string devId = BUNDLE_NAME;
    auto result = client_->StopSyncRemoteMissions(devId);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "StopSyncRemoteMissions_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StartUser_0100
 * @tc.desc: StartUser
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, StartUser_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartUser_0100 start";
    int userId = 1;
    auto result = client_->StartUser(userId);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "StartUser_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StopUser_0100
 * @tc.desc: StopUser
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, StopUser_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopUser_0100 start";
    int userId = 1;
    sptr<IStopUserCallback> callback = nullptr;
    auto result = client_->StopUser(userId, callback);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "StopUser_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_RegisterSnapshotHandler_0100
 * @tc.desc: RegisterSnapshotHandler
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, RegisterSnapshotHandler_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSnapshotHandler_0100 start";
    sptr<ISnapshotHandler> handler = nullptr;
    auto result = client_->RegisterSnapshotHandler(handler);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "RegisterSnapshotHandler_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_RegisterWindowManagerServiceHandler_0100
 * @tc.desc: RegisterWindowManagerServiceHandler
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, RegisterWindowManagerServiceHandler_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterWindowManagerServiceHandler_0100 start";
    sptr<IWindowManagerServiceHandler> handler = nullptr;
    auto result = client_->RegisterWindowManagerServiceHandler(handler);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "RegisterWindowManagerServiceHandler_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_CompleteFirstFrameDrawing_0100
 * @tc.desc: CompleteFirstFrameDrawing
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, CompleteFirstFrameDrawing_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompleteFirstFrameDrawing_0100 start";
    sptr<IRemoteObject> abilityToken = nullptr;
    client_->CompleteFirstFrameDrawing(abilityToken);

    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "CompleteFirstFrameDrawing_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StartUserTest_0100
 * @tc.desc: StartUserTest
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, StartUserTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartUserTest_0100 start";
    Want want;
    sptr<IRemoteObject> observer = nullptr;
    auto result = client_->StartUserTest(want, observer);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "StartUserTest_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_DoAbilityForeground_0100
 * @tc.desc: DoAbilityForeground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, DoAbilityForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoAbilityForeground_0100 start";
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 1;
    auto result = client_->DoAbilityForeground(token, flag);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DoAbilityForeground_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_DoAbilityBackground_0100
 * @tc.desc: DoAbilityBackground
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, DoAbilityBackground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoAbilityBackground_0100 start";
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 1;
    auto result = client_->DoAbilityBackground(token, flag);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "DoAbilityBackground_0100 end";
}

/**
 *
 * @tc.name: AbilityManagerClient_SetAbilityController_0100
 * @tc.desc: SetAbilityController
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, SetAbilityController_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAbilityController_0100 start";
    sptr<IAbilityController> abilityController = nullptr;
    bool imAStabilityTest = 0;
    auto result = client_->SetAbilityController(abilityController, imAStabilityTest);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "SetAbilityController_0100 end";
}


#ifdef ABILITY_COMMAND_FOR_TEST
/**
 * @tc.name: AbilityManagerClient_BlockAmsService_0100
 * @tc.desc: BlockAmsService
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, BlockAmsService_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BlockAmsService_0100 start";
    EXPECT_CALL(*mock_, BlockAmsService())
        .Times(1)
        .WillOnce(Return(0));
    auto result = client_->BlockAmsService();

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "BlockAmsService_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_BlockAppService_0100
 * @tc.desc: BlockAppService
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, BlockAppService_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BlockAppService_0100 start";
    EXPECT_CALL(*mock_, BlockAppService())
        .Times(1)
        .WillOnce(Return(0));
    auto result = client_->BlockAppService(info);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "BlockAppService_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_BlockAbility_0100
 * @tc.desc: BlockAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, BlockAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BlockAbility_0100 start";
    EXPECT_CALL(*mock_, BlockAbility(_))
        .Times(1)
        .WillOnce(Return(0));
    int32_t abilityRecordId = 1;
    auto result = client_->BlockAbility(abilityRecordId);

    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "BlockAbility_0100 end";
}
#endif

/**
 * @tc.name: AbilityManagerClient_DumpAbilityInfoDone_0100
 * @tc.desc: DumpAbilityInfoDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, DumpAbilityInfoDone_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DumpAbilityInfoDone_0100 start";
    std::vector<std::string> infos;
    infos.emplace_back("DumpAbilityInfoDone");
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = client_->DumpAbilityInfoDone(infos, callerToken);

    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "DumpAbilityInfoDone_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_FreeInstallAbilityFromRemote_0100
 * @tc.desc: FreeInstallAbilityFromRemote
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, FreeInstallAbilityFromRemote_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FreeInstallAbilityFromRemote_0100 start";
    Want want;
    sptr<IRemoteObject> callback = nullptr;
    int32_t userId = 0;
    auto result = client_->FreeInstallAbilityFromRemote(want, callback, userId);

    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "FreeInstallAbilityFromRemote_0100 end";
}


/**
 * @tc.name: AbilityManagerClient_SendANRProcessID_0100
 * @tc.desc: SendANRProcessID
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, SendANRProcessID_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SendANRProcessID_0100 start";
    int pid = 0;
    auto result = client_->SendANRProcessID(pid);

    EXPECT_EQ(ERR_OK, result);
    GTEST_LOG_(INFO) << "SendANRProcessID_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_UpdateMissionSnapShot_0100
 * @tc.desc: UpdateMissionSnapShot
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, UpdateMissionSnapShot_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateMissionSnapShot_0100 start";
    sptr<IRemoteObject> token = nullptr;
    client_->UpdateMissionSnapShot(token);

    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "UpdateMissionSnapShot_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_OnRemoteDied_0100
 * @tc.desc: OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, OnRemoteDied_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteDied_0100 start";
    AbilityManagerClient::AbilityMgrDeathRecipient recipient;
    wptr<IRemoteObject> remote = nullptr;
    recipient.OnRemoteDied(remote);

    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "OnRemoteDied_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_CallRequestDone_0100
 * @tc.desc: CallRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, CallRequestDone_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CallRequestDone_0100 start";
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> callStub = nullptr;
    client_->CallRequestDone(token, callStub);

    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "CallRequestDone_0100 end";
}


/**
 * @tc.name: AbilityManagerClient_IsValidMissionIds_0100
 * @tc.desc: CallRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, IsValidMissionIds_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidMissionIds_0100 start";
    std::vector<int32_t> missionIds;
    std::vector<MissionVaildResult> results;
    EXPECT_CALL(*mock_, IsValidMissionIds(_, _)).Times(1).WillOnce(Return(ERR_OK));
    EXPECT_EQ(client_->IsValidMissionIds(missionIds, results), ERR_OK);
    GTEST_LOG_(INFO) << "IsValidMissionIds_0100 end";
}
}  // namespace AAFwk
}  // namespace OHOS