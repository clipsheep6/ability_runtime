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
#include <gtest/hwext/gtest-multithread.h>

#include <string>
#include <map>

#define private public
#include "ability_delegator.h"
#include "ability_manager_client.h"
#include "context_impl.h"
#undef private

#include "ability_delegator_infos.h"
#include "ability_delegator_registry.h"
#include "app_loader.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "mock_ability_delegator_stub.h"
#include "mock_iability_monitor.h"
#include "mock_test_observer_stub.h"
#include "mock_test_runner.h"
#include "ohos_application.h"
#include "scene_board_judgement.h"
#include "test_observer_stub.h"
#include "test_observer.h"
#include "test_runner.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {
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

class AbilityDelegatorTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void MakeMockObjects() const;

public:
    static std::shared_ptr<AbilityDelegator> commonDelegator_;
    static std::shared_ptr<AbilityDelegatorArgs> delegatorArgs_;
};

std::shared_ptr<AbilityDelegator> AbilityDelegatorTest::commonDelegator_ = nullptr;
std::shared_ptr<AbilityDelegatorArgs> AbilityDelegatorTest::delegatorArgs_ = nullptr;

void AbilityDelegatorTest::SetUpTestCase()
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
    auto testRunner = TestRunner::Create(AbilityRuntime::Runtime::Create(options), delegatorArgs_, true);
    commonDelegator_ = std::make_shared<AbilityDelegator>(std::make_shared<AbilityRuntime::ContextImpl>(),
        std::move(testRunner), sptr<IRemoteObject>(new AAFwk::MockTestObserverStub));
}

void AbilityDelegatorTest::TearDownTestCase()
{}

void AbilityDelegatorTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void AbilityDelegatorTest::TearDown()
{}

void AbilityDelegatorTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<OHOS::AAFwk::IAbilityManager>(new MockAbilityDelegatorStub);

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

class AbilityDelegatorTest2 : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void MakeMockObjects() const;
};

void AbilityDelegatorTest2::SetUpTestCase()
{}

void AbilityDelegatorTest2::TearDownTestCase()
{}

void AbilityDelegatorTest2::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void AbilityDelegatorTest2::TearDown()
{}

void AbilityDelegatorTest2::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<OHOS::AAFwk::IAbilityManager>(new MockAbilityDelegatorStub2);

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Ability_Delegator_Test_0100
 * @tc.name: ClearAllMonitors and AddAbilityMonitor and GetMonitorsNum.
 * @tc.desc: Verify the ClearAllMonitors and AddAbilityMonitor and GetMonitorsNum.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_0100 is called");
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
    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        abilityArgs,
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    abilityDelegator.ClearAllMonitors();
    std::shared_ptr<IAbilityMonitor> imotor = std::make_shared<IAbilityMonitor>(ABILITY_NAME);
    std::shared_ptr<IAbilityMonitor> imotor1 = std::make_shared<IAbilityMonitor>(ABILITY_NAME);

    abilityDelegator.AddAbilityMonitor(imotor);
    EXPECT_EQ((int)(abilityDelegator.GetMonitorsNum()), ONE);
    abilityDelegator.AddAbilityMonitor(imotor);
    EXPECT_EQ((int)(abilityDelegator.GetMonitorsNum()), ONE);
    abilityDelegator.AddAbilityMonitor(imotor1);
    EXPECT_EQ((int)(abilityDelegator.GetMonitorsNum()), TWO);
}

/**
 * @tc.number: Ability_Delegator_Test_0200
 * @tc.name: ClearAllMonitors and AddAbilityMonitor and RemoveAbilityMonitor and GetMonitorsNum.
 * @tc.desc: Verify the ClearAllMonitors and AddAbilityMonitor and RemoveAbilityMonitor and GetMonitorsNum.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_0200 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    std::shared_ptr<IAbilityMonitor> imotor = std::make_shared<IAbilityMonitor>(ABILITY_NAME);
    abilityDelegator.ClearAllMonitors();
    abilityDelegator.AddAbilityMonitor(imotor);
    EXPECT_EQ((int)(abilityDelegator.GetMonitorsNum()), ONE);
    abilityDelegator.RemoveAbilityMonitor(imotor);
    EXPECT_EQ((int)(abilityDelegator.GetMonitorsNum()), ZERO);
}

/**
 * @tc.number: Ability_Delegator_Test_0300
 * @tc.name: GetBundleName
 * @tc.desc: Verify the GetBundleName is null.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_0300 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    EXPECT_EQ(abilityDelegator.GetAppContext()->GetBundleName(), "");
}

/**
 * @tc.number: Ability_Delegator_Test_0400
 * @tc.name: GetAbilityState
 * @tc.desc: Verify the GetAbilityState input para is nullptr.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_0400 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    int abilityState = static_cast<int>(abilityDelegator.GetAbilityState(nullptr));

    EXPECT_EQ(abilityState, static_cast<int>(AbilityDelegator::AbilityState::UNINITIALIZED));
}

/**
 * @tc.number: Ability_Delegator_Test_0500
 * @tc.name: GetAbilityState
 * @tc.desc: Verify the GetAbilityState input para is valid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_0500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_0500 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<ADelegatorAbilityProperty> abilityProperty = std::make_shared<ADelegatorAbilityProperty>();
    abilityProperty->token_ = token;
    abilityProperty->name_ = ABILITY_NAME;
    abilityProperty->lifecycleState_ = AbilityLifecycleExecutor::LifecycleState::STARTED_NEW;
    abilityDelegator.abilityProperties_.emplace_front(abilityProperty);
    int abilityState = static_cast<int>(abilityDelegator.GetAbilityState(token));

    EXPECT_EQ(abilityState, static_cast<int>(AbilityDelegator::AbilityState::STARTED));
}

/**
 * @tc.number: Ability_Delegator_Test_0600
 * @tc.name: GetAbilityState
 * @tc.desc: Verify the GetAbilityState input para is valid but not find in abilityProperties_.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_0600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_0600 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    int abilityState = static_cast<int>(abilityDelegator.GetAbilityState(token));

    EXPECT_EQ(abilityState, static_cast<int>(AbilityDelegator::AbilityState::UNINITIALIZED));
}

/**
 * @tc.number: Ability_Delegator_Test_0700
 * @tc.name: GetCurrentTopAbility
 * @tc.desc: Verify the GetCurrentTopAbility is invalid.
 */
HWTEST_F(AbilityDelegatorTest2, Ability_Delegator_Test_070, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_0700 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub2());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub2);
    std::shared_ptr<ADelegatorAbilityProperty> ability = std::make_shared<ADelegatorAbilityProperty>();
    ability->token_ = token;
    ability->name_ = ABILITY_NAME;
    ability->fullName_ = ABILITY_NAME;
    abilityDelegator.abilityProperties_.emplace_back(ability);

    // Empty abilityName.
    MockAbilityDelegatorStub2::testcaseBranch_ = TESTCASE_BRANCH::BRANCH_1;
    EXPECT_EQ(abilityDelegator.GetCurrentTopAbility(), nullptr);

    // Unkonwn abilityName.
    MockAbilityDelegatorStub2::testcaseBranch_ = TESTCASE_BRANCH::BRANCH_2;
    EXPECT_EQ(abilityDelegator.GetCurrentTopAbility(), nullptr);

    // Valid abilityName.
    MockAbilityDelegatorStub2::testcaseBranch_ = TESTCASE_BRANCH::BRANCH_3;
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(abilityDelegator.GetCurrentTopAbility(), ability);
    }

    // Set testcase branch to default.
    MockAbilityDelegatorStub2::testcaseBranch_ = TESTCASE_BRANCH::BRANCH_1;
}

/**
 * @tc.number: Ability_Delegator_Test_0800
 * @tc.name: DoAbilityForeground
 * @tc.desc: Verify the DoAbilityForeground is valid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_0800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_0800 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
        EXPECT_TRUE(abilityDelegator.DoAbilityForeground(token));
    }

    EXPECT_TRUE(iRemoteObj != nullptr);
}

/**
 * @tc.number: Ability_Delegator_Test_0900
 * @tc.name: DoAbilityForeground
 * @tc.desc: Verify the DoAbilityForeground is invalid.
 */
HWTEST_F(AbilityDelegatorTest2, Ability_Delegator_Test_0900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_0900 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub2());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub2);

    EXPECT_FALSE(abilityDelegator.DoAbilityForeground(token));
}

/**
 * @tc.number: Ability_Delegator_Test_1000
 * @tc.name: DoAbilityForeground
 * @tc.desc: Verify the DoAbilityForeground input token is nullptr and result is invalid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_1000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_1000 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    EXPECT_FALSE(abilityDelegator.DoAbilityForeground(nullptr));
}

/**
 * @tc.number: Ability_Delegator_Test_1100
 * @tc.name: DoAbilityBackground
 * @tc.desc: Verify the DoAbilityBackground is valid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_1100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_1100 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
        EXPECT_TRUE(abilityDelegator.DoAbilityBackground(token));
    }

    EXPECT_TRUE(iRemoteObj != nullptr);
}

/**
 * @tc.number: Ability_Delegator_Test_1200
 * @tc.name: DoAbilityBackground
 * @tc.desc: Verify the DoAbilityBackground is invalid.
 */
HWTEST_F(AbilityDelegatorTest2, Ability_Delegator_Test_1200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_1200 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub2());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub2);

    EXPECT_FALSE(abilityDelegator.DoAbilityBackground(token));
}

/**
 * @tc.number: Ability_Delegator_Test_1300
 * @tc.name: DoAbilityBackground
 * @tc.desc: Verify the DoAbilityBackground input token is nullptr and result is invalid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_1300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_1300 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    EXPECT_FALSE(abilityDelegator.DoAbilityBackground(nullptr));
}

/**
 * @tc.number: Ability_Delegator_Test_1400
 * @tc.name: ExecuteShellCommand
 * @tc.desc: Verify the ExecuteShellCommand input para cmd is null and result is invalid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_1400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_1400 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    EXPECT_EQ(abilityDelegator.ExecuteShellCommand("", TIMEOUT), nullptr);
}

/**
 * @tc.number: Ability_Delegator_Test_1500
 * @tc.name: ExecuteShellCommand
 * @tc.desc: Verify the ExecuteShellCommand para observer is null and result is invalid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_1500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_1500 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    abilityDelegator.observer_ = nullptr;

    EXPECT_EQ(abilityDelegator.ExecuteShellCommand(CMD, TIMEOUT), nullptr);
}

/**
 * @tc.number: Ability_Delegator_Test_1600
 * @tc.name: ExecuteShellCommand
 * @tc.desc: Verify the ExecuteShellCommand is valid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_1600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_1600 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    sptr<IRemoteObject> shobserver = sptr<IRemoteObject>(new MockTestObserverStub);
    abilityDelegator.observer_ = shobserver;

    EXPECT_NE(abilityDelegator.ExecuteShellCommand(CMD, TIMEOUT), nullptr);
}

/**
 * @tc.number: Ability_Delegator_Test_1700
 * @tc.name: Print
 * @tc.desc: Verify the Print is invalid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_1700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_1700 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    sptr<IRemoteObject> shobserver = sptr<IRemoteObject>(new MockTestObserverStub);
    abilityDelegator.observer_ = nullptr;
    abilityDelegator.Print(PRINT_MSG);

    EXPECT_FALSE(iface_cast<MockTestObserverStub>(shobserver)->testStatusFlag);
}

/**
 * @tc.number: Ability_Delegator_Test_1800
 * @tc.name: Print
 * @tc.desc: Verify the Print is valid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_1800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_1800 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    sptr<IRemoteObject> shobserver = sptr<IRemoteObject>(new MockTestObserverStub);
    abilityDelegator.observer_ = shobserver;
    abilityDelegator.Print(PRINT_MSG);

    EXPECT_TRUE(iface_cast<MockTestObserverStub>(shobserver)->testStatusFlag);
}

/**
 * @tc.number: Ability_Delegator_Test_1900
 * @tc.name: ProcessAbilityProperties
 * @tc.desc: Verify the ProcessAbilityProperties list is add.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_1900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_1900 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<ADelegatorAbilityProperty> ability = std::make_shared<ADelegatorAbilityProperty>();
    ability->token_ = token;
    ability->name_ = ABILITY_NAME;
    ability->lifecycleState_ = AbilityLifecycleExecutor::LifecycleState::STARTED_NEW;
    ability->object_ = std::shared_ptr<NativeReference>();
    abilityDelegator.abilityProperties_.clear();
    abilityDelegator.PostPerformStart(ability);

    EXPECT_EQ(abilityDelegator.abilityProperties_.size(), ONE);
}

/**
 * @tc.number: Ability_Delegator_Test_2000
 * @tc.name: ProcessAbilityProperties
 * @tc.desc: Verify the ProcessAbilityProperties list is not add.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_2000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_2000 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<ADelegatorAbilityProperty> ability = std::make_shared<ADelegatorAbilityProperty>();
    ability->token_ = token;
    ability->name_ = ABILITY_NAME;
    ability->lifecycleState_ = AbilityLifecycleExecutor::LifecycleState::STARTED_NEW;
    ability->object_ = std::shared_ptr<NativeReference>();
    abilityDelegator.abilityProperties_.clear();
    abilityDelegator.abilityProperties_.emplace_back(ability);
    abilityDelegator.PostPerformStart(ability);
    EXPECT_EQ(abilityDelegator.abilityProperties_.size(), ONE);
}

/**
 * @tc.number: Ability_Delegator_Test_2100
 * @tc.name: PostPerformStart
 * @tc.desc: Verify the PostPerformStart is invalid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_2100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_2100 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator.abilityMonitors_.emplace_back(mockMonitor);
    abilityDelegator.PostPerformStart(nullptr);

    EXPECT_FALSE(mockMonitor->start_);
}

/**
 * @tc.number: Ability_Delegator_Test_2200
 * @tc.name: PostPerformStart
 * @tc.desc: Verify the PostPerformStart is valid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_2200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_2200 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator.abilityMonitors_.clear();
    abilityDelegator.abilityMonitors_.emplace_back(mockMonitor);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<ADelegatorAbilityProperty> ability = std::make_shared<ADelegatorAbilityProperty>();
    ability->token_ = token;
    ability->name_ = ABILITY_NAME;
    ability->lifecycleState_ = AbilityLifecycleExecutor::LifecycleState::STARTED_NEW;
    ability->object_ = std::shared_ptr<NativeReference>();
    abilityDelegator.PostPerformStart(ability);

    EXPECT_TRUE(mockMonitor->start_);
}

/**
 * @tc.number: Ability_Delegator_Test_2300
 * @tc.name: PostPerformScenceCreated
 * @tc.desc: Verify the PostPerformScenceCreated is invalid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_23400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_2300 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator.abilityMonitors_.emplace_back(mockMonitor);
    abilityDelegator.PostPerformScenceCreated(nullptr);

    EXPECT_FALSE(mockMonitor->windowStageCreate_);
}

/**
 * @tc.number: Ability_Delegator_Test_2400
 * @tc.name: PostPerformScenceCreated
 * @tc.desc: Verify the PostPerformScenceCreated is valid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_2400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_2400 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator.abilityMonitors_.clear();
    abilityDelegator.abilityMonitors_.emplace_back(mockMonitor);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<ADelegatorAbilityProperty> ability = std::make_shared<ADelegatorAbilityProperty>();
    ability->token_ = token;
    ability->name_ = ABILITY_NAME;
    ability->lifecycleState_ = AbilityLifecycleExecutor::LifecycleState::STARTED_NEW;
    ability->object_ = std::shared_ptr<NativeReference>();
    abilityDelegator.PostPerformScenceCreated(ability);

    EXPECT_TRUE(mockMonitor->windowStageCreate_);
}

/**
 * @tc.number: Ability_Delegator_Test_2500
 * @tc.name: PostPerformScenceRestored
 * @tc.desc: Verify the PostPerformScenceRestored is invalid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_2500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_2500 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator.abilityMonitors_.emplace_back(mockMonitor);
    abilityDelegator.PostPerformScenceRestored(nullptr);

    EXPECT_FALSE(mockMonitor->windowStageRestore_);
}

/**
 * @tc.number: Ability_Delegator_Test_2600
 * @tc.name: PostPerformScenceRestored
 * @tc.desc: Verify the PostPerformScenceRestored is valid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_2600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_2600 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator.abilityMonitors_.clear();
    abilityDelegator.abilityMonitors_.emplace_back(mockMonitor);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<ADelegatorAbilityProperty> ability = std::make_shared<ADelegatorAbilityProperty>();
    ability->token_ = token;
    ability->name_ = ABILITY_NAME;
    ability->lifecycleState_ = AbilityLifecycleExecutor::LifecycleState::STARTED_NEW;
    ability->object_ = std::shared_ptr<NativeReference>();
    abilityDelegator.PostPerformScenceRestored(ability);

    EXPECT_TRUE(mockMonitor->windowStageRestore_);
}

/**
 * @tc.number: Ability_Delegator_Test_2700
 * @tc.name: PostPerformScenceDestroyed
 * @tc.desc: Verify the PostPerformScenceDestroyed is invalid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_2700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_2700 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator.abilityMonitors_.emplace_back(mockMonitor);
    abilityDelegator.PostPerformScenceDestroyed(nullptr);

    EXPECT_FALSE(mockMonitor->windowStageDestroy_);
}

/**
 * @tc.number: Ability_Delegator_Test_2800
 * @tc.name: PostPerformScenceDestroyed
 * @tc.desc: Verify the PostPerformScenceDestroyed is valid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_2800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_2800 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator.abilityMonitors_.clear();
    abilityDelegator.abilityMonitors_.emplace_back(mockMonitor);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<ADelegatorAbilityProperty> ability = std::make_shared<ADelegatorAbilityProperty>();
    ability->token_ = token;
    ability->name_ = ABILITY_NAME;
    ability->lifecycleState_ = AbilityLifecycleExecutor::LifecycleState::STARTED_NEW;
    ability->object_ = std::shared_ptr<NativeReference>();
    abilityDelegator.PostPerformScenceDestroyed(ability);

    EXPECT_TRUE(mockMonitor->windowStageDestroy_);
}

/**
 * @tc.number: Ability_Delegator_Test_2900
 * @tc.name: PostPerformForeground
 * @tc.desc: Verify the PostPerformForeground is invalid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_2900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_2900 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator.abilityMonitors_.emplace_back(mockMonitor);
    abilityDelegator.PostPerformForeground(nullptr);

    EXPECT_FALSE(mockMonitor->foreground_);
}

/**
 * @tc.number: Ability_Delegator_Test_3000
 * @tc.name: PostPerformForeground
 * @tc.desc: Verify the PostPerformForeground is valid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_3000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_3000 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    abilityDelegator.abilityMonitors_.clear();
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator.abilityMonitors_.emplace_back(mockMonitor);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<ADelegatorAbilityProperty> ability = std::make_shared<ADelegatorAbilityProperty>();
    ability->token_ = token;
    ability->name_ = ABILITY_NAME;
    ability->lifecycleState_ = AbilityLifecycleExecutor::LifecycleState::STARTED_NEW;
    ability->object_ = std::shared_ptr<NativeReference>();
    abilityDelegator.PostPerformForeground(ability);

    EXPECT_TRUE(mockMonitor->foreground_);
}

/**
 * @tc.number: Ability_Delegator_Test_3100
 * @tc.name: PostPerformBackground
 * @tc.desc: Verify the PostPerformBackground is invalid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_3100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_3100 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    abilityDelegator.abilityMonitors_.clear();
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator.abilityMonitors_.emplace_back(mockMonitor);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<ADelegatorAbilityProperty> ability = std::make_shared<ADelegatorAbilityProperty>();
    ability->token_ = token;
    ability->name_ = ABILITY_NAME;
    ability->lifecycleState_ = AbilityLifecycleExecutor::LifecycleState::STARTED_NEW;
    abilityDelegator.PostPerformBackground(nullptr);

    EXPECT_FALSE(mockMonitor->background_);
}

/**
 * @tc.number: Ability_Delegator_Test_3200
 * @tc.name: PostPerformBackground
 * @tc.desc: Verify the PostPerformBackground is valid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_3200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_3200 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    abilityDelegator.abilityMonitors_.clear();
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator.abilityMonitors_.emplace_back(mockMonitor);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<ADelegatorAbilityProperty> ability = std::make_shared<ADelegatorAbilityProperty>();
    ability->token_ = token;
    ability->name_ = ABILITY_NAME;
    ability->lifecycleState_ = AbilityLifecycleExecutor::LifecycleState::STARTED_NEW;
    ability->object_ = std::shared_ptr<NativeReference>();
    abilityDelegator.PostPerformBackground(ability);

    EXPECT_TRUE(mockMonitor->background_);
}

/**
 * @tc.number: Ability_Delegator_Test_3300
 * @tc.name: PostPerformStop
 * @tc.desc: Verify the PostPerformStop is invalid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_3300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_3300 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    abilityDelegator.abilityMonitors_.clear();
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator.abilityMonitors_.emplace_back(mockMonitor);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<ADelegatorAbilityProperty> ability = std::make_shared<ADelegatorAbilityProperty>();
    ability->token_ = token;
    ability->name_ = ABILITY_NAME;
    ability->lifecycleState_ = AbilityLifecycleExecutor::LifecycleState::STARTED_NEW;
    abilityDelegator.PostPerformStop(nullptr);

    EXPECT_FALSE(mockMonitor->stop_);
}

/**
 * @tc.number: Ability_Delegator_Test_3400
 * @tc.name: PostPerformStop
 * @tc.desc: Verify the PostPerformStop is valid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_3400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_3400 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    abilityDelegator.abilityMonitors_.clear();
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator.abilityMonitors_.emplace_back(mockMonitor);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<ADelegatorAbilityProperty> ability = std::make_shared<ADelegatorAbilityProperty>();
    ability->token_ = token;
    ability->name_ = ABILITY_NAME;
    ability->lifecycleState_ = AbilityLifecycleExecutor::LifecycleState::STARTED_NEW;
    ability->object_ = std::shared_ptr<NativeReference>();
    abilityDelegator.PostPerformStop(ability);

    EXPECT_TRUE(mockMonitor->stop_);
}

/**
 * @tc.number: Ability_Delegator_Test_3500
 * @tc.name: FinishUserTest
 * @tc.desc: Verify the FinishUserTest is valid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_3500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_3500 is called");
    MockAbilityDelegatorStub::finishFlag_ = false;
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

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        abilityArgs,
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<AbilityDelegator> abilityDelegator =
        std::make_shared<AbilityDelegator>(context, std::move(testRunner), iRemoteObj);
    AbilityDelegatorRegistry::RegisterInstance(abilityDelegator, abilityArgs);

    abilityDelegator->abilityMonitors_.clear();
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator->abilityMonitors_.emplace_back(mockMonitor);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<ADelegatorAbilityProperty> ability = std::make_shared<ADelegatorAbilityProperty>();
    ability->token_ = token;
    ability->name_ = ABILITY_NAME;
    ability->lifecycleState_ = AbilityLifecycleExecutor::LifecycleState::STARTED_NEW;
    abilityDelegator->FinishUserTest(FINISH_MSG, FINISH_RESULT_CODE);

    EXPECT_TRUE(MockAbilityDelegatorStub::finishFlag_);
}

/**
 * @tc.number: Ability_Delegator_Test_3600
 * @tc.name: FinishUserTest
 * @tc.desc: Verify the FinishUserTest is invalid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_3600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_3600 is called");
    MockAbilityDelegatorStub2::finishFlag_ = false;
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

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        abilityArgs,
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub2);
    std::shared_ptr<AbilityDelegator> abilityDelegator =
        std::make_shared<AbilityDelegator>(context, std::move(testRunner), iRemoteObj);
    AbilityDelegatorRegistry::RegisterInstance(abilityDelegator, abilityArgs);

    abilityDelegator->abilityMonitors_.clear();
    std::shared_ptr<MockIabilityMonitor> mockMonitor = std::make_shared<MockIabilityMonitor>(ABILITY_NAME);
    abilityDelegator->abilityMonitors_.emplace_back(mockMonitor);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockAbilityDelegatorStub2);
    std::shared_ptr<ADelegatorAbilityProperty> ability = std::make_shared<ADelegatorAbilityProperty>();
    ability->token_ = token;
    ability->name_ = ABILITY_NAME;
    ability->lifecycleState_ = AbilityLifecycleExecutor::LifecycleState::STARTED_NEW;
    abilityDelegator->FinishUserTest(FINISH_MSG, FINISH_RESULT_CODE);

    EXPECT_FALSE(MockAbilityDelegatorStub2::finishFlag_);
}


/**
 * @tc.number: Ability_Delegator_Test_3700
 * @tc.name: ConvertAbilityState
 * @tc.desc: Verify the ConvertAbilityState and result is STARTED.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_3700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_3700 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new AAFwk::MockTestObserverStub);
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    EXPECT_EQ((int)(abilityDelegator.ConvertAbilityState(AbilityLifecycleExecutor::LifecycleState::STARTED_NEW)),
        (int)(AbilityDelegator::AbilityState::STARTED));
}

/**
 * @tc.number: Ability_Delegator_Test_3800
 * @tc.name: ConvertAbilityState
 * @tc.desc: Verify the ConvertAbilityState and result is FOREGROUND.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_3800, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_3800 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new AAFwk::MockTestObserverStub);
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    EXPECT_EQ((int)(abilityDelegator.ConvertAbilityState(AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW)),
        (int)(AbilityDelegator::AbilityState::FOREGROUND));
}

/**
 * @tc.number: Ability_Delegator_Test_3900
 * @tc.name: ConvertAbilityState
 * @tc.desc: Verify the ConvertAbilityState and result is BACKGROUND.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_3900, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_3900 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new AAFwk::MockTestObserverStub);
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    EXPECT_EQ((int)(abilityDelegator.ConvertAbilityState(AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW)),
        (int)(AbilityDelegator::AbilityState::BACKGROUND));
}

/**
 * @tc.number: Ability_Delegator_Test_4000
 * @tc.name: ConvertAbilityState
 * @tc.desc: Verify the ConvertAbilityState and result is STOPED.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_4000, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_4000 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new AAFwk::MockTestObserverStub);
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    EXPECT_EQ((int)(abilityDelegator.ConvertAbilityState(AbilityLifecycleExecutor::LifecycleState::STOPED_NEW)),
        (int)(AbilityDelegator::AbilityState::STOPPED));
}

/**
 * @tc.number: Ability_Delegator_Test_4100
 * @tc.name: ConvertAbilityState
 * @tc.desc: Verify the ConvertAbilityState and result is UNINITIALIZED.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_4100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_4100 is called");
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new AAFwk::MockTestObserverStub);
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);

    EXPECT_EQ((int)(abilityDelegator.ConvertAbilityState(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED)),
        (int)(AbilityDelegator::AbilityState::UNINITIALIZED));
}

/**
 * @tc.number: Ability_Delegator_Test_4200
 * @tc.name: Prepare
 * @tc.desc: Verify the Prepare is invalid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_4200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_4200 is called");
    MockTestRunner::prepareFlag_ = false;
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new AAFwk::MockTestObserverStub);
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    abilityDelegator.testRunner_ = nullptr;
    abilityDelegator.Prepare();

    EXPECT_FALSE(MockTestRunner::prepareFlag_);
}

/**
 * @tc.number: Ability_Delegator_Test_4300
 * @tc.name: Prepare
 * @tc.desc: Verify the Prepare is valid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_4300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_4300 is called");
    MockTestRunner::prepareFlag_ = false;
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new AAFwk::MockTestObserverStub);
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    std::unique_ptr<TestRunner> tptr{ new MockTestRunner };
    abilityDelegator.testRunner_ = std::move(tptr);
    abilityDelegator.Prepare();

    EXPECT_TRUE(MockTestRunner::prepareFlag_);
}

/**
 * @tc.number: Ability_Delegator_Test_4400
 * @tc.name: OnRun
 * @tc.desc: Verify the OnRun is invalid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_4400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_4400 is called");
    MockTestRunner::runFlag_ = false;
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new AAFwk::MockTestObserverStub);
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    abilityDelegator.testRunner_ = nullptr;
    abilityDelegator.OnRun();

    EXPECT_FALSE(MockTestRunner::runFlag_);
}

/**
 * @tc.number: Ability_Delegator_Test_4500
 * @tc.name: OnRun
 * @tc.desc: Verify the OnRun is valid.
 */
HWTEST_F(AbilityDelegatorTest, Ability_Delegator_Test_4500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Ability_Delegator_Test_4500 is called");
    MockTestRunner::runFlag_ = false;
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new AAFwk::MockTestObserverStub);
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    std::unique_ptr<TestRunner> tptr{ new MockTestRunner };
    abilityDelegator.testRunner_ = std::move(tptr);
    abilityDelegator.OnRun();

    EXPECT_TRUE(MockTestRunner::runFlag_);
}

/**
 * @tc.name: RegisterClearFuncTest_0100
 * @tc.desc: Register clear function test.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(AbilityDelegatorTest, RegisterClearFuncTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    ASSERT_NE(commonDelegator_, nullptr);

    // Register clear function.
    auto clearFunc = [](const std::shared_ptr<ADelegatorAbilityProperty> &property) {
        TAG_LOGI(AAFwkTag::TEST, "Clear function is called");
    };
    commonDelegator_->RegisterClearFunc(clearFunc);

    // Add ability monitor, so CallClearFunc can be called by PostPerformStop
    auto iMonitor = std::make_shared<IAbilityMonitor>(ABILITY_NAME);
    commonDelegator_->AddAbilityMonitor(iMonitor);
    auto abilityProperty = std::make_shared<ADelegatorAbilityProperty>();
    commonDelegator_->PostPerformStop(abilityProperty);
}

/**
 * @tc.name: FindPropertyByNameTest_0100
 * @tc.desc: Find property by name function test.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(AbilityDelegatorTest, FindPropertyByNameTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    ASSERT_NE(commonDelegator_, nullptr);

    auto result = commonDelegator_->FindPropertyByName("");
    EXPECT_EQ(result, nullptr);

    auto token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    auto abilityProperty = std::make_shared<ADelegatorAbilityProperty>();
    abilityProperty->token_ = token;
    abilityProperty->name_ = ABILITY_NAME;
    abilityProperty->fullName_ = ABILITY_NAME;
    commonDelegator_->ProcessAbilityProperties(abilityProperty);
    EXPECT_EQ(commonDelegator_->abilityProperties_.size(), 1);

    result = commonDelegator_->FindPropertyByName(ABILITY_NAME);
    EXPECT_EQ(result, abilityProperty);
}

/**
 * @tc.name: InputParamTest_0100
 * @tc.desc: Input param test.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(AbilityDelegatorTest, InputParamTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    ASSERT_NE(commonDelegator_, nullptr);

    // Process ability properties when ability is invalid.
    commonDelegator_->ProcessAbilityProperties(nullptr);

    // Remove ability property when ability is invalid.
    commonDelegator_->RemoveAbilityProperty(nullptr);

    // Find property by invalid token.
    commonDelegator_->FindPropertyByToken(nullptr);

    // Finish user test when observer is invalid.
    AbilityRuntime::Runtime::Options options;
    AAFwk::Want want;
    auto testRunner = TestRunner::Create(AbilityRuntime::Runtime::Create(options),
        std::make_shared<AbilityDelegatorArgs>(want), true);
    auto delegator = std::make_shared<AbilityDelegator>(std::make_shared<AbilityRuntime::ContextImpl>(),
        std::move(testRunner), nullptr);
    std::string msg("");
    int64_t resultCode = 0;
    delegator->FinishUserTest(msg, resultCode);
}

/**
 * @tc.name: AbilityMonitorEmptyTest_0100
 * @tc.desc: Perform when ability monitor is empty.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(AbilityDelegatorTest, AbilityMonitorEmptyTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    ASSERT_NE(commonDelegator_, nullptr);

    auto token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    auto abilityProperty = std::make_shared<ADelegatorAbilityProperty>();
    abilityProperty->token_ = token;
    abilityProperty->name_ = ABILITY_NAME;
    abilityProperty->fullName_ = ABILITY_NAME;
    commonDelegator_->PostPerformStart(abilityProperty);
    commonDelegator_->PostPerformScenceCreated(abilityProperty);
    commonDelegator_->PostPerformScenceRestored(abilityProperty);
    commonDelegator_->PostPerformScenceDestroyed(abilityProperty);
    commonDelegator_->PostPerformForeground(abilityProperty);
    commonDelegator_->PostPerformBackground(abilityProperty);
    commonDelegator_->PostPerformStop(abilityProperty);
}

/**
 * @tc.name: GetThreadNameTest_0100
 * @tc.desc: Get thread name test.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(AbilityDelegatorTest, GetThreadNameTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    ASSERT_NE(commonDelegator_, nullptr);
    auto result = commonDelegator_->GetThreadName();
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: StartAbilityTest_0100
 * @tc.desc: Get thread name test.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(AbilityDelegatorTest, StartAbilityTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    ASSERT_NE(commonDelegator_, nullptr);
    AbilityDelegatorRegistry::RegisterInstance(commonDelegator_, delegatorArgs_);

    AAFwk::Want want;
    want.SetElementName(VALUE_TEST_BUNDLE_NAME, ABILITY_NAME);
    auto result = commonDelegator_->StartAbility(want);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: WaitAbilityMonitorTest_0100
 * @tc.desc: Wait ability monitor test.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(AbilityDelegatorTest, WaitAbilityMonitorTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    ASSERT_NE(commonDelegator_, nullptr);
    auto property = commonDelegator_->WaitAbilityMonitor(nullptr);
    EXPECT_EQ(property, nullptr);
}

/**
 * @tc.name: WaitAbilityMonitorTest_0200
 * @tc.desc: Wait ability monitor test.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
std::shared_ptr<IAbilityMonitor> gt_iAbilityMonitor = nullptr;

void IAbilityMonitorWaitTask()
{
    ASSERT_NE(gt_iAbilityMonitor, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "Running in thread %{public}d", gettid());
    auto property = AbilityDelegatorTest::commonDelegator_->WaitAbilityMonitor(gt_iAbilityMonitor);
    if (property == nullptr) {
        TAG_LOGW(AAFwkTag::TEST, "Wait for ability failed.");
    }
}

void IAbilityMonitorMatchTask()
{
    ASSERT_NE(gt_iAbilityMonitor, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "Running in thread %{public}d", gettid());
    auto token = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<ADelegatorAbilityProperty> proterty = std::make_shared<ADelegatorAbilityProperty>();
    proterty->token_ = token;
    proterty->name_ = ABILITY_NAME;
    EXPECT_TRUE(gt_iAbilityMonitor->Match(proterty, true));
}

HWTEST_F(AbilityDelegatorTest, WaitAbilityMonitorTest_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    ASSERT_NE(commonDelegator_, nullptr);

    gt_iAbilityMonitor = std::make_shared<IAbilityMonitor>(ABILITY_NAME);
    commonDelegator_->AddAbilityMonitor(gt_iAbilityMonitor);
    SET_THREAD_NUM(1);
    GTEST_RUN_TASK(IAbilityMonitorWaitTask);
    GTEST_RUN_TASK(IAbilityMonitorMatchTask);
    gt_iAbilityMonitor.reset();
}

/**
 * @tc.name: WaitAbilityStageMonitorTest_0100
 * @tc.desc: Wait ability stage monitor test.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(AbilityDelegatorTest, WaitAbilityStageMonitorTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    ASSERT_NE(commonDelegator_, nullptr);
    auto property = commonDelegator_->WaitAbilityStageMonitor(nullptr);
    EXPECT_EQ(property, nullptr);
}

/**
 * @tc.name: WaitAbilityStageMonitorTest_0200
 * @tc.desc: Wait ability stage monitor test.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
std::shared_ptr<IAbilityStageMonitor> gt_iAbilityStageMonitor = nullptr;

void IAbilityStageMonitorWaitTask()
{
    ASSERT_NE(gt_iAbilityStageMonitor, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "Running in thread %{public}d", gettid());
    auto property = AbilityDelegatorTest::commonDelegator_->WaitAbilityStageMonitor(gt_iAbilityStageMonitor);
    if (property == nullptr) {
        TAG_LOGW(AAFwkTag::TEST, "Wait for ability failed.");
    }
}

void IAbilityStageMonitorMatchTask()
{
    ASSERT_NE(gt_iAbilityStageMonitor, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "Running in thread %{public}d", gettid());
    std::shared_ptr<DelegatorAbilityStageProperty> property = std::make_shared<DelegatorAbilityStageProperty>();
    property->moduleName_ = ABILITY_STAGE_MONITOR_MODULE_NAME;
    property->srcEntrance_ = ABILITY_STAGE_MONITOR_SRC_ENTRANCE;
    EXPECT_TRUE(gt_iAbilityStageMonitor->Match(property, true));
}

HWTEST_F(AbilityDelegatorTest, WaitAbilityStageMonitorTest_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    ASSERT_NE(commonDelegator_, nullptr);

    gt_iAbilityStageMonitor = std::make_shared<IAbilityStageMonitor>(ABILITY_STAGE_MONITOR_MODULE_NAME,
        ABILITY_STAGE_MONITOR_SRC_ENTRANCE);
    commonDelegator_->AddAbilityStageMonitor(gt_iAbilityStageMonitor);
    SET_THREAD_NUM(1);
    GTEST_RUN_TASK(IAbilityStageMonitorWaitTask);
    GTEST_RUN_TASK(IAbilityStageMonitorMatchTask);
    gt_iAbilityStageMonitor.reset();
}

/**
 * @tc.name: PostPerformStageStartTest_0100
 * @tc.desc: Post perform stage start when ability stage monitor is empty.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(AbilityDelegatorTest, PostPerformStageStartTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    ASSERT_NE(commonDelegator_, nullptr);
    auto property = std::make_shared<DelegatorAbilityStageProperty>();
    property->moduleName_ = ABILITY_STAGE_MONITOR_MODULE_NAME;
    property->srcEntrance_ = ABILITY_STAGE_MONITOR_SRC_ENTRANCE;
    commonDelegator_->PostPerformStageStart(property);
}

/**
 * @tc.name: PostPerformStageStartTest_0200
 * @tc.desc: Post perform stage start when ability stage monitor is valid.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(AbilityDelegatorTest, PostPerformStageStartTest_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    ASSERT_NE(commonDelegator_, nullptr);

    auto stageMonitor = std::make_shared<IAbilityStageMonitor>(ABILITY_STAGE_MONITOR_MODULE_NAME,
        ABILITY_STAGE_MONITOR_SRC_ENTRANCE);
    commonDelegator_->AddAbilityStageMonitor(stageMonitor);
    auto property = std::make_shared<DelegatorAbilityStageProperty>();
    property->moduleName_ = ABILITY_STAGE_MONITOR_MODULE_NAME;
    property->srcEntrance_ = ABILITY_STAGE_MONITOR_SRC_ENTRANCE;
    commonDelegator_->PostPerformStageStart(property);
}
} // namespace AppExecFwk
} // namespace OHOS
