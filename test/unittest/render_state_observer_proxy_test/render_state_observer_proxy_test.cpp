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
#include "mock_render_state_observer_stub.h"
#include "render_state_observer_proxy.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class RenderStateObserverProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    sptr<RenderStateObserverProxy> observerProxy_;
    sptr<MockRenderStateObserverStub> mock_;
};

void RenderStateObserverProxyTest::SetUpTestCase(void) {}

void RenderStateObserverProxyTest::TearDownTestCase(void) {}

void RenderStateObserverProxyTest::SetUp()
{
    mock_ = new (std::nothrow) MockRenderStateObserverStub();
    observerProxy_ = new (std::nothrow) RenderStateObserverProxy(mock_);
}

void RenderStateObserverProxyTest::TearDown() {}

/**
 * @tc.name: WriteInterfaceToken_0100
 * @tc.desc: Write token into parcel data.
 * @tc.type: FUNC
 */
HWTEST_F(RenderStateObserverProxyTest, WriteInterfaceToken_0100, TestSize.Level1)
{
    MessageParcel data;
    EXPECT_TRUE(observerProxy_->WriteInterfaceToken(data));
}

/**
 * @tc.name: OnRenderStateChanged_0100
 * @tc.desc: Test when the return of WriteInterfaceToken and
 *      WriteParcelable is true and remote is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(RenderStateObserverProxyTest, OnRenderStateChanged_0100, TestSize.Level1)
{
    EXPECT_CALL(*mock_, OnRenderStateChanged(_, _)).Times(1);
    pid_t renderPid = 0;
    int state = 0;
    observerProxy_->OnRenderStateChanged(renderPid, state);
}
} // namepsace AppExecFwk
} // namespace OHOS