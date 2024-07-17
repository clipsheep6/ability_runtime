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
#include "mission_list_manager.h"
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
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = std::make_shared<MissionListManager>(MOCK_MAIN_USER_ID);
        EXPECT_TRUE(curListManager != nullptr);
        std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
        curListManager->defaultStandardList_ = missionList;
        EXPECT_TRUE(curListManager->defaultStandardList_ != nullptr);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.DiedAbility001";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.name = "com.test";
        auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        auto mission =
            std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
        abilityRecord->SetMissionId(mission->GetMissionId());
        abilityRecord->SetOwnerMissionUserId(MOCK_MAIN_USER_ID);
        curListManager->defaultStandardList_->AddMissionToTop(mission);
        EXPECT_TRUE(
            curListManager->defaultStandardList_->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);

        abilityMs_->OnAbilityDied(abilityRecord);
        EXPECT_FALSE(
            curListManager->defaultStandardList_->GetAbilityRecordByToken(abilityRecord->GetToken()) == nullptr);
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
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = std::make_shared<MissionListManager>(MOCK_MAIN_USER_ID);
        EXPECT_TRUE(curListManager != nullptr);
        std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
        curListManager->launcherList_ = missionList;
        EXPECT_TRUE(curListManager->launcherList_ != nullptr);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.DiedAbility002";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        auto mission =
            std::make_shared<Mission>(MOCK_MISSION_ID, abilityRecord, abilityRequest.abilityInfo.bundleName);
        abilityRecord->SetMissionId(mission->GetMissionId());
        abilityRecord->SetLauncherRoot();
        abilityRecord->SetOwnerMissionUserId(MOCK_MAIN_USER_ID);
        curListManager->launcherList_->AddMissionToTop(mission);
        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);

        abilityMs_->OnAbilityDied(abilityRecord);

        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
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
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = std::make_shared<MissionListManager>(MOCK_MAIN_USER_ID);
        EXPECT_TRUE(curListManager != nullptr);
        std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
        curListManager->launcherList_ = missionList;
        EXPECT_TRUE(curListManager->launcherList_ != nullptr);

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
        abilityRecord->SetMissionId(mission->GetMissionId());
        abilityRecord->SetLauncherRoot();
        curListManager->launcherList_->AddMissionToTop(mission);
        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);

        abilityMs_->HandleLoadTimeOut(abilityRecord->GetAbilityRecordId());

        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
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
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = std::make_shared<MissionListManager>(MOCK_MAIN_USER_ID);
        EXPECT_TRUE(curListManager != nullptr);
        std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
        curListManager->launcherList_ = missionList;
        EXPECT_TRUE(curListManager->launcherList_ != nullptr);

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
        launcher->SetMissionId(missionLauncher->GetMissionId());
        launcher->SetLauncherRoot();
        curListManager->launcherList_->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

        // common ability load timeout
        abilityRequest.appInfo.isLauncherApp = false;
        abilityRequest.abilityInfo.name = "com.test.Timeout002";
        auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        auto mission =
            std::make_shared<Mission>(MOCK_MISSION_ID + 1, abilityRecord, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(mission != nullptr);
        EXPECT_TRUE(abilityRecord != nullptr);

        abilityRecord->SetMissionId(mission->GetMissionId());
        missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        missionList->AddMissionToTop(mission);
        curListManager->MoveMissionListToTop(missionList);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);

        abilityMs_->HandleLoadTimeOut(abilityRecord->GetAbilityRecordId());

        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
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
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = std::make_shared<MissionListManager>(MOCK_MAIN_USER_ID);
        EXPECT_TRUE(curListManager != nullptr);
        std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
        curListManager->launcherList_ = missionList;
        EXPECT_TRUE(curListManager->launcherList_ != nullptr);

        AbilityRequest abilityRequest;
        abilityRequest.abilityInfo.type = AbilityType::PAGE;
        abilityRequest.abilityInfo.name = "com.test.rootLauncher";
        abilityRequest.abilityInfo.bundleName = "com.test";
        abilityRequest.appInfo.isLauncherApp = true;
        abilityRequest.appInfo.name = "com.test";
        auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        auto missionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID, launcher, abilityRequest.abilityInfo.bundleName);
        launcher->SetMissionId(missionLauncher->GetMissionId());
        launcher->SetLauncherRoot();
        curListManager->launcherList_->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);
        // common ability by caller
        abilityRequest.appInfo.isLauncherApp = false;
        abilityRequest.abilityInfo.name = "com.test.caller";
        auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
        auto callerMission =
            std::make_shared<Mission>(MOCK_MISSION_ID + 1, caller, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(caller != nullptr);
        EXPECT_TRUE(callerMission != nullptr);
        caller->SetMissionId(callerMission->GetMissionId());
        missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
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
        abilityRecord->SetMissionId(mission->GetMissionId());
        abilityRecord->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(mission);
        curListManager->MoveMissionListToTop(missionList);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
        EXPECT_EQ(abilityRecord->GetCallerRecord(), caller);
        abilityMs_->HandleLoadTimeOut(abilityRecord->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
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
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = std::make_shared<MissionListManager>(MOCK_MAIN_USER_ID);
        EXPECT_TRUE(curListManager != nullptr);
        std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
        curListManager->launcherList_ = missionList;
        EXPECT_TRUE(curListManager->launcherList_ != nullptr);

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
        launcher->SetMissionId(missionLauncher->GetMissionId());
        launcher->SetLauncherRoot();
        curListManager->launcherList_->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

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
            std::make_shared<Mission>(MOCK_MISSION_ID + 1, abilityRecord, abilityRequest.abilityInfo.bundleName);
        missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        EXPECT_TRUE(mission != nullptr);
        EXPECT_TRUE(missionList != nullptr);
        abilityRecord->SetMissionId(mission->GetMissionId());
        abilityRecord->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(mission);
        curListManager->MoveMissionListToTop(missionList);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
        EXPECT_EQ(abilityRecord->GetCallerRecord(), caller);

        abilityMs_->HandleLoadTimeOut(abilityRecord->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
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
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = std::make_shared<MissionListManager>(MOCK_MAIN_USER_ID);
        EXPECT_TRUE(curListManager != nullptr);
        std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
        curListManager->launcherList_ = missionList;
        EXPECT_TRUE(curListManager->launcherList_ != nullptr);

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
        launcher->SetMissionId(missionLauncher->GetMissionId());
        launcher->SetLauncherRoot();
        curListManager->launcherList_->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

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
            std::make_shared<Mission>(MOCK_MISSION_ID + 1, abilityRecord, abilityRequest.abilityInfo.bundleName);
        missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        EXPECT_TRUE(mission != nullptr);
        EXPECT_TRUE(missionList != nullptr);
        abilityRecord->SetMissionId(mission->GetMissionId());
        abilityRecord->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(mission);
        curListManager->MoveMissionListToTop(missionList);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
        EXPECT_EQ(abilityRecord->GetCallerRecord(), caller);

        abilityMs_->HandleLoadTimeOut(abilityRecord->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
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
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = std::make_shared<MissionListManager>(MOCK_MAIN_USER_ID);
        EXPECT_TRUE(curListManager != nullptr);
        std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
        curListManager->launcherList_ = missionList;
        EXPECT_TRUE(curListManager->launcherList_ != nullptr);

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
        launcher->SetMissionId(missionLauncher->GetMissionId());
        launcher->SetLauncherRoot();
        curListManager->launcherList_->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

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
            std::make_shared<Mission>(MOCK_MISSION_ID + 1, abilityRecord, abilityRequest.abilityInfo.bundleName);
        missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        EXPECT_TRUE(mission != nullptr);
        EXPECT_TRUE(missionList != nullptr);
        abilityRecord->SetMissionId(mission->GetMissionId());
        abilityRecord->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(mission);
        curListManager->MoveMissionListToTop(missionList);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
        EXPECT_EQ(abilityRecord->GetCallerRecord(), caller);

        abilityMs_->HandleLoadTimeOut(abilityRecord->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
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
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = std::make_shared<MissionListManager>(MOCK_MAIN_USER_ID);
        EXPECT_TRUE(curListManager != nullptr);
        std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
        curListManager->launcherList_ = missionList;
        EXPECT_TRUE(curListManager->launcherList_ != nullptr);

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
        launcher->SetMissionId(missionLauncher->GetMissionId());
        launcher->SetLauncherRoot();
        curListManager->launcherList_->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

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
            std::make_shared<Mission>(MOCK_MISSION_ID + 1, abilityRecord, abilityRequest.abilityInfo.bundleName);
        missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        EXPECT_TRUE(mission != nullptr);
        EXPECT_TRUE(missionList != nullptr);
        abilityRecord->SetMissionId(mission->GetMissionId());
        abilityRecord->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(mission);
        curListManager->MoveMissionListToTop(missionList);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
        EXPECT_EQ(abilityRecord->GetCallerRecord(), caller);

        abilityMs_->HandleLoadTimeOut(abilityRecord->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(abilityRecord->GetToken()) != nullptr);
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
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = std::make_shared<MissionListManager>(MOCK_MAIN_USER_ID);
        EXPECT_TRUE(curListManager != nullptr);
        std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
        curListManager->launcherList_ = missionList;
        EXPECT_TRUE(curListManager->launcherList_ != nullptr);

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
        launcher->SetMissionId(missionLauncher->GetMissionId());
        launcher->SetLauncherRoot();
        curListManager->launcherList_->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

        // common launcher ability timeout
        abilityRequest.abilityInfo.name = "com.test.TimeoutForeground002";
        auto commonLauncher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(commonLauncher != nullptr);
        auto commonMissionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID + 1, commonLauncher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(commonMissionLauncher != nullptr);
        commonLauncher->SetMissionId(commonMissionLauncher->GetMissionId());
        curListManager->launcherList_->AddMissionToTop(commonMissionLauncher);
        commonLauncher->SetAbilityState(AbilityState::FOREGROUNDING);
        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);

        // test common launcher foreground timeout.
        abilityMs_->HandleForegroundTimeOut(commonLauncher->GetAbilityRecordId());

        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
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
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = std::make_shared<MissionListManager>(MOCK_MAIN_USER_ID);
        EXPECT_TRUE(curListManager != nullptr);
        std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        curListManager->launcherList_ = missionList;
        EXPECT_TRUE(curListManager->launcherList_ != nullptr);

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
        launcher->SetMissionId(missionLauncher->GetMissionId());
        launcher->SetLauncherRoot();
        curListManager->launcherList_->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);
        // common ability by caller
        abilityRequest.appInfo.isLauncherApp = false;
        abilityRequest.abilityInfo.name = "com.test.caller";
        auto caller = AbilityRecord::CreateAbilityRecord(abilityRequest);
        auto callerMission =
            std::make_shared<Mission>(MOCK_MISSION_ID + 1, caller, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(caller != nullptr);
        EXPECT_TRUE(callerMission != nullptr);
        caller->SetMissionId(callerMission->GetMissionId());
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
        commonLauncher->SetMissionId(commonMissionLauncher->GetMissionId());
        commonLauncher->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(commonMissionLauncher);
        curListManager->MoveMissionListToTop(missionList);
        commonLauncher->SetAbilityState(AbilityState::FOREGROUNDING);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
        // test common launcher foreground timeout.
        abilityMs_->HandleForegroundTimeOut(commonLauncher->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
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
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = std::make_shared<MissionListManager>(MOCK_MAIN_USER_ID);
        EXPECT_TRUE(curListManager != nullptr);
        std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
        curListManager->launcherList_ = missionList;
        EXPECT_TRUE(curListManager->launcherList_ != nullptr);

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
        launcher->SetMissionId(missionLauncher->GetMissionId());
        launcher->SetLauncherRoot();
        curListManager->launcherList_->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

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
            std::make_shared<Mission>(MOCK_MISSION_ID + 1, commonLauncher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(commonMissionLauncher != nullptr);
        missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        commonLauncher->SetMissionId(commonMissionLauncher->GetMissionId());
        commonLauncher->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(commonMissionLauncher);
        curListManager->MoveMissionListToTop(missionList);
        commonLauncher->SetAbilityState(AbilityState::FOREGROUNDING);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);

        // test common launcher foreground timeout.
        abilityMs_->HandleForegroundTimeOut(commonLauncher->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
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
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = std::make_shared<MissionListManager>(MOCK_MAIN_USER_ID);
        EXPECT_TRUE(curListManager != nullptr);
        std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
        curListManager->launcherList_ = missionList;
        EXPECT_TRUE(curListManager->launcherList_ != nullptr);

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
        launcher->SetMissionId(missionLauncher->GetMissionId());
        launcher->SetLauncherRoot();
        curListManager->launcherList_->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

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
            std::make_shared<Mission>(MOCK_MISSION_ID + 1, commonLauncher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(commonMissionLauncher != nullptr);
        missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        commonLauncher->SetMissionId(commonMissionLauncher->GetMissionId());
        commonLauncher->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(commonMissionLauncher);
        curListManager->MoveMissionListToTop(missionList);
        commonLauncher->SetAbilityState(AbilityState::FOREGROUNDING);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);

        // test common launcher foreground timeout.
        abilityMs_->HandleForegroundTimeOut(commonLauncher->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
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
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = std::make_shared<MissionListManager>(MOCK_MAIN_USER_ID);
        EXPECT_TRUE(curListManager != nullptr);
        std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
        curListManager->launcherList_ = missionList;
        EXPECT_TRUE(curListManager->launcherList_ != nullptr);

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
        launcher->SetMissionId(missionLauncher->GetMissionId());
        launcher->SetLauncherRoot();
        curListManager->launcherList_->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

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
        missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        commonAbility->SetMissionId(commonMissionLauncher->GetMissionId());
        commonAbility->AddCallerRecord(caller->GetToken(), -1);
        missionList->AddMissionToTop(commonMissionLauncher);
        curListManager->MoveMissionListToTop(missionList);
        commonAbility->SetAbilityState(AbilityState::FOREGROUNDING);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) != nullptr);

        // test common ability foreground timeout.
        abilityMs_->HandleForegroundTimeOut(commonAbility->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonAbility->GetToken()) != nullptr);
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
    EXPECT_TRUE(abilityMs_ != nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto curListManager = std::make_shared<MissionListManager>(MOCK_MAIN_USER_ID);
        EXPECT_TRUE(curListManager != nullptr);
        std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
        curListManager->launcherList_ = missionList;
        EXPECT_TRUE(curListManager->launcherList_ != nullptr);

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
        launcher->SetMissionId(missionLauncher->GetMissionId());
        launcher->SetLauncherRoot();
        curListManager->launcherList_->AddMissionToTop(missionLauncher);
        EXPECT_TRUE(curListManager->launcherList_->GetAbilityRecordByToken(launcher->GetToken()) != nullptr);

        // common ability timeout without caller
        abilityRequest.abilityInfo.name = "com.test.TimeoutForeground007";
        auto commonLauncher = AbilityRecord::CreateAbilityRecord(abilityRequest);
        EXPECT_TRUE(commonLauncher != nullptr);
        auto commonMissionLauncher =
            std::make_shared<Mission>(MOCK_MISSION_ID + 1, commonLauncher, abilityRequest.abilityInfo.bundleName);
        EXPECT_TRUE(commonMissionLauncher != nullptr);
        missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
        commonLauncher->SetMissionId(commonMissionLauncher->GetMissionId());
        missionList->AddMissionToTop(commonMissionLauncher);
        curListManager->MoveMissionListToTop(missionList);
        commonLauncher->SetAbilityState(AbilityState::FOREGROUNDING);
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);

        // test common launcher foreground timeout.
        abilityMs_->HandleForegroundTimeOut(commonLauncher->GetAbilityRecordId());
        EXPECT_TRUE(curListManager->GetAbilityRecordByToken(commonLauncher->GetToken()) != nullptr);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
