/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "ability_foreground_state_observer_stub.h"
#undef private
#undef protected
#include "mock_ability_foreground_state_observer_server_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AbilityRuntime {
class AbilityForegroundStateObserverStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    sptr<AbilityForegroundStateObserverStub> observerStub_;
};

void AbilityForegroundStateObserverStubTest::SetUpTestCase()
{}

void AbilityForegroundStateObserverStubTest::TearDownTestCase()
{}

void AbilityForegroundStateObserverStubTest::SetUp()
{
    observerStub_ = new MockAbilityForegroundStateObserverServerStub();
}

void AbilityForegroundStateObserverStubTest::TearDown()
{}

/**
 * @tc.number: OnRemoteRequest_0100
 * @tc.desc: Verify the normal process of onremoterequest.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityForegroundStateObserverStubTest, OnRemoteRequest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest_0100 start";
    auto code = static_cast<uint32_t>(IAbilityForegroundStateObserver::Message::ON_ABILITY_STATE_CHANGED);
    MessageParcel data;
    AbilityStateData abilityStateData;
    data.WriteInterfaceToken(AbilityForegroundStateObserverStub::GetDescriptor());
    data.WriteParcelable(&abilityStateData);
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
HWTEST_F(AbilityForegroundStateObserverStubTest, OnRemoteRequest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest_0200 start";
    auto code = static_cast<uint32_t>(IAbilityForegroundStateObserver::Message::ON_ABILITY_STATE_CHANGED);
    MessageParcel data;
    AbilityStateData abilityStateData;
    data.WriteParcelable(&abilityStateData);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t result = observerStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    GTEST_LOG_(INFO) << "OnRemoteRequest_0200 end";
}

/**
 * @tc.number: HandleOnAbilityStateChanged_0100
 * @tc.desc: Verify the normal process of HandleOnAbilityStateChanged.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityForegroundStateObserverStubTest, HandleOnAbilityStateChanged_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnAbilityStateChanged_0100 start";
    AbilityStateData abilityStateData;
    MessageParcel data;
    data.WriteParcelable(&abilityStateData);
    MessageParcel reply;
    auto res = observerStub_->HandleOnAbilityStateChanged(data, reply);
    EXPECT_EQ(res, NO_ERROR);
    GTEST_LOG_(INFO) << "HandleOnAbilityStateChanged_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
