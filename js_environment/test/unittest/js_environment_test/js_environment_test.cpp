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

#include "js_environment.h"

#include <gtest/gtest.h>
#include <cstdarg>
#include <string>

#include "ecmascript/napi/include/jsnapi.h"
#include "ohos_js_env_logger.h"
#include "ohos_js_environment_impl.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace JsEnv {
class JsEnvironmentTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void JsEnvironmentTest::SetUpTestCase()
{
    AbilityRuntime::OHOSJsEnvLogger::RegisterJsEnvLogger();
}

void JsEnvironmentTest::TearDownTestCase()
{}

void JsEnvironmentTest::SetUp()
{}

void JsEnvironmentTest::TearDown()
{}

/**
 * @tc.name: JsEnvInitialize_0100
 * @tc.desc: Initialize js environment.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(JsEnvironmentTest, JsEnvInitialize_0100, TestSize.Level0)
{
    auto jsEnvImpl = std::make_shared<AbilityRuntime::OHOSJsEnvironmentImpl>();
    auto jsEnv = std::make_shared<JsEnvironment>(jsEnvImpl);
    ASSERT_NE(jsEnv, nullptr);

    panda::RuntimeOption pandaOption;
    auto ret = jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    ASSERT_EQ(ret, true);

    auto vm = jsEnv->GetVM();
    EXPECT_NE(vm, nullptr);

    auto nativeEngine = jsEnv->GetNativeEngine();
    EXPECT_NE(nativeEngine, nullptr);
}
}  // namespace JsEnv
}  // namespace OHOS
