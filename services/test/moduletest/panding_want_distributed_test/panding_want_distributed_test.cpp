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
#include "sa_mgr_client.h"
#include "pending_want_manager.h"
#include "app_scheduler.h"
#include "ohos/aafwk/content/want.h"
#include "ability_manager_service.h"
#include "want_agent_helper.h"
#include "ability_manager_client.h"
#undef private
#undef protected
#include "system_ability_definition.h"
#include "pending_want.h"
#include "mock_bundle_mgr.h"
#include "mock_distributed_sched_service.h"

using namespace OHOS::AbilityRuntime::WantAgent;
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
namespace {
    const int32_t USER_ID_U100 = 100;
}  // namespace

class PandingWantDistributedTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AAFwk::Want> GetWant(std::string deviceId, std::string abilityName, std::string bundleName);
    
    WantAgentInfo MakeWantAgentInfo(WantAgentConstant::OperationType &type, int requestCode,
        std::vector<WantAgentConstant::Flags> &flags, std::vector<std::shared_ptr<AAFwk::Want>> wants);

    WantSenderInfo MakeWantSenderInfo(WantAgentConstant::OperationType &type, int requestCode,
        uint32_t &flags, const std::shared_ptr<AAFwk::Want> &want, std::string bundleName);

public:
    std::shared_ptr<AbilityManagerService> abilityMs_;
    static sptr<OHOS::DistributedSchedule::MockDistributedSchedService> distributeMS_;
};

sptr<OHOS::DistributedSchedule::MockDistributedSchedService> PandingWantDistributedTest::distributeMS_ = nullptr;

static void WaitUntilTaskFinished()
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
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

void PandingWantDistributedTest::SetUpTestCase(void)
{
    DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new (std::nothrow) BundleMgrService());

    distributeMS_ = new OHOS::DistributedSchedule::MockDistributedSchedService();
    DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        DISTRIBUTED_SCHED_SA_ID, distributeMS_);
}

void PandingWantDistributedTest::TearDownTestCase(void)
{
    DelayedSingleton<AbilityManagerService>::DestroyInstance();
}

void PandingWantDistributedTest::SetUp()
{
    abilityMs_ = DelayedSingleton<AbilityManagerService>::GetInstance();
    abilityMs_->OnStart();
    WaitUntilTaskFinished();

    abilityMs_->StartUser(USER_ID_U100);
    auto topAbility = abilityMs_->GetListManagerByUserId(USER_ID_U100)->GetCurrentTopAbilityLocked();
    if (topAbility) {
        topAbility->SetAbilityState(AAFwk::AbilityState::FOREGROUND_NEW);
    }
}

void PandingWantDistributedTest::TearDown()
{
    abilityMs_->OnStop();
    DelayedSingleton<AbilityManagerService>::DestroyInstance();
}

std::shared_ptr<AAFwk::Want> PandingWantDistributedTest::GetWant(
    std::string deviceId, std::string abilityName, std::string bundleName)
{
    if (abilityName == "") {
        abilityName = "hiMusic";
    }
    if (bundleName == "") {
        bundleName = "com.ix.hiMusic";
    }

    ElementName element;
    element.SetDeviceID(deviceId);
    element.SetAbilityName(abilityName);
    element.SetBundleName(bundleName);
    Want want;
    want.SetElement(element);
    return std::make_shared<Want>(want);
}

WantAgentInfo PandingWantDistributedTest::MakeWantAgentInfo(WantAgentConstant::OperationType &type, int requestCode,
    std::vector<WantAgentConstant::Flags> &flags, std::vector<std::shared_ptr<AAFwk::Want>> wants)
{
    WantAgentInfo info;
    info.operationType_ = type;
    info.requestCode_ = requestCode;
    info.flags_ = flags;
    info.wants_ = wants;
    info.extraInfo_ = nullptr;
    return info;
}

WantSenderInfo PandingWantDistributedTest::MakeWantSenderInfo(WantAgentConstant::OperationType &type, int requestCode,
    uint32_t &flags, const std::shared_ptr<AAFwk::Want> &want, std::string bundleName)
{
    WantsInfo wantsInfo;
    wantsInfo.want = *want;
    wantsInfo.resolvedTypes = want != nullptr ? want->GetType() : "";

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = (int32_t)type;
    wantSenderInfo.allWants.push_back(wantsInfo);
    wantSenderInfo.bundleName = bundleName;
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = 0;
    wantSenderInfo.requestCode = requestCode;
    return wantSenderInfo;
}

/*
 * Feature: AaFwk
 * Function: PendingWantStartAbility
 * SubFunction: start a page ability
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription: get want sender / send want sender
 */
HWTEST_F(PandingWantDistributedTest, PendingWantStartAbility_001, TestSize.Level1)
{
    WantAgentConstant::OperationType type = WantAgentConstant::OperationType::START_ABILITY;
    int requsetCode = 10;
    uint32_t flags = (uint32_t)WantAgentConstant::Flags::ONE_TIME_FLAG;

    auto abilityWant = GetWant("123", "hiMusic", "com.ix.hiMusic");
    WantSenderInfo info = MakeWantSenderInfo(type, requsetCode, flags, abilityWant, "com.ix.hiMusic");
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IWantSender> target = abilityMs_->GetWantSender(info, callerToken);

    auto StartRemoteAbilityFunc =
        [](const OHOS::AAFwk::Want& want, int32_t callerUid, int32_t requestCode, uint32_t accessToken) -> int32_t {
        return ERR_OK;
    };

    EXPECT_CALL(*distributeMS_, StartRemoteAbility(_, _, _, _)).Times(1).WillOnce(Invoke(StartRemoteAbilityFunc));

    SenderInfo senderInfo;
    senderInfo.resolvedType = abilityWant->GetType();
    senderInfo.want = *abilityWant;
    senderInfo.requiredPermission = "";
    senderInfo.code = requsetCode;
    senderInfo.finishedReceiver = nullptr;

    int32_t result = abilityMs_->SendWantSender(target, senderInfo);
    EXPECT_EQ(result, ERR_OK);
    testing::Mock::AllowLeak(distributeMS_);
}
}  // namespace AAFwk
}  // namespace OHOS
