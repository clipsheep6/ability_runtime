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
#include "remote_register_service_stub.h"
#undef private

#include "hilog_wrapper.h"
#include "remote_register_service_interface.h"
#include <iremote_object.h>
#include <iremote_stub.h>
#include <map>
#include "message_parcel.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {

class MockRemoteRegisterServiceStub : public RemoteRegisterServiceStub {
public:
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

class RemoteRegisterServiceStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void RemoteRegisterServiceStubTest::SetUpTestCase(void)
{}
void RemoteRegisterServiceStubTest::TearDownTestCase(void)
{}
void RemoteRegisterServiceStubTest::SetUp(void)
{}
void RemoteRegisterServiceStubTest::TearDown(void)
{}

/*
 * Feature: RemoteRegisterServiceStub
 * Function: startAbility
 * SubFunction: NA
 * FunctionPoints: RemoteRegisterServiceStub OnRemoteRequest
 * EnvConditions: NA
 * CaseDescription: Verify startAbility
 */
HWTEST_F(RemoteRegisterServiceStubTest, remote_register_service_stub_OnRemoteRequest_001, TestSize.Level1)
{   
    MockRemoteRegisterServiceStub remoteRegisterServiceStub;
    uint32_t code = 1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = {MessageOption::TF_SYNC};
    int result = remoteRegisterServiceStub.OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ERR_INVALID_STATE, result);
}

/*
 * Feature: RemoteRegisterServiceStub
 * Function: startAbility
 * SubFunction: NA
 * FunctionPoints: RemoteRegisterServiceStub RegisterInner
 * EnvConditions: NA
 * CaseDescription: Verify startAbility
 */
HWTEST_F(RemoteRegisterServiceStubTest, remote_register_service_stub_RegisterInner_001, TestSize.Level1)
{   
    MockRemoteRegisterServiceStub remoteRegisterServiceStub;
    MessageParcel data;
    MessageParcel reply;
    int result = remoteRegisterServiceStub.RegisterInner(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, result);
}

/*
 * Feature: RemoteRegisterServiceStub
 * Function: startAbility
 * SubFunction: NA
 * FunctionPoints: RemoteRegisterServiceStub UnregisterInner
 * EnvConditions: NA
 * CaseDescription: Verify startAbility
 */
HWTEST_F(RemoteRegisterServiceStubTest, remote_register_service_stub_UnregisterInner_001, TestSize.Level1)
{   
    MockRemoteRegisterServiceStub remoteRegisterServiceStub;
    MessageParcel data;
    MessageParcel reply;
    int result = remoteRegisterServiceStub.UnregisterInner(data, reply);
    EXPECT_EQ(NO_ERROR, result);
}

/*
 * Feature: RemoteRegisterServiceStub
 * Function: startAbility
 * SubFunction: NA
 * FunctionPoints: RemoteRegisterServiceStub UpdateConnectStatusInner
 * EnvConditions: NA
 * CaseDescription: Verify startAbility
 */
HWTEST_F(RemoteRegisterServiceStubTest, remote_register_service_stub_UpdateConnectStatusInner_001, TestSize.Level1)
{   
    MockRemoteRegisterServiceStub remoteRegisterServiceStub;
    MessageParcel data;
    MessageParcel reply;
    int result = remoteRegisterServiceStub.UpdateConnectStatusInner(data, reply);
    EXPECT_EQ(NO_ERROR, result);
}

/*
 * Feature: RemoteRegisterServiceStub
 * Function: startAbility
 * SubFunction: NA
 * FunctionPoints: RemoteRegisterServiceStub ShowDeviceListInner
 * EnvConditions: NA
 * CaseDescription: Verify startAbility
 */
HWTEST_F(RemoteRegisterServiceStubTest, remote_register_service_stub_ShowDeviceListInner_001, TestSize.Level1)
{   
    MockRemoteRegisterServiceStub remoteRegisterServiceStub;
    MessageParcel data;
    MessageParcel reply;
    int result = remoteRegisterServiceStub.ShowDeviceListInner(data, reply);
    EXPECT_EQ(ERR_INVALID_DATA, result);
}
}
}