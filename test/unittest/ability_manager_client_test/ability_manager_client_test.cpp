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
#include "ability_manager_client.h"
#undef private
#undef protected
#include "ability_manager_stub_mock_test.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
const size_t SIZE_ZERO = 0;
}  // namespace

class AbilityManagerClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityManagerClient> client_ {nullptr};
    sptr<AbilityManagerStubTestMock> mock_ {nullptr};
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
 * @tc.name: GetMissionInfo_0100
 * @tc.desc: GetMissionInfo
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, GetMissionInfo_0100, TestSize.Level1)
{
    std::string deviceId = "123";
    int32_t missionId = 1;
    MissionInfo missionInfo;
    auto result = client_->GetMissionInfo(deviceId, missionId, missionInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetExtensionRunningInfos_0100s
 * @tc.desc: GetExtensionRunningInfos
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, GetExtensionRunningInfos_0100, TestSize.Level1)
{
    int upperLimit = 1;
    std::vector<ExtensionRunningInfo> info;
    auto result = client_->GetExtensionRunningInfos(upperLimit, info);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetProcessRunningInfos_0100
 * @tc.desc: GetProcessRunningInfos
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, GetProcessRunningInfos_0100, TestSize.Level1)
{
    std::vector<AppExecFwk::RunningProcessInfo> myInfo;
    auto result = client_->GetProcessRunningInfos(myInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: StopSyncRemoteMissions_0100
 * @tc.desc: StopSyncRemoteMissions
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, StopSyncRemoteMissions_0100, TestSize.Level1)
{
    std::string devId = "123";
    auto result = client_->StopSyncRemoteMissions(devId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: StartUser_0100
 * @tc.desc: StartUser
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, StartUser_0100, TestSize.Level1)
{
    int accountId = 1;
    auto result = client_->StartUser(accountId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: StopUser_0100
 * @tc.desc: StopUser
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, StopUser_0100, TestSize.Level1)
{
    int accountId = 1;
    sptr<IStopUserCallback> callback = nullptr;
    auto result = client_->StopUser(accountId, callback);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: RegisterSnapshotHandler_0100
 * @tc.desc: RegisterSnapshotHandler
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, RegisterSnapshotHandler_0100, TestSize.Level1)
{
    sptr<ISnapshotHandler> handler = nullptr;
    auto result = client_->RegisterSnapshotHandler(handler);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: SetMissionLabel_0100
 * @tc.desc: SetMissionLabel
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, SetMissionLabel_0100, TestSize.Level1)
{
    sptr<IRemoteObject> abilityToken = nullptr;
    std::string label = "myLable";
    auto result = client_->SetMissionLabel(abilityToken, label);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: RegisterWindowManagerServiceHandler_0100
 * @tc.desc: RegisterWindowManagerServiceHandler
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, RegisterWindowManagerServiceHandler_0100, TestSize.Level1)
{
    sptr<IWindowManagerServiceHandler> handler = nullptr;
    auto result = client_->RegisterWindowManagerServiceHandler(handler);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: StartUserTest_0100
 * @tc.desc: StartUserTest
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, StartUserTest_0100, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> observer = nullptr;
    auto result = client_->StartUserTest(want, observer);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: FinishUserTest_0100
 * @tc.desc: FinishUserTest
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, FinishUserTest_0100, TestSize.Level1)
{
    std::string msg = "myMsg";
    int64_t resultCode = 1;
    std::string bundleName = "myBundleName";
    auto result = client_->FinishUserTest(msg, resultCode, bundleName);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetTopAbility_0100
 * @tc.desc: GetTopAbility
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, GetTopAbility_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto result = client_->GetTopAbility(token);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: DelegatorDoAbilityForeground_0100
 * @tc.desc: DelegatorDoAbilityForeground
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, DelegatorDoAbilityForeground_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto result = client_->DelegatorDoAbilityForeground(token);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: DoAbilityForeground_0100
 * @tc.desc: DoAbilityForeground
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, DoAbilityForeground_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 1;
    auto result = client_->DoAbilityForeground(token, flag);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: DelegatorDoAbilityBackground_0100
 * @tc.desc: DelegatorDoAbilityBackground
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, DelegatorDoAbilityBackground_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto result = client_->DelegatorDoAbilityBackground(token);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: DoAbilityBackground_0100
 * @tc.desc: DoAbilityBackground
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, DoAbilityBackground_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 1;
    auto result = client_->DoAbilityBackground(token, flag);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: SendANRProcessID_0100
 * @tc.desc: SendANRProcessID
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, SendANRProcessID_0100, TestSize.Level1)
{
    int myPid = 1;
    auto result = client_->SendANRProcessID(myPid);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: BlockAmsService_0100
 * @tc.desc: BlockAmsService
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, BlockAmsService_0100, TestSize.Level1)
{
    auto result = client_->BlockAmsService();
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: BlockAbility_0100
 * @tc.desc: BlockAbility
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, BlockAbility_0100, TestSize.Level1)
{
    int32_t abilityRecordId = 1;
    auto result = client_->BlockAbility(abilityRecordId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: BlockAppService_0100
 * @tc.desc: BlockAppService
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, BlockAppService_0100, TestSize.Level1)
{
    auto result = client_->BlockAppService();
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: FreeInstallAbilityFromRemote_0100
 * @tc.desc: FreeInstallAbilityFromRemote
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, FreeInstallAbilityFromRemote_0100, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callBackTest = nullptr;
    int32_t userId = 1;
    int requestCode = DEFAULT_INVAL_VALUE;
    auto result = client_->FreeInstallAbilityFromRemote(want, callBackTest, userId, requestCode);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: DumpAbilityInfoDone_0100
 * @tc.desc: DumpAbilityInfoDone
 * @tc.type: FUNC
 * @tc.require: issueI5Q5AF
 */
HWTEST_F(AbilityManagerClientTest, DumpAbilityInfoDone_0100, TestSize.Level1)
{
    std::vector<std::string> myInfos;
    sptr<IRemoteObject> callerTokenTest = nullptr;
    auto result = client_->DumpAbilityInfoDone(myInfos, callerTokenTest);
    EXPECT_EQ(result, ERR_OK);
}
}  // namespace AAFwk
}  // namespace OHOS
