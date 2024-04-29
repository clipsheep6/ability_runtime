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
#define protected public
#include "event_handler.h"
#include "message_parcel.h"
#include "parcel.h"
#include "reverse_continuation_scheduler_primary.h"
#include "reverse_continuation_scheduler_primary_stub.h"
#undef protected
#undef private
namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
class MockReverseContinuationSchedulerPrimaryStub : public ReverseContinuationSchedulerPrimaryStub {
public:
    MockReverseContinuationSchedulerPrimaryStub() = default;
    virtual ~MockReverseContinuationSchedulerPrimaryStub() {}
    bool ContinuationBack(const AAFwk::Want& want) override
    {
        return true;
    }
    void NotifyReplicaTerminated() override {}
};
class ReverseContinuationSchedulerPrimaryStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<ReverseContinuationSchedulerPrimaryStub> primaryStub_;
};
void ReverseContinuationSchedulerPrimaryStubTest::SetUpTestCase(void) {}
void ReverseContinuationSchedulerPrimaryStubTest::TearDownTestCase(void) {}
void ReverseContinuationSchedulerPrimaryStubTest::SetUp(void)
{
    primaryStub_ = new (std::nothrow) MockReverseContinuationSchedulerPrimaryStub();
}
void ReverseContinuationSchedulerPrimaryStubTest::TearDown(void)
{
    primaryStub_ = nullptr;
}

/**
 * @tc.number: OnRemoteRequest_0100
 * @tc.name: OnRemoteRequest
 * @tc.desc: Verify that function OnRemoteRequest
 */
HWTEST_F(ReverseContinuationSchedulerPrimaryStubTest, OnRemoteRequest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest_0100 start";
    uint32_t code = 1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto result = primaryStub_->OnRemoteRequest(code, data, reply, option);

    EXPECT_EQ(result, INVALID_FD);
    GTEST_LOG_(INFO) << "OnRemoteRequest_0100 end";
}

/**
 * @tc.number: OnRemoteRequest_0200
 * @tc.name: OnRemoteRequest
 * @tc.desc: Verify that function OnRemoteRequest
 */
HWTEST_F(ReverseContinuationSchedulerPrimaryStubTest, OnRemoteRequest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest_0200 start";
    uint32_t code = 1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::u16string name = u"ohos.abilityshell.ReverseContinuationSchedulerMaster";
    data.WriteInterfaceToken(name);
    auto result = primaryStub_->OnRemoteRequest(code, data, reply, option);

    EXPECT_EQ(result, ERR_NONE);
    GTEST_LOG_(INFO) << "OnRemoteRequest_0200 end";
}

/**
 * @tc.number: OnRemoteRequest_0300
 * @tc.name: OnRemoteRequest
 * @tc.desc: Verify that function OnRemoteRequest
 */
HWTEST_F(ReverseContinuationSchedulerPrimaryStubTest, OnRemoteRequest_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest_0300 start";
    uint32_t code = 3;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::u16string name = u"ohos.abilityshell.ReverseContinuationSchedulerMaster";
    data.WriteInterfaceToken(name);
    primaryStub_->requestFuncMap_[3] = nullptr;
    auto result = primaryStub_->OnRemoteRequest(code, data, reply, option);

    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);
    GTEST_LOG_(INFO) << "OnRemoteRequest_0300 end";
}

/**
 * @tc.number: NotifyReplicaTerminatedInner_0100
 * @tc.name: NotifyReplicaTerminatedInner
 * @tc.desc: Verify that function NotifyReplicaTerminatedInner
 */
HWTEST_F(ReverseContinuationSchedulerPrimaryStubTest, NotifyReplicaTerminatedInner_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "NotifyReplicaTerminatedInner_0100 start";
    MessageParcel data;
    MessageParcel reply;
    auto result = primaryStub_->NotifyReplicaTerminatedInner(data, reply);

    EXPECT_EQ(result, ERR_NONE);
    GTEST_LOG_(INFO) << "NotifyReplicaTerminatedInner_0100 end";
}

/**
 * @tc.number: ContinuationBackInner_0100
 * @tc.name: ContinuationBackInner
 * @tc.desc: Verify that function ContinuationBackInner
 */
HWTEST_F(ReverseContinuationSchedulerPrimaryStubTest, ContinuationBackInner_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ContinuationBackInner_0100 start";
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(0);
    auto result = primaryStub_->ContinuationBackInner(data, reply);

    EXPECT_EQ(result, -1);
    GTEST_LOG_(INFO) << "ContinuationBackInner_0100 end";
}

/**
 * @tc.number: ContinuationBackInner_0200
 * @tc.name: ContinuationBackInner
 * @tc.desc: Verify that function ContinuationBackInner
 */
HWTEST_F(ReverseContinuationSchedulerPrimaryStubTest, ContinuationBackInner_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ContinuationBackInner_0200 start";
    MessageParcel data;
    MessageParcel reply;
    auto result = primaryStub_->ContinuationBackInner(data, reply);

    EXPECT_EQ(result, -1);
    GTEST_LOG_(INFO) << "ContinuationBackInner_0200 end";
}

/**
 * @tc.number: ContinuationBackInner_0300
 * @tc.name: ContinuationBackInner
 * @tc.desc: Verify that function ContinuationBackInner
 */
HWTEST_F(ReverseContinuationSchedulerPrimaryStubTest, ContinuationBackInner_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ContinuationBackInner_0300 start";
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(1);
    auto result = primaryStub_->ContinuationBackInner(data, reply);

    EXPECT_EQ(result, -1);
    GTEST_LOG_(INFO) << "ContinuationBackInner_0300 end";
}
} // namespace AppExecFwk
} // namespace OHOS