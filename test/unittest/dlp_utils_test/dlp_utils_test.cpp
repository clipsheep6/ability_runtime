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

#ifdef WITH_DLP
#include "dlp_utils.h"
#endif // WITH_DLP

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class DlpUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void DlpUtilsTest::SetUpTestCase()
{}

void DlpUtilsTest::TearDownTestCase()
{}

void DlpUtilsTest::SetUp()
{}

void DlpUtilsTest::TearDown()
{}

#ifdef WITH_DLP
/**
 * @tc.name: OtherAppsAccessDlpCheck_0100
 * @tc.desc: OtherAppsAccessDlpCheck Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(DlpUtilsTest, OtherAppsAccessDlpCheck_0100, TestSize.Level0)
{
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    abilityRecord->SetAppIndex(1);
    Want want;
    bool result = DlpUtils::OtherAppsAccessDlpCheck(abilityRecord->GetToken(), want);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: OtherAppsAccessDlpCheck_0200
 * @tc.desc: OtherAppsAccessDlpCheck Test
 * @tc.type: FUNC
 * @tc.require: issueI581RO
 */
HWTEST_F(DlpUtilsTest, OtherAppsAccessDlpCheck_0200, TestSize.Level0)
{
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    Want want;
    bool result = DlpUtils::OtherAppsAccessDlpCheck(abilityRecord->GetToken(), want);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: DlpAccessOtherAppsCheck_0100
 * @tc.desc: DlpAccessOtherAppsCheck test
 * @tc.type: FUNC
 * @tc.require: issueI581RO
 */
HWTEST_F(DlpUtilsTest, DlpAccessOtherAppsCheck_0100, TestSize.Level0)
{
    AbilityRequest abilityRequest;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    EXPECT_NE(abilityRecord, nullptr);
    Want want;
    bool result = DlpUtils::DlpAccessOtherAppsCheck(abilityRecord->GetToken(), want);
    EXPECT_TRUE(result);
}
#endif // WITH_DLP
}  // namespace AAFwk
}  // namespace OHOS
