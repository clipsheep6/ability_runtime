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
#include "want.h"
#include "application_info.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class MissionTest : public testing::Test {
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

void MissionTest::SetUpTestCase(void)
{}
void MissionTest::TearDownTestCase(void)
{}
void MissionTest::SetUp(void)
{}
void MissionTest::TearDown(void)
{}

AbilityRequest MissionTest::GenerateAbilityRequest(const std::string &deviceName, const std::string &abilityName,
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
 * Feature: Mission
 * Function: SetMissionList and GetMissionList
 * SubFunction: NA
 * FunctionPoints: Mission SetMissionList
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionList
 */
HWTEST_F(MissionTest, mission_set_mission_list_001, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr);
    EXPECT_EQ(nullptr, mission->GetMissionList());
}

/*
 * Feature: Mission
 * Function: SetMissionList and GetMissionList
 * SubFunction: NA
 * FunctionPoints: Mission SetMissionList
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionList
 */
HWTEST_F(MissionTest, mission_set_mission_list_002, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr);
    mission->SetMissionList(nullptr);
    EXPECT_EQ(nullptr, mission->GetMissionList());
}

/*
 * Feature: Mission
 * Function: SetMissionList and GetMissionList
 * SubFunction: NA
 * FunctionPoints: Mission SetMissionList
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionList
 */
HWTEST_F(MissionTest, mission_set_mission_list_003, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr);
    auto missionList = std::make_shared<MissionList>();
    mission->SetMissionList(missionList);
    EXPECT_EQ(missionList, mission->GetMissionList());
}

/*
 * Feature: Mission
 * Function: SetMissionList and GetMissionList
 * SubFunction: NA
 * FunctionPoints: Mission SetMissionList
 * EnvConditions: NA
 * CaseDescription: Verify SetMissionList
 */
HWTEST_F(MissionTest, mission_set_mission_list_004, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr);
    auto missionList = std::make_shared<MissionList>();
    mission->SetMissionList(missionList);
    auto missionList1 = std::make_shared<MissionList>();
    mission->SetMissionList(missionList1);
    EXPECT_EQ(missionList1, mission->GetMissionList());
}

/*
 * Feature: Mission
 * Function: IsSingletonAbility
 * SubFunction: NA
 * FunctionPoints: Mission IsSingletonAbility
 * EnvConditions: NA
 * CaseDescription: Verify IsSingletonAbility
 */
HWTEST_F(MissionTest, mission_is_singleton_001, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr);
    EXPECT_FALSE(mission->IsSingletonAbility());
}

/*
 * Feature: Mission
 * Function: IsSingletonAbility
 * SubFunction: NA
 * FunctionPoints: Mission IsSingletonAbility
 * EnvConditions: NA
 * CaseDescription: Verify IsSingletonAbility
 */
HWTEST_F(MissionTest, mission_is_singleton_002, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(0, abilityRecord);
    EXPECT_FALSE(mission->IsSingletonAbility());
}

/*
 * Feature: Mission
 * Function: IsSingletonAbility
 * SubFunction: NA
 * FunctionPoints: Mission IsSingletonAbility
 * EnvConditions: NA
 * CaseDescription: Verify IsSingletonAbility
 */
HWTEST_F(MissionTest, mission_is_singleton_003, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(0, abilityRecord);
    EXPECT_TRUE(mission->IsSingletonAbility());
}

/*
 * Feature: Mission
 * Function: GetMissionName
 * SubFunction: NA
 * FunctionPoints: Mission GetMissionName
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionName
 */
HWTEST_F(MissionTest, mission_get_mission_name_001, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(0, abilityRecord);
    EXPECT_TRUE("" == mission->GetMissionName());
}

/*
 * Feature: Mission
 * Function: GetMissionName
 * SubFunction: NA
 * FunctionPoints: Mission GetMissionName
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionName
 */
HWTEST_F(MissionTest, mission_get_mission_name_002, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(0, abilityRecord, "");
    EXPECT_TRUE("" == mission->GetMissionName());
}

/*
 * Feature: Mission
 * Function: GetMissionName
 * SubFunction: NA
 * FunctionPoints: Mission GetMissionName
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionName
 */
HWTEST_F(MissionTest, mission_get_mission_name_003, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(0, abilityRecord, "name1");
    EXPECT_TRUE("name1" == mission->GetMissionName());
}

/*
 * Feature: Mission
 * Function: SetLockedState/IsLockedState
 * SubFunction: NA
 * FunctionPoints: Mission SetLockedState
 * EnvConditions: NA
 * CaseDescription: Verify SetLockedState/IsLockedState
 */
HWTEST_F(MissionTest, mission_locked_state_001, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(0, abilityRecord, "name1");
    EXPECT_FALSE(mission->IsLockedState());
}

/*
 * Feature: Mission
 * Function: SetLockedState/IsLockedState
 * SubFunction: NA
 * FunctionPoints: Mission SetLockedState
 * EnvConditions: NA
 * CaseDescription: Verify SetLockedState/IsLockedState
 */
HWTEST_F(MissionTest, mission_locked_state_002, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(0, abilityRecord, "name1");
    mission->SetLockedState(true);
    EXPECT_TRUE(mission->IsLockedState());
}

/*
 * Feature: Mission
 * Function: SetLockedState/IsLockedState
 * SubFunction: NA
 * FunctionPoints: Mission SetLockedState
 * EnvConditions: NA
 * CaseDescription: Verify SetLockedState/IsLockedState
 */
HWTEST_F(MissionTest, mission_locked_state_003, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(0, abilityRecord, "name1");
    mission->SetLockedState(true);
    EXPECT_TRUE(mission->IsLockedState());
    mission->SetLockedState(false);
    EXPECT_FALSE(mission->IsLockedState());
}

/*
 * Feature: Mission
 * Function: copy constructor
 * SubFunction: NA
 * FunctionPoints: Mission copy constructor
 * EnvConditions: NA
 * CaseDescription: deep copy a object, with same content but different pointer address
 */
HWTEST_F(MissionTest, mission_copy_constructor_001, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission1 = std::make_shared<Mission>(0, abilityRecord, "name1");
    auto mission2= std::make_shared<Mission>(mission1);
    EXPECT_NE(mission1, mission2); 
    EXPECT_NE(&(mission1->missionId_), &(mission2->missionId_));
    EXPECT_NE(&(mission1->startMethod_), &(mission2->startMethod_));
    EXPECT_NE(&(mission1->abilityRecord_), &(mission2->abilityRecord_));
    EXPECT_NE(&(mission1->missionName_), &(mission2->missionName_));
    EXPECT_EQ(mission1->missionId_, mission2->missionId_);
    EXPECT_EQ(mission1->startMethod_, mission2->startMethod_);
    EXPECT_EQ(mission1->abilityRecord_, mission2->abilityRecord_);
    EXPECT_EQ(mission1->missionName_, mission2->missionName_);
}

/*
 * Feature: Mission
 * Function: IsSpecifiedAbility
 * SubFunction: NA
 * FunctionPoints: Mission IsSpecifiedAbility
 * EnvConditions: NA
 * CaseDescription: Verify IsSpecifiedAbility
 */
HWTEST_F(MissionTest, mission_is_specified_001, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr);
    EXPECT_FALSE(mission->IsSpecifiedAbility());
}

/*
 * Feature: Mission
 * Function: IsSpecifiedAbility
 * SubFunction: NA
 * FunctionPoints: Mission IsSpecifiedAbility
 * EnvConditions: NA
 * CaseDescription: Verify IsSpecifiedAbility
 */
HWTEST_F(MissionTest, mission_is_specified_002, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(0, abilityRecord);
    EXPECT_FALSE(mission->IsSpecifiedAbility());
}

/*
 * Feature: Mission
 * Function: IsSpecifiedAbility
 * SubFunction: NA
 * FunctionPoints: Mission IsSpecifiedAbility
 * EnvConditions: NA
 * CaseDescription: Verify IsSpecifiedAbility
 */
HWTEST_F(MissionTest, mission_is_specified_003, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SPECIFIED;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(0, abilityRecord);
    EXPECT_TRUE(mission->IsSpecifiedAbility());
}

/*
 * Feature: Mission
 * Function: SetSpecifiedFlag and GetSpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: Mission SetSpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify SetSpecifiedFlag
 */
HWTEST_F(MissionTest, mission_set_specified_flag_001, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr);
    EXPECT_EQ("", mission->GetSpecifiedFlag());
}

/*
 * Feature: Mission
 * Function: SetSpecifiedFlag and GetSpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: Mission SetSpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify SetSpecifiedFlag
 */
HWTEST_F(MissionTest, mission_set_specified_flag_002, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr);
    mission->SetSpecifiedFlag("");
    EXPECT_EQ("", mission->GetSpecifiedFlag());
}

/*
 * Feature: Mission
 * Function: SetSpecifiedFlag and GetSpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: Mission SetSpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify SetSpecifiedFlag
 */
HWTEST_F(MissionTest, mission_set_specified_flag_003, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr);
    mission->SetSpecifiedFlag("test_string");
    EXPECT_EQ("test_string", mission->GetSpecifiedFlag());
}

/*
 * Feature: Mission
 * Function: SetSpecifiedFlag and GetSpecifiedFlag
 * SubFunction: NA
 * FunctionPoints: Mission SetSpecifiedFlag
 * EnvConditions: NA
 * CaseDescription: Verify SetSpecifiedFlag
 */
HWTEST_F(MissionTest, mission_set_specified_flag_004, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr);
    mission->SetSpecifiedFlag("test_string");
    mission->SetSpecifiedFlag("test_string2");
    EXPECT_EQ("test_string2", mission->GetSpecifiedFlag());
}

/*
 * Feature: Mission
 * Function: SetMovingState and IsMovingState
 * SubFunction: NA
 * FunctionPoints: Mission SetMovingState
 * EnvConditions: NA
 * CaseDescription: Verify SetMovingState
 */
HWTEST_F(MissionTest, mission_set_moving_state_001, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr);
    EXPECT_EQ(false, mission->IsMovingState());
}


/*
 * Feature: Mission
 * Function: SetMovingState and IsMovingState
 * SubFunction: NA
 * FunctionPoints: Mission SetMovingState
 * EnvConditions: NA
 * CaseDescription: Verify SetMovingState
 */
HWTEST_F(MissionTest, mission_set_moving_state_002, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr);
    mission->SetMovingState(true);
    EXPECT_EQ(true, mission->IsMovingState());
}

/*
 * Feature: Mission
 * Function: SetANRState and IsANRState
 * SubFunction: NA
 * FunctionPoints: Mission SetANRState
 * EnvConditions: NA
 * CaseDescription: Verify SetANRState
 */
HWTEST_F(MissionTest, mission_set_anr_state_001, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr);
    EXPECT_EQ(false, mission->IsANRState());
}


/*
 * Feature: Mission
 * Function: SetANRState and IsANRState
 * SubFunction: NA
 * FunctionPoints: Mission SetANRState
 * EnvConditions: NA
 * CaseDescription: Verify SetANRState
 */
HWTEST_F(MissionTest, mission_set_anr_state_002, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr);
    mission->SetANRState(true);
    EXPECT_EQ(true, mission->IsANRState());
}

/*
 * Feature: Mission
 * Function: Dump
 * SubFunction: NA
 * FunctionPoints: Mission Dump
 * EnvConditions: NA
 * CaseDescription: Test Dump
 */
HWTEST_F(MissionTest, mission_dump, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto mission = std::make_shared<Mission>(0, abilityRecord, "name1");
    std::vector<std::string> info;
    mission->Dump(info);
}

/*
 * Feature: Mission
 * Function: UpdateMissionId
 * SubFunction: NA
 * FunctionPoints: Mission UpdateMissionId
 * EnvConditions: NA
 * CaseDescription: Verify UpdateMissionId
 */
HWTEST_F(MissionTest, mission_update_mission_id, TestSize.Level1)
{
    auto mission = std::make_shared<Mission>(1, nullptr, "name1", 0);
    EXPECT_EQ(1, mission->GetMissionId());
    EXPECT_EQ(false, mission->UpdateMissionId(2, 0));
    EXPECT_EQ(1, mission->GetMissionId());
    EXPECT_EQ(true, mission->UpdateMissionId(2, 1));
    EXPECT_EQ(2, mission->GetMissionId());
}
}  // namespace AAFwk
}  // namespace OHOS