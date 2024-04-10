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
#include "hilog_tag_wrapper.h"
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
    TAG_LOGI(AAFwkTag::TEST, "Test BuildJsStackInfoList start");
    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    std::vector<JsFrames> frames;
    bool ret = jsRuntime->BuildJsStackInfoList(gettid(), frames);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "Test BuildJsStackInfoList end");
}

/**
 * @tc.name: JsRuntimeNotifyApplicationStateTest_0100
 * @tc.desc: JsRuntime test for NotifyApplicationState when nativeEngine is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeNotifyApplicationStateTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyApplicationState start");

    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<MockJsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    bool isBackground = false;
    jsRuntime->NotifyApplicationState(isBackground);

    TAG_LOGI(AAFwkTag::TEST, "NotifyApplicationState end");
}

/**
 * @tc.name: JsRuntimeNotifyApplicationStateTest_0200
 * @tc.desc: JsRuntime test for NotifyApplicationState when nativeEngine is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeNotifyApplicationStateTest_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyApplicationState start");

    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    bool isBackground = true;
    jsRuntime->NotifyApplicationState(isBackground);

    TAG_LOGI(AAFwkTag::TEST, "NotifyApplicationState end");
}

/**
 * @tc.name: JsRuntimeDumpHeapSnapshotTest_0100
 * @tc.desc: JsRuntime test for DumpHeapSnapshot.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeDumpHeapSnapshotTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "DumpHeapSnapshot start");

    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    bool isPrivate = true;
    jsRuntime->DumpHeapSnapshot(isPrivate);

    TAG_LOGI(AAFwkTag::TEST, "DumpHeapSnapshot end");
}

/**
 * @tc.name: JsRuntimePreloadSystemModuleTest_0100
 * @tc.desc: JsRuntime test for PreloadSystemModule.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimePreloadSystemModuleTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "PreloadSystemModule start");

    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string moduleName = "PreloadSystemModuleTest";
    jsRuntime->PreloadSystemModule(moduleName);

    TAG_LOGI(AAFwkTag::TEST, "PreloadSystemModule end");
}

/**
 * @tc.name: JsRuntimeRunSandboxScriptTest_0100
 * @tc.desc: JsRuntime test for RunSandboxScript.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeRunSandboxScriptTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "RunSandboxScript start");

    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string path = "";
    std::string hapPath = "";
    bool ret = (static_cast<AbilityRuntime::JsRuntime&>(*jsRuntime)).RunSandboxScript(path, hapPath);
    EXPECT_FALSE(ret);

    TAG_LOGI(AAFwkTag::TEST, "RunSandboxScript end");
}

/**
 * @tc.name: JsRuntimeLoadSystemModuleByEngineTest_0100
 * @tc.desc: JsRuntime test for LoadSystemModuleByEngine.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadSystemModuleByEngineTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadSystemModuleByEngine start");

    auto runtime = AbilityRuntime::JsRuntime::Create(options_);
    auto env = (static_cast<AbilityRuntime::MockJsRuntime&>(*runtime)).GetNapiEnv();

    std::string moduleName = "";
    std::unique_ptr<NativeReference> ref = MockJsRuntime::LoadSystemModuleByEngine(env, moduleName, nullptr, 0);
    EXPECT_EQ(ref, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "LoadSystemModuleByEngine end");
}

/**
 * @tc.name: JsRuntimeFinishPreloadTest_0100
 * @tc.desc: JsRuntime test for FinishPreload.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeFinishPreloadTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "FinishPreload start");

    auto jsRuntime = std::make_unique<JsRuntime>();

    jsRuntime->FinishPreload();
    EXPECT_TRUE(jsRuntime != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "FinishPreload end");
}

/**
 * @tc.name: JsRuntimePostPreloadTest_0100
 * @tc.desc: JsRuntime test for FinishPreload.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimePostPreloadTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "PostPreload start");

    auto jsRuntime = std::make_unique<JsRuntime>();

    jsRuntime->PostPreload(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "PostPreload end");
}

/**
 * @tc.name: JsRuntimeLoadAotFileTest_0100
 * @tc.desc: JsRuntime test for LoadAotFile.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadAotFileTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadAotFile start");

    auto jsRuntime = std::make_unique<JsRuntime>();

    jsRuntime->LoadAotFile(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "LoadAotFile end");
}

/**
 * @tc.name: JsRuntimeLoadModuleTest_0100
 * @tc.desc: JsRuntime test for LoadModule.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadModuleTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadModule start");

    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string moduleName = TEST_MODULE_NAME;
    std::string modulePath = TEST_MODULE_PATH;
    std::string hapPath = TEST_HAP_PATH;
    bool esmodule = true;
    std::unique_ptr<NativeReference> ref = (static_cast<AbilityRuntime::JsRuntime&>(*jsRuntime)).LoadModule(moduleName,
        modulePath, hapPath, esmodule);
    EXPECT_EQ(ref, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "LoadModule end");
}

/**
 * @tc.name: JsRuntimeLoadSystemModuleTest_0100
 * @tc.desc: JsRuntime test for LoadSystemModule (invoke the overwrite interface).
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadSystemModuleTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadSystemModule start");

    MockJsRuntime mockJsRuntime;
    std::unique_ptr<NativeReference> ref = mockJsRuntime.LoadSystemModule("", nullptr, 0);
    EXPECT_EQ(ref, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "LoadSystemModule end");
}

/**
 * @tc.name: RuntimeSavePreloadedTest_0100
 * @tc.desc: Runtime test for SavePreloaded.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RuntimeSavePreloadedTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "SavePreloaded start");

    auto runtime = AbilityRuntime::Runtime::Create(options_);
    runtime->SavePreloaded(nullptr);
    EXPECT_TRUE(runtime != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "SavePreloaded end");
}

/**
 * @tc.name: RuntimeSetModuleLoadCheckerTest_0100
 * @tc.desc: Runtime test for SetModuleLoadChecker.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RuntimeSetModuleLoadCheckerTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "SetModuleLoadChecker start");

    auto runtime = AbilityRuntime::Runtime::Create(options_);
    runtime->SetModuleLoadChecker(nullptr);
    EXPECT_TRUE(runtime != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "SetModuleLoadChecker end");
}

/**
 * @tc.name: JsRuntimeSuspendVMTest_0100
 * @tc.desc: JsRuntime test for SuspendVM.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeSuspendVMTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "SuspendVM start");

    auto runtime = AbilityRuntime::JsRuntime::Create(options_);
    auto result = runtime->SuspendVM(gettid());
    EXPECT_EQ(result, false);

    TAG_LOGI(AAFwkTag::TEST, "SuspendVM end");
}

/**
 * @tc.name: JsRuntimeResumeVMTest_0100
 * @tc.desc: JsRuntime test for ResumeVM.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeResumeVMTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "ResumeVM start");

    auto runtime = AbilityRuntime::JsRuntime::Create(options_);
    runtime->ResumeVM(gettid());
    EXPECT_TRUE(runtime != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "ResumeVM end");
}

/**
 * @tc.name: JsRuntimeSetDeviceDisconnectCallbackTest_0100
 * @tc.desc: JsRuntime test for SetDeviceDisconnectCallback.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeSetDeviceDisconnectCallbackTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "SetDeviceDisconnectCallback start");

    auto runtime = AbilityRuntime::JsRuntime::Create(options_);
    std::function<bool()> task = [&]() {
        return true;
    };
    runtime->SetDeviceDisconnectCallback(task);
    EXPECT_TRUE(runtime != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "SetDeviceDisconnectCallback end");
}

/**
 * @tc.name: JsRuntimeDetachCallbackFuncTest_0100
 * @tc.desc: JsRuntime test for PostTask.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeDetachCallbackFuncTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "DetachCallbackFunc start");

    auto runtime = AbilityRuntime::JsRuntime::Create(options_);
    auto env = (static_cast<AbilityRuntime::MockJsRuntime&>(*runtime)).GetNapiEnv();
    int32_t value = 1;
    int32_t number = 1;
    auto result = AbilityRuntime::DetachCallbackFunc(env, &value, &number);
    EXPECT_EQ(result, &value);

    TAG_LOGI(AAFwkTag::TEST, "DetachCallbackFunc end");
}

/**
 * @tc.name: JsRuntimeLoadSystemModulesTest_0100
 * @tc.desc: JsRuntime test for LoadSystemModule.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadSystemModulesTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadSystemModule start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string moduleName = "PreloadSystemModuleTest";
    napi_value object = nullptr;
    std::unique_ptr<NativeReference> ref = jsRuntime->LoadSystemModule(moduleName, &object, 0);
    EXPECT_EQ(ref, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "LoadSystemModule end");
}

/**
 * @tc.name: JsRuntimeStartDebugModeTest_0100
 * @tc.desc: JsRuntime test for StartDebugMode.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeStartDebugModeTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "StartDebugMode start");

    auto jsRuntime = std::make_unique<JsRuntime>();

    bool needBreakPoint = true;
    bool debugApp = true;
    const std::string processName = "test";
    jsRuntime->StartDebugMode(needBreakPoint, processName, debugApp, false);
    EXPECT_TRUE(jsRuntime != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "StartDebugMode end");
}

/**
 * @tc.name: JsRuntimeStopDebugModeTest_0100
 * @tc.desc: JsRuntime test for StopDebugMode.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeStopDebugModeTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "StopDebugMode start");

    auto jsRuntime = std::make_unique<JsRuntime>();

    jsRuntime->StopDebugMode();
    EXPECT_TRUE(jsRuntime != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "StopDebugMode end");
}

/**
 * @tc.name: JsRuntimeInitConsoleModuleTest_0100
 * @tc.desc: JsRuntime test for InitConsoleModule.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeInitConsoleModuleTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "InitConsoleModule start");

    auto jsRuntime = std::make_unique<JsRuntime>();

    jsRuntime->InitConsoleModule();
    EXPECT_TRUE(jsRuntime != nullptr);

    TAG_LOGI(AAFwkTag::TEST, "InitConsoleModule end");
}

/**
 * @tc.name: JsRuntimeLoadRepairPatchTest_0100
 * @tc.desc: JsRuntime test for LoadRepairPatch.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeLoadRepairPatchTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "LoadRepairPatch start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string hqfFile = "<hqfFile>";
    std::string hapPath = "<hapPath>";
    bool lrp = jsRuntime->LoadRepairPatch(hqfFile, hapPath);
    EXPECT_EQ(lrp, false);

    TAG_LOGI(AAFwkTag::TEST, "LoadRepairPatch end");
}

/**
 * @tc.name: JsRuntimeUnLoadRepairPatchTest_0100
 * @tc.desc: JsRuntime test for UnLoadRepairPatch.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeUnLoadRepairPatchTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "UnLoadRepairPatch start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string hqfFile = "<hqfFile>";
    bool lrp = jsRuntime->UnLoadRepairPatch(hqfFile);
    EXPECT_EQ(lrp, false);

    TAG_LOGI(AAFwkTag::TEST, "UnLoadRepairPatch end");
}

/**
 * @tc.name: JsRuntimeNotifyHotReloadPageTest_0100
 * @tc.desc: JsRuntime test for NotifyHotReloadPage.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeNotifyHotReloadPageTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyHotReloadPage start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    bool lrp = jsRuntime->NotifyHotReloadPage();
    EXPECT_EQ(lrp, true);

    TAG_LOGI(AAFwkTag::TEST, "NotifyHotReloadPage end");
}

/**
 * @tc.name: JsRuntimeUpdateModuleNameAndAssetPathTest_0100
 * @tc.desc: JsRuntime test for UpdateModuleNameAndAssetPath.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeUpdateModuleNameAndAssetPathTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateModuleNameAndAssetPath start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    std::string moduleName = "moduleName";
    jsRuntime->UpdateModuleNameAndAssetPath(moduleName);

    TAG_LOGI(AAFwkTag::TEST, "UpdateModuleNameAndAssetPath end");
}

/**
 * @tc.name: JsRuntimeUpdateModuleNameAndAssetPathTest_0200
 * @tc.desc: JsRuntime test for UpdateModuleNameAndAssetPath.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeUpdateModuleNameAndAssetPathTest_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeUpdateModuleNameAndAssetPathTest_0200 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime->isBundle_ = false;
    std::string moduleName = "moduleName";
    jsRuntime->UpdateModuleNameAndAssetPath(moduleName);

    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeUpdateModuleNameAndAssetPathTest_0200 end");
}

/**
 * @tc.name: JsRuntimeUpdateModuleNameAndAssetPathTest_0300
 * @tc.desc: JsRuntime test for UpdateModuleNameAndAssetPath.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeUpdateModuleNameAndAssetPathTest_0300, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeUpdateModuleNameAndAssetPathTest_0300 start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime->isBundle_ = false;
    std::string moduleName = "";
    jsRuntime->UpdateModuleNameAndAssetPath(moduleName);

    TAG_LOGI(AAFwkTag::TEST, "JsRuntimeUpdateModuleNameAndAssetPathTest_0300 end");
}

/**
 * @tc.name: JsRuntimeInitialize_0100
 * @tc.desc: Initialize js runtime in multi thread.
 * @tc.type: FUNC
 * @tc.require: issueI6KODF
 */
HWTEST_F(JsRuntimeTest, JsRuntimeInitialize_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "Running in multi-thread, using default thread number.");

    AbilityRuntime::Runtime::Options options;
    options.loadAce = false;
    options.preload = true;
    options.isStageModel = false;

    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);
    EXPECT_NE(jsRuntime->GetEcmaVm(), nullptr);
    EXPECT_NE(jsRuntime->GetNativeEnginePointer(), nullptr);
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
    TAG_LOGI(AAFwkTag::TEST, "RegisterQuickFixQueryFunc start");

    auto jsRuntime = std::make_unique<JsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);
    std::string moudel = "<moudelName>";
    std::string hqfFile = "<hqfFile>";
    std::map<std::string, std::string> moduleAndPath;
    moduleAndPath.insert(std::make_pair(moudel, hqfFile));
    jsRuntime->RegisterQuickFixQueryFunc(moduleAndPath);

    TAG_LOGI(AAFwkTag::TEST, "RegisterQuickFixQueryFunc end");
}

/**
 * @tc.name: RegisterUncaughtExceptionHandler_0100
 * @tc.desc: JsRuntime test for RegisterUncaughtExceptionHandler.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RegisterUncaughtExceptionHandler_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterUncaughtExceptionHandler start");

    AbilityRuntime::Runtime::Options options;
    options.preload = false;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);
    JsEnv::UncaughtExceptionInfo uncaughtExceptionInfo;
    jsRuntime->RegisterUncaughtExceptionHandler(uncaughtExceptionInfo);
    TAG_LOGI(AAFwkTag::TEST, "RegisterUncaughtExceptionHandler end");
}

/**
 * @tc.name: RegisterUncaughtExceptionHandler_0200
 * @tc.desc: JsRuntime test for RegisterUncaughtExceptionHandler.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RegisterUncaughtExceptionHandler_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterUncaughtExceptionHandler start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);
    JsEnv::UncaughtExceptionInfo uncaughtExceptionInfo;
    jsRuntime->RegisterUncaughtExceptionHandler(uncaughtExceptionInfo);
    TAG_LOGI(AAFwkTag::TEST, "RegisterUncaughtExceptionHandler end");
}

/**
 * @tc.name: ReadSourceMapData_0100
 * @tc.desc: JsRuntime test for ReadSourceMapData.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, ReadSourceMapData_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "ReadSourceMapData start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);

    std::string hapPath = "";
    std::string sourceMapPath = "";
    std::string content = "";
    auto result = jsRuntime->ReadSourceMapData(hapPath, sourceMapPath, content);
    ASSERT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "ReadSourceMapData end");
}

/**
 * @tc.name: StopDebugger_0100
 * @tc.desc: JsRuntime test for StopDebugger.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, StopDebugger_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "StopDebugger start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);

    jsRuntime->StopDebugger();
    TAG_LOGI(AAFwkTag::TEST, "StopDebugger end");
}

/**
 * @tc.name: GetFileBuffer_0200
 * @tc.desc: JsRuntime test for GetFileBuffer.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, GetFileBuffer_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "GetFileBuffer start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);

    std::string filePath = "";
    std::string fileFullName = "";
    std::vector<uint8_t> buffer;
    jsRuntime->GetFileBuffer(filePath, fileFullName, buffer);
    TAG_LOGI(AAFwkTag::TEST, "GetFileBuffer end");
}

/**
 * @tc.name: JsRuntimeRunScriptTest_0100
 * @tc.desc: JsRuntime test for RunScript.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeRunScriptTest_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "RunScript start");

    AbilityRuntime::Runtime::Options options;
    options.preload = false;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    std::string srcPath = TEST_MODULE_PATH;
    std::string hapPath = TEST_HAP_PATH;
    jsRuntime->RunScript(srcPath, hapPath);
    ASSERT_NE(jsRuntime, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "RunScript end");
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

    std::string path = "/system/etc/strip.native.min.abc";
    jsRuntime->LoadScript(path);
    ASSERT_NE(jsRuntime, nullptr);
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
        TAG_LOGI(AAFwkTag::TEST, "%{public}s called.", taskName.c_str());
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
        TAG_LOGI(AAFwkTag::TEST, "%{public}s called.", taskName.c_str());
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
    jsRuntime->StartProfiler(perfCmd, needBreakPoint, processName, debugApp, false);
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
    TAG_LOGI(AAFwkTag::TEST, "PostTask_0100 start");
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options_);
    ASSERT_NE(jsRuntime, nullptr);

    std::string taskName = "postTask001";
    bool taskExecuted = false;
    auto task = [taskName, &taskExecuted]() {
        TAG_LOGI(AAFwkTag::TEST, "%{public}s called.", taskName.c_str());
        taskExecuted = true;
    };
    int64_t delayTime = 10;
    jsRuntime->PostTask(task, taskName, delayTime);
    EXPECT_NE(taskExecuted, true);
    TAG_LOGI(AAFwkTag::TEST, "PostTask_0100 end");
}

/**
 * @tc.name: RemoveTask_0100
 * @tc.desc: Js runtime remove task.
 * @tc.type: FUNC
 * @tc.require: issueI7C87T
 */
HWTEST_F(JsRuntimeTest, RemoveTask_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveTask_0100 start");
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options_);
    ASSERT_NE(jsRuntime, nullptr);

    std::string taskName = "removeTask001";
    bool taskExecuted = false;
    auto task = [taskName, &taskExecuted]() {
        TAG_LOGI(AAFwkTag::TEST, "%{public}s called.", taskName.c_str());
        taskExecuted = true;
    };
    int64_t delayTime = 10;
    jsRuntime->PostTask(task, taskName, delayTime);
    jsRuntime->RemoveTask(taskName);
    EXPECT_NE(taskExecuted, true);
    TAG_LOGI(AAFwkTag::TEST, "RemoveTask_0100 end");
}

/**
 * @tc.name: StartDebugger_0100
 * @tc.desc: JsRuntime test for StartDebugger.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, StartDebugger_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "StartDebugger_0100 start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);

    bool needBreakPoint = false;
    uint32_t instanceId = 1;

    jsRuntime->StartDebugger(needBreakPoint, instanceId);
    // debug mode is global option, maybe has started by other testcase, not check here.
    TAG_LOGI(AAFwkTag::TEST, "StartDebugger_0100 end");
}

/**
 * @tc.name: DoCleanWorkAfterStageCleaned_0100
 * @tc.desc: JsRuntime test for DoCleanWorkAfterStageCleaned.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, DoCleanWorkAfterStageCleaned_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "DoCleanWorkAfterStageCleaned_0100 start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);

    jsRuntime->DoCleanWorkAfterStageCleaned();
    TAG_LOGI(AAFwkTag::TEST, "DoCleanWorkAfterStageCleaned_0100 end");
}

/**
 * @tc.name: ReloadFormComponent_0100
 * @tc.desc: JsRuntime test for ReloadFormComponent.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, ReloadFormComponent_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "ReloadFormComponent_0100 start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    ASSERT_NE(jsRuntime, nullptr);

    jsRuntime->ReloadFormComponent();
    TAG_LOGI(AAFwkTag::TEST, "ReloadFormComponent_0100 end");
}

/**
 * @tc.name: SetRequestAotCallback_0100
 * @tc.desc: JsRuntime test for SetRequestAotCallback.
 * @tc.type: FUNC
 * @tc.require: issueI82L1A
 */
HWTEST_F(JsRuntimeTest, SetRequestAotCallback_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);

    jsRuntime->SetRequestAotCallback();
    auto ret = panda::MockJSNApi::GetInstance()->RequestAot("bundleName", "moduleName", 0);
    EXPECT_NE(ret, -1);
    TAG_LOGI(AAFwkTag::TEST, "finish");
}

/**
 * @tc.name: DestroyHeapProfiler_0100
 * @tc.desc: JsRuntime test for DestroyHeapProfiler.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, DestroyHeapProfiler_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "DestroyHeapProfiler_0100 start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    jsRuntime->DestroyHeapProfiler();
    ASSERT_NE(jsRuntime, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "DestroyHeapProfiler_0100 end");
}

/**
 * @tc.name: ForceFullGC_0100
 * @tc.desc: JsRuntime test for ForceFullGC.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, ForceFullGC_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "ForceFullGC_0100 start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    jsRuntime->ForceFullGC();
    ASSERT_NE(jsRuntime, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "ForceFullGC_0100 end");
}

/**
 * @tc.name: AllowCrossThreadExecution_0100
 * @tc.desc: JsRuntime test for AllowCrossThreadExecution.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, AllowCrossThreadExecution_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AllowCrossThreadExecution_0100 start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    jsRuntime->AllowCrossThreadExecution();
    ASSERT_NE(jsRuntime, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AllowCrossThreadExecution_0100 end");
}

/**
 * @tc.name: GetHeapPrepare_0100
 * @tc.desc: JsRuntime test for GetHeapPrepare.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, GetHeapPrepare_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "GetHeapPrepare_0100 start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    jsRuntime->GetHeapPrepare();
    ASSERT_NE(jsRuntime, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetHeapPrepare_0100 end");
}

/**
 * @tc.name: InitLoop_0100
 * @tc.desc: JsRuntime test for InitLoop.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, InitLoop_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "InitLoop_0100 start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    auto result = jsRuntime->InitLoop();
    ASSERT_EQ(result, true);
    TAG_LOGI(AAFwkTag::TEST, "InitLoop_0100 end");
}

/**
 * @tc.name: InitSourceMap_0100
 * @tc.desc: JsRuntime test for InitSourceMap.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, InitSourceMap_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "InitSourceMap_0100 start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    jsRuntime->InitSourceMap(nullptr);
    ASSERT_NE(jsRuntime, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "InitSourceMap_0100 end");
}

/**
 * @tc.name: Deinitialize_0100
 * @tc.desc: JsRuntime test for Deinitialize.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, Deinitialize_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "Deinitialize_0100 start");

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);

    jsRuntime->Deinitialize();
    ASSERT_NE(jsRuntime, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "Deinitialize_0100 end");
}

/**
 * @tc.name: GetPkgContextInfoListMap_0100
 * @tc.desc: JsRuntime test for GetPkgContextInfoListMap.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, GetPkgContextInfoListMap_0100, TestSize.Level0)
{
    HILOG_INFO("GetPkgContextInfoListMap_0100 start");

    std::map<std::string, std::string> modulePkgContentMap;
    std::string pkgContentJsonString = R"({"library":{"packageName":"library","bundleName":"com.xxx.xxxx","moduleName":
            "library","version":"1.0.0","entryPath":"","isSO":false}})";
    modulePkgContentMap["entry"] = pkgContentJsonString;

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    std::map<std::string, std::vector<std::vector<std::string>>> ret;
    std::map<std::string, std::string> pkgAliasMap;
    jsRuntime->GetPkgContextInfoListMap(modulePkgContentMap, ret, pkgAliasMap);
    std::string expectString = "library:packageName:library:bundleName:";
    expectString += "com.xxx.xxxx:moduleName:library:version:1.0.0:entryPath::isSO:false:";
    auto it = ret.find("entry");
    ASSERT_NE(it, ret.end());
    std::string pkgRetString;
    for (const auto& vec : it->second) {
        for (const auto& str : vec) {
            pkgRetString += str + ":";
        }
    }
    ASSERT_EQ(pkgRetString, expectString);
    HILOG_INFO("GetPkgContextInfoListMap_0100 end");
}

/**
 * @tc.name: GetPkgContextInfoListMap_0200
 * @tc.desc: JsRuntime test for GetPkgContextInfoListMap.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, GetPkgContextInfoListMap_0200, TestSize.Level0)
{
    HILOG_INFO("GetPkgContextInfoListMap_0200 start");

    std::map<std::string, std::string> modulePkgContentMap;
    std::string pkgContentJsonString = R"({"library":{"packageName":"library","bundleName":"com.xxx.xxxx","moduleName":
            "library","version":"1.0.0","entryPath":"","isSO":false}})";
    modulePkgContentMap["entry"] = pkgContentJsonString;

    std::string libraryString = R"({"library":{"packageName":"library","bundleName":"com.xxx.xxxx","moduleName":
            "library","version":"1.0.0","entryPath":"","isSO":false}})";
    modulePkgContentMap["library"] = libraryString;

    AbilityRuntime::Runtime::Options options;
    options.preload = true;
    auto jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    std::map<std::string, std::vector<std::vector<std::string>>> ret;
    std::map<std::string, std::string> pkgAliasMap;
    jsRuntime->GetPkgContextInfoListMap(modulePkgContentMap, ret, pkgAliasMap);
    std::string expectString = "library:packageName:library:bundleName:";
    expectString += "com.xxx.xxxx:moduleName:library:version:1.0.0:entryPath::isSO:false:";
    auto it = ret.find("entry");
    ASSERT_NE(it, ret.end());
    auto libraryIt = ret.find("library");
    ASSERT_NE(libraryIt, ret.end());
    std::string pkgRetString;
    for (const auto& vec : it->second) {
        for (const auto& str : vec) {
            pkgRetString += str + ":";
        }
    }
    ASSERT_EQ(pkgRetString, expectString);
    HILOG_INFO("GetPkgContextInfoListMap_0200 end");
}

/**
 * @tc.name: CreateJsEnv_0100
 * @tc.desc: JsRuntime test for CreateJsEnv.
 * @tc.type: FUNC
 * @tc.require: issueI9CHSB
 */
HWTEST_F(JsRuntimeTest, CreateJsEnv_0100, TestSize.Level1)
{
    auto jsRuntime = std::make_unique<JsRuntime>();
    auto ret = jsRuntime->CreateJsEnv(options_);
    EXPECT_EQ(ret, true);
}
} // namespace AbilityRuntime
} // namespace OHOS
