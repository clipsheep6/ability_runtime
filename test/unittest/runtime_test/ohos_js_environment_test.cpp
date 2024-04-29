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

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"

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
        TAG_LOGI(AAFwkTag::TEST, "%{public}s called.", name.c_str());
    };
    int64_t delayTime = 1000;
    jsEnvImpl->PostTask(task, taskName, delayTime);
    jsEnvImpl->RemoveTask(taskName);
}

/**
 * @tc.name: PostSyncTask_0100
 * @tc.desc: Js environment post sync task.
 * @tc.type: FUNC
 * @tc.require: issueI7C87T
 */
HWTEST_F(OHOSJsEnvironmentTest, PostSyncTask_0100, TestSize.Level0)
{
    auto runner = AppExecFwk::EventRunner::Create("TASK_RUNNER");
    ASSERT_NE(runner, nullptr);
    auto jsEnvImpl = std::make_shared<OHOSJsEnvironmentImpl>(runner);
    ASSERT_NE(jsEnvImpl, nullptr);

    AbilityRuntime::Runtime::Options options;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);
    auto ret = jsEnvImpl->InitLoop(jsRuntime->GetNativeEnginePointer());
    ASSERT_EQ(ret, true);

    std::string taskName = "syncTask001";
    bool taskExecuted = false;
    auto task = [taskName, &taskExecuted]() {
        TAG_LOGI(AAFwkTag::TEST, "%{public}s called.", taskName.c_str());
        taskExecuted = true;
    };
    jsEnvImpl->PostSyncTask(task, taskName);
    EXPECT_EQ(taskExecuted, true);
}

/**
 * @tc.name: InitTimerModule_0100
 * @tc.desc: Js environment init timer.
 * @tc.type: FUNC
 * @tc.require: issueI6Z5M5
 */
HWTEST_F(OHOSJsEnvironmentTest, InitTimerModule_0100, TestSize.Level0)
{
    auto jsEnvImpl = std::make_unique<OHOSJsEnvironmentImpl>();
    ASSERT_NE(jsEnvImpl, nullptr);

    // Init timer module when native engine is invalid.
    jsEnvImpl->InitTimerModule(nullptr);

    AbilityRuntime::Runtime::Options options;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);

    // Init timer module when native engine has created.
    jsEnvImpl->InitTimerModule(jsRuntime->GetNativeEnginePointer());
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
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(runtime, nullptr);
    auto jsEngine = runtime->GetNativeEnginePointer();
    std::shared_ptr<JsEnv::WorkerInfo> workerInfo = std::make_shared<JsEnv::WorkerInfo>();
    jsEnvImpl->InitWorkerModule(jsEngine, workerInfo);
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
