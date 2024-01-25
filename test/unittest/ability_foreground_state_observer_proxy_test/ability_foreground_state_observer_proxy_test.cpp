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
#include "ability_foreground_state_observer_proxy.h"
#include "ability_state_data.h"
#undef private
#include "mock_ability_foreground_state_observer_stub.h"
#include "mock_ability_foreground_state_observer_server_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AbilityRuntime {
class AbilityForegroundStateObserverProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    sptr<AbilityForegroundStateObserverProxy> observerProxy_ {nullptr};
    sptr<MockAbilityForegroundStateObserverStub> mock_;
};
void AbilityForegroundStateObserverProxyTest::SetUpTestCase()
{}

void AbilityForegroundStateObserverProxyTest::TearDownTestCase()
{}

void AbilityForegroundStateObserverProxyTest::SetUp()
{
    mock_ = new MockAbilityForegroundStateObserverStub();
    observerProxy_ = new AbilityForegroundStateObserverProxy(mock_);
}

void AbilityForegroundStateObserverProxyTest::TearDown()
{}

/**
 * @tc.number: WriteInterfaceToken_0100
 * @tc.desc: Write token into parcel data.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityForegroundStateObserverProxyTest, WriteInterfaceToken_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteInterfaceToken_0100 start";
    MessageParcel data;
    auto ret = observerProxy_->WriteInterfaceToken(data);
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "WriteInterfaceToken_0100 end";
}

/**
 * @tc.number: OnAbilityStateChanged_0100
 * @tc.desc: Test by determine whether sendRequest has been executed.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityForegroundStateObserverProxyTest, OnAbilityStateChanged_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilityStateChanged_0100 start";
    AbilityStateData abilityStateData;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &MockAbilityForegroundStateObserverStub::InvokeSendRequest));
    observerProxy_->OnAbilityStateChanged(abilityStateData);
    EXPECT_EQ(
        static_cast<uint32_t>(IAbilityForegroundStateObserver::Message::ON_ABILITY_STATE_CHANGED), mock_->GetCode());
    GTEST_LOG_(INFO) << "OnAbilityStateChanged_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
