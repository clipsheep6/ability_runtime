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

#include <gtest/gtest.h>
#define private public
#define protected public
#include "ability_manager_service.h"
#include "ability_event_handler.h"
#undef private
#undef protected

#include "app_process_data.h"
#include "system_ability_definition.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#include "bundlemgr/mock_bundle_manager.h"
#include "sa_mgr_client.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {

// static void WaitUntilTaskFinished()
// {
//     const uint32_t maxRetryCount = 1000;
//     const uint32_t sleepTime = 1000;
//     uint32_t count = 0;
//     auto handler = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
//     std::atomic<bool> taskCalled(false);
//     auto f = [&taskCalled]() { taskCalled.store(true); };
//     if (handler->PostTask(f)) {
//         while (!taskCalled.load()) {
//             ++count;
//             if (count >= maxRetryCount) {
//                 break;
//             }
//             usleep(sleepTime);
//         }
//     }
// }

// static void WaitUntilTaskFinishedByTimer()
// {
//     const uint32_t maxRetryCount = 1000;
//     const uint32_t sleepTime = 1000;
//     uint32_t count = 0;
//     auto handler = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
//     std::atomic<bool> taskCalled(false);
//     auto f = [&taskCalled]() { taskCalled.store(true); };
//     int sleepingTime = 5000;
//     if (handler->PostTask(f, "AbilityManagerServiceTest", sleepingTime)) {
//         while (!taskCalled.load()) {
//             ++count;
//             if (count >= maxRetryCount) {
//                 break;
//             }
//             usleep(sleepTime);
//         }
//     }
// }

#define SLEEP(milli) std::this_thread::sleep_for(std::chrono::seconds(milli))

namespace {
// const std::string NAME_BUNDLE_MGR_SERVICE = "BundleMgrService";
// static int32_t g_windowToken = 0;
}  // namespace

class AbilityManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void OnStartAms();
    void OnStopAms();
    // static constexpr int TEST_WAIT_TIME = 100000;

public:
    std::shared_ptr<AbilityManagerService> abilityMs_ {nullptr};
};

void AbilityManagerServiceTest::OnStartAms()
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

        abilityMs_->configuration_ = std::make_shared<AppExecFwk::Configuration>();
        EXPECT_TRUE(abilityMs_->configuration_);
        //abilityMs_->GetGlobalConfiguration();

        int userId = abilityMs_->GetUserId();
        abilityMs_->SetStackManager(userId);
        abilityMs_->systemAppManager_ = std::make_shared<KernalSystemAppManager>(userId);
        EXPECT_TRUE(abilityMs_->systemAppManager_);

        abilityMs_->InitMissionListManager(userId);

        abilityMs_->kernalAbilityManager_ = std::make_shared<KernalAbilityManager>(userId);

        abilityMs_->eventLoop_->Run();
        return;
    }

    GTEST_LOG_(INFO) << "OnStart fail";
}

void AbilityManagerServiceTest::OnStopAms()
{
    abilityMs_->eventLoop_.reset();
    abilityMs_->handler_.reset();
    abilityMs_->state_ = ServiceRunningState::STATE_NOT_START;
}

void AbilityManagerServiceTest::SetUpTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void AbilityManagerServiceTest::TearDownTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void AbilityManagerServiceTest::SetUp()
{
    abilityMs_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    OnStartAms();
}

void AbilityManagerServiceTest::TearDown()
{
    OnStopAms();
    OHOS::DelayedSingleton<AbilityManagerService>::DestroyInstance();
}

/*
 * Feature: AbilityManagerService
 * Function: query info
 * SubFunction: NA
 * FunctionPoints:query start abilites infos
 * EnvConditions: NA
 * CaseDescription: start page ability
 */
HWTEST_F(AbilityManagerServiceTest, GetAbilityRunningInfos_001, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_TRUE(topAbility);

    std::vector<AbilityRunningInfo> infos;
    abilityMs_->GetAbilityRunningInfos(infos);

    EXPECT_TRUE(infos.size() == 1);
    EXPECT_TRUE(infos[0].ability.GetAbilityName() == element.GetAbilityName());
    EXPECT_TRUE(infos[0].abilityState == topAbility->GetAbilityState());
}
#if 0
/*
 * Feature: AbilityManagerService
 * Function: query info
 * SubFunction: NA
 * FunctionPoints:query start abilites infos
 * EnvConditions: NA
 * CaseDescription: start serviec ability
 */
HWTEST_F(AbilityManagerServiceTest, GetAbilityRunningInfos_002, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiService", "ServiceAbility");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_TRUE(topAbility);

    std::vector<AbilityRunningInfo> infos;
    abilityMs_->GetAbilityRunningInfos(infos);

    EXPECT_TRUE(infos.size() == 1);
    EXPECT_TRUE(infos[0].ElementName.GetAbilityName() == element.GetAbilityName());
    EXPECT_TRUE(infos[0].abilityState == topAbility.GetAbilityState());
}

/*
 * Feature: AbilityManagerService
 * Function: query info
 * SubFunction: NA
 * FunctionPoints:query start abilites infos
 * EnvConditions: NA
 * CaseDescription: start luncher ability
 */
HWTEST_F(AbilityManagerServiceTest, GetAbilityRunningInfos_003, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiworld", "Helloworld");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_TRUE(topAbility);

    std::vector<AbilityRunningInfo> infos;
    abilityMs_->GetAbilityRunningInfos(infos);

    EXPECT_TRUE(infos.size() == 1);
    EXPECT_TRUE(infos[0].ElementName.GetAbilityName() == topAbility.GetAbilityName());
    EXPECT_TRUE(infos[0].abilityState == topAbility.GetAbilityState());
}

/*
 * Feature: AbilityManagerService
 * Function: query info
 * SubFunction: NA
 * FunctionPoints:query start abilites infos
 * EnvConditions: NA
 * CaseDescription: start date ability
 */
HWTEST_F(AbilityManagerServiceTest, GetAbilityRunningInfos_004, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiData", "hiData");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_TRUE(topAbility);

    std::vector<AbilityRunningInfo> infos;
    abilityMs_->GetAbilityRunningInfos(infos);

    EXPECT_TRUE(infos.size() == 1);
    EXPECT_TRUE(infos[0].ElementName.GetAbilityName() == topAbility.GetAbilityName());
    EXPECT_TRUE(infos[0].abilityState == topAbility.GetAbilityState());
}

/*
 * Feature: AbilityManagerService
 * Function: query info
 * SubFunction: NA
 * FunctionPoints:query start abilites infos
 * EnvConditions: NA
 * CaseDescription: start EXTENSION ability
 */
HWTEST_F(AbilityManagerServiceTest, GetExtensionRunningInfos_001, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiExtension", "hiExtension");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_TRUE(topAbility);

    std::vector<AbilityRunningInfo> infos;
    abilityMs_->GetAbilityRunningInfos(infos);

    EXPECT_TRUE(infos.size() == 1);
    EXPECT_TRUE(infos[0].ElementName.GetAbilityName() == topAbility.GetAbilityName());
    EXPECT_TRUE(infos[0].abilityState == topAbility.GetAbilityState());
}


/*
 * Feature: AbilityManagerService
 * Function: query info
 * SubFunction: NA
 * FunctionPoints:query start abilites infos
 * EnvConditions: NA
 * CaseDescription: start mulit EXTENSION ability
 */
HWTEST_F(AbilityManagerServiceTest, GetExtensionRunningInfos_002, TestSize.Level1)
{
    Want want;
    ElementName element("device", "com.ix.hiExtension", "hiExtension");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    auto topAbility = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_TRUE(topAbility);
    topAbility->SetAbilityState(AbilityState::ACTIVE);

    ElementName element("device", "com.ix.hiExtension", "hiExtensionOther");
    want.SetElement(element);
    auto result = abilityMs_->StartAbility(want);
    EXPECT_EQ(OHOS::ERR_OK, result);

    std::vector<AbilityRunningInfo> infos;
    abilityMs_->GetAbilityRunningInfos(infos);

    EXPECT_TRUE(infos.size() == 2);
    EXPECT_TRUE(infos[0].ElementName.GetAbilityName() == topAbility.GetAbilityName());
    EXPECT_TRUE(infos[0].abilityState == topAbility.GetAbilityState());

    EXPECT_TRUE(infos[1].ElementName.GetAbilityName() == topAbility.GetAbilityName());
    EXPECT_TRUE(infos[1].abilityState == topAbility.GetAbilityState());
}
#endif
}  // namespace AAFwk
}  // namespace OHOS
