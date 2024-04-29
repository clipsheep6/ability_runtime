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

#include <map>
#include <string>
#include <thread>
#include <iostream>

#include "ability_lifecycle_executor.h"

#define private public
#include "iability_monitor.h"
#include "iability_stage_monitor.h"
#include "ability_manager_client.h"
#undef private

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "mock_ability_delegator_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string ABILITY_NAME = "com.example.myapplication.MainAbilitymodule";
const std::string PROPERTY_ABILITY_NAME = "com.example.myapplication.MainAbilitymodule";
const std::string PROPERTY_ABILITY_NAME1 = "com.example.myapplication.MainAbility1module";
const std::string ABILITY_STAGE_MODULE_NAME = "com.example.entry_test";
const std::string ABILITY_STAGE_SOURCE_ENTRANCE = "./ets/Application/TestAbilityStage.ts";
const std::string PROPERTY_ABILITY_STAGE_MODULE_NAME = "com.example.entry_test";
const std::string PROPERTY_ABILITY_STAGE_SOURCE_ENTRANCE = "./ets/Application/TestAbilityStage.ts";
const std::string PROPERTY_ABILITY_STAGE_MODULE_NAME2 = "com.example.entry_test2";
const std::string PROPERTY_ABILITY_STAGE_SOURCE_ENTRANCE2 = "./ets/Application/TestAbilityStage2.ts";
}

class IabilityMonitorModuleTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;
};

void IabilityMonitorModuleTest::SetUpTestCase()
{}

void IabilityMonitorModuleTest::TearDownTestCase()
{}

void IabilityMonitorModuleTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void IabilityMonitorModuleTest::TearDown()
{}

void IabilityMonitorModuleTest::MakeMockObjects() const
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
HWTEST_F(IabilityMonitorModuleTest, Iability_Monitor_Test_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Iability_Monitor_Test_0100 is called");

    IAbilityMonitor iabilityMonitor(ABILITY_NAME);
    std::shared_ptr<ADelegatorAbilityProperty> property = std::make_shared<ADelegatorAbilityProperty>();
    property->token_ = new MockAbilityDelegatorStub;
    property->name_ = PROPERTY_ABILITY_NAME;
    EXPECT_TRUE(iabilityMonitor.Match(property));
}

/**
 * @tc.number: Iability_Monitor_Test_0200
 * @tc.name: Match AbilityStage
 * @tc.desc: Verify the AbilityStage Match.
 * @tc.require: issueI5801E
 */
HWTEST_F(IabilityMonitorModuleTest, Iability_Monitor_Test_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Iability_Monitor_Test_0200 is called");

    IAbilityStageMonitor stageMonitor(ABILITY_STAGE_MODULE_NAME, ABILITY_STAGE_SOURCE_ENTRANCE);
    std::shared_ptr<DelegatorAbilityStageProperty> property = std::make_shared<DelegatorAbilityStageProperty>();
    property->moduleName_ = PROPERTY_ABILITY_STAGE_MODULE_NAME;
    property->srcEntrance_ = PROPERTY_ABILITY_STAGE_SOURCE_ENTRANCE;
    EXPECT_TRUE(stageMonitor.Match(property));

    std::shared_ptr<DelegatorAbilityStageProperty> property2 = std::make_shared<DelegatorAbilityStageProperty>();
    property->moduleName_ = PROPERTY_ABILITY_STAGE_MODULE_NAME2;
    property->srcEntrance_ = PROPERTY_ABILITY_STAGE_SOURCE_ENTRANCE;
    EXPECT_FALSE(stageMonitor.Match(property2));

    std::shared_ptr<DelegatorAbilityStageProperty> property3 = std::make_shared<DelegatorAbilityStageProperty>();
    property->moduleName_ = PROPERTY_ABILITY_STAGE_MODULE_NAME;
    property->srcEntrance_ = PROPERTY_ABILITY_STAGE_SOURCE_ENTRANCE2;
    EXPECT_FALSE(stageMonitor.Match(property3));
}

/**
 * @tc.name: MatchTest_0100
 * @tc.desc: Match test when ability stage is invalid.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(IabilityMonitorModuleTest, MatchTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    IAbilityStageMonitor stageMonitor(ABILITY_STAGE_MODULE_NAME, ABILITY_STAGE_SOURCE_ENTRANCE);
    EXPECT_FALSE(stageMonitor.Match(nullptr));
}

/**
 * @tc.name: MatchTest_0200
 * @tc.desc: Test notify when matched.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(IabilityMonitorModuleTest, MatchTest_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    IAbilityStageMonitor stageMonitor(ABILITY_STAGE_MODULE_NAME, ABILITY_STAGE_SOURCE_ENTRANCE);
    std::shared_ptr<DelegatorAbilityStageProperty> property = std::make_shared<DelegatorAbilityStageProperty>();
    property->moduleName_ = PROPERTY_ABILITY_STAGE_MODULE_NAME;
    property->srcEntrance_ = PROPERTY_ABILITY_STAGE_SOURCE_ENTRANCE;
    EXPECT_TRUE(stageMonitor.Match(property, true));
}

/**
 * @tc.name: WaitForAbility_0100
 * @tc.desc: Wait for ability timeout.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
HWTEST_F(IabilityMonitorModuleTest, WaitForAbilityTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    IAbilityStageMonitor stageMonitor(ABILITY_STAGE_MODULE_NAME, ABILITY_STAGE_SOURCE_ENTRANCE);

    // wait for 100ms until timeout
    EXPECT_EQ(stageMonitor.WaitForAbilityStage(100), nullptr);
}

/**
 * @tc.name: WaitForAbilityTest_0200
 * @tc.desc: Wait for ability in multi-thread test.
 * @tc.type: FUNC
 * @tc.require: issueI76SHL
 */
std::shared_ptr<IAbilityStageMonitor> gt_iAbilityStageMonitor = nullptr;

void IAbilityStageMonitorWaitTask()
{
    ASSERT_NE(gt_iAbilityStageMonitor, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "Running in thread %{public}d", gettid());
    auto property = gt_iAbilityStageMonitor->WaitForAbilityStage();
    if (property == nullptr) {
        TAG_LOGW(AAFwkTag::TEST, "Wait for ability stage failed.");
    }
}

void IAbilityStageMonitorMatchTask()
{
    ASSERT_NE(gt_iAbilityStageMonitor, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "Running in thread %{public}d", gettid());
    std::shared_ptr<DelegatorAbilityStageProperty> property = std::make_shared<DelegatorAbilityStageProperty>();
    property->moduleName_ = PROPERTY_ABILITY_STAGE_MODULE_NAME;
    property->srcEntrance_ = PROPERTY_ABILITY_STAGE_SOURCE_ENTRANCE;
    EXPECT_TRUE(gt_iAbilityStageMonitor->Match(property, true));
}

HWTEST_F(IabilityMonitorModuleTest, WaitForAbilityTest_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "test start.");
    gt_iAbilityStageMonitor = std::make_shared<IAbilityStageMonitor>(ABILITY_STAGE_MODULE_NAME,
        ABILITY_STAGE_SOURCE_ENTRANCE);
    SET_THREAD_NUM(1);
    GTEST_RUN_TASK(IAbilityStageMonitorWaitTask);
    GTEST_RUN_TASK(IAbilityStageMonitorMatchTask);
    gt_iAbilityStageMonitor.reset();
}
} // namespace AppExecFwk
} // namespace OHOS
