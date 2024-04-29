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
#include "connect_callback_proxy.h"
#include "connect_callback_stub.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace testing;
using namespace std::chrono;

namespace OHOS {
namespace AppExecFwk {
class ConnectCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ConnectCallbackStubTest::SetUpTestCase(void)
{}

void ConnectCallbackStubTest::TearDownTestCase(void)
{}

void ConnectCallbackStubTest::SetUp(void)
{}

void ConnectCallbackStubTest::TearDown(void)
{}

class MockConnectCallback : public ConnectCallbackStub {
public:
    MockConnectCallback() {};
    ~MockConnectCallback() {};

    sptr<IRemoteObject> AsObject() override
    {
        if (!asObject_) {
            return nullptr;
        }
        return this;
    };

    void Connect(const string &deviceId, const string &deviceType) override
    {
        return;
    }
    void Disconnect(const string &deviceId) override
    {
        return;
    }

    int32_t returnCode_ = ERR_NONE;
    bool asObject_ = true;
};

/**
 * @tc.number: AppExecFwk_ConnectCallbackStub_ConnectInner_001
 * @tc.name: ConnectInner
 * @tc.desc: Pass in normal parameters, and the test program executes correctly without abnormal exit
 */
HWTEST_F(ConnectCallbackStubTest, AppExecFwk_ConnectCallbackStub_ConnectInner_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackStub_ConnectInner_001 start.";
    sptr<MockConnectCallback> object = new (std::nothrow) MockConnectCallback();
    EXPECT_TRUE(object != nullptr);
    MessageParcel data = {};
    MessageParcel reply = {};
    EXPECT_EQ(object->ConnectInner(data, reply), OHOS::ERR_NONE);
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackStub_ConnectInner_001 end.";
}

/**
 * @tc.number: AppExecFwk_ConnectCallbackStub_DisconnectInner_001
 * @tc.name: DisconnectInner
 * @tc.desc: Pass in normal parameters, and the test program executes correctly without abnormal exit
 */
HWTEST_F(ConnectCallbackStubTest, AppExecFwk_ConnectCallbackStub_DisconnectInner_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackStub_DisconnectInner_001 start.";
    sptr<MockConnectCallback> object = new (std::nothrow) MockConnectCallback();
    EXPECT_TRUE(object != nullptr);
    MessageParcel data = {};
    MessageParcel reply = {};
    EXPECT_EQ(object->DisconnectInner(data, reply), OHOS::ERR_NONE);
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackStub_DisconnectInner_001 end.";
}

/**
 * @tc.number: AppExecFwk_ConnectCallbackStub_OnRemoteRequest_001
 * @tc.name: OnRemoteRequest
 * @tc.desc: Pass in normal parameters, and the test program executes correctly without abnormal exit
 */
HWTEST_F(ConnectCallbackStubTest, AppExecFwk_ConnectCallbackStub_OnRemoteRequest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackStub_OnRemoteRequest_001 start.";
    sptr<MockConnectCallback> object = new (std::nothrow) MockConnectCallback();
    EXPECT_TRUE(object != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_TRUE(data.WriteInterfaceToken(u"ohos.appexecfwk.iconnectcallback"));
    EXPECT_EQ(object->OnRemoteRequest(MockConnectCallback::COMMAND_CONNECT, data, reply, option), OHOS::ERR_NONE);
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackStub_OnRemoteRequest_001 end.";
}

/**
 * @tc.number: AppExecFwk_ConnectCallbackStub_OnRemoteRequest_002
 * @tc.name: OnRemoteRequest
 * @tc.desc: Pass in normal parameters, and the test program executes correctly without abnormal exit
 */
HWTEST_F(ConnectCallbackStubTest, AppExecFwk_ConnectCallbackStub_OnRemoteRequest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackStub_OnRemoteRequest_002 start.";
    sptr<MockConnectCallback> object = new (std::nothrow) MockConnectCallback();
    EXPECT_TRUE(object != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_TRUE(data.WriteInterfaceToken(u"ohos.appexecfwk.iconnectcallback"));
    EXPECT_EQ(object->OnRemoteRequest(MockConnectCallback::COMMAND_DISCONNECT, data, reply, option), OHOS::ERR_NONE);
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackStub_OnRemoteRequest_002 end.";
}

/**
 * @tc.number: AppExecFwk_ConnectCallbackStub_OnRemoteRequest_003
 * @tc.name: OnRemoteRequest
 * @tc.desc: The passed in parameter toukenString is an abnormal value. The test program executes as
 * expected and does not exit abnormally
 */
HWTEST_F(ConnectCallbackStubTest, AppExecFwk_ConnectCallbackStub_OnRemoteRequest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackStub_OnRemoteRequest_003 start.";
    sptr<MockConnectCallback> object = new (std::nothrow) MockConnectCallback();
    EXPECT_TRUE(object != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_TRUE(data.WriteInterfaceToken(u"123"));
    EXPECT_EQ(object->OnRemoteRequest(MockConnectCallback::COMMAND_CONNECT, data, reply, option),
    OHOS::ERR_INVALID_REPLY);
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackStub_OnRemoteRequest_003 end.";
}

/**
 * @tc.number: AppExecFwk_ConnectCallbackStub_OnRemoteRequest_004
 * @tc.name: OnRemoteRequest
 * @tc.desc: The input parameter code is an abnormal value, and the test program executes as expected
 * without exiting abnormally
 */
HWTEST_F(ConnectCallbackStubTest, AppExecFwk_ConnectCallbackStub_OnRemoteRequest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackStub_OnRemoteRequest_004 start.";
    sptr<MockConnectCallback> object = new (std::nothrow) MockConnectCallback();
    EXPECT_TRUE(object != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_TRUE(data.WriteInterfaceToken(u"ohos.appexecfwk.iconnectcallback"));
    EXPECT_EQ(object->OnRemoteRequest(MockConnectCallback::COMMAND_DISCONNECT + 66, data, reply, option),
    OHOS::IPC_STUB_UNKNOW_TRANS_ERR);
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackStub_OnRemoteRequest_004 end.";
}

/**
 * @tc.number: AppExecFwk_ConnectCallbackStub_OnRemoteRequest_005
 * @tc.name: OnRemoteRequest
 * @tc.desc: The input parameter code is null ptr, and the test program executes as expected without exception
 */
HWTEST_F(ConnectCallbackStubTest, AppExecFwk_ConnectCallbackStub_OnRemoteRequest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackStub_OnRemoteRequest_005 start.";
    sptr<MockConnectCallback> object = new (std::nothrow) MockConnectCallback();
    EXPECT_TRUE(object != nullptr);
    MessageParcel data = {};
    MessageParcel reply = {};
    MessageOption option = {};
    EXPECT_TRUE(data.WriteInterfaceToken(u"ohos.appexecfwk.iconnectcallback"));
    object->memberFuncMap_[MockConnectCallback::COMMAND_DISCONNECT + 1] = nullptr;
    EXPECT_EQ(object->OnRemoteRequest(MockConnectCallback::COMMAND_DISCONNECT + 1, data, reply, option),
    IPC_STUB_UNKNOW_TRANS_ERR);
    GTEST_LOG_(INFO) << "AppExecFwk_ConnectCallbackStub_OnRemoteRequest_005 end.";
}
}   // namespace AppExecFwk
}   // namespace OHOS