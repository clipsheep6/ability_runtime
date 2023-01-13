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
#include <stack>
#include "ability_transaction_callback_info.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;

namespace {
    int32_t g_count = 0;
}

class AbilityTransactionCallbackInfoTest : public testing::Test {
public:
    AbilityTransactionCallbackInfoTest() = default;
    ~AbilityTransactionCallbackInfoTest() override = default;

    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    void SetUp() override {};
    void TearDown() override {};
};

/**
 * @tc.name: AaFwk_AbilityTransactionCallbackInfoTest_Call_0100
 * @tc.desc: Call all transaction callback.
 * @tc.type: FUNC
 * @tc.require: I5OGBZ
 */
HWTEST_F(AbilityTransactionCallbackInfoTest, AaFwk_AbilityTransactionCallbackInfoTest_Call_0100, Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityTransactionCallbackInfoTest_Call_0100 start";

    auto* callbackInfo = AbilityTransactionCallbackInfo<>::Create();
    EXPECT_NE(callbackInfo, nullptr);
    g_count = 0;
    auto asyncCallback = []() {
        g_count++;
    };

    const int32_t PUSH_COUNT = 10;
    for (int32_t i = 0; i < PUSH_COUNT; i++) {
        callbackInfo->Push(asyncCallback);
    }

    EXPECT_EQ(g_count, 0);
    callbackInfo->Call();
    EXPECT_EQ(g_count, PUSH_COUNT);
    AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    GTEST_LOG_(INFO) << "AaFwk_AbilityTransactionCallbackInfoTest_Call_0100 end";
}

/**
 * @tc.name: AaFwk_AbilityTransactionCallbackInfoTest_Call_0100
 * @tc.desc: Call all transaction callback.
 * @tc.type: FUNC
 * @tc.require: I65V83
 */
HWTEST_F(AbilityTransactionCallbackInfoTest, AaFwk_AbilityTransactionCallbackInfoTest_Call_0200, Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityTransactionCallbackInfoTest_Call_0200 start";

    auto* callbackInfo = AbilityTransactionCallbackInfo<std::stack<int32_t>>::Create();
    EXPECT_NE(callbackInfo, nullptr);
    g_count = 0;

    auto asyncCallback = [](std::stack<int32_t> &result) {
        if (!result.empty()) {
            g_count += result.top();
            result.pop();
        }
    };

    const int32_t PUSH_COUNT = 10;
    std::stack<int32_t> result {};
    int32_t sum = 0;
    for (int32_t i = 0; i < PUSH_COUNT; i++) {
        callbackInfo->Push(asyncCallback);
        result.push(i);
        sum += i;
    }

    EXPECT_EQ(g_count, 0);
    callbackInfo->Call(result);
    EXPECT_EQ(g_count, sum);
    AbilityTransactionCallbackInfo<std::stack<int32_t>>::Destroy(callbackInfo);
    GTEST_LOG_(INFO) << "AaFwk_AbilityTransactionCallbackInfoTest_Call_0200 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
