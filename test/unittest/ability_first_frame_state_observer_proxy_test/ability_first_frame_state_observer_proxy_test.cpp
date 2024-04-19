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
#include "ability_first_frame_state_observer_proxy.h"
#include "ability_first_frame_state_data.h"
#undef private
#include "mock_ability_first_frame_state_observer_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AbilityRuntime {
class AbilityFirstFrameStateObserverProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    sptr<AbilityFirstFrameStateObserverProxy> observerProxy_ {nullptr};
    sptr<MockAbilityFirstFrameStateObserverStub> mock_;
};
void AbilityFirstFrameStateObserverProxyTest::SetUpTestCase()
{}

void AbilityFirstFrameStateObserverProxyTest::TearDownTestCase()
{}

void AbilityFirstFrameStateObserverProxyTest::SetUp()
{
    mock_ = new MockAbilityFirstFrameStateObserverStub();
    observerProxy_ = new AbilityFirstFrameStateObserverProxy(mock_);
}

void AbilityFirstFrameStateObserverProxyTest::TearDown()
{}

/**
 * @tc.number: WriteInterfaceToken_0100
 * @tc.desc: Write token into parcel data.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityFirstFrameStateObserverProxyTest, WriteInterfaceToken_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteInterfaceToken_0100 start";
    MessageParcel data;
    auto ret = observerProxy_->WriteInterfaceToken(data);
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "WriteInterfaceToken_0100 end";
}

/**
 * @tc.number: OnAbilityFirstFrameState_0100
 * @tc.desc: Test by determine whether sendRequest has been executed.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityFirstFrameStateObserverProxyTest, OnAbilityFirstFrameState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityFirstFrameState_0100 start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &MockAbilityFirstFrameStateObserverStub::InvokeSendRequest));
    AbilityFirstFrameStateData abilityFirstFrameStateData;
    observerProxy_->OnAbilityFirstFrameState(abilityFirstFrameStateData);
    EXPECT_EQ(
        static_cast<uint32_t>(IAbilityFirstFrameStateObserver::Message::ON_ABILITY_FIRST_FRAME_STATE),
        mock_->GetCode());
    GTEST_LOG_(INFO) << "OnAbilityFirstFrameState_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
