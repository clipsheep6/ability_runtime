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

#include "ability_manager_errors.h"
#define private public
#define protected public
#include "ability_record.h"
#include "ability_start_setting.h"
#include "scene_board/ui_ability_lifecycle_manager.h"
#undef protected
#undef private
#include "mock_ability_info_callback_stub.h"
#include "session/host/include/session.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class UIAbilityLifecycleManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<AbilityRecord> InitAbilityRecord();
};

void UIAbilityLifecycleManagerTest::SetUpTestCase() {}

void UIAbilityLifecycleManagerTest::TearDownTestCase() {}

void UIAbilityLifecycleManagerTest::SetUp() {}

void UIAbilityLifecycleManagerTest::TearDown() {}

class UIAbilityLifcecycleManagerTestStub : public IRemoteStub<IAbilityConnection> {
public:
    UIAbilityLifcecycleManagerTestStub() {};
    virtual ~UIAbilityLifcecycleManagerTestStub() {};

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return 0;
    };

    virtual void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) {};

    /**
     * OnAbilityDisconnectDone, AbilityMs notify caller ability the result of disconnect.
     *
     * @param element, service ability's ElementName.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    virtual void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) {};
};

std::shared_ptr<AbilityRecord> UIAbilityLifecycleManagerTest::InitAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    return abilityRecord;
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartUIAbility_0100
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, nullptr), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartUIAbility_0200
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartUIAbility_0300
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartUIAbility_0400
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_004, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->startSetting = std::make_shared<AbilityStartSetting>();
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartUIAbility_0500
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_005, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_StartUIAbility_0600
 * @tc.desc: StartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, StartUIAbility_006, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfoCallback = new MockAbilityInfoCallbackStub();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo), ERR_OK);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CreateSessionInfo_0100
 * @tc.desc: CreateSessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CreateSessionInfo_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.startSetting = std::make_shared<AbilityStartSetting>();
    EXPECT_NE(mgr->CreateSessionInfo(abilityRequest), nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AbilityTransactionDone_0100
 * @tc.desc: AbilityTransactionDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AbilityTransactionDone_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto token = abilityRecord->GetToken()->AsObject();
    int state = 6;
    PacMap saveData;
    EXPECT_EQ(mgr->AbilityTransactionDone(token, state, saveData), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AbilityTransactionDone_0200
 * @tc.desc: AbilityTransactionDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AbilityTransactionDone_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->terminateAbilityList_.emplace_back(abilityRecord);
    auto token = abilityRecord->GetToken()->AsObject();
    int state = 6;
    PacMap saveData;
    EXPECT_EQ(mgr->AbilityTransactionDone(token, state, saveData), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AttachAbilityThread_0100
 * @tc.desc: AttachAbilityThread
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AttachAbilityThread_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    sptr<IAbilityScheduler> scheduler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    EXPECT_EQ(mgr->AttachAbilityThread(scheduler, token), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AttachAbilityThread_0200
 * @tc.desc: AttachAbilityThread
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AttachAbilityThread_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(1, abilityRecord);
    sptr<IAbilityScheduler> scheduler = nullptr;
    auto&& token = abilityRecord->GetToken()->AsObject();
    EXPECT_EQ(mgr->AttachAbilityThread(scheduler, token), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AttachAbilityThread_0300
 * @tc.desc: AttachAbilityThread
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AttachAbilityThread_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetStartedByCall(true);

    mgr->sessionAbilityMap_.emplace(1, abilityRecord);
    sptr<IAbilityScheduler> scheduler = nullptr;
    auto&& token = abilityRecord->GetToken()->AsObject();
    EXPECT_EQ(mgr->AttachAbilityThread(scheduler, token), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_AttachAbilityThread_0400
 * @tc.desc: AttachAbilityThread
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, AttachAbilityThread_004, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    Want want;
    want.SetParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, true);
    abilityRequest.want = want;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetStartedByCall(true);

    mgr->sessionAbilityMap_.emplace(1, abilityRecord);
    sptr<IAbilityScheduler> scheduler = nullptr;
    auto&& token = abilityRecord->GetToken()->AsObject();
    EXPECT_EQ(mgr->AttachAbilityThread(scheduler, token), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAbilityRequestDone_0100
 * @tc.desc: OnAbilityRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAbilityRequestDone_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    mgr->OnAbilityRequestDone(token, 1);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAbilityRequestDone_0200
 * @tc.desc: OnAbilityRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAbilityRequestDone_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto&& token = abilityRecord->GetToken()->AsObject();
    mgr->OnAbilityRequestDone(token, 1);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordByToken_0100
 * @tc.desc: GetAbilityRecordByToken
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordByToken_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    sptr<IRemoteObject> token = nullptr;
    EXPECT_EQ(mgr->GetAbilityRecordByToken(token), nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordByToken_0200
 * @tc.desc: GetAbilityRecordByToken
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordByToken_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto&& token = abilityRecord->GetToken()->AsObject();
    EXPECT_EQ(mgr->GetAbilityRecordByToken(token), nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetAbilityRecordByToken_0300
 * @tc.desc: GetAbilityRecordByToken
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetAbilityRecordByToken_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(1, abilityRecord);
    auto&& token = abilityRecord->GetToken()->AsObject();
    EXPECT_NE(mgr->GetAbilityRecordByToken(token), nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateAbilityRecordLaunchReason_0100
 * @tc.desc: UpdateAbilityRecordLaunchReason
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateAbilityRecordLaunchReason_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    mgr->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateAbilityRecordLaunchReason_0200
 * @tc.desc: UpdateAbilityRecordLaunchReason
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateAbilityRecordLaunchReason_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    Want want;
    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateAbilityRecordLaunchReason_0300
 * @tc.desc: UpdateAbilityRecordLaunchReason
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateAbilityRecordLaunchReason_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    Want want;
    want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_UpdateAbilityRecordLaunchReason_0400
 * @tc.desc: UpdateAbilityRecordLaunchReason
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, UpdateAbilityRecordLaunchReason_004, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    Want want;
    want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_EraseAbilityRecord_0100
 * @tc.desc: EraseAbilityRecord
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, EraseAbilityRecord_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    mgr->EraseAbilityRecord(abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_EraseAbilityRecord_0200
 * @tc.desc: EraseAbilityRecord
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, EraseAbilityRecord_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    mgr->sessionAbilityMap_.emplace(1, abilityRecord);
    mgr->EraseAbilityRecord(abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchState_0100
 * @tc.desc: DispatchState
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchState_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    EXPECT_EQ(mgr->DispatchState(abilityRecord, AbilityState::INITIAL), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchState_0200
 * @tc.desc: DispatchState
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchState_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    EXPECT_EQ(mgr->DispatchState(abilityRecord, AbilityState::FOREGROUND), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchState_0300
 * @tc.desc: DispatchState
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchState_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    EXPECT_EQ(mgr->DispatchState(abilityRecord, AbilityState::FOREGROUND_FAILED), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchState_0400
 * @tc.desc: DispatchState
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchState_004, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    int state = 130;
    EXPECT_EQ(mgr->DispatchState(abilityRecord, state), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_DispatchForeground_0100
 * @tc.desc: DispatchForeground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DispatchForeground_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_EQ(mgr->DispatchForeground(abilityRecord, true, AbilityState::FOREGROUND), ERR_INVALID_VALUE);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteForegroundSuccess_0100
 * @tc.desc: CompleteForegroundSuccess
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteForegroundSuccess_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    mgr->CompleteForegroundSuccess(abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteForegroundSuccess_0200
 * @tc.desc: CompleteForegroundSuccess
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteForegroundSuccess_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    mgr->CompleteForegroundSuccess(abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteForegroundSuccess_0300
 * @tc.desc: CompleteForegroundSuccess
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteForegroundSuccess_003, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetStartedByCall(true);
    abilityRecord->SetStartToForeground(true);
    abilityRecord->isReady_ = true;
    mgr->CompleteForegroundSuccess(abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundFailed_0100
 * @tc.desc: HandleForegroundOrFailed
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundFailed_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    mgr->HandleForegroundFailed(abilityRecord, AbilityState::FOREGROUND_FAILED);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundFailed_0200
 * @tc.desc: HandleForegroundFailed
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundFailed_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    mgr->HandleForegroundFailed(abilityRecord, AbilityState::FOREGROUND_FAILED);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_MinimizeUIAbility_0100
 * @tc.desc: MinimizeUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MinimizeUIAbility_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_EQ(uiAbilityLifecycleManager->MinimizeUIAbility(nullptr), ERR_INVALID_VALUE);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_MinimizeUIAbility_0200
 * @tc.desc: MinimizeUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MinimizeUIAbility_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    EXPECT_EQ(uiAbilityLifecycleManager->MinimizeUIAbility(abilityRecord), ERR_OK);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_MinimizeUIAbility_0300
 * @tc.desc: MinimizeUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MinimizeUIAbility_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    EXPECT_EQ(uiAbilityLifecycleManager->MinimizeUIAbility(abilityRecord), ERR_OK);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveToBackground_0100
 * @tc.desc: MoveToBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MoveToBackground_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    uiAbilityLifecycleManager->MoveToBackground(nullptr);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_MoveToBackground_0200
 * @tc.desc: MoveToBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, MoveToBackground_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->MoveToBackground(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0100
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    uint32_t msgId = 0;
    uiAbilityLifecycleManager->PrintTimeOutLog(nullptr, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0200
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 0;
    uiAbilityLifecycleManager->PrintTimeOutLog(abilityRecord, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0300
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 1;
    uiAbilityLifecycleManager->PrintTimeOutLog(abilityRecord, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0400
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 2;
    uiAbilityLifecycleManager->PrintTimeOutLog(abilityRecord, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0500
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 3;
    uiAbilityLifecycleManager->PrintTimeOutLog(abilityRecord, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0600
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 4;
    uiAbilityLifecycleManager->PrintTimeOutLog(abilityRecord, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0700
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_007, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 5;
    uiAbilityLifecycleManager->PrintTimeOutLog(abilityRecord, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_PrintTimeOutLog_0800
 * @tc.desc: PrintTimeOutLog
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, PrintTimeOutLog_008, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 6;
    uiAbilityLifecycleManager->PrintTimeOutLog(abilityRecord, msgId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteBackground_0100
 * @tc.desc: CompleteBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteBackground_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    uiAbilityLifecycleManager->CompleteBackground(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteBackground_0200
 * @tc.desc: CompleteBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteBackground_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    uiAbilityLifecycleManager->CompleteBackground(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteBackground_0300
 * @tc.desc: CompleteBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteBackground_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    uiAbilityLifecycleManager->CompleteBackground(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteBackground_0400
 * @tc.desc: CompleteBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteBackground_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    uiAbilityLifecycleManager->CompleteBackground(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteBackground_0500
 * @tc.desc: CompleteBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteBackground_005, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetStartedByCall(true);
    abilityRecord->SetStartToBackground(true);
    abilityRecord->isReady_ = true;
    mgr->CompleteBackground(abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteBackground_0600
 * @tc.desc: CompleteBackground
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteBackground_006, TestSize.Level1)
{
    auto mgr = std::make_shared<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    mgr->terminateAbilityList_.push_back(abilityRecord);
    mgr->CompleteBackground(abilityRecord);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CloseUIAbility_0100
 * @tc.desc: CloseUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetTerminatingState();
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, nullptr), ERR_OK);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CloseUIAbility_0200
 * @tc.desc: CloseUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, nullptr), ERR_INVALID_VALUE);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CloseUIAbility_0300
 * @tc.desc: CloseUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    Want want;
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, &want), ERR_INVALID_VALUE);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CloseUIAbility_0400
 * @tc.desc: CloseUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    auto abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    Want want;
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, &want), ERR_OK);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CloseUIAbility_0500
 * @tc.desc: CloseUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    auto abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    Want want;
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, &want), ERR_OK);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CloseUIAbility_0600
 * @tc.desc: CloseUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CloseUIAbility_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    auto abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    Want want;
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord, -1, &want), ERR_OK);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_DelayCompleteTerminate_0100
 * @tc.desc: DelayCompleteTerminate
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, DelayCompleteTerminate_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->DelayCompleteTerminate(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteTerminate_0100
 * @tc.desc: CompleteTerminate
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteTerminate_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUND;
    uiAbilityLifecycleManager->CompleteTerminate(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_CompleteTerminate_0200
 * @tc.desc: CompleteTerminate
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CompleteTerminate_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::TERMINATING;
    uiAbilityLifecycleManager->CompleteTerminate(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnTimeOut_0100
 * @tc.desc: OnTimeOut
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnTimeOut_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 0;
    int64_t abilityRecordId = 0;
    uiAbilityLifecycleManager->OnTimeOut(msgId, abilityRecordId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnTimeOut_0200
 * @tc.desc: OnTimeOut
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnTimeOut_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 0;
    int64_t abilityRecordId = 0;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    uiAbilityLifecycleManager->OnTimeOut(msgId, abilityRecordId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnTimeOut_0300
 * @tc.desc: OnTimeOut
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnTimeOut_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 5;
    int64_t abilityRecordId = 0;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(0, abilityRecord);
    uiAbilityLifecycleManager->OnTimeOut(msgId, abilityRecordId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnTimeOut_0400
 * @tc.desc: OnTimeOut
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnTimeOut_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 6;
    int64_t abilityRecordId = 0;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(msgId, abilityRecord);
    uiAbilityLifecycleManager->OnTimeOut(msgId, abilityRecordId);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToHandleException_0100
 * @tc.desc: NotifySCBToHandleException
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToHandleException_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    uiAbilityLifecycleManager->NotifySCBToHandleException(nullptr,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleLoadTimeout");
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToHandleException_0200
 * @tc.desc: NotifySCBToHandleException
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToHandleException_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleLoadTimeout");
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToHandleException_0300
 * @tc.desc: NotifySCBToHandleException
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToHandleException_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleLoadTimeout");
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToHandleException_0400
 * @tc.desc: NotifySCBToHandleException
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToHandleException_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleLoadTimeout");
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToHandleException_0500
 * @tc.desc: NotifySCBToHandleException
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToHandleException_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    sessionInfo->persistentId = 0;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    uiAbilityLifecycleManager->NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleLoadTimeout");
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToHandleException_0600
 * @tc.desc: NotifySCBToHandleException
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToHandleException_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    sessionInfo->persistentId = 0;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    uiAbilityLifecycleManager->NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_FOREGROUND_TIMEOUT), "handleForegroundTimeout");
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToHandleException_0700
 * @tc.desc: NotifySCBToHandleException
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToHandleException_007, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    sessionInfo->persistentId = 0;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    uiAbilityLifecycleManager->NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_DIED), "onAbilityDied");
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleLoadTimeout_0100
 * @tc.desc: HandleLoadTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleLoadTimeout_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    uiAbilityLifecycleManager->HandleLoadTimeout(nullptr);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleLoadTimeout_0200
 * @tc.desc: HandleLoadTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleLoadTimeout_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->HandleLoadTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleLoadTimeout_0300
 * @tc.desc: HandleLoadTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleLoadTimeout_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->HandleLoadTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleLoadTimeout_0400
 * @tc.desc: HandleLoadTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleLoadTimeout_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->HandleLoadTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleLoadTimeout_0500
 * @tc.desc: HandleLoadTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleLoadTimeout_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    sessionInfo->persistentId = 0;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    uiAbilityLifecycleManager->HandleLoadTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundTimeout_0100
 * @tc.desc: HandleForegroundTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundTimeout_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    uiAbilityLifecycleManager->HandleForegroundTimeout(nullptr);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundTimeout_0200
 * @tc.desc: HandleForegroundTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundTimeout_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::TERMINATING;
    uiAbilityLifecycleManager->HandleForegroundTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundTimeout_0300
 * @tc.desc: HandleForegroundTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundTimeout_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    uiAbilityLifecycleManager->HandleForegroundTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundTimeout_0400
 * @tc.desc: HandleForegroundTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundTimeout_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    uiAbilityLifecycleManager->HandleForegroundTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundTimeout_0500
 * @tc.desc: HandleForegroundTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundTimeout_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    uiAbilityLifecycleManager->HandleForegroundTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundTimeout_0600
 * @tc.desc: HandleForegroundTimeout
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundTimeout_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    sessionInfo->persistentId = 0;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    uiAbilityLifecycleManager->HandleForegroundTimeout(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAbilityDied_0100
 * @tc.desc: OnAbilityDied
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAbilityDied_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    uiAbilityLifecycleManager->OnAbilityDied(nullptr);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAbilityDied_0200
 * @tc.desc: OnAbilityDied
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAbilityDied_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->OnAbilityDied(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAbilityDied_0300
 * @tc.desc: OnAbilityDied
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAbilityDied_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->OnAbilityDied(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAbilityDied_0400
 * @tc.desc: OnAbilityDied
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAbilityDied_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->OnAbilityDied(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_OnAbilityDied_0500
 * @tc.desc: OnAbilityDied
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, OnAbilityDied_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>();
    ASSERT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    abilityRequest.sessionInfo = sessionInfo;
    sessionInfo->persistentId = 0;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    uiAbilityLifecycleManager->OnAbilityDied(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

/**
 * @tc.name: UIAbilityLifecycleManager_SetRootSceneSession_0100
 * @tc.desc: SetRootSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, SetRootSceneSession_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<IRemoteObject> object = nullptr;
    uiAbilityLifecycleManager->SetRootSceneSession(object);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_SetRootSceneSession_0200
 * @tc.desc: SetRootSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, SetRootSceneSession_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    auto abilityRecord = InitAbilityRecord();
    EXPECT_NE(abilityRecord, nullptr);
    auto token = abilityRecord->GetToken();
    EXPECT_NE(token, nullptr);
    auto object = token->AsObject();
    uiAbilityLifecycleManager->SetRootSceneSession(object);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_SetRootSceneSession_0300
 * @tc.desc: SetRootSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, SetRootSceneSession_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    sptr<Rosen::ISession> session = new Rosen::Session(info);
    EXPECT_NE(session, nullptr);
    sptr<IRemoteObject> rootSceneSession = session->AsObject();
    uiAbilityLifecycleManager->SetRootSceneSession(rootSceneSession);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBToStartUIAbility_0100
 * @tc.desc: NotifySCBToStartUIAbility
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBToStartUIAbility_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    int32_t userId = 100;
    uiAbilityLifecycleManager->NotifySCBToStartUIAbility(abilityRequest, userId);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetPersistentIdByAbilityRequest_0100
 * @tc.desc: GetPersistentIdByAbilityRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetPersistentIdByAbilityRequest_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    bool reuse = false;
    int32_t userId = 100;
    EXPECT_EQ(uiAbilityLifecycleManager->GetPersistentIdByAbilityRequest(abilityRequest, reuse, userId), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetPersistentIdByAbilityRequest_0200
 * @tc.desc: GetPersistentIdByAbilityRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetPersistentIdByAbilityRequest_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    bool reuse = false;
    int32_t userId = 100;
    EXPECT_EQ(uiAbilityLifecycleManager->GetPersistentIdByAbilityRequest(abilityRequest, reuse, userId), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetPersistentIdByAbilityRequest_0300
 * @tc.desc: GetPersistentIdByAbilityRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetPersistentIdByAbilityRequest_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    abilityRequest.abilityInfo.name = "testAbility";
    abilityRequest.abilityInfo.moduleName = "testModule";
    abilityRequest.abilityInfo.bundleName = "com.test.ut";

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    int32_t userId = 100;
    abilityRecord->SetOwnerMissionUserId(userId);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool reuse = false;
    EXPECT_EQ(uiAbilityLifecycleManager->GetPersistentIdByAbilityRequest(abilityRequest, reuse, userId),
        sessionInfo->persistentId);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetPersistentIdByAbilityRequest_0400
 * @tc.desc: GetPersistentIdByAbilityRequest
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetPersistentIdByAbilityRequest_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest1;
    abilityRequest1.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.abilityInfo.name = "testAbility";
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool reuse = false;
    int32_t userId = 100;
    EXPECT_EQ(uiAbilityLifecycleManager->GetPersistentIdByAbilityRequest(abilityRequest1, reuse, userId), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetReusedSpecifiedPersistentId_0100
 * @tc.desc: GetReusedSpecifiedPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetReusedSpecifiedPersistentId_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    bool reuse = false;
    int32_t userId = 100;
    EXPECT_EQ(uiAbilityLifecycleManager->GetReusedSpecifiedPersistentId(abilityRequest, reuse, userId), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetReusedSpecifiedPersistentId_0200
 * @tc.desc: GetReusedSpecifiedPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetReusedSpecifiedPersistentId_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.abilityInfo.name = "testAbility";
    abilityRequest.abilityInfo.moduleName = "testModule";
    abilityRequest.abilityInfo.bundleName = "com.test.ut";
    abilityRequest.startRecent = true;
    std::string flag = "specified";
    abilityRequest.specifiedFlag = flag;

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetSpecifiedFlag(flag);
    int32_t userId = 100;
    abilityRecord->SetOwnerMissionUserId(userId);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool reuse = false;
    EXPECT_EQ(uiAbilityLifecycleManager->GetReusedSpecifiedPersistentId(abilityRequest, reuse, userId),
        sessionInfo->persistentId);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetReusedSpecifiedPersistentId_0300
 * @tc.desc: GetReusedSpecifiedPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetReusedSpecifiedPersistentId_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.startRecent = true;
    std::string flag = "specified";

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetSpecifiedFlag(flag);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool reuse = false;
    int32_t userId = 100;
    EXPECT_EQ(uiAbilityLifecycleManager->GetReusedSpecifiedPersistentId(abilityRequest, reuse, userId), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetReusedStandardPersistentId_0100
 * @tc.desc: GetReusedStandardPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetReusedStandardPersistentId_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    bool reuse = false;
    int32_t userId = 100;
    EXPECT_EQ(uiAbilityLifecycleManager->GetReusedStandardPersistentId(abilityRequest, reuse, userId), 0);
}

/**
 * @tc.name: UIAbilityLifecycleManager_GetReusedStandardPersistentId_0200
 * @tc.desc: GetReusedStandardPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, GetReusedStandardPersistentId_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    abilityRequest.abilityInfo.name = "testAbility";
    abilityRequest.abilityInfo.moduleName = "testModule";
    abilityRequest.abilityInfo.bundleName = "com.test.ut";
    abilityRequest.startRecent = true;

    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    bool reuse = false;
    int32_t userId = 100;
    abilityRecord->SetOwnerMissionUserId(userId);
    EXPECT_EQ(uiAbilityLifecycleManager->GetReusedStandardPersistentId(abilityRequest, reuse, userId),
        sessionInfo->persistentId);
}

/**
 * @tc.name: UIAbilityLifecycleManager_NotifySCBPendingActivation_0100
 * @tc.desc: NotifySCBPendingActivation
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, NotifySCBPendingActivation_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    auto token = abilityRecord->GetToken();
    EXPECT_NE(token, nullptr);
    abilityRequest.callerToken = token->AsObject();
    uiAbilityLifecycleManager->NotifySCBPendingActivation(sessionInfo, abilityRequest);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ResolveLocked_0100
 * @tc.desc: ResolveLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ResolveLocked_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    int32_t userId = 100;
    EXPECT_EQ(uiAbilityLifecycleManager->ResolveLocked(abilityRequest, userId), RESOLVE_CALL_ABILITY_INNER_ERR);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ResolveLocked_0200
 * @tc.desc: ResolveLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ResolveLocked_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    int32_t userId = 100;
    EXPECT_EQ(uiAbilityLifecycleManager->ResolveLocked(abilityRequest, userId), RESOLVE_CALL_ABILITY_INNER_ERR);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallAbilityLocked_0100
 * @tc.desc: CallAbilityLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallAbilityLocked_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);

    AbilityRequest abilityRequest;
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->persistentId = 1;
    Want want;
    want.SetParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, true);
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.want = want;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    abilityRecord->isReady_ = true;

    int32_t userId = 100;
    uiAbilityLifecycleManager->CallAbilityLocked(abilityRequest, userId);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallAbilityLocked_0200
 * @tc.desc: CallAbilityLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallAbilityLocked_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    Want want;
    want.SetParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, true);
    abilityRequest.want = want;
    int32_t userId = 100;
    uiAbilityLifecycleManager->CallAbilityLocked(abilityRequest, userId);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallUIAbilityBySCB_0100
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> sessionInfo;
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallUIAbilityBySCB_0200
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = nullptr;
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallUIAbilityBySCB_0300
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto token = abilityRecord->GetToken();
    EXPECT_NE(token, nullptr);
    sessionInfo->sessionToken = token->AsObject();
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallUIAbilityBySCB_0400
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_004, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);

    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallUIAbilityBySCB_0500
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_005, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->uiAbilityId = 1;

    uiAbilityLifecycleManager->tmpAbilityMap_.emplace(1, nullptr);
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallUIAbilityBySCB_0600
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_006, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->uiAbilityId = 1;

    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->tmpAbilityMap_.emplace(1, abilityRecord);

    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallUIAbilityBySCB_0700
 * @tc.desc: CallUIAbilityBySCB
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallUIAbilityBySCB_007, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->uiAbilityId = 1;
    sessionInfo->persistentId = 1;

    AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);

    uiAbilityLifecycleManager->tmpAbilityMap_.emplace(1, abilityRecord);
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    uiAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallRequestDone_0100
 * @tc.desc: CallRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallRequestDone_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    uiAbilityLifecycleManager->CallRequestDone(nullptr, nullptr);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallRequestDone_0200
 * @tc.desc: CallRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallRequestDone_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    uiAbilityLifecycleManager->CallRequestDone(abilityRecord, nullptr);
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_CallRequestDone_0300
 * @tc.desc: CallRequestDone
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, CallRequestDone_003, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto token = abilityRecord->GetToken();
    EXPECT_NE(token, nullptr);
    uiAbilityLifecycleManager->CallRequestDone(abilityRecord, token->AsObject());
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ReleaseCallLocked_0100
 * @tc.desc: ReleaseCallLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ReleaseCallLocked_001, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    sptr<IAbilityConnection> connect = new UIAbilityLifcecycleManagerTestStub();
    AppExecFwk::ElementName element;
    auto ret = uiAbilityLifecycleManager->ReleaseCallLocked(connect, element);
    EXPECT_EQ(ret, RELEASE_CALL_ABILITY_INNER_ERR);
}

/**
 * @tc.name: UIAbilityLifecycleManager_ReleaseCallLocked_0200
 * @tc.desc: ReleaseCallLocked
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, ReleaseCallLocked_002, TestSize.Level1)
{
    auto uiAbilityLifecycleManager = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(uiAbilityLifecycleManager, nullptr);
    auto abilityRecord = InitAbilityRecord();
    uiAbilityLifecycleManager->sessionAbilityMap_.emplace(1, abilityRecord);
    sptr<IAbilityConnection> connect = new UIAbilityLifcecycleManagerTestStub();
    AppExecFwk::ElementName element("", "com.example.unittest", "MainAbility");
    auto ret = uiAbilityLifecycleManager->ReleaseCallLocked(connect, element);
    EXPECT_EQ(ret, RELEASE_CALL_ABILITY_INNER_ERR);
}
}  // namespace AAFwk
}  // namespace OHOS
