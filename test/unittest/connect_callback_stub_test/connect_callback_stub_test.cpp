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
#include "connect_callback_stub.h"
#undef private

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {

class MockConnectCallbackStub: public ConnectCallbackStub {
public:
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }
    void Connect(const string &deviceId, const string &deviceType) override
    {
        return;
    }
    void Disconnect(const string &deviceId) override
    {
        return;
    }
};

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

/*
 * Feature: ConnectCallbackStub
 * Function: ConnectInner
 * SubFunction: NA
 * FunctionPoints: ServiceExtensionContextTest ConnectInner
 * EnvConditions: NA
 * CaseDescription: Verify ConnectInner
 */
HWTEST_F(ConnectCallbackStubTest, remote_register_service_proxy_ConnectInner_001, TestSize.Level1)
{   
    MockConnectCallbackStub connectCallbackStub;
    MessageParcel data;
    MessageParcel reply;
    int result = connectCallbackStub.ConnectInner(data, reply);
    EXPECT_EQ(ERR_NONE, result);
}

/*
 * Feature: ConnectCallbackStub
 * Function: DisconnectInner
 * SubFunction: NA
 * FunctionPoints: ServiceExtensionContextTest DisconnectInner
 * EnvConditions: NA
 * CaseDescription: Verify DisconnectInner
 */
HWTEST_F(ConnectCallbackStubTest, remote_register_service_proxy_DisconnectInner, TestSize.Level1)
{   
    MockConnectCallbackStub connectCallbackStub;
    MessageParcel data;
    MessageParcel reply;
    int result = connectCallbackStub.DisconnectInner(data, reply);
    EXPECT_EQ(ERR_NONE, result);
}

/*
 * Feature: ConnectCallbackStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: ServiceExtensionContextTest OnRemoteRequest
 * EnvConditions: NA
 * CaseDescription: Verify OnRemoteRequest
 */
HWTEST_F(ConnectCallbackStubTest, remote_register_service_proxy_OnRemoteRequest, TestSize.Level1)
{   
    MockConnectCallbackStub connectCallbackStub;
    uint32_t code = 1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int result = connectCallbackStub.OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ERR_NONE, result);
}
}
}