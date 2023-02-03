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
#include "mission_info_mgr.h"
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

#ifdef SUPPORT_GRAPHICS
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
#endif

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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
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
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetReusedSpecifiedMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetReusedSpecifiedMission
 * EnvConditions: NA
 * CaseDescription: Verify GetReusedSpecifiedMission
 */
HWTEST_F(MissionListManagerTest, GetReusedSpecifiedMission_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    auto res = missionListManager->GetReusedSpecifiedMission(abilityRequest);
    EXPECT_EQ(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetReusedSpecifiedMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetReusedSpecifiedMission
 * EnvConditions: NA
 * CaseDescription: Verify GetReusedSpecifiedMission
 */
HWTEST_F(MissionListManagerTest, GetReusedSpecifiedMission_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    std::string missionName = "#::";
    std::string flag = "flag";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, missionName);
    mission->SetSpecifiedFlag(flag);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission);
    missionListManager->launcherList_ = missionList;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.abilityInfo.applicationInfo.isLauncherApp = true;
    abilityRequest.specifiedFlag = flag;
    auto res = missionListManager->GetReusedSpecifiedMission(abilityRequest);
    EXPECT_NE(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetReusedSpecifiedMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetReusedSpecifiedMission
 * EnvConditions: NA
 * CaseDescription: Verify GetReusedSpecifiedMission
 */
HWTEST_F(MissionListManagerTest, GetReusedSpecifiedMission_003, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    std::string missionName = "#::";
    std::string flag = "flag";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(2, abilityRecord, missionName);
    mission->SetSpecifiedFlag(flag);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission);
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->launcherList_ = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.clear();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.abilityInfo.applicationInfo.isLauncherApp = true;
    abilityRequest.specifiedFlag = flag;
    auto res = missionListManager->GetReusedSpecifiedMission(abilityRequest);
    EXPECT_NE(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetReusedSpecifiedMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetReusedSpecifiedMission
 * EnvConditions: NA
 * CaseDescription: Verify GetReusedSpecifiedMission
 */
HWTEST_F(MissionListManagerTest, GetReusedSpecifiedMission_004, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    std::string missionName = "#::";
    std::string flag = "flag";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(2, abilityRecord, missionName);
    mission->SetSpecifiedFlag(flag);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission);
    missionListManager->defaultStandardList_ = std::make_shared<MissionList>();
    missionListManager->launcherList_ = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.push_front(missionList);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.abilityInfo.applicationInfo.isLauncherApp = false;
    abilityRequest.specifiedFlag = flag;
    auto res = missionListManager->GetReusedSpecifiedMission(abilityRequest);
    EXPECT_NE(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveMissionToTargetList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveMissionToTargetList
 * EnvConditions: NA
 * CaseDescription: Verify MoveMissionToTargetList
 */
HWTEST_F(MissionListManagerTest, MoveMissionToTargetList_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    bool isCallFromLauncher = true;
    std::shared_ptr<MissionList> targetMissionList = std::make_shared<MissionList>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::string missionName = "missionName";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, missionName);
    mission->SetMissionList(nullptr);
    missionListManager->MoveMissionToTargetList(isCallFromLauncher, targetMissionList, mission);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveMissionToTargetList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveMissionToTargetList
 * EnvConditions: NA
 * CaseDescription: Verify MoveMissionToTargetList
 */
HWTEST_F(MissionListManagerTest, MoveMissionToTargetList_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    bool isCallFromLauncher = true;
    std::shared_ptr<MissionList> targetMissionList = std::make_shared<MissionList>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::string missionName = "missionName";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, missionName);
    mission->SetMissionList(targetMissionList);
    missionListManager->launcherList_ = targetMissionList;
    missionListManager->MoveMissionToTargetList(isCallFromLauncher, targetMissionList, mission);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveMissionToTargetList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveMissionToTargetList
 * EnvConditions: NA
 * CaseDescription: Verify MoveMissionToTargetList
 */
HWTEST_F(MissionListManagerTest, MoveMissionToTargetList_003, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    bool isCallFromLauncher = false;
    std::shared_ptr<MissionList> targetMissionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::string missionName = "missionName";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(2, abilityRecord, missionName);
    mission->SetMissionList(missionList);
    missionListManager->launcherList_ = targetMissionList;
    missionListManager->MoveMissionToTargetList(isCallFromLauncher, targetMissionList, mission);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveMissionToTargetList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveMissionToTargetList
 * EnvConditions: NA
 * CaseDescription: Verify MoveMissionToTargetList
 */
HWTEST_F(MissionListManagerTest, MoveMissionToTargetList_004, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    bool isCallFromLauncher = true;
    std::shared_ptr<MissionList> targetMissionList = std::make_shared<MissionList>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::string missionName = "missionName";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(3, abilityRecord, missionName);
    mission->SetMissionList(targetMissionList);
    missionListManager->launcherList_ = nullptr;
    missionListManager->MoveMissionToTargetList(isCallFromLauncher, targetMissionList, mission);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveNoneTopMissionToDefaultList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveNoneTopMissionToDefaultList
 * EnvConditions: NA
 * CaseDescription: Verify MoveNoneTopMissionToDefaultList
 */
HWTEST_F(MissionListManagerTest, MoveNoneTopMissionToDefaultList_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::string missionName = "missionName";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, missionName);
    mission->SetMissionList(nullptr);
    missionListManager->MoveNoneTopMissionToDefaultList(mission);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveNoneTopMissionToDefaultList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveNoneTopMissionToDefaultList
 * EnvConditions: NA
 * CaseDescription: Verify MoveNoneTopMissionToDefaultList
 */
HWTEST_F(MissionListManagerTest, MoveNoneTopMissionToDefaultList_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::string missionName = "missionName";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, missionName);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission);
    mission->SetMissionList(missionList);
    missionListManager->MoveNoneTopMissionToDefaultList(mission);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveNoneTopMissionToDefaultList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveNoneTopMissionToDefaultList
 * EnvConditions: NA
 * CaseDescription: Verify MoveNoneTopMissionToDefaultList
 */
HWTEST_F(MissionListManagerTest, MoveNoneTopMissionToDefaultList_003, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::string missionName = "missionName";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, missionName);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(nullptr);
    mission->SetMissionList(missionList);
    missionListManager->MoveNoneTopMissionToDefaultList(mission);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveNoneTopMissionToDefaultList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveNoneTopMissionToDefaultList
 * EnvConditions: NA
 * CaseDescription: Verify MoveNoneTopMissionToDefaultList
 */
HWTEST_F(MissionListManagerTest, MoveNoneTopMissionToDefaultList_004, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    std::string missionName = "missionName";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, missionName);
    std::shared_ptr<Mission> mission2 = std::make_shared<Mission>(2, abilityRecord, missionName);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission2);
    mission->SetMissionList(missionList);
    missionListManager->MoveNoneTopMissionToDefaultList(mission);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveNoneTopMissionToDefaultList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveNoneTopMissionToDefaultList
 * EnvConditions: NA
 * CaseDescription: Verify MoveNoneTopMissionToDefaultList
 */
HWTEST_F(MissionListManagerTest, MoveNoneTopMissionToDefaultList_005, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    std::string missionName = "missionName";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, missionName);
    std::shared_ptr<Mission> mission2 = std::make_shared<Mission>(2, abilityRecord, missionName);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission2);
    mission->SetMissionList(missionList);
    missionListManager->MoveNoneTopMissionToDefaultList(mission);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveMissionListToTop
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveMissionListToTop
 * EnvConditions: NA
 * CaseDescription: Verify MoveMissionListToTop
 */
HWTEST_F(MissionListManagerTest, MoveMissionListToTop_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = nullptr;
    missionListManager->MoveMissionListToTop(missionList);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MinimizeAbilityLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager MinimizeAbilityLocked
 * EnvConditions: NA
 * CaseDescription: Verify MinimizeAbilityLocked
 */
HWTEST_F(MissionListManagerTest, MinimizeAbilityLocked_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    bool fromUser = true;
    int res = missionListManager->MinimizeAbilityLocked(abilityRecord, fromUser);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MinimizeAbilityLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager MinimizeAbilityLocked
 * EnvConditions: NA
 * CaseDescription: Verify MinimizeAbilityLocked
 */
HWTEST_F(MissionListManagerTest, MinimizeAbilityLocked_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    bool fromUser = true;
    int res = missionListManager->MinimizeAbilityLocked(abilityRecord, fromUser);
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MinimizeAbilityLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager MinimizeAbilityLocked
 * EnvConditions: NA
 * CaseDescription: Verify MinimizeAbilityLocked
 */
HWTEST_F(MissionListManagerTest, MinimizeAbilityLocked_003, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    bool fromUser = true;
    int res = missionListManager->MinimizeAbilityLocked(abilityRecord, fromUser);
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetCurrentTopAbilityLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetCurrentTopAbilityLocked
 * EnvConditions: NA
 * CaseDescription: Verify GetCurrentTopAbilityLocked
 */
HWTEST_F(MissionListManagerTest, GetCurrentTopAbilityLocked_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->currentMissionLists_.clear();
    auto res = missionListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetCurrentTopAbilityLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetCurrentTopAbilityLocked
 * EnvConditions: NA
 * CaseDescription: Verify GetCurrentTopAbilityLocked
 */
HWTEST_F(MissionListManagerTest, GetCurrentTopAbilityLocked_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->currentMissionLists_.push_front(nullptr);
    auto res = missionListManager->GetCurrentTopAbilityLocked();
    EXPECT_EQ(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: AttachAbilityThread
 * SubFunction: NA
 * FunctionPoints: MissionListManager AttachAbilityThread
 * EnvConditions: NA
 * CaseDescription: Verify AttachAbilityThread
 */
HWTEST_F(MissionListManagerTest, AttachAbilityThread_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    DelayedSingleton<AbilityManagerService>::GetInstance()->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IAbilityScheduler> scheduler = nullptr;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    bool isFlag = true;
    abilityRecord->SetStartedByCall(isFlag);
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    int res = missionListManager->AttachAbilityThread(scheduler, token);
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnAbilityRequestDone
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnAbilityRequestDone
 * EnvConditions: NA
 * CaseDescription: Verify OnAbilityRequestDone
 */
HWTEST_F(MissionListManagerTest, OnAbilityRequestDone_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    sptr<IRemoteObject> token = nullptr;
    int32_t state = 2;
    missionListManager->OnAbilityRequestDone(token, state);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnAbilityRequestDone
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnAbilityRequestDone
 * EnvConditions: NA
 * CaseDescription: Verify OnAbilityRequestDone
 */
HWTEST_F(MissionListManagerTest, OnAbilityRequestDone_002, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    sptr<IRemoteObject> token = nullptr;
    int32_t state = 4;
    missionListManager->OnAbilityRequestDone(token, state);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnAppStateChanged
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify OnAppStateChanged
 */
HWTEST_F(MissionListManagerTest, OnAppStateChanged_001, TestSize.Level1)
{
    int userId = 0;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    AppInfo info;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::string processName = "processName";
    abilityRecord->applicationInfo_.bundleName = processName;
    info.state = AppState::END;
    info.processName = processName;
    missionListManager->terminateAbilityList_.push_back(nullptr);
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    missionListManager->OnAppStateChanged(info);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnAppStateChanged
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify OnAppStateChanged
 */
HWTEST_F(MissionListManagerTest, OnAppStateChanged_002, TestSize.Level1)
{
    int userId = 2;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord1 = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    std::string processName = "processName";
    abilityRecord1->abilityInfo_.process = processName;
    abilityRecord2->applicationInfo_.bundleName = processName;
    std::shared_ptr<Mission> mission1 = std::make_shared<Mission>(1, abilityRecord1, "missionName");
    std::shared_ptr<Mission> mission2 = std::make_shared<Mission>(2, abilityRecord2, "missionName");
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList2 = std::make_shared<MissionList>();
    missionList->missions_.push_back(nullptr);
    missionList->missions_.push_back(mission1);
    missionList->missions_.push_back(mission2);
    AppInfo info;
    info.processName = processName;
    missionListManager->currentMissionLists_.push_back(missionList);
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->OnAppStateChanged(info);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetAbilityRecordByToken
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetAbilityRecordByToken
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByToken
 */
HWTEST_F(MissionListManagerTest, GetAbilityRecordByToken_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_back(mission);
    missionListManager->terminateAbilityList_.clear();
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    auto res = missionListManager->GetAbilityRecordByToken(token);
    EXPECT_NE(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionById
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionById
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionById
 */
HWTEST_F(MissionListManagerTest, GetMissionById_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    int missionId = 1;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    auto res = missionListManager->GetMissionById(missionId);
    EXPECT_NE(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionById
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionById
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionById
 */
HWTEST_F(MissionListManagerTest, GetMissionById_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    int missionId = 1;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList2 = std::make_shared<MissionList>();
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList2;
    missionListManager->launcherList_ = missionList;
    auto res = missionListManager->GetMissionById(missionId);
    EXPECT_NE(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: AbilityTransactionDone
 * SubFunction: NA
 * FunctionPoints: MissionListManager AbilityTransactionDone
 * EnvConditions: NA
 * CaseDescription: Verify AbilityTransactionDone
 */
HWTEST_F(MissionListManagerTest, AbilityTransactionDone_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    int state = 0;
    PacMap saveData;
    missionList->missions_.push_back(mission);
    missionListManager->terminateAbilityList_.clear();
    missionListManager->defaultSingleList_ = missionList;
    int res = missionListManager->AbilityTransactionDone(token, state, saveData);
    EXPECT_EQ(res, INNER_ERR);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: AbilityTransactionDone
 * SubFunction: NA
 * FunctionPoints: MissionListManager AbilityTransactionDone
 * EnvConditions: NA
 * CaseDescription: Verify AbilityTransactionDone
 */
HWTEST_F(MissionListManagerTest, AbilityTransactionDone_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    int state = 6;
    PacMap saveData;
    missionList->missions_.push_back(mission);
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    missionListManager->defaultSingleList_ = missionList;
    int res = missionListManager->AbilityTransactionDone(token, state, saveData);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: DispatchState
 * SubFunction: NA
 * FunctionPoints: MissionListManager DispatchState
 * EnvConditions: NA
 * CaseDescription: Verify DispatchState
 */
HWTEST_F(MissionListManagerTest, DispatchState_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    int state = 9;
    int res = missionListManager->DispatchState(abilityRecord, state);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: DispatchState
 * SubFunction: NA
 * FunctionPoints: MissionListManager DispatchState
 * EnvConditions: NA
 * CaseDescription: Verify DispatchState
 */
HWTEST_F(MissionListManagerTest, DispatchState_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    int state = 13;
    int res = missionListManager->DispatchState(abilityRecord, state);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: DispatchState
 * SubFunction: NA
 * FunctionPoints: MissionListManager DispatchState
 * EnvConditions: NA
 * CaseDescription: Verify DispatchState
 */
HWTEST_F(MissionListManagerTest, DispatchState_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    int state = 14;
    int res = missionListManager->DispatchState(abilityRecord, state);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: DispatchState
 * SubFunction: NA
 * FunctionPoints: MissionListManager DispatchState
 * EnvConditions: NA
 * CaseDescription: Verify DispatchState
 */
HWTEST_F(MissionListManagerTest, DispatchState_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    int state = 5;
    int res = missionListManager->DispatchState(abilityRecord, state);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: DispatchForeground
 * SubFunction: NA
 * FunctionPoints: MissionListManager DispatchForeground
 * EnvConditions: NA
 * CaseDescription: Verify DispatchForeground
 */
HWTEST_F(MissionListManagerTest, DispatchForeground_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    bool success = true;
    bool isInvalidMode = true;
    int res = missionListManager->DispatchForeground(abilityRecord, success, isInvalidMode);
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: DispatchForeground
 * SubFunction: NA
 * FunctionPoints: MissionListManager DispatchForeground
 * EnvConditions: NA
 * CaseDescription: Verify DispatchForeground
 */
HWTEST_F(MissionListManagerTest, DispatchForeground_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUNDING;
    bool success = false;
    bool isInvalidMode = false;
    int res = missionListManager->DispatchForeground(abilityRecord, success, isInvalidMode);
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteForegroundSuccess
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteForegroundSuccess
 * EnvConditions: NA
 * CaseDescription: Verify CompleteForegroundSuccess
 */
HWTEST_F(MissionListManagerTest, CompleteForegroundSuccess_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.excludeFromMissions = false;
    abilityRecord->SetStartedByCall(true);
    abilityRecord->SetStartToForeground(true);
    abilityRecord->isReady_ = true;
    abilityRecord->pendingState_ = AbilityState::BACKGROUND;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    mission->SetMovingState(true);
    abilityRecord->SetMission(mission);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_[1] = true;
    InnerMissionInfo info;
    info.missionInfo.id = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.push_back(info);
    missionListManager->CompleteForegroundSuccess(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteForegroundSuccess
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteForegroundSuccess
 * EnvConditions: NA
 * CaseDescription: Verify CompleteForegroundSuccess
 */
HWTEST_F(MissionListManagerTest, CompleteForegroundSuccess_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    abilityRecord->SetStartedByCall(false);
    abilityRecord->SetStartToForeground(true);
    abilityRecord->isReady_ = true;
    abilityRecord->pendingState_ = AbilityState::FOREGROUND;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    mission->SetMovingState(true);
    abilityRecord->SetMission(mission);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_.clear();
    InnerMissionInfo info;
    info.missionInfo.id = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.clear();
    missionListManager->CompleteForegroundSuccess(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteForegroundSuccess
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteForegroundSuccess
 * EnvConditions: NA
 * CaseDescription: Verify CompleteForegroundSuccess
 */
HWTEST_F(MissionListManagerTest, CompleteForegroundSuccess_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    missionListManager->listenerController_ = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    abilityRecord->SetStartedByCall(true);
    abilityRecord->SetStartToForeground(false);
    abilityRecord->isReady_ = true;
    abilityRecord->pendingState_ = AbilityState::ACTIVE;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    mission->SetMovingState(true);
    abilityRecord->SetMission(mission);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_.clear();
    InnerMissionInfo info;
    info.missionInfo.id = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.clear();
    missionListManager->CompleteForegroundSuccess(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteForegroundSuccess
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteForegroundSuccess
 * EnvConditions: NA
 * CaseDescription: Verify CompleteForegroundSuccess
 */
HWTEST_F(MissionListManagerTest, CompleteForegroundSuccess_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    missionListManager->listenerController_ = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    abilityRecord->SetStartedByCall(true);
    abilityRecord->SetStartToForeground(true);
    abilityRecord->isReady_ = false;
    abilityRecord->pendingState_ = AbilityState::ACTIVE;
    abilityRecord->SetMission(nullptr);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_.clear();
    InnerMissionInfo info;
    info.missionInfo.id = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.clear();
    missionListManager->CompleteForegroundSuccess(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteForegroundSuccess
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteForegroundSuccess
 * EnvConditions: NA
 * CaseDescription: Verify CompleteForegroundSuccess
 */
HWTEST_F(MissionListManagerTest, CompleteForegroundSuccess_005, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    missionListManager->listenerController_ = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    abilityRecord->SetStartedByCall(true);
    abilityRecord->SetStartToForeground(false);
    abilityRecord->isReady_ = true;
    abilityRecord->pendingState_ = AbilityState::ACTIVE;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    mission->SetMovingState(false);
    abilityRecord->SetMission(mission);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_.clear();
    InnerMissionInfo info;
    info.missionInfo.id = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.clear();
    missionListManager->CompleteForegroundSuccess(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: TerminatePreviousAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager TerminatePreviousAbility
 * EnvConditions: NA
 * CaseDescription: Verify TerminatePreviousAbility
 */
HWTEST_F(MissionListManagerTest, TerminatePreviousAbility_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetPreAbilityRecord(nullptr);
    missionListManager->TerminatePreviousAbility(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: TerminatePreviousAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager TerminatePreviousAbility
 * EnvConditions: NA
 * CaseDescription: Verify TerminatePreviousAbility
 */
HWTEST_F(MissionListManagerTest, TerminatePreviousAbility_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> preAbilityRecord = InitAbilityRecord();
    preAbilityRecord->currentState_ = AbilityState::FOREGROUND;
    abilityRecord->preAbilityRecord_ = preAbilityRecord;
    missionListManager->TerminatePreviousAbility(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: TerminatePreviousAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager TerminatePreviousAbility
 * EnvConditions: NA
 * CaseDescription: Verify TerminatePreviousAbility
 */
HWTEST_F(MissionListManagerTest, TerminatePreviousAbility_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> preAbilityRecord = InitAbilityRecord();
    preAbilityRecord->currentState_ = AbilityState::BACKGROUND;
    abilityRecord->preAbilityRecord_ = preAbilityRecord;
    missionListManager->TerminatePreviousAbility(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: DispatchBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager DispatchBackground
 * EnvConditions: NA
 * CaseDescription: Verify DispatchBackground
 */
HWTEST_F(MissionListManagerTest, DispatchBackground_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    int res = missionListManager->DispatchBackground(abilityRecord);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: DispatchBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager DispatchBackground
 * EnvConditions: NA
 * CaseDescription: Verify DispatchBackground
 */
HWTEST_F(MissionListManagerTest, DispatchBackground_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    int res = missionListManager->DispatchBackground(abilityRecord);
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteBackground
 * EnvConditions: NA
 * CaseDescription: Verify CompleteBackground
 */
HWTEST_F(MissionListManagerTest, CompleteBackground_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::FOREGROUND;
    missionListManager->CompleteBackground(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteBackground
 * EnvConditions: NA
 * CaseDescription: Verify CompleteBackground
 */
HWTEST_F(MissionListManagerTest, CompleteBackground_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    abilityRecord->SetSwitchingPause(true);
    missionListManager->CompleteBackground(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteBackground
 * EnvConditions: NA
 * CaseDescription: Verify CompleteBackground
 */
HWTEST_F(MissionListManagerTest, CompleteBackground_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    abilityRecord->SetSwitchingPause(false);
    abilityRecord->SetStartedByCall(true);
    abilityRecord->SetStartToBackground(true);
    abilityRecord->isReady_ = true;
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    missionListManager->CompleteBackground(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteBackground
 * EnvConditions: NA
 * CaseDescription: Verify CompleteBackground
 */
HWTEST_F(MissionListManagerTest, CompleteBackground_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    abilityRecord->SetSwitchingPause(false);
    abilityRecord->SetStartedByCall(false);
    abilityRecord->SetStartToBackground(true);
    abilityRecord->isReady_ = true;
    abilityRecord2->currentState_ = AbilityState::BACKGROUND;
    missionListManager->terminateAbilityList_.push_back(abilityRecord2);
    missionListManager->CompleteBackground(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteBackground
 * EnvConditions: NA
 * CaseDescription: Verify CompleteBackground
 */
HWTEST_F(MissionListManagerTest, CompleteBackground_005, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    abilityRecord->SetSwitchingPause(false);
    abilityRecord->SetStartedByCall(true);
    abilityRecord->SetStartToBackground(false);
    abilityRecord->isReady_ = true;
    abilityRecord2->currentState_ = AbilityState::BACKGROUND;
    missionListManager->terminateAbilityList_.push_back(abilityRecord2);
    missionListManager->CompleteBackground(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteBackground
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteBackground
 * EnvConditions: NA
 * CaseDescription: Verify CompleteBackground
 */
HWTEST_F(MissionListManagerTest, CompleteBackground_006, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::BACKGROUNDING;
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    abilityRecord->SetSwitchingPause(false);
    abilityRecord->SetStartedByCall(true);
    abilityRecord->SetStartToBackground(true);
    abilityRecord->isReady_ = false;
    abilityRecord2->currentState_ = AbilityState::FOREGROUND;
    missionListManager->terminateAbilityList_.push_back(abilityRecord2);
    missionListManager->CompleteBackground(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verify TerminateAbility
 */
HWTEST_F(MissionListManagerTest, TerminateAbility_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetTerminatingState();
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    int resultCode = 0;
    Want* resultWant = nullptr;
    bool flag = true;
    int res = missionListManager->TerminateAbility(abilityRecord, resultCode, resultWant, flag);
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verify TerminateAbility
 */
HWTEST_F(MissionListManagerTest, TerminateAbility_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetTerminatingState();
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    int resultCode = 0;
    Want* resultWant = nullptr;
    bool flag = true;
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.clear();
    missionListManager->terminateAbilityList_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    int res = missionListManager->TerminateAbility(abilityRecord, resultCode, resultWant, flag);
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verify TerminateAbility
 */
HWTEST_F(MissionListManagerTest, TerminateAbility_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->isTerminating_ = false;
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    int resultCode = 0;
    Want* resultWant = nullptr;
    bool flag = true;
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    int res = missionListManager->TerminateAbility(abilityRecord, resultCode, resultWant, flag);
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verify TerminateAbility
 */
HWTEST_F(MissionListManagerTest, TerminateAbility_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    int resultCode = 0;
    int res = missionListManager->TerminateAbility(abilityRecord, resultCode);
    EXPECT_EQ(res, NO_FOUND_ABILITY_BY_CALLER);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: RemoveTerminatingAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager RemoveTerminatingAbility
 * EnvConditions: NA
 * CaseDescription: Verify RemoveTerminatingAbility
 */
HWTEST_F(MissionListManagerTest, RemoveTerminatingAbility_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    bool flag = true;
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    missionListManager->RemoveTerminatingAbility(abilityRecord, flag);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: RemoveTerminatingAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager RemoveTerminatingAbility
 * EnvConditions: NA
 * CaseDescription: Verify RemoveTerminatingAbility
 */
HWTEST_F(MissionListManagerTest, RemoveTerminatingAbility_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    bool flag = true;
    abilityRecord->SetMissionList(missionList);
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    missionListManager->terminateAbilityList_.clear();
    missionListManager->RemoveTerminatingAbility(abilityRecord, flag);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: RemoveTerminatingAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager RemoveTerminatingAbility
 * EnvConditions: NA
 * CaseDescription: Verify RemoveTerminatingAbility
 */
HWTEST_F(MissionListManagerTest, RemoveTerminatingAbility_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord2, "missionName");
    bool flag = false;
    missionList->missions_.push_back(mission);
    abilityRecord->SetMissionList(missionList);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    missionListManager->terminateAbilityList_.clear();
    missionListManager->RemoveTerminatingAbility(abilityRecord, flag);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: RemoveTerminatingAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager RemoveTerminatingAbility
 * EnvConditions: NA
 * CaseDescription: Verify RemoveTerminatingAbility
 */
HWTEST_F(MissionListManagerTest, RemoveTerminatingAbility_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    bool flag = true;
    missionList->missions_.clear();
    abilityRecord->SetMissionList(missionList);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);
    missionListManager->terminateAbilityList_.clear();
    missionListManager->RemoveTerminatingAbility(abilityRecord, flag);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: RemoveTerminatingAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager RemoveTerminatingAbility
 * EnvConditions: NA
 * CaseDescription: Verify RemoveTerminatingAbility
 */
HWTEST_F(MissionListManagerTest, RemoveTerminatingAbility_005, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    Want want;
    want.SetElementName(AbilityConfig::LAUNCHER_BUNDLE_NAME, AbilityConfig::LAUNCHER_RECENT_ABILITY_NAME);
    abilityRecord2->SetWant(want);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord2, "missionName");
    bool flag = true;
    missionList->missions_.push_back(mission);
    abilityRecord->SetMissionList(missionList);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);
    missionListManager->terminateAbilityList_.clear();
    missionListManager->launcherList_ = missionList;
    missionListManager->RemoveTerminatingAbility(abilityRecord, flag);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: RemoveTerminatingAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager RemoveTerminatingAbility
 * EnvConditions: NA
 * CaseDescription: Verify RemoveTerminatingAbility
 */
HWTEST_F(MissionListManagerTest, RemoveTerminatingAbility_006, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    Want want;
    want.SetElementName(AbilityConfig::LAUNCHER_BUNDLE_NAME, "abilityName");
    abilityRecord2->SetWant(want);
    abilityRecord2->SetAbilityState(AbilityState::BACKGROUND);
    abilityRecord2->minimizeReason_ = false;
    abilityRecord2->isReady_ = true;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord2, "missionName");
    bool flag = true;
    missionList->missions_.push_back(mission);
    abilityRecord->SetMissionList(missionList);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);
    missionListManager->terminateAbilityList_.clear();
    missionListManager->launcherList_ = missionList;
    missionListManager->RemoveTerminatingAbility(abilityRecord, flag);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: RemoveTerminatingAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager RemoveTerminatingAbility
 * EnvConditions: NA
 * CaseDescription: Verify RemoveTerminatingAbility
 */
HWTEST_F(MissionListManagerTest, RemoveTerminatingAbility_007, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    Want want;
    want.SetElementName("bundleName", AbilityConfig::LAUNCHER_RECENT_ABILITY_NAME);
    abilityRecord2->SetWant(want);
    abilityRecord2->SetAbilityState(AbilityState::FOREGROUND);
    abilityRecord2->minimizeReason_ = false;
    abilityRecord2->isReady_ = true;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord2, "missionName");
    bool flag = true;
    missionList->missions_.push_back(mission);
    abilityRecord->SetMissionList(missionList);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);
    missionListManager->terminateAbilityList_.clear();
    missionListManager->launcherList_ = missionList;
    missionListManager->RemoveTerminatingAbility(abilityRecord, flag);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: RemoveTerminatingAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager RemoveTerminatingAbility
 * EnvConditions: NA
 * CaseDescription: Verify RemoveTerminatingAbility
 */
HWTEST_F(MissionListManagerTest, RemoveTerminatingAbility_008, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    Want want;
    want.SetElementName("bundleName", AbilityConfig::LAUNCHER_RECENT_ABILITY_NAME);
    abilityRecord2->SetWant(want);
    abilityRecord2->SetAbilityState(AbilityState::BACKGROUND);
    abilityRecord2->minimizeReason_ = true;
    abilityRecord2->isReady_ = true;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord2, "missionName");
    bool flag = true;
    missionList->missions_.push_back(mission);
    abilityRecord->SetMissionList(missionList);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);
    missionListManager->terminateAbilityList_.clear();
    missionListManager->launcherList_ = missionList;
    missionListManager->RemoveTerminatingAbility(abilityRecord, flag);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: RemoveTerminatingAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager RemoveTerminatingAbility
 * EnvConditions: NA
 * CaseDescription: Verify RemoveTerminatingAbility
 */
HWTEST_F(MissionListManagerTest, RemoveTerminatingAbility_009, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    Want want;
    want.SetElementName("bundleName", AbilityConfig::LAUNCHER_RECENT_ABILITY_NAME);
    abilityRecord2->SetWant(want);
    abilityRecord2->SetAbilityState(AbilityState::BACKGROUND);
    abilityRecord2->minimizeReason_ = false;
    abilityRecord2->isReady_ = false;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord2, "missionName");
    bool flag = true;
    missionList->missions_.push_back(mission);
    abilityRecord->SetMissionList(missionList);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUNDING);
    missionListManager->terminateAbilityList_.clear();
    missionListManager->launcherList_ = missionList;
    missionListManager->RemoveTerminatingAbility(abilityRecord, flag);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: RemoveMissionList
 * SubFunction: NA
 * FunctionPoints: MissionListManager RemoveMissionList
 * EnvConditions: NA
 * CaseDescription: Verify RemoveMissionList
 */
HWTEST_F(MissionListManagerTest, RemoveMissionList_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList1 = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.push_back(missionList1);
    missionListManager->RemoveMissionList(missionList);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: DispatchTerminate
 * SubFunction: NA
 * FunctionPoints: MissionListManager DispatchTerminate
 * EnvConditions: NA
 * CaseDescription: Verify DispatchTerminate
 */
HWTEST_F(MissionListManagerTest, DispatchTerminate_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    int res = missionListManager->DispatchTerminate(abilityRecord);
    EXPECT_EQ(res, INNER_ERR);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: DispatchTerminate
 * SubFunction: NA
 * FunctionPoints: MissionListManager DispatchTerminate
 * EnvConditions: NA
 * CaseDescription: Verify DispatchTerminate
 */
HWTEST_F(MissionListManagerTest, DispatchTerminate_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetAbilityState(AbilityState::TERMINATING);
    int res = missionListManager->DispatchTerminate(abilityRecord);
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteTerminate
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteTerminate
 * EnvConditions: NA
 * CaseDescription: Verify CompleteTerminate
 */
HWTEST_F(MissionListManagerTest, CompleteTerminate_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    missionListManager->CompleteTerminate(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteTerminate
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteTerminate
 * EnvConditions: NA
 * CaseDescription: Verify CompleteTerminate
 */
HWTEST_F(MissionListManagerTest, CompleteTerminate_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetAbilityState(AbilityState::TERMINATING);
    missionListManager->CompleteTerminate(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteTerminateAndUpdateMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteTerminateAndUpdateMission
 * EnvConditions: NA
 * CaseDescription: Verify CompleteTerminateAndUpdateMission
 */
HWTEST_F(MissionListManagerTest, CompleteTerminateAndUpdateMission_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    abilityRecord->SetAppIndex(1);
    missionListManager->terminateAbilityList_.clear();
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    missionListManager->terminateAbilityList_.push_back(abilityRecord2);
    missionListManager->CompleteTerminateAndUpdateMission(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteTerminateAndUpdateMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteTerminateAndUpdateMission
 * EnvConditions: NA
 * CaseDescription: Verify CompleteTerminateAndUpdateMission
 */
HWTEST_F(MissionListManagerTest, CompleteTerminateAndUpdateMission_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetAppIndex(0);
    abilityRecord->abilityInfo_.removeMissionAfterTerminate = true;
    missionListManager->terminateAbilityList_.clear();
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    missionListManager->CompleteTerminateAndUpdateMission(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteTerminateAndUpdateMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteTerminateAndUpdateMission
 * EnvConditions: NA
 * CaseDescription: Verify CompleteTerminateAndUpdateMission
 */
HWTEST_F(MissionListManagerTest, CompleteTerminateAndUpdateMission_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetAppIndex(0);
    abilityRecord->abilityInfo_.removeMissionAfterTerminate = false;
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    missionListManager->terminateAbilityList_.clear();
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    missionListManager->CompleteTerminateAndUpdateMission(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteTerminateAndUpdateMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteTerminateAndUpdateMission
 * EnvConditions: NA
 * CaseDescription: Verify CompleteTerminateAndUpdateMission
 */
HWTEST_F(MissionListManagerTest, CompleteTerminateAndUpdateMission_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetAppIndex(0);
    abilityRecord->abilityInfo_.removeMissionAfterTerminate = false;
    abilityRecord->abilityInfo_.excludeFromMissions = false;
    abilityRecord->missionId_ = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_[1] = true;
    InnerMissionInfo info;
    info.missionInfo.id = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->Init(userId);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.push_back(info);
    missionListManager->terminateAbilityList_.clear();
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    missionListManager->CompleteTerminateAndUpdateMission(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteTerminateAndUpdateMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteTerminateAndUpdateMission
 * EnvConditions: NA
 * CaseDescription: Verify CompleteTerminateAndUpdateMission
 */
HWTEST_F(MissionListManagerTest, CompleteTerminateAndUpdateMission_005, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetAppIndex(0);
    abilityRecord->abilityInfo_.removeMissionAfterTerminate = false;
    abilityRecord->abilityInfo_.excludeFromMissions = false;
    abilityRecord->missionId_ = 2;
    missionListManager->listenerController_ = nullptr;
    missionListManager->terminateAbilityList_.clear();
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    missionListManager->CompleteTerminateAndUpdateMission(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteTerminateAndUpdateMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteTerminateAndUpdateMission
 * EnvConditions: NA
 * CaseDescription: Verify CompleteTerminateAndUpdateMission
 */
HWTEST_F(MissionListManagerTest, CompleteTerminateAndUpdateMission_006, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetAppIndex(0);
    abilityRecord->abilityInfo_.removeMissionAfterTerminate = false;
    abilityRecord->abilityInfo_.excludeFromMissions = false;
    abilityRecord->missionId_ = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_[1] = true;
    InnerMissionInfo info;
    info.missionInfo.id = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->Init(userId);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.push_back(info);
    missionListManager->terminateAbilityList_.clear();
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    missionListManager->CompleteTerminateAndUpdateMission(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteTerminateAndUpdateMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteTerminateAndUpdateMission
 * EnvConditions: NA
 * CaseDescription: Verify CompleteTerminateAndUpdateMission
 */
HWTEST_F(MissionListManagerTest, CompleteTerminateAndUpdateMission_007, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    missionListManager->terminateAbilityList_.clear();
    missionListManager->terminateAbilityList_.push_back(abilityRecord2);
    missionListManager->CompleteTerminateAndUpdateMission(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: ClearMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearMission
 * EnvConditions: NA
 * CaseDescription: Verify ClearMission
 */
HWTEST_F(MissionListManagerTest, ClearMission_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int missionId = -1;
    int res = missionListManager->ClearMission(missionId);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: ClearMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearMission
 * EnvConditions: NA
 * CaseDescription: Verify ClearMission
 */
HWTEST_F(MissionListManagerTest, ClearMission_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int missionId = 1;
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    mission->SetMissionList(missionList);
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList);
    int res = missionListManager->ClearMission(missionId);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: ClearMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearMission
 * EnvConditions: NA
 * CaseDescription: Verify ClearMission
 */
HWTEST_F(MissionListManagerTest, ClearMission_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int missionId = 1;
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    mission->SetMissionList(missionList);
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList);
    int res = missionListManager->ClearMission(missionId);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: ClearMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearMission
 * EnvConditions: NA
 * CaseDescription: Verify ClearMission
 */
HWTEST_F(MissionListManagerTest, ClearMission_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int missionId = 1;
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, "missionName");
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    mission->SetMissionList(nullptr);
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList);
    int res = missionListManager->ClearMission(missionId);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: ClearMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearMission
 * EnvConditions: NA
 * CaseDescription: Verify ClearMission
 */
HWTEST_F(MissionListManagerTest, ClearMission_005, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int missionId = 1;
    missionListManager->Init();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->launcherList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    int res = missionListManager->ClearMission(missionId);
    EXPECT_NE(res, ERR_INVALID_VALUE);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: ClearMissionLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearMissionLocked
 * EnvConditions: NA
 * CaseDescription: Verify ClearMissionLocked
 */
HWTEST_F(MissionListManagerTest, ClearMissionLocked_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int missionId = 1;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, nullptr);
    missionListManager->listenerController_ = nullptr;
    int res = missionListManager->ClearMissionLocked(missionId, mission);
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: ClearMissionLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearMissionLocked
 * EnvConditions: NA
 * CaseDescription: Verify ClearMissionLocked
 */
HWTEST_F(MissionListManagerTest, ClearMissionLocked_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetTerminatingState();
    int missionId = -1;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    int res = missionListManager->ClearMissionLocked(missionId, mission);
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: ClearMissionLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearMissionLocked
 * EnvConditions: NA
 * CaseDescription: Verify ClearMissionLocked
 */
HWTEST_F(MissionListManagerTest, ClearMissionLocked_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->isTerminating_ = false;
    abilityRecord->currentState_ = AbilityState::ACTIVE;
    int missionId = 1;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    int res = missionListManager->ClearMissionLocked(missionId, mission);
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: ClearAllMissions
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearAllMissions
 * EnvConditions: NA
 * CaseDescription: Verify ClearAllMissions
 */
HWTEST_F(MissionListManagerTest, ClearAllMissions_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    missionListManager->currentMissionLists_.clear();
    int res = missionListManager->ClearAllMissions();
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: ClearAllMissions
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearAllMissions
 * EnvConditions: NA
 * CaseDescription: Verify ClearAllMissions
 */
HWTEST_F(MissionListManagerTest, ClearAllMissions_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<MissionList> missionList1 = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    std::shared_ptr<MissionList> missionList2 = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.clear();
    missionListManager->currentMissionLists_.push_back(nullptr);
    missionListManager->currentMissionLists_.push_back(missionList1);
    missionListManager->currentMissionLists_.push_back(missionList2);
    int res = missionListManager->ClearAllMissions();
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: ClearAllMissionsLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearAllMissionsLocked
 * EnvConditions: NA
 * CaseDescription: Verify ClearAllMissionsLocked
 */
HWTEST_F(MissionListManagerTest, ClearAllMissionsLocked_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->Init();
    std::list<std::shared_ptr<Mission>> missionList;
    std::list<std::shared_ptr<Mission>> foregroundAbilities;
    bool searchActive = true;
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord1 = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    abilityRecord1->currentState_ = AbilityState::ACTIVE;
    abilityRecord2->currentState_ = AbilityState::BACKGROUND;
    std::shared_ptr<Mission> mission1 = std::make_shared<Mission>(1, nullptr);
    std::shared_ptr<Mission> mission2 = std::make_shared<Mission>(2, abilityRecord1);
    std::shared_ptr<Mission> mission3 = std::make_shared<Mission>(3, abilityRecord2);
    mission1->SetLockedState(true);
    mission2->SetLockedState(false);
    mission3->SetLockedState(false);
    missionList.push_back(nullptr);
    missionList.push_back(mission1);
    missionList.push_back(mission2);
    missionList.push_back(mission3);
    missionListManager->ClearAllMissionsLocked(missionList, foregroundAbilities, searchActive);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: ClearAllMissionsLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager ClearAllMissionsLocked
 * EnvConditions: NA
 * CaseDescription: Verify ClearAllMissionsLocked
 */
HWTEST_F(MissionListManagerTest, ClearAllMissionsLocked_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->Init();
    std::list<std::shared_ptr<Mission>> missionList;
    std::list<std::shared_ptr<Mission>> foregroundAbilities;
    bool searchActive = false;
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->currentState_ = AbilityState::ACTIVE;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    mission->SetLockedState(false);
    missionList.push_back(mission);
    missionListManager->ClearAllMissionsLocked(missionList, foregroundAbilities, searchActive);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: SetMissionLockedState
 * SubFunction: NA
 * FunctionPoints: MissionListManager SetMissionLockedState
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionLockedState
 */
HWTEST_F(MissionListManagerTest, SetMissionLockedState_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int missionId = -1;
    bool lockedState = true;
    int res = missionListManager->SetMissionLockedState(missionId, lockedState);
    EXPECT_EQ(res, MISSION_NOT_FOUND);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: SetMissionLockedState
 * SubFunction: NA
 * FunctionPoints: MissionListManager SetMissionLockedState
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionLockedState
 */
HWTEST_F(MissionListManagerTest, SetMissionLockedState_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    int missionId = 1;
    bool lockedState = true;
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList);
    int res = missionListManager->SetMissionLockedState(missionId, lockedState);
    EXPECT_EQ(res, MISSION_NOT_FOUND);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: SetMissionLockedState
 * SubFunction: NA
 * FunctionPoints: MissionListManager SetMissionLockedState
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionLockedState
 */
HWTEST_F(MissionListManagerTest, SetMissionLockedState_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int missionId = 1;
    bool lockedState = true;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, nullptr);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_.clear();
    int res = missionListManager->SetMissionLockedState(missionId, lockedState);
    EXPECT_EQ(res, MISSION_NOT_FOUND);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: SetMissionLockedState
 * SubFunction: NA
 * FunctionPoints: MissionListManager SetMissionLockedState
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionLockedState
 */
HWTEST_F(MissionListManagerTest, SetMissionLockedState_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    int missionId = 1;
    bool lockedState = true;
    abilityRecord->abilityInfo_.excludeFromMissions = false;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_[1] = true;
    InnerMissionInfo info;
    info.missionInfo.id = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.push_back(info);
    int res = missionListManager->SetMissionLockedState(missionId, lockedState);
    EXPECT_EQ(res, ERR_OK);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: UpdateSnapShot
 * SubFunction: NA
 * FunctionPoints: MissionListManager UpdateSnapShot
 * EnvConditions: NA
 * CaseDescription: Verify UpdateSnapShot
 */
HWTEST_F(MissionListManagerTest, UpdateSnapShot_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord1 = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionListManager->terminateAbilityList_.push_back(abilityRecord2);
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->UpdateSnapShot(abilityRecord1->GetToken());
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: UpdateSnapShot
 * SubFunction: NA
 * FunctionPoints: MissionListManager UpdateSnapShot
 * EnvConditions: NA
 * CaseDescription: Verify UpdateSnapShot
 */
HWTEST_F(MissionListManagerTest, UpdateSnapShot_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    missionListManager->UpdateSnapShot(abilityRecord->GetToken());
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveToBackgroundTask
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveToBackgroundTask
 * EnvConditions: NA
 * CaseDescription: Verify MoveToBackgroundTask
 */
HWTEST_F(MissionListManagerTest, MoveToBackgroundTask_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord;
    missionListManager->MoveToBackgroundTask(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveToBackgroundTask
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveToBackgroundTask
 * EnvConditions: NA
 * CaseDescription: Verify MoveToBackgroundTask
 */
HWTEST_F(MissionListManagerTest, MoveToBackgroundTask_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->lifeCycleStateInfo_.sceneFlag = 1;
    missionListManager->MoveToBackgroundTask(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveToBackgroundTask
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveToBackgroundTask
 * EnvConditions: NA
 * CaseDescription: Verify MoveToBackgroundTask
 */
HWTEST_F(MissionListManagerTest, MoveToBackgroundTask_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->lifeCycleStateInfo_.sceneFlag = 2;
    abilityRecord->SetClearMissionFlag(true);
    missionListManager->MoveToBackgroundTask(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: NotifyMissionCreated
 * SubFunction: NA
 * FunctionPoints: MissionListManager NotifyMissionCreated
 * EnvConditions: NA
 * CaseDescription: Verify NotifyMissionCreated
 */
HWTEST_F(MissionListManagerTest, NotifyMissionCreated_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, nullptr);
    mission->needNotify_ = true;
    abilityRecord->SetMission(mission);
    abilityRecord->abilityInfo_.excludeFromMissions = false;
    missionListManager->NotifyMissionCreated(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: NotifyMissionCreated
 * SubFunction: NA
 * FunctionPoints: MissionListManager NotifyMissionCreated
 * EnvConditions: NA
 * CaseDescription: Verify NotifyMissionCreated
 */
HWTEST_F(MissionListManagerTest, NotifyMissionCreated_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, nullptr);
    mission->needNotify_ = true;
    abilityRecord->SetMission(mission);
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    missionListManager->NotifyMissionCreated(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: NotifyMissionCreated
 * SubFunction: NA
 * FunctionPoints: MissionListManager NotifyMissionCreated
 * EnvConditions: NA
 * CaseDescription: Verify NotifyMissionCreated
 */
HWTEST_F(MissionListManagerTest, NotifyMissionCreated_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->listenerController_ = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, nullptr);
    mission->needNotify_ = true;
    abilityRecord->SetMission(mission);
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    missionListManager->NotifyMissionCreated(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: NotifyMissionCreated
 * SubFunction: NA
 * FunctionPoints: MissionListManager NotifyMissionCreated
 * EnvConditions: NA
 * CaseDescription: Verify NotifyMissionCreated
 */
HWTEST_F(MissionListManagerTest, NotifyMissionCreated_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->listenerController_ = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, nullptr);
    mission->needNotify_ = false;
    abilityRecord->SetMission(mission);
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    missionListManager->NotifyMissionCreated(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: NotifyMissionCreated
 * SubFunction: NA
 * FunctionPoints: MissionListManager NotifyMissionCreated
 * EnvConditions: NA
 * CaseDescription: Verify NotifyMissionCreated
 */
HWTEST_F(MissionListManagerTest, NotifyMissionCreated_005, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->listenerController_ = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetMission(nullptr);
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    missionListManager->NotifyMissionCreated(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(MissionListManagerTest, PrintTimeOutLog_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    uint32_t msgId = 0;
    missionListManager->PrintTimeOutLog(nullptr, msgId);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(MissionListManagerTest, PrintTimeOutLog_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 0;
    missionListManager->PrintTimeOutLog(abilityRecord, msgId);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(MissionListManagerTest, PrintTimeOutLog_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 1;
    missionListManager->PrintTimeOutLog(abilityRecord, msgId);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(MissionListManagerTest, PrintTimeOutLog_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 2;
    missionListManager->PrintTimeOutLog(abilityRecord, msgId);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(MissionListManagerTest, PrintTimeOutLog_005, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 4;
    missionListManager->PrintTimeOutLog(abilityRecord, msgId);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(MissionListManagerTest, PrintTimeOutLog_006, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 5;
    missionListManager->PrintTimeOutLog(abilityRecord, msgId);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(MissionListManagerTest, PrintTimeOutLog_007, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 6;
    missionListManager->PrintTimeOutLog(abilityRecord, msgId);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PrintTimeOutLog
 * SubFunction: NA
 * FunctionPoints: MissionListManager PrintTimeOutLog
 * EnvConditions: NA
 * CaseDescription: Verify PrintTimeOutLog
 */
HWTEST_F(MissionListManagerTest, PrintTimeOutLog_008, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    uint32_t msgId = 3;
    missionListManager->PrintTimeOutLog(abilityRecord, msgId);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: UpdateMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionListManager UpdateMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify UpdateMissionSnapshot
 */
HWTEST_F(MissionListManagerTest, UpdateMissionSnapshot_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    missionListManager->UpdateMissionSnapshot(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: UpdateMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionListManager UpdateMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify UpdateMissionSnapshot
 */
HWTEST_F(MissionListManagerTest, UpdateMissionSnapshot_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    missionListManager->listenerController_ = nullptr;
    abilityRecord->abilityInfo_.excludeFromMissions = false;
    missionListManager->UpdateMissionSnapshot(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnTimeOut
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify OnTimeOut
 */
HWTEST_F(MissionListManagerTest, OnTimeOut_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    uint32_t msgId = 0;
    int64_t eventId = 0;
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->OnTimeOut(msgId, eventId);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnTimeOut
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify OnTimeOut
 */
HWTEST_F(MissionListManagerTest, OnTimeOut_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList2 = std::make_shared<MissionList>();
    uint32_t msgId = 1;
    int64_t eventId = 0;
    abilityRecord->SetEventId(eventId);
    abilityRecord->SetStartingWindow(true);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->OnTimeOut(msgId, eventId);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnTimeOut
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify OnTimeOut
 */
HWTEST_F(MissionListManagerTest, OnTimeOut_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    uint32_t msgId = 2;
    int64_t eventId = 0;
    abilityRecord->SetEventId(eventId);
    abilityRecord->SetStartingWindow(false);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->OnTimeOut(msgId, eventId);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnTimeOut
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify OnTimeOut
 */
HWTEST_F(MissionListManagerTest, OnTimeOut_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    uint32_t msgId = 3;
    int64_t eventId = 0;
    abilityRecord->SetEventId(eventId);
    abilityRecord->SetStartingWindow(false);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->OnTimeOut(msgId, eventId);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: HandleLoadTimeout
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleLoadTimeout
 * EnvConditions: NA
 * CaseDescription: Verify HandleLoadTimeout
 */
HWTEST_F(MissionListManagerTest, HandleLoadTimeout_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->HandleLoadTimeout(nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: HandleForegroundTimeout
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleForegroundTimeout
 * EnvConditions: NA
 * CaseDescription: Verify HandleForegroundTimeout
 */
HWTEST_F(MissionListManagerTest, HandleForegroundTimeout_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->HandleForegroundTimeout(nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: HandleLoadTimeout
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleLoadTimeout
 * EnvConditions: NA
 * CaseDescription: Verify HandleLoadTimeout
 */
HWTEST_F(MissionListManagerTest, HandleLoadTimeout_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    missionListManager->HandleLoadTimeout(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteForegroundFailed
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteForegroundFailed
 * EnvConditions: NA
 * CaseDescription: Verify CompleteForegroundFailed
 */
HWTEST_F(MissionListManagerTest, CompleteForegroundFailed_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    bool isInvalidMode = true;
    missionListManager->CompleteForegroundFailed(nullptr, isInvalidMode);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteForegroundFailed
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteForegroundFailed
 * EnvConditions: NA
 * CaseDescription: Verify CompleteForegroundFailed
 */
HWTEST_F(MissionListManagerTest, CompleteForegroundFailed_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    bool isInvalidMode = true;
    missionListManager->CompleteForegroundFailed(abilityRecord, isInvalidMode);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteForegroundFailed
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteForegroundFailed
 * EnvConditions: NA
 * CaseDescription: Verify CompleteForegroundFailed
 */
HWTEST_F(MissionListManagerTest, CompleteForegroundFailed_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetStartingWindow(true);
    bool isInvalidMode = false;
    missionListManager->CompleteForegroundFailed(abilityRecord, isInvalidMode);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: HandleTimeoutAndResumeAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleTimeoutAndResumeAbility
 * EnvConditions: NA
 * CaseDescription: Verify HandleTimeoutAndResumeAbility
 */
HWTEST_F(MissionListManagerTest, HandleTimeoutAndResumeAbility_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    bool isInvalidMode = true;
    missionListManager->HandleTimeoutAndResumeAbility(nullptr, isInvalidMode);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveToTerminateList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveToTerminateList
 * EnvConditions: NA
 * CaseDescription: Verify MoveToTerminateList
 */
HWTEST_F(MissionListManagerTest, MoveToTerminateList_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->MoveToTerminateList(nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveToTerminateList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveToTerminateList
 * EnvConditions: NA
 * CaseDescription: Verify MoveToTerminateList
 */
HWTEST_F(MissionListManagerTest, MoveToTerminateList_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetMissionList(nullptr);
    missionListManager->MoveToTerminateList(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveToTerminateList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveToTerminateList
 * EnvConditions: NA
 * CaseDescription: Verify MoveToTerminateList
 */
HWTEST_F(MissionListManagerTest, MoveToTerminateList_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    abilityRecord->SetMissionList(missionList);
    abilityRecord->SetMission(nullptr);
    missionListManager->MoveToTerminateList(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveToTerminateList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveToTerminateList
 * EnvConditions: NA
 * CaseDescription: Verify MoveToTerminateList
 */
HWTEST_F(MissionListManagerTest, MoveToTerminateList_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    abilityRecord->SetMissionList(missionList);
    abilityRecord->SetMission(mission);
    abilityRecord->SetAbilityState(AbilityState::INITIAL);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_[1] = true;
    InnerMissionInfo info;
    info.missionInfo.id = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.push_back(info);
    missionListManager->listenerController_ = nullptr;
    missionListManager->MoveToTerminateList(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: MoveToTerminateList
 * SubFunction: NA
 * FunctionPoints: MissionListManager MoveToTerminateList
 * EnvConditions: NA
 * CaseDescription: Verify MoveToTerminateList
 */
HWTEST_F(MissionListManagerTest, MoveToTerminateList_005, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    abilityRecord->SetMissionList(missionList);
    abilityRecord->SetMission(mission);
    abilityRecord->SetAbilityState(AbilityState::INITIAL);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_[1] = true;
    InnerMissionInfo info;
    info.missionInfo.id = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.push_back(info);
    missionListManager->MoveToTerminateList(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetAbilityRecordByCaller
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetAbilityRecordByCaller
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByCaller
 */
HWTEST_F(MissionListManagerTest, GetAbilityRecordByCaller_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int requestCode = 0;
    auto res = missionListManager->GetAbilityRecordByCaller(nullptr, requestCode);
    EXPECT_EQ(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetAbilityRecordByCaller
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetAbilityRecordByCaller
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByCaller
 */
HWTEST_F(MissionListManagerTest, GetAbilityRecordByCaller_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    int requestCode = 0;
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    auto res = missionListManager->GetAbilityRecordByCaller(abilityRecord, requestCode);
    EXPECT_EQ(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetAbilityRecordByEventId
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetAbilityRecordByEventId
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByEventId
 */
HWTEST_F(MissionListManagerTest, GetAbilityRecordByEventId_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetEventId(0);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList1 = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList2 = std::make_shared<MissionList>();
    int64_t eventId = 0;
    missionList2->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList1);
    missionListManager->defaultSingleList_ = missionList2;
    auto res = missionListManager->GetAbilityRecordByEventId(eventId);
    EXPECT_NE(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetAbilityRecordByEventId
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetAbilityRecordByEventId
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByEventId
 */
HWTEST_F(MissionListManagerTest, GetAbilityRecordByEventId_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    int64_t eventId = 0;
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    auto res = missionListManager->GetAbilityRecordByEventId(eventId);
    EXPECT_EQ(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnAbilityDied
 * EnvConditions: NA
 * CaseDescription: Verify OnAbilityDied
 */
HWTEST_F(MissionListManagerTest, OnAbilityDied_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int32_t currentUserId = 0;
    missionListManager->OnAbilityDied(nullptr, currentUserId);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnAbilityDied
 * EnvConditions: NA
 * CaseDescription: Verify OnAbilityDied
 */
HWTEST_F(MissionListManagerTest, OnAbilityDied_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    int32_t currentUserId = 0;
    missionListManager->OnAbilityDied(abilityRecord, currentUserId);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: MissionListManager OnAbilityDied
 * EnvConditions: NA
 * CaseDescription: Verify OnAbilityDied
 */
HWTEST_F(MissionListManagerTest, OnAbilityDied_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.type = AbilityType::PAGE;
    abilityRecord->SetStartingWindow(true);
    abilityRecord->isLauncherRoot_ = false;
    int32_t currentUserId = 0;
    missionListManager->OnAbilityDied(abilityRecord, currentUserId);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetTargetMissionList
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetTargetMissionList
 * EnvConditions: NA
 * CaseDescription: Verify GetTargetMissionList
 */
HWTEST_F(MissionListManagerTest, GetTargetMissionList_005, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    mission->SetMissionList(nullptr);
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList);
    int missionId = 1;
    auto res = missionListManager->GetTargetMissionList(missionId, mission);
    EXPECT_EQ(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetTargetMissionList
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetTargetMissionList
 * EnvConditions: NA
 * CaseDescription: Verify GetTargetMissionList
 */
HWTEST_F(MissionListManagerTest, GetTargetMissionList_006, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    mission->SetMissionList(missionList);
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList);
    int missionId = 1;
    auto res = missionListManager->GetTargetMissionList(missionId, mission);
    EXPECT_EQ(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetTargetMissionList
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetTargetMissionList
 * EnvConditions: NA
 * CaseDescription: Verify GetTargetMissionList
 */
HWTEST_F(MissionListManagerTest, GetTargetMissionList_007, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    mission->SetMissionList(missionList);
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList);
    int missionId = 1;
    auto res = missionListManager->GetTargetMissionList(missionId, mission);
    EXPECT_NE(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetTargetMissionList
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetTargetMissionList
 * EnvConditions: NA
 * CaseDescription: Verify GetTargetMissionList
 */
HWTEST_F(MissionListManagerTest, GetTargetMissionList_008, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<Mission> mission;
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->launcherList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    int missionId = 1;
    auto res = missionListManager->GetTargetMissionList(missionId, mission);
    EXPECT_EQ(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetTargetMissionList
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetTargetMissionList
 * EnvConditions: NA
 * CaseDescription: Verify GetTargetMissionList
 */
HWTEST_F(MissionListManagerTest, GetTargetMissionList_009, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<Mission> mission;
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    int missionId = 1;
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->launcherList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_[1] = true;
    InnerMissionInfo info;
    info.missionInfo.id = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.push_back(info);
    DelayedSingleton<AbilityManagerService>::GetInstance()->Init();
    auto res = missionListManager->GetTargetMissionList(missionId, mission);
    EXPECT_EQ(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionIdByAbilityToken
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionIdByAbilityToken
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionIdByAbilityToken
 */
HWTEST_F(MissionListManagerTest, GetMissionIdByAbilityToken_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int res = missionListManager->GetMissionIdByAbilityToken(nullptr);
    EXPECT_EQ(res, -1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionIdByAbilityToken
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionIdByAbilityToken
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionIdByAbilityToken
 */
HWTEST_F(MissionListManagerTest, GetMissionIdByAbilityToken_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    abilityRecord->SetMission(nullptr);
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    int res = missionListManager->GetMissionIdByAbilityToken(token);
    EXPECT_EQ(res, -1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionIdByAbilityToken
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionIdByAbilityToken
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionIdByAbilityToken
 */
HWTEST_F(MissionListManagerTest, GetMissionIdByAbilityToken_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    abilityRecord->SetMission(mission);
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    int res = missionListManager->GetMissionIdByAbilityToken(token);
    EXPECT_EQ(res, 1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetAbilityTokenByMissionId
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetAbilityTokenByMissionId
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityTokenByMissionId
 */
HWTEST_F(MissionListManagerTest, GetAbilityTokenByMissionId_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int missionId = 1;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList);
    auto res = missionListManager->GetAbilityTokenByMissionId(missionId);
    EXPECT_NE(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetAbilityTokenByMissionId
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetAbilityTokenByMissionId
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityTokenByMissionId
 */
HWTEST_F(MissionListManagerTest, GetAbilityTokenByMissionId_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int missionId = 1;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    auto res = missionListManager->GetAbilityTokenByMissionId(missionId);
    EXPECT_NE(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: HandleAbilityDied
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleAbilityDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleAbilityDied
 */
HWTEST_F(MissionListManagerTest, HandleAbilityDied_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.type = AbilityType::DATA;
    missionListManager->HandleAbilityDied(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: HandleAbilityDied
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleAbilityDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleAbilityDied
 */
HWTEST_F(MissionListManagerTest, HandleAbilityDied_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.type = AbilityType::PAGE;
    abilityRecord->isLauncherAbility_ = true;
    missionListManager->HandleAbilityDied(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: HandleLauncherDied
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleLauncherDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleLauncherDied
 */
HWTEST_F(MissionListManagerTest, HandleLauncherDied_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> launcherList = std::make_shared<MissionList>();
    mission->SetMissionList(missionList);
    abilityRecord->SetMission(mission);
    missionListManager->launcherList_ = launcherList;
    missionListManager->HandleLauncherDied(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: HandleLauncherDied
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleLauncherDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleLauncherDied
 */
HWTEST_F(MissionListManagerTest, HandleLauncherDied_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    mission->SetMissionList(missionList);
    abilityRecord->SetMission(mission);
    abilityRecord->SetLauncherRoot();
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    missionListManager->launcherList_ = missionList;
    missionListManager->HandleLauncherDied(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: HandleLauncherDied
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleLauncherDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleLauncherDied
 */
HWTEST_F(MissionListManagerTest, HandleLauncherDied_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    mission->SetMissionList(missionList);
    abilityRecord->SetMission(mission);
    abilityRecord->isLauncherRoot_ = false;
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    missionListManager->launcherList_ = missionList;
    missionListManager->HandleLauncherDied(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: HandleAbilityDiedByDefault
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleAbilityDiedByDefault
 * EnvConditions: NA
 * CaseDescription: Verify HandleAbilityDiedByDefault
 */
HWTEST_F(MissionListManagerTest, HandleAbilityDiedByDefault_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetTerminatingState();
    missionListManager->HandleAbilityDiedByDefault(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: HandleAbilityDiedByDefault
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleAbilityDiedByDefault
 * EnvConditions: NA
 * CaseDescription: Verify HandleAbilityDiedByDefault
 */
HWTEST_F(MissionListManagerTest, HandleAbilityDiedByDefault_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord2 = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<Mission> mission2 = std::make_shared<Mission>(2, abilityRecord2);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList2 = std::make_shared<MissionList>();
    abilityRecord2->SetLauncherRoot();
    mission->SetMissionList(missionList);
    missionList2->missions_.push_back(mission);
    abilityRecord->SetMission(mission);
    abilityRecord->SetAppIndex(1);
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    abilityRecord->isUninstall_ = false;
    abilityRecord->isTerminating_ = false;
    missionListManager->launcherList_ = missionList2;
    missionListManager->HandleAbilityDiedByDefault(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: HandleAbilityDiedByDefault
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleAbilityDiedByDefault
 * EnvConditions: NA
 * CaseDescription: Verify HandleAbilityDiedByDefault
 */
HWTEST_F(MissionListManagerTest, HandleAbilityDiedByDefault_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList2 = std::make_shared<MissionList>();
    mission->SetMissionList(missionList);
    abilityRecord->SetMission(mission);
    abilityRecord->SetAppIndex(0);
    abilityRecord->abilityInfo_.removeMissionAfterTerminate = true;
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    abilityRecord->isUninstall_ = false;
    abilityRecord->isTerminating_ = false;
    missionListManager->launcherList_ = missionList2;
    missionListManager->HandleAbilityDiedByDefault(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: HandleAbilityDiedByDefault
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleAbilityDiedByDefault
 * EnvConditions: NA
 * CaseDescription: Verify HandleAbilityDiedByDefault
 */
HWTEST_F(MissionListManagerTest, HandleAbilityDiedByDefault_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList2 = std::make_shared<MissionList>();
    mission->SetMissionList(missionList);
    abilityRecord->SetMission(mission);
    abilityRecord->SetAppIndex(0);
    abilityRecord->abilityInfo_.removeMissionAfterTerminate = false;
    abilityRecord->abilityInfo_.excludeFromMissions = true;
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    abilityRecord->isUninstall_ = false;
    abilityRecord->isTerminating_ = false;
    missionListManager->launcherList_ = missionList2;
    missionListManager->HandleAbilityDiedByDefault(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: HandleAbilityDiedByDefault
 * SubFunction: NA
 * FunctionPoints: MissionListManager HandleAbilityDiedByDefault
 * EnvConditions: NA
 * CaseDescription: Verify HandleAbilityDiedByDefault
 */
HWTEST_F(MissionListManagerTest, HandleAbilityDiedByDefault_005, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList2 = std::make_shared<MissionList>();
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_[1] = true;
    InnerMissionInfo info;
    info.missionInfo.id = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.push_back(info);
    mission->SetMissionList(missionList);
    abilityRecord->SetMission(mission);
    abilityRecord->SetAppIndex(0);
    abilityRecord->abilityInfo_.removeMissionAfterTerminate = false;
    abilityRecord->abilityInfo_.excludeFromMissions = false;
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    abilityRecord->isUninstall_ = false;
    abilityRecord->isTerminating_ = false;
    missionListManager->launcherList_ = missionList2;
    missionListManager->listenerController_ = nullptr;
    missionListManager->HandleAbilityDiedByDefault(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: BackToLauncher
 * SubFunction: NA
 * FunctionPoints: MissionListManager BackToLauncher
 * EnvConditions: NA
 * CaseDescription: Verify BackToLauncher
 */
HWTEST_F(MissionListManagerTest, BackToLauncher_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionListManager->launcherList_ = missionList;
    missionListManager->BackToLauncher();
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: BackToLauncher
 * SubFunction: NA
 * FunctionPoints: MissionListManager BackToLauncher
 * EnvConditions: NA
 * CaseDescription: Verify BackToLauncher
 */
HWTEST_F(MissionListManagerTest, BackToLauncher_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetLauncherRoot();
    abilityRecord->SetMission(nullptr);
    abilityRecord->abilityInfo_.bundleName = AbilityConfig::LAUNCHER_BUNDLE_NAME;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_back(mission);
    missionListManager->launcherList_ = missionList;
    missionListManager->BackToLauncher();
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: BackToLauncher
 * SubFunction: NA
 * FunctionPoints: MissionListManager BackToLauncher
 * EnvConditions: NA
 * CaseDescription: Verify BackToLauncher
 */
HWTEST_F(MissionListManagerTest, BackToLauncher_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    abilityRecord->SetLauncherRoot();
    abilityRecord->SetMission(mission);
    abilityRecord->abilityInfo_.bundleName = AbilityConfig::LAUNCHER_BUNDLE_NAME;
    std::shared_ptr<Mission> mission2 = std::make_shared<Mission>(2, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_back(mission2);
    missionListManager->launcherList_ = missionList;
    missionListManager->BackToLauncher();
    missionListManager.reset();
}

#ifdef SUPPORT_GRAPHICS
/*
 * Feature: MissionListManager
 * Function: SetMissionLabel
 * SubFunction: NA
 * FunctionPoints: MissionListManager SetMissionLabel
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionLabel
 */
HWTEST_F(MissionListManagerTest, SetMissionLabel_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(0, abilityRecord);
    abilityRecord->SetMission(mission);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    std::string label = "label";
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    int res = missionListManager->SetMissionLabel(token, label);
    EXPECT_EQ(res, -1);
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
HWTEST_F(MissionListManagerTest, SetMissionLabel_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    InnerMissionInfo info;
    info.missionInfo.id = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->Init(userId);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.push_back(info);
    mission->needNotify_ = true;
    abilityRecord->SetMission(mission);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    std::string label = "label";
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    int res = missionListManager->SetMissionLabel(token, label);
    EXPECT_EQ(res, 0);
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
HWTEST_F(MissionListManagerTest, SetMissionLabel_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->listenerController_ = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    InnerMissionInfo info;
    info.missionInfo.id = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->Init(userId);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.push_back(info);
    mission->needNotify_ = false;
    abilityRecord->SetMission(mission);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    std::string label = "label";
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    int res = missionListManager->SetMissionLabel(token, label);
    EXPECT_EQ(res, 0);
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
HWTEST_F(MissionListManagerTest, SetMissionLabel_005, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->listenerController_ = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->taskDataPersistenceMgr_ = nullptr;
    mission->needNotify_ = false;
    abilityRecord->SetMission(mission);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    std::string label = "label";
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    int res = missionListManager->SetMissionLabel(token, label);
    EXPECT_EQ(res, -1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: SetMissionIcon
 * SubFunction: NA
 * FunctionPoints: MissionListManager SetMissionIcon
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionIcon
 */
HWTEST_F(MissionListManagerTest, SetMissionIcon_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<Media::PixelMap> icon;
    int res = missionListManager->SetMissionIcon(nullptr, icon);
    EXPECT_EQ(res, -1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: SetMissionIcon
 * SubFunction: NA
 * FunctionPoints: MissionListManager SetMissionIcon
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionIcon
 */
HWTEST_F(MissionListManagerTest, SetMissionIcon_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<Media::PixelMap> icon;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(0, abilityRecord);
    abilityRecord->SetMission(mission);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    int res = missionListManager->SetMissionIcon(token, icon);
    EXPECT_EQ(res, -1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: SetMissionIcon
 * SubFunction: NA
 * FunctionPoints: MissionListManager SetMissionIcon
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionIcon
 */
HWTEST_F(MissionListManagerTest, SetMissionIcon_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<Media::PixelMap> icon;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    abilityRecord->SetMission(mission);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    int res = missionListManager->SetMissionIcon(token, icon);
    EXPECT_EQ(res, 0);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: SetMissionIcon
 * SubFunction: NA
 * FunctionPoints: MissionListManager SetMissionIcon
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionIcon
 */
HWTEST_F(MissionListManagerTest, SetMissionIcon_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<Media::PixelMap> icon;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    abilityRecord->SetMission(mission);
    abilityRecord->abilityInfo_.excludeFromMissions = false;
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    int res = missionListManager->SetMissionIcon(token, icon);
    EXPECT_EQ(res, 0);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteFirstFrameDrawing
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteFirstFrameDrawing
 * EnvConditions: NA
 * CaseDescription: Verify CompleteFirstFrameDrawing
 */
HWTEST_F(MissionListManagerTest, CompleteFirstFrameDrawing_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->CompleteFirstFrameDrawing(nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteFirstFrameDrawing
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteFirstFrameDrawing
 * EnvConditions: NA
 * CaseDescription: Verify CompleteFirstFrameDrawing
 */
HWTEST_F(MissionListManagerTest, CompleteFirstFrameDrawing_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IRemoteObject> abilityToken = abilityRecord->GetToken();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionListManager->terminateAbilityList_.clear();
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->CompleteFirstFrameDrawing(abilityToken);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteFirstFrameDrawing
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteFirstFrameDrawing
 * EnvConditions: NA
 * CaseDescription: Verify CompleteFirstFrameDrawing
 */
HWTEST_F(MissionListManagerTest, CompleteFirstFrameDrawing_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IRemoteObject> abilityToken = abilityRecord->GetToken();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    DelayedSingleton<AbilityManagerService>::GetInstance()->handler_ = nullptr;
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    missionListManager->CompleteFirstFrameDrawing(abilityToken);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: CompleteFirstFrameDrawing
 * SubFunction: NA
 * FunctionPoints: MissionListManager CompleteFirstFrameDrawing
 * EnvConditions: NA
 * CaseDescription: Verify CompleteFirstFrameDrawing
 */
HWTEST_F(MissionListManagerTest, CompleteFirstFrameDrawing_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IRemoteObject> abilityToken = abilityRecord->GetToken();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    DelayedSingleton<AbilityManagerService>::GetInstance()->Init();
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    missionListManager->CompleteFirstFrameDrawing(abilityToken);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetCancelStartingWindowTask
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetCancelStartingWindowTask
 * EnvConditions: NA
 * CaseDescription: Verify GetCancelStartingWindowTask
 */
HWTEST_F(MissionListManagerTest, GetCancelStartingWindowTask_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    DelayedSingleton<AbilityManagerService>::GetInstance()->wmsHandler_ = nullptr;
    missionListManager->GetCancelStartingWindowTask(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PostCancelStartingWindowTask
 * SubFunction: NA
 * FunctionPoints: MissionListManager PostCancelStartingWindowTask
 * EnvConditions: NA
 * CaseDescription: Verify PostCancelStartingWindowTask
 */
HWTEST_F(MissionListManagerTest, PostCancelStartingWindowTask_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    DelayedSingleton<AbilityManagerService>::GetInstance()->handler_ = nullptr;
    missionListManager->PostCancelStartingWindowTask(abilityRecord);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PostCancelStartingWindowTask
 * SubFunction: NA
 * FunctionPoints: MissionListManager PostCancelStartingWindowTask
 * EnvConditions: NA
 * CaseDescription: Verify PostCancelStartingWindowTask
 */
HWTEST_F(MissionListManagerTest, PostCancelStartingWindowTask_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    DelayedSingleton<AbilityManagerService>::GetInstance()->Init();
    missionListManager->PostCancelStartingWindowTask(abilityRecord);
    missionListManager.reset();
}
#endif

/*
 * Feature: MissionListManager
 * Function: Dump
 * SubFunction: NA
 * FunctionPoints: MissionListManager Dump
 * EnvConditions: NA
 * CaseDescription: Verify Dump
 */
HWTEST_F(MissionListManagerTest, Dump_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::vector<std::string> info;
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->launcherList_ = missionList;
    missionListManager->Dump(info);
    missionListManager->currentMissionLists_.push_back(nullptr);
    missionListManager->Dump(info);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: DumpMissionListByRecordId
 * SubFunction: NA
 * FunctionPoints: MissionListManager DumpMissionListByRecordId
 * EnvConditions: NA
 * CaseDescription: Verify DumpMissionListByRecordId
 */
HWTEST_F(MissionListManagerTest, DumpMissionListByRecordId_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::vector<std::string> info;
    bool isClient = true;
    int32_t abilityRecordId = 0;
    std::vector<std::string> params;
    missionListManager->currentMissionLists_.clear();
    missionListManager->Init();
    missionListManager->DumpMissionListByRecordId(info, isClient, abilityRecordId, params);
    missionListManager->currentMissionLists_.push_back(nullptr);
    missionListManager->DumpMissionListByRecordId(info, isClient, abilityRecordId, params);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: DumpMissionList
 * SubFunction: NA
 * FunctionPoints: MissionListManager DumpMissionList
 * EnvConditions: NA
 * CaseDescription: Verify DumpMissionList
 */
HWTEST_F(MissionListManagerTest, DumpMissionList_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "args";
    missionListManager->DumpMissionList(info, isClient, args);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: DumpMissionList
 * SubFunction: NA
 * FunctionPoints: MissionListManager DumpMissionList
 * EnvConditions: NA
 * CaseDescription: Verify DumpMissionList
 */
HWTEST_F(MissionListManagerTest, DumpMissionList_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "";
    missionListManager->currentMissionLists_.clear();
    missionListManager->DumpMissionList(info, isClient, args);
    missionListManager->currentMissionLists_.push_back(nullptr);
    missionListManager->DumpMissionList(info, isClient, args);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: DumpMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager DumpMission
 * EnvConditions: NA
 * CaseDescription: Verify DumpMission
 */
HWTEST_F(MissionListManagerTest, DumpMission_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int missionId = 0;
    std::vector<std::string> info;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_.clear();
    missionListManager->DumpMission(missionId, info);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: DumpMission
 * SubFunction: NA
 * FunctionPoints: MissionListManager DumpMission
 * EnvConditions: NA
 * CaseDescription: Verify DumpMission
 */
HWTEST_F(MissionListManagerTest, DumpMission_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int missionId = 1;
    std::vector<std::string> info;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionIdMap_[1] = true;
    InnerMissionInfo innerMissionInfo;
    innerMissionInfo.missionInfo.id = 1;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->missionInfoList_.push_back(innerMissionInfo);
    missionListManager->DumpMission(missionId, info);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetAbilityRecordByName
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetAbilityRecordByName
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByName
 */
HWTEST_F(MissionListManagerTest, GetAbilityRecordByName_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::string deviceId = "deviceId";
    std::string bundleName = "bundleName";
    std::string name  = "name";
    ElementName element(deviceId, bundleName, name);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.deviceId = deviceId;
    abilityRecord->abilityInfo_.bundleName = bundleName;
    abilityRecord->abilityInfo_.name = name;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(nullptr);
    missionListManager->launcherList_ = missionList;
    auto res = missionListManager->GetAbilityRecordByName(element);
    EXPECT_NE(res, nullptr);
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
HWTEST_F(MissionListManagerTest, OnAcceptWantResponse_006, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    Want want;
    std::string flag = "";
    missionListManager->OnAcceptWantResponse(want, flag);
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
HWTEST_F(MissionListManagerTest, OnAcceptWantResponse_007, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    Want want;
    std::string flag = "flag";
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->launcherList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->OnAcceptWantResponse(want, flag);
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
HWTEST_F(MissionListManagerTest, OnAcceptWantResponse_008, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    Want want;
    std::string flag = "flag";
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    want.SetElementName(bundleName, abilityName);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.name = abilityName;
    abilityRecord->applicationInfo_.bundleName = bundleName;
    abilityRecord->SetSpecifiedFlag(flag);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList);
    missionListManager->OnAcceptWantResponse(want, flag);
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
HWTEST_F(MissionListManagerTest, GetMissionBySpecifiedFlag_005, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    Want want;
    std::string flag = "flag";
    missionListManager->currentMissionLists_.push_back(missionList);
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->launcherList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    auto res = missionListManager->GetMissionBySpecifiedFlag(want, flag);
    EXPECT_EQ(res, nullptr);
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
HWTEST_F(MissionListManagerTest, GetMissionBySpecifiedFlag_006, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList1 = std::make_shared<MissionList>();
    Want want;
    std::string flag = "flag";
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    want.SetElementName(bundleName, abilityName);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.name = abilityName;
    abilityRecord->applicationInfo_.bundleName = bundleName;
    abilityRecord->SetSpecifiedFlag(flag);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList2 = std::make_shared<MissionList>();
    missionList2->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(nullptr);
    missionListManager->defaultSingleList_ = missionList1;
    missionListManager->launcherList_ = missionList2;
    auto res = missionListManager->GetMissionBySpecifiedFlag(want, flag);
    EXPECT_NE(res, nullptr);
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
HWTEST_F(MissionListManagerTest, GetMissionBySpecifiedFlag_007, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    Want want;
    std::string flag = "flag";
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->launcherList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    auto res = missionListManager->GetMissionBySpecifiedFlag(want, flag);
    EXPECT_EQ(res, nullptr);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: IsReachToLimitLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager IsReachToLimitLocked
 * EnvConditions: NA
 * CaseDescription: Verify IsReachToLimitLocked
 */
HWTEST_F(MissionListManagerTest, IsReachToLimitLocked_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    missionListManager->Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->abilityInfo_.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    std::string missionName = "#::";
    std::string flag = "flag";
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord, missionName);
    mission->SetSpecifiedFlag(flag);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission);
    missionListManager->launcherList_ = missionList;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    abilityRequest.abilityInfo.applicationInfo.isLauncherApp = true;
    abilityRequest.specifiedFlag = flag;
    bool res = missionListManager->IsReachToLimitLocked(abilityRequest);
    EXPECT_FALSE(res);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: IsReachToLimitLocked
 * SubFunction: NA
 * FunctionPoints: MissionListManager IsReachToLimitLocked
 * EnvConditions: NA
 * CaseDescription: Verify IsReachToLimitLocked
 */
HWTEST_F(MissionListManagerTest, IsReachToLimitLocked_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(nullptr);
    missionListManager->currentMissionLists_.push_back(missionList);
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->defaultSingleList_ = missionList;
    bool res = missionListManager->IsReachToLimitLocked(abilityRequest);
    EXPECT_FALSE(res);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionSnapshot
 */
HWTEST_F(MissionListManagerTest, GetMissionSnapshot_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int32_t missionId = 1;
    MissionSnapshot missionSnapshot;
    bool isLowResolution = true;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    bool res = missionListManager->GetMissionSnapshot(missionId, abilityRecord->GetToken(), missionSnapshot, isLowResolution);
    EXPECT_FALSE(res);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionSnapshot
 */
HWTEST_F(MissionListManagerTest, GetMissionSnapshot_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int32_t missionId = 1;
    MissionSnapshot missionSnapshot;
    bool isLowResolution = true;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    bool res = missionListManager->GetMissionSnapshot(missionId, abilityRecord->GetToken(), missionSnapshot, isLowResolution);
    EXPECT_FALSE(res);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionSnapshot
 */
HWTEST_F(MissionListManagerTest, GetMissionSnapshot_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int32_t missionId = 1;
    MissionSnapshot missionSnapshot;
    bool isLowResolution = true;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionListManager->terminateAbilityList_.clear();
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    bool res = missionListManager->GetMissionSnapshot(missionId, abilityRecord->GetToken(), missionSnapshot, isLowResolution);
    EXPECT_FALSE(res);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetAbilityRunningInfos
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRunningInfos
 */
HWTEST_F(MissionListManagerTest, GetAbilityRunningInfos_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::vector<AbilityRunningInfo> info;
    bool isPerm = true;
    missionListManager->currentMissionLists_.push_back(missionList);
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->GetAbilityRunningInfos(info, isPerm);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetAbilityRunningInfos
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRunningInfos
 */
HWTEST_F(MissionListManagerTest, GetAbilityRunningInfos_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::vector<AbilityRunningInfo> info;
    bool isPerm = true;
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList);
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->GetAbilityRunningInfos(info, isPerm);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: AddUninstallTags
 * SubFunction: NA
 * FunctionPoints: MissionListManager AddUninstallTags
 * EnvConditions: NA
 * CaseDescription: Verify AddUninstallTags
 */
HWTEST_F(MissionListManagerTest, AddUninstallTags_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::string bundleName = "bundleName";
    int32_t uid = 0;
    missionListManager->Init();
    missionListManager->currentMissionLists_.push_back(missionList);
    missionListManager->AddUninstallTags(bundleName, uid);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: AddUninstallTags
 * SubFunction: NA
 * FunctionPoints: MissionListManager AddUninstallTags
 * EnvConditions: NA
 * CaseDescription: Verify AddUninstallTags
 */
HWTEST_F(MissionListManagerTest, AddUninstallTags_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    missionList->missions_.push_back(mission);
    std::string bundleName = "bundleName";
    int32_t uid = 0;
    missionListManager->Init();
    missionListManager->listenerController_ = nullptr;
    missionListManager->currentMissionLists_.push_back(missionList);
    missionListManager->AddUninstallTags(bundleName, uid);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: AddUninstallTags
 * SubFunction: NA
 * FunctionPoints: MissionListManager AddUninstallTags
 * EnvConditions: NA
 * CaseDescription: Verify AddUninstallTags
 */
HWTEST_F(MissionListManagerTest, AddUninstallTags_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::string bundleName = "bundleName";
    int32_t uid = 0;
    missionListManager->Init();
    missionListManager->currentMissionLists_.clear();
    missionListManager->AddUninstallTags(bundleName, uid);
    missionListManager->currentMissionLists_.push_back(nullptr);
    missionListManager->AddUninstallTags(bundleName, uid);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: EraseWaitingAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager EraseWaitingAbility
 * EnvConditions: NA
 * CaseDescription: Verify EraseWaitingAbility
 */
HWTEST_F(MissionListManagerTest, EraseWaitingAbility_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    missionList->missions_.push_back(mission);
    std::string bundleName = "bundleName";
    int32_t uid = 0;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = bundleName;
    abilityRequest.uid = uid;
    missionListManager->waitingAbilityQueue_.push(abilityRequest);
    missionListManager->EraseWaitingAbility(bundleName, uid);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: EraseWaitingAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager EraseWaitingAbility
 * EnvConditions: NA
 * CaseDescription: Verify EraseWaitingAbility
 */
HWTEST_F(MissionListManagerTest, EraseWaitingAbility_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    missionList->missions_.push_back(mission);
    std::string bundleName = "bundleName";
    int32_t uid = 0;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = bundleName;
    abilityRequest.uid = 1;
    missionListManager->waitingAbilityQueue_.push(abilityRequest);
    missionListManager->EraseWaitingAbility(bundleName, uid);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: EraseWaitingAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager EraseWaitingAbility
 * EnvConditions: NA
 * CaseDescription: Verify EraseWaitingAbility
 */
HWTEST_F(MissionListManagerTest, EraseWaitingAbility_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    missionList->missions_.push_back(mission);
    std::string bundleName = "bundleName";
    int32_t uid = 0;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "";
    abilityRequest.uid = 1;
    missionListManager->waitingAbilityQueue_.push(abilityRequest);
    missionListManager->EraseWaitingAbility(bundleName, uid);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PauseManager
 * SubFunction: NA
 * FunctionPoints: MissionListManager PauseManager
 * EnvConditions: NA
 * CaseDescription: Verify PauseManager
 */
HWTEST_F(MissionListManagerTest, PauseManager_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    missionList->missions_.push_back(mission);
    missionListManager->currentMissionLists_.push_back(missionList);
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->PauseManager();
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: PauseManager
 * SubFunction: NA
 * FunctionPoints: MissionListManager PauseManager
 * EnvConditions: NA
 * CaseDescription: Verify PauseManager
 */
HWTEST_F(MissionListManagerTest, PauseManager_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionList->missions_.clear();
    missionListManager->currentMissionLists_.push_back(missionList);
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->PauseManager();
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetAllForegroundAbilities
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetAllForegroundAbilities
 * EnvConditions: NA
 * CaseDescription: Verify GetAllForegroundAbilities
 */
HWTEST_F(MissionListManagerTest, GetAllForegroundAbilities_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::list<std::shared_ptr<AbilityRecord>> foregroundList;
    missionList->missions_.clear();
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->GetAllForegroundAbilities(foregroundList);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetForegroundAbilities
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetForegroundAbilities
 * EnvConditions: NA
 * CaseDescription: Verify GetForegroundAbilities
 */
HWTEST_F(MissionListManagerTest, GetForegroundAbilities_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::list<std::shared_ptr<AbilityRecord>> foregroundList;
    missionList->missions_.clear();
    missionListManager->GetForegroundAbilities(missionList, foregroundList);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: GetForegroundAbilities
 * SubFunction: NA
 * FunctionPoints: MissionListManager GetForegroundAbilities
 * EnvConditions: NA
 * CaseDescription: Verify GetForegroundAbilities
 */
HWTEST_F(MissionListManagerTest, GetForegroundAbilities_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    std::shared_ptr<Mission> mission2 = std::make_shared<Mission>(1, nullptr);
    missionList->missions_.push_back(mission);
    missionList->missions_.push_back(nullptr);
    missionList->missions_.push_back(mission2);
    std::list<std::shared_ptr<AbilityRecord>> foregroundList;
    missionList->missions_.clear();
    missionListManager->GetForegroundAbilities(missionList, foregroundList);
    missionListManager.reset();
}

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: MissionListManager
 * Function: BlockAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager BlockAbility
 * EnvConditions: NA
 * CaseDescription: Verify BlockAbility
 */
HWTEST_F(MissionListManagerTest, BlockAbility_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<MissionList> missionList2 = std::make_shared<MissionList>();
    int32_t abilityRecordId = 1;
    missionListManager->currentMissionLists_.push_back(missionList);
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->launcherList_ = missionList2;
    int res = missionListManager->BlockAbility(abilityRecordId);
    EXPECT_EQ(res, -1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: BlockAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager BlockAbility
 * EnvConditions: NA
 * CaseDescription: Verify BlockAbility
 */
HWTEST_F(MissionListManagerTest, BlockAbility_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    int32_t abilityRecordId = 1;
    missionListManager->currentMissionLists_.push_back(missionList);
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->launcherList_ = missionList;
    int res = missionListManager->BlockAbility(abilityRecordId);
    EXPECT_EQ(res, -1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: BlockAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager BlockAbility
 * EnvConditions: NA
 * CaseDescription: Verify BlockAbility
 */
HWTEST_F(MissionListManagerTest, BlockAbility_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int32_t abilityRecordId = 1;
    missionListManager->currentMissionLists_.push_back(nullptr);
    missionListManager->defaultStandardList_ = nullptr;
    missionListManager->defaultSingleList_ = nullptr;
    missionListManager->launcherList_ = nullptr;
    int res = missionListManager->BlockAbility(abilityRecordId);
    EXPECT_EQ(res, -1);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: BlockAbility
 * SubFunction: NA
 * FunctionPoints: MissionListManager BlockAbility
 * EnvConditions: NA
 * CaseDescription: Verify BlockAbility
 */
HWTEST_F(MissionListManagerTest, BlockAbility_004, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    int32_t abilityRecordId = 1;
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultStandardList_ = nullptr;
    missionListManager->defaultSingleList_ = nullptr;
    missionListManager->launcherList_ = nullptr;
    int res = missionListManager->BlockAbility(abilityRecordId);
    EXPECT_EQ(res, -1);
    missionListManager.reset();
}
#endif

/*
 * Feature: MissionListManager
 * Function: SetMissionANRStateByTokens
 * SubFunction: NA
 * FunctionPoints: MissionListManager SetMissionANRStateByTokens
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionANRStateByTokens
 */
HWTEST_F(MissionListManagerTest, SetMissionANRStateByTokens_001, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, abilityRecord);
    abilityRecord->SetMission(mission);
    std::vector<sptr<IRemoteObject>> tokens;
    tokens.push_back(abilityRecord->GetToken());
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    missionListManager->SetMissionANRStateByTokens(tokens);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: SetMissionANRStateByTokens
 * SubFunction: NA
 * FunctionPoints: MissionListManager SetMissionANRStateByTokens
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionANRStateByTokens
 */
HWTEST_F(MissionListManagerTest, SetMissionANRStateByTokens_002, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    abilityRecord->SetMission(nullptr);
    std::vector<sptr<IRemoteObject>> tokens;
    tokens.push_back(abilityRecord->GetToken());
    missionListManager->terminateAbilityList_.push_back(abilityRecord);
    missionListManager->SetMissionANRStateByTokens(tokens);
    missionListManager.reset();
}

/*
 * Feature: MissionListManager
 * Function: SetMissionANRStateByTokens
 * SubFunction: NA
 * FunctionPoints: MissionListManager SetMissionANRStateByTokens
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionANRStateByTokens
 */
HWTEST_F(MissionListManagerTest, SetMissionANRStateByTokens_003, TestSize.Level1)
{
    int userId = 3;
    auto missionListManager = std::make_shared<MissionListManager>(userId);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    std::vector<sptr<IRemoteObject>> tokens;
    tokens.push_back(abilityRecord->GetToken());
    missionListManager->terminateAbilityList_.clear();
    missionListManager->currentMissionLists_.clear();
    missionListManager->defaultSingleList_ = missionList;
    missionListManager->defaultStandardList_ = missionList;
    missionListManager->SetMissionANRStateByTokens(tokens);
    tokens.clear();
    missionListManager->SetMissionANRStateByTokens(tokens);
    missionListManager.reset();
}
}  // namespace AAFwk
}  // namespace OHOS
