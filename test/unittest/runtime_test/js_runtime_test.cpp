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

#define private public
#define protected public
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_worker.h"
#undef private
#undef protected

#include "event_runner.h"
#include "mock_js_runtime.h"
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;

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
    options_.hapPath = TEST_HAP_PATH;
    options_.loadAce = true;
    options_.isBundle = true;
    options_.preload = false;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);
    options_.eventRunner = eventRunner;
}

void JsRuntimeTest::TearDown()
{}

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
    options_.appLibPaths[appLibPathKey].emplace_back(libPath);
    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    EXPECT_TRUE(jsRuntime != nullptr);
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
    auto& jsEngine = (static_cast<AbilityRuntime::JsRuntime&>(*runtime)).GetNativeEngine();

    NativeReference* callbackRef = jsEngine.CreateReference(jsEngine.CreateUndefined(), 1);
    std::unique_ptr<AsyncTask> task = std::make_unique<AsyncTask>(callbackRef, nullptr, nullptr);
    task->ResolveWithNoError(jsEngine, jsEngine.CreateUndefined());
    EXPECT_TRUE(task->callbackRef_ == nullptr);

    NativeDeferred* nativeDeferred = nullptr;
    jsEngine.CreatePromise(&nativeDeferred);
    task = std::make_unique<AsyncTask>(nativeDeferred, nullptr, nullptr);
    task->ResolveWithNoError(jsEngine, jsEngine.CreateUndefined());
    EXPECT_TRUE(task->deferred_ == nullptr);

    task->deferred_ = nullptr;
    task->callbackRef_ = nullptr;
    task->ResolveWithNoError(jsEngine, jsEngine.CreateUndefined());
    EXPECT_TRUE(task->deferred_ == nullptr);
    EXPECT_TRUE(task->callbackRef_ == nullptr);
}

/**
 * @tc.name: JsWorkerTest_0100
 * @tc.desc: JsWorker Test
 * @tc.type: FUNC
 * @tc.require: issueI581RO
 */
HWTEST_F(JsRuntimeTest, JsWorkerTest_0100, TestSize.Level0)
{
    auto runtime = AbilityRuntime::Runtime::Create(options_);
    auto& jsEngine = (static_cast<AbilityRuntime::JsRuntime&>(*runtime)).GetNativeEngine();

    std::vector<uint8_t> content;
    std::string str = "test";
    InitWorkerModule(jsEngine, "", true, "", 0);
    jsEngine.CallGetAssetFunc("", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test.test", content, str);
    EXPECT_TRUE(content.empty());

    InitWorkerModule(jsEngine, "", false, "", 0);
    jsEngine.CallGetAssetFunc("", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test.test", content, str);
    EXPECT_TRUE(content.empty());

    InitWorkerModule(jsEngine, TEST_CODE_PATH, true, "", 0);
    jsEngine.CallGetAssetFunc("", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test.test", content, str);
    EXPECT_TRUE(content.empty());

    InitWorkerModule(jsEngine, TEST_CODE_PATH, false, "", 0);
    jsEngine.CallGetAssetFunc("", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test.test", content, str);
    EXPECT_TRUE(content.empty());
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
    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<MockJsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime->nativeEngine_ = std::make_unique<MockJsNativeEngine>();

    std::vector<JsFrames> frames;
    bool ret = jsRuntime->BuildJsStackInfoList(gettid(), frames);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: JsRuntimeNotifyApplicationStateTest_0100
 * @tc.desc: JsRuntime test for NotifyApplicationState when nativeEngine_ is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeNotifyApplicationStateTest_0100, TestSize.Level0)
{
    HILOG_INFO("NotifyApplicationState start");

    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<MockJsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime->nativeEngine_ = nullptr;

    bool isBackground = false;
    jsRuntime->NotifyApplicationState(isBackground);

    HILOG_INFO("NotifyApplicationState end");
}

/**
 * @tc.name: JsRuntimeNotifyApplicationStateTest_0200
 * @tc.desc: JsRuntime test for NotifyApplicationState when nativeEngine_ is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeNotifyApplicationStateTest_0200, TestSize.Level0)
{
    HILOG_INFO("NotifyApplicationState start");

    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<MockJsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime->nativeEngine_ = std::make_unique<MockJsNativeEngine>();

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

    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<MockJsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime->nativeEngine_ = std::make_unique<MockJsNativeEngine>();

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

    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<MockJsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime->nativeEngine_ = std::make_unique<MockJsNativeEngine>();

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

    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<MockJsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime->nativeEngine_ = std::make_unique<MockJsNativeEngine>();

    std::string path = "";
    std::string hapPath = "";
    bool ret = jsRuntime->RunSandboxScript(path, hapPath);
    EXPECT_TRUE(ret);

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
    auto& jsEngine = (static_cast<AbilityRuntime::MockJsRuntime&>(*runtime)).GetNativeEngine();

    std::string moduleName = "";
    std::unique_ptr<NativeReference> ref = MockJsRuntime::LoadSystemModuleByEngine(&jsEngine, moduleName, nullptr, 0);
    EXPECT_NE(ref, nullptr);

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

    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<MockJsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime->nativeEngine_ = std::make_unique<MockJsNativeEngine>();

    std::string moduleName = TEST_MODULE_NAME;
    std::string modulePath = TEST_MODULE_PATH;
    std::string hapPath = TEST_HAP_PATH;
    bool esmodule = true;
    std::unique_ptr<NativeReference> ref = jsRuntime->LoadModule(moduleName, modulePath, hapPath, esmodule);
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
 * @tc.name: JsRuntimeGetSourceMapTest_0100
 * @tc.desc: JsRuntime test for GetSourceMap.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimeGetSourceMapTest_0100, TestSize.Level0)
{
    HILOG_INFO("GetSourceMap start");

    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<MockJsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    options_.bundleCodeDir = TEST_CODE_PATH;
    options_.isStageModel = true;
    jsRuntime->bindSourceMaps_ = std::make_unique<ModSourceMap>(options_.bundleCodeDir, options_.isStageModel);

    auto& sourceMap = jsRuntime->GetSourceMap();
    EXPECT_NE(&sourceMap, nullptr);

    HILOG_INFO("GetSourceMap end");
}

/**
 * @tc.name: JsRuntimePostTaskTest_0100
 * @tc.desc: JsRuntime test for PostTask.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, JsRuntimePostTaskTest_0100, TestSize.Level0)
{
    HILOG_INFO("PostTask start");

    std::unique_ptr<JsRuntime> jsRuntime = std::make_unique<MockJsRuntime>();
    EXPECT_TRUE(jsRuntime != nullptr);

    jsRuntime->eventHandler_ = nullptr;

    auto task = []() { GTEST_LOG_(INFO) << "JsRuntimePostTaskTest_0100 task called"; };
    std::string name = "";
    int64_t delayTime = 0;
    jsRuntime->PostTask(task, name, delayTime);

    HILOG_INFO("PostTask end");
}

/**
 * @tc.name: RuntimeSavePreloadedTest_0100
 * @tc.desc: Runtime test for SavePreloaded.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeTest, RuntimeSavePreloadedTest_0100, TestSize.Level0)
{
    HILOG_INFO("SavePreloaded start");

    Runtime::SavePreloaded(nullptr);

    HILOG_INFO("SavePreloaded end");
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
    auto& jsEngine = (static_cast<AbilityRuntime::MockJsRuntime&>(*runtime)).GetNativeEngine();
    int32_t value = 1;
    int32_t number = 1;
    auto result = AbilityRuntime::DetachCallbackFunc(&jsEngine, &value, &number);
    EXPECT_EQ(result, &value);

    HILOG_INFO("DetachCallbackFunc end");
}
}  // namespace AbilityRuntime
}  // namespace OHOS
