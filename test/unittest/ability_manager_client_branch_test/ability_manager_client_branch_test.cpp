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
#include "ability_connect_manager.h"
#undef private
#undef protected

#include "hilog_wrapper.h"
#include "mock_ability_connect_callback.h"
#include "session/host/include/session.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
const size_t SIZE_ZERO = 0;
const std::string BUNDLE_NAME = "BUNDLE_NAME";
const std::string EMPTY_STRING = "";
}  // namespace

class AbilityManagerClientBranchTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityManagerClient> client_{ nullptr };
    sptr<AbilityManagerStubTestMock> mock_{ nullptr };
    sptr<SessionInfo> MockSessionInfo(uint64_t persistentId);
};

    sptr<SessionInfo> AbilityManagerClientBranchTest::MockSessionInfo(uint64_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        HILOG_ERROR("sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

void AbilityManagerClientBranchTest::SetUpTestCase(void)
{}
void AbilityManagerClientBranchTest::TearDownTestCase(void)
{}
void AbilityManagerClientBranchTest::TearDown()
{}

void AbilityManagerClientBranchTest::SetUp()
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
HWTEST_F(AbilityManagerClientBranchTest, SendResultToAbility_001, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, StartAbilityByCall_001, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, ReleaseCall_001, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_DumpSysState_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_SetMissionIcon_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_SetMissionIcon_0200, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, ScheduleConnectAbilityDone_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, ScheduleDisconnectAbilityDone_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, StartExtensionAbility_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, StopExtensionAbility_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, TerminateAbility_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, TerminateAbilityResult_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, MinimizeAbility_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, DumpState_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, ForceTimeoutForTest_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, ClearUpApplicationData_0100, TestSize.Level1)
{
    std::string bundleName = "bundleName_test";
    auto result = client_->ClearUpApplicationData(bundleName);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_StartContinuation_0100
 * @tc.desc: StartContinuation
 * @tc.type: FUNC
 * @tc.require: issueI5NRWT
 */
HWTEST_F(AbilityManagerClientBranchTest, StartContinuation_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, NotifyContinuationResult_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, LockMissionForCleanup_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, UnlockMissionForCleanup_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, RegisterMissionListener_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, RegisterMissionListener_0200, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, UnRegisterMissionListener_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, UnRegisterMissionListener_0200, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, GetMissionInfos_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, GetMissionSnapshot_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, CleanMission_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, CleanAllMissions_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, MoveMissionToFront_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, MoveMissionToFront_0200, TestSize.Level1)
{
    StartOptions startOptions;
    auto result = client_->MoveMissionToFront(1, startOptions);
    EXPECT_EQ(result, ERR_OK);
}


/**
 * @tc.name: AbilityManagerClient_MoveMissionsToForeground_0100
 * @tc.desc: MoveMissionsToForeground
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, MoveMissionsToForeground_0100, TestSize.Level1)
{
    auto result = client_->MoveMissionsToForeground({1, 2, 3}, 1);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_MoveMissionsToBackground_0100
 * @tc.desc: MoveMissionsToBackground
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, MoveMissionsToBackground_0100, TestSize.Level1)
{
    std::vector<int32_t> rs;
    auto result = client_->MoveMissionsToBackground({1, 2, 3}, rs);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_GetAbilityRunningInfos_0100
 * @tc.desc: GetAbilityRunningInfos
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, GetAbilityRunningInfos_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, ScheduleCommandAbilityDone_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto result = client_->ScheduleCommandAbilityDone(token);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_ScheduleCommandAbilityWindowDone_0100
 * @tc.desc: ScheduleCommandAbilityWindowDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ScheduleCommandAbilityWindowDone_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    auto result = client_->ScheduleCommandAbilityWindowDone(
        token, sessionInfo, WIN_CMD_FOREGROUND, ABILITY_CMD_FOREGROUND);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_CloseAbility_0100
 * @tc.desc: CloseAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CloseAbility_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, ConnectDataShareExtensionAbility_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, ConnectExtensionAbility_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, ReleaseDataAbility_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, KillProcess_0100, TestSize.Level1)
{
    auto result = client_->KillProcess(BUNDLE_NAME);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: AbilityManagerClient_ContinueMission_0100
 * @tc.desc: ContinueMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ContinueMission_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, NotifyCompleteContinuation_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, GetMissionIdByToken_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, GetExtensionRunningInfos_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, GetProcessRunningInfos_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, StartSyncRemoteMissions_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, StopSyncRemoteMissions_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, StartUser_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, StopUser_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, RegisterSnapshotHandler_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, RegisterWindowManagerServiceHandler_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, CompleteFirstFrameDrawing_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, StartUserTest_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, DoAbilityForeground_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, DoAbilityBackground_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, SetAbilityController_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, BlockAmsService_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, BlockAppService_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, BlockAbility_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, DumpAbilityInfoDone_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, FreeInstallAbilityFromRemote_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, SendANRProcessID_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, UpdateMissionSnapShot_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, OnRemoteDied_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, CallRequestDone_0100, TestSize.Level1)
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
HWTEST_F(AbilityManagerClientBranchTest, IsValidMissionIds_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidMissionIds_0100 start";
    std::vector<int32_t> missionIds;
    std::vector<MissionVaildResult> results;
    EXPECT_CALL(*mock_, IsValidMissionIds(_, _)).Times(1).WillOnce(Return(ERR_OK));
    EXPECT_EQ(client_->IsValidMissionIds(missionIds, results), ERR_OK);
    GTEST_LOG_(INFO) << "IsValidMissionIds_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StartAbilityAsCaller_0100
 * @tc.desc: StartAbilityAsCaller
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartAbilityAsCaller_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityAsCaller_0100 start";
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 1;
    int32_t userId = 2;
    EXPECT_EQ(client_->StartAbilityAsCaller(want, callerToken, requestCode, userId), ERR_OK);
    GTEST_LOG_(INFO) << "StartAbilityAsCaller_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_StartAbilityAsCaller_0200
 * @tc.desc: StartAbilityAsCaller
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartAbilityAsCaller_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityAsCaller_0200 start";
    Want want;
    StartOptions startOptions;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = 1;
    int32_t userId = 2;
    EXPECT_EQ(client_->StartAbilityAsCaller(want, startOptions, callerToken, requestCode, userId), ERR_OK);
    GTEST_LOG_(INFO) << "StartAbilityAsCaller_0200 end";
}

/**
 * @tc.name: AbilityManagerClient_StartUIExtensionAbility_0100
 * @tc.desc: StartUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartUIExtensionAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartUIExtensionAbility_0100 start";
    Want want;
    EXPECT_EQ(client_->StartUIExtensionAbility(want, nullptr, 100, AppExecFwk::ExtensionAbilityType::UI),
        ERR_OK);
    GTEST_LOG_(INFO) << "StartUIExtensionAbility_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_TerminateUIExtensionAbility_0100
 * @tc.desc: TerminateUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, TerminateUIExtensionAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateUIExtensionAbility_0100 start";
    Want* resultWant = nullptr;
    EXPECT_EQ(client_->TerminateUIExtensionAbility(nullptr, 1, resultWant), ERR_OK);
    GTEST_LOG_(INFO) << "TerminateUIExtensionAbility_0100 end";
}

/**
 * @tc.name: AbilityManagerClient_MinimizeUIExtensionAbility_0100
 * @tc.desc: MinimizeUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, MinimizeUIExtensionAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MinimizeUIExtensionAbility_0100 start";
    EXPECT_EQ(client_->MinimizeUIExtensionAbility(nullptr, true), ERR_OK);
    GTEST_LOG_(INFO) << "MinimizeUIExtensionAbility_0100 end";
}

#ifdef ABILITY_COMMAND_FOR_TEST
/**
 * @tc.name: AbilityManagerClient_ForceTimeoutForTest_0100
 * @tc.desc: ForceTimeoutForTest
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ForceTimeoutForTest_001, TestSize.Level1)
{
    EXPECT_TRUE(client_->ForceTimeoutForTest("clean", ""), ERR_OK);
}
#endif

/**
 * @tc.name: AbilityManagerClient_StartAbilityByCall_0200
 * @tc.desc: StartAbilityByCall
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartAbilityByCall_002, TestSize.Level1)
{
    Want want;
    EXPECT_EQ(client_->StartAbilityByCall(want, nullptr), ERR_OK);
    client_->EnableRecoverAbility(nullptr);
    EXPECT_EQ(client_->AddFreeInstallObserver(nullptr), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_StartUIAbilityBySCB_0100
 * @tc.desc: StartUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartUIAbilityBySCB_001, TestSize.Level1)
{
    EXPECT_EQ(AbilityManagerClient::GetInstance()->StartUIAbilityBySCB(nullptr), ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerClient_StartUIAbilityBySCB_0200
 * @tc.desc: StartUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartUIAbilityBySCB_002, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    EXPECT_EQ(AbilityManagerClient::GetInstance()->StartUIAbilityBySCB(sessionInfo), ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerClient_StartUIAbilityBySCB_0300
 * @tc.desc: StartUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, StartUIAbilityBySCB_003, TestSize.Level1)
{
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    EXPECT_EQ(AbilityManagerClient::GetInstance()->StartUIAbilityBySCB(sessionInfo),
        ERR_WRONG_INTERFACE_CALL);
}

/**
 * @tc.name: AbilityManagerClient_MinimizeUIAbilityBySCB_0100
 * @tc.desc: MinimizeUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, MinimizeUIAbilityBySCB_001, TestSize.Level1)
{
    EXPECT_EQ(AbilityManagerClient::GetInstance()->MinimizeUIAbilityBySCB(nullptr), ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerClient_MinimizeUIAbilityBySCB_0200
 * @tc.desc: MinimizeUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, MinimizeUIAbilityBySCB_002, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    EXPECT_EQ(AbilityManagerClient::GetInstance()->MinimizeUIAbilityBySCB(sessionInfo), ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerClient_MinimizeUIAbilityBySCB_0300
 * @tc.desc: MinimizeUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, MinimizeUIAbilityBySCB_003, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    Rosen::SessionInfo info;
    sessionInfo->sessionToken = new (std::nothrow) Rosen::Session(info);
    EXPECT_EQ(AbilityManagerClient::GetInstance()->MinimizeUIAbilityBySCB(sessionInfo), ERR_WRONG_INTERFACE_CALL);
}

/**
 * @tc.name: AbilityManagerClient_CloseUIAbilityBySCB_0100
 * @tc.desc: CloseUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CloseUIAbilityBySCB_001, TestSize.Level1)
{
    EXPECT_EQ(AbilityManagerClient::GetInstance()->CloseUIAbilityBySCB(nullptr), ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerClient_CloseUIAbilityBySCB_0200
 * @tc.desc: CloseUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CloseUIAbilityBySCB_002, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    EXPECT_EQ(AbilityManagerClient::GetInstance()->CloseUIAbilityBySCB(sessionInfo), ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerClient_CloseUIAbilityBySCB_0300
 * @tc.desc: CloseUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CloseUIAbilityBySCB_003, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    Rosen::SessionInfo info;
    sessionInfo->sessionToken = new (std::nothrow) Rosen::Session(info);
    EXPECT_EQ(AbilityManagerClient::GetInstance()->CloseUIAbilityBySCB(sessionInfo), ERR_WRONG_INTERFACE_CALL);
}

/**
 * @tc.name: AbilityManagerClient_SetRootSceneSession_0100
 * @tc.desc: SetRootSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, SetRootSceneSession_001, TestSize.Level1)
{
    Rosen::SessionInfo info;
    sptr<Rosen::ISession> session = new Rosen::Session(info);
    EXPECT_NE(session, nullptr);
    sptr<IRemoteObject> rootSceneSession = session->AsObject();
    AbilityManagerClient::GetInstance()->SetRootSceneSession(rootSceneSession);
    EXPECT_NE(session, nullptr);
}

/**
 * @tc.name: AbilityManagerClient_CallUIAbilityBySCB_0100
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CallUIAbilityBySCB_001, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    AbilityManagerClient::GetInstance()->CallUIAbilityBySCB(sessionInfo);
    EXPECT_NE(sessionInfo, nullptr);
}

/**
 * @tc.name: AbilityManagerClient_CallUIAbilityBySCB_0200
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, CallUIAbilityBySCB_002, TestSize.Level1)
{
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    EXPECT_NE(sessionInfo, nullptr);
    AbilityManagerClient::GetInstance()->CallUIAbilityBySCB(sessionInfo);
    EXPECT_NE(sessionInfo, nullptr);
}

/**
 * @tc.name: AbilityManagerClient_ConnectAbility_0100
 * @tc.desc: ConnectAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ConnectAbility_0100, TestSize.Level1)
{
    Want want;
    sptr<IAbilityConnection> connect = nullptr;
    int32_t userId = 1;
    EXPECT_EQ(client_->ConnectAbility(want, connect, userId), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_ConnectUIExtensionAbility_0100
 * @tc.desc: ConnectUIExtensionAbility
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ConnectUIExtensionAbility_0100, TestSize.Level1)
{
    Want want;
    sptr<IAbilityConnection> connect = nullptr;
    sptr<SessionInfo> sessionInfo = nullptr;
    int32_t userId = 1;
    EXPECT_EQ(client_->ConnectUIExtensionAbility(want, connect, sessionInfo, userId), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_GetMissionInfo_0100
 * @tc.desc: GetMissionInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, GetMissionInfo_0100, TestSize.Level1)
{
    std::string deviceId = "123";
    int32_t missionId = 1;
    MissionInfo missionInfo;
    EXPECT_EQ(client_->GetMissionInfo(deviceId, missionId, missionInfo), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_AcquireShareData_0100
 * @tc.desc: AcquireShareData
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AcquireShareData_0100, TestSize.Level1)
{
    int32_t missionId = 1;
    sptr<IAcquireShareDataCallback> shareData = nullptr;
    EXPECT_EQ(client_->AcquireShareData(missionId, shareData), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_ShareDataDone_0100
 * @tc.desc: ShareDataDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, ShareDataDone_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    int32_t resultCode = 1;
    int32_t uniqueId = 1;
    WantParams wantParam;
    EXPECT_EQ(client_->ShareDataDone(token, resultCode, uniqueId, wantParam), ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_GetAbilityTokenByCalleeObj_0100
 * @tc.desc: GetAbilityTokenByCalleeObj
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientBranchTest, AbilityManagerClient_GetAbilityTokenByCalleeObj_0100, TestSize.Level1)
{
    EXPECT_TRUE(client_ != nullptr);
    sptr<IRemoteObject> callStub = nullptr;
    sptr<IRemoteObject> token = nullptr;
    client_->GetAbilityTokenByCalleeObj(callStub, token);
}

/**
 * @tc.name: AbilityManagerClient_RegisterOnListener_0100
 * @tc.desc: RegisterOnListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, RegisterOnListener_0100, TestSize.Level1)
{
    std::string type = "";
    sptr<IRemoteOnListener> listener = nullptr;
    auto result = client_->RegisterOnListener(type, listener);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_RegisterOffListener_0100
 * @tc.desc: RegisterOffListener
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, RegisterOffListener_0100, TestSize.Level1)
{
    std::string type = "";
    sptr<IRemoteOnListener> listener = nullptr;
    auto result = client_->RegisterOffListener(type, listener);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AbilityManagerClient_RequestDialogService_0100
 * @tc.desc: RequestDialogService
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityManagerClientBranchTest, RequestDialogService_0100, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    auto result = client_->RequestDialogService(want, callerToken);
    EXPECT_EQ(result, ERR_OK);
}
}  // namespace AAFwk
}  // namespace OHOS