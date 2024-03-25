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

#include <map>
#include <string>
#include <thread>
#include <iostream>

#include "ability_lifecycle_executor.h"

#define private public
#include "iability_monitor.h"
#include "ability_manager_client.h"
#undef private

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "mock_ability_delegator_stub.h"
#include "native_engine/native_reference.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string ABILITY_NAME = "com.example.myapplication.MainAbility";
const std::string PROPERTY_ABILITY_NAME = "com.example.myapplication.MainAbility";
const std::string PROPERTY_ABILITY_NAME1 = "com.example.myapplication.MainAbility1";
}

class IabilityMonitorTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;
};

void IabilityMonitorTest::SetUpTestCase()
{}

void IabilityMonitorTest::TearDownTestCase()
{}

void IabilityMonitorTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void IabilityMonitorTest::TearDown()
{}

void IabilityMonitorTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<OHOS::AAFwk::IAbilityManager>(new MockAbilityDelegatorStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Iability_Monitor_Test_0100
 * @tc.name: Match
 * @tc.desc: Verify the Match.
 */
HWTEST_F(IabilityMonitorTest, Iability_Monitor_Test_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Iability_Monitor_Test_0100 is called");

    IAbilityMonitor iabilityMonitor(ABILITY_NAME);
    EXPECT_FALSE(iabilityMonitor.Match(nullptr));
}

/**
 * @tc.number: Iability_Monitor_Test_0200
 * @tc.name: Match
 * @tc.desc: Verify the Match.
 */
HWTEST_F(IabilityMonitorTest, Iability_Monitor_Test_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Iability_Monitor_Test_0200 is called");

    IAbilityMonitor iabilityMonitor("");
    std::shared_ptr<ADelegatorAbilityProperty> proterty = std::make_shared<ADelegatorAbilityProperty>();
    proterty->token_ = new MockAbilityDelegatorStub;
    proterty->name_ = PROPERTY_ABILITY_NAME;
    EXPECT_FALSE(iabilityMonitor.Match(proterty));
}

/**
 * @tc.number: Iability_Monitor_Test_0300
 * @tc.name: Match
 * @tc.desc: Verify the Match.
 */
HWTEST_F(IabilityMonitorTest, Iability_Monitor_Test_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Iability_Monitor_Test_0300 is called");

    IAbilityMonitor iabilityMonitor(ABILITY_NAME);
    std::shared_ptr<ADelegatorAbilityProperty> proterty = std::make_shared<ADelegatorAbilityProperty>();
    proterty->token_ = new MockAbilityDelegatorStub;
    proterty->name_ = PROPERTY_ABILITY_NAME;
    EXPECT_TRUE(iabilityMonitor.Match(proterty));
}

/**
 * @tc.name: MatchTest_0100
 * @tc.desc: Match test when ability name is different.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(IabilityMonitorTest, MatchTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    IAbilityMonitor iabilityMonitor(ABILITY_NAME);
    std::shared_ptr<ADelegatorAbilityProperty> proterty = std::make_shared<ADelegatorAbilityProperty>();
    proterty->token_ = new MockAbilityDelegatorStub;
    proterty->name_ = PROPERTY_ABILITY_NAME1;
    EXPECT_FALSE(iabilityMonitor.Match(proterty));
}

/**
 * @tc.name: MatchTest_0200
 * @tc.desc: Test notify when matched.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(IabilityMonitorTest, MatchTest_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    IAbilityMonitor iabilityMonitor(ABILITY_NAME);
    std::shared_ptr<ADelegatorAbilityProperty> proterty = std::make_shared<ADelegatorAbilityProperty>();
    proterty->token_ = new MockAbilityDelegatorStub;
    proterty->name_ = PROPERTY_ABILITY_NAME;
    EXPECT_TRUE(iabilityMonitor.Match(proterty, true));
}

/**
 * @tc.name: WaitForAbility_0100
 * @tc.desc: Wait for ability timeout.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(IabilityMonitorTest, WaitForAbilityTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    IAbilityMonitor iabilityMonitor(ABILITY_NAME);

    // wait for 100ms until timeout
    EXPECT_EQ(iabilityMonitor.WaitForAbility(100), nullptr);
}

/**
 * @tc.name: WaitForAbilityTest_0200
 * @tc.desc: Wait for ability in multi-thread test.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
std::shared_ptr<IAbilityMonitor> gt_iAbilityMonitor = nullptr;

void IAbilityMonitorWaitTask()
{
    ASSERT_NE(gt_iAbilityMonitor, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "Running in thread %{public}d", gettid());
    auto property = gt_iAbilityMonitor->WaitForAbility();
    if (property == nullptr) {
        TAG_LOGW(AAFwkTag::TEST, "Wait for ability failed.");
    }
}

void IAbilityMonitorMatchTask()
{
    ASSERT_NE(gt_iAbilityMonitor, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "Running in thread %{public}d", gettid());
    std::shared_ptr<ADelegatorAbilityProperty> proterty = std::make_shared<ADelegatorAbilityProperty>();
    proterty->token_ = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    proterty->name_ = PROPERTY_ABILITY_NAME;
    EXPECT_TRUE(gt_iAbilityMonitor->Match(proterty, true));
}

HWTEST_F(IabilityMonitorTest, WaitForAbilityTest_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    gt_iAbilityMonitor = std::make_shared<IAbilityMonitor>(ABILITY_NAME);
    SET_THREAD_NUM(1);
    GTEST_RUN_TASK(IAbilityMonitorWaitTask);
    GTEST_RUN_TASK(IAbilityMonitorMatchTask);
    gt_iAbilityMonitor.reset();
}

/**
 * @tc.name: FuncTest_0100
 * @tc.desc: IAbilityMonitor function test.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(IabilityMonitorTest, FuncTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    auto iabilityMonitor = new IAbilityMonitor(ABILITY_NAME);
    ASSERT_NE(iabilityMonitor, nullptr);
    auto nativeRef = std::shared_ptr<NativeReference>();
    iabilityMonitor->OnAbilityStart(nativeRef);
    iabilityMonitor->OnAbilityForeground(nativeRef);
    iabilityMonitor->OnAbilityBackground(nativeRef);
    iabilityMonitor->OnAbilityStop(nativeRef);
    iabilityMonitor->OnWindowStageCreate(nativeRef);
    iabilityMonitor->OnWindowStageRestore(nativeRef);
    iabilityMonitor->OnWindowStageDestroy(nativeRef);
    delete iabilityMonitor;
}
} // namespace AppExecFwk
} // namespace OHOS
