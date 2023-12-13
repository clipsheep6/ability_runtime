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
#include "ability_info_callback_stub.h"
#include "app_state_data.h"
#include "iremote_stub.h"
#include "mock_app_foreground_state_observer_stub.h"
#include "parcel.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppForegroundStateObserverStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<AppForegroundStateObserverStub> observerStub_ {nullptr};
};

void AppForegroundStateObserverStubTest::SetUpTestCase(void)
{}

void AppForegroundStateObserverStubTest::TearDownTestCase(void)
{}

void AppForegroundStateObserverStubTest::SetUp()
{}

void AppForegroundStateObserverStubTest::TearDown()
{}

/**
 * @tc.number: HandleOnAppStateChanged_0100
 * @tc.desc: Test when processData is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppForegroundStateObserverStubTest, HandleOnAppStateChanged_0100, TestSize.Level1)
{
    auto stub = new (std::nothrow) MockAppForegroundStateObserverStub();
    EXPECT_TRUE(stub);
    EXPECT_CALL(*stub, OnAppStateChanged(_)).Times(1);
    MessageParcel data;
    AppStateData appStateData;
    data.WriteParcelable(&appStateData);
    MessageParcel reply;
    auto result = stub->HandleOnAppStateChanged(data, reply);
    EXPECT_EQ(NO_ERROR, result);
}

/**
 * @tc.number: OnRemoteRequest_0100
 * @tc.desc: Test when descriptor and remoteDescriptor is different and 
 *      itFunc is not end memberFunc is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AppForegroundStateObserverStubTest, OnRemoteRequest_0100, TestSize.Level1)
{
    auto stub = new (std::nothrow) MockAppForegroundStateObserverStub();
    EXPECT_NE(nullptr, stub);
    int32_t resultCode = 0;
    int32_t userId = 0;
    MessageParcel data;
    data.WriteInterfaceToken(stub->GetDescriptor());
    MessageParcel reply;
    uint32_t code = 1;
    MessageOption option;
    auto res = stub->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(NO_ERROR, res);
}
} // namespace AppExecFwk 
} // namespace OHOS 
