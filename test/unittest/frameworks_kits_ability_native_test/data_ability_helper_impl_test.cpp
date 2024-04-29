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

#include "gtest/gtest.h"

#define private public
#define protected public

#include "ability_connect_manager.h"
#include "ability_context.h"
#include "ability_manager_client.h"
#include "ability_scheduler_proxy.h"
#include "ability_thread.h"
#include "abs_shared_result_set.h"
#include "context.h"
#include "datashare_helper.h"
#include "data_ability_helper.h"
#include "data_ability_helper_impl.h"
#include "data_ability_predicates.h"
#include "data_ability_observer_interface.h"
#include "data_ability_observer_stub.h"
#include "data_ability_observer_proxy.h"
#include "mock_ability_manager_client.h"
#include "mock_ability_manager_service.h"
#include "mock_ability_runtime_context.h"
#include "mock_ability_scheduler_for_observer.h"
#include "values_bucket.h"

#undef private
#undef protected

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;

class DataAbilityHelperImplForObserverTest : public testing::Test {
public:
    DataAbilityHelperImplForObserverTest() {}
    virtual ~DataAbilityHelperImplForObserverTest() {}
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

class MolDataAbilityObserver : public AAFwk::DataAbilityObserverStub {
public:
    MolDataAbilityObserver() {};
    virtual ~MolDataAbilityObserver() {};
    void OnChange() override {};
};

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_OnRemoteDied_0100
 * @tc.name: OnRemoteDied
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_OnRemoteDied_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_OnRemoteDied_0100 start";
    DataAbilityDeathRecipient::RemoteDiedHandler handler;
    const wptr<IRemoteObject> remote;
    DataAbilityDeathRecipient dataAbilityDeathRecipient(handler);
    dataAbilityDeathRecipient.OnRemoteDied(remote);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_OnRemoteDied_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_OnRemoteDied_0200
 * @tc.name: OnRemoteDied
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_OnRemoteDied_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_OnRemoteDied_0200 start";
    DataAbilityDeathRecipient::RemoteDiedHandler handler = nullptr;
    const wptr<IRemoteObject> remote;
    DataAbilityDeathRecipient dataAbilityDeathRecipient(handler);
    dataAbilityDeathRecipient.OnRemoteDied(remote);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_OnRemoteDied_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_0100
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0100 start";
    std::shared_ptr<Context> context;
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    EXPECT_TRUE(dataAbilityHelperImpl == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_0200
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0200 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    EXPECT_TRUE(dataAbilityHelperImpl != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_0300
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0300 start";
    std::shared_ptr<Context> context;
    std::shared_ptr<Uri> uri = nullptr;
    bool tryBind = false;
    auto dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context, uri, tryBind);
    EXPECT_TRUE(dataAbilityHelperImpl == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0300 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_0400
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0400 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri = nullptr;
    bool tryBind = false;
    auto dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context, uri, tryBind);
    EXPECT_TRUE(dataAbilityHelperImpl == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0400 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_0500
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0500 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    bool tryBind = false;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    auto dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context, uri, tryBind);
    EXPECT_TRUE(dataAbilityHelperImpl != nullptr);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0500 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_0600
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0600 start";
    std::shared_ptr<OHOS::AbilityRuntime::Context> context = nullptr;
    std::shared_ptr<Uri> uri = nullptr;
    bool tryBind = false;
    auto dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context, uri, tryBind);
    EXPECT_TRUE(dataAbilityHelperImpl == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0600 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_0700
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0700 start";
    std::shared_ptr<OHOS::AbilityRuntime::Context> context =
        std::make_shared<AbilityRuntime::MockAbilityRuntimeContext>();
    std::shared_ptr<Uri> uri = nullptr;
    bool tryBind = false;
    auto dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context, uri, tryBind);
    EXPECT_TRUE(dataAbilityHelperImpl == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0700 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_0800
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0800 start";
    std::shared_ptr<OHOS::AbilityRuntime::Context> context =
        std::make_shared<AbilityRuntime::MockAbilityRuntimeContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    bool tryBind = false;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    auto dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context, uri, tryBind);
    porxyNew->acquireDataAbility_ = nullptr;
    EXPECT_TRUE(dataAbilityHelperImpl != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0800 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_0900
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_0900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0900 start";
    sptr<IRemoteObject> token;
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token);
    EXPECT_TRUE(dataAbilityHelperImpl == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_0900 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_1000
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_1000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_1000 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token);
    EXPECT_TRUE(dataAbilityHelperImpl != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_1000 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_1100
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_1100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_1100 start";
    sptr<IRemoteObject> token;
    std::shared_ptr<Uri> uri = nullptr;
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_1100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_1200
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_1200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_1200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    std::shared_ptr<Uri> uri = nullptr;
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_1200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_1300
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_1300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_1300 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    porxyNew->acquireDataAbility_ = nullptr;
    EXPECT_TRUE(dataAbilityHelperImpl != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_1300 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_1400
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_1400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_1400 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<AAFwk::IAbilityManager> proxy = AbilityManagerClient::GetInstance()->proxy_;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_1400 end";
    AbilityManagerClient::GetInstance()->proxy_ = proxy;
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_1500
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_1500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_1500 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    bool tryBind = true;
    sptr<AAFwk::IAbilityManager> proxy = AbilityManagerClient::GetInstance()->proxy_;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    auto dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context, uri, tryBind);
    EXPECT_TRUE(dataAbilityHelperImpl == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_1500 end";
    AbilityManagerClient::GetInstance()->proxy_ = proxy;
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Creator_1600
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Creator_1600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_1600 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    std::shared_ptr<OHOS::AbilityRuntime::Context> context =
        std::make_shared<AbilityRuntime::MockAbilityRuntimeContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    bool tryBind = true;
    sptr<AAFwk::IAbilityManager> proxy = AbilityManagerClient::GetInstance()->proxy_;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    auto dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context, uri, tryBind);
    EXPECT_TRUE(dataAbilityHelperImpl == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Creator_1600 end";
    AbilityManagerClient::GetInstance()->proxy_ = proxy;
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_CheckUri_0100
 * @tc.name: CheckUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_CheckUri_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUri_0100 start";
    std::shared_ptr<Context> context;
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    EXPECT_TRUE(dataAbilityHelperImpl == nullptr);
    std::shared_ptr<Uri> uri = nullptr;
    bool ret = true;
    ret = dataAbilityHelperImpl->CheckUri(uri);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUri_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_CheckUri_0200
 * @tc.name: CheckUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_CheckUri_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUri_0200 start";
    std::shared_ptr<Context> context;
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    EXPECT_TRUE(dataAbilityHelperImpl == nullptr);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    bool ret = false;
    ret = dataAbilityHelperImpl->CheckUri(uri);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUri_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_CheckUri_0300
 * @tc.name: CheckUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_CheckUri_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUri_0300 start";
    std::shared_ptr<Context> context;
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    EXPECT_TRUE(dataAbilityHelperImpl == nullptr);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("datashare://device_id/com.domainname.dataability.persondata/person/10");
    bool ret = true;
    ret = dataAbilityHelperImpl->CheckUri(uri);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUri_0300 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Release_0100
 * @tc.name: Release
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Release_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Release_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    dataAbilityHelperImpl->uri_ = nullptr;
    porxyNew->acquireDataAbility_ = nullptr;
    bool ret = true;
    ret = dataAbilityHelperImpl->Release();
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Release_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Release_0200
 * @tc.name: Release
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Release_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Release_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    dataAbilityHelperImpl->uri_ = uri;
    porxyNew->acquireDataAbility_ = nullptr;
    bool ret = false;
    ret = dataAbilityHelperImpl->Release();
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Release_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Release_0300
 * @tc.name: Release
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Release_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Release_0300 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    porxyNew->releaseDataAbility_ = 1;
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    dataAbilityHelperImpl->uri_ = uri;
    porxyNew->acquireDataAbility_ = nullptr;
    bool ret = true;
    ret = dataAbilityHelperImpl->Release();
    EXPECT_FALSE(ret);
    porxyNew->releaseDataAbility_ = 0;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Release_0300 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_GetFileTypes_0100
 * @tc.name: GetFileTypes
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_GetFileTypes_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_GetFileTypes_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    const std::string mimeTypeFilter = "abc";
    std::vector<std::string> matchedMIMEs;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl->GetFileTypes(urivalue, mimeTypeFilter) == matchedMIMEs);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_GetFileTypes_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_GetFileTypes_0200
 * @tc.name: GetFileTypes
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_GetFileTypes_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_GetFileTypes_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    const std::string mimeTypeFilter = "mimeTypeFiltertest";
    std::vector<std::string> matchedMIMEs;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl->GetFileTypes(urivalue, mimeTypeFilter) == matchedMIMEs);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_GetFileTypes_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_OpenFile_0100
 * @tc.name: OpenFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_OpenFile_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_OpenFile_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    const std::string mode = "abc";
    constexpr int32_t number = -1;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl->OpenFile(urivalue, mode) == number);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_OpenFile_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_OpenFile_0200
 * @tc.name: OpenFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_OpenFile_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_OpenFile_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    const std::string mode = "modetest";
    constexpr int32_t number = 0;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_EQ(dataAbilityHelperImpl->OpenFile(urivalue, mode), number);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_OpenFile_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_OpenRawFile_0100
 * @tc.name: OpenRawFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_OpenRawFile_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_OpenRawFile_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    const std::string mode = "abc";
    constexpr int32_t number = -1;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl->OpenRawFile(urivalue, mode) == number);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_OpenRawFile_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_OpenRawFile_0200
 * @tc.name: OpenRawFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_OpenRawFile_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_OpenRawFile_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    const std::string mode = "modetest";
    constexpr int32_t number = 0;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_EQ(dataAbilityHelperImpl->OpenRawFile(urivalue, mode), number);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_OpenRawFile_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Insert_0100
 * @tc.name: Insert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Insert_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Insert_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);
    constexpr int32_t number = -1;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl->Insert(urivalue, val) == number);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Insert_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Insert_0200
 * @tc.name: Insert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Insert_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Insert_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);
    constexpr int32_t number = 0;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_EQ(dataAbilityHelperImpl->Insert(urivalue, val), number);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Insert_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Call_0100
 * @tc.name: Call
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Call_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Call_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    const std::string method;
    const std::string arg;
    AppExecFwk::PacMap pacMap;
    std::shared_ptr<AppExecFwk::PacMap> result = nullptr;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl->Call(urivalue, method, arg, pacMap) == result);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Call_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Call_0200
 * @tc.name: Call
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Call_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Call_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    const std::string method;
    const std::string arg;
    AppExecFwk::PacMap pacMap;
    std::shared_ptr<AppExecFwk::PacMap> result = nullptr;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl->Call(urivalue, method, arg, pacMap) == result);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Call_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Update_0100
 * @tc.name: Update
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Update_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Update_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    constexpr int32_t number = -1;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_EQ(dataAbilityHelperImpl->Update(urivalue, val, predicates), number);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Update_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Update_0200
 * @tc.name: Update
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Update_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Update_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    constexpr int32_t number = 0;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_EQ(dataAbilityHelperImpl->Update(urivalue, val, predicates), number);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Update_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Delete_0100
 * @tc.name: Delete
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Delete_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Delete_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    constexpr int32_t number = -1;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_EQ(dataAbilityHelperImpl->Delete(urivalue, predicates), number);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Delete_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Delete_0200
 * @tc.name: Delete
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Delete_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Delete_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    constexpr int32_t number = 0;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_EQ(dataAbilityHelperImpl->Delete(urivalue, predicates), number);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Delete_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Query_0100
 * @tc.name: Query
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Query_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Query_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultset = nullptr;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_EQ(dataAbilityHelperImpl->Query(urivalue, columns, predicates), resultset);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Query_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Query_0200
 * @tc.name: Query
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Query_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Query_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultset = nullptr;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_EQ(dataAbilityHelperImpl->Query(urivalue, columns, predicates), resultset);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Query_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_GetType_0100
 * @tc.name: GetType
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_GetType_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_GetType_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    const std::string type;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_EQ(dataAbilityHelperImpl->GetType(urivalue), type);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_GetType_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_GetType_0200
 * @tc.name: GetType
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_GetType_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_GetType_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    const std::string type;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_EQ(dataAbilityHelperImpl->GetType(urivalue), type);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_GetType_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Reload_0100
 * @tc.name: Reload
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Reload_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Reload_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    PacMap extras;
    bool ret = false;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_EQ(dataAbilityHelperImpl->Reload(urivalue, extras), ret);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Reload_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_Reload_0200
 * @tc.name: Reload
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_Reload_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Reload_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    PacMap extras;
    bool ret = false;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_EQ(dataAbilityHelperImpl->Reload(urivalue, extras), ret);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_Reload_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_BatchInsert_0100
 * @tc.name: BatchInsert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_BatchInsert_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_BatchInsert_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    std::vector<NativeRdb::ValuesBucket> values;
    constexpr int32_t ret = -1;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_EQ(dataAbilityHelperImpl->BatchInsert(urivalue, values), ret);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_BatchInsert_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_BatchInsert_0200
 * @tc.name: BatchInsert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_BatchInsert_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_BatchInsert_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    constexpr int32_t ret = 0;
    std::vector<NativeRdb::ValuesBucket> values;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_EQ(dataAbilityHelperImpl->BatchInsert(urivalue, values), ret);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_BatchInsert_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_NotifyChange_0100
 * @tc.name: NotifyChange
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_NotifyChange_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_NotifyChange_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    dataAbilityHelperImpl->NotifyChange(urivalue);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_NotifyChange_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_NotifyChange_0200
 * @tc.name: NotifyChange
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_NotifyChange_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_NotifyChange_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    dataAbilityHelperImpl->NotifyChange(urivalue);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_NotifyChange_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_NormalizeUri_0100
 * @tc.name: NormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_NormalizeUri_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_NormalizeUri_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    Uri urivalue_("");
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl->NormalizeUri(urivalue) == urivalue_);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_NormalizeUri_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_NormalizeUri_0200
 * @tc.name: NormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_NormalizeUri_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_NormalizeUri_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    Uri urivalue_("");
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("dataability://device_id/com.domainname.dataability.");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    dataAbilityHelperImpl->NormalizeUri(urivalue);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_NormalizeUri_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_DenormalizeUri_0100
 * @tc.name: DenormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_DenormalizeUri_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_DenormalizeUri_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    Uri urivalue_("");
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl->DenormalizeUri(urivalue) == urivalue_);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_DenormalizeUri_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_DenormalizeUri_0200
 * @tc.name: DenormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_DenormalizeUri_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_DenormalizeUri_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    Uri urivalue_("");
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("dataability://device_id/com.domainname.dataability.");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl->DenormalizeUri(urivalue) == urivalue_);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_DenormalizeUri_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_ExecuteBatch_0100
 * @tc.name: ExecuteBatch
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_ExecuteBatch_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_ExecuteBatch_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl->ExecuteBatch(urivalue, operations) == results);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_ExecuteBatch_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_ExecuteBatch_0200
 * @tc.name: ExecuteBatch
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_ExecuteBatch_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_ExecuteBatch_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl->ExecuteBatch(urivalue, operations) == results);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_ExecuteBatch_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_CheckOhosUri_0100
 * @tc.name: CheckOhosUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_CheckOhosUri_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckOhosUri_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    Uri urivalue("\nullptr");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_FALSE(dataAbilityHelperImpl->CheckOhosUri(urivalue));
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckOhosUri_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_CheckOhosUri_0200
 * @tc.name: CheckOhosUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_CheckOhosUri_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckOhosUri_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    EXPECT_TRUE(dataAbilityHelperImpl->CheckOhosUri(*uri));
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckOhosUri_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0100
 * @tc.name: ReleaseDataAbility
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityHelperImpl->GetDataAbilityProxy(*uri);
    dataAbilityHelperImpl->ReleaseDataAbility(dataAbilityProxy);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0200
 * @tc.name: ReleaseDataAbility
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->token_ = token;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityHelperImpl->GetDataAbilityProxy(*uri);
    dataAbilityHelperImpl->ReleaseDataAbility(dataAbilityProxy);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0300
 * @tc.name: ReleaseDataAbility
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0300 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->uri_ = uri;
    dataAbilityHelperImpl->token_ = token;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy;
    dataAbilityHelperImpl->ReleaseDataAbility(dataAbilityProxy);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0300 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0400
 * @tc.name: ReleaseDataAbility
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0400 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->token_ = token;
    dataAbilityHelperImpl->uri_ = uri;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityHelperImpl->GetDataAbilityProxy(*uri);
    dataAbilityHelperImpl->ReleaseDataAbility(dataAbilityProxy);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_ReleaseDataAbility_0400 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_OnSchedulerDied_0100
 * @tc.name: OnSchedulerDied
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_OnSchedulerDied_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_OnSchedulerDied_0100 start";
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    const wptr<IRemoteObject> remote;
    dataAbilityHelperImpl->OnSchedulerDied(remote);
    porxyNew->acquireDataAbility_ = nullptr;
    EXPECT_TRUE(dataAbilityHelperImpl->uri_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_OnSchedulerDied_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_AddDataAbilityDeathRecipient_0100
 * @tc.name: AddDataAbilityDeathRecipient
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_AddDataAbilityDeathRecipient_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_AddDataAbilityDeathRecipient_0100 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    DataAbilityDeathRecipient::RemoteDiedHandler handler;
    sptr<IRemoteObject::DeathRecipient> callerDeathRecipient = new (std::nothrow) DataAbilityDeathRecipient(handler);
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->callerDeathRecipient_ = callerDeathRecipient;
    dataAbilityHelperImpl->AddDataAbilityDeathRecipient(token);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_AddDataAbilityDeathRecipient_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_AddDataAbilityDeathRecipient_0200
 * @tc.name: AddDataAbilityDeathRecipient
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_AddDataAbilityDeathRecipient_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_AddDataAbilityDeathRecipient_0200 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token;
    DataAbilityDeathRecipient::RemoteDiedHandler handler;
    sptr<IRemoteObject::DeathRecipient> callerDeathRecipient = new (std::nothrow) DataAbilityDeathRecipient(handler);
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->callerDeathRecipient_ = callerDeathRecipient;
    dataAbilityHelperImpl->AddDataAbilityDeathRecipient(token);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_AddDataAbilityDeathRecipient_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_AddDataAbilityDeathRecipient_0300
 * @tc.name: AddDataAbilityDeathRecipient
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_AddDataAbilityDeathRecipient_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_AddDataAbilityDeathRecipient_0300 start";
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> token = new OHOS::AAFwk::Token(abilityRecord);
    sptr<IRemoteObject::DeathRecipient> callerDeathRecipient = nullptr;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->callerDeathRecipient_ = callerDeathRecipient;
    dataAbilityHelperImpl->AddDataAbilityDeathRecipient(token);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_AddDataAbilityDeathRecipient_0300 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_CheckUriAndDataObserver_0100
 * @tc.name: CheckUriAndDataObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_CheckUriAndDataObserver_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUriAndDataObserver_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("\nullptr");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    EXPECT_FALSE(dataAbilityHelperImpl->CheckUriAndDataObserver(*uri, dataObserver));
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUriAndDataObserver_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_CheckUriAndDataObserver_0200
 * @tc.name: CheckUriAndDataObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_CheckUriAndDataObserver_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUriAndDataObserver_0200 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    EXPECT_FALSE(dataAbilityHelperImpl->CheckUriAndDataObserver(*uri, dataObserver));
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUriAndDataObserver_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_CheckUriAndDataObserver_0300
 * @tc.name: CheckUriAndDataObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_CheckUriAndDataObserver_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUriAndDataObserver_0300 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver = new (std::nothrow) MolDataAbilityObserver();
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    EXPECT_TRUE(dataAbilityHelperImpl->CheckUriAndDataObserver(*uri, dataObserver));
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUriAndDataObserver_0300 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_CheckUriParam_0100
 * @tc.name: CheckUriParam
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_CheckUriParam_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUriParam_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("\nullptr");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    EXPECT_FALSE(dataAbilityHelperImpl->CheckUriParam(*uri));
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUriParam_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_CheckUriParam_0200
 * @tc.name: CheckUriParam
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_CheckUriParam_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUriParam_0200 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    EXPECT_TRUE(dataAbilityHelperImpl->CheckUriParam(*uri));
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_CheckUriParam_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_GetDataAbilityProxy_0100
 * @tc.name: GetDataAbilityProxy
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_GetDataAbilityProxy_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_GetDataAbilityProxy_0100 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("\nullptr");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    bool addDeathRecipient = true;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    EXPECT_TRUE(dataAbilityHelperImpl->GetDataAbilityProxy(*uri, addDeathRecipient) == nullptr);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_GetDataAbilityProxy_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_GetDataAbilityProxy_0200
 * @tc.name: GetDataAbilityProxy
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_GetDataAbilityProxy_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_GetDataAbilityProxy_0200 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    bool addDeathRecipient = true;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    EXPECT_TRUE(dataAbilityHelperImpl->GetDataAbilityProxy(*uri, addDeathRecipient) != nullptr);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_GetDataAbilityProxy_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_GetDataAbilityProxy_0300
 * @tc.name: GetDataAbilityProxy
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_GetDataAbilityProxy_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_GetDataAbilityProxy_0300 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    bool addDeathRecipient = true;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->uri_ = nullptr;
    dataAbilityHelperImpl->isSystemCaller_ = true;
    EXPECT_TRUE(dataAbilityHelperImpl->GetDataAbilityProxy(*uri, addDeathRecipient) != nullptr);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_GetDataAbilityProxy_0300 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0100
 * @tc.name: RegisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0100 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->RegisterObserver(*uri, dataObserver);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0200
 * @tc.name: RegisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0200 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("\nullptr");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->RegisterObserver(*uri, dataObserver);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0300
 * @tc.name: RegisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0300 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver = new (std::nothrow) MolDataAbilityObserver();
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->uri_ = nullptr;
    dataAbilityHelperImpl->RegisterObserver(*uri, dataObserver);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0300 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0400
 * @tc.name: RegisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0400 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver = new (std::nothrow) MolDataAbilityObserver();
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->uri_ = uri;
    dataAbilityHelperImpl->RegisterObserver(*uri, dataObserver);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0400 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0500
 * @tc.name: RegisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0500 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver = new (std::nothrow) MolDataAbilityObserver();
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->uri_ = uri;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityHelperImpl->GetDataAbilityProxy(*uri);
    dataAbilityHelperImpl->dataAbilityProxy_ = dataAbilityProxy;
    dataAbilityHelperImpl->RegisterObserver(*uri, dataObserver);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_RegisterObserver_0500 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0100
 * @tc.name: UnregisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0100 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->UnregisterObserver(*uri, dataObserver);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0100 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0200
 * @tc.name: UnregisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0200 start";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("\nullptr");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->UnregisterObserver(*uri, dataObserver);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0200 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0300
 * @tc.name: UnregisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0300 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver = new (std::nothrow) MolDataAbilityObserver();
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->uri_ = nullptr;
    dataAbilityHelperImpl->UnregisterObserver(*uri, dataObserver);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0300 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0400
 * @tc.name: UnregisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0400 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver = new (std::nothrow) MolDataAbilityObserver();
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->uri_ = uri;
    dataAbilityHelperImpl->UnregisterObserver(*uri, dataObserver);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0400 end";
}

/**
 * @tc.number: AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0500
 * @tc.name: UnregisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperImplForObserverTest,
    AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0500 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    const sptr<AAFwk::IDataAbilityObserver> dataObserver = new (std::nothrow) MolDataAbilityObserver();
    sptr<AAFwk::MockAbilityManagerService> porxyNew = new (std::nothrow) AAFwk::MockAbilityManagerService();
    porxyNew->acquireDataAbility_ = new (std::nothrow) MockAbilitySchedulerStub();
    AbilityManagerClient::GetInstance()->proxy_ = porxyNew;
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    dataAbilityHelperImpl->uri_ = uri;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityHelperImpl->GetDataAbilityProxy(*uri);
    dataAbilityHelperImpl->dataAbilityProxy_ = dataAbilityProxy;
    dataAbilityHelperImpl->UnregisterObserver(*uri, dataObserver);
    porxyNew->acquireDataAbility_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_DataAbilityHelperImpl_UnregisterObserver_0500 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS