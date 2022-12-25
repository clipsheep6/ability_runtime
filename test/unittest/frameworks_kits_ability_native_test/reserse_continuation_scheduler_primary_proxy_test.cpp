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

#include "gmock/gmock.h"
#define private public
#define protected public
#include "message_parcel.h"
#include "mock_ability_connect_callback.h"
#include "peer_holder.h"
#include "reverse_continuation_scheduler_primary.h"
#include "reverse_continuation_scheduler_primary_proxy.h"
#include "reverse_continuation_scheduler_primary_stub.h"
#undef private
#undef protected
namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using ::testing::_;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
class MockContinuationSchedulerPrimaryStub : public ReverseContinuationSchedulerPrimaryStub {
public:
    MOCK_METHOD4(OnRemoteRequest, int(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option));
    MOCK_METHOD0(NotifyReplicaTerminated, void());
    MOCK_METHOD1(ContinuationBack, bool(const AAFwk::Want& want));
};

class ReverseContinuationSchedulerPrimaryProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ReverseContinuationSchedulerPrimaryProxyTest::SetUpTestCase(void) {}
void ReverseContinuationSchedulerPrimaryProxyTest::TearDownTestCase(void) {}
void ReverseContinuationSchedulerPrimaryProxyTest::SetUp(void) {}
void ReverseContinuationSchedulerPrimaryProxyTest::TearDown(void) {}

/**
 * @tc.number: ContinuationBack_0100
 * @tc.name: ContinuationBack
 * @tc.desc: Verify that function ContinuationBack.
 */
HWTEST_F(ReverseContinuationSchedulerPrimaryProxyTest, ContinuationBack_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ContinuationBack_0100 start";
    auto remoteObject = new (std::nothrow) MockContinuationSchedulerPrimaryStub();
    auto primaryProxy_ = new (std::nothrow) ReverseContinuationSchedulerPrimaryProxy(remoteObject->AsObject());
    AAFwk::Want want;
    EXPECT_CALL(*remoteObject, OnRemoteRequest(_, _, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(::testing::Return(2));
    auto result = primaryProxy_->ContinuationBack(want);
    EXPECT_TRUE(result);
    testing::Mock::AllowLeak(remoteObject);
    GTEST_LOG_(INFO) << "ContinuationBack_0100 end";
}

/**
 * @tc.number: ContinuationBack_0200
 * @tc.name: ContinuationBack
 * @tc.desc: Verify that function ContinuationBack.
 */
HWTEST_F(ReverseContinuationSchedulerPrimaryProxyTest, ContinuationBack_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ContinuationBack_0100 start";
    sptr<IRemoteObject> remoteObject = nullptr;
    auto primaryProxy_ = new (std::nothrow) ReverseContinuationSchedulerPrimaryProxy(remoteObject);
    AAFwk::Want want;
    auto result = primaryProxy_->ContinuationBack(want);
    EXPECT_TRUE(!result);
    GTEST_LOG_(INFO) << "ContinuationBack_0100 end";
}

/**
 * @tc.number: NotifyReplicaTerminated_0100
 * @tc.name: NotifyReplicaTerminated
 * @tc.desc: Verify that function NotifyReplicaTerminated.
 */
HWTEST_F(ReverseContinuationSchedulerPrimaryProxyTest, NotifyReplicaTerminated_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "NotifyReplicaTerminated_0100 start";
    sptr<IRemoteObject> remoteObject = nullptr;
    auto primaryProxy_ = new (std::nothrow) ReverseContinuationSchedulerPrimaryProxy(remoteObject);
    primaryProxy_->NotifyReplicaTerminated();

    auto Object = new (std::nothrow) MockContinuationSchedulerPrimaryStub();
    primaryProxy_ = new (std::nothrow) ReverseContinuationSchedulerPrimaryProxy(Object->AsObject());
    EXPECT_CALL(*Object, OnRemoteRequest(_, _, _, _))
        .Times(::testing::AtLeast(1)).WillOnce(::testing::Return(2));
    primaryProxy_->NotifyReplicaTerminated();
    testing::Mock::AllowLeak(Object);
    GTEST_LOG_(INFO) << "NotifyReplicaTerminated_0100 end";
}
} // namespace AppExecFwk
} // namespace OHOS