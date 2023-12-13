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
#include "app_foreground_state_observer_proxy.h"
#include "mock_app_foreground_state_observer_stub.h"
#include "peer_holder.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppForegroundStateObserverProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    sptr<AppForegroundStateObserverProxy> observerProxy_;    
    sptr<MockAppForegroundStateObserverStub> mock_;
};

void AppForegroundStateObserverProxyTest::SetUpTestCase(void)
{}

void AppForegroundStateObserverProxyTest::TearDownTestCase(void)
{}

void AppForegroundStateObserverProxyTest::SetUp()
{
    mock_ = new (std::nothrow) MockAppForegroundStateObserverStub();
    observerProxy_ = new (std::nothrow) AppForegroundStateObserverProxy(mock_);
}

void AppForegroundStateObserverProxyTest::TearDown()
{} 

/**
 * @tc.name: WriteInterfaceToken_0100
 * @tc.desc: Write token into parcel data.
 * @tc.type: FUNC
 */
HWTEST_F(AppForegroundStateObserverProxyTest, WriteInterfaceToken_0100, TestSize.Level1)
{
    MessageParcel data;
    EXPECT_TRUE(observerProxy_->WriteInterfaceToken(data));
}

/**
 * @tc.name: OnAppStateChanged_0100
 * @tc.desc: Test when the return of WriteInterfaceToken and 
 *      WriteParcelable is true and remote is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppForegroundStateObserverProxyTest, OnAppStateChanged_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, OnAppStateChanged(_)).Times(1);
    AppStateData appStateData;
    observerProxy_->OnAppStateChanged(appStateData);  
}
} // namespace AppExecFwk
} // namespace OHOS
