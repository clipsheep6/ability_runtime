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
#include <memory>
#include <string>
#include <regex>

#include "ability_delegator_registry.h"
#include "hilog_wrapper.h"
#include "cj_runtime.h"
#include "runner_runtime/cj_test_runner.h"
#include "runner_runtime/cj_test_runner_object.h"

#include "cj_mock_runtime.h"
#include "constants.h"
#include "app_loader.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "napi/native_common.h"
#include "ohos_application.h"

using namespace OHOS;
using namespace RunnerRuntime;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityBase::Constants;
using namespace testing;
using namespace testing::ext;

namespace {
const std::string KEY_TEST_BUNDLE_NAME = "-p";
const std::string VALUE_TEST_BUNDLE_NAME = "com.example.myapplicationjs";
const std::string KEY_TEST_RUNNER_CLASS = "-s unittest";
const std::string VALUE_TEST_RUNNER_CLASS = "CjUserTestRunnerCj";
const std::string KEY_TEST_CASE = "-s class";
const std::string VALUE_TEST_CASE =
"ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010_js";
const std::string KEY_TEST_WAIT_TIMEOUT = "-w";
const std::string VALUE_TEST_WAIT_TIMEOUT = "35";
const std::string REPORT_FINISH_MSG = "report finish message";
const std::string TEST_BUNDLE_NAME = "com.ohos.contactsdataability";
const std::string TEST_MODULE_NAME = ".ContactsDataAbility";
const std::string TEST_ABILITY_NAME = "ContactsDataAbility";
const std::string TEST_CODE_PATH = "/data/storage/el1/bundle";
const std::string TEST_HAP_PATH = "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
const std::string TEST_LIB_PATH = "/data/storage/el1/bundle/lib/";
const std::string TEST_MODULE_PATH = "/data/storage/el1/bundle/curCJModulePath";
}

class CjTestRunnerTest : public Test {
public:
    CjTestRunnerTest()
    {}
    ~CjTestRunnerTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
    Runtime::Options options_;

protected:
    std::unique_ptr<CJTestRunner> testRunner_;
    std::shared_ptr<AbilityDelegatorArgs> delegator_;
    std::unique_ptr<CJRuntime> runtime_;
    AppExecFwk::BundleInfo bundleInfo_;
};

void CjTestRunnerTest::SetUpTestCase()
{}

void CjTestRunnerTest::TearDownTestCase()
{}

void CjTestRunnerTest::SetUp()
{
    options_.bundleName = TEST_BUNDLE_NAME;
    options_.codePath = TEST_CODE_PATH;
    options_.loadAce = false;
    options_.isBundle = true;
    options_.preload = false;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);
    options_.eventRunner = eventRunner;
    options_.preload = true;
    options_.lang = CJRuntime::Language::CJ;
    std::unique_ptr<CJRuntime> runtime = std::make_unique<cjMockRuntime>();
    AppLibPathMap appLibPaths{};
    std::vector<std::string> paths = {"/data/test/"};
    appLibPaths.emplace("", paths);
    CJRuntime::SetAppLibPath(appLibPaths);
    runtime_ = runtime->Create(options_);
}

void CjTestRunnerTest::TearDown()
{
}

/**
 * @tc.name: CjTestRunnerTestCreate_Failed_RuntimeNull_001
 * @tc.desc: CjTestRunnerTest test for OnMemoryLevel.
 * @tc.type: FUNC
 */
HWTEST_F(CjTestRunnerTest, CjTestRunnerTestCreate_Failed_RuntimeNull_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "CjTestRunnerTestInitialize_Success_001 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }
    std::shared_ptr<AbilityDelegatorArgs> abilityArgs = std::make_shared<AbilityDelegatorArgs>(want);

    std::unique_ptr<Runtime> runtime = static_cast<std::unique_ptr<Runtime>>(std::move(runtime_));
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        runtime,
        abilityArgs,
        true);
}
