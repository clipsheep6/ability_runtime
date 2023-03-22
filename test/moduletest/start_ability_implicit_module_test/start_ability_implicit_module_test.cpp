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
#include "ability_event_handler.h"
#include "ability_manager_service.h"
#undef private
#undef protected

#include "ability_manager_errors.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "mock_ability_connect_callback.h"
#include "mock_bundle_mgr.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "ui_service_mgr_client_mock.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace {
const std::string EVENT_MULT_APP_CHOOSE = "EVENT_MULT_APP_CHOOSE";
const std::string EVENT_MULT_APP_CLOSE = "EVENT_MULT_APP_CLOSE";
const std::string EVENT_TIPS_APP = "EVENT_TIPS_APP";
const std::string ACTION_VIEW = "ohos.want.action.viewData";
const std::string WANT_TYPE = "image/png";
const int32_t MOCK_MAIN_USER_ID = 100;
}
namespace OHOS {
namespace AAFwk {
static void WaitUntilTaskFinished()
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    auto handler = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    std::atomic<bool> taskCalled(false);
    auto f = [&taskCalled]() { taskCalled.store(true); };
    if (handler->PostTask(f)) {
        while (!taskCalled.load()) {
            ++count;
            if (count >= maxRetryCount) {
                break;
            }
            usleep(sleepTime);
        }
    }
}

class StartAbilityImplicitModuleTest : public testing::Test {
public:
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void OnStartAms() const;
    void OnStopAms() const;
    static constexpr int TEST_WAIT_TIME = 100000;

public:
    AbilityRequest abilityRequest_;
    std::shared_ptr<AbilityRecord> abilityRecord_{ nullptr };
    std::shared_ptr<AbilityManagerService> abilityMs_ = DelayedSingleton<AbilityManagerService>::GetInstance();
};

void StartAbilityImplicitModuleTest::OnStartAms() const
{
    if (abilityMs_) {
        if (abilityMs_->state_ == ServiceRunningState::STATE_RUNNING) {
            return;
        }

        abilityMs_->state_ = ServiceRunningState::STATE_RUNNING;
        abilityMs_->eventLoop_ = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
        EXPECT_TRUE(abilityMs_->eventLoop_);
        abilityMs_->handler_ = std::make_shared<AbilityEventHandler>(abilityMs_->eventLoop_, abilityMs_);
        // init user controller.
        abilityMs_->userController_ = std::make_shared<UserController>();
        EXPECT_TRUE(abilityMs_->userController_);
        abilityMs_->userController_->Init();
        int userId = MOCK_MAIN_USER_ID;
        abilityMs_->InitConnectManager(userId, true);
        abilityMs_->InitDataAbilityManager(userId, true);
        abilityMs_->InitPendWantManager(userId, true);

        abilityMs_->dataAbilityManager_ = std::make_shared<DataAbilityManager>();
        abilityMs_->dataAbilityManagers_.emplace(0, abilityMs_->dataAbilityManager_);
        EXPECT_TRUE(abilityMs_->dataAbilityManager_);
        AmsConfigurationParameter::GetInstance().Parse();
        abilityMs_->pendingWantManager_ = std::make_shared<PendingWantManager>();
        EXPECT_TRUE(abilityMs_->pendingWantManager_);
        abilityMs_->iBundleManager_ = new BundleMgrService();
        EXPECT_TRUE(abilityMs_->iBundleManager_);

        abilityMs_->implicitStartProcessor_ = std::make_shared<ImplicitStartProcessor>();
        EXPECT_TRUE(abilityMs_->implicitStartProcessor_);
        abilityMs_->implicitStartProcessor_->iBundleManager_ = new BundleMgrService();
        EXPECT_TRUE(abilityMs_->implicitStartProcessor_->iBundleManager_);

        DelayedSingleton<SystemDialogScheduler>::GetInstance()->SetDeviceType("phone");
        abilityMs_->InitMissionListManager(userId, true);
        abilityMs_->SwitchManagers(0, false);
        abilityMs_->userController_->SetCurrentUserId(MOCK_MAIN_USER_ID);
        abilityMs_->eventLoop_->Run();
        return;
    }

    GTEST_LOG_(INFO) << "OnStart fail";
}

void StartAbilityImplicitModuleTest::OnStopAms() const
{
    abilityMs_->currentMissionListManager_->launcherList_->missions_.clear();
    abilityMs_->currentMissionListManager_->defaultStandardList_->missions_.clear();
    abilityMs_->currentMissionListManager_->defaultSingleList_->missions_.clear();
    abilityMs_->currentMissionListManager_->currentMissionLists_.clear();
    abilityMs_->OnStop();
}

void StartAbilityImplicitModuleTest::TearDownTestCase()
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
}

void StartAbilityImplicitModuleTest::SetUp()
{
    OnStartAms();
    WaitUntilTaskFinished();
}

void StartAbilityImplicitModuleTest::TearDown()
{
    OnStopAms();
}

/*
 * Feature: StartAbilityImplicitModuleTest
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: start ability by action
 * EnvConditions: NA
 * CaseDescription: start ability by action and type successful
 */
HWTEST_F(StartAbilityImplicitModuleTest, StartAbility_001, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);

    Want want;
    want.SetAction(ACTION_VIEW);
    want.SetType(WANT_TYPE);
    want.SetParam("numMock", 3);

    Ace::UIServiceMgrClient::GetInstance()->SetDialogCheckState(EVENT_MULT_APP_CHOOSE);
    abilityMs_->StartAbility(want, MOCK_MAIN_USER_ID);
    auto params = Ace::UIServiceMgrClient::GetInstance()->GetParams();
    auto isCallBack = Ace::UIServiceMgrClient::GetInstance()->IsCallBack();

    EXPECT_TRUE(!params.empty());
    EXPECT_TRUE(isCallBack);

    auto abilityRecord = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_TRUE(abilityRecord != nullptr);

    GTEST_LOG_(INFO) << "ability:" << abilityRecord->GetAbilityInfo().name;
    GTEST_LOG_(INFO) << "bundle:" << abilityRecord->GetAbilityInfo().bundleName;
}

/*
 * Feature: StartAbilityImplicitModuleTest
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: start ability by action
 * EnvConditions: NA
 * CaseDescription: start ability by action and type successful
 */
HWTEST_F(StartAbilityImplicitModuleTest, StartAbility_002, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);

    Want want;
    want.SetAction(ACTION_VIEW);
    want.SetType(WANT_TYPE);
    want.SetParam("numMock", 3);

    Ace::UIServiceMgrClient::GetInstance()->SetDialogCheckState(EVENT_MULT_APP_CLOSE);
    abilityMs_->StartAbility(want, MOCK_MAIN_USER_ID);
    auto params = Ace::UIServiceMgrClient::GetInstance()->GetParams();
    auto isCallBack = Ace::UIServiceMgrClient::GetInstance()->IsCallBack();

    EXPECT_TRUE(!params.empty());
    EXPECT_TRUE(isCallBack);

    auto abilityRecord = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_TRUE(abilityRecord == nullptr);
}

/*
 * Feature: StartAbilityImplicitModuleTest
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: start ability by action
 * EnvConditions: NA
 * CaseDescription: start ability by action and type successful
 */
HWTEST_F(StartAbilityImplicitModuleTest, StartAbility_003, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);

    Want want;
    want.SetAction(ACTION_VIEW);
    want.SetType(WANT_TYPE);
    want.SetParam("numMock", 1);

    Ace::UIServiceMgrClient::GetInstance()->SetDialogCheckState(EVENT_MULT_APP_CHOOSE);
    abilityMs_->StartAbility(want, MOCK_MAIN_USER_ID);
    auto params = Ace::UIServiceMgrClient::GetInstance()->GetParams();
    auto isCallBack = Ace::UIServiceMgrClient::GetInstance()->IsCallBack();

    EXPECT_TRUE(params.empty());
    EXPECT_TRUE(!isCallBack);

    auto abilityRecord = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: StartAbilityImplicitModuleTest
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: start ability by action
 * EnvConditions: NA
 * CaseDescription: start ability by action and type successful
 */
HWTEST_F(StartAbilityImplicitModuleTest, StartAbility_004, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);

    Want want;
    want.SetAction(ACTION_VIEW);
    want.SetType(WANT_TYPE);
    want.SetParam("numMock", 0);

    Ace::UIServiceMgrClient::GetInstance()->SetDialogCheckState(EVENT_TIPS_APP);
    abilityMs_->StartAbility(want, MOCK_MAIN_USER_ID);
    auto params = Ace::UIServiceMgrClient::GetInstance()->GetParams();
    auto isCallBack = Ace::UIServiceMgrClient::GetInstance()->IsCallBack();

    EXPECT_TRUE(!params.empty());
    EXPECT_TRUE(isCallBack);

    auto abilityRecord = abilityMs_->currentMissionListManager_->GetCurrentTopAbilityLocked();
    EXPECT_TRUE(abilityRecord == nullptr);
}

/*
 * Feature: StartAbilityImplicitModuleTest
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: start ability by action
 * EnvConditions: NA
 * CaseDescription: start ability by action and type successful
 */
HWTEST_F(StartAbilityImplicitModuleTest, StartAbility_005, TestSize.Level1)
{
    EXPECT_TRUE(abilityMs_ != nullptr);

    Want want;
    want.SetAction(ACTION_VIEW);
    want.SetParam("numMock", 3);

    Ace::UIServiceMgrClient::GetInstance()->SetDialogCheckState(EVENT_MULT_APP_CHOOSE);
    auto ret = abilityMs_->StartAbility(want, MOCK_MAIN_USER_ID);

    EXPECT_EQ(ret, ERR_OK);
}
}
}