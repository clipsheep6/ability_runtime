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
#include "iremote_stub.h"
#include "mock_render_state_observer_stub.h"
#include "parcel.h"
#undef protected
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class RenderStateObserverStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<RenderStateObserverStub> observerStub_ {nullptr};
};

void RenderStateObserverStubTest::SetUpTestCase(void)
{}

void RenderStateObserverStubTest::TearDownTestCase(void)
{}

void RenderStateObserverStubTest::SetUp()
{}

void RenderStateObserverStubTest::TearDown()
{}

/**
 * @tc.number: OnRenderStateChangedInner_0100
 * @tc.desc: Test when params is not nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(RenderStateObserverStubTest, OnRenderStateChangedInner_0100, TestSize.Level1)
{
    auto stub = new (std::nothrow) MockRenderStateObserverStub();
    EXPECT_TRUE(stub);
    EXPECT_CALL(*stub, OnRenderStateChanged(_, _)).Times(1);
    MessageParcel data;
    pid_t renderPid = 0;
    int32_t state = 0;
    data.WriteInt32(renderPid);
    data.WriteInt32(state);
    MessageParcel reply;
    auto result = stub->OnRenderStateChangedInner(data, reply);
    EXPECT_EQ(NO_ERROR, result);
}

} // namespace AppExecFwk
} // namespace OHOS