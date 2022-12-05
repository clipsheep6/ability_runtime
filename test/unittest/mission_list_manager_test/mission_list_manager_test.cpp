/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "ability_info.h"
#include "ability_manager_service.h"
#include "mission.h"
#include "mission_list_manager.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string DLP_INDEX = "ohos.dlp.params.index";
}
class MissionListManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void InitMockMission(std::shared_ptr<MissionListManager>& missionListManager,
        AbilityRequest& abilityRequest, Want& want, std::shared_ptr<AbilityRecord>& ability);
    std::shared_ptr<AbilityRecord> InitAbilityRecord();
};

void MissionListManagerTest::SetUpTestCase(void)
{}
void MissionListManagerTest::TearDownTestCase(void)
{}
void MissionListManagerTest::SetUp(void)
{}
void MissionListManagerTest::TearDown(void)
{}

void MissionListManagerTest::InitMockMission(std::shared_ptr<MissionListManager>& missionListManager,
    AbilityRequest& abilityRequest, Want& want, std::shared_ptr<AbilityRecord>& ability)
{
    missionListManager->Init();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    AppExecFwk::ApplicationInfo applicationInfo;
    ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;
    ability->SetSpecifiedFlag("flag");
    ability->SetIsNewWant(false);

    abilityRequest.callerToken = ability->GetToken();
    missionListManager->EnqueueWaitingAbility(abilityRequest);
    missionListManager->defaultStandardList_->AddMissionToTop(mission);
}

std::shared_ptr<AbilityRecord> MissionListManagerTest::InitAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    return abilityRecord;
}

bool g_notifyWindowTransitionCalled = false;
bool g_cancelStartingWindowCalled = false;

class MockWMSHandler : public IWindowManagerServiceHandler {
public:
    virtual void NotifyWindowTransition(sptr<AbilityTransitionInfo> fromInfo, sptr<AbilityTransitionInfo> toInfo)
    {
        g_notifyWindowTransitionCalled = true;
    }

    virtual int32_t GetFocusWindow(sptr<IRemoteObject>& abilityToken)
    {
        return 0;
    }

    virtual void StartingWindow(sptr<AbilityTransitionInfo> info,
        std::shared_ptr<Media::PixelMap> pixelMap, uint32_t bgColor) {}

    virtual void StartingWindow(sptr<AbilityTransitionInfo> info, std::shared_ptr<Media::PixelMap> pixelMap) {}

    virtual void CancelStartingWindow(sptr<IRemoteObject> abilityToken)
    {
        g_cancelStartingWindowCalled = true;
    }

    virtual sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }
};

/*
 * Feature: MissionListManager
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify StartAbility
 */
HWTEST_F(MissionListManagerTest, StartAbility_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);

    std::shared_ptr<AbilityRecord> currentTopAbility;
    std::shared_ptr<AbilityRecord> callerAbility;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;

    auto result = missionListManager->StartAbility(currentTopAbility, callerAbility, abilityRequest);

    EXPECT_EQ(0, result);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionBySpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionBySpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionBySpecifiedFlag
 */
HWTEST_F(MissionListManagerTest, GetMissionBySpecifiedFlag_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;
    ability->SetSpecifiedFlag("flag");
    missionListManager->launcherList_->AddMissionToTop(mission);

    auto mission1 = missionListManager->GetMissionBySpecifiedFlag(want, "flag");
    EXPECT_EQ(mission, mission1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionBySpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionBySpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionBySpecifiedFlag
 */
HWTEST_F(MissionListManagerTest, GetMissionBySpecifiedFlag_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;

    ability->SetSpecifiedFlag("flag");
    missionListManager->defaultSingleList_->AddMissionToTop(mission);
    auto mission1 = missionListManager->GetMissionBySpecifiedFlag(want, "flag");
    EXPECT_EQ(mission, mission1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionBySpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionBySpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionBySpecifiedFlag
 */
HWTEST_F(MissionListManagerTest, GetMissionBySpecifiedFlag_003, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;
    ability->SetSpecifiedFlag("flag");
    missionListManager->launcherList_->AddMissionToTop(mission);

    auto mission1 = missionListManager->GetMissionBySpecifiedFlag(want, "flag");
    EXPECT_EQ(mission, mission1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionBySpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionBySpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionBySpecifiedFlag
 */
HWTEST_F(MissionListManagerTest, GetMissionBySpecifiedFlag_004, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;
    ability->SetSpecifiedFlag("flag");
    missionListManager->defaultStandardList_->AddMissionToTop(mission);

    auto mission1 = missionListManager->GetMissionBySpecifiedFlag(want, "flag");
    EXPECT_EQ(mission, mission1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnAcceptWantResponse
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnAcceptWantResponse
 * EnvConditions: NA
 * CaseDescription: Verify OnAcceptWantResponse
 */
HWTEST_F(MissionListManagerTest, OnAcceptWantResponse_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;
    ability->SetSpecifiedFlag("flag");
    ability->SetIsNewWant(false);

    AbilityRequest abilityRequest;
    abilityRequest.callerToken = ability->GetToken();
    missionListManager->EnqueueWaitingAbility(abilityRequest);
    missionListManager->defaultStandardList_->AddMissionToTop(mission);

    missionListManager->OnAcceptWantResponse(want, "flag");
    EXPECT_EQ(ability->IsNewWant(), true);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnAcceptWantResponse
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnAcceptWantResponse
 * EnvConditions: NA
 * CaseDescription: Verify OnAcceptWantResponse
 */
HWTEST_F(MissionListManagerTest, OnAcceptWantResponse_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;
    ability->SetIsNewWant(false);

    missionListManager->OnAcceptWantResponse(want, "flag");
    EXPECT_EQ(ability->IsNewWant(), false);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnAcceptWantResponse
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnAcceptWantResponse
 * EnvConditions: NA
 * CaseDescription: Verify OnAcceptWantResponse launchReason
 */
HWTEST_F(MissionListManagerTest, OnAcceptWantResponse_003, TestSize.Level3)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    Want want;
    std::shared_ptr<AbilityRecord> ability;
    AbilityRequest abilityRequest;
    abilityRequest.want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    InitMockMission(missionListManager, abilityRequest, want, ability);
    if (ability == nullptr) {
        return;
    }

    missionListManager->OnAcceptWantResponse(want, "flag");
    EXPECT_EQ(ability->lifeCycleStateInfo_.launchParam.launchReason, LaunchReason::LAUNCHREASON_CONTINUATION);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnAcceptWantResponse
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnAcceptWantResponse
 * EnvConditions: NA
 * CaseDescription: Verify OnAcceptWantResponse launchReason
 */
HWTEST_F(MissionListManagerTest, OnAcceptWantResponse_004, TestSize.Level3)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    Want want;
    std::shared_ptr<AbilityRecord> ability;
    AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    InitMockMission(missionListManager, abilityRequest, want, ability);
    if (ability == nullptr) {
        return;
    }

    missionListManager->OnAcceptWantResponse(want, "flag");
    EXPECT_EQ(ability->lifeCycleStateInfo_.launchParam.launchReason, LaunchReason::LAUNCHREASON_APP_RECOVERY);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnAcceptWantResponse
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnAcceptWantResponse
 * EnvConditions: NA
 * CaseDescription: Verify OnAcceptWantResponse launchReason
 */
HWTEST_F(MissionListManagerTest, OnAcceptWantResponse_005, TestSize.Level3)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    Want want;
    std::shared_ptr<AbilityRecord> ability;
    AbilityRequest abilityRequest;
    InitMockMission(missionListManager, abilityRequest, want, ability);
    if (ability == nullptr) {
        return;
    }

    missionListManager->OnAcceptWantResponse(want, "flag");
    EXPECT_EQ(ability->lifeCycleStateInfo_.launchParam.launchReason, LaunchReason::LAUNCHREASON_START_ABILITY);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: SetMissionLabel
 * SubFunction: NA
 * FunctionPoints: MissionListManager SetMissionLabel
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionLabel
 */
HWTEST_F(MissionListManagerTest, SetMissionLabel_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto ability = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(11, ability, "missionName");
    mission->abilityRecord_ = ability;

    EXPECT_EQ(missionListManager->SetMissionLabel(nullptr, "label"), -1);
    EXPECT_EQ(missionListManager->SetMissionLabel(ability->GetToken(), "label"), -1);

    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: NA
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: The back event supports transition animation.
 */
HWTEST_F(MissionListManagerTest, BackAnimation_001, TestSize.Level1)
{
    g_notifyWindowTransitionCalled = false;
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();

    auto abilityMs = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    sptr<MockWMSHandler> wmsHandler = new MockWMSHandler();
    abilityMs->wmsHandler_ = wmsHandler;

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityA = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto abilityB = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityB->SetAbilityState(AbilityState::FOREGROUND);
    abilityB->SetNextAbilityRecord(abilityA);
    missionListManager->TerminateAbilityLocked(abilityB, false);
    EXPECT_EQ(g_notifyWindowTransitionCalled, true);

    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: NA
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: The back event supports closing animation.
 */
HWTEST_F(MissionListManagerTest, BackAnimation_002, TestSize.Level1)
{
    g_notifyWindowTransitionCalled = false;
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();

    auto abilityMs = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    sptr<MockWMSHandler> wmsHandler = new MockWMSHandler();
    abilityMs->wmsHandler_ = wmsHandler;

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityB = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityB->SetAbilityState(AbilityState::FOREGROUND);
    missionListManager->TerminateAbilityLocked(abilityB, false);
    EXPECT_EQ(g_notifyWindowTransitionCalled, true);

    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: NA
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Load timeout cancel startingWindow.
 */
HWTEST_F(MissionListManagerTest, CancelStartingWindow_001, TestSize.Level1)
{
    g_cancelStartingWindowCalled = false;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->SetStartingWindow(true);
    auto windowHandler = std::make_shared<MockWMSHandler>();

    auto task = [windowHandler, abilityRecord] {
        if (windowHandler && abilityRecord && abilityRecord->IsStartingWindow() &&
            (abilityRecord->GetScheduler() == nullptr ||
            abilityRecord->GetAbilityState() != AbilityState::FOREGROUNDING)) {
            HILOG_INFO("PostCancelStartingWindowColdTask, call windowHandler CancelStartingWindow.");
            windowHandler->CancelStartingWindow(abilityRecord->GetToken());
            abilityRecord->SetStartingWindow(false);
        }
    };
    task();
    EXPECT_EQ(g_cancelStartingWindowCalled, true);
}

/*
 * Feature: MissionListManager
 * Function: NA
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Foreground timeout cancel startingWindow.
 */
HWTEST_F(MissionListManagerTest, CancelStartingWindow_002, TestSize.Level1)
{
    g_cancelStartingWindowCalled = false;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->SetStartingWindow(true);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    auto windowHandler = std::make_shared<MockWMSHandler>();

    auto task = [windowHandler, abilityRecord] {
        if (windowHandler && abilityRecord && abilityRecord->IsStartingWindow() &&
            abilityRecord->GetAbilityState() != AbilityState::FOREGROUNDING) {
            HILOG_INFO("PostCancelStartingWindowHotTask, call windowHandler CancelStartingWindow.");
            windowHandler->CancelStartingWindow(abilityRecord->GetToken());
            abilityRecord->SetStartingWindow(false);
        }
    };
    task();
    EXPECT_EQ(g_cancelStartingWindowCalled, true);
}

/*
 * Feature: MissionListManager
 * Function: RegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: MissionListManager RegisterMissionListener
 * EnvConditions: NA
 * CaseDescription: Verify RegisterMissionListener
 */
HWTEST_F(MissionListManagerTest, RegisterMissionListener_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    sptr<IMissionListener> listener;
    missionListManager->listenerController_ = nullptr;
    int res = missionListManager->RegisterMissionListener(listener);
    EXPECT_EQ(res, -1);
}

/*
 * Feature: MissionListManager
 * Function: RegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: MissionListManager RegisterMissionListener
 * EnvConditions: NA
 * CaseDescription: Verify RegisterMissionListener
 */
HWTEST_F(MissionListManagerTest, RegisterMissionListener_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    sptr<IMissionListener> listener;
    missionListManager->Init();
    int res = missionListManager->RegisterMissionListener(listener);
    EXPECT_EQ(res, -1);
}

/*
 * Feature: MissionListManager
 * Function: UnRegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: MissionListManager UnRegisterMissionListener
 * EnvConditions: NA
 * CaseDescription: Verify UnRegisterMissionListener
 */
HWTEST_F(MissionListManagerTest, UnRegisterMissionListener_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    sptr<IMissionListener> listener;
    missionListManager->listenerController_ = nullptr;
    int res = missionListManager->UnRegisterMissionListener(listener);
    EXPECT_EQ(res, -1);
}

/*
 * Feature: MissionListManager
 * Function: UnRegisterMissionListener
 * SubFunction: NA
 * FunctionPoints: MissionListManager UnRegisterMissionListener
 * EnvConditions: NA
 * CaseDescription: Verify UnRegisterMissionListener
 */
HWTEST_F(MissionListManagerTest, UnRegisterMissionListener_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    sptr<IMissionListener> listener;
    missionListManager->Init();
    int res = missionListManager->UnRegisterMissionListener(listener);
    EXPECT_NE(res, -1);
}

/*
 * Feature: MissionListManager
 * Function: GetMissionInfos
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionInfos
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionInfos
 */
HWTEST_F(MissionListManagerTest, GetMissionInfos_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int32_t numMax = -1;
    std::vector<MissionInfo> missionInfos;
    int res = missionListManager->GetMissionInfos(numMax, missionInfos);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: MissionListManager
 * Function: GetMissionInfos
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionInfos
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionInfos
 */
HWTEST_F(MissionListManagerTest, GetMissionInfos_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int32_t numMax = 0;
    std::vector<MissionInfo> missionInfos;
    int res = missionListManager->GetMissionInfos(numMax, missionInfos);
    EXPECT_NE(res, ERR_INVALID_VALUE);
}

/*
 * Feature: MissionListManager
 * Function: StartWaitingAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager StartWaitingAbility
 * EnvConditions: NA
 * CaseDescription: Verify StartWaitingAbility
 */
HWTEST_F(MissionListManagerTest, StartWaitingAbility_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = FOREGROUND;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission);
    missionListManager->currentMissionLists_.push_front(missionList);
    missionListManager->StartWaitingAbility();
}

/*
 * Feature: MissionListManager
 * Function: StartWaitingAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager StartWaitingAbility
 * EnvConditions: NA
 * CaseDescription: Verify StartWaitingAbility
 */
HWTEST_F(MissionListManagerTest, StartWaitingAbility_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = BACKGROUND;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission);
    missionListManager->currentMissionLists_.push_front(missionList);
    missionListManager->StartWaitingAbility();
}

/*
 * Feature: MissionListManager
 * Function: CreateOrReusedMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionListManager CreateOrReusedMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify CreateOrReusedMissionInfo
 */
HWTEST_F(MissionListManagerTest, CreateOrReusedMissionInfo_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    AbilityRequest abilityRequest;
    InnerMissionInfo info;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    bool res = missionListManager->CreateOrReusedMissionInfo(abilityRequest, info);
    EXPECT_FALSE(res);
}

/*
 * Feature: MissionListManager
 * Function: CreateOrReusedMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionListManager CreateOrReusedMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify CreateOrReusedMissionInfo
 */
HWTEST_F(MissionListManagerTest, CreateOrReusedMissionInfo_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    AbilityRequest abilityRequest;
    InnerMissionInfo info;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.abilityInfo.applicationInfo.isLauncherApp = true;
    bool res = missionListManager->CreateOrReusedMissionInfo(abilityRequest, info);
    EXPECT_FALSE(res);
}

/*
 * Feature: MissionListManager
 * Function: CreateOrReusedMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionListManager CreateOrReusedMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify CreateOrReusedMissionInfo
 */
HWTEST_F(MissionListManagerTest, CreateOrReusedMissionInfo_003, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    AbilityRequest abilityRequest;
    InnerMissionInfo info;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.abilityInfo.applicationInfo.isLauncherApp = false;
    info.missionInfo.id = 0;
    bool res = missionListManager->CreateOrReusedMissionInfo(abilityRequest, info);
    EXPECT_FALSE(res);
}

/*
 * Feature: MissionListManager
 * Function: CreateOrReusedMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionListManager CreateOrReusedMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify CreateOrReusedMissionInfo
 */
HWTEST_F(MissionListManagerTest, CreateOrReusedMissionInfo_004, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    AbilityRequest abilityRequest;
    InnerMissionInfo info;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.abilityInfo.applicationInfo.isLauncherApp = false;
    info.missionInfo.id = 1;
    bool res = missionListManager->CreateOrReusedMissionInfo(abilityRequest, info);
    EXPECT_FALSE(res);
}

/*
 * Feature: MissionListManager
 * Function: BuildInnerMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionListManager BuildInnerMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify BuildInnerMissionInfo
 */
HWTEST_F(MissionListManagerTest, BuildInnerMissionInfo_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    InnerMissionInfo info;
    std::string missionName = "missionName";
    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::INVALID_TYPE;
    abilityRequest.want.SetParam(DLP_INDEX, 0);
    missionListManager->BuildInnerMissionInfo(info, missionName, abilityRequest);
}

/*
 * Feature: MissionListManager
 * Function: BuildInnerMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionListManager BuildInnerMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify BuildInnerMissionInfo
 */
HWTEST_F(MissionListManagerTest, BuildInnerMissionInfo_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    InnerMissionInfo info;
    std::string missionName = "missionName";
    AbilityRequest abilityRequest;
    std::string deviceId = "deviceId";
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    std::string moduleName = "moduleName";
    abilityRequest.callType = AbilityCallType::START_OPTIONS_TYPE;
    abilityRequest.want.SetParam(DLP_INDEX, 1);
    abilityRequest.want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    missionListManager->BuildInnerMissionInfo(info, missionName, abilityRequest);
}

/*
 * Feature: MissionListManager
 * Function: GetTargetMissionList
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetTargetMissionList
 * EnvConditions: NA
 * CaseDescription: Verify GetTargetMissionList
 */
HWTEST_F(MissionListManagerTest, GetTargetMissionList_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> callerAbility;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.applicationInfo.isLauncherApp = true;
    auto res = missionListManager->GetTargetMissionList(callerAbility, abilityRequest);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: MissionListManager
 * Function: GetTargetMissionList
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetTargetMissionList
 * EnvConditions: NA
 * CaseDescription: Verify GetTargetMissionList
 */
HWTEST_F(MissionListManagerTest, GetTargetMissionList_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> callerAbility;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.applicationInfo.isLauncherApp = false;
    auto res = missionListManager->GetTargetMissionList(callerAbility, abilityRequest);
    EXPECT_NE(res, nullptr);
}

/*
 * Feature: MissionListManager
 * Function: GetTargetMissionList
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetTargetMissionList
 * EnvConditions: NA
 * CaseDescription: Verify GetTargetMissionList
 */
HWTEST_F(MissionListManagerTest, GetTargetMissionList_003, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> callerAbility = InitAbilityRecord();
    AbilityRequest abilityRequest;
    callerAbility->isLauncherAbility_ = true;
    abilityRequest.abilityInfo.applicationInfo.isLauncherApp = false;
    auto res = missionListManager->GetTargetMissionList(callerAbility, abilityRequest);
    EXPECT_NE(res, nullptr);
}

/*
 * Feature: MissionListManager
 * Function: GetTargetMissionList
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetTargetMissionList
 * EnvConditions: NA
 * CaseDescription: Verify GetTargetMissionList
 */
HWTEST_F(MissionListManagerTest, GetTargetMissionList_004, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> callerAbility = InitAbilityRecord();
    AbilityRequest abilityRequest;
    callerAbility->isLauncherAbility_ = false;
    abilityRequest.abilityInfo.applicationInfo.isLauncherApp = false;
    auto res = missionListManager->GetTargetMissionList(callerAbility, abilityRequest);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: MissionListManager
 * Function: GetReusedMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetReusedMission
 * EnvConditions: NA
 * CaseDescription: Verify GetReusedMission
 */
HWTEST_F(MissionListManagerTest, GetReusedMission_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    auto res = missionListManager->GetReusedMission(abilityRequest);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: MissionListManager
 * Function: GetReusedMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetReusedMission
 * EnvConditions: NA
 * CaseDescription: Verify GetReusedMission
 */
HWTEST_F(MissionListManagerTest, GetReusedMission_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    auto res = missionListManager->GetReusedMission(abilityRequest);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: MissionListManager
 * Function: GetReusedMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetReusedMission
 * EnvConditions: NA
 * CaseDescription: Verify GetReusedMission
 */
HWTEST_F(MissionListManagerTest, GetReusedMission_003, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    abilityRequest.abilityInfo.applicationInfo.isLauncherApp = true;
    auto res = missionListManager->GetReusedMission(abilityRequest);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: MissionListManager
 * Function: GetReusedMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetReusedMission
 * EnvConditions: NA
 * CaseDescription: Verify GetReusedMission
 */
HWTEST_F(MissionListManagerTest, GetReusedMission_004, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    std::string missionName = "#::";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, missionName);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission);
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    abilityRequest.abilityInfo.applicationInfo.isLauncherApp = true;
    missionListManager->launcherList_ = missionList;
    auto res = missionListManager->GetReusedMission(abilityRequest);
    EXPECT_NE(res, nullptr);
}

/*
 * Feature: MissionListManager
 * Function: GetReusedMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetReusedMission
 * EnvConditions: NA
 * CaseDescription: Verify GetReusedMission
 */
HWTEST_F(MissionListManagerTest, GetReusedMission_005, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    std::string missionName = "#::";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(2, abilityRecord, missionName);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission);
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    abilityRequest.abilityInfo.applicationInfo.isLauncherApp = false;
    missionListManager->currentMissionLists_.push_front(missionList);
    auto res = missionListManager->GetReusedMission(abilityRequest);
    EXPECT_NE(res, nullptr);
}

/*
 * Feature: MissionListManager
 * Function: GetReusedMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetReusedMission
 * EnvConditions: NA
 * CaseDescription: Verify GetReusedMission
 */
HWTEST_F(MissionListManagerTest, GetReusedMission_006, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    std::string missionName = "#::";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(3, abilityRecord, missionName);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission);
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    abilityRequest.abilityInfo.applicationInfo.isLauncherApp = false;
    missionListManager->defaultSingleList_ = missionList;
    auto res = missionListManager->GetReusedMission(abilityRequest);
    EXPECT_NE(res, nullptr);
}
}  // namespace AAFwk
}  // namespace OHOS
