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
#include "ability_thread.h"
#include "ui_ability_thread.h"
#undef private
#undef protected
#include "ability_handler.h"
#include "mock_ability_token.h"
#include "ui_ability.h"
#include "ui_ability_impl.h"
#include "context_deal.h"
// #include "mock_ui_ability_impl.h"
#include "mock_ability_lifecycle_callbacks.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using Want = OHOS::AAFwk::Want;

static const int32_t STARTID = 0;
// static const int32_t ASSERT_NUM = -1;
static const std::string DEVICE_ID = "deviceId";
static const std::string TEST = "test";
const unsigned int ZEROTAG = 0;

class UIAbilityThreadTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void UIAbilityThreadTest::SetUpTestCase(void)
{}

void UIAbilityThreadTest::TearDownTestCase(void)
{}

void UIAbilityThreadTest::SetUp(void)
{}

void UIAbilityThreadTest::TearDown(void)
{}

/**
 * @tc.name: AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0100
 * @tc.desc: DumpAbilityInfo
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0100 start";

    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
        abilityInfo->type = AbilityType::PAGE; 
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);

            std::vector<std::string> params;

            std::vector<std::string> info;
            abilitythread->DumpAbilityInfo(params, info);

            EXPECT_EQ(info.size(), ZEROTAG);

            GTEST_LOG_(INFO) << "info:";
            for (auto item : info) {
                GTEST_LOG_(INFO) << item;
            }
        }
        
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0200
 * @tc.name: DumpAbilityInfo
 * @tc.desc: Test DumpAbilityInfo function when token_ and abilityHandler_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0200 start";
    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> params;
    std::vector<std::string> info;

    abilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilitythread->token_, nullptr);
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    abilitythread->DumpAbilityInfo(params, info);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0200 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0300
 * @tc.name: DumpAbilityInfo
 * @tc.desc: Test DumpAbilityInfo function when abilityHandler_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0300 start";
    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> params;
    std::vector<std::string> info;

    abilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilitythread->token_, nullptr);
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    abilitythread->DumpAbilityInfo(params, info);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0300 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0400
 * @tc.name: DumpAbilityInfo
 * @tc.desc: Test DumpAbilityInfo function when token_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0400 start";
    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> params;
    std::vector<std::string> info;

    abilitythread->token_ = nullptr;
    EXPECT_EQ(abilitythread->token_, nullptr);

    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    abilitythread->DumpAbilityInfo(params, info);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpAbilityInfo_0400 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_ScheduleSaveAbilityState_0100
 * @tc.name: ScheduleSaveAbilityState
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_ScheduleSaveAbilityState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleSaveAbilityState_0100 start";

    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
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
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleSaveAbilityState_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_ScheduleSaveAbilityState_0200
 * @tc.name: ScheduleSaveAbilityState
 * @tc.desc: Test ScheduleSaveAbilityState function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_ScheduleSaveAbilityState_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleSaveAbilityState_0200 start";

    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        abilitythread->ScheduleSaveAbilityState();
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleSaveAbilityState_0200 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_ScheduleRestoreAbilityState_0100
 * @tc.name: ScheduleRestoreAbilityState
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_ScheduleRestoreAbilityState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleRestoreAbilityState_0100 start";

    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
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
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleRestoreAbilityState_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_ScheduleRestoreAbilityState_0200
 * @tc.name: ScheduleRestoreAbilityState
 * @tc.desc: Test Attach_3_Param function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_ScheduleRestoreAbilityState_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleRestoreAbilityState_0200 start";

    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        abilitythread->ScheduleSaveAbilityState();
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleRestoreAbilityState_0200 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_Attach_3_Param_0100
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_Attach_3_Param_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_Attach_3_Param_0100 start";
    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
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
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_Attach_3_Param_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_Attach_3_Param_0200
 * @tc.name: Attach
 * @tc.desc: Test Attach_3_Param function when application is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_Attach_3_Param_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_Attach_3_Param_0200 start";
    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
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
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_Attach_3_Param_0200 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_Attach_2_Param_0100
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_Attach_2_Param_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_Attach_2_Param_0100 start";
    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
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
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_Attach_2_Param_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_Attach_2_Param_0200
 * @tc.name: Attach
 * @tc.desc: Test Attach_2_Param function when application is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_Attach_2_Param_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_Attach_2_Param_0200 start";
    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
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
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_Attach_2_Param_0200 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0100
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0100 start";

    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
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
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0200
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Validate when normally entering a string ???
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0200 start";
    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            abilitythread->Attach(application, abilityRecord, nullptr); // ??? why this function use function overloading???

            Want want;
            LifeCycleStateInfo lifeCycleStateInfo;
            abilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0200 end";
}

/*
 * @tc.number: AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0300
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Test ScheduleAbilityTransaction function when token_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0100 start";

    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        abilitythread->token_ = nullptr;
        EXPECT_EQ(abilitythread->token_, nullptr);

        abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
        EXPECT_NE(abilitythread->abilityHandler_, nullptr);

        Want want;
        LifeCycleStateInfo lifeCycleStateInfo;
        abilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);

    }
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0300 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0300
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Test ScheduleAbilityTransaction function when token_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0400 start";
    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    abilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilitythread->token_, nullptr);

    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    abilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0400 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0400
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Test ScheduleAbilityTransaction function when abilityHandler_ and token_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0500 start";
    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    abilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilitythread->token_, nullptr);
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);

    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    abilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleAbilityTransaction_0500 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_SendResult_0100
 * @tc.name: SendResult
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_SendResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_SendResult_0100 start";

    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            std::shared_ptr<AbilityRuntime::UIAbilityImpl> abilityimpl = std::make_shared<AbilityRuntime::UIAbilityImpl>();
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);

            int requestCode = 0;
            int resultCode = 0;
            Want want;
            abilitythread->SendResult(requestCode, resultCode, want);

            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_SendResult_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_SendResult_0200
 * @tc.name: SendResult
 * @tc.desc: Test SendResult function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_SendResult_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_SendResult_0200 start";

    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
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
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_SendResult_0200 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_SendResult_0300
 * @tc.name: SendResult
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_SendResult_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_SendResult_0300 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockUIAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto application = std::make_shared<OHOSApplication>();
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
    abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
    
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);

    int requestCode = STARTID;
    int resultCode = STARTID;
    Want want;
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    abilitythread->SendResult(requestCode, resultCode, want);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_SendResult_0300 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_HandleAbilityTransaction_0100
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Test HandleAbilityTransaction function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_HandleAbilityTransaction_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_HandleAbilityTransaction_0100 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);

    abilitythread->HandleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_HandleAbilityTransaction_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_HandleAbilityTransaction_0200
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Test HandleAbilityTransaction function when abilityImpl_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_HandleAbilityTransaction_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_HandleAbilityTransaction_0200 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);

    abilitythread->HandleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_HandleAbilityTransaction_0200 end";
}



/**
 * @tc.number: AbilityRuntime_UIAbilityThread_ScheduleUpdateConfiguration_0100
 * @tc.name: ScheduleUpdateConfiguration
 * @tc.desc: Test ScheduleUpdateConfiguration function when abilityHandler_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_ScheduleUpdateConfiguration_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleUpdateConfiguration_0100 start";
    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Configuration config;
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    abilitythread->ScheduleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleUpdateConfiguration_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_ScheduleUpdateConfiguration_0200
 * @tc.name: ScheduleUpdateConfiguration
 * @tc.desc: Test ScheduleUpdateConfiguration function when abilityHandler_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_ScheduleUpdateConfiguration_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleUpdateConfiguration_0200 start";
    AbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Configuration config;
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    abilitythread->ScheduleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ScheduleUpdateConfiguration_0200 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_HandleUpdateConfiguration_0100
 * @tc.name: HandleUpdateConfiguration
 * @tc.desc: Test HandleUpdateConfiguration function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_HandleUpdateConfiguration_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_HandleUpdateConfiguration_0100 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Configuration config;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    abilitythread->HandleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_HandleUpdateConfiguration_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_HandleUpdateConfiguration_0200
 * @tc.name: HandleUpdateConfiguration
 * @tc.desc: Test HandleUpdateConfiguration function when abilityImpl_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_HandleUpdateConfiguration_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_HandleUpdateConfiguration_0200 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    Configuration config;
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->HandleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_HandleUpdateConfiguration_0200 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_ContinueAbility_0100
 * @tc.name: ContinueAbility
 * @tc.desc: Test ContinueAbility function when abilityImpl_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_ContinueAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ContinueAbility_0100 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::string deviceId = DEVICE_ID;
    uint32_t versionCode = STARTID;
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->ContinueAbility(deviceId, versionCode);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ContinueAbility_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_ContinueAbility_0200
 * @tc.name: ContinueAbility
 * @tc.desc: Test ContinueAbility function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_ContinueAbility_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ContinueAbility_0200 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::string deviceId = DEVICE_ID;
    uint32_t versionCode = STARTID;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    abilitythread->ContinueAbility(deviceId, versionCode);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_ContinueAbility_0200 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_NotifyContinuationResult_0100
 * @tc.name: NotifyContinuationResult
 * @tc.desc: Test NotifyContinuationResult function when abilityImpl_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_NotifyContinuationResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_NotifyContinuationResult_0100 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    int32_t result = STARTID;
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->NotifyContinuationResult(result);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_NotifyContinuationResult_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_NotifyContinuationResult_0200
 * @tc.name: NotifyContinuationResult
 * @tc.desc: Test NotifyContinuationResult function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_NotifyContinuationResult_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_NotifyContinuationResult_0200 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    int32_t result = STARTID;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    abilitythread->NotifyContinuationResult(result);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_NotifyContinuationResult_0200 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_NotifyMemoryLevel_0100
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Test NotifyMemoryLevel function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_NotifyMemoryLevel_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_NotifyMemoryLevel_0100 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    int32_t result = STARTID;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    abilitythread->NotifyMemoryLevel(result);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_NotifyMemoryLevel_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_NotifyMemoryLevel_0200
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Test NotifyMemoryLevel function when abilityImpl_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_NotifyMemoryLevel_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_NotifyMemoryLevel_0200 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    int32_t result = STARTID;
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->NotifyMemoryLevel(result);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_NotifyMemoryLevel_0200 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_BuildAbilityContext_0100
 * @tc.name: BuildAbilityContext
 * @tc.desc: Test BuildAbilityContext function when Parameters is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_BuildAbilityContext_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_BuildAbilityContext_0100 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockUIAbility";
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());

    EXPECT_NE(abilityInfo, nullptr);
    EXPECT_NE(application, nullptr);
    EXPECT_NE(token, nullptr);
    abilitythread->BuildAbilityContext(abilityInfo, application, token, nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_BuildAbilityContext_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_DumpAbilityInfoInner_0100
 * @tc.name: DumpAbilityInfoInner
 * @tc.desc: Test DumpAbilityInfoInner function when currentAbility_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_DumpAbilityInfoInner_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpAbilityInfoInner_0100 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> params;
    std::vector<std::string> info;

    abilitythread->currentAbility_ = std::make_shared<AbilityRuntime::UIAbility>();
    EXPECT_NE(abilitythread->currentAbility_, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->DumpAbilityInfoInner(params, info);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpAbilityInfoInner_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_DumpAbilityInfoInner_0200
 * @tc.name: DumpAbilityInfoInner
 * @tc.desc: Test DumpAbilityInfoInner function when currentAbility_ is nulllptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_DumpAbilityInfoInner_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpAbilityInfoInner_0200 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> params;
    std::vector<std::string> info;

    EXPECT_EQ(abilitythread->currentAbility_, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->DumpAbilityInfoInner(params, info);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpAbilityInfoInner_0200 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_DumpOtherInfo_0100
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when abilityHandler_ and currentAbility_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_DumpOtherInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpOtherInfo_0100 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockUIAbility";
    abilityInfo->type = AbilityType::PAGE;
    auto setRunner = EventRunner::Create(abilityInfo->name);
    abilitythread->abilityHandler_->SetEventRunner(setRunner);
    auto getRunner = abilitythread->abilityHandler_->GetEventRunner();
    EXPECT_NE(getRunner, nullptr);

    std::vector<std::string> info;
    abilitythread->currentAbility_ = std::make_shared<AbilityRuntime::UIAbility>();
    EXPECT_NE(abilitythread->currentAbility_, nullptr);
    abilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpOtherInfo_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_DumpOtherInfo_0200
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when abilityHandler_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_DumpOtherInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpOtherInfo_0200 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> info;
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    abilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpOtherInfo_0200 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_DumpOtherInfo_0300
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when currentAbility_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_DumpOtherInfo_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpOtherInfo_0300 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> info;
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    EXPECT_EQ(abilitythread->currentAbility_, nullptr);
    abilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_DumpOtherInfo_0300 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_CallRequest_0100
 * @tc.name: CallRequest
 * @tc.desc: Test CallRequest function when abilityHandler_ and currentAbility_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_CallRequest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_CallRequest_0100 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> info;
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    abilitythread->currentAbility_ = std::make_shared<AbilityRuntime::UIAbility>();
    EXPECT_NE(abilitythread->currentAbility_, nullptr);
    abilitythread->CallRequest();
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_CallRequest_0100 end";
}

/**
 * @tc.number: AbilityRuntime_UIAbilityThread_CallRequest_0200
 * @tc.name: CallRequest
 * @tc.desc: Test CallRequest function when abilityHandler_ and currentAbility_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_UIAbilityThread_CallRequest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_CallRequest_0200 start";
    AbilityRuntime::UIAbilityThread* abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);

    std::vector<std::string> info;
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    EXPECT_EQ(abilitythread->currentAbility_, nullptr);
    abilitythread->CallRequest();
    GTEST_LOG_(INFO) << "AbilityRuntime_UIAbilityThread_CallRequest_0200 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
