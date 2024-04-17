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
#include "ability_manager_service.h"
#undef private
#undef protected

#include "ability_manager_errors.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const int32_t MOCK_MAIN_USER_ID = 100;
const int32_t MOCK_MISSION_ID = 10000;
const int32_t MOCK_U0_USER_ID = 0;
}  // namespace

class AbilityTimeoutTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static constexpr int TEST_WAIT_TIME = 100000;
};

void AbilityTimeoutTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase.";
}

void AbilityTimeoutTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase.";
}

void AbilityTimeoutTest::SetUp() {}

void AbilityTimeoutTest::TearDown() {}

/*
 * Feature: AbilityManagerService
 * Function: GetMaxRestartNum
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService GetMaxRestartNum success
 */
HWTEST_F(AbilityTimeoutTest, GetMaxRestartNum_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);

    int maxRestart = -1;
    maxRestart = AmsConfigurationParameter::GetInstance().GetMaxRestartNum(true);

    EXPECT_TRUE(maxRestart > -1);
}

/*
 * Feature: AbilityManagerService
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService OnAbilityDied success
 */
HWTEST_F(AbilityTimeoutTest, OnAbilityDied_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_TRUE(abilityMs_->subManagersHelper_->currentMissionListManager_ != nullptr);
        auto defList = abilityMs_->subManagersHelper_->currentMissionListManager_->defaultStandardList_;
        EXPECT_TRUE(defList != nullptr);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.DiedAbility001";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.name = "com.test";
        auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        auto mission =
            std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
        abilityRecord->SetMission(mission);
        abilityRecord->SetMissionList(defList);
        abilityRecord->SetOwnerMissionUserId(MOCK_MAIN_USER_ID);
        defList->AddMissionToTop(mission);
        EXPECT_TRUE(defList->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);

        abilityMs_->OnAbilityDied(abilityRecord);

        EXPECT_FALSE(defList->GetAbilityRecordByToken(abilityRecord->GetToken()) == nullptr);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: OnAbilityDied
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService OnAbilityDied success
 */
HWTEST_F(AbilityTimeoutTest, OnAbilityDied_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_TRUE(abilityMs_->subManagersHelper_->currentMissionListManager_ != nullptr);
        auto lauList = abilityMs_->subManagersHelper_->currentMissionListManager_->launcherList_;
        EXPECT_TRUE(lauList != nullptr);
        EXPECT_EQ((int)(abilityMs_->subManagersHelper_->currentMissionListManager_->currentMissionLists_.size()), 1);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.DiedAbility002";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        auto mission =
            std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
        abilityRecord->SetMission(mission);
        abilityRecord->SetMissionList(lauList);
        abilityRecord->SetLauncherRoot();
        abilityRecord->SetOwnerMissionUserId(MOCK_MAIN_USER_ID);
        lauList->AddMissionToTop(mission);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);

        abilityMs_->OnAbilityDied(abilityRecord);

        EXPECT_TRUE(lauList->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
        EXPECT_TRUE(abilityRecord->IsRestarting());
        EXPECT_TRUE(abilityRecord->restartCount_ < abilityRecord->restartMax_);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleLoadTimeOut success
 */
HWTEST_F(AbilityTimeoutTest, HandleLoadTimeOut_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_TRUE(abilityMs_->subManagersHelper_->currentMissionListManager_ != nullptr);
        auto lauList = abilityMs_->subManagersHelper_->currentMissionListManager_->launcherList_;
        EXPECT_TRUE(lauList != nullptr);
        EXPECT_EQ((int)(abilityMs_->subManagersHelper_->currentMissionListManager_->currentMissionLists_.size()), 1);

        // root launcher ability load timeout
        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.Timeout001";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        auto mission =
            std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(mission != nullptr);
        EXPECT_TRUE(abilityRecord != nullptr);
        abilityRecord->SetMission(mission);
        abilityRecord->SetMissionList(lauList);
        abilityRecord->SetLauncherRoot();
        lauList->AddMissionToTop(mission);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);

        abilityMs_->HandleLoadTimeOut(abilityRecord->GetAbilityRecordId());

        EXPECT_TRUE(lauList->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
        EXPECT_TRUE(abilityRecord->IsRestarting());
        EXPECT_TRUE(abilityRecord->restartCount_ < abilityRecord->restartMax_);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleLoadTimeOut success
 */
HWTEST_F(AbilityTimeoutTest, HandleLoadTimeOut_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = abilityMs_->subManagersHelper_->currentMissionListManager_;
        auto lauList = abilityMs_->subManagersHelper_->currentMissionListManager_->launcherList_;
        EXPECT_TRUE(curListManager != nullptr);
        EXPECT_TRUE(lauList != nullptr);
        EXPECT_EQ((int)(abilityMs_->subManagersHelper_->currentMissionListManager_->currentMissionLists_.size()), 1);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.rootLauncher";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        auto missionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(launcher != nullptr);
        EXPECT_TRUE(missionLauncher != nullptr);
        launcher->SetMission(missionLauncher);
        launcher->SetMissionList(lauList);
        launcher->SetLauncherRoot();
        lauList->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

        // common ability load timeout
        abilityRequest.appInfo.isLauncherApp = false;
        abilityRequest.abilityInfo.name = "com.test.Timeout002";
        auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        auto mission =
            std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(mission != nullptr);
        EXPECT_TRUE(abilityRecord != nullptr);

        abilityRecord->SetMission(mission);
        auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        abilityRecord->SetMissionList(missionList);
        missionList->AddMissionToTop(mission);
        curListManager->MoveMissionListToTop(missionList);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);

        abilityMs_->HandleLoadTimeOut(abilityRecord->GetAbilityRecordId());

        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) == nullptr);
        auto topAbility = curListManager->GetCurrentTopAbilityLocked();
        EXPECT_EQ(launcher, topAbility);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleLoadTimeOut success
 */
HWTEST_F(AbilityTimeoutTest, HandleLoadTimeOut_003, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = abilityMs_->subManagersHelper_->currentMissionListManager_;
        auto lauList = abilityMs_->subManagersHelper_->currentMissionListManager_->launcherList_;
        EXPECT_TRUE(curListManager != nullptr);
        EXPECT_TRUE(lauList != nullptr);
        EXPECT_EQ((int)(abilityMs_->subManagersHelper_->currentMissionListManager_->currentMissionLists_.size()), 1);
        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.rootLauncher";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        auto missionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
        launcher->SetMission(missionLauncher);
        launcher->SetMissionList(lauList);
        launcher->SetLauncherRoot();
        lauList->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);
        // common ability by caller
        abilityRequest.appInfo.isLauncherApp = false;
        abilityRequest.abilityInfo.name = "com.test.caller";
        auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
        auto callerMission =
            std::make_shared<Mission>(MOCK_MISSION_ID + 1, caller, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(caller != nullptr);
        EXPECT_TRUE(callerMission != nullptr);
        caller->SetMission(callerMission);
        auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        caller->SetMissionList(missionList);
        missionList->AddMissionToTop(callerMission);
        curListManager->MoveMissionListToTop(missionList);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(caller->GetToken()) != nullptr);
        // common ability load timeout
        abilityRequest.abilityInfo.name = "com.test.Timeout003";
        auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        auto mission =
            std::make_shared<Mission>(MOCK_MISSION_ID + 2, abilityRecord, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(abilityRecord != nullptr);
        EXPECT_TRUE(mission != nullptr);
        abilityRecord->SetMission(mission);
        abilityRecord->SetMissionList(missionList);
        abilityRecord->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(mission);
        curListManager->MoveMissionListToTop(missionList);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
        EXPECT_EQ(abilityRecord->GetCallerRecord(), caller);
        abilityMs_->HandleLoadTimeOut(abilityRecord->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) == nullptr);
        auto topAbility = curListManager->GetCurrentTopAbilityLocked();
        EXPECT_EQ(caller, topAbility);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleLoadTimeOut success
 */
HWTEST_F(AbilityTimeoutTest, HandleLoadTimeOut_004, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = abilityMs_->subManagersHelper_->currentMissionListManager_;
        auto lauList = abilityMs_->subManagersHelper_->currentMissionListManager_->launcherList_;

        EXPECT_TRUE(curListManager != nullptr);
        EXPECT_TRUE(lauList != nullptr);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.rootLauncher";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(launcher != nullptr);
        auto missionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(missionLauncher != nullptr);
        launcher->SetMission(missionLauncher);
        launcher->SetMissionList(lauList);
        launcher->SetLauncherRoot();
        lauList->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

        // common ability by caller with service ability type
        abilityRequest.appInfo.isLauncherApp = false;
        abilityRequest.abilityInfo.name = "com.test.caller";
        abilityRequest.abilityInfo.type = AbilityType::SERVICE;
        auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(caller != nullptr);

        // common ability load timeout
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.Timeout004";
        auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(launcher != nullptr);
        auto mission =
            std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
        auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        EXPECT_TRUE(mission != nullptr);
        EXPECT_TRUE(missionList != nullptr);
        abilityRecord->SetMission(mission);
        abilityRecord->SetMissionList(missionList);
        abilityRecord->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(mission);
        curListManager->MoveMissionListToTop(missionList);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
        EXPECT_EQ(abilityRecord->GetCallerRecord(), caller);

        abilityMs_->HandleLoadTimeOut(abilityRecord->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) == nullptr);
        auto topAbility = curListManager->GetCurrentTopAbilityLocked();
        EXPECT_EQ(launcher, topAbility);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleLoadTimeOut success
 */
HWTEST_F(AbilityTimeoutTest, HandleLoadTimeOut_005, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = abilityMs_->subManagersHelper_->currentMissionListManager_;
        auto lauList = abilityMs_->subManagersHelper_->currentMissionListManager_->launcherList_;

        EXPECT_TRUE(curListManager != nullptr);
        EXPECT_TRUE(lauList != nullptr);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.rootLauncher";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(launcher != nullptr);
        auto missionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(missionLauncher != nullptr);
        launcher->SetMission(missionLauncher);
        launcher->SetMissionList(lauList);
        launcher->SetLauncherRoot();
        lauList->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

        // common ability by caller with extension ability type
        abilityRequest.appInfo.isLauncherApp = false;
        abilityRequest.abilityInfo.type = AbilityType::EXTENSION;
        abilityRequest.abilityInfo.name = "com.test.caller";
        auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(caller != nullptr);

        // common ability load timeout
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.Timeout005";
        auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(launcher != nullptr);
        auto mission =
            std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
        auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        EXPECT_TRUE(mission != nullptr);
        EXPECT_TRUE(missionList != nullptr);
        abilityRecord->SetMission(mission);
        abilityRecord->SetMissionList(missionList);
        abilityRecord->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(mission);
        curListManager->MoveMissionListToTop(missionList);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
        EXPECT_EQ(abilityRecord->GetCallerRecord(), caller);

        abilityMs_->HandleLoadTimeOut(abilityRecord->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) == nullptr);
        auto topAbility = curListManager->GetCurrentTopAbilityLocked();
        EXPECT_EQ(launcher, topAbility);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleLoadTimeOut success
 */
HWTEST_F(AbilityTimeoutTest, HandleLoadTimeOut_006, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = abilityMs_->subManagersHelper_->currentMissionListManager_;
        auto lauList = abilityMs_->subManagersHelper_->currentMissionListManager_->launcherList_;

        EXPECT_TRUE(curListManager != nullptr);
        EXPECT_TRUE(lauList != nullptr);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.rootLauncher";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(launcher != nullptr);
        auto missionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(missionLauncher != nullptr);
        launcher->SetMission(missionLauncher);
            launcher->SetMissionList(lauList);
        launcher->SetLauncherRoot();
        lauList->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

        // common ability by caller as launcher type
        auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(caller != nullptr);

        // common ability load timeout
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.appInfo.isLauncherApp = false;
        abilityRequest.appInfo.name = "com.test.Timeout006";
        auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(launcher != nullptr);
        auto mission =
            std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
        auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        EXPECT_TRUE(mission != nullptr);
        EXPECT_TRUE(missionList != nullptr);
        abilityRecord->SetMission(mission);
        abilityRecord->SetMissionList(missionList);
        abilityRecord->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(mission);
        curListManager->MoveMissionListToTop(missionList);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
        EXPECT_EQ(abilityRecord->GetCallerRecord(), caller);

        abilityMs_->HandleLoadTimeOut(abilityRecord->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) == nullptr);
        auto topAbility = curListManager->GetCurrentTopAbilityLocked();
        EXPECT_EQ(launcher, topAbility);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: HandleLoadTimeOut
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleLoadTimeOut success
 */
HWTEST_F(AbilityTimeoutTest, HandleLoadTimeOut_007, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = abilityMs_->subManagersHelper_->currentMissionListManager_;
        auto lauList = abilityMs_->subManagersHelper_->currentMissionListManager_->launcherList_;

        EXPECT_TRUE(curListManager != nullptr);
        EXPECT_TRUE(lauList != nullptr);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.rootLauncher";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(launcher != nullptr);
        auto missionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(missionLauncher != nullptr);
        launcher->SetMission(missionLauncher);
        launcher->SetMissionList(lauList);
        launcher->SetLauncherRoot();
        lauList->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

        // common ability by caller
        abilityRequest.appInfo.isLauncherApp = false;
        abilityRequest.abilityInfo.name = "com.test.caller";
        auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(caller != nullptr);

        // common launcher ability load timeout
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.abilityInfo.name = "com.test.Timeout007";
        auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(launcher != nullptr);
        auto mission =
            std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
        auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        EXPECT_TRUE(mission != nullptr);
        EXPECT_TRUE(missionList != nullptr);
        abilityRecord->SetMission(mission);
        abilityRecord->SetMissionList(missionList);
        abilityRecord->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(mission);
        curListManager->MoveMissionListToTop(missionList);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
        EXPECT_EQ(abilityRecord->GetCallerRecord(), caller);

        abilityMs_->HandleLoadTimeOut(abilityRecord->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) == nullptr);
        auto topAbility = curListManager->GetCurrentTopAbilityLocked();
        EXPECT_EQ(launcher, topAbility);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: HandleForgroundNewTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleForgroundNewTimeout success
 */
HWTEST_F(AbilityTimeoutTest, HandleForgroundNewTimeout_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = abilityMs_->subManagersHelper_->currentMissionListManager_;
        auto lauList = abilityMs_->subManagersHelper_->currentMissionListManager_->launcherList_;

        EXPECT_TRUE(curListManager != nullptr);
        EXPECT_TRUE(lauList != nullptr);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.rootLauncher";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(launcher != nullptr);
        auto missionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(missionLauncher != nullptr);
        launcher->SetMission(missionLauncher);
        launcher->SetMissionList(lauList);
        launcher->SetLauncherRoot();
        lauList->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

        // test root launcher foreground timeout.
        launcher->SetAbilityState(AbilityState::FOREGROUNDING);
        abilityMs_->HandleForegroundTimeOut(launcher->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);
        auto topAbility = curListManager->GetCurrentTopAbilityLocked();
        EXPECT_EQ(launcher, topAbility);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: HandleForgroundNewTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleForgroundNewTimeout success
 */
HWTEST_F(AbilityTimeoutTest, HandleForgroundNewTimeout_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = abilityMs_->subManagersHelper_->currentMissionListManager_;
        auto lauList = abilityMs_->subManagersHelper_->currentMissionListManager_->launcherList_;

        EXPECT_TRUE(curListManager != nullptr);
        EXPECT_TRUE(lauList != nullptr);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.rootLauncher";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(launcher != nullptr);
        auto missionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(missionLauncher != nullptr);
        launcher->SetMission(missionLauncher);
        launcher->SetMissionList(lauList);
        launcher->SetLauncherRoot();
        lauList->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

        // common launcher ability timeout
        abilityRequest.abilityInfo.name = "com.test.TimeoutForeground002";
        auto commonLauncher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(commonLauncher != nullptr);
        auto commonMissionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, commonLauncher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(commonMissionLauncher != nullptr);
        commonLauncher->SetMission(commonMissionLauncher);
        commonLauncher->SetMissionList(lauList);
        lauList->AddMissionToTop(commonMissionLauncher);
        commonLauncher->SetAbilityState(AbilityState::FOREGROUNDING);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);

        // test common launcher foreground timeout.
        abilityMs_->HandleForegroundTimeOut(commonLauncher->GetAbilityRecordId());

        EXPECT_TRUE(lauList->GetAbilityRecordByToken(commonLauncher->GetToken()) == nullptr);
        auto topAbility = curListManager->GetCurrentTopAbilityLocked();
        EXPECT_EQ(launcher, topAbility);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: HandleForgroundNewTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleForgroundNewTimeout success
 */
HWTEST_F(AbilityTimeoutTest, HandleForgroundNewTimeout_003, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = abilityMs_->subManagersHelper_->currentMissionListManager_;
        auto lauList = abilityMs_->subManagersHelper_->currentMissionListManager_->launcherList_;
        EXPECT_TRUE(curListManager != nullptr);
        EXPECT_TRUE(lauList != nullptr);
        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.rootLauncher";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(launcher != nullptr);
        auto missionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(missionLauncher != nullptr);
        launcher->SetMission(missionLauncher);
        launcher->SetMissionList(lauList);
        launcher->SetLauncherRoot();
        lauList->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);
        // common ability by caller
        abilityRequest.appInfo.isLauncherApp = false;
        abilityRequest.abilityInfo.name = "com.test.caller";
        auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
        auto callerMission =
            std::make_shared<Mission>(MOCK_MISSION_ID + 1, caller, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(caller != nullptr);
        EXPECT_TRUE(callerMission != nullptr);
        caller->SetMission(callerMission);
        auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        caller->SetMissionList(missionList);
        missionList->AddMissionToTop(callerMission);
        curListManager->MoveMissionListToTop(missionList);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(caller->GetToken()) != nullptr);
        // common ability timeout
        abilityRequest.abilityInfo.name = "com.test.TimeoutForeground003";
        auto commonLauncher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(commonLauncher != nullptr);
        auto commonMissionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID + 2, commonLauncher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(commonMissionLauncher != nullptr);
        commonLauncher->SetMission(commonMissionLauncher);
        commonLauncher->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(commonMissionLauncher);
        curListManager->MoveMissionListToTop(missionList);
        commonLauncher->SetAbilityState(AbilityState::FOREGROUNDING);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
        // test common launcher foreground timeout.
        abilityMs_->HandleForegroundTimeOut(commonLauncher->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
        auto topAbility = curListManager->GetCurrentTopAbilityLocked();
        EXPECT_EQ(caller, topAbility);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: HandleForgroundNewTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleForgroundNewTimeout success
 */
HWTEST_F(AbilityTimeoutTest, HandleForgroundNewTimeout_004, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = abilityMs_->subManagersHelper_->currentMissionListManager_;
        auto lauList = abilityMs_->subManagersHelper_->currentMissionListManager_->launcherList_;

        EXPECT_TRUE(curListManager != nullptr);
        EXPECT_TRUE(lauList != nullptr);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.rootLauncher";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(launcher != nullptr);
        auto missionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(missionLauncher != nullptr);
        launcher->SetMission(missionLauncher);
        launcher->SetMissionList(lauList);
        launcher->SetLauncherRoot();
        lauList->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

        // common ability by caller (launcher type)
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.abilityInfo.name = "com.test.caller";
        auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(caller != nullptr);

        // common ability timeout
        abilityRequest.appInfo.isLauncherApp = false;
        abilityRequest.abilityInfo.name = "com.test.TimeoutForeground004";
        auto commonLauncher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(commonLauncher != nullptr);
        auto commonMissionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, commonLauncher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(commonMissionLauncher != nullptr);
        auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        commonLauncher->SetMissionList(missionList);
        commonLauncher->SetMission(commonMissionLauncher);
        commonLauncher->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(commonMissionLauncher);
        curListManager->MoveMissionListToTop(missionList);
        commonLauncher->SetAbilityState(AbilityState::FOREGROUNDING);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);

        // test common launcher foreground timeout.
        abilityMs_->HandleForegroundTimeOut(commonLauncher->GetAbilityRecordId());

        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
        auto topAbility = curListManager->GetCurrentTopAbilityLocked();
        EXPECT_EQ(launcher, topAbility);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: HandleForgroundNewTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleForgroundNewTimeout success
 */
HWTEST_F(AbilityTimeoutTest, HandleForgroundNewTimeout_005, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = abilityMs_->subManagersHelper_->currentMissionListManager_;
        auto lauList = abilityMs_->subManagersHelper_->currentMissionListManager_->launcherList_;

        EXPECT_TRUE(curListManager != nullptr);
        EXPECT_TRUE(lauList != nullptr);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.rootLauncher";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(launcher != nullptr);
        auto missionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(missionLauncher != nullptr);
        launcher->SetMission(missionLauncher);
        launcher->SetMissionList(lauList);
        launcher->SetLauncherRoot();
        lauList->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

        // common ability by service ability
        abilityRequest.appInfo.isLauncherApp = false;
        abilityRequest.abilityInfo.type = AbilityType::SERVICE;
        abilityRequest.abilityInfo.name = "com.test.caller";
        auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(caller != nullptr);

        // common ability timeout
        abilityRequest.appInfo.isLauncherApp = false;
        abilityRequest.abilityInfo.name = "com.test.TimeoutForeground005";
        auto commonLauncher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(commonLauncher != nullptr);
        auto commonMissionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, commonLauncher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(commonMissionLauncher != nullptr);
        auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        commonLauncher->SetMissionList(missionList);
        commonLauncher->SetMission(commonMissionLauncher);
        commonLauncher->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(commonMissionLauncher);
        curListManager->MoveMissionListToTop(missionList);
        commonLauncher->SetAbilityState(AbilityState::FOREGROUNDING);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);

        // test common launcher foreground timeout.
        abilityMs_->HandleForegroundTimeOut(commonLauncher->GetAbilityRecordId());

        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
        auto topAbility = curListManager->GetCurrentTopAbilityLocked();
        EXPECT_EQ(launcher, topAbility);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: HandleForgroundNewTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleForgroundNewTimeout success
 */
HWTEST_F(AbilityTimeoutTest, HandleForgroundNewTimeout_006, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = abilityMs_->subManagersHelper_->currentMissionListManager_;
        auto lauList = abilityMs_->subManagersHelper_->currentMissionListManager_->launcherList_;

        EXPECT_TRUE(curListManager != nullptr);
        EXPECT_TRUE(lauList != nullptr);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.rootLauncher";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(launcher != nullptr);
        auto missionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(missionLauncher != nullptr);
        launcher->SetMission(missionLauncher);
        launcher->SetMissionList(lauList);
        launcher->SetLauncherRoot();
        lauList->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

        // common ability by service ability
        abilityRequest.appInfo.isLauncherApp = false;
        abilityRequest.abilityInfo.type = AbilityType::EXTENSION;
        abilityRequest.abilityInfo.name = "com.test.caller";
        auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(caller != nullptr);

        // common ability timeout
        abilityRequest.appInfo.isLauncherApp = false;
        abilityRequest.abilityInfo.name = "com.test.TimeoutForeground006";
        auto commonAbility = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(commonAbility != nullptr);
        auto commonMissionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, commonAbility, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(commonMissionLauncher != nullptr);
        auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        commonAbility->SetMissionList(missionList);
        commonAbility->SetMission(commonMissionLauncher);
        commonAbility->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(commonMissionLauncher);
        curListManager->MoveMissionListToTop(missionList);
        commonAbility->SetAbilityState(AbilityState::FOREGROUNDING);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) != nullptr);

        // test common ability foreground timeout.
        abilityMs_->HandleForegroundTimeOut(commonAbility->GetAbilityRecordId());

        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) != nullptr);
        auto topAbility = curListManager->GetCurrentTopAbilityLocked();
        EXPECT_EQ(launcher, topAbility);
    }
}

/*
 * Feature: AbilityManagerService
 * Function: HandleForgroundNewTimeout
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: Verify AbilityManagerService HandleForgroundNewTimeout success
 */
HWTEST_F(AbilityTimeoutTest, HandleForgroundNewTimeout_007, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = abilityMs_->subManagersHelper_->currentMissionListManager_;
        auto lauList = abilityMs_->subManagersHelper_->currentMissionListManager_->launcherList_;

        EXPECT_TRUE(curListManager != nullptr);
        EXPECT_TRUE(lauList != nullptr);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.rootLauncher";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(launcher != nullptr);
        auto missionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(missionLauncher != nullptr);
        launcher->SetMission(missionLauncher);
        launcher->SetMissionList(lauList);
        launcher->SetLauncherRoot();
        lauList->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(lauList->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

        // common ability timeout without caller
        abilityRequest.abilityInfo.name = "com.test.TimeoutForeground007";
        auto commonLauncher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(commonLauncher != nullptr);
        auto commonMissionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, commonLauncher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(commonMissionLauncher != nullptr);
        auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        commonLauncher->SetMissionList(missionList);
        commonLauncher->SetMission(commonMissionLauncher);
        missionList->AddMissionToTop(commonMissionLauncher);
        curListManager->MoveMissionListToTop(missionList);
        commonLauncher->SetAbilityState(AbilityState::FOREGROUNDING);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);

        // test common launcher foreground timeout.
        abilityMs_->HandleForegroundTimeOut(commonLauncher->GetAbilityRecordId());

        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
        auto topAbility = curListManager->GetCurrentTopAbilityLocked();
        EXPECT_EQ(launcher, topAbility);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
