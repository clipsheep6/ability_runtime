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
}  // namespace AAFwk
}  // namespace OHOS
