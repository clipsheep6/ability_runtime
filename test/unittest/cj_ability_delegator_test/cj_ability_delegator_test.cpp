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

#include <iostream>
#include <string>
#include "gtest/gtest.h"
#include "cj_ability_delegator.h"
#include "ability_delegator_registry.h"
#include "cj_application_context.h"
#include "application_context.h"
#include "runner_runtime/cj_test_runner.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::FFI;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
using namespace OHOS::RunnerRuntime;

namespace OHOS {
namespace AbilityDelegatorCJ  {
namespace {
const std::string KEY_TEST_BUNDLE_NAME = "-p";
const std::string VALUE_TEST_BUNDLE_NAME = "com.example.myapplication";
const std::string CHANGE_VALUE_TEST_BUNDLE_NAME = "com.example.myapplication1";
const std::string KEY_TEST_RUNNER_CLASS = "-s unittest";
const std::string VALUE_TEST_RUNNER_CLASS = "JSUserTestRunner";
const std::string CHANGE_VALUE_TEST_RUNNER_CLASS = "JSUserTestRunner1";
const std::string KEY_TEST_CASE = "-s class";
const std::string VALUE_TEST_CASE = "ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010";
const std::string CHANGE_VALUE_TEST_CASE =
    "ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction00101";
const std::string KEY_TEST_WAIT_TIMEOUT = "-w";
const std::string VALUE_TEST_WAIT_TIMEOUT = "50";
const std::string CHANGE_VALUE_TEST_WAIT_TIMEOUT = "80";
const std::string SET_VALUE_TEST_BUNDLE_NAME = "com.example.myapplicationset";
const std::string ABILITY_NAME = "com.example.myapplication.MainAbility";
const std::string FINISH_MSG = "finish message";
const int32_t FINISH_RESULT_CODE = 144;
const std::string PRINT_MSG = "print aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
const int ZERO = 0;
const int ONE = 1;
const int TWO = 2;
const int64_t TIMEOUT = 50;
const std::string CMD = "ls -l";
const std::string KEY_TEST_DEBUG {"-D"};
const std::string VALUE_TEST_DEBUG {"true"};
const std::string ABILITY_STAGE_MONITOR_MODULE_NAME {"entry"};
const std::string ABILITY_STAGE_MONITOR_SRC_ENTRANCE {"MainAbility"};
}  // namespace

class CjAbilityDelegatorTest : public testing::Test {
public:
    CjAbilityDelegatorTest()
    {}
    ~CjAbilityDelegatorTest()
    {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
protected:
    static std::shared_ptr<CJAbilityDelegator> cjDelegator;
    static std::shared_ptr<AbilityDelegator> commonDelegator_;
    static std::shared_ptr<AbilityDelegatorArgs> delegatorArgs_;
};

std::shared_ptr<CJAbilityDelegator> CjAbilityDelegatorTest::cjDelegator = nullptr;
std::shared_ptr<AbilityDelegator> CjAbilityDelegatorTest::commonDelegator_ = nullptr;
std::shared_ptr<AbilityDelegatorArgs> CjAbilityDelegatorTest::delegatorArgs_ = nullptr;

void CjAbilityDelegatorTest::SetUpTestCase()
{
    // Construct a common ability delegator firstly.
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);
    paras.emplace(KEY_TEST_DEBUG, VALUE_TEST_DEBUG);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    delegatorArgs_ = std::make_shared<AbilityDelegatorArgs>(want);
    AbilityRuntime::Runtime::Options options;
    BundleInfo bundleInfo;
    auto testRunner = CJTestRunner::Create(AbilityRuntime::Runtime::Create(options), delegatorArgs_, bundleInfo);
    commonDelegator_ = std::make_shared<AbilityDelegator>(std::make_shared<AbilityRuntime::ContextImpl>(),
        std::move(testRunner), nullptr);

    // 创建一个 CJAbilityDelegator 对象
    cjDelegator = std::make_shared<CJAbilityDelegator>(commonDelegator_);
}

void CjAbilityDelegatorTest::TearDownTestCase()
{}

void CjAbilityDelegatorTest::SetUp()
{}

void CjAbilityDelegatorTest::TearDown()
{}

/**
 * @tc.name: CjAbilityDelegatorTestStartAbility_001
 * @tc.desc: CjAbilityDelegatorTest test for FFICJWantAddEntity.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorTest, CjAbilityDelegatorTestStartAbility_001, TestSize.Level1)
{
    EXPECT_NE(commonDelegator_, nullptr);
    AbilityDelegatorRegistry::RegisterInstance(commonDelegator_, delegatorArgs_);

    AAFwk::Want want;
    want.SetElementName(VALUE_TEST_BUNDLE_NAME, ABILITY_NAME);
    auto result = cjDelegator->StartAbility(want);
}

/**
 * @tc.name: CjAbilityDelegatorTestExecuteShellCommand_001
 * @tc.desc: CjAbilityDelegatorTest test for ExecuteShellCommand.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorTest, CjAbilityDelegatorTestExecuteShellCommand_001, TestSize.Level1)
{
    const char* cmd = "ls";
    int64_t timeoutSec = 10;
    auto shellCmdResult = cjDelegator->ExecuteShellCommand(cmd, timeoutSec);
    EXPECT_EQ(shellCmdResult, nullptr);
}

/**
 * @tc.name: CjAbilityDelegatorTestGetAppContext_001
 * @tc.desc: CjAbilityDelegatorTest test for GetAppContext.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorTest, CjAbilityDelegatorTestGetAppContext_001, TestSize.Level1)
{
    auto appContext = cjDelegator->GetAppContext();
    EXPECT_EQ(appContext, nullptr);
}

/**
 * @tc.name: CjAbilityDelegatorTestGetExitCode_001
 * @tc.desc: CjAbilityDelegatorTest test for GetExitCode.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorTest, CjAbilityDelegatorTestGetExitCode_001, TestSize.Level1)
{
    auto shellResult = std::make_shared<AppExecFwk::ShellCmdResult>();
    CJShellCmdResult shellCmdResult(shellResult);
    int32_t exitCode = shellCmdResult.GetExitCode();
    EXPECT_GE(exitCode, -1);
}

/**
 * @tc.name: CjAbilityDelegatorTestGetStdResult_001
 * @tc.desc: CjAbilityDelegatorTest test for GetStdResult.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorTest, CjAbilityDelegatorTestGetStdResult_001, TestSize.Level1)
{
    auto shellResult = std::make_shared<AppExecFwk::ShellCmdResult>();
    CJShellCmdResult shellCmdResult(shellResult);
    std::string stdResult = shellCmdResult.GetStdResult();
    EXPECT_TRUE(stdResult.empty());
}

/**
 * @tc.name: CjAbilityDelegatorTestDump_001
 * @tc.desc: CjAbilityDelegatorTest test for Dump.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorTest, CjAbilityDelegatorTestDump_001, TestSize.Level1)
{
    auto shellResult = std::make_shared<AppExecFwk::ShellCmdResult>();
    CJShellCmdResult shellCmdResult(shellResult);
    shellCmdResult.Dump();
}

/**
 * @tc.name: CJAbilityDelegatorTestFFIAbilityDelegatorRegistryGetAbilityDelegator_001
 * @tc.desc: CjAbilityDelegatorTest test for FFIAbilityDelegatorRegistryGetAbilityDelegator.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorTest,
    CJAbilityDelegatorTestFFIAbilityDelegatorRegistryGetAbilityDelegator_001, TestSize.Level1)
{
    FFIAbilityDelegatorRegistryGetAbilityDelegator();
}

/**
 * @tc.name: CJAbilityDelegatorTestFFIAbilityDelegatorStartAbility_001
 * @tc.desc: CjAbilityDelegatorTest test for FFIAbilityDelegatorStartAbility.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorTest, CJAbilityDelegatorTestFFIAbilityDelegatorStartAbility_001, TestSize.Level1)
{
    Want want;
    WantHandle wantHandle = const_cast<AAFwk::Want *>(&want);
    auto delegator = OHOS::AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    auto cjDelegator = FFI::FFIData::Create<CJAbilityDelegator>(delegator);
    int64_t id = cjDelegator->GetID();
    int64_t ret = FFIAbilityDelegatorStartAbility(id, wantHandle);
    EXPECT_NE(ret, 1);
}

/**
 * @tc.name: CJAbilityDelegatorTestFFIAbilityDelegatorExecuteShellCommand_001
 * @tc.desc: CjAbilityDelegatorTest test for FFIAbilityDelegatorExecuteShellCommand.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorTest, CJAbilityDelegatorTestFFIAbilityDelegatorExecuteShellCommand_001, TestSize.Level1)
{
    const char* cmd = "test";
    int64_t timeoutSec = 1000;
    auto delegator = OHOS::AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    auto cjDelegator = FFI::FFIData::Create<CJAbilityDelegator>(delegator);
    int64_t id = cjDelegator->GetID();
    FFIAbilityDelegatorExecuteShellCommand(id, cmd, timeoutSec);
}

/**
 * @tc.name: CJAbilityDelegatorTestFFIGetExitCode_001
 * @tc.desc: CjAbilityDelegatorTest test for FFIGetExitCode.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorTest, CJAbilityDelegatorTestFFIGetExitCode_001, TestSize.Level1)
{
    int64_t timeoutSec = 1000;
    const char* cmd = "ls";

    auto shellResult = std::make_shared<AppExecFwk::ShellCmdResult>();
    auto cJShellCmdResult = FFI::FFIData::Create<CJShellCmdResult>(shellResult);
    int64_t id = cJShellCmdResult->GetID();
    FFIGetExitCode(id);
}

/**
 * @tc.name: CJAbilityDelegatorTestFFIGetStdResult_001
 * @tc.desc: CjAbilityDelegatorTest test for FFIGetStdResult.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorTest, CJAbilityDelegatorTestFFIGetStdResult_001, TestSize.Level1)
{
    int64_t timeoutSec = 1000;
    const char* cmd = "ls";
    auto shellResult = std::make_shared<AppExecFwk::ShellCmdResult>();
    auto cJShellCmdResult = FFI::FFIData::Create<CJShellCmdResult>(shellResult);
    int64_t id = cJShellCmdResult->GetID();
    FFIGetStdResult(id);
}

/**
 * @tc.name: CJAbilityDelegatorTestFFIDump_001
 * @tc.desc: CjAbilityDelegatorTest test for FFIDump.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorTest, CJAbilityDelegatorTestFFIDump_001, TestSize.Level1)
{
    int64_t timeoutSec = 1000;
    const char* cmd = "ls";
    auto shellResult = std::make_shared<AppExecFwk::ShellCmdResult>();
    auto cJShellCmdResult = FFI::FFIData::Create<CJShellCmdResult>(shellResult);
    int64_t id = cJShellCmdResult->GetID();
    FFIDump(id);
}

/**
 * @tc.name: CJAbilityDelegatorTestFFIAbilityDelegatorApplicationContext_001
 * @tc.desc: CjAbilityDelegatorTest test for FFIAbilityDelegatorApplicationContext.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorTest, CJAbilityDelegatorTestFFIAbilityDelegatorApplicationContext_001, TestSize.Level1)
{
    auto delegator = OHOS::AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    auto cjDelegator = FFI::FFIData::Create<CJAbilityDelegator>(delegator);
    int64_t id = cjDelegator->GetID();
    FFIAbilityDelegatorApplicationContext(id);
}

}  // namespace AbilityRuntime
}  // namespace OHOS