/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include <gtest/hwext/gtest-multithread.h>

#define private public
#define protected public
#include "js_environment.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_worker.h"
#undef private
#undef protected
#include "event_runner.h"
#include "mock_js_runtime.h"
#include "mock_jsnapi.h"
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string TEST_BUNDLE_NAME = "com.ohos.contactsdataability";
const std::string TEST_MODULE_NAME = ".ContactsDataAbility";
const std::string TEST_ABILITY_NAME = "ContactsDataAbility";
const std::string TEST_CODE_PATH = "/data/storage/el1/bundle";
const std::string TEST_HAP_PATH = "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
const std::string TEST_LIB_PATH = "/data/storage/el1/bundle/lib/";
const std::string TEST_MODULE_PATH = "/data/storage/el1/bundle/curJsModulePath";
}  // namespace
class JsRuntimeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    Runtime::Options options_;
};

void JsRuntimeTest::SetUpTestCase()
{}

void JsRuntimeTest::TearDownTestCase()
{}

void JsRuntimeTest::SetUp()
{
    options_.bundleName = TEST_BUNDLE_NAME;
    options_.codePath = TEST_CODE_PATH;
    options_.loadAce = false;
    options_.isBundle = true;
    options_.preload = false;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);
    options_.eventRunner = eventRunner;
}

void JsRuntimeTest::TearDown()
{}

/**
 * @tc.name: JsperfProfilerCommandParse_100
 * @tc.desc: JsRuntime test for JsperfProfilerCommandParse.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsperfProfilerCommandParse_100, TestSize.Level1)
{
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options_);
    std::string command = "";
    constexpr int32_t defaultVal = 500;
    constexpr int32_t emptyVal = 0;
    ASSERT_EQ(jsRuntime->JsperfProfilerCommandParse(command, defaultVal), emptyVal);
    command = "jsperfabc";
    ASSERT_EQ(jsRuntime->JsperfProfilerCommandParse(command, defaultVal), defaultVal);
    command = "jsperf";
    ASSERT_EQ(jsRuntime->JsperfProfilerCommandParse(command, defaultVal), defaultVal);
    command = "jsperf ";
    ASSERT_EQ(jsRuntime->JsperfProfilerCommandParse(command, defaultVal), defaultVal);
    command = "jsperf 1000";
    ASSERT_NE(jsRuntime->JsperfProfilerCommandParse(command, defaultVal), defaultVal);
    command = " jsperf 1000";
    ASSERT_NE(jsRuntime->JsperfProfilerCommandParse(command, defaultVal), defaultVal);
}

/**
 * @tc.name: JsRuntimeTest_0100
 * @tc.desc: JsRuntime Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(JsRuntimeTest, JsRuntimeTest_0100, TestSize.Level0)
{
    options_.preload = true;
    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime = nullptr;
    options_.preload = false;
    jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);
}

/**
 * @tc.name: JsRuntimeTest_0200
 * @tc.desc: JsRuntime Test
 * @tc.type: FUNC
 * @tc.require: issueI581RO
 */
HWTEST_F(JsRuntimeTest, JsRuntimeTest_0200, TestSize.Level0)
{
    std::string appLibPathKey = TEST_BUNDLE_NAME + TEST_MODULE_NAME;
    std::string libPath = TEST_LIB_PATH;

    AppLibPathMap appLibPaths {};
    JsRuntime::SetAppLibPath(appLibPaths);

    appLibPaths[appLibPathKey].emplace_back(libPath);
    EXPECT_NE(appLibPaths.size(), 0);
    JsRuntime::SetAppLibPath(appLibPaths);
}

/**
 * @tc.name: JsRuntimeUtilsTest_0100
 * @tc.desc: JsRuntimeUtils Test
 * @tc.type: FUNC
 * @tc.require: issueI581RO
 */
HWTEST_F(JsRuntimeTest, JsRuntimeUtilsTest_0100, TestSize.Level0)
{
    auto runtime = AbilityRuntime::Runtime::Create(options_);
    auto env = (static_cast<AbilityRuntime::JsRuntime&>(*runtime)).GetNapiEnv();

    napi_ref callbackRef = nullptr;
    napi_create_reference(env, CreateJsUndefined(env), 1, &callbackRef);
    std::unique_ptr<NapiAsyncTask> task = std::make_unique<NapiAsyncTask>(callbackRef, nullptr, nullptr);
    task->ResolveWithNoError(env, CreateJsUndefined(env));
    EXPECT_TRUE(task->callbackRef_ == nullptr);

    napi_deferred nativeDeferred = nullptr;
    napi_value result;
    napi_create_promise(env, &nativeDeferred, &result);
    task = std::make_unique<NapiAsyncTask>(nativeDeferred, nullptr, nullptr);
    task->ResolveWithNoError(env, CreateJsUndefined(env));
    EXPECT_TRUE(task->deferred_ == nullptr);

    task->deferred_ = nullptr;
    task->callbackRef_ = nullptr;
    task->ResolveWithNoError(env, CreateJsUndefined(env));
    EXPECT_TRUE(task->deferred_ == nullptr);
    EXPECT_TRUE(task->callbackRef_ == nullptr);
}

/**
 * @tc.name: JsRuntimeGetLanguageTest_0100
 * @tc.desc: JsRuntime Test
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeGetLanguageTest_0100, TestSize.Level0)
{
    options_.preload = true;
    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    JsRuntime::Language language = jsRuntime->GetLanguage();
    EXPECT_TRUE(language == JsRuntime::Language::JS);
}

/**
 * @tc.name: JsRuntimeBuildJsStackInfoListTest_0100
 * @tc.desc: JsRuntime test for BuildJsStackInfoList.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeBuildJsStackInfoListTest_0100, TestSize.Level0)
{
    HILOG_INFO("Test BuildJsStackInfoList start");
    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    std::vector<JsFrames> frames;
    bool ret = jsRuntime->BuildJsStackInfoList(gettid(), frames);
    EXPECT_FALSE(ret);
    HILOG_INFO("Test BuildJsStackInfoList end");
}

/**
 * @tc.name: JsRuntimeNotifyApplicationStateTest_0100
 * @tc.desc: JsRuntime test for NotifyApplicationState when nativeEngine is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeNotifyApplicationStateTest_0100, TestSize.Level0)
{
    HILOG_INFO("NotifyApplicationState start");

    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<MockJsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    bool isBackground = false;
    jsRuntime->NotifyApplicationState(isBackground);

    HILOG_INFO("NotifyApplicationState end");
}

/**
 * @tc.name: JsRuntimeNotifyApplicationStateTest_0200
 * @tc.desc: JsRuntime test for NotifyApplicationState when nativeEngine is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeNotifyApplicationStateTest_0200, TestSize.Level0)
{
    HILOG_INFO("NotifyApplicationState start");

    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    bool isBackground = true;
    jsRuntime->NotifyApplicationState(isBackground);

    HILOG_INFO("NotifyApplicationState end");
}

/**
 * @tc.name: JsRuntimeDumpHeapSnapshotTest_0100
 * @tc.desc: JsRuntime test for DumpHeapSnapshot.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeDumpHeapSnapshotTest_0100, TestSize.Level0)
{
    HILOG_INFO("DumpHeapSnapshot start");

    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    bool isPrivate = true;
    jsRuntime->DumpHeapSnapshot(isPrivate);

    HILOG_INFO("DumpHeapSnapshot end");
}

/**
 * @tc.name: JsRuntimePreloadSystemModuleTest_0100
 * @tc.desc: JsRuntime test for PreloadSystemModule.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimePreloadSystemModuleTest_0100, TestSize.Level0)
{
    HILOG_INFO("PreloadSystemModule start");

    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string moduleName = "PreloadSystemModuleTest";
    jsRuntime->PreloadSystemModule(moduleName);

    HILOG_INFO("PreloadSystemModule end");
}

/**
 * @tc.name: JsRuntimeRunSandboxScriptTest_0100
 * @tc.desc: JsRuntime test for RunSandboxScript.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeRunSandboxScriptTest_0100, TestSize.Level0)
{
    HILOG_INFO("RunSandboxScript start");

    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string path = "";
    std::string hapPath = "";
    bool ret = (static_cast<AbilityRuntime::JsRuntime&>(*jsRuntime)).RunSandboxScript(path, hapPath);
    EXPECT_FALSE(ret);

    HILOG_INFO("RunSandboxScript end");
}

/**
 * @tc.name: JsRuntimeLoadSystemModuleByEngineTest_0100
 * @tc.desc: JsRuntime test for LoadSystemModuleByEngine.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadSystemModuleByEngineTest_0100, TestSize.Level0)
{
    HILOG_INFO("LoadSystemModuleByEngine start");

    auto runtime = AbilityRuntime::JsRuntime::Create(options_);
    auto env = (static_cast<AbilityRuntime::MockJsRuntime&>(*runtime)).GetNapiEnv();

    std::string moduleName = "";
    std::unique_ptr<NativeReference> ref = MockJsRuntime::LoadSystemModuleByEngine(env, moduleName, nullptr, 0);
    EXPECT_EQ(ref, nullptr);

    HILOG_INFO("LoadSystemModuleByEngine end");
}

/**
 * @tc.name: JsRuntimeLoadModuleTest_0100
 * @tc.desc: JsRuntime test for LoadModule.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadModuleTest_0100, TestSize.Level0)
{
    HILOG_INFO("LoadModule start");

    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string moduleName = TEST_MODULE_NAME;
    std::string modulePath = TEST_MODULE_PATH;
    std::string hapPath = TEST_HAP_PATH;
    bool esmodule = true;
    std::unique_ptr<NativeReference> ref = (static_cast<AbilityRuntime::JsRuntime&>(*jsRuntime)).LoadModule(moduleName,
        modulePath, hapPath, esmodule);
    EXPECT_EQ(ref, nullptr);

    HILOG_INFO("LoadModule end");
}

/**
 * @tc.name: JsRuntimeLoadSystemModuleTest_0100
 * @tc.desc: JsRuntime test for LoadSystemModule (invoke the overwrite interface).
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadSystemModuleTest_0100, TestSize.Level0)
{
    HILOG_INFO("LoadSystemModule start");

    MockJsRuntime mockJsRuntime;
    std::unique_ptr<NativeReference> ref = mockJsRuntime.LoadSystemModule("", nullptr, 0);
    EXPECT_EQ(ref, nullptr);

    HILOG_INFO("LoadSystemModule end");
}

/**
 * @tc.name: RuntimeSavePreloadedTest_0100
 * @tc.desc: Runtime test for SavePreloaded.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RuntimeSavePreloadedTest_0100, TestSize.Level0)
{
    HILOG_INFO("SavePreloaded start");

    auto runtime = AbilityRuntime::Runtime::Create(options_);
    runtime->SavePreloaded(nullptr);
    EXPECT_TRUE(runtime != nullptr);

    HILOG_INFO("SavePreloaded end");
}

/**
 * @tc.name: RuntimeSetModuleLoadCheckerTest_0100
 * @tc.desc: Runtime test for SetModuleLoadChecker.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RuntimeSetModuleLoadCheckerTest_0100, TestSize.Level0)
{
    HILOG_INFO("SetModuleLoadChecker start");

    auto runtime = AbilityRuntime::Runtime::Create(options_);
    runtime->SetModuleLoadChecker(nullptr);
    EXPECT_TRUE(runtime != nullptr);

    HILOG_INFO("SetModuleLoadChecker end");
}

/**
 * @tc.name: JsRuntimeSuspendVMTest_0100
 * @tc.desc: JsRuntime test for SuspendVM.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeSuspendVMTest_0100, TestSize.Level0)
{
    HILOG_INFO("SuspendVM start");

    auto runtime = AbilityRuntime::JsRuntime::Create(options_);
    auto result = runtime->SuspendVM(gettid());
    EXPECT_EQ(result, false);

    HILOG_INFO("SuspendVM end");
}

/**
 * @tc.name: JsRuntimeResumeVMTest_0100
 * @tc.desc: JsRuntime test for ResumeVM.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeResumeVMTest_0100, TestSize.Level0)
{
    HILOG_INFO("ResumeVM start");

    auto runtime = AbilityRuntime::JsRuntime::Create(options_);
    runtime->ResumeVM(gettid());
    EXPECT_TRUE(runtime != nullptr);

    HILOG_INFO("ResumeVM end");
}

/**
 * @tc.name: JsRuntimeSetDeviceDisconnectCallbackTest_0100
 * @tc.desc: JsRuntime test for SetDeviceDisconnectCallback.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeSetDeviceDisconnectCallbackTest_0100, TestSize.Level0)
{
    HILOG_INFO("SetDeviceDisconnectCallback start");

    auto runtime = AbilityRuntime::JsRuntime::Create(options_);
    std::function<bool()> task = [&]() {
        return true;
    };
    runtime->SetDeviceDisconnectCallback(task);
    EXPECT_TRUE(runtime != nullptr);

    HILOG_INFO("SetDeviceDisconnectCallback end");
}

/**
 * @tc.name: JsRuntimeDetachCallbackFuncTest_0100
 * @tc.desc: JsRuntime test for PostTask.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeDetachCallbackFuncTest_0100, TestSize.Level0)
{
    HILOG_INFO("DetachCallbackFunc start");

    auto runtime = AbilityRuntime::JsRuntime::Create(options_);
    auto env = (static_cast<AbilityRuntime::MockJsRuntime&>(*runtime)).GetNapiEnv();
    int32_t value = 1;
    int32_t number = 1;
    auto result = AbilityRuntime::DetachCallbackFunc(env, &value, &number);
    EXPECT_EQ(result, &value);

    HILOG_INFO("DetachCallbackFunc end");
}

/**
 * @tc.name: JsRuntimeLoadSystemModulesTest_0100
 * @tc.desc: JsRuntime test for LoadSystemModule.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadSystemModulesTest_0100, TestSize.Level0)
{
    HILOG_INFO("LoadSystemModule start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string moduleName = "PreloadSystemModuleTest";
    napi_value object = nullptr;
    std::unique_ptr<NativeReference> ref = jsRuntime->LoadSystemModule(moduleName, &object, 0);
    EXPECT_EQ(ref, nullptr);

    HILOG_INFO("LoadSystemModule end");
}

/**
 * @tc.name: JsRuntimeStartDebugModeTest_0100
 * @tc.desc: JsRuntime test for StartDebugMode.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeStartDebugModeTest_0100, TestSize.Level0)
{
    HILOG_INFO("StartDebugMode start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    bool needBreakPoint = true;
    bool debugApp = true;
    const std::string processName = "test";
    jsRuntime->StartDebugMode(needBreakPoint, processName, debugApp);
    jsRuntime->StopDebugMode();

    HILOG_INFO("StartDebugMode end");
}

/**
 * @tc.name: JsRuntimeLoadRepairPatchTest_0100
 * @tc.desc: JsRuntime test for LoadRepairPatch.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadRepairPatchTest_0100, TestSize.Level0)
{
    HILOG_INFO("LoadRepairPatch start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string hqfFile = "<hqfFile>";
    std::string hapPath = "<hapPath>";
    bool lrp = jsRuntime->LoadRepairPatch(hqfFile, hapPath);
    EXPECT_EQ(lrp, false);

    HILOG_INFO("LoadRepairPatch end");
}

/**
 * @tc.name: JsRuntimeUnLoadRepairPatchTest_0100
 * @tc.desc: JsRuntime test for UnLoadRepairPatch.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeUnLoadRepairPatchTest_0100, TestSize.Level0)
{
    HILOG_INFO("UnLoadRepairPatch start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string hqfFile = "<hqfFile>";
    bool lrp = jsRuntime->UnLoadRepairPatch(hqfFile);
    EXPECT_EQ(lrp, false);

    HILOG_INFO("UnLoadRepairPatch end");
}

/**
 * @tc.name: JsRuntimeNotifyHotReloadPageTest_0100
 * @tc.desc: JsRuntime test for NotifyHotReloadPage.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeNotifyHotReloadPageTest_0100, TestSize.Level0)
{
    HILOG_INFO("NotifyHotReloadPage start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    bool lrp = jsRuntime->NotifyHotReloadPage();
    EXPECT_EQ(lrp, true);

    HILOG_INFO("NotifyHotReloadPage end");
}

/**
 * @tc.name: JsRuntimeUpdateModuleNameAndAssetPathTest_0100
 * @tc.desc: JsRuntime test for UpdateModuleNameAndAssetPath.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeUpdateModuleNameAndAssetPathTest_0100, TestSize.Level0)
{
    HILOG_INFO("UpdateModuleNameAndAssetPath start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string moduleName = "moduleName";
    jsRuntime->UpdateModuleNameAndAssetPath(moduleName);

    HILOG_INFO("UpdateModuleNameAndAssetPath end");
}

/**
 * @tc.name: JsRuntimeInitialize_0100
 * @tc.desc: Initialize js runtime in multi thread.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(JsRuntimeTest, JsRuntimeInitialize_0100, TestSize.Level0)
{
    HILOG_INFO("Running in multi-thread, using default thread number.");

    auto task = []() {
        HILOG_INFO("Running in thread %{public}d", gettid());
        AbilityRuntime::Runtime::Options options;
        options.loadAce = false;
        options.preload = false;
        options.isStageModel = false;

        auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
        ASSERT_NE(jsRuntime, nullptr);
        EXPECT_NE(jsRuntime->GetEcmaVm(), nullptr);
        EXPECT_NE(jsRuntime->GetNativeEnginePointer(), nullptr);
    };

    GTEST_RUN_TASK(task);
}

/**
 * @tc.name: JsRuntimeInitialize_0200
 * @tc.desc: preload js runtime.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(JsRuntimeTest, JsRuntimeInitialize_0200, TestSize.Level0)
{
    AbilityRuntime::Runtime::Options options;
    options.preload = true;

    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);
    EXPECT_NE(jsRuntime->GetEcmaVm(), nullptr);
    EXPECT_NE(jsRuntime->GetNativeEnginePointer(), nullptr);

    options.preload = false;
    jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);
    EXPECT_NE(jsRuntime->GetEcmaVm(), nullptr);
    EXPECT_NE(jsRuntime->GetNativeEnginePointer(), nullptr);
}

/**
 * @tc.name: RegisterQuickFixQueryFunc_0100
 * @tc.desc: JsRuntime test for RegisterQuickFixQueryFunc.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RegisterQuickFixQueryFunc_0100, TestSize.Level0)
{
    HILOG_INFO("RegisterQuickFixQueryFunc start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);
    std::string moudel = "<moudelName>";
    std::string hqfFile = "<hqfFile>";
    std::map<std::string, std::string> moduleAndPath;
    moduleAndPath.insert(std::make_pair(moudel, hqfFile));
    jsRuntime->RegisterQuickFixQueryFunc(moduleAndPath);

    HILOG_INFO("RegisterQuickFixQueryFunc end");
}

/**
 * @tc.name: RegisterUncaughtExceptionHandler_0100
 * @tc.desc: JsRuntime test for RegisterUncaughtExceptionHandler.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RegisterUncaughtExceptionHandler_0100, TestSize.Level0)
{
    HILOG_INFO("RegisterUncaughtExceptionHandler start");

    AbilityRuntime::Runtime::Options options;
    options.preload = false;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);
    JsEnv::UncaughtExceptionInfo uncaughtExceptionInfo;
    jsRuntime->RegisterUncaughtExceptionHandler(uncaughtExceptionInfo);
    HILOG_INFO("RegisterUncaughtExceptionHandler end");
}

/**
 * @tc.name: RegisterUncaughtExceptionHandler_0200
 * @tc.desc: JsRuntime test for RegisterUncaughtExceptionHandler.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RegisterUncaughtExceptionHandler_0200, TestSize.Level0)
{
    HILOG_INFO("RegisterUncaughtExceptionHandler start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);
    JsEnv::UncaughtExceptionInfo uncaughtExceptionInfo;
    jsRuntime->RegisterUncaughtExceptionHandler(uncaughtExceptionInfo);
    HILOG_INFO("RegisterUncaughtExceptionHandler end");
}

/**
 * @tc.name: ReadSourceMapData_0100
 * @tc.desc: JsRuntime test for ReadSourceMapData.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, ReadSourceMapData_0100, TestSize.Level0)
{
    HILOG_INFO("ReadSourceMapData start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);

    std::string hapPath = "";
    std::string sourceMapPath = "";
    std::string content = "";
    jsRuntime->ReadSourceMapData(hapPath, sourceMapPath, content);
    HILOG_INFO("ReadSourceMapData end");
}

/**
 * @tc.name: StopDebugger_0100
 * @tc.desc: JsRuntime test for StopDebugger.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, StopDebugger_0100, TestSize.Level0)
{
    HILOG_INFO("StopDebugger start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);

    jsRuntime->StopDebugger();
    HILOG_INFO("StopDebugger end");
}

/**
 * @tc.name: GetFileBuffer_0200
 * @tc.desc: JsRuntime test for GetFileBuffer.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, GetFileBuffer_0200, TestSize.Level0)
{
    HILOG_INFO("GetFileBuffer start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);

    std::string filePath = "";
    std::string fileFullName = "";
    std::vector<uint8_t> buffer;
    jsRuntime->GetFileBuffer(filePath, fileFullName, buffer);
    HILOG_INFO("GetFileBuffer end");
}

/**
 * @tc.name: JsRuntimeRunScriptTest_0100
 * @tc.desc: JsRuntime test for RunScript.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeRunScriptTest_0100, TestSize.Level0)
{
    HILOG_INFO("RunScript start");

    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string srcPath = "";
    std::string hapPath = "";
    bool useCommonChunk = true;
    bool ret = (static_cast<AbilityRuntime::JsRuntime&>(*jsRuntime)).RunScript(srcPath, hapPath, useCommonChunk);
    EXPECT_TRUE(ret);

    HILOG_INFO("RunScript end");
}

/**
 * @tc.name: JsRuntimeLoadScriptTest_0100
 * @tc.desc: JsRuntime test for LoadScript.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadScriptTest_0100, TestSize.Level0)
{
    AbilityRuntime::Runtime::Options options;
    options.preload = false;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);

    std::string path = "";
    std::vector<uint8_t>* buffer = nullptr;
    bool isBundle = true;
    jsRuntime->LoadScript(path, buffer, isBundle);
}

/**
 * @tc.name: JsRuntimeLoadScriptTest_0200
 * @tc.desc: JsRuntime test for LoadScript.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadScriptTest_0200, TestSize.Level0)
{
    AbilityRuntime::Runtime::Options options;
    options.preload = false;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);

    std::string path = "";
    uint8_t *buffer = nullptr;
    size_t len = 1;
    bool isBundle = true;
    jsRuntime->LoadScript(path, buffer, len, isBundle);
}

/**
 * @tc.name: JsRuntimeStopDebuggerTest_0100
 * @tc.desc: JsRuntime test for StopDebugger.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeStopDebuggerTest_0100, TestSize.Level0)
{
    AbilityRuntime::Runtime::Options options;
    options.preload = false;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);

    jsRuntime->StopDebugger();
}

/**
 * @tc.name: PostSyncTask_0100
 * @tc.desc: Js runtime post sync task.
 * @tc.type: FUNC
 * @tc.require: issueI7C87T
 */
HWTEST_F(JsRuntimeTest, PostSyncTask_0100, TestSize.Level0)
{
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options_);
    ASSERT_NE(jsRuntime, nullptr);

    std::string taskName = "syncTask001";
    bool taskExecuted = false;
    auto task = [taskName, &taskExecuted]() {
        HILOG_INFO("%{public}s called.", taskName.c_str());
        taskExecuted = true;
    };
    jsRuntime->PostSyncTask(task, taskName);
    EXPECT_EQ(taskExecuted, true);
}

/**
 * @tc.name: PostSyncTask_0200
 * @tc.desc: Js runtime post sync task in preload scene.
 * @tc.type: FUNC
 * @tc.require: issueI7C87T
 */
HWTEST_F(JsRuntimeTest, PostSyncTask_0200, TestSize.Level1)
{
    options_.preload = true;
    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    Runtime::SavePreloaded(std::move(jsRuntime));

    options_.preload = false;
    auto newJsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(newJsRuntime != nullptr);

    std::string taskName = "syncTask002";
    bool taskExecuted = false;
    auto task = [taskName, &taskExecuted]() {
        HILOG_INFO("%{public}s called.", taskName.c_str());
        taskExecuted = true;
    };
    newJsRuntime->PostSyncTask(task, taskName);
    EXPECT_EQ(taskExecuted, true);
}

/**
 * @tc.name: ReInitJsEnvImpl_0100
 * @tc.desc: Js runtime reinit js env impl.
 * @tc.type: FUNC
 * @tc.require: issueI7C87T
 */
HWTEST_F(JsRuntimeTest, ReInitJsEnvImpl_0100, TestSize.Level1)
{
    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    // called when jsEnv is invalid.
    jsRuntime->ReInitJsEnvImpl(options_);

    auto ret = jsRuntime->CreateJsEnv(options_);
    EXPECT_EQ(ret, true);
    jsRuntime->ReInitJsEnvImpl(options_);
}

/**
 * @tc.name: JsRuntimeStartProfilerTest_0100
 * @tc.desc: JsRuntime test for StartProfiler.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeStartProfilerTest_0100, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    options.preload = false;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    bool needBreakPoint = false;
    uint32_t instanceId = 1;
    jsRuntime->StartDebugger(needBreakPoint, instanceId);

    const std::string perfCmd = "profile jsperf 100";
    bool debugApp = true;
    const std::string processName = "test";
    jsRuntime->StartProfiler(perfCmd, needBreakPoint, processName, debugApp);
    ASSERT_NE(jsRuntime, nullptr);
}

/**
 * @tc.name: PostTask_0100
 * @tc.desc: Js runtime post task.
 * @tc.type: FUNC
 * @tc.require: issueI7C87T
 */
HWTEST_F(JsRuntimeTest, PostTask_0100, TestSize.Level0)
{
    HILOG_INFO("PostTask_0100 start");
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options_);
    ASSERT_NE(jsRuntime, nullptr);

    std::string taskName = "postTask001";
    bool taskExecuted = false;
    auto task = [taskName, &taskExecuted]() {
        HILOG_INFO("%{public}s called.", taskName.c_str());
        taskExecuted = true;
    };
    int64_t delayTime = 10;
    jsRuntime->PostTask(task, taskName, delayTime);
    EXPECT_NE(taskExecuted, true);
    HILOG_INFO("PostTask_0100 end");
}

/**
 * @tc.name: RemoveTask_0100
 * @tc.desc: Js runtime remove task.
 * @tc.type: FUNC
 * @tc.require: issueI7C87T
 */
HWTEST_F(JsRuntimeTest, RemoveTask_0100, TestSize.Level0)
{
    HILOG_INFO("RemoveTask_0100 start");
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options_);
    ASSERT_NE(jsRuntime, nullptr);

    std::string taskName = "removeTask001";
    bool taskExecuted = false;
    auto task = [taskName, &taskExecuted]() {
        HILOG_INFO("%{public}s called.", taskName.c_str());
        taskExecuted = true;
    };
    int64_t delayTime = 10;
    jsRuntime->PostTask(task, taskName, delayTime);
    jsRuntime->RemoveTask(taskName);
    EXPECT_NE(taskExecuted, true);
    HILOG_INFO("RemoveTask_0100 end");
}

/**
 * @tc.name: StartDebugger_0100
 * @tc.desc: JsRuntime test for StartDebugger.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, StartDebugger_0100, TestSize.Level0)
{
    HILOG_INFO("StartDebugger_0100 start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);

    bool needBreakPoint = false;
    uint32_t instanceId = 1;

    jsRuntime->StartDebugger(needBreakPoint, instanceId);
    // debug mode is global option, maybe has started by other testcase, not check here.
    HILOG_INFO("StartDebugger_0100 end");
}

/**
 * @tc.name: DoCleanWorkAfterStageCleaned_0100
 * @tc.desc: JsRuntime test for DoCleanWorkAfterStageCleaned.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, DoCleanWorkAfterStageCleaned_0100, TestSize.Level0)
{
    HILOG_INFO("DoCleanWorkAfterStageCleaned_0100 start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);

    jsRuntime->DoCleanWorkAfterStageCleaned();
    HILOG_INFO("DoCleanWorkAfterStageCleaned_0100 end");
}

/**
 * @tc.name: ReloadFormComponent_0100
 * @tc.desc: JsRuntime test for ReloadFormComponent.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, ReloadFormComponent_0100, TestSize.Level0)
{
    HILOG_INFO("ReloadFormComponent_0100 start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);

    jsRuntime->ReloadFormComponent();
    HILOG_INFO("ReloadFormComponent_0100 end");
}

/**
 * @tc.name: SetRequestAotCallback_0100
 * @tc.desc: JsRuntime test for SetRequestAotCallback.
 * @tc.type: FUNC
 * @tc.require: issueI82L1A
 */
HWTEST_F(JsRuntimeTest, SetRequestAotCallback_0100, TestSize.Level0)
{
    HILOG_INFO("start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);

    jsRuntime->SetRequestAotCallback();
    auto ret = panda::MockJSNApi::GetInstance()->RequestAot("bundleName", "moduleName", 0);
    EXPECT_NE(ret, -1);
    HILOG_INFO("finish");
}

/**
 * @tc.name: DestroyHeapProfiler_0100
 * @tc.desc: JsRuntime test for DestroyHeapProfiler.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, DestroyHeapProfiler_0100, TestSize.Level0)
{
    HILOG_INFO("DestroyHeapProfiler_0100 start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    jsRuntime->DestroyHeapProfiler();
    ASSERT_NE(jsRuntime, nullptr);
    HILOG_INFO("DestroyHeapProfiler_0100 end");
}

/**
 * @tc.name: ForceFullGC_0100
 * @tc.desc: JsRuntime test for ForceFullGC.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, ForceFullGC_0100, TestSize.Level0)
{
    HILOG_INFO("ForceFullGC_0100 start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    jsRuntime->ForceFullGC();
    ASSERT_NE(jsRuntime, nullptr);
    HILOG_INFO("ForceFullGC_0100 end");
}
} // namespace AbilityRuntime
} // namespace OHOS
