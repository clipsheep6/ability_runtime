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
#define protected public
#include "ability_manager_service.h"
#undef private
#undef protected

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class AbilityManagerServiceInsightIntentTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityManagerServiceInsightIntentTest::SetUpTestCase(void)
{}
void AbilityManagerServiceInsightIntentTest::TearDownTestCase(void)
{}
void AbilityManagerServiceInsightIntentTest::SetUp(void)
{}
void AbilityManagerServiceInsightIntentTest::TearDown(void)
{}

/*
 * Feature: InsightIntentContext
 * Function: StartAbilityByInsightIntent
 * SubFunction: NA
 * FunctionPoints: StartAbilityByInsightIntent in normal case
 * EnvConditions: NA
 * CaseDescription: Verify if function acts as expect in normal case.
 */
HWTEST_F(InsightIntentContextTest, StartAbilityByInsightIntent_001, TestSize.Level1)
{
    std::string bundleNameFromWant = "testBundleName";
    std::string bundleNameFromIntentMgr = "testBundleName";
    std::string bundleNameFromAbilityRecord = "testBundleName";
    EXPECT_CALL(*(DelayedSingleton<InsightIntentExecuteManager>::GetInstance()), GetBundleName())
        .WillOnce(testing::DoAll(SetArgReferee<1>(bundleNameFromIntentMgr), Return(ERR_OK)));
    std::shared_ptr<AbilityManagerService> abilityManagerService = std::make_shared<AbilityManagerService>();
    sptr<IRemoteObject> token = nullptr;
    AAFwk::Want want;
    want.SetElementName("testAbilityName", bundleNameFromWant);
    EXPECT_EQ(abilityManagerService->StartAbilityByInsightIntent(want, nullptr, 0, 0), ERR_OK);
}
} // namespace AbilityRuntime
} // namespace OHOS