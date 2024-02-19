/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "hilog_wrapper.h"
#include "want.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_execute_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class InsightIntentExecuteManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void InsightIntentExecuteManagerTest::SetUpTestCase(void)
{}

void InsightIntentExecuteManagerTest::TearDownTestCase(void)
{}

void InsightIntentExecuteManagerTest::SetUp()
{}

void InsightIntentExecuteManagerTest::TearDown()
{}

/**
 * @tc.name: GenerateWant_0100
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerTest, GenerateWant_0100, TestSize.Level1)
{
    HILOG_INFO("begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    // other member has default value.
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    Want want;
    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, want);
    EXPECT_EQ(ret, ERR_OK);
    // get display id of want, expect don't contain
    auto displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -100);
    EXPECT_EQ(displayId, -100);
    HILOG_INFO("end.");
}

/**
 * @tc.name: GenerateWant_0200
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerTest, GenerateWant_0200, TestSize.Level1)
{
    HILOG_INFO("begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    param.displayId_ = 2;
    // other member has default value.
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    Want want;
    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, want);
    EXPECT_EQ(ret, ERR_OK);
    // get display id of want
    auto displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -100);
    EXPECT_EQ(displayId, 2);
    HILOG_INFO("end.");
}
} // namespace AAFwk
} // namespace OHOS