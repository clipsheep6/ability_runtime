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
#include "remote_register_service_stub.h"
#include "connect_callback_stub.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace testing;
using namespace std::chrono;

namespace OHOS {
namespace AppExecFwk {
class RemoteRegisterServiceStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:

};

void RemoteRegisterServiceStubTest::SetUpTestCase(void)
{}
void RemoteRegisterServiceStubTest::TearDownTestCase(void)
{}

void RemoteRegisterServiceStubTest::SetUp(void)
{}

void RemoteRegisterServiceStubTest::TearDown(void)
{}

class MockRegisterService : public RemoteRegisterServiceStub {
public:
    MockRegisterService() : RemoteRegisterServiceStub() {};
    ~MockRegisterService() {};

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        GTEST_LOG_(INFO) << "MockRegisterService::SendRequest called. return value " << returnCode_;
        reply.WriteInt32(ERR_NONE);
        return returnCode_;
    }

    int Register(const std::string &bundleName, const sptr<IRemoteObject> &token, const ExtraParams &extras,
        const sptr<IConnectCallback> &callback) override
    {
        return 0;
    };
    bool Unregister(int registerToken) override
    {
        return true;
    };
    bool UpdateConnectStatus(int registerToken, const std::string &deviceId, int status) override
    {
        return true;
    };
    bool ShowDeviceList(int registerToken, const ExtraParams &extras) override
    {
        return true;
    };

    int32_t returnCode_ = ERR_NONE;
    int32_t register_ = ERR_NONE;
    bool unregister_ = true;
    bool updateConnectStatus_ = true;
    bool showDeviceList_ = true;
};

class MoclConnectCallback : public ConnectCallbackStub {
public:
    MoclConnectCallback() {};
    ~MoclConnectCallback() {};

    sptr<IRemoteObject> AsObject() override
    {
        if (!asObject_) {
            return nullptr;
        }

        return this;
    };

    virtual void Connect(const string &deviceId, const string &deviceType) override {};
    virtual void Disconnect(const string &deviceId) override {};

    bool asObject_ = true;
};

/*
 * Feature: AbilityManager
 * Function: RemoteRegisterServiceStub
 * SubFunction: OnRemoteRequest
 * FunctionPoints: The parameter of function OnRemoteRequest.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function OnRemoteRequest register parameter data
 */
HWTEST_F(RemoteRegisterServiceStubTest, AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_001 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    std::string bundleName = "ABC";
    sptr<IRemoteObject> token = object;
    ExtraParams extras = {};
    sptr<MoclConnectCallback> callback = new (std::nothrow) MoclConnectCallback();

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    EXPECT_TRUE(data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()));
    EXPECT_TRUE(data.WriteString(bundleName));
    EXPECT_TRUE(data.WriteRemoteObject(token));
    EXPECT_TRUE(data.WriteInt32(1));
    EXPECT_TRUE(extras.Marshalling(data));
    EXPECT_TRUE(data.WriteRemoteObject(callback->AsObject()));

    EXPECT_EQ(object->OnRemoteRequest(MockRegisterService::COMMAND_REGISTER, data, reply, option), NO_ERROR);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_001 end.";
}

/*
 * Feature: AbilityManager
 * Function: RemoteRegisterServiceStub
 * SubFunction: OnRemoteRequest
 * FunctionPoints: The parameter of function OnRemoteRequest.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function OnRemoteRequest unregister parameter data
 */
HWTEST_F(RemoteRegisterServiceStubTest, AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_002 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);

    constexpr int32_t registerToken = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    EXPECT_TRUE(data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()));
    EXPECT_TRUE(data.WriteInt32(registerToken));

    EXPECT_EQ(object->OnRemoteRequest(MockRegisterService::COMMAND_UNREGISTER, data, reply, option), NO_ERROR);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_002 end.";
}


/*
 * Feature: AbilityManager
 * Function: RemoteRegisterServiceStub
 * SubFunction: OnRemoteRequest
 * FunctionPoints: The parameter of function OnRemoteRequest.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function OnRemoteRequest update connect status parameter data
 */
HWTEST_F(RemoteRegisterServiceStubTest, AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_003 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);

    constexpr int32_t registerToken = 0;
    constexpr int32_t status = 0;
    const std::string deviceId = "7001005458323933328a592135733900";
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    EXPECT_TRUE(data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()));
    EXPECT_TRUE(data.WriteInt32(registerToken));
    EXPECT_TRUE(data.WriteString(deviceId));
    EXPECT_TRUE(data.WriteInt32(status));

    EXPECT_EQ(
        object->OnRemoteRequest(MockRegisterService::COMMAND_UPDATE_CONNECT_STATUS, data, reply, option), NO_ERROR);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_003 end.";
}

/*
 * Feature: AbilityManager
 * Function: RemoteRegisterServiceStub
 * SubFunction: OnRemoteRequest
 * FunctionPoints: The parameter of function OnRemoteRequest.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function OnRemoteRequest show device list parameter data
 */
HWTEST_F(RemoteRegisterServiceStubTest, AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_004 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);

    constexpr int32_t registerToken = 0;
    ExtraParams extras = {};
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    EXPECT_TRUE(data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()));
    EXPECT_TRUE(data.WriteInt32(registerToken));
    EXPECT_TRUE(data.WriteInt32(1));

    EXPECT_EQ(object->OnRemoteRequest(MockRegisterService::COMMAND_SHOW_DEVICE_LIST, data, reply, option), NO_ERROR);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_004 end.";
}

/*
 * Feature: AbilityManager
 * Function: RemoteRegisterServiceStub
 * SubFunction: OnRemoteRequest
 * FunctionPoints: The parameter of function OnRemoteRequest.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function OnRemoteRequest parameter error touken string
 */
HWTEST_F(RemoteRegisterServiceStubTest, AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_005 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    std::string bundleName = "ABC";
    sptr<IRemoteObject> token = object;
    ExtraParams extras = {};
    sptr<MoclConnectCallback> callback = new (std::nothrow) MoclConnectCallback();

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    EXPECT_TRUE(data.WriteInterfaceToken(u"123"));

    EXPECT_EQ(
        object->OnRemoteRequest(
            MockRegisterService::COMMAND_SHOW_DEVICE_LIST, data, reply, option), ERR_INVALID_STATE);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_005 end.";
}

/*
 * Feature: AbilityManager
 * Function: RemoteRegisterServiceStub
 * SubFunction: OnRemoteRequest
 * FunctionPoints: The parameter of function OnRemoteRequest.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function OnRemoteRequest parameter error cmd
 */
HWTEST_F(RemoteRegisterServiceStubTest, AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_006 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    EXPECT_TRUE(data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()));

    EXPECT_EQ(
        object->OnRemoteRequest(
            MockRegisterService::COMMAND_SHOW_DEVICE_LIST + 66, data, reply, option), IPC_STUB_UNKNOW_TRANS_ERR);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_006 end.";
}

/*
 * Feature: AbilityManager
 * Function: RemoteRegisterServiceStub
 * SubFunction: OnRemoteRequest
 * FunctionPoints: The parameter of function OnRemoteRequest.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function OnRemoteRequest parameter emptry funciton cmd
 */
HWTEST_F(RemoteRegisterServiceStubTest, AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_007 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    EXPECT_TRUE(data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()));

    object->requestFuncMap_[MockRegisterService::COMMAND_SHOW_DEVICE_LIST + 1] = nullptr;

    EXPECT_EQ(
        object->OnRemoteRequest(
            MockRegisterService::COMMAND_SHOW_DEVICE_LIST + 1, data, reply, option), IPC_STUB_UNKNOW_TRANS_ERR);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_007 end.";
}

/*
 * Feature: AbilityManager
 * Function: RemoteRegisterServiceStub
 * SubFunction: OnRemoteRequest
 * FunctionPoints: The parameter of function OnRemoteRequest.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function OnRemoteRequest register token nullptr parameter data
 */
HWTEST_F(RemoteRegisterServiceStubTest, AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_008 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    std::string bundleName = "ABC";
    sptr<IRemoteObject> token = object;
    ExtraParams extras = {};
    sptr<MoclConnectCallback> callback = new (std::nothrow) MoclConnectCallback();

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    EXPECT_TRUE(data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()));
    EXPECT_TRUE(data.WriteString(bundleName));
    EXPECT_TRUE(data.WriteRemoteObject(token));
    EXPECT_TRUE(data.WriteInt32(0));
    EXPECT_TRUE(extras.Marshalling(data));
    EXPECT_TRUE(data.WriteRemoteObject(callback->AsObject()));

    EXPECT_EQ(object->OnRemoteRequest(MockRegisterService::COMMAND_REGISTER, data, reply, option), ERR_INVALID_DATA);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_008 end.";
}

/*
 * Feature: AbilityManager
 * Function: RemoteRegisterServiceStub
 * SubFunction: OnRemoteRequest
 * FunctionPoints: The parameter of function OnRemoteRequest.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function OnRemoteRequest register callback nullptr parameter data
 */
HWTEST_F(RemoteRegisterServiceStubTest, AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_009 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    std::string bundleName = "ABC";
    sptr<IRemoteObject> token = object;
    ExtraParams extras = {};

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    EXPECT_TRUE(data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()));
    EXPECT_TRUE(data.WriteString(bundleName));
    EXPECT_TRUE(data.WriteRemoteObject(token));
    EXPECT_TRUE(data.WriteInt32(1));
    EXPECT_TRUE(extras.Marshalling(data));

    EXPECT_EQ(object->OnRemoteRequest(MockRegisterService::COMMAND_REGISTER, data, reply, option), ERR_NULL_OBJECT);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_009 end.";
}

/*
 * Feature: AbilityManager
 * Function: RemoteRegisterServiceStub
 * SubFunction: OnRemoteRequest
 * FunctionPoints: The parameter of function OnRemoteRequest.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function OnRemoteRequest show device list extras nullptr parameter data
 */
HWTEST_F(RemoteRegisterServiceStubTest, AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_010 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    constexpr int32_t registerToken = 100;
    ExtraParams extras = {};

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    EXPECT_TRUE(data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()));
    EXPECT_TRUE(data.WriteInt32(registerToken));
    EXPECT_TRUE(data.WriteInt32(0));
    EXPECT_TRUE(extras.Marshalling(data));

    EXPECT_EQ(
        object->OnRemoteRequest(
            MockRegisterService::COMMAND_SHOW_DEVICE_LIST, data, reply, option), ERR_INVALID_DATA);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_010 end.";
}

/*
 * Feature: AbilityManager
 * Function: RemoteRegisterServiceStub
 * SubFunction: OnRemoteRequest
 * FunctionPoints: The parameter of function OnRemoteRequest.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify construct destructor function of RemoteRegisterServiceStub
 */
HWTEST_F(RemoteRegisterServiceStubTest, AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_011 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    object.clear();
    object = nullptr;
    EXPECT_TRUE(object == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceStub_OnRemoteRequest_011 end.";
}
}   // namespace AppExecFwk
}   // namespace OHOS