/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "ability_first_frame_state_observer_stub.h"
#undef private
#undef protected
#include "mock_ability_first_frame_state_observer_server_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AbilityRuntime {
class AbilityFirstFrameStateObserverStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    sptr<AbilityFirstFrameStateObserverStub> observerStub_;
};

void AbilityFirstFrameStateObserverStubTest::SetUpTestCase()
{}

void AbilityFirstFrameStateObserverStubTest::TearDownTestCase()
{}

void AbilityFirstFrameStateObserverStubTest::SetUp()
{
    observerStub_ = new MockAbilityFirstFrameStateObserverServerStub();
}

void AbilityFirstFrameStateObserverStubTest::TearDown()
{}

/**
 * @tc.number: OnRemoteRequest_0100
 * @tc.desc: Verify the normal process of onremoterequest.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityFirstFrameStateObserverStubTest, OnRemoteRequest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest_0100 start";
    auto code = static_cast<uint32_t>(IAbilityFirstFrameStateObserver::Message::ON_ABILITY_FIRST_FRAME_STATE);
    MessageParcel data;
    AbilityFirstFrameStateData abilityFirstFrameStateData;
    data.WriteInterfaceToken(AbilityFirstFrameStateObserverStub::GetDescriptor());
    data.WriteParcelable(&abilityFirstFrameStateData);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t result = observerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
    GTEST_LOG_(INFO) << "OnRemoteRequest_0100 end";
}

/**
 * @tc.number: OnRemoteRequest_0200
 * @tc.desc: Verify the exception process of onremoterequest.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityFirstFrameStateObserverStubTest, OnRemoteRequest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest_0200 start";
    auto code = static_cast<uint32_t>(IAbilityFirstFrameStateObserver::Message::ON_ABILITY_FIRST_FRAME_STATE);
    MessageParcel data;
    AbilityFirstFrameStateData abilityFirstFrameStateData;
    data.WriteParcelable(&abilityFirstFrameStateData);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t result = observerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    GTEST_LOG_(INFO) << "OnRemoteRequest_0200 end";
}

/**
 * @tc.number: HandleOnAbilityFirstFrameStateChanged_0100
 * @tc.desc: Verify the normal process of HandleOnAbilityFirstFrameStateChanged.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityFirstFrameStateObserverStubTest, HandleOnAbilityFirstFrameStateChanged_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnAbilityFirstFrameStateChanged_0100 start";
    AbilityFirstFrameStateData abilityFirstFrameStateData;
    MessageParcel data;
    data.WriteParcelable(&abilityFirstFrameStateData);
    MessageParcel reply;
    auto res = observerStub_->HandleOnAbilityFirstFrameStateChanged(data, reply);
    EXPECT_EQ(res, NO_ERROR);
    GTEST_LOG_(INFO) << "HandleOnAbilityFirstFrameStateChanged_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
