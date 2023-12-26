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
#include <gtest/hwext/gtest-multithread.h>
#include <cstdarg>
#include <string>

#include "ecmascript/napi/include/jsnapi.h"
#include "js_env_logger.h"
#include "ohos_js_env_logger.h"
#include "ohos_js_environment_impl.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;

namespace {
bool callbackModuleFlag;
}

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

namespace {
void CallBackModuleFunc()
{
    callbackModuleFlag = true;
}
}

/**
 * @tc.name: JsEnvInitialize_0100
 * @tc.desc: Initialize js environment.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(JsEnvironmentTest, JsEnvInitialize_0100, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);
    ASSERT_EQ(jsEnv->GetVM(), nullptr);
    ASSERT_EQ(jsEnv->GetNativeEngine(), nullptr);

    panda::RuntimeOption pandaOption;
    auto ret = jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    ASSERT_EQ(ret, true);

    auto vm = jsEnv->GetVM();
    EXPECT_NE(vm, nullptr);

    auto nativeEngine = jsEnv->GetNativeEngine();
    EXPECT_NE(nativeEngine, nullptr);
}

/**
 * @tc.name: JsEnvInitialize_0200
 * @tc.desc: Initialize js environment in multi thread.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(JsEnvironmentTest, JsEnvInitialize_0200, TestSize.Level0)
{
    JSENV_LOG_I("Running in multi-thread, using default thread number.");

    auto task = []() {
        JSENV_LOG_I("Running in thread %{public}" PRIu64 "", gettid());
        auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
        ASSERT_NE(jsEnv, nullptr);

        panda::RuntimeOption pandaOption;
        ASSERT_EQ(jsEnv->Initialize(pandaOption, nullptr), true);
        EXPECT_NE(jsEnv->GetVM(), nullptr);
        EXPECT_NE(jsEnv->GetNativeEngine(), nullptr);
    };

    GTEST_RUN_TASK(task);
}

/**
 * @tc.name: LoadScript_0100
 * @tc.desc: load script with invalid engine.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(JsEnvironmentTest, LoadScript_0100, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    EXPECT_EQ(jsEnv->LoadScript(""), false);
}

/**
 * @tc.name: LoadScript_0200
 * @tc.desc: load script with invalid path.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(JsEnvironmentTest, LoadScript_0200, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    panda::RuntimeOption pandaOption;
    auto ret = jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    ASSERT_EQ(ret, true);

    EXPECT_EQ(jsEnv->LoadScript(""), false);
}

/**
 * @tc.name: LoadScript_0300
 * @tc.desc: load script with specify path.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(JsEnvironmentTest, LoadScript_0300, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    panda::RuntimeOption pandaOption;
    auto ret = jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    ASSERT_EQ(ret, true);

    EXPECT_EQ(jsEnv->LoadScript("/system/etc/strip.native.min.abc"), true);
}

/**
 * @tc.name: JsEnvInitTimerModule_0100
 * @tc.desc: Initialize timer module.
 * @tc.type: FUNC
 * @tc.require: issueI6Z5M5
 */
HWTEST_F(JsEnvironmentTest, JsEnvInitTimerModule_0100, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    // Init timer module when native engine is invalid.
    jsEnv->InitTimerModule();

    panda::RuntimeOption pandaOption;
    auto ret = jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    ASSERT_EQ(ret, true);

    // Init timer module when native engine has created.
    jsEnv->InitTimerModule();
}

/**
 * @tc.name: PostTask_0100
 * @tc.desc: PostTask
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, PostTask_0100, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    // Init timer module when native engine is invalid.
    std::function<void()> task = CallBackModuleFunc;
    std::string name = "NAME";
    int64_t delayTime = 10;
    jsEnv->PostTask(task, name, delayTime);
}

/**
 * @tc.name: RemoveTask_0100
 * @tc.desc: RemoveTask
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, RemoveTask_0100, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    std::string name = "NAME";
    jsEnv->RemoveTask(name);
}

/**
 * @tc.name: InitSyscapModule_0100
 * @tc.desc: InitSyscapModule
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, InitSyscapModule_0100, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    jsEnv->InitSyscapModule();
}

/**
 * @tc.name: RegisterUncaughtExceptionHandler_0100
 * @tc.desc: RegisterUncaughtExceptionHandler
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, RegisterUncaughtExceptionHandler_0100, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    JsEnv::UncaughtExceptionInfo uncaughtExceptionInfo;
    jsEnv->RegisterUncaughtExceptionHandler(uncaughtExceptionInfo);
}

/**
 * @tc.name: StartDebugger_0100
 * @tc.desc: StartDebugger
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, StartDebugger_0100, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    std::string option = "ark:1234@Debugger";
    const char* libraryPath = "LIBRARYPATH";
    uint32_t socketFd = 10;
    bool needBreakPoint = true;
    uint32_t instanceId = 10;
    bool result = jsEnv->StartDebugger(option, libraryPath, socketFd, needBreakPoint, instanceId);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: StopDebugger_0100
 * @tc.desc: StopDebugger
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, StopDebugger_0100, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    jsEnv->StopDebugger();
}

/**
 * @tc.name: InitConsoleModule_0100
 * @tc.desc: InitConsoleModule
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(JsEnvironmentTest, InitConsoleModule_0100, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    jsEnv->InitConsoleModule();

    panda::RuntimeOption pandaOption;
    auto ret = jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    ASSERT_EQ(ret, true);

    jsEnv->InitConsoleModule();
}

/**
 * @tc.name: StartProfiler_0100
 * @tc.desc: StartProfiler
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, StartProfiler_0100, TestSize.Level1)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    const char* libraryPath = "LIBRARYPATH";
    jsEnv->StartProfiler(libraryPath, 0, JsEnvironment::PROFILERTYPE::PROFILERTYPE_CPU, 0, 0, true);
    ASSERT_EQ(jsEnv->GetVM(), nullptr);
}

/**
 * @tc.name: StartProfiler_0200
 * @tc.desc: StartProfiler
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, StartProfiler_0200, TestSize.Level1)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    panda::RuntimeOption pandaOption;
    auto ret = jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    ASSERT_EQ(ret, true);

    const char* libraryPath = "LIBRARYPATH";
    jsEnv->StartProfiler(libraryPath, 0, JsEnvironment::PROFILERTYPE::PROFILERTYPE_HEAP, 0, 0, true);
    ASSERT_NE(jsEnv->GetVM(), nullptr);
}

/**
 * @tc.name: PostSyncTask_0100
 * @tc.desc: Js environment post sync task.
 * @tc.type: FUNC
 * @tc.require: issueI7C87T
 */
HWTEST_F(JsEnvironmentTest, PostSyncTask_0100, TestSize.Level0)
{
    auto runner = AppExecFwk::EventRunner::Create("TASK_RUNNER");
    ASSERT_NE(runner, nullptr);
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>(runner));
    ASSERT_NE(jsEnv, nullptr);
    panda::RuntimeOption pandaOption;
    ASSERT_EQ(jsEnv->Initialize(pandaOption, static_cast<void*>(this)), true);
    ASSERT_EQ(jsEnv->InitLoop(), true);

    std::string taskName = "syncTask001";
    bool taskExecuted = false;
    auto task = [taskName, &taskExecuted]() {
        JSENV_LOG_I("%{public}s called.", taskName.c_str());
        taskExecuted = true;
    };
    jsEnv->PostSyncTask(task, taskName);
    EXPECT_EQ(taskExecuted, true);
}

/**
 * @tc.name: SetRequestAotCallback_0100
 * @tc.desc: Js environment SetRequestAotCallback.
 * @tc.type: FUNC
 * @tc.require: issueI82L1A
 */
HWTEST_F(JsEnvironmentTest, SetRequestAotCallback_0100, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);

    auto callback = [](const std::string& bundleName, const std::string& moduleName, int32_t triggerMode) -> int32_t {
        JSENV_LOG_I("set request aot callback.");
        return 0;
    };
    jsEnv->SetRequestAotCallback(callback);
}

/**
 * @tc.name: ParseHdcRegisterOption_0100
 * @tc.desc: Js environment ParseHdcRegisterOption.
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentTest, ParseHdcRegisterOption_0100, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);
    std::string option1 = "";
    int result1 = jsEnv->ParseHdcRegisterOption(option1);
    ASSERT_EQ(result1, -1);
    std::string option2 = "@";
    int result2 = jsEnv->ParseHdcRegisterOption(option2);
    ASSERT_EQ(result2, -1);
    std::string option3 = ":";
    int result3 = jsEnv->ParseHdcRegisterOption(option3);
    ASSERT_EQ(result3, -1);
    std::string option4 = "ark:123@Debugger";
    int result4 = jsEnv->ParseHdcRegisterOption(option4);
    ASSERT_EQ(result4, 123);
    std::string option5 = "ark:123@456@Debugger";
    int result5 = jsEnv->ParseHdcRegisterOption(option5);
    ASSERT_EQ(result5, 456);
}

/**
 * @tc.name: SetDeviceDisconnectCallback_0100
 * @tc.desc: Js environment SetDeviceDisconnectCallback.
 * @tc.type: FUNC
 * @tc.require: issueI82L1A
 */
HWTEST_F(JsEnvironmentTest, SetDeviceDisconnectCallback_0100, TestSize.Level0)
{
    auto jsEnv = std::make_shared<JsEnvironment>(std::make_unique<AbilityRuntime::OHOSJsEnvironmentImpl>());
    ASSERT_NE(jsEnv, nullptr);
    panda::RuntimeOption pandaOption;
    auto ret = jsEnv->Initialize(pandaOption, static_cast<void*>(this));
    ASSERT_EQ(ret, true);

    bool taskExecuted = false;
    auto task = [&taskExecuted]() {
        return true;
    };
    jsEnv->SetDeviceDisconnectCallback(task);
    ASSERT_EQ(taskExecuted, false);
}
} // namespace JsEnv
} // namespace OHOS
