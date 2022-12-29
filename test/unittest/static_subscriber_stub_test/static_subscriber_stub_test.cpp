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
#include "static_subscriber_proxy.h"
#include "static_subscriber_stub.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace testing;
using namespace std::chrono;

namespace OHOS {
namespace AppExecFwk {
class StaticSubscriberStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void StaticSubscriberStubTest::SetUpTestCase(void)
{}

void StaticSubscriberStubTest::TearDownTestCase(void)
{}

void StaticSubscriberStubTest::SetUp(void)
{}

void StaticSubscriberStubTest::TearDown(void)
{}

class MockStaticSubscriber : public StaticSubscriberStub {
public:
    MockStaticSubscriber() {};
    virtual ~MockStaticSubscriber() {};

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        GTEST_LOG_(INFO) << "MockRegisterService::SendRequest called. return value " << returnCode_;
        reply.WriteInt32(ERR_NONE);
        return returnCode_;
    }

    ErrCode OnReceiveEvent(CommonEventData* inData) override
    {
        GTEST_LOG_(INFO) << "MockRegisterService::OnReceiveEvent called.";
        return 0;
    }

    int32_t returnCode_ = ERR_NONE;
};

/**
 * @tc.number: StaticSubscriberStub_OnRemoteRequest_001
 * @tc.name: OnRemoteRequest
 * @tc.desc: Parameter code is default value, execute the corresponding
 *           branch and return the corresponding return value
 */
HWTEST_F(StaticSubscriberStubTest, StaticSubscriberStub_OnRemoteRequest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StaticSubscriberStub_OnRemoteRequest_001 start.";
    sptr<MockStaticSubscriber> object = new (std::nothrow) MockStaticSubscriber();
    EXPECT_TRUE(object != nullptr);
    uint32_t code = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_TRUE(data.WriteInterfaceToken(u"OHOS.AppExecFwk.IStaticSubscriber"));
    EXPECT_EQ(object->OnRemoteRequest(code, data, reply, option), IPC_STUB_UNKNOW_TRANS_ERR);
    GTEST_LOG_(INFO) << "StaticSubscriberStub_OnRemoteRequest_001 end.";
}

/**
 * @tc.number: StaticSubscriberStub_OnRemoteRequest_002
 * @tc.name: OnRemoteRequest
 * @tc.desc: The parameter data is different from the GetDescriptor field. Execute the corresponding
 *           branch and return the corresponding return value
 */
HWTEST_F(StaticSubscriberStubTest, StaticSubscriberStub_OnRemoteRequest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StaticSubscriberStub_OnRemoteRequest_002 start.";
    sptr<MockStaticSubscriber> object = new (std::nothrow) MockStaticSubscriber();
    EXPECT_TRUE(object != nullptr);
    uint32_t code = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(object->OnRemoteRequest(code, data, reply, option), ERR_TRANSACTION_FAILED);
    GTEST_LOG_(INFO) << "StaticSubscriberStub_OnRemoteRequest_002 end.";
}

/**
 * @tc.number: StaticSubscriberStub_OnRemoteRequest_003
 * @tc.name: OnRemoteRequest
 * @tc.desc: Parameter code is COMMAND_ON_RECEIVE_EVENT value, execute the corresponding
 *           branch and return the corresponding return value
 */
HWTEST_F(StaticSubscriberStubTest, StaticSubscriberStub_OnRemoteRequest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StaticSubscriberStub_OnRemoteRequest_003 start.";
    sptr<MockStaticSubscriber> object = new (std::nothrow) MockStaticSubscriber();
    EXPECT_TRUE(object != nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_TRUE(data.WriteInterfaceToken(u"OHOS.AppExecFwk.IStaticSubscriber"));
    EXPECT_EQ(object->OnRemoteRequest(MockStaticSubscriber::COMMAND_ON_RECEIVE_EVENT, data, reply, option), ERR_NONE);
    GTEST_LOG_(INFO) << "StaticSubscriberStub_OnRemoteRequest_003 end.";
}
}   // namespace AppExecFwk
}   // namespace OHOS