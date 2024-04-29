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
#include "ability_handler.h"
#include "continuation_handler.h"
#include "continuation_manager.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "mock_new_ability.h"
#include "mock_reverse_continuation_scheduler_replica_stub.h"
#include "reverse_continuation_scheduler_primary.h"
#include "reverse_continuation_scheduler_replica_proxy.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class ReverseContinuationSchedulerReplicaProxyTest : public testing::Test {
public:
    ReverseContinuationSchedulerReplicaProxyTest()
    {}
    ~ReverseContinuationSchedulerReplicaProxyTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ReverseContinuationSchedulerReplicaProxyTest::SetUpTestCase(void)
{}

void ReverseContinuationSchedulerReplicaProxyTest::TearDownTestCase(void)
{}

void ReverseContinuationSchedulerReplicaProxyTest::SetUp(void)
{}

void ReverseContinuationSchedulerReplicaProxyTest::TearDown(void)
{}

 /**
  * @tc.number: PassPrimary_0100
  * @tc.name: PassPrimary_0100
  * @tc.desc: Verify the PassPrimary function when remoteObject is nullptr.
  */
HWTEST_F(ReverseContinuationSchedulerReplicaProxyTest, PassPrimary_0100, Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PassPrimary_0100 start");
    auto continuationManager = std::make_shared<ContinuationManager>();
    std::weak_ptr<ContinuationManager> weakContinuationManager = continuationManager;
    std::weak_ptr<Ability> weakMockAbility = std::make_shared<MockNewAbility>();
    auto continuationHandler = std::make_shared<ContinuationHandler>(weakContinuationManager, weakMockAbility);
    std::weak_ptr<ContinuationHandler> weakContinuationHandler = continuationHandler;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    auto primary = sptr<ReverseContinuationSchedulerPrimary>(
        new (std::nothrow) ReverseContinuationSchedulerPrimary(continuationHandler, handler));

    auto proxy = std::make_shared<ReverseContinuationSchedulerReplicaProxy>(nullptr);
    proxy->PassPrimary(primary);
    sptr<MockReverseContinuationSchedulerReplicaStub> mockStub(new MockReverseContinuationSchedulerReplicaStub());
    EXPECT_FALSE(mockStub->SendRequest_called);
    TAG_LOGI(AAFwkTag::TEST, "PassPrimary_0100 end");
}

 /**
  * @tc.number: PassPrimary_0200
  * @tc.name: PassPrimary_0200
  * @tc.desc: Verify the PassPrimary function when remoteObject is normal value.
  */
HWTEST_F(ReverseContinuationSchedulerReplicaProxyTest, PassPrimary_0200, Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PassPrimary_0200 start");
    auto continuationManager = std::make_shared<ContinuationManager>();
    std::weak_ptr<ContinuationManager> weakContinuationManager = continuationManager;
    std::weak_ptr<Ability> weakMockAbility = std::make_shared<MockNewAbility>();
    auto continuationHandler
        = std::make_shared<ContinuationHandler>(weakContinuationManager, weakMockAbility);
    std::weak_ptr<ContinuationHandler> weakContinuationHandler = continuationHandler;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    auto primary = sptr<ReverseContinuationSchedulerPrimary>(
        new (std::nothrow) ReverseContinuationSchedulerPrimary(continuationHandler, handler));

    sptr<MockReverseContinuationSchedulerReplicaStub> mockStub(new MockReverseContinuationSchedulerReplicaStub());
    auto proxy = std::make_shared<ReverseContinuationSchedulerReplicaProxy>(mockStub);
    proxy->PassPrimary(primary);
    EXPECT_TRUE(mockStub->SendRequest_called);
    TAG_LOGI(AAFwkTag::TEST, "PassPrimary_0200 end");
}

 /**
  * @tc.number: PassPrimary_0300
  * @tc.name: PassPrimary_0300
  * @tc.desc: Verify the PassPrimary function when primary is nullptr.
  */
HWTEST_F(ReverseContinuationSchedulerReplicaProxyTest, PassPrimary_0300, Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PassPrimary_0300 start");
    sptr<ReverseContinuationSchedulerPrimary> primary = nullptr;
    sptr<MockReverseContinuationSchedulerReplicaStub> mockStub(new MockReverseContinuationSchedulerReplicaStub());
    auto proxy = std::make_shared<ReverseContinuationSchedulerReplicaProxy>(mockStub);
    proxy->PassPrimary(primary);
    EXPECT_TRUE(mockStub->SendRequest_called);
    TAG_LOGI(AAFwkTag::TEST, "PassPrimary_0300 end");
}

 /**
  * @tc.number: ReverseContinuation_0100
  * @tc.name: ReverseContinuation_0100
  * @tc.desc: Verify the ReverseContinuation function when primary is nullptr.
  */
HWTEST_F(ReverseContinuationSchedulerReplicaProxyTest, ReverseContinuation_0100, Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuation_0100 start");
    auto proxy = std::make_shared<ReverseContinuationSchedulerReplicaProxy>(nullptr);
    bool result = proxy->ReverseContinuation();
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuation_0100 end");
}

 /**
  * @tc.number: ReverseContinuation_0200
  * @tc.name: ReverseContinuation_0200
  * @tc.desc: Verify the ReverseContinuation function when remoteObject is normal value.
  */
HWTEST_F(ReverseContinuationSchedulerReplicaProxyTest, ReverseContinuation_0200, Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuation_0200 start");
    sptr<ReverseContinuationSchedulerReplicaStub> stub(new MockReverseContinuationSchedulerReplicaStub());
    auto proxy = std::make_shared<ReverseContinuationSchedulerReplicaProxy>(stub);
    bool result = proxy->ReverseContinuation();
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "ReverseContinuation_0200 end");
}

 /**
  * @tc.number: NotifyReverseResult_0100
  * @tc.name: NotifyReverseResult_0100
  * @tc.desc: Verify the NotifyReverseResult function when remoteObject is nullptr.
  */
HWTEST_F(ReverseContinuationSchedulerReplicaProxyTest, NotifyReverseResult_0100, Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyReverseResult_0100 start");
    auto proxy = std::make_shared<ReverseContinuationSchedulerReplicaProxy>(nullptr);
    proxy->NotifyReverseResult(0);
    sptr<MockReverseContinuationSchedulerReplicaStub> mockStub(new MockReverseContinuationSchedulerReplicaStub());
    EXPECT_FALSE(mockStub->SendRequest_called);
    TAG_LOGI(AAFwkTag::TEST, "NotifyReverseResult_0100 end");
}

 /**
  * @tc.number: NotifyReverseResult_0200
  * @tc.name: NotifyReverseResult_0200
  * @tc.desc: Verify the NotifyReverseResult function when remoteObject is normal value.
  */
HWTEST_F(ReverseContinuationSchedulerReplicaProxyTest, NotifyReverseResult_0200, Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyReverseResult_0200 start");
    sptr<MockReverseContinuationSchedulerReplicaStub> mockStub(new MockReverseContinuationSchedulerReplicaStub());
    auto proxy = std::make_shared<ReverseContinuationSchedulerReplicaProxy>(mockStub);
    proxy->NotifyReverseResult(1);
    EXPECT_TRUE(mockStub->SendRequest_called);
    TAG_LOGI(AAFwkTag::TEST, "NotifyReverseResult_0200 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
