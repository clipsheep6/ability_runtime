/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "ability_state_data.h"
#include "element_name.h"
#include "hilog_tag_wrapper.h"
#include "ipc_object_stub.h"
#include "start_options.h"
#include "status_bar_delegate_proxy.h"
#include "ui_extension_session_info.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
const size_t SIZE_ONE = 1;
const int32_t ABILITYID = 1002;
const int32_t UID = 10000;
const int REQUESTCODE = 1008;
const int ERR_BUNDLE_MANAGER_INVALID_UID = 8521233;
}  // namespace

class AbilityManagerClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void SetWant(Want& want, const std::string bundleName);
};

void AbilityManagerClientTest::SetUpTestCase(void)
{}

void AbilityManagerClientTest::TearDownTestCase(void)
{}

void AbilityManagerClientTest::SetUp()
{}

void AbilityManagerClientTest::TearDown()
{}

void AbilityManagerClientTest::SetWant(Want& want, const std::string bundleName)
{
    AppExecFwk::ElementName name;
    name.SetBundleName(bundleName);
    name.SetAbilityName("testAbility");
    want.SetElement(name);
}

/**
 * @tc.name: AbilityManagerClient_DumpSysState_0100
 * @tc.desc: DumpSysState
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_DumpSysState_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_DumpSysState_0100 start");

    std::string args = "-a";
    std::vector<std::string> state;
    bool isClient = false;
    bool isUserID = true;
    auto result = AbilityManagerClient::GetInstance()->DumpSysState(args, state, isClient, isUserID, USER_ID);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_GT(state.size(), SIZE_ONE);

    TAG_LOGI(AAFwkTag::TEST, "state.size() = %{public}zu", state.size());
    for (auto item : state) {
        TAG_LOGI(AAFwkTag::TEST, "item = %{public}s", item.c_str());
    }

    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_DumpSysState_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_ForceExitApp_0100
 * @tc.desc: ForceExitApp
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_ForceExitApp_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_ForceExitApp_0100 start");
    int32_t pid = 0;
    ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
    auto result = AbilityManagerClient::GetInstance()->ForceExitApp(pid, exitReason);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_ForceExitApp_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_RecordAppExitReason_0100
 * @tc.desc: RecordAppExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_RecordAppExitReason_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_RecordAppExitReason_0100 start");
    ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
    auto result = AbilityManagerClient::GetInstance()->RecordAppExitReason(exitReason);
    EXPECT_EQ(result, ERR_BUNDLE_MANAGER_INVALID_UID);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_RecordAppExitReason_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_RecordProcessExitReason_0100
 * @tc.desc: RecordAppExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_RecordProcessExitReason_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_RecordProcessExitReason_0100 start");
    ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
    auto result = AbilityManagerClient::GetInstance()->RecordAppExitReason(exitReason);
    EXPECT_EQ(result, ERR_BUNDLE_MANAGER_INVALID_UID);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_RecordProcessExitReason_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_RegisterStatusBarDelegate_0100
 * @tc.desc: RegisterStatusBarDelegate
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_RegisterStatusBarDelegate_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_RegisterStatusBarDelegate_001 start");
    sptr<IRemoteObject> impl(new IPCObjectStub());
    sptr<AbilityRuntime::IStatusBarDelegate> delegate(new AbilityRuntime::StatusBarDelegateProxy(impl));
    auto result = AbilityManagerClient::GetInstance()->RegisterStatusBarDelegate(delegate);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_RegisterStatusBarDelegate_001 result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_RegisterStatusBarDelegate_001 end");
}

/**
 * @tc.name: AbilityManagerClient_ScheduleClearRecoveryPageStack_0100
 * @tc.desc: ScheduleClearRecoveryPageStack
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_ScheduleClearRecoveryPageStack_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_ScheduleClearRecoveryPageStack_001 start");
    AbilityManagerClient::GetInstance()->ScheduleClearRecoveryPageStack();
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_ScheduleClearRecoveryPageStack_001 end");
}

/**
 * @tc.name: AbilityManagerClient_IsValidMissionIds_0100
 * @tc.desc: IsValidMissionIds
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_IsValidMissionIds_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_IsValidMissionIds_001 start");
    std::vector<int32_t> missionIds;
    missionIds.push_back(ABILITYID);
    std::vector<MissionValidResult> results;
    auto result = AbilityManagerClient::GetInstance()->IsValidMissionIds(missionIds, results);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_IsValidMissionIds_001 result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_IsValidMissionIds_001 end");
}

/**
 * @tc.name: AbilityManagerClient_GetForegroundUIAbilities_0100
 * @tc.desc: GetForegroundUIAbilities
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_GetForegroundUIAbilities_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_GetForegroundUIAbilities_001 start");
    std::vector<AppExecFwk::AbilityStateData> list;
    auto result = AbilityManagerClient::GetInstance()->GetForegroundUIAbilities(list);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_GetForegroundUIAbilities_001 result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_GetForegroundUIAbilities_001 end");
}

/**
 * @tc.name: AbilityManagerClient_GetUIExtensionSessionInfo_0100
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_GetUIExtensionSessionInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_GetUIExtensionSessionInfo_001 start");
    sptr<IRemoteObject> token_(new IPCObjectStub());
    UIExtensionSessionInfo uiExtensionSessionInfo;
    auto result = AbilityManagerClient::GetInstance()->GetUIExtensionSessionInfo(token_,
        uiExtensionSessionInfo, USER_ID);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_GetUIExtensionSessionInfo_001 result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_GetUIExtensionSessionInfo_001 end");
}

/**
 * @tc.name: AbilityManagerClient_StartShortCut_0100
 * @tc.desc: StartShortCut
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_StartShortCut_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_StartShortCut_001 start");
    Want want;
    StartOptions startOptions;
    SetWant(want, "bundleName");
    auto result = AbilityManagerClient::GetInstance()->StartShortcut(want, startOptions);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_StartShortCut_001 result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_StartShortCut_001 end");
}

/**
 * @tc.name: AbilityManagerClient_NotifyFrozenProcessByRSS_0100
 * @tc.desc: NotifyFrozenProcessByRSS
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_NotifyFrozenProcessByRSS_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_NotifyFrozenProcessByRSS_001 start");
    std::vector<int32_t> pidList;
    pidList.push_back(19082);
    AbilityManagerClient::GetInstance()->NotifyFrozenProcessByRSS(pidList, UID);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_NotifyFrozenProcessByRSS_001 end");
}

/**
 * @tc.name: AbilityManagerClient_PreStartMission_0100
 * @tc.desc: PreStartMission
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_PreStartMission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_PreStartMission_001 start");
    auto result = AbilityManagerClient::GetInstance()->PreStartMission("com.ix.hiservcie", "entry",
        "ServiceAbility", "2024-07-19 10:00:00");
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_PreStartMission_001 result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_PreStartMission_001 end");
}

/**
 * @tc.name: AbilityManagerClient_OpenLink_0100
 * @tc.desc: OpenLink
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_OpenLink, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_OpenLink start");
    Want want;
    sptr<IRemoteObject> token_(new IPCObjectStub());
    SetWant(want, "bundleName");
    auto result = AbilityManagerClient::GetInstance()->OpenLink(want, token_,
        USER_ID, REQUESTCODE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_OpenLink result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerClient_OpenLink end");
}
}  // namespace AAFwk
}  // namespace OHOS
