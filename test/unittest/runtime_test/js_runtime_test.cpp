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
    InitWorkerModule(jsEngine, "", true);
    jsEngine.CallGetAssetFunc("", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test.test", content, str);
    EXPECT_TRUE(content.empty());

    InitWorkerModule(jsEngine, "", false);
    jsEngine.CallGetAssetFunc("", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test.test", content, str);
    EXPECT_TRUE(content.empty());

    InitWorkerModule(jsEngine, TEST_CODE_PATH, true);
    jsEngine.CallGetAssetFunc("", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test.test", content, str);
    EXPECT_TRUE(content.empty());

    InitWorkerModule(jsEngine, TEST_CODE_PATH, false);
    jsEngine.CallGetAssetFunc("", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test", content, str);
    EXPECT_TRUE(content.empty());

    jsEngine.CallGetAssetFunc("test.test", content, str);
    EXPECT_TRUE(content.empty());
}
}  // namespace AbilityRuntime
}  // namespace OHOS
