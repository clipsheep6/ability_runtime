/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <memory>
#include <thread>
#include <chrono>
#include "system_ability_definition.h"
#include "ability_info.h"
#include "application_info.h"
#define private public
#define protected public
#include "ability_manager_service.h"
#undef private
#undef protected
#include "gtest/gtest.h"
#include "ability_manager_errors.h"
#include "mock_bundle_manager.h"
#include "want.h"
#include "sa_mgr_client.h"
#include "appmgr_test_service.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
#define SLEEP(milli) std::this_thread::sleep_for(std::chrono::seconds(milli))

namespace {
const std::string NAME_BUNDLE_MGR_SERVICE = "BundleMgrService";
static ElementName g_testAbility1("device", "com.ix.test1", "MainAbility1");
static ElementName g_testAbility2("device", "com.ix.test2", "MainAbility2");
static ElementName g_testAbility3("device", "com.ix.test3", "MainAbility3");
static ElementName g_testAbility4("device", "com.ix.test4", "MainAbility4");
static ElementName g_launcherAbility("device", "com.ix.hiworld", "MainAbility");
}  // namespace
class AbilityWithApplicationsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void OnStartabilityAms();
public:
    static constexpr int TEST_WAIT_TIME = 100000;
    std::shared_ptr<AbilityManagerService> abilityMs_ {nullptr};
};

void AbilityWithApplicationsTest::SetUpTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}
void AbilityWithApplicationsTest::TearDownTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void AbilityWithApplicationsTest::SetUp()
{
    abilityMs_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    OnStartabilityAms();
}

void AbilityWithApplicationsTest::TearDown()
{
    abilityMs_->OnStop();
    OHOS::DelayedSingleton<AbilityManagerService>::DestroyInstance();
}

void AbilityWithApplicationsTest::OnStartabilityAms()
{
    if (abilityMs_) {
        if (abilityMs_->state_ == ServiceRunningState::STATE_RUNNING) {
            return;
        }

        abilityMs_->state_ = ServiceRunningState::STATE_RUNNING;
        
        abilityMs_->eventLoop_ = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
        EXPECT_TRUE(abilityMs_->eventLoop_);

        abilityMs_->handler_ = std::make_shared<AbilityEventHandler>(abilityMs_->eventLoop_, abilityMs_);
        EXPECT_TRUE(abilityMs_->handler_);
        EXPECT_TRUE(abilityMs_->connectManager_);

        abilityMs_->connectManager_->SetEventHandler(abilityMs_->handler_);

        abilityMs_->dataAbilityManager_ = std::make_shared<DataAbilityManager>();
        EXPECT_TRUE(abilityMs_->dataAbilityManager_);

        abilityMs_->amsConfigResolver_ = std::make_shared<AmsConfigurationParameter>();
        EXPECT_TRUE(abilityMs_->amsConfigResolver_);
        abilityMs_->amsConfigResolver_->Parse();

        abilityMs_->pendingWantManager_ = std::make_shared<PendingWantManager>();
        EXPECT_TRUE(abilityMs_->pendingWantManager_);
      
        int userId = abilityMs_->GetUserId();
        abilityMs_->SetStackManager(userId);
        abilityMs_->systemAppManager_ = std::make_shared<KernalSystemAppManager>(userId);
        EXPECT_TRUE(abilityMs_->systemAppManager_);

        abilityMs_->eventLoop_->Run();

        return;
    }

    GTEST_LOG_(INFO) << "OnStart fail";
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify that the result of StartAbility with applications
 */
HWTEST_F(AbilityWithApplicationsTest, Start_Ability_With_Applications_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "AbilityWithApplicationTest_001 start";
    Want want1;
    want1.SetElement(g_testAbility1);
    Want want2;
    want2.SetElement(g_testAbility2);
    if (abilityMs_->GetStackManager()->GetCurrentTopAbility()) {
        abilityMs_->GetStackManager()->GetCurrentTopAbility()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    }
    int result = abilityMs_->StartAbility(want1);
    EXPECT_EQ(0, result);
    if (abilityMs_->GetStackManager()->GetCurrentTopAbility()) {
        abilityMs_->GetStackManager()->GetCurrentTopAbility()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    }
    result = abilityMs_->StartAbility(want2);
    if (abilityMs_->GetStackManager()->GetCurrentTopAbility()) {
        abilityMs_->GetStackManager()->GetCurrentTopAbility()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    }
    /* get stack manager */
    std::shared_ptr<AbilityStackManager> stackmgr = abilityMs_->GetStackManager();
    std::shared_ptr<MissionRecord> curMission;
    if (stackmgr) {
        curMission = stackmgr->GetCurrentMissionStack()->GetTopMissionRecord();
    }
    std::shared_ptr<AbilityRecord> topAbility;
    if (curMission) {
        topAbility = curMission->GetTopAbilityRecord();
    }
    EXPECT_TRUE(topAbility != nullptr);
    EXPECT_TRUE(curMission != nullptr);
    if (topAbility) {
        EXPECT_NE("com.ix.test1", topAbility->GetAbilityInfo().applicationName);
        EXPECT_NE(BACKGROUND, topAbility->GetAbilityState());
    }
    if (curMission) {
        std::shared_ptr<AbilityRecord> bottomAbility = curMission->GetBottomAbilityRecord();
        EXPECT_TRUE(bottomAbility != nullptr);
        EXPECT_EQ("com.ix.test1", bottomAbility->GetAbilityInfo().applicationName);
        EXPECT_NE(BACKGROUND, bottomAbility->GetAbilityState());
    }
    /* clear the mission stack */
    AbilityRequest request;
    abilityMs_->GetStackManager()->GetTargetMissionStack(request)->RemoveAll();
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify that the result of StartAbility with applications when launch active
 */
HWTEST_F(AbilityWithApplicationsTest, Start_Ability_With_Applications_002, TestSize.Level2)
{
    Want want1;
    want1.SetElement(g_testAbility1);
    Want want2;
    want2.SetElement(g_testAbility2);
    Want want3;
    want3.SetElement(g_launcherAbility);
    want3.AddEntity(Want::ENTITY_HOME);
    Want want4;
    want4.SetElement(g_testAbility4);
    if (abilityMs_->GetStackManager()->GetCurrentTopAbility()) {
        abilityMs_->GetStackManager()->GetCurrentTopAbility()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    }
    int result = abilityMs_->StartAbility(want1);
    EXPECT_EQ(0, result);
    SLEEP(2);
    if (abilityMs_->GetStackManager()->GetCurrentTopAbility()) {
        abilityMs_->GetStackManager()->GetCurrentTopAbility()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    }
    result = abilityMs_->StartAbility(want2);
    SLEEP(2);
    EXPECT_EQ(0, result);
    if (abilityMs_->GetStackManager()->GetCurrentTopAbility()) {
        abilityMs_->GetStackManager()->GetCurrentTopAbility()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    }
    result = abilityMs_->StartAbility(want3);
    EXPECT_EQ(0, result);
    SLEEP(2);
    if (abilityMs_->GetStackManager()->GetCurrentTopAbility()) {
        abilityMs_->GetStackManager()->GetCurrentTopAbility()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    }
    result = abilityMs_->StartAbility(want4);
    EXPECT_EQ(0, result);
    SLEEP(2);
    if (abilityMs_->GetStackManager()->GetCurrentTopAbility()) {
        abilityMs_->GetStackManager()->GetCurrentTopAbility()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    }
    auto stackmgr = abilityMs_->GetStackManager();
    auto curMission = stackmgr->GetCurrentMissionStack();
    EXPECT_NE(curMission, nullptr);
    auto topAbility = curMission->GetTopAbilityRecord();
    EXPECT_TRUE(topAbility != nullptr);
    EXPECT_NE("com.ix.test1", topAbility->GetAbilityInfo().applicationName);
    EXPECT_NE(BACKGROUND, topAbility->GetAbilityState());
    AbilityRequest request;
    abilityMs_->GetStackManager()->GetTargetMissionStack(request)->RemoveAll();
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbility
 * EnvConditions: NA
 * CaseDescription: terminate the ability in mission, and active launchAbility
 */
HWTEST_F(AbilityWithApplicationsTest, Teminate_Ability_With_Applications_001, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "Teminate_Ability_With_Applications_001 begin";
    Want want1;
    want1.SetElement(g_testAbility1);

    if (abilityMs_->GetStackManager()->GetCurrentTopAbility()) {
        abilityMs_->GetStackManager()->GetCurrentTopAbility()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    }

    int result = abilityMs_->StartAbility(want1);
    EXPECT_EQ(0, result);
    SLEEP(2);

    if (abilityMs_->GetStackManager()->GetCurrentTopAbility()) {
        abilityMs_->GetStackManager()->GetCurrentTopAbility()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    }
    /* get stack manager */
    std::shared_ptr<AbilityStackManager> stackmgr = abilityMs_->GetStackManager();
    std::shared_ptr<MissionRecord> curMission;
    if (stackmgr) {
        curMission = stackmgr->GetCurrentMissionStack()->GetTopMissionRecord();
    }

    std::shared_ptr<AbilityRecord> topAbility;
    if (curMission) {
        topAbility = curMission->GetTopAbilityRecord();
    }

    EXPECT_TRUE(topAbility != nullptr);
    EXPECT_TRUE(curMission != nullptr);

    if (topAbility) {
        EXPECT_EQ("com.ix.test1", topAbility->GetAbilityInfo().applicationName);
        EXPECT_NE(BACKGROUND, topAbility->GetAbilityState());
    }

    if (curMission) {
        std::shared_ptr<AbilityRecord> bottomAbility = curMission->GetBottomAbilityRecord();
        EXPECT_TRUE(bottomAbility != nullptr);
        EXPECT_EQ("com.ix.test1", bottomAbility->GetAbilityInfo().applicationName);
        EXPECT_NE(BACKGROUND, bottomAbility->GetAbilityState());
    }

    EXPECT_TRUE(topAbility != nullptr);
    abilityMs_->TerminateAbility(topAbility->GetToken(), -1);
    SLEEP(2);
    auto missionstack = stackmgr->GetCurrentMissionStack();

    if (missionstack) {
        topAbility = missionstack->GetTopAbilityRecord();
    }

    if (topAbility) {
        EXPECT_EQ("com.ohos.launcher", topAbility->GetAbilityInfo().applicationName);
        EXPECT_NE(ACTIVE, topAbility->GetAbilityState());
    }

    if (missionstack) {
        missionstack->RemoveAll();
    }

    GTEST_LOG_(INFO) << "Teminate_Ability_With_Applications_001 end";
}

/*
 * Feature: AbilityManagerService
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService TerminateAbility
 * EnvConditions: NA
 * CaseDescription: terminate the ability in mission, and active prev app ability
 */
HWTEST_F(AbilityWithApplicationsTest, Teminate_Ability_With_Applications_002, TestSize.Level2)
{
    GTEST_LOG_(INFO) << "Teminate_Ability_With_Applications_002 begin";
    Want want1;
    want1.SetElement(g_testAbility1);
    Want want2;
    want2.SetElement(g_testAbility2);
    if (abilityMs_->GetStackManager()->GetCurrentTopAbility()) {
        abilityMs_->GetStackManager()->GetCurrentTopAbility()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    }
    int result = abilityMs_->StartAbility(want1);
    SLEEP(2);
    EXPECT_EQ(0, result);
    if (abilityMs_->GetStackManager()->GetCurrentTopAbility()) {
        abilityMs_->GetStackManager()->GetCurrentTopAbility()->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    }
    result = abilityMs_->StartAbility(want2);
    SLEEP(2);
    EXPECT_EQ(0, result);

    /* get stack manager */
    /* get stack manager */
    std::shared_ptr<AbilityStackManager> stackmgr = abilityMs_->GetStackManager();
    std::shared_ptr<MissionRecord> curMission;
    if (stackmgr) {
        curMission = stackmgr->GetCurrentMissionStack()->GetTopMissionRecord();
    }
    GTEST_LOG_(INFO) << "GetTopAbilityRecord";
    std::shared_ptr<AbilityRecord> topAbility;
    if (curMission) {
        topAbility = curMission->GetTopAbilityRecord();
    }

    EXPECT_TRUE(topAbility != nullptr);
    EXPECT_TRUE(curMission != nullptr);

    if (topAbility) {
        EXPECT_NE("com.ix.test1", topAbility->GetAbilityInfo().applicationName);
        EXPECT_NE(BACKGROUND, topAbility->GetAbilityState());
    }

    if (curMission) {
        std::shared_ptr<AbilityRecord> bottomAbility = curMission->GetBottomAbilityRecord();
        EXPECT_TRUE(bottomAbility != nullptr);
        EXPECT_EQ("com.ix.test1", bottomAbility->GetAbilityInfo().applicationName);
        EXPECT_NE(BACKGROUND, bottomAbility->GetAbilityState());
    }
    EXPECT_TRUE(topAbility != nullptr);
    Want resultWant;
    std::string key("key");
    int resultValue = 4;
    resultWant.SetParam(key, resultValue);
    result = abilityMs_->TerminateAbility(topAbility->GetToken(), -1, &resultWant);
    SLEEP(2);
    EXPECT_EQ(0, result);

    auto missionstack = stackmgr->GetCurrentMissionStack();
    EXPECT_EQ(1, missionstack->GetMissionStackId());
    topAbility = missionstack->GetTopAbilityRecord();
    EXPECT_TRUE(topAbility != nullptr);
    EXPECT_EQ("com.ix.test1", topAbility->GetAbilityInfo().applicationName);
    AbilityRequest request;
    abilityMs_->GetStackManager()->GetTargetMissionStack(request)->RemoveAll();
    GTEST_LOG_(INFO) << "Teminate_Ability_With_Applications_002 end";
}
}  // namespace AAFwk
}  // namespace OHOS