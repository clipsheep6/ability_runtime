/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "extension_ability_thread.h"
#undef private
#undef protected
#include "mock_ability_thread.h"
#include "mock_ability_token.h"
#include "ohos_application.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

static const int32_t STARTID = 0;

class ExtensionAbilityThreadTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void ExtensionAbilityThreadTest::SetUpTestCase(void) {}

void ExtensionAbilityThreadTest::TearDownTestCase(void) {}

void ExtensionAbilityThreadTest::SetUp(void) {}

void ExtensionAbilityThreadTest::TearDown(void) {}

/**
 * @tc.name: ExtensionAbilityThread_DumpAbilityInfo_0100
 * @tc.desc: DumpAbilityInfo
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpAbilityInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpAbilityInfo_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
        std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
        std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
        extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
        std::vector<std::string> params;
        std::vector<std::string> info;
        extensionabilitythread->DumpAbilityInfo(params, info);
        EXPECT_EQ(info.size(), 0);
        GTEST_LOG_(INFO) << "info:";
        for (auto item : info) {
            GTEST_LOG_(INFO) << item;
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfo_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpAbilityInfo_0200
 * @tc.name: DumpAbilityInfo
 * @tc.desc: Test DumpAbilityInfo function when token_ and abilityHandler_ is
 * not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpAbilityInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfo_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(extensionabilitythread->token_, nullptr);
    extensionabilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(extensionabilitythread->abilityHandler_, nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    extensionabilitythread->DumpAbilityInfo(params, info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfo_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpAbilityInfo_0300
 * @tc.name: DumpAbilityInfo
 * @tc.desc: Test DumpAbilityInfo function when abilityHandler_ is not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpAbilityInfo_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfo_0300 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(extensionabilitythread->token_, nullptr);
    EXPECT_EQ(extensionabilitythread->abilityHandler_, nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    extensionabilitythread->DumpAbilityInfo(params, info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfo_0300 end";
}

/**
 * @tc.number: ExtensionAbilityThread_Attach_3_Param_0100
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_Attach_3_Param_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_3_Param_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_3_Param_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_Attach_3_Param_0200
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_Attach_3_Param_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_3_Param_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = nullptr;
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_3_Param_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_Attach_2_Param_0100
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_Attach_2_Param_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_2_Param_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            extensionabilitythread->Attach(application, abilityRecord, nullptr);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_2_Param_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_Attach_2_Param_0200
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_Attach_2_Param_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_2_Param_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = nullptr;
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, nullptr);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_2_Param_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleAbilityTransaction_0100
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleAbilityTransaction_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionAbilityThread_ScheduleAbilityTransaction_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            Want want;
            LifeCycleStateInfo lifeCycleStateInfo;
            extensionabilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityTransaction_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleAbilityTransaction_0200
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleAbilityTransaction_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityTransaction_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            extensionabilitythread->Attach(application, abilityRecord, nullptr);
            Want want;
            LifeCycleStateInfo lifeCycleStateInfo;
            extensionabilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityTransaction_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleAbilityTransaction_0300
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Test ScheduleAbilityTransaction function when token_ is not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleAbilityTransaction_0300,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityTransaction_0300 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(extensionabilitythread->token_, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    extensionabilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityTransaction_0300 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleAbilityTransaction_0400
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Test ScheduleAbilityTransaction function when abilityHandler_ and
 * token_ is not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleAbilityTransaction_0400,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityTransaction_0400 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(extensionabilitythread->token_, nullptr);
    extensionabilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(extensionabilitythread->abilityHandler_, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    extensionabilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityTransaction_0400 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleConnectAbility_0100
 * @tc.name: ScheduleConnectAbility
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleConnectAbility_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleConnectAbility_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            Want want;
            extensionabilitythread->ScheduleConnectAbility(want);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleConnectAbility_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleConnectAbility_0200
 * @tc.name: ScheduleConnectAbility
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleConnectAbility_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionAbilityThread_ScheduleConnectAbility_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
        std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
        std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
        extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
        Want want;
        extensionabilitythread->ScheduleConnectAbility(want);
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleConnectAbility_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleDisconnectAbility_0100
 * @tc.name: ScheduleDisconnectAbility
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleDisconnectAbility_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleDisconnectAbility_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            Want want;
            extensionabilitythread->ScheduleDisconnectAbility(want);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleDisconnectAbility_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleDisconnectAbility_0200
 * @tc.name: ScheduleDisconnectAbility
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleDisconnectAbility_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleDisconnectAbility_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
        std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
        std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
        extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
        Want want;
        extensionabilitythread->ScheduleDisconnectAbility(want);
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleDisconnectAbility_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleCommandAbility_0100
 * @tc.name: ScheduleCommandAbility
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleCommandAbility_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleCommandAbility_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        abilityInfo->isNativeAbility = true;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            Want want;
            bool restart = true;
            int startId = 0;
            extensionabilitythread->ScheduleCommandAbility(want, restart, startId);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleCommandAbility_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleCommandAbility_0200
 * @tc.name: ScheduleCommandAbility
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleCommandAbility_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleCommandAbility_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
        std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
        std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
        extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
        Want want;
        bool restart = true;
        int startId = 0;
        extensionabilitythread->ScheduleCommandAbility(want, restart, startId);
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleCommandAbility_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleCommandAbilityWindow_0100
 * @tc.name: ScheduleCommandAbilityWindow
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleCommandAbilityWindow_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleCommandAbilityWindow_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            std::shared_ptr<AbilityLocalRecord> abilityRecord =
                std::make_shared<AbilityLocalRecord>(abilityInfo, token);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            Want want;
            sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
            extensionabilitythread->ScheduleCommandAbilityWindow(want, session, AAFwk::WIN_CMD_FOREGROUND);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleCommandAbilityWindow_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleExtensionTransaction_0100
 * @tc.name: HandleExtensionTransaction
 * @tc.desc: Test HandleExtensionTransaction function when extensionImpl_ is
 * nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleExtensionTransaction_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleExtensionTransaction_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    extensionabilitythread->HandleExtensionTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleExtensionTransaction_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleExtensionTransaction_0200
 * @tc.name: HandleExtensionTransaction
 * @tc.desc: Test HandleExtensionTransaction function when extensionImpl_ is not
 * nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleExtensionTransaction_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleExtensionTransaction_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    extensionabilitythread->HandleExtensionTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleExtensionTransaction_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleConnectExtension_0100
 * @tc.name: HandleConnectExtension
 * @tc.desc: Test HandleConnectExtension function when extensionImpl_ is nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleConnectExtension_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleConnectExtension_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    extensionabilitythread->HandleConnectExtension(want);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleConnectExtension_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleConnectExtension_0200
 * @tc.name: HandleConnectExtension
 * @tc.desc: Test HandleConnectExtension function when extensionImpl_ is not
 * nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleConnectExtension_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleConnectExtension_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    extensionabilitythread->HandleConnectExtension(want);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleConnectExtension_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleCommandExtension_0100
 * @tc.name: HandleCommandExtension
 * @tc.desc: Test HandleCommandExtension function when extensionImpl_ is nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleCommandExtension_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleCommandExtension_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    extensionabilitythread->HandleCommandExtension(want, false, STARTID);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleCommandExtension_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleCommandExtension_0200
 * @tc.name: HandleCommandExtension
 * @tc.desc: Test HandleCommandExtension function when extensionImpl_ is not
 * nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleCommandExtension_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleCommandExtension_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    extensionabilitythread->HandleCommandExtension(want, false, STARTID);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleCommandExtension_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleCommandExtensionWindow_0100
 * @tc.name: HandleCommandExtensionWindow
 * @tc.desc: Test HandleCommandExtensionWindow function when extensionImpl_ is
 * nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleCommandExtensionWindow_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleCommandExtensionWindow_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
    extensionabilitythread->HandleCommandExtensionWindow(want, session, AAFwk::WIN_CMD_FOREGROUND);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleCommandExtensionWindow_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleUpdateConfiguration_0100
 * @tc.name: ScheduleUpdateConfiguration
 * @tc.desc: Test ScheduleUpdateConfiguration function when abilityHandler_ is
 * nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleUpdateConfiguration_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleUpdateConfiguration_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->abilityHandler_, nullptr);
    Configuration config;
    extensionabilitythread->ScheduleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleUpdateConfiguration_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleUpdateConfiguration_0200
 * @tc.name: ScheduleUpdateConfiguration
 * @tc.desc: Test ScheduleUpdateConfiguration function when abilityHandler_ is
 * not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleUpdateConfiguration_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleUpdateConfiguration_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(extensionabilitythread->abilityHandler_, nullptr);
    Configuration config;
    extensionabilitythread->ScheduleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleUpdateConfiguration_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ExtensionUpdateConfiguration_0100
 * @tc.name: HandleExtensionUpdateConfiguration
 * @tc.desc: Test HandleExtensionUpdateConfiguration function when
 * extensionImpl_ is nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ExtensionUpdateConfiguration_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ExtensionUpdateConfiguration_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->extensionImpl_, nullptr);
    Configuration config;
    extensionabilitythread->HandleExtensionUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ExtensionUpdateConfiguration_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ExtensionUpdateConfiguration_0200
 * @tc.name: HandleExtensionUpdateConfiguration
 * @tc.desc: Test HandleExtensionUpdateConfiguration function when
 * extensionImpl_ is not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ExtensionUpdateConfiguration_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ExtensionUpdateConfiguration_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    Configuration config;
    extensionabilitythread->HandleExtensionUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ExtensionUpdateConfiguration_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_NotifyMemoryLevel_0100
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Test NotifyMemoryLevel function when isExtension_ is false and
 * abilityImpl_ is nullptr
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_NotifyMemoryLevel_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->extensionImpl_, nullptr);
    int32_t result = STARTID;
    extensionabilitythread->NotifyMemoryLevel(result);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_NotifyMemoryLevel_0200
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Test NotifyMemoryLevel function when isExtension_ is false and
 * abilityImpl_ is not nullptr
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_NotifyMemoryLevel_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    int32_t result = STARTID;
    extensionabilitythread->NotifyMemoryLevel(result);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_NotifyMemoryLevel_0300
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Test NotifyMemoryLevel function when isExtension_ is true and
 * extensionImpl_ is nullptr
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_NotifyMemoryLevel_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0300 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->extensionImpl_, nullptr);
    int32_t result = STARTID;
    extensionabilitythread->NotifyMemoryLevel(result);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0300 end";
}

/**
 * @tc.number: ExtensionAbilityThread_NotifyMemoryLevel_0400
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Test NotifyMemoryLevel function when isExtension_ is true and
 * extensionImpl_ is not nullptr
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_NotifyMemoryLevel_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0400 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    int32_t result = STARTID;
    extensionabilitythread->NotifyMemoryLevel(result);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0400 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpAbilityInfoInner_0100
 * @tc.name: DumpAbilityInfoInner
 * @tc.desc: Test DumpAbilityInfoInner function when currentAbility_ and
 * currentExtension_ is not nullptr
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpAbilityInfoInner_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfoInner_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->currentExtension_ = std::make_shared<AbilityRuntime::Extension>();
    EXPECT_NE(extensionabilitythread->currentExtension_, nullptr);
    extensionabilitythread->currentExtension_ = std::make_shared<AbilityRuntime::Extension>();
    EXPECT_NE(extensionabilitythread->currentExtension_, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    extensionabilitythread->DumpAbilityInfoInner(params, info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfoInner_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpAbilityInfoInner_0200
 * @tc.name: DumpAbilityInfoInner
 * @tc.desc: Test DumpAbilityInfoInner function when currentAbility_ is nullptr
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpAbilityInfoInner_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfoInner_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->currentExtension_, nullptr);
    extensionabilitythread->currentExtension_ = std::make_shared<AbilityRuntime::Extension>();
    EXPECT_NE(extensionabilitythread->currentExtension_, nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    extensionabilitythread->DumpAbilityInfoInner(params, info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfoInner_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpAbilityInfoInner_0300
 * @tc.name: DumpAbilityInfoInner
 * @tc.desc: Test DumpAbilityInfoInner function when currentExtension_ is
 * nullptr
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpAbilityInfoInner_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfoInner_0300 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->currentExtension_ = std::make_shared<AbilityRuntime::Extension>();
    EXPECT_NE(extensionabilitythread->currentExtension_, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    extensionabilitythread->DumpAbilityInfoInner(params, info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfoInner_0300 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpOtherInfo_0100
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when abilityHandler_ and
 * currentAbility_ is not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpOtherInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpOtherInfo_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(extensionabilitythread->abilityHandler_, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ExtensionAbility";
    abilityInfo->type = AbilityType::EXTENSION;
    auto setRunner = EventRunner::Create(abilityInfo->name);
    extensionabilitythread->abilityHandler_->SetEventRunner(setRunner);
    auto getRunner = extensionabilitythread->abilityHandler_->GetEventRunner();
    EXPECT_NE(getRunner, nullptr);
    extensionabilitythread->currentExtension_ = std::make_shared<AbilityRuntime::Extension>();
    EXPECT_NE(extensionabilitythread->currentExtension_, nullptr);
    std::vector<std::string> info;
    extensionabilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpOtherInfo_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpOtherInfo_0200
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when abilityHandler_ is nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpOtherInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpOtherInfo_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->abilityHandler_, nullptr);
    std::vector<std::string> info;
    extensionabilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpOtherInfo_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpOtherInfo_0300
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when currentAbility_ is nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpOtherInfo_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpOtherInfo_0300 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(extensionabilitythread->abilityHandler_, nullptr);
    EXPECT_EQ(extensionabilitythread->currentExtension_, nullptr);
    std::vector<std::string> info;
    extensionabilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpOtherInfo_0300 end";
}

/**
 * @tc.number: ExtensionAbilityThread_CreateAbilityName_0100
 * @tc.name: CreateAbilityName
 * @tc.desc: Test CreateAbilityName function when parameters are application,
 * mainRunner and abilityRecord
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_CreateAbilityName_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_CreateAbilityName_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ExtensionAbility";
    abilityInfo->type = AbilityType::EXTENSION;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
    std::string abilityName = extensionabilitythread->CreateAbilityName(abilityRecord, application);
    auto extension = AbilityLoader::GetInstance().GetExtensionByName(abilityName);
    EXPECT_EQ(extension, nullptr);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_CreateAbilityName_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleDisconnectExtension_0100
 * @tc.name: HandleDisconnectExtension
 * @tc.desc: Test HandleDisconnectExtension function when abilityImpl_ is not
 * nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleDisconnectExtension_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleDisconnectExtension_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    extensionabilitythread->HandleDisconnectExtension(want);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleDisconnectExtension_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS