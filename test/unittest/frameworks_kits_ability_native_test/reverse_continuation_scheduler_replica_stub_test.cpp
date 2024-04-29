/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "mock_reverse_continuation_scheduler_replica_stub.h"
#include "reverse_continuation_scheduler_replica_stub.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using testing::_;

class ReverseContinuationSchedulerReplicaStubTest : public testing::Test {
public:
    ReverseContinuationSchedulerReplicaStubTest() {}
    virtual ~ReverseContinuationSchedulerReplicaStubTest() {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ReverseContinuationSchedulerReplicaStubTest::SetUpTestCase(void)
{}

void ReverseContinuationSchedulerReplicaStubTest::TearDownTestCase(void)
{}

void ReverseContinuationSchedulerReplicaStubTest::SetUp(void)
{}

void ReverseContinuationSchedulerReplicaStubTest::TearDown(void)
{}

 /**
  * @tc.number: PassPrimaryInner_0100
  * @tc.name: PassPrimaryInner_0100
  * @tc.desc: Verify the PassPrimaryInner function when bool value in data is false.
  */
HWTEST_F(ReverseContinuationSchedulerReplicaStubTest, PassPrimaryInner_0100, Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuationSchedulerReplicaStubTest PassPrimaryInner_0100 start");
    MessageParcel data;
    MessageParcel reply;

    auto stub = std::make_shared<MockReverseContinuationSchedulerReplicaStub>();
    EXPECT_CALL(*stub, PassPrimary(_)).Times(1);
    int32_t result = stub->PassPrimaryInner(data, reply);
    TAG_LOGI(AAFwkTag::TEST, "result is %{public}d", result);
    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuationSchedulerReplicaStubTest PassPrimaryInner_0100 end");
}

 /**
  * @tc.number: PassPrimaryInner_0200
  * @tc.name: PassPrimaryInner_0200
  * @tc.desc: Verify the PassPrimaryInner function when bool value in data is true.
  */
HWTEST_F(ReverseContinuationSchedulerReplicaStubTest, PassPrimaryInner_0200, Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuationSchedulerReplicaStubTest PassPrimaryInner_0200 start");
    MessageParcel data;
    MessageParcel reply;

    data.WriteBool(true);
    auto stub = std::make_shared<MockReverseContinuationSchedulerReplicaStub>();
    EXPECT_CALL(*stub, PassPrimary(_)).Times(1);
    int32_t result = stub->PassPrimaryInner(data, reply);
    TAG_LOGI(AAFwkTag::TEST, "result is %{public}d", result);
    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuationSchedulerReplicaStubTest PassPrimaryInner_0200 end");
}

 /**
  * @tc.number: ReverseContinuationInner_0100
  * @tc.name: ReverseContinuationInner_0100
  * @tc.desc: Verify the ReverseContinuationInner function.
  */
HWTEST_F(ReverseContinuationSchedulerReplicaStubTest, ReverseContinuationInner_0100, Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuationSchedulerReplicaStubTest ReverseContinuationInner_0100 start");
    MessageParcel data;
    MessageParcel reply;

    auto stub = std::make_shared<MockReverseContinuationSchedulerReplicaStub>();
    EXPECT_CALL(*stub, ReverseContinuation()).Times(1);
    int32_t result = stub->ReverseContinuationInner(data, reply);
    TAG_LOGI(AAFwkTag::TEST, "result is %{public}d", result);
    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuationSchedulerReplicaStubTest ReverseContinuationInner_0100 end");
}

 /**
  * @tc.number: NotifyReverseResultInner_0100
  * @tc.name: NotifyReverseResultInner_0100
  * @tc.desc: Verify the NotifyReverseResultInner function.
  */
HWTEST_F(ReverseContinuationSchedulerReplicaStubTest, NotifyReverseResultInner_0100, Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuationSchedulerReplicaStubTest NotifyReverseResultInner_0100 start");
    MessageParcel data;
    MessageParcel reply;

    auto stub = std::make_shared<MockReverseContinuationSchedulerReplicaStub>();
    EXPECT_CALL(*stub, NotifyReverseResult(_)).Times(1);
    int32_t result = stub->NotifyReverseResultInner(data, reply);
    TAG_LOGI(AAFwkTag::TEST, "result is %{public}d", result);
    EXPECT_EQ(result, NO_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuationSchedulerReplicaStubTest NotifyReverseResultInner_0100 end");
}

 /**
  * @tc.number: OnRemoteRequest_0100
  * @tc.name: OnRemoteRequest_0100
  * @tc.desc: Verify the OnRemoteRequest function when descriptor is not equal.
  */
HWTEST_F(ReverseContinuationSchedulerReplicaStubTest, OnRemoteRequest_0100, Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuationSchedulerReplicaStubTest OnRemoteRequest_0100 start");
    uint32_t code = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    auto stub = std::make_shared<MockReverseContinuationSchedulerReplicaStub>();
    int32_t result = stub->OnRemoteRequest(code, data, reply, option);
    TAG_LOGI(AAFwkTag::TEST, "result is %{public}d", result);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuationSchedulerReplicaStubTest OnRemoteRequest_0100 end");
}

 /**
  * @tc.number: OnRemoteRequest_0200
  * @tc.name: OnRemoteRequest_0200
  * @tc.desc: Verify the OnRemoteRequest function when descriptor is equal.
  */
HWTEST_F(ReverseContinuationSchedulerReplicaStubTest, OnRemoteRequest_0200, Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuationSchedulerReplicaStubTest OnRemoteRequest_0200 start");
    uint32_t code = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(u"ohos.appexecfwk.IReverseContinuationSchedulerReplica");
    auto stub = std::make_shared<MockReverseContinuationSchedulerReplicaStub>();
    int32_t result = stub->OnRemoteRequest(code, data, reply, option);
    TAG_LOGI(AAFwkTag::TEST, "result is %{public}d", result);
    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuationSchedulerReplicaStubTest OnRemoteRequest_0200 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
