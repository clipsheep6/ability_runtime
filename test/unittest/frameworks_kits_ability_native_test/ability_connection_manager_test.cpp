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
#include <iremote_stub.h>
#define private public
#define protected public
#include "ability_connect_callback.h"
#include "ability_connection.h"
#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "connection_manager.h"
#include "hilog_wrapper.h"
#undef protected
#undef private

using namespace testing::ext;
using OHOS::AppExecFwk::ElementName;
namespace OHOS {
namespace AbilityRuntime {

class ConnectionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

class MockAbilityConnectCallback : public AbilityConnectCallback {
public:
    MockAbilityConnectCallback() {};
    virtual ~MockAbilityConnectCallback() {};
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode) {}
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) {}
};

void ConnectionManagerTest::SetUpTestCase() {}

void ConnectionManagerTest::TearDownTestCase() {}

void ConnectionManagerTest::SetUp() {}

void ConnectionManagerTest::TearDown() {}

/**
 * @tc.number: ConnectAbilityWithAccount_0100
 * @tc.name: ConnectAbilityWithAccount
 * @tc.desc: ConnectAbilityWithAccount Test, return RESOLVE_ABILITY_ERR.
 */
HWTEST_F(ConnectionManagerTest, ConnectAbilityWithAccount_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbilityWithAccount_0100 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    AAFwk::Want want;
    int32_t accountId = -1;
    sptr<IRemoteObject> connectCaller = new (std::nothrow) AbilityConnection();
    sptr<AbilityConnectCallback> connectCallback = new (std::nothrow) MockAbilityConnectCallback();
    auto result = mgr->ConnectAbilityWithAccount(connectCaller, want, accountId, connectCallback);
    EXPECT_EQ(result, AAFwk::RESOLVE_ABILITY_ERR);
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbilityWithAccount_0100 end";
}

/**
 * @tc.number: ConnectAbility_0100
 * @tc.name: ConnectAbility
 * @tc.desc: ConnectAbility Test, return RESOLVE_ABILITY_ERR.
 */
HWTEST_F(ConnectionManagerTest, ConnectAbility_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbility_0100 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    AAFwk::Want want;
    sptr<IRemoteObject> connectCaller = new (std::nothrow) AbilityConnection();
    sptr<AbilityConnectCallback> connectCallback = new (std::nothrow) MockAbilityConnectCallback();
    auto result = mgr->ConnectAbility(connectCaller, want, connectCallback);
    EXPECT_EQ(result, AAFwk::RESOLVE_ABILITY_ERR);
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbility_0100 end";
}

/**
 * @tc.number: ConnectAbilityInner_0100
 * @tc.name: ConnectAbilityInner
 * @tc.desc: ConnectAbilityInner Test, connectCaller is nullptr, return ERR_INVALID_CALLER.
 */
HWTEST_F(ConnectionManagerTest, ConnectAbilityInner_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbilityInner_0100 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    AAFwk::Want want;
    int32_t accountId = -1;
    sptr<AbilityConnectCallback> connectCallback = new (std::nothrow) MockAbilityConnectCallback();
    auto result = mgr->ConnectAbilityInner(nullptr, want, accountId, connectCallback);
    EXPECT_EQ(result, AAFwk::ERR_INVALID_CALLER);
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbilityInner_0100 end";
}

/**
 * @tc.number: ConnectAbilityInner_0200
 * @tc.name: ConnectAbilityInner
 * @tc.desc: ConnectAbilityInner Test, connectCallback is nullptr, return ERR_INVALID_CALLER.
 */
HWTEST_F(ConnectionManagerTest, ConnectAbilityInner_0200, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbilityInner_0200 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    sptr<IRemoteObject> connectCaller = new (std::nothrow) AbilityConnection();
    AAFwk::Want want;
    int32_t accountId = -1;
    auto result = mgr->ConnectAbilityInner(connectCaller, want, accountId, nullptr);
    EXPECT_EQ(result, AAFwk::ERR_INVALID_CALLER);
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbilityInner_0200 end";
}

/**
 * @tc.number: ConnectAbilityInner_0300
 * @tc.name: ConnectAbilityInner
 * @tc.desc: ConnectAbilityInner Test, connectCallback is nullptr, connectCaller is nullptr, return ERR_INVALID_CALLER.
 */
HWTEST_F(ConnectionManagerTest, ConnectAbilityInner_0300, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbilityInner_0300 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    AAFwk::Want want;
    int32_t accountId = -1;
    auto result = mgr->ConnectAbilityInner(nullptr, want, accountId, nullptr);
    EXPECT_EQ(result, AAFwk::ERR_INVALID_CALLER);
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbilityInner_0300 end";
}

/**
 * @tc.number: ConnectAbilityInner_0400
 * @tc.name: ConnectAbilityInner
 * @tc.desc: ConnectAbilityInner Test, connectCallback is not nullptr, connectCaller is not nullptr,
 * return RESOLVE_ABILITY_ERR.
 */
HWTEST_F(ConnectionManagerTest, ConnectAbilityInner_0400, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbilityInner_0400 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    sptr<IRemoteObject> connectCaller = new (std::nothrow) AbilityConnection();
    AAFwk::Want want;
    int32_t accountId = -1;
    sptr<AbilityConnectCallback> connectCallback = new (std::nothrow) MockAbilityConnectCallback();
    auto result = mgr->ConnectAbilityInner(connectCaller, want, accountId, connectCallback);
    EXPECT_EQ(result, AAFwk::RESOLVE_ABILITY_ERR);
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbilityInner_0400 end";
}

/**
 * @tc.number: ConnectAbilityInner_0500
 * @tc.name: ConnectAbilityInner
 * @tc.desc: ConnectAbilityInner Test, return INVALID_CONNECTION_STATE.
 */
HWTEST_F(ConnectionManagerTest, ConnectAbilityInner_0500, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbilityInner_0500 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    sptr<IRemoteObject> connectCaller = new (std::nothrow) AbilityConnection();
    sptr<AbilityConnectCallback> connectCallback = new (std::nothrow) MockAbilityConnectCallback();
    AppExecFwk::ElementName connectReceiver;
    AAFwk::Want want;
    want.SetElementName("abc", "edf");
    int32_t accountId = -1;
    connectReceiver.SetBundleName("abc");
    connectReceiver.SetAbilityName("edf");
    std::vector<sptr<AbilityConnectCallback>> callbacks;
    callbacks.push_back(connectCallback);
    sptr<AbilityConnection> abilityConnection = new (std::nothrow) AbilityConnection();
    ConnectionInfo connectionInfo(connectCaller, connectReceiver, abilityConnection);
    connectionInfo.connectReceiver.SetBundleName("abc");
    connectionInfo.connectReceiver.SetAbilityName("edf");
    connectionInfo.connectCaller = connectCaller;
    mgr->abilityConnections_.emplace(connectionInfo, callbacks);
    auto result = mgr->ConnectAbilityInner(connectCaller, want, accountId, connectCallback);
    EXPECT_EQ(result, AAFwk::INVALID_CONNECTION_STATE);
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbilityInner_0500 end";
}

/**
 * @tc.number: ConnectAbilityInner_0600
 * @tc.name: ConnectAbilityInner
 * @tc.desc: ConnectAbilityInner Test, return ERR_OK.
 */
HWTEST_F(ConnectionManagerTest, ConnectAbilityInner_0600, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbilityInner_0600 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    sptr<IRemoteObject> connectCaller = new (std::nothrow) AbilityConnection();
    sptr<AbilityConnectCallback> connectCallback = new (std::nothrow) MockAbilityConnectCallback();
    AppExecFwk::ElementName connectReceiver;
    AAFwk::Want want;
    want.SetElementName("abc", "edf");
    int32_t accountId = -1;
    int32_t connectionState = 1;
    connectReceiver.SetBundleName("abc");
    connectReceiver.SetAbilityName("edf");
    std::vector<sptr<AbilityConnectCallback>> callbacks;
    callbacks.push_back(connectCallback);
    sptr<AbilityConnection> abilityConnection = new (std::nothrow) AbilityConnection();
    ConnectionInfo connectionInfo(connectCaller, connectReceiver, abilityConnection);
    connectionInfo.connectReceiver.SetBundleName("abc");
    connectionInfo.connectReceiver.SetAbilityName("edf");
    connectionInfo.connectCaller = connectCaller;
    mgr->abilityConnections_.emplace(connectionInfo, callbacks);
    abilityConnection->SetConnectionState(connectionState);
    auto result = mgr->ConnectAbilityInner(connectCaller, want, accountId, connectCallback);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "ConnectionManagerTest ConnectAbilityInner_0600 end";
}

/**
 * @tc.number: IsConnectCallerEqual_0100
 * @tc.name: IsConnectCallerEqual
 * @tc.desc: IsConnectCallerEqual Test, return false.
 */
HWTEST_F(ConnectionManagerTest, IsConnectCallerEqual_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest IsConnectCallerEqual_0100 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    sptr<IRemoteObject> connectCaller = new (std::nothrow) AbilityConnection();
    sptr<IRemoteObject> connectCallerOther = new (std::nothrow) AbilityConnection();
    auto result = mgr->IsConnectCallerEqual(connectCaller, connectCallerOther);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "ConnectionManagerTest IsConnectCallerEqual_0100 end";
}

/**
 * @tc.number: IsConnectReceiverEqual_0100
 * @tc.name: IsConnectReceiverEqual
 * @tc.desc: IsConnectReceiverEqual Test, return true.
 */
HWTEST_F(ConnectionManagerTest, IsConnectReceiverEqual_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest IsConnectReceiverEqual_0100 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    AppExecFwk::ElementName connectReceiver("deviceId", "bundleName", "abilityName", "");
    AppExecFwk::ElementName connectReceiverOther("deviceId", "bundleName", "abilityName", "");
    auto result = mgr->IsConnectReceiverEqual(connectReceiver, connectReceiverOther);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "ConnectionManagerTest IsConnectReceiverEqual_0100 end";
}

/**
 * @tc.number: CreateConnection_0100
 * @tc.name: CreateConnection
 * @tc.desc: CreateConnection Test, return CHECK_PERMISSION_FAILED.
 * return ERR_INVALID_VALUE.
 */
HWTEST_F(ConnectionManagerTest, CreateConnection_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest CreateConnection_0100 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    sptr<IRemoteObject> connectCallernew = new (std::nothrow) AbilityConnection();
    AAFwk::Want want;
    int32_t accountId = 0;
    sptr<AbilityConnectCallback> connectCallback = new (std::nothrow) MockAbilityConnectCallback();
    AppExecFwk::ElementName connectReceiver;
    auto result = mgr->CreateConnection(connectCallernew, want, accountId, connectCallback, connectReceiver);
    EXPECT_EQ(result, AAFwk::CHECK_PERMISSION_FAILED);
    GTEST_LOG_(INFO) << "ConnectionManagerTest CreateConnection_0100 end";
}

/**
 * @tc.number: DisconnectAbility_0100
 * @tc.name: DisconnectAbility
 * @tc.desc: DisconnectAbility Test, connectCaller is not nullptr, connectCallback is not nullptr,
 * return CONNECTION_NOT_EXIST.
 */
HWTEST_F(ConnectionManagerTest, DisconnectAbility_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectAbility_0100 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    sptr<IRemoteObject> connectCaller = new (std::nothrow) AbilityConnection();
    sptr<AbilityConnectCallback> connectCallback = new (std::nothrow) MockAbilityConnectCallback();
    AppExecFwk::ElementName connectReceiver;
    auto result = mgr->DisconnectAbility(connectCaller, connectReceiver, connectCallback);
    EXPECT_EQ(result, AAFwk::CONNECTION_NOT_EXIST);
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectAbility_0100 end";
}

/**
 * @tc.number: DisconnectAbility_0200
 * @tc.name: DisconnectAbility
 * @tc.desc: DisconnectAbility Test, connectCallback is nullptr, return ERR_INVALID_CALLER.
 */
HWTEST_F(ConnectionManagerTest, DisconnectAbility_0200, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectAbility_0200 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    sptr<IRemoteObject> connectCaller = new (std::nothrow) AbilityConnection();
    AppExecFwk::ElementName connectReceiver;
    auto result = mgr->DisconnectAbility(connectCaller, connectReceiver, nullptr);
    EXPECT_EQ(result, AAFwk::ERR_INVALID_CALLER);
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectAbility_0200 end";
}

/**
 * @tc.number: DisconnectAbility_0300
 * @tc.name: DisconnectAbility
 * @tc.desc: DisconnectAbility Test, connectCaller is nullptr, return ERR_INVALID_CALLER.
 */
HWTEST_F(ConnectionManagerTest, DisconnectAbility_0300, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectAbility_0300 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    AppExecFwk::ElementName connectReceiver;
    sptr<AbilityConnectCallback> connectCallback = new (std::nothrow) MockAbilityConnectCallback();
    auto result = mgr->DisconnectAbility(nullptr, connectReceiver, connectCallback);
    EXPECT_EQ(result, AAFwk::ERR_INVALID_CALLER);
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectAbility_0300 end";
}

/**
 * @tc.number: DisconnectAbility_0400
 * @tc.name: DisconnectAbility
 * @tc.desc: DisconnectAbility Test, connectCallback is nullptr, connectCaller is nullptr,
 * return ERR_INVALID_CALLER.
 */
HWTEST_F(ConnectionManagerTest, DisconnectAbility_0400, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectAbility_0400 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    AppExecFwk::ElementName connectReceiver;
    auto result = mgr->DisconnectAbility(nullptr, connectReceiver, nullptr);
    EXPECT_EQ(result, AAFwk::ERR_INVALID_CALLER);
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectAbility_0400 end";
}

/**
 * @tc.number: DisconnectAbility_0500
 * @tc.name: DisconnectAbility
 * @tc.desc: DisconnectAbility Test, return ERR_OK.
 */
HWTEST_F(ConnectionManagerTest, DisconnectAbility_0500, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectAbility_0500 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    sptr<IRemoteObject> connectCaller = new (std::nothrow) AbilityConnection();
    sptr<AbilityConnectCallback> connectCallback = new (std::nothrow) MockAbilityConnectCallback();
    AppExecFwk::ElementName connectReceiver;
    connectReceiver.SetBundleName("abc");
    connectReceiver.SetAbilityName("edf");
    std::vector<sptr<AbilityConnectCallback>> callbacks;
    callbacks.emplace_back(connectCallback);
    sptr<AbilityConnection> abilityConnection = new (std::nothrow) AbilityConnection();
    ConnectionInfo connectionInfo(connectCaller, connectReceiver, abilityConnection);
    connectionInfo.connectReceiver.SetBundleName("abc");
    connectionInfo.connectReceiver.SetAbilityName("edf");
    connectionInfo.connectCaller = connectCaller;
    mgr->abilityConnections_.emplace(connectionInfo, callbacks);
    auto result = mgr->DisconnectAbility(connectCaller, connectReceiver, connectCallback);
    EXPECT_EQ(result, ERR_OK);
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectAbility_0500 end";
}

/**
 * @tc.number: DisconnectCaller_0100
 * @tc.name: DisconnectCaller
 * @tc.desc: DisconnectCaller Test, connectCaller is nullptr, return false.
 */
HWTEST_F(ConnectionManagerTest, DisconnectCaller_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectCaller_0100 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    auto result = mgr->DisconnectCaller(nullptr);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectCaller_0100 end";
}

/**
 * @tc.number: DisconnectCaller_0200
 * @tc.name: DisconnectCaller
 * @tc.desc: DisconnectCaller Test, connectCaller is not nullptr, return false.
 */
HWTEST_F(ConnectionManagerTest, DisconnectCaller_0200, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectCaller_0200 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    sptr<IRemoteObject> connectCaller = new (std::nothrow) AbilityConnection();
    auto result = mgr->DisconnectCaller(connectCaller);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectCaller_0200 end";
}

/**
 * @tc.number: DisconnectCaller_0300
 * @tc.name: DisconnectCaller
 * @tc.desc: DisconnectCaller Test, return true.
 */
HWTEST_F(ConnectionManagerTest, DisconnectCaller_0300, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectCaller_0300 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    sptr<IRemoteObject> connectCaller = new (std::nothrow) AbilityConnection();
    AppExecFwk::ElementName connectReceiver;
    std::vector<sptr<AbilityConnectCallback>> callbacks;
    sptr<AbilityConnection> abilityConnection = new (std::nothrow) AbilityConnection();
    ConnectionInfo connectionInfo(connectCaller, connectReceiver, abilityConnection);
    mgr->abilityConnections_.emplace(connectionInfo, callbacks);
    auto result = mgr->DisconnectCaller(connectCaller);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectCaller_0300 end";
}

/**
 * @tc.number: DisconnectReceiver_0100
 * @tc.name: DisconnectReceiver
 * @tc.desc: DisconnectReceiver Test, return true.
 */
HWTEST_F(ConnectionManagerTest, DisconnectReceiver_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectReceiver_0100 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    AppExecFwk::ElementName connectReceiver;
    sptr<IRemoteObject> connectCaller = new (std::nothrow) AbilityConnection();
    std::vector<sptr<AbilityConnectCallback>> callbacks;
    sptr<AbilityConnection> abilityConnection = new (std::nothrow) AbilityConnection();
    ConnectionInfo connectionInfo(connectCaller, connectReceiver, abilityConnection);
    mgr->abilityConnections_.emplace(connectionInfo, callbacks);
    auto result = mgr->DisconnectReceiver(connectReceiver);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectReceiver_0100 end";
}

/**
 * @tc.number: DisconnectReceiver_0200
 * @tc.name: DisconnectReceiver
 * @tc.desc: DisconnectReceiver Test, connectReceiver is not nullptr, return false.
 */
HWTEST_F(ConnectionManagerTest, DisconnectReceiver_0200, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectReceiver_0200 start";
    std::shared_ptr<OHOS::AbilityRuntime::ConnectionManager> mgr =
        std::make_shared<OHOS::AbilityRuntime::ConnectionManager>();
    AppExecFwk::ElementName connectReceiver;
    auto result = mgr->DisconnectReceiver(connectReceiver);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "ConnectionManagerTest DisconnectReceiver_0200 end";
}
} // namespace AbilityRuntime
} // namespace OHOS