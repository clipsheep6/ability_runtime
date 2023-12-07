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

#include <functional>
#include <gtest/gtest.h>
#define private public
#define protected public
#include "ability_loader.h"
#include "fa_ability_thread.h"
#undef private
#undef protected
#include "ability.h"
#include "ability_impl.h"
#include "ability_impl_factory.h"
#include "context_deal.h"
#include "hilog_wrapper.h"
#include "mock_ability_impl.h"
#include "mock_ability_lifecycle_callbacks.h"
#include "mock_ability_thread.h"
#include "mock_ability_token.h"
#include "mock_data_ability.h"
#include "mock_data_obs_mgr_stub.h"
#include "mock_page_ability.h"
#include "mock_service_ability.h"
#include "ohos_application.h"
#include "page_ability_impl.h"
#include "uri.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

REGISTER_AA(MockDataAbility)
REGISTER_AA(MockPageAbility)
REGISTER_AA(MockServiceAbility)
static const int32_t STARTID = 0;
static const int32_t ASSERT_NUM = -1;
static const std::string DEVICE_ID = "deviceId";
static const std::string TEST = "test";

class AbilityThreadTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void AbilityThreadTest::SetUpTestCase(void)
{}

void AbilityThreadTest::TearDownTestCase(void)
{}

void AbilityThreadTest::SetUp(void)
{}

void AbilityThreadTest::TearDown(void)
{}

/**
 * @tc.name: AaFwk_AbilityThread_DumpAbilityInfo_0100
 * @tc.desc: DumpAbilityInfo
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_DumpAbilityInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfo_0100 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
        std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
        std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
        abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);

        std::vector<std::string> params;

        std::vector<std::string> info;
        abilitythread->DumpAbilityInfo(params, info);

        EXPECT_EQ(info.size(), 0);

        GTEST_LOG_(INFO) << "info:";
        for (auto item : info) {
            GTEST_LOG_(INFO) << item;
        }
    }

    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfo_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleSaveAbilityState_0100
 * @tc.name: ScheduleSaveAbilityState
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleSaveAbilityState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleSaveAbilityState_0100 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);

            abilitythread->ScheduleSaveAbilityState();

            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleSaveAbilityState_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleSaveAbilityState_0200
 * @tc.name: ScheduleSaveAbilityState
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleSaveAbilityState_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleSaveAbilityState_0200 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        abilitythread->ScheduleSaveAbilityState();
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleSaveAbilityState_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleRestoreAbilityState_0100
 * @tc.name: ScheduleRestoreAbilityState
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleRestoreAbilityState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleRestoreAbilityState_0100 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            PacMap state;

            abilitythread->ScheduleRestoreAbilityState(state);

            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleRestoreAbilityState_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleRestoreAbilityState_0200
 * @tc.name: ScheduleRestoreAbilityState
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleRestoreAbilityState_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleRestoreAbilityState_0200 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        abilitythread->ScheduleSaveAbilityState();
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleRestoreAbilityState_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Attach_3_Param_0100
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Attach_3_Param_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Attach_3_Param_0100 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Attach_3_Param_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Attach_3_Param_0200
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Attach_3_Param_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Attach_3_Param_0200 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = nullptr;
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);

            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Attach_3_Param_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Attach_2_Param_0100
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Attach_2_Param_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Attach_2_Param_0100 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            abilitythread->Attach(application, abilityRecord, nullptr);

            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Attach_2_Param_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Attach_2_Param_0200
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Attach_2_Param_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Attach_2_Param_0200 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = nullptr;
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);

            abilitythread->Attach(application, abilityRecord, nullptr);

            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Attach_2_Param_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleAbilityTransaction_0100
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleAbilityTransaction_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleAbilityTransaction_0100 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);

            Want want;
            LifeCycleStateInfo lifeCycleStateInfo;
            abilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);

            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleAbilityTransaction_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleAbilityTransaction_0200
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleAbilityTransaction_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleAbilityTransaction_0200 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            abilitythread->Attach(application, abilityRecord, nullptr);

            Want want;
            LifeCycleStateInfo lifeCycleStateInfo;
            abilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleAbilityTransaction_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleConnectAbility_0100
 * @tc.name: ScheduleConnectAbility
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleConnectAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleConnectAbility_0100 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);

            Want want;
            abilitythread->ScheduleConnectAbility(want);

            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleConnectAbility_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleConnectAbility_0200
 * @tc.name: ScheduleConnectAbility
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleConnectAbility_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleConnectAbility_0200 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
        std::shared_ptr<AbilityLocalRecord> abilityRecord =
            std::make_shared<AbilityLocalRecord>(abilityInfo, token);
        std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
        abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);

        Want want;
        abilitythread->ScheduleConnectAbility(want);
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleConnectAbility_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleDisconnectAbility_0100
 * @tc.name: ScheduleDisconnectAbility
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleDisconnectAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleDisconnectAbility_0100 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);

            Want want;
            abilitythread->ScheduleDisconnectAbility(want);

            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleDisconnectAbility_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleDisconnectAbility_0200
 * @tc.name: ScheduleDisconnectAbility
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleDisconnectAbility_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleDisconnectAbility_0200 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
        std::shared_ptr<AbilityLocalRecord> abilityRecord =
            std::make_shared<AbilityLocalRecord>(abilityInfo, token);
        std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
        abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);

        Want want;
        abilitythread->ScheduleDisconnectAbility(want);
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleDisconnectAbility_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleCommandAbility_0100
 * @tc.name: ScheduleCommandAbility
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleCommandAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleCommandAbility_0100 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockServiceAbility";
        abilityInfo->type = AbilityType::SERVICE;
        abilityInfo->isNativeAbility = true;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);

            Want want;
            bool restart = true;
            int startId = 0;

            abilitythread->ScheduleCommandAbility(want, restart, startId);

            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleCommandAbility_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleCommandAbility_0200
 * @tc.name: ScheduleCommandAbility
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleCommandAbility_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleCommandAbility_0200 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
        std::shared_ptr<AbilityLocalRecord> abilityRecord =
            std::make_shared<AbilityLocalRecord>(abilityInfo, token);
        std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
        abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);

        Want want;
        bool restart = true;
        int startId = 0;
        abilitythread->ScheduleCommandAbility(want, restart, startId);
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleCommandAbility_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_SendResult_0100
 * @tc.name: SendResult
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_SendResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SendResult_0100 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            std::shared_ptr<AbilityImpl> abilityimpl = std::make_shared<AbilityImpl>();
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);

            int requestCode = 0;
            int resultCode = 0;
            Want want;
            abilitythread->SendResult(requestCode, resultCode, want);

            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SendResult_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_SendResult_0200
 * @tc.name: SendResult
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_SendResult_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SendResult_0200 start";

    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
        std::shared_ptr<AbilityLocalRecord> abilityRecord =
            std::make_shared<AbilityLocalRecord>(abilityInfo, token);
        std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
        abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);

        int requestCode = 0;
        int resultCode = 0;
        Want want;
        abilitythread->SendResult(requestCode, resultCode, want);
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SendResult_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_AbilityThreadMain_0100
 * @tc.name: AbilityThreadMain
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_AbilityThreadMain_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_AbilityThreadMain_0100 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);

            abilitythread->AbilityThreadMain(application, abilityRecord, mainRunner, nullptr);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_AbilityThreadMain_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_AbilityThreadMain_0200
 * @tc.name: AbilityThreadMain
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_AbilityThreadMain_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_AbilityThreadMain_0200 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = nullptr;
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);

            abilitythread->AbilityThreadMain(application, abilityRecord, mainRunner, nullptr);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_AbilityThreadMain_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_AbilityThreadMain_0300
 * @tc.name: AbilityThreadMain
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_AbilityThreadMain_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_AbilityThreadMain_0300 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);

            abilitythread->AbilityThreadMain(application, abilityRecord, nullptr);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_AbilityThreadMain_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_AbilityThreadMain_0400
 * @tc.name: AbilityThreadMain
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_AbilityThreadMain_0400, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_AbilityThreadMain_0400 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockPageAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = nullptr;
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);

            abilitythread->AbilityThreadMain(application, abilityRecord, nullptr);
        }
    }
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_AbilityThreadMain_0400 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_HandleAbilityTransaction_0100
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Test HandleAbilityTransaction function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_HandleAbilityTransaction_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleAbilityTransaction_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);

    abilitythread->HandleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleAbilityTransaction_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_HandleAbilityTransaction_0200
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Test HandleAbilityTransaction function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_HandleAbilityTransaction_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleAbilityTransaction_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);

    abilitythread->HandleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleAbilityTransaction_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_HandleConnectAbility_0100
 * @tc.name: HandleConnectAbility
 * @tc.desc: Test HandleConnectAbility function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_HandleConnectAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleConnectAbility_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Want want;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);

    abilitythread->HandleConnectAbility(want);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleConnectAbility_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_HandleDisconnectAbility_0100
 * @tc.name: HandleDisconnectAbility
 * @tc.desc: Test HandleDisconnectAbility function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_HandleDisconnectAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleDisconnectAbility_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Want want;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);

    abilitythread->HandleDisconnectAbility(want);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleDisconnectAbility_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_HandleRestoreAbilityState_0100
 * @tc.name: HandleRestoreAbilityState
 * @tc.desc: Test HandleRestoreAbilityState function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_HandleRestoreAbilityState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleRestoreAbilityState_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    PacMap state;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);

    abilitythread->HandleRestoreAbilityState(state);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleRestoreAbilityState_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_HandleRestoreAbilityState_0200
 * @tc.name: HandleRestoreAbilityState
 * @tc.desc: Test HandleRestoreAbilityState function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_HandleRestoreAbilityState_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleRestoreAbilityState_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    PacMap state;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);

    abilitythread->HandleRestoreAbilityState(state);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleRestoreAbilityState_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleUpdateConfiguration_0100
 * @tc.name: ScheduleUpdateConfiguration
 * @tc.desc: Test ScheduleUpdateConfiguration function when abilityHandler_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleUpdateConfiguration_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleUpdateConfiguration_0100 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Configuration config;
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    abilitythread->ScheduleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleUpdateConfiguration_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleUpdateConfiguration_0200
 * @tc.name: ScheduleUpdateConfiguration
 * @tc.desc: Test ScheduleUpdateConfiguration function when abilityHandler_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleUpdateConfiguration_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleUpdateConfiguration_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Configuration config;
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    abilitythread->ScheduleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleUpdateConfiguration_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_HandleUpdateConfiguration_0100
 * @tc.name: HandleUpdateConfiguration
 * @tc.desc: Test HandleUpdateConfiguration function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_HandleUpdateConfiguration_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleUpdateConfiguration_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Configuration config;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    abilitythread->HandleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleUpdateConfiguration_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_HandleUpdateConfiguration_0200
 * @tc.name: HandleUpdateConfiguration
 * @tc.desc: Test HandleUpdateConfiguration function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_HandleUpdateConfiguration_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleUpdateConfiguration_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Configuration config;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->HandleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleUpdateConfiguration_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleAbilityTransaction_0300
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Test ScheduleAbilityTransaction function when token_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleAbilityTransaction_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleAbilityTransaction_0300 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    abilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilitythread->token_, nullptr);

    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    abilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleAbilityTransaction_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleAbilityTransaction_0400
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Test ScheduleAbilityTransaction function when abilityHandler_ and token_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleAbilityTransaction_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleAbilityTransaction_0400 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    abilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilitythread->token_, nullptr);
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);

    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    abilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleAbilityTransaction_0400 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_SendResult_0300
 * @tc.name: SendResult
 * @tc.desc: Test SendResult function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_SendResult_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SendResult_0300 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto application = std::make_shared<OHOSApplication>();
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
    abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
    
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);

    int requestCode = STARTID;
    int resultCode = STARTID;
    Want want;
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    abilitythread->SendResult(requestCode, resultCode, want);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SendResult_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_SendResult_0400
 * @tc.name: SendResult
 * @tc.desc: Test SendResult function when abilityHandler_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_SendResult_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SendResult_0400 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);

    int requestCode = STARTID;
    int resultCode = STARTID;
    Want want;
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    abilitythread->SendResult(requestCode, resultCode, want);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SendResult_0400 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_SendResult_0500
 * @tc.name: SendResult
 * @tc.desc: Test SendResult function when abilityHandler_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_SendResult_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SendResult_0500 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);

    int requestCode = STARTID;
    int resultCode = STARTID;
    Want want;
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    abilitythread->SendResult(requestCode, resultCode, want);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_SendResult_0500 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_GetFileTypes_0100
 * @tc.name: GetFileTypes
 * @tc.desc: Test GetFileTypes function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_GetFileTypes_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_GetFileTypes_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    std::string mimeTypeFilter = "";
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->GetFileTypes(uri, mimeTypeFilter);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_GetFileTypes_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_GetFileTypes_0200
 * @tc.name: GetFileTypes
 * @tc.desc: Test GetFileTypes function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_GetFileTypes_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_GetFileTypes_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    std::string mimeTypeFilter = "";
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    abilitythread->GetFileTypes(uri, mimeTypeFilter);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_GetFileTypes_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_OpenFile_0100
 * @tc.name: OpenFile
 * @tc.desc: Test OpenFile function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_OpenFile_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_OpenFile_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    std::string mode = "";
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->OpenFile(uri, mode);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_OpenFile_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_OpenFile_0200
 * @tc.name: OpenFile
 * @tc.desc: Test OpenFile function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_OpenFile_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_OpenFile_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    std::string mode = "";
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    EXPECT_EQ(abilitythread->OpenFile(uri, mode), ASSERT_NUM);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_OpenFile_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_OpenRawFile_0100
 * @tc.name: OpenRawFile
 * @tc.desc: Test OpenRawFile function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_OpenRawFile_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_OpenRawFile_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    std::string mode = "";
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->OpenRawFile(uri, mode);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_OpenRawFile_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_OpenRawFile_0200
 * @tc.name: OpenRawFile
 * @tc.desc: Test OpenRawFile function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_OpenRawFile_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_OpenRawFile_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    std::string mode = "";
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    EXPECT_EQ(abilitythread->OpenRawFile(uri, mode), ASSERT_NUM);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_OpenRawFile_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Insert_0100
 * @tc.name: Insert
 * @tc.desc: Test Insert function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Insert_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Insert_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    NativeRdb::ValuesBucket value;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->Insert(uri, value);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Insert_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Insert_0200
 * @tc.name: Insert
 * @tc.desc: Test Insert function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Insert_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Insert_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    NativeRdb::ValuesBucket value;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    EXPECT_EQ(abilitythread->Insert(uri, value), ASSERT_NUM);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Insert_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Call_0100
 * @tc.name: Call
 * @tc.desc: Test Call function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Call_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Call_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    std::string method = "";
    std::string arg = "";
    AppExecFwk::PacMap pacMap;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->Call(uri, method, arg, pacMap);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Call_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Call_0200
 * @tc.name: Call
 * @tc.desc: Test Call function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Call_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Call_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    std::string method = "";
    std::string arg = "";
    AppExecFwk::PacMap pacMap;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    abilitythread->Call(uri, method, arg, pacMap);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Call_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Update_0100
 * @tc.name: Update
 * @tc.desc: Test Update function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Update_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Update_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    NativeRdb::ValuesBucket value;
    NativeRdb::DataAbilityPredicates predicates;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->Update(uri, value, predicates);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Update_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Update_0200
 * @tc.name: Update
 * @tc.desc: Test Update function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Update_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Update_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    NativeRdb::ValuesBucket value;
    NativeRdb::DataAbilityPredicates predicates;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    EXPECT_EQ(abilitythread->Update(uri, value, predicates), ASSERT_NUM);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Update_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Delete_0100
 * @tc.name: Delete
 * @tc.desc: Test Delete function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Delete_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Delete_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    NativeRdb::DataAbilityPredicates predicates;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->Delete(uri, predicates);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Delete_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Delete_0200
 * @tc.name: Delete
 * @tc.desc: Test Delete function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Delete_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Delete_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    NativeRdb::ValuesBucket value;
    NativeRdb::DataAbilityPredicates predicates;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    EXPECT_EQ(abilitythread->Delete(uri, predicates), ASSERT_NUM);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Delete_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Query_0100
 * @tc.name: Query
 * @tc.desc: Test Query function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Query_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Query_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->Query(uri, columns, predicates);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Query_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Query_0200
 * @tc.name: Query
 * @tc.desc: Test Query function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Query_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Query_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    abilitythread->Query(uri, columns, predicates);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Query_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_GetType_0100
 * @tc.name: GetType
 * @tc.desc: Test GetType function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_GetType_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_GetType_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->GetType(uri);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_GetType_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_GetType_0200
 * @tc.name: GetType
 * @tc.desc: Test GetType function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_GetType_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_GetType_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    abilitythread->GetType(uri);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_GetType_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Reload_0100
 * @tc.name: Reload
 * @tc.desc: Test Reload function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Reload_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Reload_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    PacMap extras;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->Reload(uri, extras);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Reload_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_Reload_0200
 * @tc.name: Reload
 * @tc.desc: Test Reload function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_Reload_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Reload_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    PacMap extras;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    EXPECT_FALSE(abilitythread->Reload(uri, extras));
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_Reload_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_BatchInsert_0100
 * @tc.name: BatchInsert
 * @tc.desc: Test BatchInsert function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_BatchInsert_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_BatchInsert_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    std::vector<NativeRdb::ValuesBucket> values;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->BatchInsert(uri, values);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_BatchInsert_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_BatchInsert_0200
 * @tc.name: BatchInsert
 * @tc.desc: Test BatchInsert function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_BatchInsert_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_BatchInsert_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    std::vector<NativeRdb::ValuesBucket> values;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    EXPECT_EQ(abilitythread->BatchInsert(uri, values), ASSERT_NUM);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_BatchInsert_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ContinueAbility_0100
 * @tc.name: ContinueAbility
 * @tc.desc: Test ContinueAbility function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ContinueAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ContinueAbility_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::string deviceId = DEVICE_ID;
    uint32_t versionCode = STARTID;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->ContinueAbility(deviceId, versionCode);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ContinueAbility_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ContinueAbility_0200
 * @tc.name: ContinueAbility
 * @tc.desc: Test ContinueAbility function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ContinueAbility_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ContinueAbility_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::string deviceId = DEVICE_ID;
    uint32_t versionCode = STARTID;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    abilitythread->ContinueAbility(deviceId, versionCode);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ContinueAbility_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_NotifyContinuationResult_0100
 * @tc.name: NotifyContinuationResult
 * @tc.desc: Test NotifyContinuationResult function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_NotifyContinuationResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_NotifyContinuationResult_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    int32_t result = STARTID;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->NotifyContinuationResult(result);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_NotifyContinuationResult_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_NotifyContinuationResult_0200
 * @tc.name: NotifyContinuationResult
 * @tc.desc: Test NotifyContinuationResult function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_NotifyContinuationResult_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_NotifyContinuationResult_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    int32_t result = STARTID;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    abilitythread->NotifyContinuationResult(result);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_NotifyContinuationResult_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_NotifyMemoryLevel_0100
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Test NotifyMemoryLevel function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_NotifyMemoryLevel_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_NotifyMemoryLevel_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    int32_t result = STARTID;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    abilitythread->NotifyMemoryLevel(result);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_NotifyMemoryLevel_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_NotifyMemoryLevel_0200
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Test NotifyMemoryLevel function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_NotifyMemoryLevel_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_NotifyMemoryLevel_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    int32_t result = STARTID;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->NotifyMemoryLevel(result);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_NotifyMemoryLevel_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_InitUIAbilityFlag_0100
 * @tc.name: InitUIAbilityFlag
 * @tc.desc: Test InitUIAbilityFlag function when parameter is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_InitUIAbilityFlag_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_InitUIAbilityFlag_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->InitUIAbilityFlag(nullptr);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_InitUIAbilityFlag_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_NormalizeUri_0100
 * @tc.name: NormalizeUri
 * @tc.desc: Test NormalizeUri function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_NormalizeUri_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_NormalizeUri_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    EXPECT_EQ(abilitythread->NormalizeUri(uri), uri);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_NormalizeUri_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_NormalizeUri_0200
 * @tc.name: NormalizeUri
 * @tc.desc: Test NormalizeUri function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_NormalizeUri_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_NormalizeUri_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    Uri uriAssert("");
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    EXPECT_EQ(abilitythread->NormalizeUri(uri), uriAssert);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_NormalizeUri_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_DenormalizeUri_0100
 * @tc.name: DenormalizeUri
 * @tc.desc: Test DenormalizeUri function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_DenormalizeUri_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DenormalizeUri_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    EXPECT_EQ(abilitythread->DenormalizeUri(uri), uri);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DenormalizeUri_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_DenormalizeUri_0200
 * @tc.name: DenormalizeUri
 * @tc.desc: Test DenormalizeUri function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_DenormalizeUri_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DenormalizeUri_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    Uri uriAssert("");
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    EXPECT_EQ(abilitythread->DenormalizeUri(uri), uriAssert);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DenormalizeUri_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_HandleNotifyChange_0100
 * @tc.name: HandleNotifyChange
 * @tc.desc: Test HandleNotifyChange function when uri is "test"
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_HandleNotifyChange_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleNotifyChange_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    EXPECT_TRUE(abilitythread->HandleNotifyChange(uri));
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleNotifyChange_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_HandleRegisterObserver_0100
 * @tc.name: HandleRegisterObserver
 * @tc.desc: Test HandleRegisterObserver function
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_HandleRegisterObserver_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleRegisterObserver_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    sptr<AAFwk::IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());
    EXPECT_TRUE(abilitythread->HandleRegisterObserver(uri, dataObserver));
    delete dataObserver;
    dataObserver = nullptr;
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleRegisterObserver_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_HandleUnregisterObserver_0100
 * @tc.name: HandleUnregisterObserver
 * @tc.desc: Test HandleUnregisterObserver function
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_HandleUnregisterObserver_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleUnregisterObserver_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    sptr<AAFwk::IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());
    EXPECT_FALSE(abilitythread->HandleUnregisterObserver(uri, dataObserver));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_HandleUnregisterObserver_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleRegisterObserver_0100
 * @tc.name: ScheduleRegisterObserver
 * @tc.desc: Test ScheduleRegisterObserver function when abilityHandler_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleRegisterObserver_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleRegisterObserver_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    sptr<AAFwk::IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());
    EXPECT_FALSE(abilitythread->ScheduleRegisterObserver(uri, dataObserver));
    delete dataObserver;
    dataObserver = nullptr;
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleRegisterObserver_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleRegisterObserver_0200
 * @tc.name: ScheduleRegisterObserver
 * @tc.desc: Test ScheduleRegisterObserver function when abilityHandler_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleRegisterObserver_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleRegisterObserver_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    sptr<AAFwk::IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    EXPECT_FALSE(abilitythread->ScheduleRegisterObserver(uri, dataObserver));
    delete dataObserver;
    dataObserver = nullptr;
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleRegisterObserver_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleUnregisterObserver_0100
 * @tc.name: ScheduleUnregisterObserver
 * @tc.desc: Test ScheduleUnregisterObserver function when abilityHandler_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleUnregisterObserver_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleUnregisterObserver_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    sptr<AAFwk::IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());
    EXPECT_FALSE(abilitythread->ScheduleUnregisterObserver(uri, dataObserver));
    delete dataObserver;
    dataObserver = nullptr;
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleUnregisterObserver_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleUnregisterObserver_0200
 * @tc.name: ScheduleUnregisterObserver
 * @tc.desc: Test ScheduleUnregisterObserver function when abilityHandler_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleUnregisterObserver_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleUnregisterObserver_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    sptr<AAFwk::IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    EXPECT_FALSE(abilitythread->ScheduleUnregisterObserver(uri, dataObserver));
    delete dataObserver;
    dataObserver = nullptr;
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleUnregisterObserver_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleNotifyChange_0100
 * @tc.name: ScheduleNotifyChange
 * @tc.desc: Test ScheduleNotifyChange function when abilityHandler_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleNotifyChange_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleNotifyChange_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    EXPECT_FALSE(abilitythread->ScheduleNotifyChange(uri));
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleNotifyChange_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ScheduleNotifyChange_0200
 * @tc.name: ScheduleNotifyChange
 * @tc.desc: Test ScheduleNotifyChange function when abilityHandler_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ScheduleNotifyChange_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleNotifyChange_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Uri uri(TEST);
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    EXPECT_FALSE(abilitythread->ScheduleNotifyChange(uri));
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ScheduleNotifyChange_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ExecuteBatch_0100
 * @tc.name: ExecuteBatch
 * @tc.desc: Test ExecuteBatch function when abilityImpl_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ExecuteBatch_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ExecuteBatch_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->ExecuteBatch(operations);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ExecuteBatch_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_ExecuteBatch_0200
 * @tc.name: ExecuteBatch
 * @tc.desc: Test ExecuteBatch function when abilityImpl_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_ExecuteBatch_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ExecuteBatch_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    abilitythread->ExecuteBatch(operations);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_ExecuteBatch_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_BuildAbilityContext_0100
 * @tc.name: BuildAbilityContext
 * @tc.desc: Test BuildAbilityContext function when Parameters is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_BuildAbilityContext_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_BuildAbilityContext_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    EXPECT_NE(abilityInfo, nullptr);
    EXPECT_NE(application, nullptr);
    EXPECT_NE(token, nullptr);
    abilitythread->BuildAbilityContext(abilityInfo, application, token, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_BuildAbilityContext_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_DumpAbilityInfo_0200
 * @tc.name: DumpAbilityInfo
 * @tc.desc: Test DumpAbilityInfo function when token_ and abilityHandler_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_DumpAbilityInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfo_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> params;
    std::vector<std::string> info;

    abilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilitythread->token_, nullptr);
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    abilitythread->DumpAbilityInfo(params, info);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfo_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_DumpAbilityInfo_0300
 * @tc.name: DumpAbilityInfo
 * @tc.desc: Test DumpAbilityInfo function when abilityHandler_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_DumpAbilityInfo_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfo_0300 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> params;
    std::vector<std::string> info;

    abilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilitythread->token_, nullptr);
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    abilitythread->DumpAbilityInfo(params, info);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfo_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_DumpAbilityInfoInner_0100
 * @tc.name: DumpAbilityInfoInner
 * @tc.desc: Test DumpAbilityInfoInner function when currentAbility_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_DumpAbilityInfoInner_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfoInner_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> params;
    std::vector<std::string> info;

    abilitythread->currentAbility_ = std::make_shared<Ability>();
    EXPECT_NE(abilitythread->currentAbility_, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->DumpAbilityInfoInner(params, info);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfoInner_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_DumpAbilityInfoInner_0200
 * @tc.name: DumpAbilityInfoInner
 * @tc.desc: Test DumpAbilityInfoInner function when currentAbility_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_DumpAbilityInfoInner_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfoInner_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> params;
    std::vector<std::string> info;

    EXPECT_EQ(abilitythread->currentAbility_, nullptr);
    abilitythread->DumpAbilityInfoInner(params, info);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpAbilityInfoInner_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_DumpOtherInfo_0100
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when abilityHandler_ and currentAbility_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_DumpOtherInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpOtherInfo_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    abilityInfo->type = AbilityType::PAGE;
    auto setRunner = EventRunner::Create(abilityInfo->name);
    abilitythread->abilityHandler_->SetEventRunner(setRunner);
    auto getRunner = abilitythread->abilityHandler_->GetEventRunner();
    EXPECT_NE(getRunner, nullptr);

    std::vector<std::string> info;
    abilitythread->currentAbility_ = std::make_shared<Ability>();
    EXPECT_NE(abilitythread->currentAbility_, nullptr);
    abilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpOtherInfo_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_DumpOtherInfo_0200
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when abilityHandler_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_DumpOtherInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpOtherInfo_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> info;
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    abilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpOtherInfo_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_DumpOtherInfo_0300
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when currentAbility_ is nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_DumpOtherInfo_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpOtherInfo_0300 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> info;
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    EXPECT_EQ(abilitythread->currentAbility_, nullptr);
    abilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_DumpOtherInfo_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_CallRequest_0100
 * @tc.name: CallRequest
 * @tc.desc: Test CallRequest function when abilityHandler_ and currentAbility_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_CallRequest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CallRequest_0100 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> info;
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    abilitythread->currentAbility_ = std::make_shared<Ability>();
    EXPECT_NE(abilitythread->currentAbility_, nullptr);
    abilitythread->CallRequest();
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CallRequest_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityThread_CallRequest_0200
 * @tc.name: CallRequest
 * @tc.desc: Test CallRequest function when abilityHandler_ and currentAbility_ is not nullptr
 */
HWTEST_F(AbilityThreadTest, AaFwk_AbilityThread_CallRequest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CallRequest_0200 start";
    AbilityRuntime::FAAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> info;
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    EXPECT_EQ(abilitythread->currentAbility_, nullptr);
    abilitythread->CallRequest();
    GTEST_LOG_(INFO) << "AaFwk_AbilityThread_CallRequest_0200 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS