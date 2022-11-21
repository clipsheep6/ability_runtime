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

// #define OHOS_ABILITY_RUNTIME_JS_RUNTIME_UTILS_H

#include <gtest/gtest.h>
#include "js_runtime_utils.h"
// #define private public
// #define protected public
// #include "js_ability_context.h"
// #undef private
// #undef protected

using namespace testing;
using namespace testing::ext;
// using namespace OHOS::AppExecFwk;
// using OHOS::AppExecFwk::ElementName;

// namespace OHOS {
// namespace AAFwk {
// namespace {
// template<class T>
// T* CheckParamsAndGetThis(NativeEngine* engine, NativeCallbackInfo* info, const char* name = nullptr)
// {
//     if (engine == nullptr || info == nullptr) {
//         return nullptr;
//     }
//     JsAbilityContext jsAbilityContext = new JsAbilityContext(nullptr);
//     return jsAbilityContext;
// }
// } // namespace
class JsAbilityContextTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void JsAbilityContextTest::SetUpTestCase()
{
    MockNativeEngine engine;

}

void JsAbilityContextTest::TearDownTestCase()
{}

void JsAbilityContextTest::SetUp()
{}

void JsAbilityContextTest::TearDown()
{}

/*
 * @tc.name: JsAbilityContextTest_0100
 * @tc.desc: test if JsAbilityContext::StartAbility function executed as expected in normal case.
 * @tc.type: FUNC
 * @tc.require: #I5SJ62
 *
 */
HWTEST_F(JsAbilityContextTest, JsAbilityContextTest_0100, TestSize.Level1)
{

}
// } // namespace AAFwk
// } // namespace OHOS
