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

#include <gtest/gtest.h>
#define private public
#include "local_call_container.h"
#define protected public
#include "ability_loader.h"
#include "ability_thread.h"
#include "mock_serviceability_manager_service.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

class LocalCallContainerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void LocalCallContainerTest::SetUpTestCase(void)
{
    OHOS::sptr<OHOS::IRemoteObject> abilityObject = new (std::nothrow) MockServiceAbilityManagerService();

    auto sysMgr = OHOS::DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == NULL) {
        GTEST_LOG_(ERROR) << "fail to get ISystemAbilityManager";
        return;
    }

    sysMgr->RegisterSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID, abilityObject);
}

void LocalCallContainerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "LocalCallContainerTest::TearDownTestCase end";
}

void LocalCallContainerTest::SetUp(void)
{}

void LocalCallContainerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "LocalCallContainerTest::TearDown end";
}

/**
 * @tc.number: Local_Call_Container_StartAbilityInner_0100
 * @tc.name: StartAbility
 * @tc.desc: Local Call Container to process StartAbilityInner, and the result is success.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_StartAbilityInner_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Local_Call_Container_StartAbilityInner_0100::" <<
        "Local_Call_Container_StartAbilityInner_0100 begain";

    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();

    Want want;
    want.SetElementName("DemoDeviceIdB", "DemoBundleNameB", "DemoAbilityNameB");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    GTEST_LOG_(INFO) << "Local_Call_Container_StartAbilityInner_0100::" <<
        "Local_Call_Container_StartAbilityInner_0100 begain1";
    ErrCode ret = localCallContainer->StartAbilityByCallInner(want, callback, nullptr);
    GTEST_LOG_(INFO) << "Local_Call_Container_StartAbilityInner_0100::" <<
        "Local_Call_Container_StartAbilityInner_0100 begain2";
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Local_Call_Container_StartAbilityInner_0100::" <<
        "Local_Call_Container_StartAbilityInner_0100 end";
}

/**
 * @tc.number: Local_Call_Container_StartAbilityInner_0200
 * @tc.name: StartAbility
 * @tc.desc: Local Call Container to process StartAbilityInner, and the result is fail because callback is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_StartAbilityInner_0200, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");

    sptr<IRemoteObject> callRemoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);

    ErrCode ret = localCallContainer.StartAbilityByCallInner(want, nullptr, callRemoteObject);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_StartAbilityInner_0300
 * @tc.name: StartAbility
 * @tc.desc: Local Call Container to process StartAbilityInner, and the result is fail because bundle name is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_StartAbilityInner_0300, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    Want want;
    want.SetElementName("DemoDeviceId", "", "DemoAbilityName");

    sptr<IRemoteObject> callRemoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);

    ErrCode ret = localCallContainer.StartAbilityByCallInner(want, nullptr, callRemoteObject);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_StartAbilityInner_0400
 * @tc.name: StartAbility
 * @tc.desc: Local Call Container to process StartAbilityInner, and the result is fail because ability name is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_StartAbilityInner_0400, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "");

    sptr<IRemoteObject> callRemoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);

    ErrCode ret = localCallContainer.StartAbilityByCallInner(want, nullptr, callRemoteObject);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_Release_0100
 * @tc.name: Release
 * @tc.desc: Local Call Container to process Release, and the result is success.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0100, Function | MediumTest | Level1)
{
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();

    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    std::shared_ptr<CallerCallBack> callbackSec = std::make_shared<CallerCallBack>();

    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    localCallRecord->AddCaller(callbackSec);

    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    callbackSec->SetCallBack([](const sptr<IRemoteObject>&) {});

    std::string uri = elementName.GetURI();
    localCallContainer->callProxyRecords_.emplace(uri, localCallRecord);

    ErrCode ret = localCallContainer->ReleaseCall(callback);
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.number: Local_Call_Container_Release_0200
 * @tc.name: Release
 * @tc.desc: Local Call Container to process Release, and the result is fail because callProxyRecords_ is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0200, Function | MediumTest | Level1)
{
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();

    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();

    callback->SetCallBack([](const sptr<IRemoteObject>&) {});

    ErrCode ret = localCallContainer->ReleaseCall(callback);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_Release_0300
 * @tc.name: Release
 * @tc.desc: Parameter 'callback' is nullptr, and Error 'ERR_INVALID_VALUE' returned.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0300, Function | MediumTest | Level1)
{
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();
    ErrCode ret = localCallContainer->ReleaseCall(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_Release_0400
 * @tc.name: Release
 * @tc.desc: Parameter 'localCallRecord' is nullptr, and Error 'ERR_INVALID_VALUE' returned.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0400, Function | MediumTest | Level1)
{
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();
    ErrCode ret = localCallContainer->ReleaseCall(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_Release_0700
 * @tc.name: Release
 * @tc.desc: Parameter 'connect' is nullptr, and Error 'ERR_INVALID_VALUE' returned.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0700, Function | MediumTest | Level1)
{
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> LocalCallRecordFirst = std::make_shared<LocalCallRecord>(elementName);
    std::weak_ptr<LocalCallRecord> LocalCallRecordSec = LocalCallRecordFirst;
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetRecord(LocalCallRecordSec);
    ErrCode ret = localCallContainer->ReleaseCall(callback);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_Release_0800
 * @tc.name: Release
 * @tc.desc: When 'localCallRecord' is an 'IsSingletonRemote',
 * remove 'localCallRecord' by 'RemoveSingletonCallLocalRecord', and return 'ERR_OK'.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0800, Function | MediumTest | Level1)
{
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> LocalCallRecordFirst = std::make_shared<LocalCallRecord>(elementName);
    std::weak_ptr<LocalCallRecord> LocalCallRecordSec = LocalCallRecordFirst;
    LocalCallRecordFirst->SetIsSingleton(true);
    LocalCallRecordFirst->SetConnection(localCallContainer);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetRecord(LocalCallRecordSec);
    ErrCode ret = localCallContainer->ReleaseCall(callback);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Local_Call_Container_Release_0900
 * @tc.name: Release
 * @tc.desc: Release record in multiple not found.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_0900, Function | MediumTest | Level1)
{
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> LocalCallRecordFirst = std::make_shared<LocalCallRecord>(elementName);
    std::weak_ptr<LocalCallRecord> LocalCallRecordSec = LocalCallRecordFirst;
    LocalCallRecordFirst->SetConnection(localCallContainer);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetRecord(LocalCallRecordSec);
    ErrCode ret = localCallContainer->ReleaseCall(callback);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: Local_Call_Container_Release_1000
 * @tc.name: Release
 * @tc.desc: When 'element' doesn't have record, delete itself from 'multipleCallProxyRecords_'.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_Release_1000, Function | MediumTest | Level1)
{
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> LocalCallRecordFirst = std::make_shared<LocalCallRecord>(elementName);
    std::weak_ptr<LocalCallRecord> LocalCallRecordSec = LocalCallRecordFirst;
    LocalCallRecordFirst->SetConnection(localCallContainer);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetRecord(LocalCallRecordSec);
    localCallContainer->SetMultipleCallLocalRecord(elementName, LocalCallRecordFirst);
    ErrCode ret = localCallContainer->ReleaseCall(callback);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Local_Call_Container_SetCallLocalRecord_1000
 * @tc.name: SetCallLocalRecord
 * @tc.desc: Set 'callProxyRecords_'.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_SetCallLocalRecord_0100, Function | MediumTest | Level1)
{
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallContainer->SetCallLocalRecord(elementName, localCallRecord);
    EXPECT_NE(
        localCallContainer->callProxyRecords_.find(elementName.GetURI()),
        localCallContainer->callProxyRecords_.end());
}

/**
 * @tc.number: Local_Call_Container_SetMultipleCallLocalRecord_0100
 * @tc.name: SetMultipleCallLocalRecord
 * @tc.desc: Set 'multipleCallProxyRecords_'.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_SetMultipleCallLocalRecord_0100, Function | MediumTest | Level1)
{
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecordFirst = std::make_shared<LocalCallRecord>(elementName);
    constexpr int32_t COUNT_NUM = 1;
    localCallContainer->SetMultipleCallLocalRecord(elementName, localCallRecordFirst);
    EXPECT_EQ(
        COUNT_NUM, localCallContainer->multipleCallProxyRecords_[elementName.GetURI()].count(localCallRecordFirst));

    std::shared_ptr<LocalCallRecord> localCallRecordSecond = std::make_shared<LocalCallRecord>(elementName);
    localCallContainer->SetMultipleCallLocalRecord(elementName, localCallRecordSecond);

    EXPECT_EQ(
        COUNT_NUM, localCallContainer->multipleCallProxyRecords_[elementName.GetURI()].count(localCallRecordSecond));
}

/**
 * @tc.number: Local_Call_Container_SetRecordAndContainer_0100
 * @tc.name: SetRecordAndContainer
 * @tc.desc: Set 'container_' and 'localCallRecord_'.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_SetRecordAndContainer_0100, Function | MediumTest | Level1)
{
    sptr<CallerConnection> Container = new CallerConnection();
    sptr<IRemoteObject> callRemoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    Container->SetRecordAndContainer(localCallRecord, callRemoteObject);
    EXPECT_EQ(Container->localCallRecord_, localCallRecord);
    EXPECT_EQ(Container->container_, iface_cast<LocalCallContainer>(callRemoteObject));
}

/**
 * @tc.number: Local_Call_Container_OnAbilityConnectDone_0400
 * @tc.name: OnAbilityConnectDone
 * @tc.desc: OnAbilityConnectDone container is nullptr.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityConnectDone_0400, Function | MediumTest | Level1)
{
    sptr<CallerConnection> Container = new CallerConnection();
    sptr<IRemoteObject> callRemoteObject =
        OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    Container->OnAbilityConnectDone(elementName, callRemoteObject,
        static_cast<int32_t>(AppExecFwk::LaunchMode::SINGLETON));
    EXPECT_EQ(nullptr, Container->container_.promote());
}

/**
 * @tc.number: Local_Call_Container_OnAbilityConnectDone_0500
 * @tc.name: OnAbilityConnectDone
 * @tc.desc: 'Code' is a Singleton.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityConnectDone_0500, Function | MediumTest | Level1)
{
    sptr<CallerConnection> Connection = new CallerConnection();
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    Connection->container_ = localCallContainer;
    constexpr int32_t code = 0;
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    Connection->localCallRecord_ = localCallRecord;
    Connection->OnAbilityConnectDone(elementName, localCallContainer, code);
    EXPECT_EQ(localCallContainer->callProxyRecords_[elementName.GetURI()], Connection->localCallRecord_);
}

/**
 * @tc.number: Local_Call_Container_OnAbilityConnectDone_0600
 * @tc.name: OnAbilityConnectDone
 * @tc.desc: 'Code' is not a Singleton.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityConnectDone_0600, Function | MediumTest | Level1)
{
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    sptr<CallerConnection> Connection = new CallerConnection();
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    Connection->container_ = localCallContainer;
    constexpr int32_t code = 1;
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    Connection->localCallRecord_ = localCallRecord;
    Connection->OnAbilityConnectDone(elementName, localCallContainer, code);
    constexpr int32_t COUNT_NUM = 1;
    EXPECT_EQ(Connection->container_->multipleCallProxyRecords_[elementName.GetURI()].
        count(Connection->localCallRecord_), COUNT_NUM);
    EXPECT_TRUE(callback->isCallBack_);
}

/**
 * @tc.number: Local_Call_Container_OnAbilityDisconnectDone_0100
 * @tc.name: OnAbilityDisconnectDone
 * @tc.desc: OnAbilityDisconnectDone container is nullptr.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityDisconnectDone_0100, Function | MediumTest | Level1)
{
    sptr<CallerConnection> Connection = new CallerConnection();
    constexpr int32_t code = 0;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    Connection->container_ = nullptr;
    Connection->OnAbilityDisconnectDone(elementName, code);
    EXPECT_EQ(Connection->container_, nullptr);
}

/**
 * @tc.number: Local_Call_Container_OnAbilityDisconnectDone_0200
 * @tc.name: OnAbilityDisconnectDone
 * @tc.desc: OnAbilityDisconnectDone container is not nullptr.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnAbilityDisconnectDone_0200, Function | MediumTest | Level1)
{
    sptr<CallerConnection> Connection = new CallerConnection();
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();
    constexpr int32_t code = 0;
    Connection->container_ = localCallContainer;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    Connection->OnAbilityDisconnectDone(elementName, code);
    EXPECT_NE(Connection->container_, nullptr);
}

/**
 * @tc.number: Local_Call_Container_DumpCalls_0100
 * @tc.name: DumpCalls
 * @tc.desc: Local Call Container to process DumpCalls, and the result is success(call back is called).
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_DumpCalls_0100, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->isCallBack_ = true;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);

    std::string uri = elementName.GetURI();
    localCallContainer.callProxyRecords_.emplace(uri, localCallRecord);

    std::vector<std::string> info;
    std::string resultReceiver = "";
    EXPECT_TRUE(resultReceiver == "");
    localCallContainer.DumpCalls(info);

    for (auto it : info) {
        GTEST_LOG_(ERROR) << it;
        resultReceiver += it + "\n";
    }
    string::size_type idx;
    idx = resultReceiver.find("REQUESTEND");
    EXPECT_TRUE(idx != string::npos);
}

/**
 * @tc.number: Local_Call_Container_DumpCalls_0200
 * @tc.name: DumpCalls
 * @tc.desc: Local Call Container to process DumpCalls, and the result is success(call back is not called).
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_DumpCalls_0200, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);

    std::string uri = elementName.GetURI();
    localCallContainer.callProxyRecords_.emplace(uri, localCallRecord);

    std::vector<std::string> info;
    std::string resultReceiver = "";
    std::string::size_type idx;
    idx = resultReceiver.find("REQUESTING");
    EXPECT_FALSE(idx != string::npos);
    localCallContainer.DumpCalls(info);

    for (auto it : info) {
        resultReceiver += it + "\n";
    }
    idx = resultReceiver.find("REQUESTING");
    EXPECT_TRUE(idx != string::npos);
}

/**
 * @tc.number: Local_Call_Container_DumpCalls_0300
 * @tc.name: DumpCalls
 * @tc.desc: Local Call Container to process DumpCalls, and the result is success(no caller).
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_DumpCalls_0300, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);

    std::string uri = elementName.GetURI();
    localCallContainer.callProxyRecords_.emplace(uri, localCallRecord);

    std::vector<std::string> info;
    std::string resultReceiver = "";
    std::string::size_type idx;
    idx = resultReceiver.find("REQUESTEND");
    EXPECT_FALSE(idx != string::npos);
    localCallContainer.DumpCalls(info);

    for (auto it : info) {
        resultReceiver += it + "\n";
    }

    idx = resultReceiver.find("REQUESTEND");
    EXPECT_TRUE(idx != string::npos);
}

/**
 * @tc.number: Local_Call_Container_DumpCalls_0400
 * @tc.name: DumpCalls
 * @tc.desc: Local Call Container to process DumpCalls, and the result is success(callProxyRecords_ is empty).
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_DumpCalls_0400, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::vector<std::string> info;
    std::string resultReceiver = "";
    EXPECT_TRUE(resultReceiver == "");
    localCallContainer.DumpCalls(info);
    EXPECT_TRUE(resultReceiver == "");
}

/**
 * @tc.number: Local_Call_Container_OnRemoteStateChanged_0100
 * @tc.name: OnRemoteStateChanged
 * @tc.desc: Local Call Container to process OnRemoteStateChanged, and the result is success resultCode == ERR_OK.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnRemoteStateChanged_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    sptr<CallerConnection> Connection = new CallerConnection();
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();
    Connection->container_ = localCallContainer;
    constexpr int32_t code = 1;
    Connection->localCallRecord_ = localCallRecord;
    Connection->OnAbilityConnectDone(elementName, localCallContainer, code);

    localCallContainer->OnRemoteStateChanged(elementName, 0);
    EXPECT_EQ(callback->isCallBack_, true);
}

/**
 * @tc.number: Local_Call_Container_OnRemoteStateChanged_0200
 * @tc.name: OnRemoteStateChanged
 * @tc.desc: Local Call Container to process OnRemoteStateChanged success when resultCode != ERR_OK.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnRemoteStateChanged_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    sptr<CallerConnection> Connection = new CallerConnection();
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();
    Connection->container_ = localCallContainer;
    constexpr int32_t code = 1;
    Connection->localCallRecord_ = localCallRecord;
    Connection->OnAbilityConnectDone(elementName, localCallContainer, code);

    localCallContainer->OnRemoteStateChanged(elementName, -1);
    EXPECT_EQ(callback->isCallBack_, true);
}

/**
 * @tc.number: Local_Call_Container_OnRemoteStateChanged_0300
 * @tc.name: OnRemoteStateChanged
 * @tc.desc: Local Call Container to process OnRemoteStateChanged fail because localCallRecord is null.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnRemoteStateChanged_0300, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    EXPECT_EQ(callback->isCallBack_, false);
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);

    localCallContainer.OnRemoteStateChanged(elementName, 0);
    EXPECT_EQ(callback->isCallBack_, false);
}

/**
 * @tc.number: Local_Call_Container_GetCallLocalRecord_0100
 * @tc.name: GetCallLocalRecord
 * @tc.desc: Local Call Container to process GetCallLocalRecord, and the result is success.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_GetCallLocalRecord_0100, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);

    std::string uri = elementName.GetURI();
    localCallContainer.callProxyRecords_.emplace(uri, localCallRecord);

    bool ret = localCallContainer.GetCallLocalRecord(elementName, localCallRecord);
    EXPECT_TRUE(ret == true);
}

/**
 * @tc.number: Local_Call_Container_GetCallLocalRecord_0200
 * @tc.name: GetCallLocalRecord
 * @tc.desc: Local Call Container to process GetCallLocalRecord, and the result is fail
 *           because call proxy records is empty.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_GetCallLocalRecord_0200, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);

    bool ret = localCallContainer.GetCallLocalRecord(elementName, localCallRecord);
    EXPECT_TRUE(ret == false);
}

/**
 * @tc.number: Local_Call_Container_OnCallStubDied_0100
 * @tc.name: OnCallStubDied
 * @tc.desc: Verify that function IsSameObject.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnCallStubDied_0100, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;

    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);

    sptr<IRemoteObject> remoteObject = new (std::nothrow) MockServiceAbilityManagerService();
    wptr<IRemoteObject> remote(remoteObject);

    localCallContainer.OnCallStubDied(remote);
    EXPECT_TRUE(localCallContainer.callProxyRecords_.empty());
}

/**
 * @tc.number: Local_Call_Container_OnCallStubDied_0200
 * @tc.name: OnCallStubDied
 * @tc.desc: Delete records with empty remote in single instance.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnCallStubDied_0200, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    sptr<IRemoteObject> remoteObject = new (std::nothrow) MockServiceAbilityManagerService();
    wptr<IRemoteObject> remote(remoteObject);
    const std::string strKey = elementName.GetURI();
    localCallRecord->SetRemoteObject(remoteObject);
    localCallContainer.callProxyRecords_.emplace(strKey, localCallRecord);
    localCallContainer.OnCallStubDied(remote);
    EXPECT_TRUE(localCallContainer.callProxyRecords_.empty());
}

/**
 * @tc.number: Local_Call_Container_OnCallStubDied_0300
 * @tc.name: OnCallStubDied
 * @tc.desc: Delete the record of the specified remote in multiple instances.
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnCallStubDied_0300, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    AppExecFwk::ElementName elementNameSec("DemoDeviceIdSec", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecordSec = std::make_shared<LocalCallRecord>(elementNameSec);
    localCallContainer.SetMultipleCallLocalRecord(elementName, localCallRecord);
    localCallContainer.SetMultipleCallLocalRecord(elementNameSec, localCallRecordSec);
    sptr<IRemoteObject> remoteObject = new (std::nothrow) MockServiceAbilityManagerService();
    wptr<IRemoteObject> remote(remoteObject);
    localCallRecordSec->SetRemoteObject(remoteObject);
    localCallContainer.OnCallStubDied(remote);
    EXPECT_FALSE(localCallContainer.multipleCallProxyRecords_.empty());
}

/**
 * @tc.number: Local_Call_Container_OnCallStubDied_0400
 * @tc.name: OnCallStubDied
 * @tc.desc: Delete the specified remote in multiple instances
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_OnCallStubDied_0400, Function | MediumTest | Level1)
{
    LocalCallContainer localCallContainer;
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    std::shared_ptr<LocalCallRecord> localCallRecordSec = std::make_shared<LocalCallRecord>(elementName);
    localCallContainer.SetMultipleCallLocalRecord(elementName, localCallRecord);
    localCallContainer.SetMultipleCallLocalRecord(elementName, localCallRecordSec);
    sptr<IRemoteObject> remoteObject = new (std::nothrow) MockServiceAbilityManagerService();
    wptr<IRemoteObject> remote(remoteObject);
    localCallRecordSec->SetRemoteObject(remoteObject);
    localCallContainer.OnCallStubDied(remote);
    EXPECT_FALSE(localCallContainer.multipleCallProxyRecords_.empty());
}

/**
 * @tc.number: Local_Call_Container_ClearFailedCallConnection_0400
 * @tc.name: ClearFailedCallConnection
 * @tc.desc: clear failed call connection execute normally
 */
HWTEST_F(LocalCallContainerTest, Local_Call_Container_ClearFailedCallConnection_0400, Function | MediumTest | Level1)
{
    constexpr int32_t COUNT_ZERO = 0;
    sptr<LocalCallContainer> localCallContainer = new (std::nothrow)LocalCallContainer();
    localCallContainer->ClearFailedCallConnection(nullptr);
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    localCallContainer->ClearFailedCallConnection(callback);
    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    localCallContainer->ClearFailedCallConnection(callback);
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    connect->SetRecordAndContainer(localCallRecord, localCallContainer);
    localCallContainer->connections_.emplace(connect);
    EXPECT_NE(COUNT_ZERO, localCallContainer->connections_.size());
    localCallContainer->ClearFailedCallConnection(callback);
    EXPECT_EQ(COUNT_ZERO, localCallContainer->connections_.size());
}
} // namespace AppExecFwk
} // namespace OHOS
