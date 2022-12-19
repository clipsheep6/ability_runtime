/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "mission.h"
#include "mission_list.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class MissionListTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static AbilityRequest GenerateAbilityRequest(const std::string &deviceName, const std::string &abilityName,
        const std::string &appName, const std::string &bundleName);

    Want want_ {};
    AbilityInfo abilityInfo_ {};
    ApplicationInfo appInfo_ {};
};

void MissionListTest::SetUpTestCase(void)
{}
void MissionListTest::TearDownTestCase(void)
{}
void MissionListTest::SetUp(void)
{}
void MissionListTest::TearDown(void)
{}

AbilityRequest MissionListTest::GenerateAbilityRequest(const std::string &deviceName, const std::string &abilityName,
    const std::string &appName, const std::string &bundleName)
{
    ElementName element(deviceName, abilityName, bundleName);
    Want want;
    want.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    ApplicationInfo appinfo;
    appinfo.name = appName;

    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    return abilityRequest;
}

/*
 * Feature: MissionList
 * Function: AddMissionToTop and GetTopMission
 * SubFunction: NA
 * FunctionPoints: MissionList AddMissionToTop and GetTopMission
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionToTop and GetTopMission
 */
HWTEST_F(MissionListTest, mission_list_get_top_mission_001, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    missionList->AddMissionToTop(nullptr);
    EXPECT_EQ(nullptr, missionList->GetTopMission());
    EXPECT_TRUE(missionList->IsEmpty());
}

/*
 * Feature: MissionList
 * Function: AddMissionToTop and GetTopMission
 * SubFunction: NA
 * FunctionPoints: MissionList AddMissionToTop and GetTopMission
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionToTop and GetTopMission
 */
HWTEST_F(MissionListTest, mission_list_get_top_mission_002, TestSize.Level1)
{
    // 1. create a mission
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, nullptr, "");
    // 2. create a mission list
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    // 3. add the mission
    missionList->AddMissionToTop(mission);
    // 4. try to add the mission again
    missionList->AddMissionToTop(mission);
    EXPECT_EQ(mission, missionList->GetTopMission());
    EXPECT_TRUE(missionList->GetAllMissions().size() == 1);
}

/*
 * Feature: MissionList
 * Function: AddMissionToTop and GetTopMission
 * SubFunction: NA
 * FunctionPoints: MissionList AddMissionToTop and GetTopMission
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionToTop and GetTopMission
 */
HWTEST_F(MissionListTest, mission_list_get_top_mission_003, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, nullptr, "");
    missionList->AddMissionToTop(mission);
    EXPECT_EQ(mission, missionList->GetTopMission());
}

/*
 * Feature: MissionList
 * Function: AddMissionToTop and GetTopMission
 * SubFunction: NA
 * FunctionPoints: MissionList AddMissionToTop and GetTopMission
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionToTop and GetTopMission
 */
HWTEST_F(MissionListTest, mission_list_get_top_mission_005, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    EXPECT_EQ(nullptr, missionList->GetTopMission());
}

/*
 * Feature: MissionList
 * Function: AddMissionToTop and GetTopMission
 * SubFunction: NA
 * FunctionPoints: MissionList AddMissionToTop and GetTopMission
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionToTop and GetTopMission
 */
HWTEST_F(MissionListTest, mission_list_get_top_mission_006, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, nullptr, "");
    missionList->AddMissionToTop(mission);
    std::shared_ptr<Mission> mission1 = std::make_shared<Mission>(1, nullptr, "");
    missionList->AddMissionToTop(mission1);
    EXPECT_EQ(mission1, missionList->GetTopMission());
}

/*
 * Feature: MissionList
 * Function: AddMissionToTop and GetTopMission
 * SubFunction: NA
 * FunctionPoints: MissionList AddMissionToTop and GetTopMission
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionToTop and GetTopMission
 */
HWTEST_F(MissionListTest, mission_list_get_top_mission_007, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    // 1. create and add mission1
    std::shared_ptr<Mission> mission1 = std::make_shared<Mission>(1, nullptr, "");
    missionList->AddMissionToTop(mission1);
    // 2. create and add mission2
    std::shared_ptr<Mission> mission2 = std::make_shared<Mission>(1, nullptr, "");
    missionList->AddMissionToTop(mission2);

    // 3. put agian mission1
    missionList->AddMissionToTop(mission1);
    EXPECT_EQ(mission1, missionList->GetTopMission());
    EXPECT_TRUE(missionList->GetAllMissions().size() == 2);
}

/*
 * Feature: MissionList
 * Function: RemoveMission
 * SubFunction: NA
 * FunctionPoints: MissionList RemoveMission
 * EnvConditions: NA
 * CaseDescription: Verify RemoveMission
 */
HWTEST_F(MissionListTest, mission_list_remove_mission_001, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, nullptr, "");
    missionList->AddMissionToTop(mission);
    std::shared_ptr<Mission> mission1 = std::make_shared<Mission>(1, nullptr, "");
    missionList->AddMissionToTop(mission1);
    EXPECT_EQ(mission1, missionList->GetTopMission());

    missionList->RemoveMission(mission1);
    EXPECT_EQ(mission, missionList->GetTopMission());

    missionList->RemoveMission(mission1);
    EXPECT_EQ(mission, missionList->GetTopMission());

    missionList->RemoveMission(mission);
    EXPECT_EQ(nullptr, missionList->GetTopMission());
}

/*
 * Feature: MissionList
 * Function: RemoveMission
 * SubFunction: NA
 * FunctionPoints: MissionList RemoveMission
 * EnvConditions: NA
 * CaseDescription: Verify RemoveMission
 */
HWTEST_F(MissionListTest, mission_list_remove_mission_002, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(1, nullptr, "");
    missionList->AddMissionToTop(mission);
    std::shared_ptr<Mission> mission1 = std::make_shared<Mission>(1, nullptr, "");
    missionList->AddMissionToTop(mission1);
    EXPECT_EQ(mission1, missionList->GetTopMission());

    missionList->RemoveMission(mission1);
    EXPECT_EQ(mission, missionList->GetTopMission());

    missionList->RemoveMission(mission1);
    EXPECT_EQ(mission, missionList->GetTopMission());

    missionList->RemoveMission(mission);
    EXPECT_EQ(nullptr, missionList->GetTopMission());
}

/*
 * Feature: Mission
 * Function: GetSingletonMissionByName
 * SubFunction: NA
 * FunctionPoints: Mission GetSingletonMissionByName
 * EnvConditions: NA
 * CaseDescription: Verify GetSingletonMissionByName
 */
HWTEST_F(MissionListTest, mission_list_get_mission_001, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(1, abilityRecord, "name1");

    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    missionList->AddMissionToTop(mission);

    auto mission1 = missionList->GetSingletonMissionByName("name1");
    EXPECT_EQ(mission, mission1);
}

/*
 * Feature: Mission
 * Function: GetSingletonMissionByName
 * SubFunction: NA
 * FunctionPoints: Mission GetSingletonMissionByName
 * EnvConditions: NA
 * CaseDescription: Verify GetSingletonMissionByName
 */
HWTEST_F(MissionListTest, mission_list_get_mission_002, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(1, abilityRecord, "name1");

    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    missionList->AddMissionToTop(mission);

    auto mission1 = missionList->GetSingletonMissionByName("name1");
    EXPECT_EQ(nullptr, mission1);
}

/*
 * Feature: Mission
 * Function: GetSingletonMissionByName
 * SubFunction: NA
 * FunctionPoints: Mission GetSingletonMissionByName
 * EnvConditions: NA
 * CaseDescription: Verify GetSingletonMissionByName
 */
HWTEST_F(MissionListTest, mission_list_get_mission_003, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(1, abilityRecord, "name");

    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    missionList->AddMissionToTop(mission);

    auto mission1 = missionList->GetSingletonMissionByName("name1");
    EXPECT_EQ(nullptr, mission1);

    auto mission2 = missionList->GetSingletonMissionByName("");
    EXPECT_EQ(nullptr, mission2);
}

/*
 * Feature: Mission
 * Function: GetSpecifiedMission
 * SubFunction: NA
 * FunctionPoints: Mission GetSpecifiedMission
 * EnvConditions: NA
 * CaseDescription: Verify GetSpecifiedMission
 */
HWTEST_F(MissionListTest, mission_list_get_specified_mission_001, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>();
    auto mission1 = missionList->GetSpecifiedMission("", "flag");
    EXPECT_EQ(nullptr, mission1);

    auto mission2 = missionList->GetSpecifiedMission("name", "");
    EXPECT_EQ(nullptr, mission2);
}

/*
 * Feature: Mission
 * Function: GetSpecifiedMission
 * SubFunction: NA
 * FunctionPoints: Mission GetSpecifiedMission
 * EnvConditions: NA
 * CaseDescription: Verify GetSpecifiedMission
 */
HWTEST_F(MissionListTest, mission_list_get_specified_mission_002, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(1, abilityRecord, "name");
    mission->SetSpecifiedFlag("flag");

    auto missionList = std::make_shared<MissionList>();
    missionList->AddMissionToTop(mission);

    auto mission1 = missionList->GetSpecifiedMission("name", "flag");
    EXPECT_NE(nullptr, mission1);
}

/*
 * Feature: Mission
 * Function: GetSpecifiedMission
 * SubFunction: NA
 * FunctionPoints: Mission GetSpecifiedMission
 * EnvConditions: NA
 * CaseDescription: Verify GetSpecifiedMission
 */
HWTEST_F(MissionListTest, mission_list_get_specified_mission_003, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(1, abilityRecord, "name");
    mission->SetSpecifiedFlag("flag");

    auto missionList = std::make_shared<MissionList>();
    missionList->AddMissionToTop(mission);

    auto mission1 = missionList->GetSpecifiedMission("name", "flag1");
    EXPECT_EQ(nullptr, mission1);

    auto mission2 = missionList->GetSpecifiedMission("name1", "flag1");
    EXPECT_EQ(nullptr, mission2);
}

/*
 * Feature: Mission
 * Function: GetSpecifiedMission
 * SubFunction: NA
 * FunctionPoints: Mission GetSpecifiedMission
 * EnvConditions: NA
 * CaseDescription: Verify GetSpecifiedMission
 */
HWTEST_F(MissionListTest, mission_list_get_specified_mission_004, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(1, abilityRecord, "name");
    mission->SetSpecifiedFlag("flag");

    auto missionList = std::make_shared<MissionList>();
    missionList->AddMissionToTop(mission);

    auto mission1 = missionList->GetSpecifiedMission("name", "flag");
    EXPECT_EQ(nullptr, mission1);
}

/*
 * Feature: MissionList
 * Function: GetAbilityRecordByToken
 * SubFunction: NA
 * FunctionPoints: MissionList GetAbilityRecordByToken
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByToken
 */
HWTEST_F(MissionListTest, mission_list_get_ability_record_by_token_001, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    EXPECT_EQ(nullptr, missionList->GetAbilityRecordByToken(nullptr));
}

/*
 * Feature: MissionList
 * Function: GetAbilityRecordByToken
 * SubFunction: NA
 * FunctionPoints: MissionList GetAbilityRecordByToken
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByToken
 */
HWTEST_F(MissionListTest, mission_list_get_ability_record_by_token_002, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<AbilityRecord> abilityRecord2 = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    abilityRecord2->Init();
    auto mission1 = std::make_shared<Mission>(1, abilityRecord, "name");
    auto mission2 = std::make_shared<Mission>(1, abilityRecord2, "name");
    auto mission3 = std::make_shared<Mission>(1, nullptr, "name");

    auto missionList = std::make_shared<MissionList>();
    EXPECT_TRUE(missionList->missions_.size() == 0);
    missionList->AddMissionToTop(mission1);
    missionList->AddMissionToTop(mission2);
    missionList->AddMissionToTop(mission3);
    missionList->missions_.push_front(nullptr);
    EXPECT_TRUE(missionList->missions_.size() == 4);

    EXPECT_EQ(abilityRecord, missionList->GetAbilityRecordByToken(abilityRecord->GetToken()));
}

/*
 * Feature: MissionList
 * Function: RemoveMissionByAbilityRecord
 * SubFunction: NA
 * FunctionPoints: MissionList RemoveMissionByAbilityRecord
 * EnvConditions: NA
 * CaseDescription: Verify RemoveMissionByAbilityRecord
 */
HWTEST_F(MissionListTest, mission_list_remove_ability_record_by_token_001, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    auto mission = std::make_shared<Mission>(1, abilityRecord, "name");

    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    missionList->AddMissionToTop(mission);

    missionList->RemoveMissionByAbilityRecord(abilityRecord);

    EXPECT_EQ(nullptr, missionList->GetTopMission());
}

/*
 * Feature: MissionList
 * Function: GetMissionById
 * SubFunction: NA
 * FunctionPoints: MissionList GetMissionById
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionById
 */
HWTEST_F(MissionListTest, mission_list_get_mission_by_id_001, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>();
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    auto mission = std::make_shared<Mission>(1, abilityRecord, "name");
    missionList->missions_.push_back(mission);
    missionList->missions_.push_back(nullptr);

    auto res1 = missionList->GetMissionById(1);
    EXPECT_NE(res1, nullptr);

    auto res2 = missionList->GetMissionById(0);
    EXPECT_EQ(res2, nullptr);
}

/*
 * Feature: MissionList
 * Function: GetMissionBySpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: MissionList GetMissionBySpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionBySpecifiedFlag
 */
HWTEST_F(MissionListTest, mission_list_get_mission_by_specified_flag_001, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>();
    std::string flag = "flag";
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    want.SetElementName("ability", "bundle");
    abilityRecord->abilityInfo_.name = "ability";
    abilityRecord->applicationInfo_.bundleName = "bundle";
    abilityRecord->SetSpecifiedFlag("");
    auto mission1 = std::make_shared<Mission>(1, abilityRecord, "name");
    abilityRecord->applicationInfo_.bundleName = "";
    auto mission2 = std::make_shared<Mission>(2, abilityRecord, "name");
    abilityRecord->abilityInfo_.name = "";
    auto mission3 = std::make_shared<Mission>(3, abilityRecord, "name");
    auto mission4 = std::make_shared<Mission>(4, nullptr, "name");

    missionList->missions_.push_back(mission1);
    missionList->missions_.push_back(mission2);
    missionList->missions_.push_back(mission3);
    missionList->missions_.push_back(mission4);
    auto res1 = missionList->GetMissionBySpecifiedFlag(want, flag);
    EXPECT_EQ(res1, nullptr);

    missionList->missions_.push_front(nullptr);
    auto res2 = missionList->GetMissionBySpecifiedFlag(want, flag);
    EXPECT_EQ(res2, nullptr);
}

/*
 * Feature: MissionList
 * Function: IsEmpty
 * SubFunction: NA
 * FunctionPoints: MissionList IsEmpty
 * EnvConditions: NA
 * CaseDescription: Verify IsEmpty
 */
HWTEST_F(MissionListTest, mission_list_is_empty_001, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    EXPECT_TRUE(missionList->IsEmpty());
}

/*
 * Feature: MissionList
 * Function: IsEmpty
 * SubFunction: NA
 * FunctionPoints: MissionList IsEmpty
 * EnvConditions: NA
 * CaseDescription: Verify IsEmpty
 */
HWTEST_F(MissionListTest, mission_list_is_empty_002, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    auto mission = std::make_shared<Mission>(1, abilityRecord, "name");

    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    missionList->AddMissionToTop(mission);

    EXPECT_FALSE(missionList->IsEmpty());
}

/*
 * Feature: MissionList
 * Function: GetTopAbility
 * SubFunction: NA
 * FunctionPoints: MissionList GetTopAbility
 * EnvConditions: NA
 * CaseDescription: Verify GetTopAbility
 */
HWTEST_F(MissionListTest, mission_list_get_top_ability_001, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(nullptr);
    auto res = missionList->GetTopAbility();
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: MissionList
 * Function: GetType
 * SubFunction: NA
 * FunctionPoints: MissionList GetType
 * EnvConditions: NA
 * CaseDescription: Verify GetType
 */
HWTEST_F(MissionListTest, mission_list_get_type_001, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>();
    EXPECT_EQ(MissionListType::CURRENT, missionList->GetType());
}

/*
 * Feature: MissionList
 * Function: GetType
 * SubFunction: NA
 * FunctionPoints: MissionList GetType
 * EnvConditions: NA
 * CaseDescription: Verify GetType
 */
HWTEST_F(MissionListTest, mission_list_get_type_002, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::CURRENT);
    EXPECT_EQ(MissionListType::CURRENT, missionList->GetType());
}

/*
 * Feature: MissionList
 * Function: GetType
 * SubFunction: NA
 * FunctionPoints: MissionList GetType
 * EnvConditions: NA
 * CaseDescription: Verify GetType
 */
HWTEST_F(MissionListTest, mission_list_get_type_003, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    EXPECT_EQ(MissionListType::DEFAULT_STANDARD, missionList->GetType());
}

/*
 * Feature: MissionList
 * Function: GetType
 * SubFunction: NA
 * FunctionPoints: MissionList GetType
 * EnvConditions: NA
 * CaseDescription: Verify GetType
 */
HWTEST_F(MissionListTest, mission_list_get_type_004, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::DEFAULT_SINGLE);
    EXPECT_EQ(MissionListType::DEFAULT_SINGLE, missionList->GetType());
}

/*
 * Feature: MissionList
 * Function: GetType
 * SubFunction: NA
 * FunctionPoints: MissionList GetType
 * EnvConditions: NA
 * CaseDescription: Verify GetType
 */
HWTEST_F(MissionListTest, mission_list_get_type_005, TestSize.Level1)
{
    auto missionList = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    EXPECT_EQ(MissionListType::LAUNCHER, missionList->GetType());
}

/*
 * Feature: MissionList
 * Function: GetLauncherRoot
 * SubFunction: NA
 * FunctionPoints: MissionList GetLauncherRoot
 * EnvConditions: NA
 * CaseDescription: Verify GetLauncherRoot
 */
HWTEST_F(MissionListTest, mission_list_get_launcher_root_001, TestSize.Level1)
{
    // 1. create mission1
    AppExecFwk::AbilityInfo abilityInfo;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    abilityRecord->SetLauncherRoot();
    auto mission1 = std::make_shared<Mission>(1, abilityRecord, "launcher");

    // 2. create mission2
    AppExecFwk::AbilityInfo abilityInfo1;
    Want want1;
    AppExecFwk::ApplicationInfo applicationInfo1;
    std::shared_ptr<AbilityRecord> abilityRecord1
        = std::make_shared<AbilityRecord>(want1, abilityInfo1, applicationInfo1);
    abilityRecord1->Init();
    abilityRecord1->isLauncherRoot_ = false;
    auto mission2 = std::make_shared<Mission>(1, abilityRecord1, "launcher");

    // 3. create mission3
    auto mission3 = std::make_shared<Mission>(1, nullptr, "launcher");

    auto missionList = std::make_shared<MissionList>();
    missionList->AddMissionToTop(mission1);
    missionList->AddMissionToTop(mission2);
    missionList->AddMissionToTop(mission3);
    missionList->missions_.push_front(nullptr);

    EXPECT_EQ(abilityRecord, missionList->GetLauncherRoot());
}

/*
 * Feature: MissionList
 * Function: GetAbilityRecordById
 * SubFunction: NA
 * FunctionPoints: MissionList GetAbilityRecordById
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordById
 */
HWTEST_F(MissionListTest, mission_list_get_ability_record_by_id_001, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    abilityRecord->SetEventId(1);
    auto mission = std::make_shared<Mission>(1, abilityRecord, "name");
    auto missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission);
    missionList->missions_.push_front(nullptr);
    auto res = missionList->GetAbilityRecordById(0);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: MissionList
 * Function: GetAbilityRecordByCaller
 * SubFunction: NA
 * FunctionPoints: MissionList GetAbilityRecordByCaller
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByCaller
 */
HWTEST_F(MissionListTest, mission_list_get_ability_record_by_caller_001, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord1 = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord1->Init();
    std::shared_ptr<CallerRecord> callerRecord1 = std::make_shared<CallerRecord>(0, abilityRecord1);
    std::shared_ptr<CallerRecord> callerRecord2 = std::make_shared<CallerRecord>(1, abilityRecord1);
    std::shared_ptr<CallerRecord> callerRecord3 = std::make_shared<CallerRecord>(1, nullptr);
    abilityRecord1->callerList_.push_back(callerRecord2);
    abilityRecord1->callerList_.push_back(callerRecord3);
    abilityRecord1->callerList_.push_back(callerRecord1);
    auto mission1 = std::make_shared<Mission>(1, abilityRecord1, "name");

    std::shared_ptr<AbilityRecord> abilityRecord2 = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord2->Init();
    abilityRecord2->callerList_.clear();
    auto mission2 = std::make_shared<Mission>(2, abilityRecord2, "name");

    auto missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission1);
    missionList->missions_.push_front(mission2);
    auto res = missionList->GetAbilityRecordByCaller(abilityRecord1, 0);
    EXPECT_EQ(res, abilityRecord1);
}

/*
 * Feature: MissionList
 * Function: GetAbilityRecordByName
 * SubFunction: NA
 * FunctionPoints: MissionList GetAbilityRecordByName
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordByName
 */
HWTEST_F(MissionListTest, mission_list_get_ability_record_by_name_001, TestSize.Level1)
{
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    abilityInfo.deviceId = "1";
    abilityInfo.bundleName = "bundle";
    abilityInfo.name = "name";
    abilityInfo.moduleName = "bundle";
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    auto mission = std::make_shared<Mission>(1, abilityRecord, "name");
    auto missionList = std::make_shared<MissionList>();
    ElementName element("", "", "");
    missionList->missions_.push_front(mission);
    missionList->missions_.push_front(nullptr);
    auto res = missionList->GetAbilityRecordByName(element);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: MissionList
 * Function: GetAbilityTokenByMissionId
 * SubFunction: NA
 * FunctionPoints: MissionList GetAbilityTokenByMissionId
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityTokenByMissionId
 */
HWTEST_F(MissionListTest, mission_list_get_ability_token_by_mission_id_001, TestSize.Level1)
{
    auto mission1 = std::make_shared<Mission>(1, nullptr, "name");
    auto mission2 = std::make_shared<Mission>(2, nullptr, "name");
    auto missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission1);
    missionList->missions_.push_front(mission2);
    missionList->missions_.push_front(nullptr);
    auto res = missionList->GetAbilityTokenByMissionId(1);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: MissionList
 * Function: HandleUnInstallApp
 * SubFunction: NA
 * FunctionPoints: MissionList HandleUnInstallApp
 * EnvConditions: NA
 * CaseDescription: Verify HandleUnInstallApp
 */
HWTEST_F(MissionListTest, mission_list_handle_uninstall_app_001, TestSize.Level1)
{
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    abilityInfo.bundleName = "bundle";
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->SetUid(0);
    abilityRecord->SetAbilityState(AbilityState::INITIAL);
    auto mission = std::make_shared<Mission>(1, abilityRecord, "name");
    auto missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(mission);
    missionList->HandleUnInstallApp("bundle", 0);
}

/*
 * Feature: MissionList
 * Function: MatchedInitialMission
 * SubFunction: NA
 * FunctionPoints: MissionList MatchedInitialMission
 * EnvConditions: NA
 * CaseDescription: Verify MatchedInitialMission
 */
HWTEST_F(MissionListTest, mission_list_match_initial_mission_001, TestSize.Level1)
{
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    abilityInfo.bundleName = "bundle";
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->SetUid(0);
    abilityRecord->SetAbilityState(AbilityState::ACTIVE);
    auto mission1 = std::make_shared<Mission>(1, abilityRecord, "name");
    auto mission2 = std::make_shared<Mission>(1, nullptr, "name");
    auto missionList = std::make_shared<MissionList>();
    bool res1 = missionList->MatchedInitialMission(nullptr, "bundle", 0);
    EXPECT_FALSE(res1);

    bool res2 = missionList->MatchedInitialMission(mission2, "bundle", 0);
    EXPECT_FALSE(res2);

    bool res3 = missionList->MatchedInitialMission(mission1, "bundle", 1);
    EXPECT_FALSE(res3);

    bool res4 = missionList->MatchedInitialMission(mission1, "bundle1", 0);
    EXPECT_FALSE(res4);

    bool res5 = missionList->MatchedInitialMission(mission1, "bundle", 0);
    EXPECT_FALSE(res5);
}

/*
 * Feature: MissionList
 * Function: Dump
 * SubFunction: NA
 * FunctionPoints: MissionList Dump
 * EnvConditions: NA
 * CaseDescription: Verify Dump
 */
HWTEST_F(MissionListTest, mission_list_dump_001, TestSize.Level1)
{
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(1, abilityRecord, "name");
    auto missionList = std::make_shared<MissionList>();
    std::vector<std::string> info;
    missionList->missions_.push_front(nullptr);
    missionList->missions_.push_front(mission);
    EXPECT_EQ(missionList->missions_.front(), mission);
    EXPECT_EQ(missionList->missions_.back(), nullptr);
    missionList->Dump(info);
}

/*
 * Feature: MissionList
 * Function: DumpStateByRecordId
 * SubFunction: NA
 * FunctionPoints: MissionList DumpStateByRecordId
 * EnvConditions: NA
 * CaseDescription: Verify DumpStateByRecordId
 */
HWTEST_F(MissionListTest, mission_list_dump_state_by_record_id_001, TestSize.Level1)
{
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->recordId_ = 0;
    auto mission1 = std::make_shared<Mission>(1, abilityRecord, "name");
    auto mission2 = std::make_shared<Mission>(1, nullptr, "name");
    auto missionList = std::make_shared<MissionList>();
    std::vector<std::string> info;
    bool isClient = false;
    int32_t abilityRecordId = 1;
    std::vector<std::string> params;
    missionList->missions_.push_front(nullptr);
    missionList->missions_.push_front(mission1);
    missionList->missions_.push_front(mission2);
    missionList->DumpStateByRecordId(info, isClient, abilityRecordId, params);
}

/*
 * Feature: MissionList
 * Function: DumpList
 * SubFunction: NA
 * FunctionPoints: MissionList DumpList
 * EnvConditions: NA
 * CaseDescription: Verify DumpList
 */
HWTEST_F(MissionListTest, mission_list_dump_list_001, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr, "name");
    auto missionList = std::make_shared<MissionList>();
    std::vector<std::string> info;
    bool isClient = false;
    missionList->missions_.push_front(nullptr);
    missionList->missions_.push_front(mission);
    missionList->DumpList(info, isClient);
}

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: MissionList
 * Function: BlockAbilityByRecordId
 * SubFunction: NA
 * FunctionPoints: MissionList BlockAbilityByRecordId
 * EnvConditions: NA
 * CaseDescription: Verify BlockAbilityByRecordId
 */
HWTEST_F(MissionListTest, mission_block_ability_by_record_id_001, TestSize.Level1)
{
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->recordId_ = 0;
    auto mission1 = std::make_shared<Mission>(1, abilityRecord, "name");
    auto mission2 = std::make_shared<Mission>(1, nullptr, "name");
    auto missionList = std::make_shared<MissionList>();
    std::vector<std::string> info;
    bool isClient = false;
    std::vector<std::string> params;
    missionList->missions_.push_front(nullptr);
    missionList->missions_.push_front(mission1);
    missionList->missions_.push_front(mission2);
    missionList->DumpStateByRecordId(info, isClient, 1, params);
    missionList->DumpStateByRecordId(info, isClient, 0, params);
}
#endif

/*
 * Feature: MissionList
 * Function: GetMissionCountByUid
 * SubFunction: NA
 * FunctionPoints: MissionList GetMissionCountByUid
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionCountByUid
 */
HWTEST_F(MissionListTest, mission_list_get_mission_count_by_uid_001, TestSize.Level1)
{
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->SetUid(1);
    auto mission1 = std::make_shared<Mission>(1, abilityRecord, "name");
    auto mission2 = std::make_shared<Mission>(1, nullptr, "name");
    auto missionList = std::make_shared<MissionList>();
    missionList->missions_.push_front(nullptr);
    missionList->missions_.push_front(mission1);
    missionList->missions_.push_front(mission2);
    int32_t res1 = missionList->GetMissionCountByUid(0);
    EXPECT_EQ(res1, 0);
    int32_t res2 = missionList->GetMissionCountByUid(1);
    EXPECT_EQ(res2, 1);
}
}  // namespace AAFwk
}  // namespace OHOS
