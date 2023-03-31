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

#include "ohos_js_environment_impl.h"

#include <gtest/gtest.h>
#include <cstdarg>
#include <string>

#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class OHOSJsEnvironmentTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void OHOSJsEnvironmentTest::SetUpTestCase()
{}

void OHOSJsEnvironmentTest::TearDownTestCase()
{}

void OHOSJsEnvironmentTest::SetUp()
{}

void OHOSJsEnvironmentTest::TearDown()
{}

/**
 * @tc.name: PostTask_0100
 * @tc.desc: Js environment post and remove task.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(OHOSJsEnvironmentTest, PostTask_0100, TestSize.Level0)
{
    auto jsEnvImpl = std::make_shared<OHOSJsEnvironmentImpl>();
    ASSERT_NE(jsEnvImpl, nullptr);

    std::string taskName = "task001";
    auto task = [name = taskName]() {
        HILOG_INFO("%{public}s called.", name.c_str());
    };
    int64_t delayTime = 1000;
    jsEnvImpl->PostTask(task, taskName, delayTime);
    jsEnvImpl->RemoveTask(taskName);
}

/**
 * @tc.name: InitTimerModule_0100
 * @tc.desc: Js environment init timer.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(OHOSJsEnvironmentTest, InitTimerModule_0100, TestSize.Level0)
{
    auto jsEnvImpl = std::make_shared<OHOSJsEnvironmentImpl>();
    ASSERT_NE(jsEnvImpl, nullptr);

    jsEnvImpl->InitTimerModule();
}

/**
 * @tc.name: InitConsoleLogModule_0100
 * @tc.desc: Js environment init console log.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(OHOSJsEnvironmentTest, InitConsoleLogModule_0100, TestSize.Level0)
{
    auto jsEnvImpl = std::make_shared<OHOSJsEnvironmentImpl>();
    ASSERT_NE(jsEnvImpl, nullptr);

    jsEnvImpl->InitConsoleLogModule();
}

/**
 * @tc.name: InitWorkerModule_0100
 * @tc.desc: Js environment init worker.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(OHOSJsEnvironmentTest, InitWorkerModule_0100, TestSize.Level0)
{
    auto jsEnvImpl = std::make_shared<OHOSJsEnvironmentImpl>();
    ASSERT_NE(jsEnvImpl, nullptr);

    jsEnvImpl->InitWorkerModule();
}

/**
 * @tc.name: InitSyscapModule_0100
 * @tc.desc: Js environment init syscap.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(OHOSJsEnvironmentTest, InitSyscapModule_0100, TestSize.Level0)
{
    auto jsEnvImpl = std::make_shared<OHOSJsEnvironmentImpl>();
    ASSERT_NE(jsEnvImpl, nullptr);

    jsEnvImpl->InitSyscapModule();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
