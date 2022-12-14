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

#include <chrono>
#include <thread>
#include <gtest/gtest.h>

#define private public
#define protected public
#include "message_parcel.h"
#include "connect_callback_proxy.h"
#include "connect_callback_stub.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace testing;
using namespace std::chrono;

namespace OHOS {
namespace AppExecFwk {
class ConnectCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ConnectCallbackProxyTest::SetUpTestCase(void)
{}

void ConnectCallbackProxyTest::TearDownTestCase(void)
{}

void ConnectCallbackProxyTest::SetUp(void)
{}

void ConnectCallbackProxyTest::TearDown(void)
{}

class MockConnectCallback : public ConnectCallbackStub {
public:
    MockConnectCallback() {};
    ~MockConnectCallback() {};

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        GTEST_LOG_(INFO) << "MockRegisterService::SendRequest called. return value " << returnCode_;
        reply.WriteInt32(ERR_NONE);
        flag = true;
        return returnCode_;
    }

    void Connect(const string &deviceId, const string &deviceType) override
    {
        GTEST_LOG_(INFO) << "MockRegisterService::Connect called.";
        return;
    }

    void Disconnect(const string &deviceId) override
    {
        GTEST_LOG_(INFO) << "MockRegisterService::Connect called.";
        return;
    }

    int32_t returnCode_ = ERR_NONE;
    bool flag = false;
};

/**
 * @tc.number: AppExecFwk_ConnectCallbackProxy_Connect_001
 * @tc.name: Connect
 * @tc.desc: Pass in normal parameters, and the test program executes correctly without abnormal exit
 */
HWTEST_F(ConnectCallbackProxyTest, AppExecFwk_ConnectCallbackProxy_Connect_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackProxy_Connect_001 start.";
    sptr<MockConnectCallback> object = new (std::nothrow) MockConnectCallback();
    sptr<ConnectCallbackProxy> testProxy = new (std::nothrow) ConnectCallbackProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    EXPECT_TRUE(testProxy->remoteObject_ != nullptr);
    std::string deviceId = "7001005458323933328a592135733900";
    std::string deviceType = "rk3568";
    EXPECT_FALSE(object->flag);
    testProxy->Connect(deviceId, deviceType);
    EXPECT_TRUE(object->flag);
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackProxy_Connect_001 end.";
}

/**
 * @tc.number: AppExecFwk_ConnectCallbackProxy_Disconnect_001
 * @tc.name: Disconnect
 * @tc.desc: Pass in normal parameters, and the test program executes correctly without abnormal exit
 */
HWTEST_F(ConnectCallbackProxyTest, AppExecFwk_ConnectCallbackProxy_Disconnect_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackProxy_Disconnect_001 start.";
    sptr<MockConnectCallback> object = new (std::nothrow) MockConnectCallback();
    sptr<ConnectCallbackProxy> testProxy = new (std::nothrow) ConnectCallbackProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    EXPECT_TRUE(testProxy->remoteObject_ != nullptr);
    std::string deviceId = "7001005458323933328a592135733900";
    EXPECT_FALSE(object->flag);
    testProxy->Disconnect(deviceId);
    EXPECT_TRUE(object->flag);
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackProxy_Disconnect_001 end.";
}

/**
 * @tc.number: AppExecFwk_ConnectCallbackProxy_RemoteRequest_001
 * @tc.name: RemoteRequest
 * @tc.desc: Pass in normal parameters, and the test program executes correctly without abnormal exit
 */
HWTEST_F(ConnectCallbackProxyTest, AppExecFwk_ConnectCallbackProxy_RemoteRequest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackProxy_RemoteRequest_001 start.";
    sptr<MockConnectCallback> object = new (std::nothrow) MockConnectCallback();
    sptr<ConnectCallbackProxy> testProxy = new (std::nothrow) ConnectCallbackProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    EXPECT_TRUE(testProxy->remoteObject_ != nullptr);
    int commandDisconnect = 1;
    MessageParcel data = {};
    EXPECT_FALSE(object->flag);
    testProxy->RemoteRequest(data, commandDisconnect);
    EXPECT_TRUE(object->flag);
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackProxy_RemoteRequest_001 end.";
}

/**
 * @tc.number: AppExecFwk_ConnectCallbackProxy_RemoteRequest_002
 * @tc.name: RemoteRequest
 * @tc.desc: The incoming Remote() is nullptr, and the test program executes as expected without exiting abnormally
 */
HWTEST_F(ConnectCallbackProxyTest, AppExecFwk_ConnectCallbackProxy_RemoteRequest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackProxy_RemoteRequest_002 start.";
    sptr<MockConnectCallback> object = nullptr;
    sptr<ConnectCallbackProxy> testProxy = new (std::nothrow) ConnectCallbackProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    int commandDisconnect = 1;
    MessageParcel data = {};
    testProxy->RemoteRequest(data, commandDisconnect);
    EXPECT_TRUE(testProxy->remoteObject_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackProxy_RemoteRequest_002 end.";
}
}   // namespace AppExecFwk
}   // namespace OHOS