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

#include "hilog_tag_wrapper.h"

using namespace testing::ext;
using UTAAFwkTag = OHOS::AAFwk::AAFwkLogTag;

namespace OHOS {
namespace AbilityRuntime {
class AbilityServiceLogTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityServiceLogTest::SetUpTestCase(void)
{}

void AbilityServiceLogTest::TearDownTestCase(void)
{}

void AbilityServiceLogTest::SetUp()
{}

void AbilityServiceLogTest::TearDown()
{}

/*
 * Feature: HILOG
 * Function: Create
 * SubFunction: NA
 * FunctionPoints: Create
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityServiceLogTest, Log_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityServiceLog_0100 start";
    TAG_LOGI(AAFwkTag::TEST, "AbilityServiceLog_0100 info start");
    TAG_LOGD(AAFwkTag::TEST, "AbilityServiceLog_0100 debug start");
    TAG_LOGW(AAFwkTag::TEST, "AbilityServiceLog_0100 warn start");
    TAG_LOGE(AAFwkTag::TEST, "AbilityServiceLog_0100 error start");
    TAG_LOGF(AAFwkTag::TEST, "AbilityServiceLog_0100 fatal start");
    GTEST_LOG_(INFO) << "AbilityServiceLog_0100 end";
}

/*
 * Feature: HILOG
 * Function: Create
 * SubFunction: NA
 * FunctionPoints: Create
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityServiceLogTest, Log_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityServiceLog_0200 start";
    TAG_LOGI(AAFwkTag::END, "AbilityServiceLog_0200 info start");
    TAG_LOGD(AAFwkTag::END, "AbilityServiceLog_0200 debug start");
    TAG_LOGW(AAFwkTag::END, "AbilityServiceLog_0200 warn start");
    TAG_LOGE(AAFwkTag::END, "AbilityServiceLog_0200 error start");
    TAG_LOGF(AAFwkTag::END, "AbilityServiceLog_0200 fatal start");
    GTEST_LOG_(INFO) << "AbilityServiceLog_0200 end";
}

/*
 * Feature: HILOG
 * Function: Create
 * SubFunction: NA
 * FunctionPoints: Create
 * EnvConditions: NA
 * CaseDescription: NA
 */
HWTEST_F(AbilityServiceLogTest, Log_0300, TestSize.Level1)
{
    size_t tag = 255;
    GTEST_LOG_(INFO) << "AbilityServiceLog_0300 start";
    TAG_LOGI(static_cast<AAFwkTag>(tag), "AbilityServiceLog_0300 info start");
    TAG_LOGD(static_cast<AAFwkTag>(tag), "AbilityServiceLog_0300 debug start");
    TAG_LOGW(static_cast<AAFwkTag>(tag), "AbilityServiceLog_0300 warn start");
    TAG_LOGE(static_cast<AAFwkTag>(tag), "AbilityServiceLog_0300 error start");
    TAG_LOGF(static_cast<AAFwkTag>(tag), "AbilityServiceLog_0300 fatal start");
    GTEST_LOG_(INFO) << "AbilityServiceLog_0300 end";
}
}  // namespace AbilityRuntime
}  // namespace OHOS
