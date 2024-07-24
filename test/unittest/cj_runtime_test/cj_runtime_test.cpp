/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "cj_runtime.h"
#include "runtime.h"
#include "cj_mock_runtime.h"

#include "event_runner.h"
#include "hilog_wrapper.h"
#include "cj_runtime.h"

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
const std::string TEST_MODULE_PATH = "/data/storage/el1/bundle/curCJModulePath";
}  // namespace
class CjRuntimeTest : public testing::Test {
public:
    CjRuntimeTest()
    {}
    ~CjRuntimeTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
    Runtime::Options options_;
};

void CjRuntimeTest::SetUpTestCase(void)
{}

void CjRuntimeTest::TearDownTestCase(void)
{}

void CjRuntimeTest::SetUp(void)
{
    options_.bundleName = TEST_BUNDLE_NAME;
    options_.codePath = TEST_CODE_PATH;
    options_.loadAce = false;
    options_.isBundle = true;
    options_.preload = false;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);
    options_.eventRunner = eventRunner;
}

void CjRuntimeTest::TearDown(void)
{}

/**
 * @tc.name: CjRuntimeCreate_001
 * @tc.desc: Interface Create Test
 * @tc.type: FUNC
 */
HWTEST_F(CjRuntimeTest, CjRuntimeCreate_001, TestSize.Level1)
{
    options_.preload = true;
    options_.lang = CJRuntime::Language::CJ;
    std::unique_ptr<CJRuntime> runtime = std::make_unique<cjMockRuntime>();
    AppLibPathMap appLibPaths{};
    CJRuntime::SetAppLibPath(appLibPaths);
    auto cjRuntime = runtime->Create(options_);
    EXPECT_EQ(cjRuntime, nullptr);
    std::vector<std::string> paths = {"/data/test/"};
    appLibPaths.emplace("", paths);
    CJRuntime::SetAppLibPath(appLibPaths);
    cjRuntime = runtime->Create(options_);
}

/**
 * @tc.name: CjRuntimeCreate_002
 * @tc.desc: Interface Create Test for Fail Situation
 * @tc.type: FUNC
 */
HWTEST_F(CjRuntimeTest, CjRuntimeCreate_002, TestSize.Level1)
{
    options_.preload = true;
    options_.lang = CJRuntime::Language::JS;
    std::unique_ptr<CJRuntime> runtime = std::make_unique<cjMockRuntime>();
    auto cjRuntime = runtime->Create(options_);
    EXPECT_TRUE(cjRuntime == nullptr);
}

/**
 * @tc.name: CjRuntimeSetAppLibPath_001
 * @tc.desc: Interface SetAppLibPath Test
 * @tc.type: FUNC
 */
HWTEST_F(CjRuntimeTest, CjRuntimeSetAppLibPath_001, TestSize.Level0)
{
    std::string appLibPathKey = TEST_BUNDLE_NAME + TEST_MODULE_NAME;
    std::string libPath = TEST_LIB_PATH;

    AppLibPathMap appLibPaths{};
    CJRuntime::SetAppLibPath(appLibPaths);

    appLibPaths[appLibPathKey].emplace_back(libPath);
    EXPECT_NE(appLibPaths.size(), 0);
    CJRuntime::SetAppLibPath(appLibPaths);
}

/**
 * @tc.name: CjRuntimeGetLanguageTest_001
 * @tc.desc: CjRuntime Test for GetLanguage
 * @tc.type: FUNC
 */
HWTEST_F(CjRuntimeTest, CjRuntimeGetLanguageTest_001, TestSize.Level0)
{
    auto instance = std::make_unique<CJRuntime>();

    CJRuntime::Language language = instance->GetLanguage();
    EXPECT_TRUE(language == CJRuntime::Language::CJ);
    instance->UnLoadCJAppLibrary();
}

/**
 * @tc.name: CjRuntimeStartDebuggerMode_001
 * @tc.desc: CjRuntime test for StartDebuggerMode.
 * @tc.type: FUNC
 */
HWTEST_F(CjRuntimeTest, CjRuntimeStartDebuggerMode_001, TestSize.Level0)
{
    auto instance = std::make_unique<CJRuntime>();

    bool needBreakPoint = true;
    bool debugApp = true;
    const std::string processName = "test";

    CJRuntime::DebugOption debugOption;
    debugOption.isStartWithDebug = needBreakPoint;
    debugOption.isDebugApp = debugApp;
    debugOption.processName = processName;

    instance->StartDebugMode(debugOption);
    instance->StartDebugMode(debugOption);
}

}  // namespace Runtime
}  // namespace OHOS
