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

#include "napi_base_context.h"
#include "hilog_wrapper.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class NapiBaseContextTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void NapiBaseContextTest::SetUpTestCase(void)
{}

void NapiBaseContextTest::TearDownTestCase(void)
{}

void NapiBaseContextTest::SetUp()
{}

void NapiBaseContextTest::TearDown()
{}

/**
 * @tc.name: IsStageContext_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBaseContextTest, IsStageContext_0100, TestSize.Level1)
{
    HILOG_INFO("IsStageContext start");

    napi_env env = nullptr;
    bool stageMode = false;
    napi_value args[0] = {};
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, args[0], stageMode);
    EXPECT_NE(status, napi_ok);
    EXPECT_FALSE(stageMode);

    HILOG_INFO("IsStageContext end");
}

/**
 * @tc.name: GetStageModeContext_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBaseContextTest, GetStageModeContext_0100, TestSize.Level1)
{
    HILOG_INFO("GetStageModeContext start");

    napi_env env = nullptr;
    napi_value argv[0] = {};
    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, argv[0]);
    EXPECT_EQ(context, nullptr);

    HILOG_INFO("GetStageModeContext end");
}

/**
 * @tc.name: GetCurrentAbility_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBaseContextTest, GetCurrentAbility_0100, TestSize.Level1)
{
    HILOG_INFO("GetCurrentAbility start");

    napi_env env = nullptr;
    auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
    EXPECT_EQ(ability, nullptr);

    HILOG_INFO("GetCurrentAbility end");
}

}  // namespace AbilityRuntime
}  // namespace OHOS
