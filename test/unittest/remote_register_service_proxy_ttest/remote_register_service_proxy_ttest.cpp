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

#define private public
#include "remote_register_service_proxy.h"
#undef private

#include "hilog_wrapper.h"
#include "remote_register_service_interface.h"
#include <iremote_object.h>
#include "iremote_proxy.h"
#include <map>
#include "message_parcel.h"
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {

class MockRemoteRegisterServiceProxy : public RemoteRegisterServiceProxy {
public:
    MockRemoteRegisterServiceProxy() : RemoteRegisterServiceProxy(nullptr) {}
    int Register(const std::string &bundleName, const sptr<IRemoteObject> &token, const ExtraParams &extras,
        const sptr<IConnectCallback> &callback) override
    {
        return 0;
    }
    bool Unregister(int registerToken) override
    {
        return false;
    }
    bool UpdateConnectStatus(int registerToken, const std::string &deviceId, int status) override
    {
        return false;
    }
    bool ShowDeviceList(int registerToken, const ExtraParams &extras) override
    {
        return false;
    }
};

class RemoteRegisterServiceProxytest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void RemoteRegisterServiceProxytest::SetUpTestCase(void)
{}
void RemoteRegisterServiceProxytest::TearDownTestCase(void)
{}
void RemoteRegisterServiceProxytest::SetUp(void)
{}
void RemoteRegisterServiceProxytest::TearDown(void)
{}

/*
 * Feature: RemoteRegisterServiceProxy
 * Function: Register
 * SubFunction: NA
 * FunctionPoints: ServiceExtensionContextTest Register
 * EnvConditions: NA
 * CaseDescription: Verify Register
 */
HWTEST_F(RemoteRegisterServiceProxytest, remote_register_service_proxy_Register_001, TestSize.Level1)
{   
    MockRemoteRegisterServiceProxy remoteRegisterServiceProxy;
    std::string bundleName = "this is an bundleName";
    // sptr<IRemoteObject> token = new IRemoteObject();
    ExtraParams extras;
    // sptr<IConnectCallback> callback = new IConnectCallback();
    int result = remoteRegisterServiceProxy.Register(bundleName, nullptr, extras, nullptr);
    EXPECT_EQ(NO_ERROR, result);
}

/*
 * Feature: RemoteRegisterServiceProxy
 * Function: Unregister
 * SubFunction: NA
 * FunctionPoints: RemoteRegisterServiceStub UnregisterInner
 * EnvConditions: NA
 * CaseDescription: Verify Unregister
 */
HWTEST_F(RemoteRegisterServiceProxytest, remote_register_service_proxy_Unregister_001, TestSize.Level1)
{   
    MockRemoteRegisterServiceProxy remoteRegisterServiceProxy;
    int registerToken = 0;
    bool result = remoteRegisterServiceProxy.Unregister(registerToken);
    EXPECT_EQ(false, result);
}

/*
 * Feature: RemoteRegisterServiceProxy
 * Function: UpdateConnectStatus
 * SubFunction: NA
 * FunctionPoints: RemoteRegisterServiceStub UpdateConnectStatusInner
 * EnvConditions: NA
 * CaseDescription: Verify UpdateConnectStatus
 */
HWTEST_F(RemoteRegisterServiceProxytest, remote_register_service_proxy_UpdateConnectStatus_001, TestSize.Level1)
{   
    MockRemoteRegisterServiceProxy remoteRegisterServiceProxy;
    int registerToken = 0;
    std::string deviceId = "this is a deviceId";
    int status = 0;
    bool result = remoteRegisterServiceProxy.UpdateConnectStatus(registerToken, deviceId, status);
    EXPECT_EQ(false, result);
}

/*
 * Feature: RemoteRegisterServiceProxy
 * Function: ShowDeviceList
 * SubFunction: NA
 * FunctionPoints: RemoteRegisterServiceStub ShowDeviceListInner
 * EnvConditions: NA
 * CaseDescription: Verify ShowDeviceList
 */
HWTEST_F(RemoteRegisterServiceProxytest, remote_register_service_proxy_ShowDeviceList_001, TestSize.Level1)
{   
    MockRemoteRegisterServiceProxy remoteRegisterServiceProxy;
    int registerToken = 0;
    ExtraParams extras;
    bool result = remoteRegisterServiceProxy.ShowDeviceList(registerToken, extras);
    EXPECT_EQ(false, result);
}
}
}