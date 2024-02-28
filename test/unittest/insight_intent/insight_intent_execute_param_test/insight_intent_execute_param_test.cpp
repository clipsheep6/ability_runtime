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
#include "want_params_wrapper.h"
#include "insight_intent_execute_param.h"
#include "int_wrapper.h"
#include "string_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
using InsightIntentExecuteParam = AppExecFwk::InsightIntentExecuteParam;

namespace {
const std::string TEST_BUNDLE_NANE = "test.bundleName";
const std::string TEST_MODULE_NANE = "test.entry";
const std::string TEST_ABILITY_NANE = "test.abilityName";
const std::string TEST_CALLER_BUNDLE_NANE = "test.callerBundleName";
const std::string TEST_INSIGHT_INTENT_NANE = "PlayMusic";
} // namespace

class InsightIntentExecuteParamTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void InsightIntentExecuteParamTest::SetUpTestCase(void)
{}

void InsightIntentExecuteParamTest::TearDownTestCase(void)
{}

void InsightIntentExecuteParamTest::SetUp()
{}

void InsightIntentExecuteParamTest::TearDown()
{}

/**
 * @tc.name: GenerateFromWant_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require: issueI91RLM
 */
HWTEST_F(InsightIntentExecuteParamTest, GenerateFromWant_0100, TestSize.Level1)
{
    HILOG_INFO("begin.");
    WantParams wantParams;
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, AAFwk::String::Box(TEST_INSIGHT_INTENT_NANE));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, AAFwk::String::Box("1"));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_TOKEN, Integer::Box(1000));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_UID, Integer::Box(1001));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_PID, Integer::Box(1002));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, AAFwk::String::Box(TEST_CALLER_BUNDLE_NANE));

    WantParams insightIntentParam;
    insightIntentParam.SetParam("dummy", Integer::Box(-1));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM, WantParamWrapper::Box(insightIntentParam));

    Want want;
    want.SetElementName("", TEST_BUNDLE_NANE, TEST_ABILITY_NANE, TEST_MODULE_NANE);
    want.SetParams(wantParams);

    InsightIntentExecuteParam executeParam;
    auto ret = InsightIntentExecuteParam::GenerateFromWant(want, executeParam);
    EXPECT_EQ(ret, true);

    // check execute param
    EXPECT_EQ(executeParam.bundleName_, TEST_BUNDLE_NANE);
    EXPECT_EQ(executeParam.moduleName_, TEST_MODULE_NANE);
    EXPECT_EQ(executeParam.abilityName_, TEST_ABILITY_NANE);
    EXPECT_EQ(executeParam.insightIntentName_, TEST_INSIGHT_INTENT_NANE);
    EXPECT_EQ(executeParam.insightIntentId_, 1);

    // check caller info
    std::shared_ptr<WantParams> insightIntentParamGot = executeParam.insightIntentParam_;
    ASSERT_NE(insightIntentParamGot, nullptr);
    EXPECT_EQ(insightIntentParamGot->GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0), 1000);
    EXPECT_EQ(insightIntentParamGot->GetIntParam(Want::PARAM_RESV_CALLER_UID, 0), 1001);
    EXPECT_EQ(insightIntentParamGot->GetIntParam(Want::PARAM_RESV_CALLER_PID, 0), 1002);
    EXPECT_EQ(insightIntentParamGot->GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME), TEST_CALLER_BUNDLE_NANE);

    HILOG_INFO("end.");
}

/**
 * @tc.name: UpdateInsightIntentCallerInfo_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require: issueI91RLM
 */
HWTEST_F(InsightIntentExecuteParamTest, UpdateInsightIntentCallerInfo_0100, TestSize.Level1)
{
    HILOG_INFO("begin.");
    WantParams wantParams;
    wantParams.SetParam(Want::PARAM_RESV_CALLER_TOKEN, Integer::Box(1000));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_UID, Integer::Box(1001));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_PID, Integer::Box(1002));
    wantParams.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, AAFwk::String::Box(TEST_CALLER_BUNDLE_NANE));

    WantParams insightIntentParam;
    InsightIntentExecuteParam::UpdateInsightIntentCallerInfo(wantParams, insightIntentParam);

    // check caller info
    EXPECT_EQ(insightIntentParam.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0), 1000);
    EXPECT_EQ(insightIntentParam.GetIntParam(Want::PARAM_RESV_CALLER_UID, 0), 1001);
    EXPECT_EQ(insightIntentParam.GetIntParam(Want::PARAM_RESV_CALLER_PID, 0), 1002);
    EXPECT_EQ(insightIntentParam.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME), TEST_CALLER_BUNDLE_NANE);

    HILOG_INFO("end.");
}
} // namespace AAFwk
} // namespace OHOS
