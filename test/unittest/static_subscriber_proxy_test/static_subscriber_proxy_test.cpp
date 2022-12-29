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
class StaticSubscriberProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void StaticSubscriberProxyTest::SetUpTestCase(void)
{}

void StaticSubscriberProxyTest::TearDownTestCase(void)
{}

void StaticSubscriberProxyTest::SetUp(void)
{}

void StaticSubscriberProxyTest::TearDown(void)
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
        return 0;
    }

    int32_t returnCode_ = ERR_NONE;
};

class MockStaticSubscriberStub : public StaticSubscriberStub {
public:
    MockStaticSubscriberStub() {};
    virtual ~MockStaticSubscriberStub() {};

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        GTEST_LOG_(INFO) << "MockRegisterService::SendRequest called. return value " << returnCode_;
        return returnCode_;
    }

    ErrCode OnReceiveEvent(CommonEventData* inData) override
    {
        return 0;
    }

    int32_t returnCode_ = 1;
};

/**
 * @tc.number: StaticSubscriberProxy_OnReceiveEvent_001
 * @tc.name: OnReceiveEvent
 * @tc.desc: Pass in normal parameters, and the test program executes correctly without abnormal exit
 */
HWTEST_F(StaticSubscriberProxyTest, StaticSubscriberProxy_OnReceiveEvent_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StaticSubscriberProxy_OnReceiveEvent_001 start.";
    sptr<MockStaticSubscriber> object = new (std::nothrow) MockStaticSubscriber();
    sptr<StaticSubscriberProxy> testProxy = new (std::nothrow) StaticSubscriberProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    EXPECT_TRUE(testProxy->remoteObject_ != nullptr);
    CommonEventData* inData = nullptr;
    EXPECT_EQ(testProxy->OnReceiveEvent(inData), ERR_OK);
    GTEST_LOG_(INFO) << "StaticSubscriberProxy_OnReceiveEvent_001 end.";
}

/**
 * @tc.number: StaticSubscriberProxy_OnReceiveEvent_002
 * @tc.name: OnReceiveEvent
 * @tc.desc: Pass in normal parameters, and the test program executes correctly without abnormal exit
 */
HWTEST_F(StaticSubscriberProxyTest, StaticSubscriberProxy_OnReceiveEvent_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StaticSubscriberProxy_OnReceiveEvent_002 start.";
    sptr<MockStaticSubscriberStub> object = new (std::nothrow) MockStaticSubscriberStub();
    sptr<StaticSubscriberProxy> testProxy = new (std::nothrow) StaticSubscriberProxy(object);
    EXPECT_TRUE(testProxy != nullptr);
    EXPECT_TRUE(testProxy->remoteObject_ != nullptr);
    CommonEventData* inData = nullptr;
    EXPECT_EQ(testProxy->OnReceiveEvent(inData), 1);
    GTEST_LOG_(INFO) << "StaticSubscriberProxy_OnReceiveEvent_002 end.";
}
}   // namespace AppExecFwk
}   // namespace OHOS