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
#include "ability_info.h"
#include "want.h"
#include "application_info.h"
#include "ability_record.h"
#include "dlp_state_item.h"

using namespace testing::ext;
namespace OHOS {
namespace AAFwk {

class DlpStateItemTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DlpStateItemTest::SetUpTestCase()
{}

void DlpStateItemTest::TearDownTestCase()
{}

void DlpStateItemTest::SetUp()
{}

void DlpStateItemTest::TearDown()
{}

/*
 * Feature: DLP State Item
 * Function: AddDlpConnectionState AddDlpConnectionState
 * SubFunction: HandleDlpConnectionState
 * FunctionPoints:DLP State Item HandleDlpConnectionState
 * EnvConditions: NA
 * CaseDescription: Verify andleDlpConnectionState
 */
HWTEST_F(DlpStateItemTest, dlp_state_item_test_handle_001, TestSize.Level1)
{
    auto item = std::make_shared<DlpStateItem>(1, 2);
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    abilityRecord->SetAppIndex(1);
    AbilityRuntime::DlpStateData data;
    EXPECT_FALSE(item->RemoveDlpConnectionState(abilityRecord, data));
    EXPECT_TRUE(item->AddDlpConnectionState(abilityRecord, data));
    EXPECT_FALSE(item->AddDlpConnectionState(abilityRecord, data));
    EXPECT_TRUE(item->RemoveDlpConnectionState(abilityRecord, data));
    EXPECT_FALSE(item->RemoveDlpConnectionState(abilityRecord, data));
}

/*
 * Feature: DLP State Item
 * Function: GetOpenedAbilitySize
 * SubFunction: NA
 * FunctionPoints:DLP State Item GetOpenedAbilitySize
 * EnvConditions: NA
 * CaseDescription: Verify GetOpenedAbilitySize
 */
HWTEST_F(DlpStateItemTest, dlp_state_item_test_get_size_002, TestSize.Level1)
{
    auto item = std::make_shared<DlpStateItem>(1, 2);
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    AbilityRuntime::DlpStateData data;
    abilityRecord->SetAppIndex(0);
    EXPECT_FALSE(item->AddDlpConnectionState(abilityRecord, data));
    abilityRecord->SetAppIndex(1);
    EXPECT_TRUE(item->AddDlpConnectionState(abilityRecord, data));
    auto item2 = std::make_shared<DlpStateItem>(0, 0);
    EXPECT_FALSE(item2->AddDlpConnectionState(abilityRecord, data));
}

/*
 * Feature: DLP State Item
 * Function: GetDlpUid
 * SubFunction: H
 * FunctionPoints:DLP State ItemGetDlpUid
 * EnvConditions: NA
 * CaseDescription: Verify GetDlpUid
 */
HWTEST_F(DlpStateItemTest, dlp_state_item_test_get_uid_001, TestSize.Level1)
{
    auto item = std::make_shared<DlpStateItem>(10, 2);
    EXPECT_EQ(10, item->GetDlpUid());
}

/*
 * Feature: DLP State Item
 * Function: GetOpenedAbilitySize
 * SubFunction: GenerateDlpStateData
 * FunctionPoints:DLP State Item GetOpenedAbilitySize
 * EnvConditions: NA
 * CaseDescription: Verify GetOpenedAbilitySize
 */
HWTEST_F(DlpStateItemTest, dlp_state_item_test_get_size_001, TestSize.Level1)
{
    auto item = std::make_shared<DlpStateItem>(1, 2);
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.launchMode = AppExecFwk::LaunchMode::STANDARD;
    Want want;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    abilityRecord->SetAppIndex(1);
    AbilityRuntime::DlpStateData data;
    EXPECT_EQ(0, item->GetOpenedAbilitySize());
    EXPECT_TRUE(item->AddDlpConnectionState(abilityRecord, data));
    EXPECT_LT(0, item->GetOpenedAbilitySize());
    EXPECT_TRUE(item->RemoveDlpConnectionState(abilityRecord, data));
    EXPECT_EQ(0, item->GetOpenedAbilitySize());
}
}  // namespace AppExecFwk
}  // namespace OHOS
