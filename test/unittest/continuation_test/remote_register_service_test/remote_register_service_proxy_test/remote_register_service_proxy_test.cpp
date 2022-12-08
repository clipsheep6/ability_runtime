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
#include "remote_register_service_proxy.h"
#include "remote_register_service_stub.h"
#include "connect_callback_stub.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace testing;
using namespace std::chrono;

namespace OHOS {
namespace AppExecFwk {
class RemoteRegisterServiceProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:

};

void RemoteRegisterServiceProxyTest::SetUpTestCase(void)
{}
void RemoteRegisterServiceProxyTest::TearDownTestCase(void)
{}

void RemoteRegisterServiceProxyTest::SetUp(void)
{}

void RemoteRegisterServiceProxyTest::TearDown(void)
{}

class MockRegisterService : public RemoteRegisterServiceStub {
public:
    MockRegisterService() {};
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
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: Register
 * FunctionPoints: The parameter of function Register.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Register parameter bundleName token extras callback
 */
HWTEST_F(RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_Register_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Register_001 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    std::string bundleName = "ABC";
    sptr<IRemoteObject> token = object;
    ExtraParams extras = {};
    sptr<MoclConnectCallback> callback = new (std::nothrow) MoclConnectCallback();
    EXPECT_EQ(testProxy->Register(bundleName, token, extras, callback), ERR_NONE);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Register_001 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: Register
 * FunctionPoints: The parameter of function Register.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Register parameter bundlename empty
 */
HWTEST_F(RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_Register_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Register_002 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    std::string bundleName = "";
    sptr<IRemoteObject> token = object;
    ExtraParams extras = {};
    sptr<MoclConnectCallback> callback = new (std::nothrow) MoclConnectCallback();
    EXPECT_EQ(testProxy->Register(bundleName, token, extras, callback), ERR_INVALID_DATA);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Register_002 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: Register
 * FunctionPoints: The parameter of function Register.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Register Object is empty
 */
HWTEST_F(RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_Register_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Register_003 start.";
    sptr<MockRegisterService> object = nullptr;
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    std::string bundleName = "ABC";
    sptr<IRemoteObject> token = new (std::nothrow) MockRegisterService();
    ExtraParams extras = {};
    sptr<MoclConnectCallback> callback = new (std::nothrow) MoclConnectCallback();
    EXPECT_EQ(testProxy->Register(bundleName, token, extras, callback), ERR_NULL_OBJECT);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Register_003 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: Register
 * FunctionPoints: The parameter of function Register.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Register parameter callback->AsObject is empty
 */
HWTEST_F(RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_Register_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Register_004 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    std::string bundleName = "ABC";
    sptr<IRemoteObject> token = object;
    ExtraParams extras = {};
    sptr<MoclConnectCallback> callback = new (std::nothrow) MoclConnectCallback();
    callback->asObject_ = false;
    EXPECT_EQ(testProxy->Register(bundleName, token, extras, callback), IPC_INVOKER_WRITE_TRANS_ERR);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Register_004 end.";
}
/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: Register
 * FunctionPoints: The parameter of function Register.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Register ipc error
 */
HWTEST_F(RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_Register_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Register_005 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    std::string bundleName = "ABC";
    sptr<IRemoteObject> token = object;
    ExtraParams extras = {};
    sptr<MoclConnectCallback> callback = new (std::nothrow) MoclConnectCallback();
    object->returnCode_ = ERR_NONE - 1;
    EXPECT_EQ(testProxy->Register(bundleName, token, extras, callback), IPC_INVOKER_TRANSLATE_ERR);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Register_005 end.";
}


/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: Register
 * FunctionPoints: The parameter of function Register.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Register parameter token is empty
 */
HWTEST_F(RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_Register_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Register_006 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    std::string bundleName = "abc";
    sptr<IRemoteObject> token = nullptr;
    ExtraParams extras = {};
    sptr<MoclConnectCallback> callback = new (std::nothrow) MoclConnectCallback();
    EXPECT_EQ(testProxy->Register(bundleName, token, extras, callback), ERR_INVALID_DATA);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Register_006 end.";
}


/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: Register
 * FunctionPoints: The parameter of function Register.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Register parameter callback is empty
 */
HWTEST_F(RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_Register_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Register_007 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    std::string bundleName = "abc";
    sptr<IRemoteObject> token = object;
    ExtraParams extras = {};
    sptr<MoclConnectCallback> callback = nullptr;
    EXPECT_EQ(testProxy->Register(bundleName, token, extras, callback), ERR_INVALID_DATA);
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Register_007 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: Unregister
 * FunctionPoints: The parameter of function Unregister.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Unregister parameter registerToken is 0
 */
HWTEST_F(RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_Unregister_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Unregister_001 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    constexpr int32_t registerToken = 0;
    EXPECT_TRUE(testProxy->Unregister(registerToken));
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Unregister_001 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: Unregister
 * FunctionPoints: The parameter of function Unregister.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Unregister parameter object is nullptr
 */
HWTEST_F(RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_Unregister_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Unregister_002 start.";
    sptr<MockRegisterService> object = nullptr;
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    constexpr int32_t registerToken = 0;
    EXPECT_FALSE(testProxy->Unregister(registerToken));
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Unregister_002 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: Unregister
 * FunctionPoints: The parameter of function Unregister.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Unregister parameter registerToken is 100
 */
HWTEST_F(RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_Unregister_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Unregister_003 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    constexpr int32_t registerToken = 100;
    EXPECT_TRUE(testProxy->Unregister(registerToken));
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Unregister_003 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: Unregister
 * FunctionPoints: The parameter of function Unregister.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Unregister ipc error
 */
HWTEST_F(RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_Unregister_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Unregister_004 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    constexpr int32_t registerToken = 0;
    object->returnCode_ = ERR_NONE - 1;
    EXPECT_FALSE(testProxy->Unregister(registerToken));
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_Unregister_004 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: UpdateConnectStatus
 * FunctionPoints: The parameter of function UpdateConnectStatus.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function UpdateConnectStatus parameter registerToken status deviceId
 */
HWTEST_F(
    RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_UpdateConnectStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_UpdateConnectStatus_001 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    constexpr int32_t registerToken = 0;
    constexpr int32_t status = 0;
    const std::string deviceId = "7001005458323933328a592135733900";
    EXPECT_TRUE(testProxy->UpdateConnectStatus(registerToken, deviceId, status));
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_UpdateConnectStatus_001 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: UpdateConnectStatus
 * FunctionPoints: The parameter of function UpdateConnectStatus.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function UpdateConnectStatus object is nullptr
 */
HWTEST_F(
    RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_UpdateConnectStatus_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_UpdateConnectStatus_002 start.";
    sptr<MockRegisterService> object = nullptr;
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    constexpr int32_t registerToken = 0;
    constexpr int32_t status = 0;
    const std::string deviceId = "7001005458323933328a592135733900";
    EXPECT_FALSE(testProxy->UpdateConnectStatus(registerToken, deviceId, status));
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_UpdateConnectStatus_002 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: UpdateConnectStatus
 * FunctionPoints: The parameter of function UpdateConnectStatus.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function UpdateConnectStatus parameter registerToken is 100 status is 1
 */
HWTEST_F(
    RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_UpdateConnectStatus_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_UpdateConnectStatus_003 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    constexpr int32_t registerToken = 100;
    constexpr int32_t status = 1;
    const std::string deviceId = "7001005458323933328a592135733900";
    EXPECT_TRUE(testProxy->UpdateConnectStatus(registerToken, deviceId, status));
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_UpdateConnectStatus_003 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: UpdateConnectStatus
 * FunctionPoints: The parameter of function UpdateConnectStatus.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function UpdateConnectStatus ipc error
 */
HWTEST_F(
    RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_UpdateConnectStatus_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_UpdateConnectStatus_004 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    constexpr int32_t registerToken = 0;
    constexpr int32_t status = 0;
    const std::string deviceId = "7001005458323933328a592135733900";
    object->returnCode_ = ERR_NONE - 1;
    EXPECT_FALSE(testProxy->UpdateConnectStatus(registerToken, deviceId, status));
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_UpdateConnectStatus_004 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: UpdateConnectStatus
 * FunctionPoints: The parameter of function UpdateConnectStatus.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function UpdateConnectStatus registerToken is 100 ipc error
 */
HWTEST_F(
    RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_UpdateConnectStatus_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_UpdateConnectStatus_005 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    constexpr int32_t registerToken = 100;
    constexpr int32_t status = 1;
    const std::string deviceId = "7001005458323933328a592135733900";
    object->returnCode_ = ERR_NONE - 1;
    EXPECT_FALSE(testProxy->UpdateConnectStatus(registerToken, deviceId, status));
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_UpdateConnectStatus_005 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: ShowDeviceList
 * FunctionPoints: The parameter of function ShowDeviceList.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function ShowDeviceList registerToken extras
 */
HWTEST_F(
    RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_ShowDeviceList_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_ShowDeviceList_001 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    constexpr int32_t registerToken = 0;
    ExtraParams extras = {};
    EXPECT_TRUE(testProxy->ShowDeviceList(registerToken, extras));
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_ShowDeviceList_001 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: ShowDeviceList
 * FunctionPoints: The parameter of function ShowDeviceList.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function ShowDeviceList object is nullptr
 */
HWTEST_F(
    RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_ShowDeviceList_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_ShowDeviceList_002 start.";
    sptr<MockRegisterService> object = nullptr;
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    constexpr int32_t registerToken = 0;
    ExtraParams extras = {};
    EXPECT_FALSE(testProxy->ShowDeviceList(registerToken, extras));
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_ShowDeviceList_002 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: ShowDeviceList
 * FunctionPoints: The parameter of function ShowDeviceList.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Register parameter registerToken is 100
 */
HWTEST_F(
    RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_ShowDeviceList_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_ShowDeviceList_003 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    constexpr int32_t registerToken = 100;
    ExtraParams extras = {};
    EXPECT_TRUE(testProxy->ShowDeviceList(registerToken, extras));
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_ShowDeviceList_003 end.";
}

/*
 * Feature: AbilityRuntime
 * Function: RemoteRegisterServiceProxy
 * SubFunction: ShowDeviceList
 * FunctionPoints: The parameter of function ShowDeviceList.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Register ipc error
 */
HWTEST_F(
    RemoteRegisterServiceProxyTest, AppExecFwk_RemoteRegisterServiceProxy_ShowDeviceList_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_ShowDeviceList_004 start.";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    sptr<RemoteRegisterServiceProxy> testProxy = new (std::nothrow) RemoteRegisterServiceProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    constexpr int32_t registerToken = 0;
    ExtraParams extras = {};
    object->returnCode_ = ERR_NONE - 1;
    EXPECT_FALSE(testProxy->ShowDeviceList(registerToken, extras));
    GTEST_LOG_(INFO) << "AppExecFwk_RemoteRegisterServiceProxy_ShowDeviceList_004 end.";
}
}   // namespace AppExecFwk
}   // namespace OHOS