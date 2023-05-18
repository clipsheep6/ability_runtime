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

#define private public
#include "scene_board/ui_ability_lifecycle_manager.h"
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest, sessionInfo);
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
    sessionInfo->persistentId = 1;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest, sessionInfo);
    abilityRecord->SetTerminatingState();
    mgr->sessionAbilityMap_.emplace(sessionInfo->persistentId, abilityRecord);
    EXPECT_EQ(mgr->StartUIAbility(abilityRequest, sessionInfo), ERR_INVALID_VALUE);
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
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest, sessionInfo);
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
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundTimeoutOrFailed_0100
 * @tc.desc: HandleForegroundTimeoutOrFailed
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundTimeoutOrFailed_001, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    mgr->HandleForegroundTimeoutOrFailed(abilityRecord, AbilityState::FOREGROUND_FAILED);
    EXPECT_NE(mgr, nullptr);
}

/**
 * @tc.name: UIAbilityLifecycleManager_HandleForegroundTimeoutOrFailed_0200
 * @tc.desc: HandleForegroundTimeoutOrFailed
 * @tc.type: FUNC
 */
HWTEST_F(UIAbilityLifecycleManagerTest, HandleForegroundTimeoutOrFailed_002, TestSize.Level1)
{
    auto mgr = std::make_unique<UIAbilityLifecycleManager>();
    EXPECT_NE(mgr, nullptr);
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);
    mgr->HandleForegroundTimeoutOrFailed(abilityRecord, AbilityState::FOREGROUND_FAILED);
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
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
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
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
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
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
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
    abilityRecord->SetAbilityState(AbilityState::BACKGROUNDING);
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
    abilityRecord->SetAbilityState(AbilityState::BACKGROUNDING);
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
    abilityRecord->SetAbilityState(AbilityState::BACKGROUNDING);
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    uiAbilityLifecycleManager->CompleteBackground(abilityRecord);
    uiAbilityLifecycleManager.reset();
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
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord), ERR_OK);
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
    EXPECT_EQ(uiAbilityLifecycleManager->CloseUIAbility(abilityRecord), ERR_OK);
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
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
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
    abilityRecord->SetAbilityState(AbilityState::TERMINATING);
    uiAbilityLifecycleManager->CompleteTerminate(abilityRecord);
    uiAbilityLifecycleManager.reset();
}

}  // namespace AAFwk
}  // namespace OHOS
