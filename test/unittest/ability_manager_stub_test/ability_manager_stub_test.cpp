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
#include "ability_manager_stub_impl_mock.h"
#include "ability_scheduler.h"
#include "app_debug_listener_stub_mock.h"
#include "iremote_proxy.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
}  // namespace

class AbilityManagerStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void WriteInterfaceToken(MessageParcel& data);
    sptr<AbilityManagerStubImplMock> stub_{ nullptr };
};

void AbilityManagerStubTest::SetUpTestCase(void)
{}
void AbilityManagerStubTest::TearDownTestCase(void)
{}
void AbilityManagerStubTest::TearDown()
{}

void AbilityManagerStubTest::SetUp()
{
    stub_ = new AbilityManagerStubImplMock();
}

void AbilityManagerStubTest::WriteInterfaceToken(MessageParcel& data)
{
    data.WriteInterfaceToken(AbilityManagerStub::GetDescriptor());
}

/**
 * @tc.name: AbilityManagerStub_DumpSysStateInner_0100
 * @tc.desc: DumpSysStateInner
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DumpSysStateInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerStub_DumpSysStateInner_0100 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);

    std::string args = "-a";
    data.WriteString16(Str8ToStr16(args));

    bool isClient = false;
    data.WriteBool(isClient);

    bool isUserID = true;
    data.WriteBool(isUserID);

    data.WriteInt32(USER_ID);

    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMPSYS_STATE),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerStub_DumpSysStateInner_0100 end");
}  // namespace AAFwk

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_ABILITY
 * CaseDescription: Verify that on remote request is normal and abnormal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    Want want;
    WriteInterfaceToken(data);
    want.SetFlags(10);
    data.WriteParcelable(&want);
    data.WriteInt32(1);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);

    data.WriteParcelable(nullptr);
    data.WriteInt32(1);
    int res1 = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY),
        data, reply, option);
    EXPECT_NE(res1, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is TERMINATE_ABILITY
 * CaseDescription: Verify that on remote request is normal and abnormal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    Want want;
    want.SetFlags(10);
    OHOS::sptr<IRemoteObject> token = nullptr;
    WriteInterfaceToken(data);
    data.WriteParcelable(token);
    data.WriteParcelable(&want);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is CONNECT_ABILITY
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_004, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    Want want;
    want.SetFlags(10);
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    sptr<IRemoteObject> callerToken = nullptr;
    WriteInterfaceToken(data);
    data.WriteParcelable(&want);
    data.WriteParcelable(connect->AsObject());
    data.WriteParcelable(callerToken);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: OnRemoteRequest static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_005, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    Want want;
    want.SetFlags(10);
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    sptr<IRemoteObject> callerToken = nullptr;
    WriteInterfaceToken(data);
    data.WriteParcelable(&want);
    data.WriteParcelable(connect->AsObject());
    data.WriteParcelable(callerToken);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: OnRemoteRequest static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_006, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    sptr<IRemoteObject> callerToken = nullptr;
    WriteInterfaceToken(data);
    data.WriteParcelable(nullptr);
    data.WriteParcelable(connect->AsObject());
    data.WriteParcelable(callerToken);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY),
        data, reply, option);

    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is DISCONNECT_ABILITY
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_007, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    WriteInterfaceToken(data);
    data.WriteParcelable(connect->AsObject());
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::DISCONNECT_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is ATTACH_ABILITY_THREAD
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_008, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IAbilityScheduler> scheduler = new AbilityScheduler();
    sptr<IRemoteObject> token = nullptr;
    WriteInterfaceToken(data);
    data.WriteParcelable(scheduler->AsObject());
    data.WriteParcelable(token);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::ATTACH_ABILITY_THREAD),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is ABILITY_TRANSITION_DONE
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_009, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IRemoteObject> token = sptr<AppExecFwk::MockAbilityToken>(new (std::nothrow) AppExecFwk::MockAbilityToken());
    WriteInterfaceToken(data);
    bool ret = data.WriteRemoteObject(token);
    ret |= data.WriteInt32(1);
    PacMap pMap;
    pMap.PutIntValue(std::string("1"), 1);
    ret |= data.WriteParcelable(&pMap);
    if (ret) {
        int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_TRANSITION_DONE),
        data, reply, option);
        EXPECT_EQ(res, NO_ERROR);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is CONNECT_ABILITY_DONE
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_010, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> remoteObject = nullptr;
    WriteInterfaceToken(data);
    data.WriteParcelable(token);
    data.WriteParcelable(remoteObject);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY_DONE),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is DISCONNECT_ABILITY_DONE
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_011, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    sptr<IRemoteObject> token = nullptr;
    WriteInterfaceToken(data);
    data.WriteParcelable(token);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::DISCONNECT_ABILITY_DONE),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is SEND_RESULT_TO_ABILITY
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_012, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int requestCode = -1;
    int resultCode = -1;
    Want want;
    data.WriteInt32(requestCode);
    data.WriteInt32(resultCode);
    data.WriteParcelable(&want);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_RESULT_TO_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is DUMP_STATE
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_013, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    std::string args = "aaa";
    std::vector<std::string> info;
    WriteInterfaceToken(data);
    data.WriteString16(Str8ToStr16(args));
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMP_STATE),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is default
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_016, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(5000, data, reply, option);

    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_CALL_ABILITY
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_018, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    Want want;
    WriteInterfaceToken(data);
    want.SetFlags(10);
    data.WriteParcelable(&want);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CALL_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_CALL_ABILITY
 * CaseDescription: Verify that on remote request is ERR_INVALID_VALUE
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_019, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CALL_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_CALL_ABILITY
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_020, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    AppExecFwk::ElementName element;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    data.WriteParcelable(connect->AsObject());
    data.WriteParcelable(&element);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_CALL_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService OnRemoteRequest
 * EnvConditions: code is START_CALL_ABILITY
 * CaseDescription: Verify that on remote request is ERR_INVALID_VALUE
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_021, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_CALL_ABILITY),
        data, reply, option);

    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetTopAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetTopAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetTopAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetTopAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetTopAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function TerminateAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_TerminateAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->TerminateAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SendResultToAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendResultToAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SendResultToAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SendResultToAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SendResultToAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: MinimizeAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MinimizeAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function MinimizeAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_MinimizeAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->MinimizeAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AttachAbilityThreadInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AttachAbilityThreadInner
 * EnvConditions: NA
 * CaseDescription: Verify the function AttachAbilityThreadInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_AttachAbilityThreadInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->AttachAbilityThreadInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AbilityTransitionDoneInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AbilityTransitionDoneInner
 * EnvConditions: NA
 * CaseDescription: Verify the function AbilityTransitionDoneInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_AbilityTransitionDoneInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->AbilityTransitionDoneInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleConnectAbilityDoneInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleConnectAbilityDoneInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ScheduleConnectAbilityDoneInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ScheduleConnectAbilityDoneInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ScheduleConnectAbilityDoneInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleDisconnectAbilityDoneInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleDisconnectAbilityDoneInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ScheduleDisconnectAbilityDoneInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ScheduleDisconnectAbilityDoneInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ScheduleDisconnectAbilityDoneInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityDoneInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityDoneInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ScheduleCommandAbilityDoneInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ScheduleCommandAbilityDoneInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ScheduleCommandAbilityDoneInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ScheduleCommandAbilityWindowDoneInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleCommandAbilityWindowDoneInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ScheduleCommandAbilityWindowDoneInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ScheduleCommandAbilityWindowDoneInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ScheduleCommandAbilityWindowDoneInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: AcquireDataAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService AcquireDataAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function AcquireDataAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_AcquireDataAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->AcquireDataAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseDataAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseDataAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseDataAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ReleaseDataAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ReleaseDataAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: KillProcessInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService KillProcessInner
 * EnvConditions: NA
 * CaseDescription: Verify the function KillProcessInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_KillProcessInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->KillProcessInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ClearUpApplicationDataInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ClearUpApplicationDataInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ClearUpApplicationDataInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ClearUpApplicationDataInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ClearUpApplicationDataInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: UninstallAppInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UninstallAppInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UninstallAppInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UninstallAppInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UninstallAppInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartExtensionAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartExtensionAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartExtensionAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartExtensionAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StopExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopExtensionAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StopExtensionAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StopExtensionAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StopExtensionAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAddCallerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAddCallerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityAddCallerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityAddCallerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityAddCallerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCallerByTokenInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCallerByTokenInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityAsCallerByTokenInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityAsCallerByTokenInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityAsCallerByTokenInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityAsCallerForOptionInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityAsCallerForOptionInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityAsCallerForOptionInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityAsCallerForOptionInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityAsCallerForOptionInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ConnectAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ConnectAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ConnectAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ConnectAbilityWithTypeInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ConnectAbilityWithTypeInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ConnectAbilityWithTypeInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ConnectAbilityWithTypeInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ConnectAbilityWithTypeInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DisconnectAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DisconnectAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DisconnectAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DisconnectAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DisconnectAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StopServiceAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StopServiceAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StopServiceAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StopServiceAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DumpSysStateInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpSysStateInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DumpSysStateInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DumpSysStateInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DumpSysStateInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DumpStateInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpStateInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DumpStateInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DumpStateInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DumpStateInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForSettingsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityForSettingsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityForSettingsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityForSettingsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityForSettingsInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityForOptionsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityForOptionsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityForOptionsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityForOptionsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityForOptionsInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetWantSenderInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSenderInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetWantSenderInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetWantSenderInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetWantSenderInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: SendWantSenderInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendWantSenderInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SendWantSenderInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SendWantSenderInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SendWantSenderInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: CancelWantSenderInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CancelWantSenderInner
 * EnvConditions: NA
 * CaseDescription: Verify the function CancelWantSenderInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CancelWantSenderInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CancelWantSenderInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantUidInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantUidInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetPendingWantUidInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetPendingWantUidInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetPendingWantUidInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantUserIdInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantUserIdInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetPendingWantUserIdInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetPendingWantUserIdInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetPendingWantUserIdInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantBundleNameInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantBundleNameInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetPendingWantBundleNameInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetPendingWantBundleNameInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetPendingWantBundleNameInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantCodeInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantCodeInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetPendingWantCodeInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetPendingWantCodeInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetPendingWantCodeInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingWantTypeInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingWantTypeInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetPendingWantTypeInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetPendingWantTypeInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetPendingWantTypeInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterCancelListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterCancelListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterCancelListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterCancelListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterCancelListenerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: UnregisterCancelListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnregisterCancelListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UnregisterCancelListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnregisterCancelListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UnregisterCancelListenerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetPendingRequestWantInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetPendingRequestWantInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetPendingRequestWantInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetPendingRequestWantInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetPendingRequestWantInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetWantSenderInfoInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetWantSenderInfoInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetWantSenderInfoInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetWantSenderInfoInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetWantSenderInfoInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAppMemorySizeInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAppMemorySizeInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetAppMemorySizeInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetAppMemorySizeInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetAppMemorySizeInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: IsRamConstrainedDeviceInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRamConstrainedDeviceInner
 * EnvConditions: NA
 * CaseDescription: Verify the function IsRamConstrainedDeviceInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_IsRamConstrainedDeviceInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->IsRamConstrainedDeviceInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueMissionInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueMissionInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ContinueMissionInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ContinueMissionInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ContinueMissionInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueMissionOfBundleNameInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueMissionOfBundleNameInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ContinueMissionOfBundleNameInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ContinueMissionOfBundleNameInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ContinueMissionOfBundleNameInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: ContinueAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ContinueAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ContinueAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ContinueAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ContinueAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartContinuationInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartContinuationInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartContinuationInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartContinuationInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartContinuationInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyCompleteContinuationInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyCompleteContinuationInner
 * EnvConditions: NA
 * CaseDescription: Verify the function NotifyCompleteContinuationInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_NotifyCompleteContinuationInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->NotifyCompleteContinuationInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyContinuationResultInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyContinuationResultInner
 * EnvConditions: NA
 * CaseDescription: Verify the function NotifyContinuationResultInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_NotifyContinuationResultInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->NotifyContinuationResultInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: LockMissionForCleanupInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService LockMissionForCleanupInner
 * EnvConditions: NA
 * CaseDescription: Verify the function LockMissionForCleanupInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_LockMissionForCleanupInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->LockMissionForCleanupInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: UnlockMissionForCleanupInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnlockMissionForCleanupInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UnlockMissionForCleanupInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnlockMissionForCleanupInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UnlockMissionForCleanupInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterMissionListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterMissionListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterMissionListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterMissionListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterMissionListenerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: UnRegisterMissionListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnRegisterMissionListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UnRegisterMissionListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnRegisterMissionListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UnRegisterMissionListenerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionInfosInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionInfosInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetMissionInfosInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetMissionInfosInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetMissionInfosInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionInfoInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionInfoInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetMissionInfoInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetMissionInfoInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetMissionInfoInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: CleanMissionInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CleanMissionInner
 * EnvConditions: NA
 * CaseDescription: Verify the function CleanMissionInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CleanMissionInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CleanMissionInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: CleanAllMissionsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CleanAllMissionsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function CleanAllMissionsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CleanAllMissionsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CleanAllMissionsInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToFrontInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToFrontInner
 * EnvConditions: NA
 * CaseDescription: Verify the function MoveMissionToFrontInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_MoveMissionToFrontInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->MoveMissionToFrontInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionsToForegroundInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionsToForegroundInner
 * EnvConditions: NA
 * CaseDescription: Verify the function MoveMissionsToForegroundInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_MoveMissionsToForegroundInner_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "%{public}s is called.", __func__);
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->MoveMissionsToForegroundInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionsToBackgroundInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionsToBackgroundInner
 * EnvConditions: NA
 * CaseDescription: Verify the function MoveMissionsToBackgroundInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_MoveMissionsToBackgroundInner_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "%{public}s is called.", __func__);
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->MoveMissionsToBackgroundInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionIdByTokenInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionIdByTokenInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetMissionIdByTokenInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetMissionIdByTokenInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetMissionIdByTokenInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: MoveMissionToFrontByOptionsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService MoveMissionToFrontByOptionsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function MoveMissionToFrontByOptionsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_MoveMissionToFrontByOptionsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->MoveMissionToFrontByOptionsInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbilityByCallInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbilityByCallInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartAbilityByCallInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartAbilityByCallInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartAbilityByCallInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: ReleaseCallInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ReleaseCallInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ReleaseCallInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ReleaseCallInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ReleaseCallInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUserInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUserInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartUserInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUserInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartUserInner(data, reply);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StopUserInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopUserInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StopUserInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StopUserInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StopUserInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: LogoutUserInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService LogoutUserInner
 * EnvConditions: NA
 * CaseDescription: Verify the function LogoutUserInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_LogoutUserInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->LogoutUserInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfosInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAbilityRunningInfosInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetAbilityRunningInfosInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetAbilityRunningInfosInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetAbilityRunningInfosInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetExtensionRunningInfosInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetExtensionRunningInfosInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetExtensionRunningInfosInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetExtensionRunningInfosInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetExtensionRunningInfosInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetProcessRunningInfosInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetProcessRunningInfosInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetProcessRunningInfosInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetProcessRunningInfosInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetProcessRunningInfosInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartSyncRemoteMissionsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartSyncRemoteMissionsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartSyncRemoteMissionsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartSyncRemoteMissionsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartSyncRemoteMissionsInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StopSyncRemoteMissionsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopSyncRemoteMissionsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StopSyncRemoteMissionsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StopSyncRemoteMissionsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StopSyncRemoteMissionsInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterRemoteMissionListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterRemoteMissionListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterRemoteMissionListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterRemoteMissionListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterRemoteMissionListenerInner(data, reply);
    EXPECT_EQ(res, INVALID_PARAMETERS_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: UnRegisterRemoteMissionListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnRegisterRemoteMissionListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UnRegisterRemoteMissionListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnRegisterRemoteMissionListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UnRegisterRemoteMissionListenerInner(data, reply);
    EXPECT_EQ(res, INVALID_PARAMETERS_ERR);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterRemoteOnListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterRemoteOnListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterRemoteOnListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterRemoteOnListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterRemoteOnListenerInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterRemoteOffListenerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterRemoteOffListenerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterRemoteOffListenerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterRemoteOffListenerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterRemoteOffListenerInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterSnapshotHandlerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterSnapshotHandlerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterSnapshotHandlerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterSnapshotHandlerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterSnapshotHandlerInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: GetMissionSnapshotInfoInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetMissionSnapshotInfoInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetMissionSnapshotInfoInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetMissionSnapshotInfoInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetMissionSnapshotInfoInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SetAbilityControllerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetAbilityControllerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SetAbilityControllerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SetAbilityControllerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SetAbilityControllerInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: IsRunningInStabilityTestInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsRunningInStabilityTestInner
 * EnvConditions: NA
 * CaseDescription: Verify the function IsRunningInStabilityTestInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_IsRunningInStabilityTestInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->IsRunningInStabilityTestInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: StartUserTestInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartUserTestInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartUserTestInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartUserTestInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartUserTestInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: FinishUserTestInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService FinishUserTestInner
 * EnvConditions: NA
 * CaseDescription: Verify the function FinishUserTestInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_FinishUserTestInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->FinishUserTestInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetTopAbilityTokenInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetTopAbilityTokenInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetTopAbilityTokenInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetTopAbilityTokenInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetTopAbilityTokenInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerStub
 * Function: CheckUIExtensionIsFocusedInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckUIExtensionIsFocusedInner
 * EnvConditions: NA
 * CaseDescription: Verify the function CheckUIExtensionIsFocused is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CheckUIExtensionIsFocusedInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CheckUIExtensionIsFocusedInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}


/*
 * Feature: AbilityManagerService
 * Function: DelegatorDoAbilityForegroundInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityForegroundInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DelegatorDoAbilityForegroundInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DelegatorDoAbilityForegroundInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DelegatorDoAbilityForegroundInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DelegatorDoAbilityBackgroundInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DelegatorDoAbilityBackgroundInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DelegatorDoAbilityBackgroundInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DelegatorDoAbilityBackgroundInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DelegatorDoAbilityBackgroundInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityForeground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityForeground
 * EnvConditions: NA
 * CaseDescription: Verify the function DoAbilityForeground is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DoAbilityForeground_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 0;
    auto res = stub_->DoAbilityForeground(token, flag);
    EXPECT_EQ(res, 0);
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityBackground
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityBackground
 * EnvConditions: NA
 * CaseDescription: Verify the function DoAbilityBackground is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DoAbilityBackground_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    uint32_t flag = 0;
    auto res = stub_->DoAbilityBackground(token, flag);
    EXPECT_EQ(res, 0);
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityForegroundInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityForegroundInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DoAbilityForegroundInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DoAbilityForegroundInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DoAbilityForegroundInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: DoAbilityBackgroundInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DoAbilityBackgroundInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DoAbilityBackgroundInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DoAbilityBackgroundInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DoAbilityBackgroundInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SendANRProcessIDInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SendANRProcessIDInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SendANRProcessIDInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SendANRProcessIDInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SendANRProcessIDInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterObserver is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterObserver_001, TestSize.Level1)
{
    sptr<AbilityRuntime::IConnectionObserver> observer = nullptr;
    auto res = stub_->RegisterObserver(observer);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnregisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify the function UnregisterObserver is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnregisterObserver_001, TestSize.Level1)
{
    sptr<AbilityRuntime::IConnectionObserver> observer = nullptr;
    auto res = stub_->UnregisterObserver(observer);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetDlpConnectionInfos
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDlpConnectionInfos
 * EnvConditions: NA
 * CaseDescription: Verify the function GetDlpConnectionInfos is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetDlpConnectionInfos_001, TestSize.Level1)
{
    std::vector<AbilityRuntime::DlpConnectionInfo> infos;
    auto res = stub_->GetDlpConnectionInfos(infos);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: GetConnectionData
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetConnectionData
 * EnvConditions: NA
 * CaseDescription: Verify the function GetConnectionData is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetConnectionData_001, TestSize.Level1)
{
    std::vector<AbilityRuntime::ConnectionData> connectionData;
    auto res = stub_->GetConnectionData(connectionData);
    EXPECT_EQ(res, NO_ERROR);
}

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: ForceTimeoutForTestInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ForceTimeoutForTestInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ForceTimeoutForTestInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ForceTimeoutForTestInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ForceTimeoutForTestInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}
#endif

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: BlockAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function BlockAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_BlockAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->BlockAbilityInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}
#endif

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: BlockAmsServiceInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAmsServiceInner
 * EnvConditions: NA
 * CaseDescription: Verify the function BlockAmsServiceInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_BlockAmsServiceInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->BlockAmsServiceInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}
#endif

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: BlockAppServiceInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService BlockAppServiceInner
 * EnvConditions: NA
 * CaseDescription: Verify the function BlockAppServiceInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_BlockAppServiceInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->BlockAppServiceInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}
#endif

/*
 * Feature: AbilityManagerService
 * Function: FreeInstallAbilityFromRemoteInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService FreeInstallAbilityFromRemoteInner
 * EnvConditions: NA
 * CaseDescription: Verify the function FreeInstallAbilityFromRemoteInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_FreeInstallAbilityFromRemoteInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->FreeInstallAbilityFromRemoteInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: DumpAbilityInfoDoneInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService DumpAbilityInfoDoneInner
 * EnvConditions: NA
 * CaseDescription: Verify the function DumpAbilityInfoDoneInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DumpAbilityInfoDoneInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DumpAbilityInfoDoneInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.number: HandleReportDrawnCompleted_001
 * @tc.name: HandleReportDrawnCompleted
 * @tc.desc: Returns ERR_INVALID_VALUE after passing in parameters
 */
HWTEST_F(AbilityManagerStubTest, HandleReportDrawnCompleted_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->HandleReportDrawnCompleted(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: ScheduleRecoverAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ScheduleRecoverAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ScheduleRecoverAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_ScheduleRecoverAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->ScheduleRecoverAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}
#endif

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AbilityManagerService
 * Function: EnableRecoverAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService EnableRecoverAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function EnableRecoverAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_EnableRecoverAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->EnableRecoverAbilityInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}
#endif

/*
 * Feature: AbilityManagerService
 * Function: RegisterConnectionObserverInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterConnectionObserverInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterConnectionObserverInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterConnectionObserverInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterConnectionObserverInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: UnregisterConnectionObserverInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UnregisterConnectionObserverInner
 * EnvConditions: NA
 * CaseDescription: Verify the function UnregisterConnectionObserverInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnregisterConnectionObserverInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UnregisterConnectionObserverInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: GetDlpConnectionInfosInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetDlpConnectionInfosInner
 * EnvConditions: NA
 * CaseDescription: Verify the function GetDlpConnectionInfosInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetDlpConnectionInfosInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetDlpConnectionInfosInner(data, reply);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityManagerService
 * Function: SetMissionContinueStateInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionContinueStateInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SetMissionContinueStateInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SetMissionContinueStateInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SetMissionContinueStateInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: SetMissionLabelInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionLabelInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SetMissionLabelInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SetMissionLabelInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SetMissionLabelInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: SetMissionIconInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetMissionIconInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SetMissionIconInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_SetMissionIconInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->SetMissionIconInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterWindowManagerServiceHandlerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterWindowManagerServiceHandlerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterWindowManagerServiceHandlerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterWindowManagerServiceHandlerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterWindowManagerServiceHandlerInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: CompleteFirstFrameDrawingInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CompleteFirstFrameDrawingInner
 * EnvConditions: NA
 * CaseDescription: Verify the function CompleteFirstFrameDrawingInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CompleteFirstFrameDrawingInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->CompleteFirstFrameDrawingInner(data, reply);
    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: CallRequestDone
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CallRequestDone
 * EnvConditions: NA
 * CaseDescription: Verify the function CallRequestDone is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_CallRequestDone_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> callStub = nullptr;
    stub_->CallRequestDone(token, callStub);
    EXPECT_TRUE(true);
}

/*
 * Feature: AbilityManagerService
 * Function: IsValidMissionIdsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsValidMissionIdsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function IsValidMissionIdsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_IsValidMissionIdsInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto isValidMissionIdsTask = [&](const std::vector<int32_t>& ids, std::vector<MissionValidResult>& results) {
        MissionValidResult result;
        result.missionId = 1;
        result.isValid = true;
        results.push_back(result);
        return ERR_OK;
    };
    EXPECT_CALL(*stub_, IsValidMissionIds(_, _)).Times(1).WillOnce(testing::Invoke(isValidMissionIdsTask));
    EXPECT_EQ(stub_->IsValidMissionIdsInner(data, reply), NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: IsValidMissionIdsInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsValidMissionIdsInner
 * EnvConditions: NA
 * CaseDescription: Verify the function IsValidMissionIdsInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_IsValidMissionIdsInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto isValidMissionIdsTask = [&](const std::vector<int32_t>& ids, std::vector<MissionValidResult>& results) {
        GTEST_LOG_(INFO) << "AbilityManagerStub_IsValidMissionIdsInner_002 caller";
        return ERR_INVALID_VALUE;
    };
    EXPECT_CALL(*stub_, IsValidMissionIds(_, _)).Times(1).WillOnce(testing::Invoke(isValidMissionIdsTask));
    EXPECT_EQ(stub_->IsValidMissionIdsInner(data, reply), NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: PrepareTerminateAbilityBySCBInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService PrepareTerminateAbilityBySCBInner
 * EnvConditions: NA
 * CaseDescription: Verify the normal process of PrepareTerminateAbilityBySCB.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_PrepareTerminateAbilityBySCBInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto prepareTerminateAbilityBySCBTask = [&](const sptr<SessionInfo> &sessionInfo, bool &isTerminate) {
        GTEST_LOG_(INFO) << "AbilityManagerStub_IsValidMissionIdsInner_002 caller";
        return ERR_INVALID_VALUE;
    };
    EXPECT_CALL(*stub_, PrepareTerminateAbilityBySCB(_, _)).Times(1).WillOnce(
        testing::Invoke(prepareTerminateAbilityBySCBTask));
    EXPECT_EQ(stub_->PrepareTerminateAbilityBySCBInner(data, reply), ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: RegisterSessionHandlerInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService RegisterSessionHandlerInner
 * EnvConditions: NA
 * CaseDescription: Verify the function RegisterSessionHandlerInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterSessionHandlerInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterSessionHandlerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartSpecifiedAbilityBySCBInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartSpecifiedAbilityBySCBInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartSpecifiedAbilityBySCBInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartSpecifiedAbilityBySCBInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->StartSpecifiedAbilityBySCBInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityManagerService
 * Function: StartSpecifiedAbilityBySCBInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartSpecifiedAbilityBySCBInner
 * EnvConditions: NA
 * CaseDescription: Verify the function StartSpecifiedAbilityBySCBInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_StartSpecifiedAbilityBySCBInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    Want want;
    data.WriteParcelable(&want);
    EXPECT_EQ(stub_->StartSpecifiedAbilityBySCBInner(data, reply), NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: IsAbilityControllerStartInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService IsAbilityControllerStartInner
 * EnvConditions: NA
 * CaseDescription: Verify the function IsAbilityControllerStartInner is normal flow.
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_IsAbilityControllerStartInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    Want want;
    data.WriteParcelable(&want);
    EXPECT_EQ(stub_->IsAbilityControllerStartInner(data, reply), NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStub_RegisterAppDebugListenerInner_001
 * @tc.desc: Test the status of RegisterAppDebugListenerInner, check empty AppDebugListener.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterAppDebugListenerInner_001, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->RegisterAppDebugListenerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerStub_RegisterAppDebugListenerInner_002
 * @tc.desc: Test the status of RegisterAppDebugListenerInner.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_RegisterAppDebugListenerInner_002, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    auto token = new AppExecFwk::AppDebugListenerStubMock();
    EXPECT_NE(token, nullptr);
    auto ret = data.WriteRemoteObject(token);
    EXPECT_EQ(ret, true);
    int res = stub_->RegisterAppDebugListenerInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStub_UnregisterAppDebugListenerInner_001
 * @tc.desc: Test the status of UnregisterAppDebugListenerInner, check empty appDebugListener.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnregisterAppDebugListenerInner_001, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->UnregisterAppDebugListenerInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerStub_UnregisterAppDebugListenerInner_002
 * @tc.desc: Test the status of UnregisterAppDebugListenerInner.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_UnregisterAppDebugListenerInner_002, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    auto token = new AppExecFwk::AppDebugListenerStubMock();
    EXPECT_NE(token, nullptr);
    bool ret = data.WriteRemoteObject(token);
    EXPECT_EQ(ret, true);
    auto res = stub_->UnregisterAppDebugListenerInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStub_AttachAppDebugInner_001
 * @tc.desc: Test the state of AttachAppDebugInner.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_AttachAppDebugInner_001, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    std::string bundleName = "bundleName";
    data.WriteString(bundleName);
    auto res = stub_->AttachAppDebugInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStub_AttachAppDebugInner_002
 * @tc.desc: Test the state of AttachAppDebugInner, check empty bundleName;
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_AttachAppDebugInner_002, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->AttachAppDebugInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerStub_DetachAppDebugInner_001
 * @tc.desc: Test the state of DetachAppDebugInner.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DetachAppDebugInner_001, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    std::string bundleName = "bundleName";
    data.WriteString(bundleName);
    auto res = stub_->DetachAppDebugInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStub_DetachAppDebugInner_002
 * @tc.desc: Test the state of DetachAppDebugInner, check empty bundleName.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_DetachAppDebugInner_002, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->DetachAppDebugInner(data, reply);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerStub_GetForegroundUIAbilitiesInner_001
 * @tc.desc: Test function GetForegroundUIAbilitiesInner when normally.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, AbilityManagerStub_GetForegroundUIAbilitiesInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto res = stub_->GetForegroundUIAbilitiesInner(data, reply);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: AbilityManagerStubTest_RegisterAutoStartupSystemCallbackInner_0100
 * @tc.desc: Test the state of RegisterAutoStartupSystemCallbackInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, RegisterAutoStartupSystemCallbackInner_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<AppExecFwk::MockAbilityToken> token = new (std::nothrow) AppExecFwk::MockAbilityToken();
    EXPECT_NE(token, nullptr);
    data.WriteRemoteObject(token);
    auto result = stub_->RegisterAutoStartupSystemCallbackInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStubTest_RegisterAutoStartupSystemCallbackInner_0200
 * @tc.desc: Test the state of RegisterAutoStartupSystemCallbackInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, RegisterAutoStartupSystemCallbackInner_0200, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = stub_->RegisterAutoStartupSystemCallbackInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerStubTest_UnregisterAutoStartupSystemCallbackInner_0100
 * @tc.desc: Test the state of UnregisterAutoStartupSystemCallbackInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, UnregisterAutoStartupSystemCallbackInner_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<AppExecFwk::MockAbilityToken> token = new (std::nothrow) AppExecFwk::MockAbilityToken();
    EXPECT_NE(token, nullptr);
    data.WriteRemoteObject(token);
    auto result = stub_->UnregisterAutoStartupSystemCallbackInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStubTest_UnregisterAutoStartupSystemCallbackInner_0200
 * @tc.desc: Test the state of UnregisterAutoStartupSystemCallbackInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, UnregisterAutoStartupSystemCallbackInner_0200, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = stub_->UnregisterAutoStartupSystemCallbackInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}
/**
 * @tc.name: AbilityManagerStubTest_SetApplicationAutoStartupInner_0100
 * @tc.desc: Test the state of SetApplicationAutoStartupInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, SetApplicationAutoStartupInner_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    Want want;
    data.WriteParcelable(&want);
    auto result = stub_->SetApplicationAutoStartupInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStubTest_SetApplicationAutoStartupInner_0200
 * @tc.desc: Test the state of SetApplicationAutoStartupInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, SetApplicationAutoStartupInner_0200, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = stub_->SetApplicationAutoStartupInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerStubTest_CancelApplicationAutoStartupInner_0100
 * @tc.desc: Test the state of CancelApplicationAutoStartupInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, CancelApplicationAutoStartupInner_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    Want want;
    data.WriteParcelable(&want);
    auto result = stub_->CancelApplicationAutoStartupInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: AbilityManagerStubTest_CancelApplicationAutoStartupInner_0200
 * @tc.desc: Test the state of CancelApplicationAutoStartupInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, CancelApplicationAutoStartupInner_0200, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = stub_->CancelApplicationAutoStartupInner(data, reply);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityManagerStubTest_QueryAllAutoStartupApplicationsInner_0100
 * @tc.desc: Test the state of QueryAllAutoStartupApplicationsInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubTest, QueryAllAutoStartupApplicationsInner_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    Want want;
    data.WriteParcelable(&want);
    auto result = stub_->QueryAllAutoStartupApplicationsInner(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: GetUIExtensionRootHostInfo_0100
 * @tc.desc: GetUIExtensionRootHostInfo
 * @tc.type: FUNC
 * @tc.require: issueI92G6Z
 */
HWTEST_F(AbilityManagerStubTest, GetUIExtensionRootHostInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");

    MessageParcel data;
    bool writeRet = data.WriteInterfaceToken(AbilityManagerStubImplMock::GetDescriptor());
    auto token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    auto userId = USER_ID;
    writeRet &= data.WriteBool(true);
    writeRet &= data.WriteRemoteObject(token);
    writeRet &= data.WriteInt32(userId);
    EXPECT_EQ(writeRet, true);

    EXPECT_CALL(*stub_, GetUIExtensionRootHostInfo(_, _, _)).Times(1);

    MessageParcel reply;
    MessageOption option;
    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_UI_EXTENSION_ROOT_HOST_INFO), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "end");
}
} // namespace AAFwk
} // namespace OHOS
