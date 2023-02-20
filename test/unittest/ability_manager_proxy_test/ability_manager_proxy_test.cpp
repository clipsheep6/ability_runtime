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
#include "ability_manager_proxy.h"
#undef private

#include "ability_manager_errors.h"
#include "ability_manager_stub_mock.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "ability_scheduler_mock.h"
#include "ability_record.h"
#include "ability_scheduler.h"
#include "mission_snapshot.h"
#include "want_sender_info.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
}  // namespace

class AbilityManagerProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityManagerProxy> proxy_{ nullptr };
    sptr<AbilityManagerStubMock> mock_{ nullptr };
};

void AbilityManagerProxyTest::SetUpTestCase(void)
{}
void AbilityManagerProxyTest::TearDownTestCase(void)
{}
void AbilityManagerProxyTest::TearDown()
{}

void AbilityManagerProxyTest::SetUp()
{
    mock_ = new AbilityManagerStubMock();
    proxy_ = std::make_shared<AbilityManagerProxy>(mock_);
}

/**
 * @tc.name: AbilityManagerProxy_DumpSysState_0100
 * @tc.desc: DumpSysState
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DumpSysState_0100, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerProxy_DumpSysState_0100 start");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    std::string args;
    std::vector<std::string> info;
    bool isClient = false;
    bool isUserID = true;

    proxy_->DumpSysState(args, info, isClient, isUserID, USER_ID);
    EXPECT_EQ(IAbilityManager::DUMPSYS_STATE, mock_->code_);

    HILOG_INFO("AbilityManagerProxy_DumpSysState_0100 end");
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of startability
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    auto res = proxy_->StartAbility(want, 9);

    EXPECT_EQ(IAbilityManager::START_ABILITY, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify that the return value of startability is abnormal
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    const Want want;
    auto res = proxy_->StartAbility(want, 9);

    EXPECT_EQ(IAbilityManager::START_ABILITY, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of TerminateAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_003, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->TerminateAbility(token, -1, &want);

    EXPECT_EQ(IAbilityManager::TERMINATE_ABILITY, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verify that the return value of TerminateAbility is abnormal
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_004, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    const Want want;
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->TerminateAbility(token, -1, &want);

    EXPECT_EQ(IAbilityManager::TERMINATE_ABILITY, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SendResultToAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendResultToAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of SendResultToAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_SendResultToAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    int res1 = proxy_->SendResultToAbility(-1, -1, want);
    EXPECT_EQ(res1, NO_ERROR);
    EXPECT_EQ(IAbilityManager::SEND_RESULT_TO_ABILITY, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: SendResultToAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendResultToAbility
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of SendResultToAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_SendResult_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    Want want;
    int res = proxy_->SendResultToAbility(-1, -1, want);

    EXPECT_EQ(IAbilityManager::SEND_RESULT_TO_ABILITY, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of connectability
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_005, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    want.SetFlags(10);
    sptr<IAbilityConnection> nullConnect = nullptr;
    sptr<IRemoteObject> callerToken = nullptr;
    int res = proxy_->ConnectAbility(want, nullConnect, callerToken);
    EXPECT_NE(res, NO_ERROR);
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res1 = proxy_->ConnectAbility(want, connect, callerToken);
    EXPECT_EQ(res1, NO_ERROR);
    EXPECT_EQ(IAbilityManager::CONNECT_ABILITY_WITH_TYPE, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of connectability
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_006, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    const Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res = proxy_->ConnectAbility(want, connect, callerToken);

    EXPECT_EQ(IAbilityManager::CONNECT_ABILITY_WITH_TYPE, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of disconnectAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_007, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res = proxy_->DisconnectAbility(connect);

    EXPECT_EQ(IAbilityManager::DISCONNECT_ABILITY, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectAbility
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of disconnectAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_008, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    int res = proxy_->DisconnectAbility(connect);

    EXPECT_EQ(IAbilityManager::DISCONNECT_ABILITY, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AttachAbilityThread
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of attachAbilityThread
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_009, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IAbilityScheduler> scheduler = new AbilitySchedulerMock();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->AttachAbilityThread(scheduler, token);

    EXPECT_EQ(IAbilityManager::ATTACH_ABILITY_THREAD, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AttachAbilityThread
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of attachAbilityThread
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_010, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IAbilityScheduler> scheduler = new AbilitySchedulerMock();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->AttachAbilityThread(scheduler, token);

    EXPECT_EQ(IAbilityManager::ATTACH_ABILITY_THREAD, mock_->code_);
    EXPECT_NE(res, NO_ERROR);

    sptr<IAbilityScheduler> nullScheduler = nullptr;
    auto res1 = proxy_->AttachAbilityThread(nullScheduler, token);
    EXPECT_NE(res1, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AbilityTransitionDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AbilityTransitionDone
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of abilityTransitionDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_0011, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    PacMap saveData;
    auto res = proxy_->AbilityTransitionDone(token, 1, saveData);

    EXPECT_EQ(IAbilityManager::ABILITY_TRANSITION_DONE, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AbilityTransitionDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AbilityTransitionDone
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of abilityTransitionDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_012, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    PacMap saveData;
    auto res = proxy_->AbilityTransitionDone(token, 1, saveData);

    EXPECT_EQ(IAbilityManager::ABILITY_TRANSITION_DONE, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of scheduleConnectAbilityDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_0013, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> remoteObject = nullptr;
    auto res = proxy_->ScheduleConnectAbilityDone(token, remoteObject);

    EXPECT_EQ(IAbilityManager::CONNECT_ABILITY_DONE, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of scheduleConnectAbilityDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_014, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> remoteObject = nullptr;
    auto res = proxy_->ScheduleConnectAbilityDone(token, remoteObject);

    EXPECT_EQ(IAbilityManager::CONNECT_ABILITY_DONE, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleDisconnectAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleDisconnectAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of scheduleDisconnectAbilityDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_0015, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto res = proxy_->ScheduleDisconnectAbilityDone(token);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DumpState
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpState
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of dumpState
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_0017, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string args = "aaa";
    std::vector<std::string> info;
    proxy_->DumpState(args, info);

    EXPECT_EQ(IAbilityManager::DUMP_STATE, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbilityResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityResult
 * EnvConditions: NA
 * CaseDescription: Verify the normal conditions of terminateAbilityResult
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_0018, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->TerminateAbilityResult(token, 1);

    EXPECT_EQ(IAbilityManager::TERMINATE_ABILITY_RESULT, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbilityResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityResult
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of terminateAbilityResult
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_019, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->TerminateAbilityResult(token, 1);

    EXPECT_EQ(IAbilityManager::TERMINATE_ABILITY_RESULT, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityDone
 * EnvConditions: NA
 * CaseDescription: Verify the abnormal conditions of ScheduleCommandAbilityDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_026, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->ScheduleCommandAbilityDone(token);

    EXPECT_EQ(IAbilityManager::COMMAND_ABILITY_DONE, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StopServiceAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_027, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    auto res = proxy_->StopServiceAbility(want);

    EXPECT_EQ(IAbilityManager::STOP_SERVICE_ABILITY, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_028
 * @tc.desc: test StartContinuation send request succeeded
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_028, TestSize.Level0)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    int res = proxy_->StartContinuation(want, abilityToken, 0);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(IAbilityManager::START_CONTINUATION, mock_->code_);
}

/**
 * @tc.name: AbilityManagerProxy_029
 * @tc.desc: test StartContinuation send request failed
 * @tc.type: FUNC
 * @tc.require: AR000GI8IL
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_029, TestSize.Level0)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    const Want want;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    int res = proxy_->StartContinuation(want, abilityToken, 0);

    EXPECT_EQ(IAbilityManager::START_CONTINUATION, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerProxy_030
 * @tc.desc: test NotifyContinuationResult send request succeeded
 * @tc.type: FUNC
 * @tc.require: AR000GI8IH
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_030, TestSize.Level0)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t result = 0;
    int res = proxy_->NotifyContinuationResult(0, result);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(IAbilityManager::NOTIFY_CONTINUATION_RESULT, mock_->code_);
}

/**
 * @tc.name: AbilityManagerProxy_031
 * @tc.desc: test NotifyContinuationResult send request failed
 * @tc.type: FUNC
 * @tc.require: AR000GI8IH
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_031, TestSize.Level0)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));
    int32_t result = 0;
    int res = proxy_->NotifyContinuationResult(0, result);

    EXPECT_EQ(IAbilityManager::NOTIFY_CONTINUATION_RESULT, mock_->code_);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function AcquireDataAbility normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_AcquireDataAbility_001, TestSize.Level1)
{
    OHOS::Uri dataAbilityUri("dataability:///data.bundle.DataAbility");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->AcquireDataAbility(dataAbilityUri, true, abilityRecord->GetToken()), nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function AcquireDataAbility callerToken is nullptr.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_AcquireDataAbility_002, TestSize.Level1)
{
    OHOS::Uri dataAbilityUri("dataability:///data.bundle.DataAbility");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(0);
    EXPECT_EQ(proxy_->AcquireDataAbility(dataAbilityUri, true, nullptr), nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function AcquireDataAbility SendRequest return error.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_AcquireDataAbility_003, TestSize.Level1)
{
    OHOS::Uri dataAbilityUri("dataability:///data.bundle.DataAbility");
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(TRANSACTION_ERR));
    EXPECT_EQ(proxy_->AcquireDataAbility(dataAbilityUri, true, abilityRecord->GetToken()), nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseDataAbility normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseDataAbility_001, TestSize.Level1)
{
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_EQ(proxy_->ReleaseDataAbility(scheduler, abilityRecord->GetToken()), 0);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseDataAbility dataAbilityScheduler is nullptr.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseDataAbility_002, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(0);
    EXPECT_EQ(proxy_->ReleaseDataAbility(nullptr, abilityRecord->GetToken()), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseDataAbility callerToken is nullptr.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseDataAbility_003, TestSize.Level1)
{
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(0);
    EXPECT_EQ(proxy_->ReleaseDataAbility(scheduler, nullptr), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseDataAbility SendRequest error.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseDataAbility_004, TestSize.Level1)
{
    OHOS::sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "data.client.bundle";
    abilityRequest.abilityInfo.name = "ClientAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(TRANSACTION_ERR));
    EXPECT_EQ(proxy_->ReleaseDataAbility(scheduler, abilityRecord->GetToken()), TRANSACTION_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByCall
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityByCall connect is nullptr.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityByCall_001, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IAbilityConnection> connect = nullptr;
    EXPECT_EQ(proxy_->StartAbilityByCall(want, connect, callerToken), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByCall
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityByCall is normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityByCall_002, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    EXPECT_EQ(proxy_->StartAbilityByCall(want, connect, callerToken), ERR_OK);
    EXPECT_EQ(IAbilityManager::START_CALL_ABILITY, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseCall
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseCall connect is nullptr.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseCall_001, TestSize.Level1)
{
    AppExecFwk::ElementName element;
    sptr<IAbilityConnection> connect = nullptr;
    EXPECT_EQ(proxy_->ReleaseCall(connect, element), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseCall
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseCall
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseCall is normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ReleaseCall_002, TestSize.Level1)
{
    AppExecFwk::ElementName element;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    EXPECT_EQ(proxy_->ReleaseCall(connect, element), ERR_OK);
    EXPECT_EQ(IAbilityManager::RELEASE_CALL_ABILITY, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: GetTopAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetTopAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function GetTopAbility is normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetTopAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    proxy_->GetTopAbility();
    EXPECT_EQ(IAbilityManager::GET_TOP_ABILITY, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: GetTopAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetTopAbility
 * EnvConditions: NA
 * CaseDescription: Verify the function GetTopAbility is normal flow.
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetTopAbility_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IRemoteObject> token = nullptr;
    proxy_->GetTopAbility(token);
    EXPECT_EQ(IAbilityManager::GET_TOP_ABILITY_TOKEN, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartExtensionAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartExtensionAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartExtensionAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    auto res = proxy_->StartExtensionAbility(want, callerToken);
    EXPECT_EQ(IAbilityManager::START_EXTENSION_ABILITY, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StopExtensionAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StopExtensionAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StopExtensionAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    auto res = proxy_->StopExtensionAbility(want, callerToken);
    EXPECT_EQ(IAbilityManager::STOP_EXTENSION_ABILITY, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbilityByCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityByCaller
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of TerminateAbilityByCaller
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_TerminateAbilityByCaller_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IRemoteObject> callerToken = nullptr;
    auto res = proxy_->TerminateAbilityByCaller(callerToken, -1);
    EXPECT_EQ(IAbilityManager::TERMINATE_ABILITY_BY_CALLER, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of MinimizeAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_MinimizeAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->MinimizeAbility(token);
    EXPECT_EQ(IAbilityManager::MINIMIZE_ABILITY, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetMissionSnapshot
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetMissionSnapshot_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string deviceId = "";
    int32_t missionId = 1;
    MissionSnapshot snapshot;
    bool isLowResolution = true;
    proxy_->GetMissionSnapshot(deviceId, missionId, snapshot, isLowResolution);
    EXPECT_EQ(IAbilityManager::GET_MISSION_SNAPSHOT_INFO, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateMissionSnapShot
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateMissionSnapShot
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of UpdateMissionSnapShot
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_UpdateMissionSnapShot_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    proxy_->UpdateMissionSnapShot(token);
    EXPECT_EQ(IAbilityManager::UPDATE_MISSION_SNAPSHOT, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: EnableRecoverAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService EnableRecoverAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of EnableRecoverAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_EnableRecoverAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    proxy_->EnableRecoverAbility(token);
    EXPECT_EQ(IAbilityManager::ABILITY_RECOVERY_ENABLE, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleRecoverAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleRecoverAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of ScheduleRecoverAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ScheduleRecoverAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    int32_t reason = 0;
    proxy_->ScheduleRecoverAbility(token, reason);
    EXPECT_EQ(IAbilityManager::ABILITY_RECOVERY, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: KillProcess
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillProcess
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of KillProcess
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_KillProcess_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string bundleName = "";
    auto res = proxy_->KillProcess(bundleName);
    EXPECT_EQ(IAbilityManager::KILL_PROCESS, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: ForceTimeoutForTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ForceTimeoutForTest
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of ForceTimeoutForTest
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ForceTimeoutForTest_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string abilityName = "";
    std::string state = "";
    auto res = proxy_->ForceTimeoutForTest(abilityName, state);
    EXPECT_EQ(IAbilityManager::FORCE_TIMEOUT, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}
#endif

/*
 * Feature: AbilityManagerService
 * Function: ClearUpApplicationData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ClearUpApplicationData
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of ClearUpApplicationData
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ClearUpApplicationData_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string bundleName = "";
    auto res = proxy_->ClearUpApplicationData(bundleName);
    EXPECT_EQ(IAbilityManager::CLEAR_UP_APPLICATION_DATA, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: UninstallApp
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UninstallApp
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of UninstallApp
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_UninstallApp_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string bundleName = "";
    int32_t uid = 1;
    auto res = proxy_->UninstallApp(bundleName, uid);
    EXPECT_EQ(IAbilityManager::UNINSTALL_APP, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetWantSender
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSender
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetWantSender
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetWantSender_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    WantSenderInfo wantSenderInfo;
    sptr<IRemoteObject> callerToken = nullptr;
    auto res = proxy_->GetWantSender(wantSenderInfo, callerToken);
    EXPECT_EQ(IAbilityManager::GET_PENDING_WANT_SENDER, mock_->code_);
    EXPECT_TRUE(res == nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: SendWantSender
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendWantSender
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of SendWantSender
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_SendWantSender_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    SenderInfo senderInfo;
    auto res = proxy_->SendWantSender(target, senderInfo);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantUid
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantUid
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetPendingWantUid
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetPendingWantUid_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    auto res = proxy_->GetPendingWantUid(target);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantUserId
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantUserId
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetPendingWantUserId
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetPendingWantUserId_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    auto res = proxy_->GetPendingWantUserId(target);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantBundleName
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetPendingWantBundleName
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetPendingWantBundleName_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    auto res = proxy_->GetPendingWantBundleName(target);
    EXPECT_EQ(res, "");
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantCode
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantCode
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetPendingWantCode
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetPendingWantCode_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    auto res = proxy_->GetPendingWantCode(target);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantType
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantType
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetPendingWantType
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetPendingWantType_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    auto res = proxy_->GetPendingWantType(target);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingRequestWant
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingRequestWant
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetPendingRequestWant
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetPendingRequestWant_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    auto want = std::make_shared<Want>();
    auto res = proxy_->GetPendingRequestWant(target, want);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetWantSenderInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSenderInfo
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetWantSenderInfo
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetWantSenderInfo_001, TestSize.Level1)
{
    sptr<IWantSender> target = nullptr;
    auto info = std::make_shared<WantSenderInfo>();
    auto res = proxy_->GetWantSenderInfo(target, info);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAppMemorySize
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAppMemorySize
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetAppMemorySize
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetAppMemorySize_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto res = proxy_->GetAppMemorySize();
    EXPECT_EQ(IAbilityManager::GET_APP_MEMORY_SIZE, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: IsRamConstrainedDevice
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRamConstrainedDevice
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of IsRamConstrainedDevice
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_IsRamConstrainedDevice_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto res = proxy_->IsRamConstrainedDevice();
    EXPECT_EQ(IAbilityManager::IS_RAM_CONSTRAINED_DEVICE, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueMission
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of ContinueMission
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ContinueMission_001, TestSize.Level1)
{
    std::string srcDeviceId = "";
    std::string dstDeviceId = "";
    int32_t missionId = 1;
    const sptr<IRemoteObject>& callBack = nullptr;
    AAFwk::WantParams wantParams;
    auto res = proxy_->ContinueMission(srcDeviceId, dstDeviceId, missionId, callBack, wantParams);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of ContinueAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_ContinueAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string deviceId = "";
    int32_t missionId = 1;
    uint32_t versionCode = 1;
    auto res = proxy_->ContinueAbility(deviceId, missionId, versionCode);
    EXPECT_EQ(IAbilityManager::CONTINUE_ABILITY, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyCompleteContinuation
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyCompleteContinuation
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of NotifyCompleteContinuation
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_NotifyCompleteContinuation_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string deviceId = "";
    int32_t sessionId = 1;
    bool isSuccess = true;
    proxy_->NotifyCompleteContinuation(deviceId, sessionId, isSuccess);
    EXPECT_EQ(IAbilityManager::NOTIFY_COMPLETE_CONTINUATION, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyContinuationResult
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyContinuationResult
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of NotifyContinuationResult
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_NotifyContinuationResult_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t missionId = 1;
    int32_t result = 1;
    auto res = proxy_->NotifyContinuationResult(missionId, result);
    EXPECT_EQ(IAbilityManager::NOTIFY_CONTINUATION_RESULT, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: LockMissionForCleanup
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService LockMissionForCleanup
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of LockMissionForCleanup
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_LockMissionForCleanup_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t missionId = 1;
    auto res = proxy_->LockMissionForCleanup(missionId);
    EXPECT_EQ(IAbilityManager::LOCK_MISSION_FOR_CLEANUP, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: UnlockMissionForCleanup
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnlockMissionForCleanup
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of UnlockMissionForCleanup
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_UnlockMissionForCleanup_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t missionId = 1;
    auto res = proxy_->UnlockMissionForCleanup(missionId);
    EXPECT_EQ(IAbilityManager::UNLOCK_MISSION_FOR_CLEANUP, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterMissionListener
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of RegisterMissionListener
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_RegisterMissionListener_001, TestSize.Level1)
{
    sptr<IMissionListener> listener = nullptr;
    auto res = proxy_->RegisterMissionListener(listener);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: UnRegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnRegisterMissionListener
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of UnRegisterMissionListener
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_UnRegisterMissionListener_001, TestSize.Level1)
{
    sptr<IMissionListener> listener = nullptr;
    auto res = proxy_->UnRegisterMissionListener(listener);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: CleanMission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CleanMission
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of CleanMission
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_CleanMission_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t missionId = 1;
    auto res = proxy_->CleanMission(missionId);
    EXPECT_EQ(IAbilityManager::CLEAN_MISSION, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: CleanAllMissions
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CleanAllMissions
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of CleanAllMissions
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_CleanAllMissions_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto res = proxy_->CleanAllMissions();
    EXPECT_EQ(IAbilityManager::CLEAN_ALL_MISSIONS, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToFront
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToFront
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of MoveMissionToFront
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_MoveMissionToFront_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t missionId = 1;
    auto res = proxy_->MoveMissionToFront(missionId);
    EXPECT_EQ(IAbilityManager::MOVE_MISSION_TO_FRONT, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUser
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUser
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartUser
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartUser_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int userId = 1;
    auto res = proxy_->StartUser(userId);
    EXPECT_EQ(IAbilityManager::START_USER, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StopUser
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopUser
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StopUser
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StopUser_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int userId = 1;
    sptr<IStopUserCallback> callback = nullptr;
    auto res = proxy_->StopUser(userId, callback);
    EXPECT_EQ(IAbilityManager::STOP_USER, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SetMissionLabel
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionLabel
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of SetMissionLabel
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_SetMissionLabel_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::string label = "";
    auto res = proxy_->SetMissionLabel(token, label);
    EXPECT_EQ(IAbilityManager::SET_MISSION_LABEL, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterWindowManagerServiceHandler
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterWindowManagerServiceHandler
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of RegisterWindowManagerServiceHandler
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_RegisterWindowManagerServiceHandler_001, TestSize.Level1)
{
    sptr<IWindowManagerServiceHandler> handler = nullptr;
    auto res = proxy_->RegisterWindowManagerServiceHandler(handler);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: CompleteFirstFrameDrawing
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CompleteFirstFrameDrawing
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of CompleteFirstFrameDrawing
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_CompleteFirstFrameDrawing_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    proxy_->CompleteFirstFrameDrawing(abilityToken);
    EXPECT_EQ(IAbilityManager::COMPLETEFIRSTFRAMEDRAWING, mock_->code_);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityRunningInfos
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetAbilityRunningInfos
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetAbilityRunningInfos_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::vector<AbilityRunningInfo> info;
    auto res = proxy_->GetAbilityRunningInfos(info);
    EXPECT_EQ(IAbilityManager::GET_ABILITY_RUNNING_INFO, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetExtensionRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetExtensionRunningInfos
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetExtensionRunningInfos
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetExtensionRunningInfos_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int upperLimit = 0;
    std::vector<ExtensionRunningInfo> info;
    auto res = proxy_->GetExtensionRunningInfos(upperLimit, info);
    EXPECT_EQ(IAbilityManager::GET_EXTENSION_RUNNING_INFO, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetProcessRunningInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetProcessRunningInfos
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetProcessRunningInfos
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetProcessRunningInfos_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::vector<AppExecFwk::RunningProcessInfo> info;
    auto res = proxy_->GetProcessRunningInfos(info);
    EXPECT_EQ(IAbilityManager::GET_PROCESS_RUNNING_INFO, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartSyncRemoteMissions
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartSyncRemoteMissions
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartSyncRemoteMissions
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartSyncRemoteMissions_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string devId = "";
    bool fixConflict = true;
    int64_t tag = 0;
    auto res = proxy_->StartSyncRemoteMissions(devId, fixConflict, tag);
    EXPECT_EQ(IAbilityManager::START_SYNC_MISSIONS, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StopSyncRemoteMissions
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopSyncRemoteMissions
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StopSyncRemoteMissions
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StopSyncRemoteMissions_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string devId = "";
    auto res = proxy_->StopSyncRemoteMissions(devId);
    EXPECT_EQ(IAbilityManager::STOP_SYNC_MISSIONS, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SetAbilityController
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetAbilityController
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of SetAbilityController
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_SetAbilityController_001, TestSize.Level1)
{
    sptr<AppExecFwk::IAbilityController> abilityController = nullptr;
    bool imAStabilityTest = true;
    auto res = proxy_->SetAbilityController(abilityController, imAStabilityTest);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: IsRunningInStabilityTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRunningInStabilityTest
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of IsRunningInStabilityTest
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_IsRunningInStabilityTest_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto res = proxy_->IsRunningInStabilityTest();
    EXPECT_EQ(IAbilityManager::IS_USER_A_STABILITY_TEST, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUserTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUserTest
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartUserTest
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartUserTest_001, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> observer = nullptr;
    auto res = proxy_->StartUserTest(want, observer);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: FinishUserTest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService FinishUserTest
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of FinishUserTest
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_FinishUserTest_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    std::string msg = "";
    int64_t resultCode = 0;
    std::string bundleName = "";
    auto res = proxy_->FinishUserTest(msg, resultCode, bundleName);
    EXPECT_EQ(IAbilityManager::FINISH_USER_TEST, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DelegatorDoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityForeground
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of DelegatorDoAbilityForeground
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DelegatorDoAbilityForeground_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto res = proxy_->DelegatorDoAbilityForeground(token);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DelegatorDoAbilityBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityBackground
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of DelegatorDoAbilityBackground
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DelegatorDoAbilityBackground_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    auto res = proxy_->DelegatorDoAbilityBackground(token);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityForeground
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of DoAbilityForeground
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DoAbilityForeground_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 0;
    auto res = proxy_->DoAbilityForeground(token, flag);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityBackground
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of DoAbilityBackground
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DoAbilityBackground_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 0;
    auto res = proxy_->DoAbilityBackground(token, flag);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: SendANRProcessID
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendANRProcessID
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of SendANRProcessID
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_SendANRProcessID_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int pid = 0;
    auto res = proxy_->SendANRProcessID(pid);
    EXPECT_EQ(IAbilityManager::SEND_APP_NOT_RESPONSE_PROCESS_ID, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionIdByToken
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionIdByToken
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of GetMissionIdByToken
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_GetMissionIdByToken_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto res = proxy_->GetMissionIdByToken(token);
    EXPECT_EQ(IAbilityManager::GET_MISSION_ID_BY_ABILITY_TOKEN, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: BlockAmsService
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAmsService
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of BlockAmsService
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_BlockAmsService_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto res = proxy_->BlockAmsService();
    EXPECT_EQ(IAbilityManager::BLOCK_AMS_SERVICE, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}
#endif

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: BlockAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of BlockAbility
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_BlockAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    int32_t abilityRecordId = 0;
    auto res = proxy_->BlockAbility(abilityRecordId);
    EXPECT_EQ(IAbilityManager::BLOCK_ABILITY, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}
#endif

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: BlockAppService
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAppService
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of BlockAppService
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_BlockAppService_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    auto res = proxy_->BlockAppService();
    EXPECT_EQ(IAbilityManager::BLOCK_APP_SERVICE, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}
#endif

/*
 * Feature: AbilityManagerService
 * Function: FreeInstallAbilityFromRemote
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService FreeInstallAbilityFromRemote
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of FreeInstallAbilityFromRemote
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_FreeInstallAbilityFromRemote_001, TestSize.Level1)
{
    Want want;
    sptr<IRemoteObject> callback = nullptr;
    int32_t userId = 0;
    int requestCode = 0;
    auto res = proxy_->FreeInstallAbilityFromRemote(want, callback, userId, requestCode);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: DumpAbilityInfoDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpAbilityInfoDone
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of DumpAbilityInfoDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_DumpAbilityInfoDone_001, TestSize.Level1)
{
    std::vector<std::string> infos;
    sptr<IRemoteObject> callerToken = nullptr;
    auto res = proxy_->DumpAbilityInfoDone(infos, callerToken);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of startability
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbility_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    auto res = proxy_->StartAbility(want, callerToken);
    EXPECT_EQ(IAbilityManager::START_ABILITY_ADD_CALLER, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCaller
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartAbilityAsCaller
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityAsCaller_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    auto res = proxy_->StartAbilityAsCaller(want, callerToken);
    EXPECT_EQ(IAbilityManager::START_ABILITY_AS_CALLER_BY_TOKEN, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCaller
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCaller
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of StartAbilityAsCaller
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_StartAbilityAsCaller_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));
    const Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    StartOptions startOptions;
    auto res = proxy_->StartAbilityAsCaller(want, startOptions, callerToken);
    EXPECT_EQ(IAbilityManager::START_ABILITY_AS_CALLER_FOR_OPTIONS, mock_->code_);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: CallRequestDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CallRequestDone
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of CallRequestDone
 */
HWTEST_F(AbilityManagerProxyTest, AbilityManagerProxy_CallRequestDone_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> callStub = nullptr;
    proxy_->CallRequestDone(token, callStub);
    EXPECT_TRUE(true);
}
}  // namespace AAFwk
}  // namespace OHOS
