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
#include <gmock/gmock.h>
#include "insight_intent_context.h"
#include "ability_manager_client.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class InsightIntentContextTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void InsightIntentContextTest::SetUpTestCase(void)
{}
void InsightIntentContextTest::TearDownTestCase(void)
{}
void InsightIntentContextTest::SetUp(void)
{}
void InsightIntentContextTest::TearDown(void)
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
    EXPECT_CALL(*(AAFwk::AbilityManagerClient::GetInstance()), StartAbilityByInsightIntent(_, _, _))
        .WillOnce(Return(ERR_OK));
    sptr<IRemoteObject> token = nullptr;
    std::string bundleName = "";
    int32_t winMode = 0;
    uint64_t intentId = 0;
    std::shared_ptr<InsightIntentContext> insightIntentContext =
        std::make_shared<InsightIntentContext>(token, bundleName, winMode, intentId);
    AAFwk::Want want;
    EXPECT_EQ(insightIntentContext->StartAbilityByInsightIntent(want), ERR_OK);
}

/*
 * Feature: InsightIntentContext
 * Function: StartAbilityByInsightIntent
 * SubFunction: NA
 * FunctionPoints: StartAbilityByInsightIntent in normal case
 * EnvConditions: NA
 * CaseDescription: Verify if function acts as expect when abilitymanagerclient invoking failed.
 */
HWTEST_F(InsightIntentContextTest, StartAbilityByInsightIntent_002, TestSize.Level1)
{
    EXPECT_CALL(*(AAFwk::AbilityManagerClient::GetInstance()), StartAbilityByInsightIntent(_, _, _))
        .WillOnce(Return(ERR_INVALID_VALUE));
    sptr<IRemoteObject> token = nullptr;
    std::string bundleName = "";
    int32_t winMode = 0;
    uint64_t intentId = 0;
    std::shared_ptr<InsightIntentContext> insightIntentContext =
        std::make_shared<InsightIntentContext>(token, bundleName, winMode, intentId);
    AAFwk::Want want;
    EXPECT_EQ(insightIntentContext->StartAbilityByInsightIntent(want), ERR_INVALID_VALUE);
}

/*
 * Feature: InsightIntentContext
 * Function: GetBundleName
 * SubFunction: NA
 * FunctionPoints: GetBundleName in normal case
 * EnvConditions: NA
 * CaseDescription: Verify if function acts as expect.
 */
HWTEST_F(InsightIntentContextTest, GetBundleName_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    std::string bundleName = "testBundleName";
    int32_t winMode = 0;
    uint64_t intentId = 0;
    std::shared_ptr<InsightIntentContext> insightIntentContext =
        std::make_shared<InsightIntentContext>(token, bundleName, winMode, intentId);
    EXPECT_EQ(insightIntentContext->GetBundleName(), bundleName);
}

/*
 * Feature: InsightIntentContext
 * Function: GetBundleName
 * SubFunction: NA
 * FunctionPoints: GetBundleName in normal case
 * EnvConditions: NA
 * CaseDescription: Verify if function acts as expect.
 */
HWTEST_F(InsightIntentContextTest, GetBundleName_002, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    std::string bundleName = "";
    int32_t winMode = 0;
    uint64_t intentId = 0;
    std::shared_ptr<InsightIntentContext> insightIntentContext =
        std::make_shared<InsightIntentContext>(token, bundleName, winMode, intentId);
    EXPECT_EQ(insightIntentContext->GetBundleName(), bundleName);
}

/*
 * Feature: InsightIntentContext
 * Function: GetCurrentWindowMode
 * SubFunction: NA
 * FunctionPoints: GetCurrentWindowMode in normal case
 * EnvConditions: NA
 * CaseDescription: Verify if function acts as expect.
 */
HWTEST_F(InsightIntentContextTest, GetCurrentWindowMode_001, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    std::string bundleName = "";
    int32_t winMode = 0;
    uint64_t intentId = 0;
    std::shared_ptr<InsightIntentContext> insightIntentContext =
        std::make_shared<InsightIntentContext>(token, bundleName, winMode, intentId);
    EXPECT_EQ(insightIntentContext->GetCurrentWindowMode(), winMode);
}

/*
 * Feature: InsightIntentContext
 * Function: GetCurrentWindowMode
 * SubFunction: NA
 * FunctionPoints: GetCurrentWindowMode in normal case
 * EnvConditions: NA
 * CaseDescription: Verify if function acts as expect.
 */
HWTEST_F(InsightIntentContextTest, GetCurrentWindowMode_002, TestSize.Level1)
{
    sptr<IRemoteObject> token = nullptr;
    std::string bundleName = "";
    int32_t winMode = 1;
    uint64_t intentId = 0;
    std::shared_ptr<InsightIntentContext> insightIntentContext =
        std::make_shared<InsightIntentContext>(token, bundleName, winMode, intentId);
    EXPECT_EQ(insightIntentContext->GetCurrentWindowMode(), winMode);
}
} // namespace AbilityRuntime
} // namespace OHOS