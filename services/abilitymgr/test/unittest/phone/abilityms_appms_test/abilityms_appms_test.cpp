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
#include "gmock/gmock.h"
#include "system_ability_definition.h"

#define private public
#define protected public
#include "ability_scheduler.h"
#include "app_scheduler.h"
#include "ability_manager_service.h"
#undef private
#undef protected

#include "app_process_data.h"
#include "mock_bundle_manager.h"
#include "mock_app_manager_client.h"
#include "sa_mgr_client.h"
#include "app_state_callback_host.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace testing::ext;
using testing::_;

namespace OHOS {
namespace AAFwk {

namespace {
const std::string NAME_BUNDLE_MGR_SERVICE = "BundleMgrService";
}

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

class AppStateCallbackS : public AppStateCallback {
public:
    AppStateCallbackS()
    {}
    ~AppStateCallbackS()
    {}
    MOCK_METHOD2(OnAbilityRequestDone, void(const sptr<IRemoteObject> &token, const int32_t state));
    MOCK_METHOD1(OnAppStateChanged, void(const AppInfo &info));
};

class AbilityMsAppmsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void OnStartabilityAms();
    std::shared_ptr<AbilityRecord> GetAbilityRecord() const;
    void ResetAbilityRecord();
    void StartAbility();

public:
    std::shared_ptr<AbilityRecord> abilityRecord_ {nullptr};
    std::shared_ptr<AbilityManagerService> abilityMs_ = DelayedSingleton<AbilityManagerService>::GetInstance();
};

void AbilityMsAppmsTest::OnStartabilityAms()
{
    if (abilityMs_) {
        if (abilityMs_->state_ == ServiceRunningState::STATE_RUNNING) {
            return;
        }

        abilityMs_->state_ = ServiceRunningState::STATE_RUNNING;

        abilityMs_->eventLoop_ = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
        EXPECT_TRUE(abilityMs_->eventLoop_);

        abilityMs_->handler_ = std::make_shared<AbilityEventHandler>(abilityMs_->eventLoop_, abilityMs_);
        abilityMs_->connectManager_ = std::make_shared<AbilityConnectManager>(0);
        abilityMs_->dataAbilityManager_ = std::make_shared<DataAbilityManager>();
        abilityMs_->dataAbilityManagers_.emplace(0, abilityMs_->dataAbilityManager_);
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

        abilityMs_->eventLoop_->Run();

        return;
    }

    GTEST_LOG_(INFO) << "OnStart fail";
}

void AbilityMsAppmsTest::SetUpTestCase(void)
{
    DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
    DelayedSingleton<AppScheduler>::GetInstance()->Init(std::make_shared<AppStateCallbackS>());
}

void AbilityMsAppmsTest::TearDownTestCase(void)
{
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
    DelayedSingleton<AppScheduler>::DestroyInstance();
}

void AbilityMsAppmsTest::SetUp(void)
{
    OnStartabilityAms();
    StartAbility();
    GTEST_LOG_(INFO) << "SetUp";
}

void AbilityMsAppmsTest::TearDown(void)
{
    abilityMs_->OnStop();
    GTEST_LOG_(INFO) << "TearDown";
}

std::shared_ptr<AbilityRecord> AbilityMsAppmsTest::GetAbilityRecord() const
{
    return abilityRecord_;
}

void AbilityMsAppmsTest::ResetAbilityRecord()
{
    abilityRecord_.reset();
    const Want want;
    AbilityInfo abilityInfo;
    abilityInfo.name = "HelloWorld";
    abilityInfo.applicationName = "HelloWorld";
    abilityInfo.package = "com.ix.hiworld";
    ApplicationInfo applicationInfo;
    applicationInfo.name = "HelloWorld";
    applicationInfo.bundleName = "HelloWorld";

    abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord_->Init();
}

void AbilityMsAppmsTest::StartAbility()
{
    Want want;
    AbilityInfo abilityInfo;
    ApplicationInfo applicationInfo;

    EXPECT_TRUE(abilityMs_->currentStackManager_);
    auto currentTopAbilityRecord = abilityMs_->currentStackManager_->GetCurrentTopAbility();
    if (currentTopAbilityRecord) {
        currentTopAbilityRecord->SetAbilityState(AbilityState::ACTIVE);
    }

    ElementName element("device", "com.ix.hiworld", "luncherAbility");
    want.SetElement(element);
    abilityMs_->StartAbility(want);
    auto topAbility = abilityMs_->GetStackManager()->GetCurrentTopAbility();
    if (topAbility) {
        topAbility->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVE);
    }
    abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
}

/*
 * Feature:  Interaction of abilityms and appms
 * Function: Interaction of abilityms and appms
 * SubFunction: NA
 * FunctionPoints: LoadAbility
 * EnvConditions:NA
 * CaseDescription: verify LoadAbility parameters. LoadAbility fail if token is nullptr.
 */
HWTEST_F(AbilityMsAppmsTest, AaFwk_AbilityMS_AppMS_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMsAppMsTest_AaFwk_AbilityMS_AppMS_001 start";
    auto result = GetAbilityRecord()->LoadAbility();
    EXPECT_EQ(ERR_INVALID_VALUE, result);
    GTEST_LOG_(INFO) << "AbilityMsAppMsTest_AaFwk_AbilityMS_AppMS_001 end";
}

/*
 * Feature: Interaction of abilityms and appms
 * Function: Interaction of abilityms and appms
 * SubFunction: NA
 * FunctionPoints: LoadAbility
 * EnvConditions:NA
 * CaseDescription: verify LoadAbility parameters. LoadAbility fail if abilityinfo or appinfo is empty.
 */
HWTEST_F(AbilityMsAppmsTest, AaFwk_AbilityMS_AppMS_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMsAppMsTest_AaFwk_AbilityMS_AppMS_002 start";
    GetAbilityRecord()->Init();
    auto result = GetAbilityRecord()->LoadAbility();
    EXPECT_EQ(ERR_INVALID_VALUE, result);
    GTEST_LOG_(INFO) << "AbilityMsAppMsTest_AaFwk_AbilityMS_AppMS_002 end";
}

/*
 * Feature: Interaction of abilityms and appms
 * Function: Interaction of abilityms and appms
 * SubFunction: NA
 * FunctionPoints: LoadAbility
 * EnvConditions:NA
 *  CaseDescription: 1. abilityinfo or appinfo is empty.
 *                   2. Load ability
 *                   3. the result of load ability is successfully,
 *                      the LoadAbility function of Appscheduler is called.
 *                   4. Ability state is still INITIAL, since called is asynchronous.
 */
HWTEST_F(AbilityMsAppmsTest, AaFwk_AbilityMS_AppMS_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMsAppMsTest_AaFwk_AbilityMS_AppMS_003 start";
    ResetAbilityRecord();
    int result = GetAbilityRecord()->LoadAbility();
    EXPECT_EQ(ERR_OK, result);
    auto state = GetAbilityRecord()->GetAbilityState();
    EXPECT_EQ(AAFwk::AbilityState::INITIAL, state);
    GTEST_LOG_(INFO) << "AbilityMsAppMsTest_AaFwk_AbilityMS_AppMS_003 end";
}

/*
 * Feature: Interaction of abilityms and appms
 * Function: Interaction of abilityms and appms
 * SubFunction: NA
 * FunctionPoints: MoveForeground
 * EnvConditions:NA
 * CaseDescription: 1. launcher ability is started.
 *                  2. the LoadAbility and MoveToForeground function of AppSchedule are called
 *                  3. the Active  function of AbilityRecord is called.
 *                  4. the state of launcher ability is ACTIVATING.
 */
HWTEST_F(AbilityMsAppmsTest, AaFwk_AbilityMS_AppMS_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMsAppMsTest_AaFwk_AbilityMS_AppMS_004 start";
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    sptr<AbilityScheduler> scheduler = new AbilityScheduler();
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    sptr<Token> token = nullptr;
    auto checkStateFun = [&scheduler, &token, &abilityRecord]() {
        auto stackManager = DelayedSingleton<AbilityManagerService>::GetInstance()->GetStackManager();
        EXPECT_TRUE(stackManager);
        abilityRecord = stackManager->GetCurrentTopAbility();
        EXPECT_TRUE(abilityRecord);
        token = abilityRecord->GetToken();
        EXPECT_TRUE(token);
        DelayedSingleton<AbilityManagerService>::GetInstance()->AttachAbilityThread(scheduler, token);
    };

    handler->PostTask(checkStateFun);
    WaitUntilTaskFinished();
    GTEST_LOG_(INFO) << "AbilityMsAppMsTest_AaFwk_AbilityMS_AppMS_004 end";
}

/*
 * Feature: Interaction of abilityms and appms
 * Function: Interaction of abilityms and appms
 * SubFunction: NA
 * FunctionPoints: MoveBackground
 * EnvConditions:NA
 * CaseDescription: 1. launcher ability is started.
 *                  2. the LoadAbility and MoveToForeground function of AppSchedule are called
 *                  3. the Active  function of AbilityRecord is called.
 *                  4. the state of launcher ability is ACTIVATING.
 *                  5. perform ability to background
 *                  6. the MoveBackground fun of appSchedule is called.
 */
HWTEST_F(AbilityMsAppmsTest, AaFwk_AbilityMS_AppMS_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityMsAppMsTest_AaFwk_AbilityMS_AppMS_005 start";
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();

    sptr<AbilityScheduler> scheduler = new AbilityScheduler();
    std::shared_ptr<AbilityRecord> sourceAbilityRecord = nullptr;
    std::shared_ptr<AbilityStackManager> stackManager = nullptr;
    sptr<Token> sourcetoken = nullptr;

    auto checkSourceActivtingState = [&stackManager, &sourceAbilityRecord, &scheduler, &sourcetoken]() {
        stackManager = DelayedSingleton<AbilityManagerService>::GetInstance()->GetStackManager();
        sourceAbilityRecord = stackManager->GetCurrentTopAbility();
        EXPECT_TRUE(sourceAbilityRecord);
        sourcetoken = sourceAbilityRecord->GetToken();
        EXPECT_TRUE(sourcetoken);
        DelayedSingleton<AbilityManagerService>::GetInstance()->AttachAbilityThread(scheduler, sourcetoken);
        auto sourceAbilityInfo = sourceAbilityRecord->GetAbilityInfo();
        EXPECT_EQ(sourceAbilityInfo.bundleName, "com.ix.hiworld");
    };
    handler->PostTask(checkSourceActivtingState);
    WaitUntilTaskFinished();
    GTEST_LOG_(INFO) << "AbilityMsAppMsTest_AaFwk_AbilityMS_AppMS_005 end";
}
}  // namespace AAFwk
}  // namespace OHOS