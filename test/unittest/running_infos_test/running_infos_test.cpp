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

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const int32_t MOCK_MAIN_USER_ID = 100;
}  // namespace
class RunningInfosTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void RunningInfosTest::SetUpTestCase() {}

void RunningInfosTest::TearDownTestCase() {}

void RunningInfosTest::SetUp() {}

void RunningInfosTest::TearDown() {}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start page ability, call query function.
 */
HWTEST_F(RunningInfosTest, GetAbilityRunningInfos_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);

    if (result == OHOS::ERR_OK) {
        std::vector<AbilityRunningInfo> infos;
        abilityMs_->GetAbilityRunningInfos(infos);
        size_t infoCount{ 1 };
        EXPECT_TRUE(infos.size() == infoCount);
        if (infos.size() == infoCount) {
            EXPECT_TRUE(infos[0].ability.GetAbilityName() == element.GetAbilityName());
            EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
        }
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start service ability, call query function.
 */
HWTEST_F(RunningInfosTest, GetAbilityRunningInfos_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("device", "com.ix.hiService", "ServiceAbility");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);

    if (result == OHOS::ERR_OK) {
        std::vector<AbilityRunningInfo> infos;
        abilityMs_->GetAbilityRunningInfos(infos);
        size_t infoCount{ 1 };
        EXPECT_TRUE(infos.size() == infoCount);
        if (infos.size() == infoCount) {
            EXPECT_TRUE(infos[0].ability.GetAbilityName() == element.GetAbilityName());
            EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
        }
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start launcher, call query function.
 */
HWTEST_F(RunningInfosTest, GetAbilityRunningInfos_003, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("device", "com.ohos.launcher", "com.ohos.launcher.MainAbility");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);

    if (result == OHOS::ERR_OK) {
        std::vector<AbilityRunningInfo> infos;
        abilityMs_->GetAbilityRunningInfos(infos);
        size_t infoCount{ 1 };
        EXPECT_TRUE(infos.size() == infoCount);
        if (infos.size() == infoCount) {
            EXPECT_TRUE(infos[0].ability.GetAbilityName() == element.GetAbilityName());
            EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
        }
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start two page abilities, call query function.
 */
HWTEST_F(RunningInfosTest, GetAbilityRunningInfos_004, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);

    if (result == OHOS::ERR_OK) {
        auto topAbility = abilityMs_->subManagersHelper_->currentMissionListManager_->GetCurrentTopAbilityLocked();
        EXPECT_TRUE(topAbility);
        topAbility->SetAbilityState(AbilityState::FOREGROUND);
    }

    ElementName element2("device", "com.ix.hiMusicOther", "MusicAbilityOther");
    want.SetElement(element2);
    auto result2 = abilityMs_->StartAbility(want);

    if (result2 == OHOS::ERR_OK) {
        std::vector<AbilityRunningInfo> infos;
        abilityMs_->GetAbilityRunningInfos(infos);
        size_t infoCount{ 2 };
        EXPECT_TRUE(infos.size() == infoCount);
        if (infos.size() == infoCount) {
            EXPECT_TRUE(infos[0].ability.GetAbilityName() == element2.GetAbilityName());
            EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
            EXPECT_TRUE(infos[1].ability.GetAbilityName() == element.GetAbilityName());
            EXPECT_TRUE(infos[1].abilityState == static_cast<int>(AbilityState::FOREGROUND));
        }
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start two service abilities, call query function.
 */
HWTEST_F(RunningInfosTest, GetAbilityRunningInfos_005, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("device", "com.ix.hiService", "ServiceAbility");
    want.SetElement(element);
    abilityMs_->StartAbility(want);

    ElementName element2("device", "com.ix.hiServiceOther", "ServiceAbilityOther");
    want.SetElement(element2);
    auto result2 = abilityMs_->StartAbility(want);

    if (result2 == OHOS::ERR_OK) {
        std::vector<AbilityRunningInfo> infos;
        abilityMs_->GetAbilityRunningInfos(infos);
        size_t infoCount{ 2 };
        EXPECT_TRUE(infos.size() == infoCount);
        if (infos.size() == infoCount) {
            EXPECT_TRUE(infos[0].ability.GetAbilityName() == element.GetAbilityName());
            EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
            EXPECT_TRUE(infos[1].ability.GetAbilityName() == element2.GetAbilityName());
            EXPECT_TRUE(infos[1].abilityState == static_cast<int>(AbilityState::INITIAL));
        }
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start two launcher, call query function.
 */
HWTEST_F(RunningInfosTest, GetAbilityRunningInfos_006, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    Want want;
    ElementName element("device", "com.ohos.launcher", "com.ohos.launcher.MainAbility");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);

    if (result == OHOS::ERR_OK) {
        auto topAbility = abilityMs_->subManagersHelper_->currentMissionListManager_->GetCurrentTopAbilityLocked();
        EXPECT_TRUE(topAbility);
        topAbility->SetAbilityState(AbilityState::FOREGROUND);
    }

    ElementName element2("device", "com.ohos.launcherOther", "com.ohos.launcher.MainAbilityOther");
    want.SetElement(element2);
    auto result2 = abilityMs_->StartAbility(want);

    if (result2 == OHOS::ERR_OK) {
        std::vector<AbilityRunningInfo> infos;
        abilityMs_->GetAbilityRunningInfos(infos);
        size_t infoCount{ 2 };
        EXPECT_TRUE(infos.size() == infoCount);
        if (infos.size() == infoCount) {
            EXPECT_TRUE(infos[0].ability.GetAbilityName() == element2.GetAbilityName());
            EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
            EXPECT_TRUE(infos[1].ability.GetAbilityName() == element.GetAbilityName());
            EXPECT_TRUE(infos[1].abilityState == static_cast<int>(AbilityState::FOREGROUND));
        }
    }
}

/*
 * @tc.name: GetAbilityRunningInfos_007
 * @tc.desc: GetAbilityRunningInfos Test Foucs State
 * @tc.type: FUNC
 * @tc.require: issueI5PXW4
 */
HWTEST_F(RunningInfosTest, GetAbilityRunningInfos_007, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);

    if (result == OHOS::ERR_OK) {
        auto topAbility = abilityMs_->subManagersHelper_->currentMissionListManager_->GetCurrentTopAbilityLocked();
        EXPECT_TRUE(topAbility);
        topAbility->SetAbilityState(AbilityState::ACTIVE);

        std::vector<AbilityRunningInfo> infos;
        abilityMs_->GetAbilityRunningInfos(infos);

        size_t infoCount{ 1 };
        EXPECT_TRUE(infos.size() == infoCount);
        if (infos.size() == infoCount) {
            EXPECT_TRUE(infos[0].ability.GetAbilityName() == element.GetAbilityName());
            EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::ACTIVE));
        }
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetExtensionRunningInfos
 * SubFunction: NA
 * FunctionPoints:query extension running infos
 * EnvConditions: NA
 * CaseDescription: start service ability, call query function.
 */
HWTEST_F(RunningInfosTest, GetExtensionRunningInfos_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("device", "com.ix.hiExtension", "hiExtension");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);

    if (result == OHOS::ERR_OK) {
        std::vector<ExtensionRunningInfo> infos;
        size_t infoCount{ 1 };
        int upperLimit = 10;
        abilityMs_->GetExtensionRunningInfos(upperLimit, infos);
        EXPECT_TRUE(infos.size() == infoCount);
        if (infos.size() == infoCount) {
            EXPECT_TRUE(infos[0].extension.GetAbilityName() == element.GetAbilityName());
        }
    }
}

/*
 * Feature: AbilityManagerService
 * Function: GetExtensionRunningInfos
 * SubFunction: NA
 * FunctionPoints:query extension running infos
 * EnvConditions: NA
 * CaseDescription: start two service abilities, call query function.
 */
HWTEST_F(RunningInfosTest, GetExtensionRunningInfos_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("device", "com.ix.hiExtension", "hiExtension");
    want.SetElement(element);
    abilityMs_->StartAbility(want);

    ElementName element2("device", "com.ix.hiExtension", "hiExtensionOther");
    want.SetElement(element2);
    auto result2 = abilityMs_->StartAbility(want);

    if (result2 == OHOS::ERR_OK) {
        std::vector<ExtensionRunningInfo> infos;
        int upperLimit = 10;
        abilityMs_->GetExtensionRunningInfos(upperLimit, infos);
        size_t infoCount{ 2 };
        EXPECT_TRUE(infos.size() == infoCount);
        if (infos.size() == infoCount) {
            EXPECT_TRUE(infos[0].extension.GetAbilityName() == element.GetAbilityName());
            EXPECT_TRUE(infos[1].extension.GetAbilityName() == element2.GetAbilityName());
        }
    }
}

/*
 * @tc.name: GetAbilityRunningInfos_006
 * @tc.desc: GetAbilityRunningInfos Test
 * @tc.type: FUNC
 * @tc.require: issueI5PXW4
 */
HWTEST_F(RunningInfosTest, GetProcessRunningInfos_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("device", "com.ix.hiExtension", "hiExtension");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);

    if (result == OHOS::ERR_OK) {
        std::vector<RunningProcessInfo> infos;
        auto ret = abilityMs_->GetProcessRunningInfos(infos);
        EXPECT_EQ(OHOS::ERR_OK, ret);
    }
}

/*
 * Feature: AbilityConnectManager
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start service ability, call query function.
 */
HWTEST_F(RunningInfosTest, ConnectManagerGetAbilityRunningInfos_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("device", "com.ix.hiService", "ServiceAbility");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);

    if (result == OHOS::ERR_OK) {
        std::vector<AbilityRunningInfo> infos;
        abilityMs_->subManagersHelper_->currentConnectManager_->GetAbilityRunningInfos(infos, true);
        size_t infoCount{ 1 };
        EXPECT_TRUE(infos.size() == infoCount);
        if (infos.size() == infoCount) {
            EXPECT_TRUE(infos[0].ability.GetAbilityName() == element.GetAbilityName());
            EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
        }
    }
}

/*
 * Feature: AbilityConnectManager
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start two service abilities, call query function.
 */
HWTEST_F(RunningInfosTest, ConnectManagerGetAbilityRunningInfos_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("device", "com.ix.hiService", "ServiceAbility");
    want.SetElement(element);
    abilityMs_->StartAbility(want);

    ElementName element2("device", "com.ix.hiServiceOther", "ServiceAbilityOther");
    want.SetElement(element2);
    auto result2 = abilityMs_->StartAbility(want);

    if (result2 == OHOS::ERR_OK) {
        std::vector<AbilityRunningInfo> infos;
        abilityMs_->subManagersHelper_->currentConnectManager_->GetAbilityRunningInfos(infos, true);

        size_t infoCount{ 2 };
        EXPECT_TRUE(infos.size() == infoCount);
        if (infos.size() == infoCount) {
            EXPECT_TRUE(infos[0].ability.GetAbilityName() == element.GetAbilityName());
            EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
            EXPECT_TRUE(infos[1].ability.GetAbilityName() == element2.GetAbilityName());
            EXPECT_TRUE(infos[1].abilityState == static_cast<int>(AbilityState::INITIAL));
        }
    }
}

/*
 * Feature: AbilityConnectManager
 * Function: GetExtensionRunningInfos
 * SubFunction: NA
 * FunctionPoints:query extension running infos
 * EnvConditions: NA
 * CaseDescription: start service ability, call query function.
 */
HWTEST_F(RunningInfosTest, ConnectManagerGetExtensionRunningInfos_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("device", "com.ix.hiExtension", "hiExtension");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);

    if (result == OHOS::ERR_OK) {
        std::vector<ExtensionRunningInfo> infos;
        int upperLimit = 10;
        int userId = 100;
        size_t infoCount{ 1 };
        abilityMs_->subManagersHelper_->currentConnectManager_->GetExtensionRunningInfos(
            upperLimit, infos, userId, true);
        EXPECT_TRUE(infos.size() == infoCount);
        if (infos.size() == infoCount) {
            EXPECT_TRUE(infos[0].extension.GetAbilityName() == element.GetAbilityName());
        }
    }
}

/*
 * Feature: AbilityConnectManager
 * Function: GetExtensionRunningInfos
 * SubFunction: NA
 * FunctionPoints:query extension running infos
 * EnvConditions: NA
 * CaseDescription: start two service abilities, call query function.
 */
HWTEST_F(RunningInfosTest, ConnectManagerGetExtensionRunningInfos_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("device", "com.ix.hiExtension", "hiExtension");
    want.SetElement(element);
    abilityMs_->StartAbility(want);

    ElementName element2("device", "com.ix.hiExtension", "hiExtensionOther");
    want.SetElement(element2);
    auto result2 = abilityMs_->StartAbility(want);

    if (result2 == OHOS::ERR_OK) {
        std::vector<ExtensionRunningInfo> infos;
        int upperLimit = 10;
        int userId = 100;
        size_t infoCount{ 2 };
        abilityMs_->subManagersHelper_->currentConnectManager_->GetExtensionRunningInfos(
            upperLimit, infos, userId, true);
        EXPECT_TRUE(infos.size() == infoCount);
        if (infos.size() == infoCount) {
            EXPECT_TRUE(infos[0].extension.GetAbilityName() == element.GetAbilityName());
            EXPECT_TRUE(infos[1].extension.GetAbilityName() == element2.GetAbilityName());
        }
    }
}

/*
 * Feature: MissionListManager
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start page ability, call query function.
 */
HWTEST_F(RunningInfosTest, MissionGetAbilityRunningInfos_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);

    if (result == OHOS::ERR_OK) {
        std::vector<AbilityRunningInfo> infos;
        abilityMs_->subManagersHelper_->currentMissionListManager_->GetAbilityRunningInfos(infos, true);
        size_t infoCount{ 1 };
        EXPECT_TRUE(infos.size() == infoCount);
        if (infos.size() == infoCount) {
            EXPECT_TRUE(infos[0].ability.GetAbilityName() == element.GetAbilityName());
            EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
        }
    }
}

/*
 * Feature: MissionListManager
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: start two page abilities, call query function.
 */
HWTEST_F(RunningInfosTest, MissionGetAbilityRunningInfos_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);

    if (result == OHOS::ERR_OK) {
        auto topAbility = abilityMs_->subManagersHelper_->currentMissionListManager_->GetCurrentTopAbilityLocked();
        EXPECT_TRUE(topAbility);
        topAbility->SetAbilityState(AbilityState::FOREGROUND);
    }

    ElementName element2("device", "com.ix.hiMusicOther", "MusicAbilityOther");
    want.SetElement(element2);
    auto result2 = abilityMs_->StartAbility(want);

    if (result2 == OHOS::ERR_OK) {
        std::vector<AbilityRunningInfo> infos;
        abilityMs_->subManagersHelper_->currentMissionListManager_->GetAbilityRunningInfos(infos, true);

        size_t infoCount{ 2 };
        EXPECT_TRUE(infos.size() == infoCount);
        if (infos.size() == infoCount) {
            EXPECT_TRUE(infos[0].ability.GetAbilityName() == element2.GetAbilityName());
            EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
            EXPECT_TRUE(infos[1].ability.GetAbilityName() == element.GetAbilityName());
            EXPECT_TRUE(infos[1].abilityState == static_cast<int>(AbilityState::FOREGROUND));
        }
    }
}

/*
 * Feature: DataAbilityManager
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: dataAbilityRecordsLoading insert data, call query function.
 */
HWTEST_F(RunningInfosTest, DataGetAbilityRunningInfos_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);

    AbilityRequest abilityRequest;
    int userId = 100;
    abilityMs_->GenerateAbilityRequest(want, -1, abilityRequest, nullptr, userId);
    DataAbilityManager::DataAbilityRecordPtr dataAbilityRecord;
    dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(abilityRequest.want,
        abilityRequest.abilityInfo,
        abilityRequest.appInfo,
        abilityRequest.requestCode);
    dataAbilityRecord->ability_ = abilityRecord;
    const std::string dataAbilityName(abilityRequest.abilityInfo.bundleName + '.' + abilityRequest.abilityInfo.name);
    abilityMs_->subManagersHelper_->currentDataAbilityManager_->dataAbilityRecordsLoading_.insert(
        std::pair<std::string, std::shared_ptr<DataAbilityRecord>>(dataAbilityName, dataAbilityRecord));

    std::vector<AbilityRunningInfo> infos;
    abilityMs_->subManagersHelper_->currentDataAbilityManager_->GetAbilityRunningInfos(infos, true);
    size_t infoCount{ 1 };
    EXPECT_TRUE(infos.size() == infoCount);
    if (infos.size() == infoCount) {
        EXPECT_TRUE(infos[0].ability.GetAbilityName() == element.GetAbilityName());
        EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
    }
}

/*
 * Feature: DataAbilityManager
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: dataAbilityRecordsLoaded insert data, call query function.
 */
HWTEST_F(RunningInfosTest, DataGetAbilityRunningInfos_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);

    AbilityRequest abilityRequest;
    int userId = 100;
    abilityMs_->GenerateAbilityRequest(want, -1, abilityRequest, nullptr, userId);
    DataAbilityManager::DataAbilityRecordPtr dataAbilityRecord;
    dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(abilityRequest.want,
        abilityRequest.abilityInfo,
        abilityRequest.appInfo,
        abilityRequest.requestCode);
    dataAbilityRecord->ability_ = abilityRecord;
    const std::string dataAbilityName(abilityRequest.abilityInfo.bundleName + '.' + abilityRequest.abilityInfo.name);
    abilityMs_->subManagersHelper_->currentDataAbilityManager_->dataAbilityRecordsLoaded_.insert(
        std::pair<std::string, std::shared_ptr<DataAbilityRecord>>(dataAbilityName, dataAbilityRecord));

    std::vector<AbilityRunningInfo> infos;
    abilityMs_->subManagersHelper_->currentDataAbilityManager_->GetAbilityRunningInfos(infos, true);
    size_t infoCount{ 1 };
    EXPECT_TRUE(infos.size() == infoCount);
    if (infos.size() == infoCount) {
        EXPECT_TRUE(infos[0].ability.GetAbilityName() == element.GetAbilityName());
        EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
    }
}

/*
 * Feature: DataAbilityManager
 * Function: GetAbilityRunningInfos
 * SubFunction: NA
 * FunctionPoints:query ability running infos
 * EnvConditions: NA
 * CaseDescription: insert abilities, call query function.
 */
HWTEST_F(RunningInfosTest, DataGetAbilityRunningInfos_003, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    abilityMs_->OnStart();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);

    AbilityRequest abilityRequest;
    int userId = 100;
    abilityMs_->GenerateAbilityRequest(want, -1, abilityRequest, nullptr, userId);
    DataAbilityManager::DataAbilityRecordPtr dataAbilityRecord;
    dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(abilityRequest.want,
        abilityRequest.abilityInfo,
        abilityRequest.appInfo,
        abilityRequest.requestCode);
    dataAbilityRecord->ability_ = abilityRecord;
    const std::string dataAbilityName(abilityRequest.abilityInfo.bundleName + '.' + abilityRequest.abilityInfo.name);
    abilityMs_->subManagersHelper_->currentDataAbilityManager_->dataAbilityRecordsLoading_.insert(
        std::pair<std::string, std::shared_ptr<DataAbilityRecord>>(dataAbilityName, dataAbilityRecord));

    ElementName element2("device", "com.ix.hiMusic", "MusicAbilityOther");
    want.SetElement(element2);
    AbilityRequest abilityRequest2;
    abilityMs_->GenerateAbilityRequest(want, -1, abilityRequest2, nullptr, userId);
    DataAbilityManager::DataAbilityRecordPtr dataAbilityRecord2;
    dataAbilityRecord2 = std::make_shared<DataAbilityRecord>(abilityRequest2);
    std::shared_ptr<AbilityRecord> abilityRecord2 = std::make_shared<AbilityRecord>(abilityRequest2.want,
        abilityRequest2.abilityInfo,
        abilityRequest2.appInfo,
        abilityRequest2.requestCode);
    dataAbilityRecord2->ability_ = abilityRecord2;
    const std::string dataAbilityName2(abilityRequest2.abilityInfo.bundleName + '.' + abilityRequest2.abilityInfo.name);
    abilityMs_->subManagersHelper_->currentDataAbilityManager_->dataAbilityRecordsLoaded_.insert(
        std::pair<std::string, std::shared_ptr<DataAbilityRecord>>(dataAbilityName2, dataAbilityRecord2));

    std::vector<AbilityRunningInfo> infos;
    abilityMs_->subManagersHelper_->currentDataAbilityManager_->GetAbilityRunningInfos(infos, true);
    size_t infoCount{ 2 };
    EXPECT_TRUE(infos.size() == infoCount);
    if (infos.size() == infoCount) {
        EXPECT_TRUE(infos[0].ability.GetAbilityName() == element.GetAbilityName());
        EXPECT_TRUE(infos[0].abilityState == static_cast<int>(AbilityState::INITIAL));
        EXPECT_TRUE(infos[1].ability.GetAbilityName() == element2.GetAbilityName());
        EXPECT_TRUE(infos[1].abilityState == static_cast<int>(AbilityState::INITIAL));
    }
}
}  // namespace AAFwk
}  // namespace OHOS
