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

#include "render_state_observer_proxy.h"
#define private public
#include "render_state_observer_manager.h"
#undef private
#include "render_state_observer_stub.h"

using namespace testing;
using namespace testing::ext;

int32_t onRenderStateChangedResult = 0;

namespace OHOS {
namespace AppExecFwk {
class MockRenderStateObserver : public RenderStateObserverStub {
public:
    MockRenderStateObserver() = default;
    virtual ~MockRenderStateObserver() = default;
    void OnRenderStateChanged(const RenderStateData &renderStateData) override
    {
        onRenderStateChangedResult = 1;
    }
};

class RenderStateObserverManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void RenderStateObserverManagerTest::SetUpTestCase()
{}

void RenderStateObserverManagerTest::TearDownTestCase()
{}

void RenderStateObserverManagerTest::SetUp()
{
    onRenderStateChangedResult = 0;
}

void RenderStateObserverManagerTest::TearDown()
{}

/**
 * @tc.name: RegisterRenderStateObserver_0100
 * @tc.desc: Test regiter nullptr return error.
 * @tc.type: FUNC
 */
HWTEST_F(RenderStateObserverManagerTest, RegisterRenderStateObserver_0100, TestSize.Level1)
{
    auto manager = std::make_shared<RenderStateObserverManager>();
    manager->Init();
    int32_t res = manager->RegisterRenderStateObserver(nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RegisterRenderStateObserver_0200
 * @tc.desc: Test register observer return OK.
 * @tc.type: FUNC
 */
HWTEST_F(RenderStateObserverManagerTest, RegisterRenderStateObserver_0200, TestSize.Level1)
{
    auto manager = std::make_shared<RenderStateObserverManager>();
    manager->Init();
    sptr<IRenderStateObserver> observer = new MockRenderStateObserver();
    int32_t res = manager->RegisterRenderStateObserver(observer);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: UnregisterRenderStateObserver_0100
 * @tc.desc: Test unregister nullptr return error.
 * @tc.type: FUNC
 */
HWTEST_F(RenderStateObserverManagerTest, UnregisterRenderStateObserver_0100, TestSize.Level1)
{
    auto manager = std::make_shared<RenderStateObserverManager>();
    manager->Init();
    int32_t res = manager->UnregisterRenderStateObserver(nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: UnregisterRenderStateObserver_0200
 * @tc.desc: Test unregister observer return OK.
 * @tc.type: FUNC
 */
HWTEST_F(RenderStateObserverManagerTest, UnregisterRenderStateObserver_0200, TestSize.Level1)
{
    auto manager = std::make_shared<RenderStateObserverManager>();
    manager->Init();
    sptr<IRenderStateObserver> observer = new MockRenderStateObserver();
    int32_t res = manager->RegisterRenderStateObserver(observer);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: OnRenderStateChanged_0100
 * @tc.desc: Test observer can be handled.
 * @tc.type: FUNC
 */
HWTEST_F(RenderStateObserverManagerTest, OnRenderStateChanged_0100, TestSize.Level1)
{
    auto manager = std::make_shared<RenderStateObserverManager>();
    manager->Init();
    sptr<IRenderStateObserver> observer = new MockRenderStateObserver();
    manager->RegisterRenderStateObserver(observer);
    std::shared_ptr<RenderRecord> renderRecord;
    int res = manager->OnRenderStateChanged(renderRecord, 0);
    EXPECT_EQ(res, ERR_OK);
    EXPECT_EQ(onRenderStateChangedResult, 1);
}

/**
 * @tc.name: OnRenderStateChanged_0200
 * @tc.desc: Test handle nothing without observer.
 * @tc.type: FUNC
 */
HWTEST_F(RenderStateObserverManagerTest, OnRenderStateChanged_0200, TestSize.Level1)
{
    auto manager = std::make_shared<RenderStateObserverManager>();
    manager->Init();
    std::shared_ptr<RenderRecord> renderRecord;
    int res = manager->OnRenderStateChanged(renderRecord, 0);
    EXPECT_EQ(res, ERR_OK);
    EXPECT_EQ(onRenderStateChangedResult, 0);
}

/**
 * @tc.name: OnObserverDied_0100
 * @tc.desc: Test handle nothing when the observer died.
 * @tc.type: FUNC
 */
HWTEST_F(RenderStateObserverManagerTest, OnObserverDied_0100, TestSize.Level1)
{
    auto manager = std::make_shared<RenderStateObserverManager>();
    manager->Init();
    sptr<IRenderStateObserver> observer = new MockRenderStateObserver();
    manager->RegisterRenderStateObserver(observer);
    manager->OnObserverDied(observer);
    std::shared_ptr<RenderRecord> renderRecord;
    int res = manager->OnRenderStateChanged(renderRecord, 0);
    EXPECT_EQ(res, ERR_OK);
    EXPECT_EQ(onRenderStateChangedResult, 0);
}
} // namespace AppExecFwk
} // namespace OHOS